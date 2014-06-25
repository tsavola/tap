#include "core.hpp"

namespace tap {

static int none_traverse(PyObject *object, visitproc visit, void *arg)
{
	return 0;
}

static Py_ssize_t none_marshaled_size(PyObject *object)
{
	return 0;
}

static int none_marshal(PyObject *object, void *buf, Py_ssize_t size, PeerObject &peer)
{
	return 0;
}

static PyObject *none_unmarshal_alloc(const void *data, Py_ssize_t size, PeerObject &peer)
{
	Py_RETURN_NONE;
}

static int none_unmarshal_init(PyObject *object, const void *data, Py_ssize_t size, PeerObject &peer)
{
	return 0;
}

const TypeHandler none_type_handler = {
	NONE_TYPE_ID,
	none_traverse,
	none_marshaled_size,
	none_marshal,
	none_unmarshal_alloc,
	none_unmarshal_init,
};

} // namespace tap
