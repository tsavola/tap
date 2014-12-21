#include "core.hpp"
#include "portable.hpp"

namespace tap {

static int tuple_traverse(PyObject *object, visitproc visit, void *arg) noexcept
{
	for (Py_ssize_t i = 0; i < PyTuple_GET_SIZE(object); ++i)
		Py_VISIT(PyTuple_GET_ITEM(object, i));

	return 0;
}

static Py_ssize_t tuple_marshaled_size(PyObject *object) noexcept
{
	return sizeof (Key) * PyTuple_GET_SIZE(object);
}

static int tuple_marshal(PyObject *object, void *buf, Py_ssize_t size, PeerObject &peer) noexcept
{
	Key *portable = reinterpret_cast<Key *> (buf);

	for (Py_ssize_t i = 0; i < PyTuple_GET_SIZE(object); ++i) {
		Key remote_key = peer.key_for_remote(PyTuple_GET_ITEM(object, i));
		if (remote_key < 0)
			return -1;

		portable[i] = port(remote_key);
	}

	return 0;
}

static PyObject *tuple_unmarshal_alloc(const void *data, Py_ssize_t size, PeerObject &peer) noexcept
{
	if (size % sizeof (Key))
		return nullptr;

	return PyTuple_New(size / sizeof (Key));
}

static int tuple_unmarshal_init(PyObject *object, const void *data, Py_ssize_t size, PeerObject &peer) noexcept
{
	const Key *portable = reinterpret_cast<const Key *> (data);

	for (Py_ssize_t i = 0; i < PyTuple_GET_SIZE(object); ++i) {
		PyObject *item = peer.object(port(portable[i]));
		if (item == nullptr)
			return -1;

		Py_INCREF(item);
		PyTuple_SET_ITEM(object, i, item);
	}

	return 0;
}

const TypeHandler tuple_type_handler = {
	TUPLE_TYPE_ID,
	tuple_traverse,
	tuple_marshaled_size,
	tuple_marshal,
	tuple_unmarshal_alloc,
	tuple_unmarshal_init,
};

} // namespace tap
