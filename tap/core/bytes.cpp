#include "core.hpp"

#include <cstring>

namespace tap {

static int bytes_traverse(PyObject *object, visitproc visit, void *arg)
{
	return 0;
}

static Py_ssize_t bytes_marshaled_size(PyObject *object)
{
	return PyBytes_GET_SIZE(object);
}

static int bytes_marshal(PyObject *object, void *buf, Py_ssize_t size, PeerObject &peer)
{
	memcpy(buf, PyBytes_AS_STRING(object), size);
	return 0;
}

static PyObject *bytes_unmarshal_alloc(const void *data, Py_ssize_t size, PeerObject &peer)
{
	return PyBytes_FromStringAndSize(NULL, size);
}

static int bytes_unmarshal_init(PyObject *object, const void *data, Py_ssize_t size, PeerObject &peer)
{
	memcpy(PyBytes_AS_STRING(object), data, size);
	return 0;
}

const TypeHandler bytes_type_handler = {
	BYTES_TYPE_ID,
	bytes_traverse,
	bytes_marshaled_size,
	bytes_marshal,
	bytes_unmarshal_alloc,
	bytes_unmarshal_init,
};

} // namespace tap
