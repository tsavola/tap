#include "core.hpp"

namespace tap {

static int long_traverse(PyObject *object, visitproc visit, void *arg)
{
	return 0;
}

static Py_ssize_t long_marshaled_size(PyObject *object)
{
	return sizeof (int64_t);
}

static int long_marshal(PyObject *object, void *buf, Py_ssize_t size, PeerObject &peer)
{
	*reinterpret_cast<int64_t *> (buf) = port(PyLong_AsLongLong(object));
	return 0;
}

static PyObject *long_unmarshal_alloc(const void *data, Py_ssize_t size, PeerObject &peer)
{
	if (size != sizeof (int64_t))
		return NULL;

	return PyLong_FromLongLong(port(*reinterpret_cast<const int64_t *> (data)));
}

static int long_unmarshal_init(PyObject *object, const void *data, Py_ssize_t size, PeerObject &peer)
{
	return 0;
}

const TypeHandler long_type_handler = {
	LONG_TYPE_ID,
	long_traverse,
	long_marshaled_size,
	long_marshal,
	long_unmarshal_alloc,
	long_unmarshal_init,
};

} // namespace tap
