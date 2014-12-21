#include "core.hpp"
#include "portable.hpp"

#include <cstdio>
#include <stdexcept>
#include <unordered_set>

namespace tap {

struct MarshalHeader {
	Key root_key;
} TAP_PACKED;

struct ObjectHeader {
	int32_t size;
	int32_t type_id;
	Key key;
} TAP_PACKED;

struct Marshaler {
	PeerObject &peer;
	PyObject *bytearray;
	std::unordered_set<PyObject *> seen;

	Marshaler(PeerObject &peer, PyObject *bytearray) noexcept:
		peer(peer),
		bytearray(bytearray)
	{
	}
};

static int marshal_visit(PyObject *object, void *arg) noexcept
{
	Marshaler &marshaler = *reinterpret_cast<Marshaler *> (arg);

	if (marshaler.seen.find(object) != marshaler.seen.end())
		return 0;

	try {
		marshaler.seen.insert(object);
	} catch (...) {
		return -1;
	}

	auto pair = marshaler.peer.insert_or_clear(object);

	Key key = pair.first;
	if (key < 0)
		return -1;

	const TypeHandler *handler = type_handler_for_object(object);

	bool needed = pair.second;
	if (needed) {
		Py_ssize_t size = handler->marshaled_size(object);
		if (size < 0)
			return -1;

		Py_ssize_t extent_size = sizeof (ObjectHeader) + size;
		if (extent_size > 0x7fffffff)
			return -1;

		Py_ssize_t offset = PyByteArray_GET_SIZE(marshaler.bytearray);

		if (PyByteArray_Resize(marshaler.bytearray, offset + extent_size) < 0)
			return -1;

		Py_buffer buffer;

		if (PyObject_GetBuffer(marshaler.bytearray, &buffer, PyBUF_WRITABLE) < 0)
			return -1;

		auto header = reinterpret_cast<ObjectHeader *> (reinterpret_cast<char *> (buffer.buf) + offset);

		header->size = port(extent_size);
		header->type_id = port(handler->type_id);
		header->key = port(key);

		int ret = handler->marshal(object, header + 1, size, marshaler.peer);

		PyBuffer_Release(&buffer);

		if (ret < 0)
			return -1;
	}

	return handler->traverse(object, marshal_visit, arg);
}

int marshal(PeerObject &peer, PyObject *bytearray, PyObject *object) noexcept
{
	Py_ssize_t orig_size = PyByteArray_GET_SIZE(bytearray);
	Marshaler marshaler(peer, bytearray);
	Key root_key;
	Py_buffer buffer;
	MarshalHeader *header;

	if (PyByteArray_Resize(bytearray, orig_size + sizeof (MarshalHeader)) < 0)
		goto fail;

	if (marshal_visit(object, &marshaler) < 0)
		goto fail;

	root_key = peer.key(object);
	if (root_key < 0)
		goto fail;

	if (PyObject_GetBuffer(bytearray, &buffer, PyBUF_WRITABLE) < 0)
		goto fail;

	header = reinterpret_cast<MarshalHeader *> (reinterpret_cast<char *> (buffer.buf) + orig_size);
	header->root_key = port(root_key);

	PyBuffer_Release(&buffer);

	return 0;

fail:
	PyByteArray_Resize(bytearray, orig_size);
	return -1;
}

struct Unmarshaler {
	std::unordered_set<PyObject *> created;

	~Unmarshaler()
	{
		for (PyObject *object: created)
			Py_DECREF(object);
	}

	int alloc(PeerObject &peer, const void *data, Py_ssize_t size) noexcept
	{
		while (size >= Py_ssize_t(sizeof (ObjectHeader))) {
			const auto header = reinterpret_cast<const ObjectHeader *> (data);
			int32_t item_size = port(header->size);
			int32_t item_type_id = port(header->type_id);
			Key item_key = port(header->key);

			if (item_size > size) {
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
					created.insert(object);
				} catch (...) {
					return -1;
				}

				peer.insert(object, item_key);
			}

			data = reinterpret_cast<const char *> (data) + item_size;
			size -= item_size;
		}

		if (size > 0) {
			fprintf(stderr, "tap unmarshal: trailing garbage or truncated data\n");
			return -1;
		}

		return 0;
	}

	int init(PeerObject &peer, const void *data, Py_ssize_t size, bool init_dicts) noexcept
	{
		while (size >= Py_ssize_t(sizeof (ObjectHeader))) {
			const auto header = reinterpret_cast<const ObjectHeader *> (data);
			int32_t item_size = port(header->size);
			int32_t item_type_id = port(header->type_id);
			Key item_key = port(header->key);

			const TypeHandler *handler = type_handler_for_id(item_type_id);

			Py_ssize_t marshal_size = item_size - sizeof (ObjectHeader);
			const void *marshal_data = header + 1;

			PyObject *object = peer.object(item_key);

			if (!PyDict_Check(object) == !init_dicts) {
				int ret;

				if (created.find(object) != created.end())
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
};

PyObject *unmarshal(PeerObject &peer, const void *data, Py_ssize_t size) noexcept
{
	if (size < Py_ssize_t(sizeof (MarshalHeader))) {
		fprintf(stderr, "tap unmarshal: not enough data\n");
		return nullptr;
	}

	const auto header = reinterpret_cast<const MarshalHeader *> (data);
	Key root_key = port(header->root_key);

	data = reinterpret_cast<const char *> (data) + sizeof (MarshalHeader);
	size -= sizeof (MarshalHeader);

	Unmarshaler unmarshaler;

	if (unmarshaler.alloc(peer, data, size) < 0)
		return nullptr;

	if (unmarshaler.init(peer, data, size, false) < 0 || unmarshaler.init(peer, data, size, true) < 0)
		return nullptr;

	auto root = peer.object(root_key);
	Py_XINCREF(root);
	return root;
}

} // namespace tap
