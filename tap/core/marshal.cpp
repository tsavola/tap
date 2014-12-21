#include "core.hpp"
#include "portable.hpp"

#include <cstdio>
#include <stdexcept>
#include <unordered_set>

namespace tap {

enum SectionId {
	OBJECT_SECTION_ID,
	FREE_SECTION_ID,
};

struct SectionHeader {
	int32_t size;
	int32_t id;
} TAP_PACKED;

struct ObjectSectionHeader {
	SectionHeader section;
	Key root_key;
} TAP_PACKED;

struct ObjectHeader {
	int32_t size;
	int32_t type_id;
	Key key;
} TAP_PACKED;

static Py_ssize_t extend_and_get_offset(PyObject *bytearray, Py_ssize_t increment) noexcept
{
	Py_ssize_t offset = PyByteArray_GET_SIZE(bytearray);

	if (PyByteArray_Resize(bytearray, offset + increment) < 0)
		return -1;

	return offset;
}

template <typename T>
static T *get_buffer_at(PyObject *bytearray, Py_buffer *buffer, Py_ssize_t offset) noexcept
{
	if (PyObject_GetBuffer(bytearray, buffer, PyBUF_WRITABLE) < 0)
		return nullptr;

	return reinterpret_cast<T *> (reinterpret_cast<char *> (buffer->buf) + offset);
}

template <typename T>
static T *extend_and_get_buffer(PyObject *bytearray, Py_ssize_t increment, Py_buffer *buffer) noexcept
{
	Py_ssize_t offset = extend_and_get_offset(bytearray, increment);
	if (offset < 0)
		return nullptr;

	return get_buffer_at<T>(bytearray, buffer, offset);
}

struct ObjectMarshaler {
	PeerObject &peer;
	PyObject *bytearray;
	std::unordered_set<PyObject *> seen;

	ObjectMarshaler(PeerObject &peer, PyObject *bytearray):
		peer(peer),
		bytearray(bytearray)
	{
	}
};

static int marshal_visit_objects(PyObject *object, void *arg) noexcept
{
	ObjectMarshaler &marshaler = *reinterpret_cast<ObjectMarshaler *> (arg);

	if (marshaler.seen.find(object) != marshaler.seen.end())
		return 0;

	try {
		marshaler.seen.insert(object);
	} catch (...) {
		return -1;
	}

	auto pair = marshaler.peer.insert_or_clear_for_remote(object);
	Key remote_key = pair.first;
	bool object_changed = pair.second;

	if (remote_key < 0)
		return -1;

	const TypeHandler *handler = type_handler_for_object(object);

	if (object_changed) {
		Py_ssize_t size = handler->marshaled_size(object);
		if (size < 0)
			return -1;

		auto extent_size = sizeof (ObjectHeader) + size;
		if (extent_size > 0x7fffffff)
			return -1;

		Py_buffer buffer;
		auto header = extend_and_get_buffer<ObjectHeader>(marshaler.bytearray, extent_size, &buffer);
		if (header == nullptr)
			return -1;

		header->size = port(int32_t(extent_size));
		header->type_id = port(handler->type_id);
		header->key = port(remote_key);

		int ret = handler->marshal(object, header + 1, size, marshaler.peer);

		PyBuffer_Release(&buffer);

		if (ret < 0)
			return -1;
	}

	return handler->traverse(object, marshal_visit_objects, arg);
}

static int marshal_objects(PeerObject &peer, PyObject *bytearray, PyObject *object) noexcept
{
	Py_ssize_t offset = extend_and_get_offset(bytearray, sizeof (ObjectSectionHeader));
	if (offset < 0)
		return -1;

	try {
		ObjectMarshaler marshaler(peer, bytearray);

		if (marshal_visit_objects(object, &marshaler) < 0)
			return -1;
	} catch (...) {
		return -1;
	}

	auto section_size = PyByteArray_GET_SIZE(bytearray) - offset;
	if (section_size > 0x7fffffff)
		return -1;

	Key remote_root_key = peer.key_for_remote(object);
	if (remote_root_key < 0)
		return -1;

	Py_buffer buffer;
	auto header = get_buffer_at<ObjectSectionHeader>(bytearray, &buffer, offset);
	if (header == nullptr)
		return -1;

	header->section.size = port(int32_t(section_size));
	header->section.id = port(int32_t(OBJECT_SECTION_ID));
	header->root_key = port(remote_root_key);

	PyBuffer_Release(&buffer);

	return 0;
}

static int marshal_freed(PeerObject &peer, PyObject *bytearray) noexcept
{
	auto size = sizeof (SectionHeader) + peer.freed.size() * sizeof (Key);
	if (size > 0x7fffffff)
		return -1;

	Py_buffer buffer;
	auto header = extend_and_get_buffer<SectionHeader>(bytearray, size, &buffer);
	if (header == nullptr)
		return -1;

	header->size = port(int32_t(size));
	header->id = port(int32_t(FREE_SECTION_ID));

	Key *data = reinterpret_cast<Key *> (header + 1);

	for (Key key: peer.freed)
		*data++ = port(key);

	PyBuffer_Release(&buffer);

	peer.freed.clear();

	return 0;
}

int marshal(PeerObject &peer, PyObject *bytearray, PyObject *object) noexcept
{
	Py_ssize_t orig_size = PyByteArray_GET_SIZE(bytearray);

	if (marshal_freed(peer, bytearray) < 0)
		goto fail;

	if (object && marshal_objects(peer, bytearray, object) < 0)
		goto fail;

	return 0;

fail:
	PyByteArray_Resize(bytearray, orig_size);
	return -1;
}

struct ObjectUnmarshaler {
	std::unordered_set<PyObject *> pending;

	~ObjectUnmarshaler()
	{
		for (PyObject *object: pending)
			Py_DECREF(object);
	}

	int alloc(PeerObject &peer, const void *data, Py_ssize_t size) noexcept
	{
		while (size >= Py_ssize_t(sizeof (ObjectHeader))) {
			auto header = reinterpret_cast<const ObjectHeader *> (data);
			int32_t item_size = port(header->size);
			int32_t item_type_id = port(header->type_id);
			Key item_key = port(header->key);

			if (item_size < Py_ssize_t(sizeof (ObjectHeader)) || item_size > size) {
				fprintf(stderr, "tap unmarshal: header size out of bounds\n");
				return -1;
			}

			const TypeHandler *handler = type_handler_for_id(item_type_id);
			if (handler == nullptr) {
				fprintf(stderr, "tap unmarshal: object type id is unknown\n");
				return -1;
			}

			Py_ssize_t marshal_size = item_size - sizeof (ObjectHeader);
			const void *marshal_data = header + 1;

			PyObject *object = peer.object(item_key);
			if (object) {
				if (handler->unmarshal_update == nullptr) {
					fprintf(stderr, "tap unmarshal: update of immutable object\n");
					return -1;
				}

				peer.clear(object);
			} else {
				object = handler->unmarshal_alloc(marshal_data, marshal_size, peer);
				if (object == nullptr) {
					fprintf(stderr, "tap unmarshal: allocation failed (type_id=%d)\n", item_type_id);
					return -1;
				}

				try {
					pending.insert(object);
				} catch (...) {
					return -1;
				}

				peer.insert(object, item_key);
			}

			data = reinterpret_cast<const char *> (data) + item_size;
			size -= item_size;
		}

		if (size > 0) {
			fprintf(stderr, "tap unmarshal: trailing garbage or truncated data in object section\n");
			return -1;
		}

		return 0;
	}

	int init(PeerObject &peer, const void *data, Py_ssize_t size, bool init_dicts) noexcept
	{
		while (size >= Py_ssize_t(sizeof (ObjectHeader))) {
			auto header = reinterpret_cast<const ObjectHeader *> (data);
			int32_t item_size = port(header->size);
			int32_t item_type_id = port(header->type_id);
			Key item_key = port(header->key);

			const TypeHandler *handler = type_handler_for_id(item_type_id);

			Py_ssize_t marshal_size = item_size - sizeof (ObjectHeader);
			const void *marshal_data = header + 1;

			PyObject *object = peer.object(item_key);

			if (!PyDict_Check(object) == !init_dicts) {
				int ret;

				if (pending.find(object) != pending.end())
					ret = handler->unmarshal_init(object, marshal_data, marshal_size, peer);
				else
					ret = handler->unmarshal_update(object, marshal_data, marshal_size, peer);

				if (ret < 0) {
					fprintf(stderr, "tap unmarshal: type handler failed to unmarshal: %s\n", object->ob_type->tp_name);
					return -1;
				}
			}

			data = reinterpret_cast<const char *> (data) + item_size;
			size -= item_size;
		}

		return 0;
	}

	void finalize(PeerObject &peer) noexcept
	{
		peer.set_references(pending);
		pending.clear();
	}
};

static PyObject *unmarshal_objects(PeerObject &peer, const void *data, Py_ssize_t size) noexcept
{
	if (size < Py_ssize_t(sizeof (ObjectSectionHeader))) {
		fprintf(stderr, "tap unmarshal: not enough data in object section\n");
		return nullptr;
	}

	auto header = reinterpret_cast<const ObjectSectionHeader *> (data);
	Key root_key = port(header->root_key);

	data = reinterpret_cast<const char *> (data) + sizeof (ObjectSectionHeader);
	size -= sizeof (ObjectSectionHeader);

	try {
		ObjectUnmarshaler unmarshaler;

		if (unmarshaler.alloc(peer, data, size) < 0)
			return nullptr;

		if (unmarshaler.init(peer, data, size, false) < 0 || unmarshaler.init(peer, data, size, true) < 0)
			return nullptr;

		unmarshaler.finalize(peer);
	} catch (...) {
		return nullptr;
	}

	auto root = peer.object(root_key);
	Py_XINCREF(root);

	return root;
}

static int unmarshal_freed(PeerObject &peer, const void *data, Py_ssize_t size) noexcept
{
	data = reinterpret_cast<const char *> (data) + sizeof (SectionHeader);
	size -= sizeof (SectionHeader);

	if ((size % sizeof (Key)) != 0) {
		fprintf(stderr, "tap unmarshal: trailing garbage or truncated data in freed section\n");
		return -1;
	}

	const Key *portable = reinterpret_cast<const Key *> (data);
	int count = size / sizeof (Key);

	for (int i = 0; i < count; i++) {
		Key key = port(portable[i]);

		fprintf(stderr, "tap unmarshal: object with key %ld dereference\n", key);

		peer.dereference(key);
	}

	return 0;
}

PyObject *unmarshal(PeerObject &peer, const void *data, Py_ssize_t size) noexcept
{
	PyObject *root = nullptr;

	while (size >= Py_ssize_t(sizeof (SectionHeader))) {
		auto header = reinterpret_cast<const SectionHeader *> (data);
		auto section_size = port(header->size);
		auto section_id = port(header->id);

		if (section_size < Py_ssize_t(sizeof (SectionHeader)) || section_size > size) {
			fprintf(stderr, "tap unmarshal: section size out of bounds\n");
			goto fail;
		}

		switch (SectionId(section_id)) {
		case OBJECT_SECTION_ID:
			Py_XDECREF(root);

			root = unmarshal_objects(peer, data, section_size);
			if (root == nullptr)
				goto fail;

			break;

		case FREE_SECTION_ID:
			if (unmarshal_freed(peer, data, section_size) < 0)
				goto fail;

			if (root == nullptr) {
				root = Py_None;
				Py_INCREF(root);
			}

			break;

		default:
			fprintf(stderr, "tap unmarshal: unknown section id: %d\n", section_id);
			goto fail;
		}

		data = reinterpret_cast<const char *> (data) + section_size;
		size -= section_size;
	}

	if (size > 0) {
		fprintf(stderr, "tap unmarshal: trailing garbage or truncated data after sections\n");
		goto fail;
	}

	return root;

fail:
	Py_XDECREF(root);
	return nullptr;
}

} // namespace tap
