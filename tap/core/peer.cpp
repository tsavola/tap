#include "core.hpp"

#include <cstdio>

namespace tap {

struct PeerObject::State {
	enum {
		DIRTY_FLAG     = 1 << 0,
		REFERENCE_FLAG = 1 << 1,
	};

	State() noexcept:
		key(-1),
		flags(0)
	{
	}

	State(Key key, unsigned int flags) noexcept:
		key(key),
		flags(flags)
	{
	}

	State &operator=(const State &other) noexcept
	{
		key = other.key;
		flags = other.flags;
		return *this;
	}

	void set_flag(unsigned int mask) noexcept
	{
		flags |= mask;
	}

	void clear_flag(unsigned int mask) noexcept
	{
		flags &= ~mask;
	}

	bool test_flag(unsigned int mask) const noexcept
	{
		return flags & mask;
	}

	Key key;
	unsigned int flags;
};

PeerObject::PeerObject():
	next_object_id(0)
{
	instance_peers().insert(this);
}

PeerObject::~PeerObject()
{
	instance_peers().erase(this);

	for (auto pair: states) {
		if (pair.second.test_flag(State::REFERENCE_FLAG))
			Py_DECREF(pair.first);
	}
}

int PeerObject::insert(PyObject *object, Key key) noexcept
{
	return insert(object, key, 0);
}

int PeerObject::insert(PyObject *object, Key key, unsigned int flags) noexcept
{
	try {
		states[object] = State(key, flags);
	} catch (...) {
		return -1;
	}

	try {
		objects[key] = object;
	} catch (...) {
		states.erase(object);
		return -1;
	}

	return 0;
}

Key PeerObject::insert_new(PyObject *object, unsigned int flags) noexcept
{
	Key key = next_object_id;

	if (insert(object, key, flags) < 0)
		return -1;

	next_object_id++;

	return key;
}

void PeerObject::clear(PyObject *object) noexcept
{
	auto i = states.find(object);
	if (i != states.end())
		i->second.clear_flag(State::DIRTY_FLAG);
}

std::pair<Key, bool> PeerObject::insert_or_clear_for_remote(PyObject *object) noexcept
{
	bool object_changed;
	Key key;

	auto i = states.find(object);
	if (i != states.end()) {
		object_changed = i->second.test_flag(State::DIRTY_FLAG);
		i->second.clear_flag(State::DIRTY_FLAG);
		key = i->second.key;
	} else {
		object_changed = true;
		key = insert_new(object, 0);
	}

	Key remote_key = key_for_remote(key);

	return std::make_pair(remote_key, object_changed);
}

Key PeerObject::key_for_remote(PyObject *object) noexcept
{
	Key key;

	auto i = states.find(object);
	if (i != states.end())
		key = i->second.key;
	else
		key = insert_new(object, State::DIRTY_FLAG);

	return key_for_remote(key);
}

Key PeerObject::key_for_remote(Key key) noexcept
{
	if (key < 0)
		return -1;

	uint32_t object_id = key;
	int32_t remote_id = key >> 32;

	remote_id = !remote_id;

	return (Key(remote_id) << 32) | object_id;
}

PyObject *PeerObject::object(Key key) noexcept
{
	PyObject *object = nullptr;

	auto i = objects.find(key);
	if (i != objects.end()) {
		object = i->second;

		if (object->ob_refcnt <= 0) {
			fprintf(stderr, "tap peer: %s object %p with invalid reference count %ld during lookup\n", object->ob_type->tp_name, object, object->ob_refcnt);
			object = nullptr;
		}
	}

	return object;
}

void PeerObject::touch(PyObject *object) noexcept
{
	auto i = states.find(object);
	if (i != states.end())
		i->second.set_flag(State::DIRTY_FLAG);
}

void PeerObject::set_references(const std::unordered_set<PyObject *> &referenced) noexcept
{
	for (PyObject *object: referenced)
		states.find(object)->second.set_flag(State::REFERENCE_FLAG);
}

void PeerObject::object_freed(void *ptr) noexcept
{
	auto i = states.find(ptr);
	if (i != states.end()) {
		PyObject *object = reinterpret_cast<PyObject *> (ptr);

		fprintf(stderr, "tap peer: %s object %p freed\n", object->ob_type->tp_name, object);

		if (object->ob_refcnt != 0)
			fprintf(stderr, "tap peer: %s object %p with unexpected reference count %ld when freed\n", object->ob_type->tp_name, object, object->ob_refcnt);

		Key key = i->second.key;

		objects.erase(key);
		states.erase(i);

		freed.push_back(key);
	}
}

static PyObject *peer_new(PyTypeObject *type, PyObject *args, PyObject *kwargs) noexcept
{
	PyObject *peer = type->tp_alloc(type, 0);
	if (peer) {
		try {
			new (peer) PeerObject;
		} catch (...) {
			type->tp_free(peer);
			peer = nullptr;
		}
	}

	return peer;
}

static void peer_dealloc(PyObject *peer) noexcept
{
	reinterpret_cast<PeerObject *> (peer)->~PeerObject();
	Py_TYPE(peer)->tp_free(peer);
}

int peer_type_init() noexcept
{
	return PyType_Ready(&peer_type);
}

PyTypeObject peer_type = {
	PyVarObject_HEAD_INIT(nullptr, 0)
	"tap.core.Peer",                /* tp_name */
	sizeof (PeerObject),            /* tp_basicsize */
	0,                              /* tp_itemsize */
	peer_dealloc,                   /* tp_dealloc */
	0,                              /* tp_print */
	0,                              /* tp_getattr */
	0,                              /* tp_setattr */
	0,                              /* tp_reserved */
	0,                              /* tp_repr */
	0,                              /* tp_as_number */
	0,                              /* tp_as_sequence */
	0,                              /* tp_as_mapping */
	0,                              /* tp_hash  */
	0,                              /* tp_call */
	0,                              /* tp_str */
	0,                              /* tp_getattro */
	0,                              /* tp_setattro */
	0,                              /* tp_as_buffer */
	Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,  /* tp_flags */
	nullptr,                        /* tp_doc */
	0,                              /* tp_traverse */
	0,                              /* tp_clear */
	0,                              /* tp_richcompare */
	0,                              /* tp_weaklistoffset */
	0,                              /* tp_iter */
	0,                              /* tp_iternext */
	0,                              /* tp_methods */
	0,                              /* tp_members */
	0,                              /* tp_getset */
	0,                              /* tp_base */
	0,                              /* tp_dict */
	0,                              /* tp_descr_get */
	0,                              /* tp_descr_set */
	0,                              /* tp_dictoffset */
	0,                              /* tp_init */
	0,                              /* tp_alloc */
	peer_new,                       /* tp_new */
};

void peers_touch(PyObject *object) noexcept
{
	for (PeerObject *peer: instance_peers())
		peer->touch(object);
}

} // namespace tap
