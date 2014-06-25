#include "core.hpp"

#include <cstdio>

namespace tap {

class PeerObject::State
{
public:
	State() throw ()
	{
	}

	State(Key key, bool dirty) throw ():
		key(key),
		dirty(dirty)
	{
	}

	State(const State &other) throw ():
		key(other.key),
		dirty(other.dirty)
	{
	}

	State &operator=(const State &other) throw ()
	{
		key = other.key;
		dirty = other.dirty;
		return *this;
	}

	Key key;
	bool dirty;
};

PeerObject::PeerObject() throw (std::bad_alloc):
	next_key(0)
{
	instance_peers().insert(this);
}

PeerObject::~PeerObject()
{
	instance_peers().erase(this);
}

int PeerObject::insert(PyObject *object, Key key, bool dirty)
{
	try {
		states[object] = State(key, dirty);
	} catch (std::bad_alloc) {
		return -1;
	}

	try {
		objects[key] = object;
	} catch (std::bad_alloc) {
		states.erase(object);
		return -1;
	}

	return 0;
}

Key PeerObject::insert_new(PyObject *object, bool dirty)
{
	Key key = next_key;

	if (insert(object, key, dirty) < 0)
		return -1;

	next_key++;

	return key;
}

void PeerObject::clear(PyObject *object)
{
	auto i = states.find(object);
	if (i != states.end() && i->second.dirty)
		i->second.dirty = false;
}

std::pair<Key, bool> PeerObject::insert_or_clear(PyObject *object)
{
	auto i = states.find(object);
	if (i != states.end()) {
		bool was_dirty = false;
		if (i->second.dirty) {
			i->second.dirty = false;
			was_dirty = true;
		}
		return std::make_pair(i->second.key, was_dirty);
	} else {
		Key key = insert_new(object, false);
		return std::make_pair(key, true);
	}
}

Key PeerObject::key(PyObject *object)
{
	auto i = states.find(object);
	if (i != states.end()) {
		return i->second.key;
	} else {
		return insert_new(object, true);
	}
}

PyObject *PeerObject::object(Key key)
{
	PyObject *object = NULL;

	auto i = objects.find(key);
	if (i != objects.end()) {
		object = i->second;

		if (object->ob_refcnt <= 0) {
			fprintf(stderr, "tap peer: %s object %p with invalid reference count %ld during lookup\n", object->ob_type->tp_name, object, object->ob_refcnt);
			object = NULL;
		}
	}

	return object;
}

void PeerObject::object_freed(void *ptr)
{
	PyObject *object = reinterpret_cast<PyObject *> (ptr);

	auto i = states.find(object);
	if (i != states.end()) {
		fprintf(stderr, "tap peer: %s object %p freed\n", 	object->ob_type->tp_name, object);

		if (object->ob_refcnt != 0)
			fprintf(stderr, "tap peer: %s object %p with unexpected reference count %ld when freed\n", object->ob_type->tp_name, object, object->ob_refcnt);

		objects.erase(i->second.key);
		states.erase(i);
	}
}

extern "C" {
	static PyObject *peer_new(PyTypeObject *type, PyObject *args, PyObject *kwargs)
	{
		PyObject *peer = type->tp_alloc(type, 0);
		if (peer) {
			try {
				new (peer) PeerObject;
			} catch (std::bad_alloc) {
				type->tp_free(peer);
				peer = NULL;
			}
		}

		return peer;
	}

	static void peer_dealloc(PyObject *peer)
	{
		reinterpret_cast<PeerObject *> (peer)->~PeerObject();
		Py_TYPE(peer)->tp_free(peer);
	}
}

int peer_type_init()
{
	return PyType_Ready(&peer_type);
}

PyTypeObject peer_type = {
	PyVarObject_HEAD_INIT(NULL, 0)
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
	NULL,                           /* tp_doc */
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

} // namespace tap