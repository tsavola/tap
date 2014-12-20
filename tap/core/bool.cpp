#include "core.hpp"

namespace tap {

static int bool_traverse(PyObject *object, visitproc visit, void *arg) noexcept
{
	return 0;
}

static Py_ssize_t bool_marshaled_size(PyObject *object) noexcept
{
	return sizeof (uint8_t);
}

static int bool_marshal(PyObject *object, void *buf, Py_ssize_t size, PeerObject &peer) noexcept
{
	*reinterpret_cast<uint8_t *> (buf) = port(object != Py_False);
	return 0;
}

static PyObject *bool_unmarshal_alloc(const void *data, Py_ssize_t size, PeerObject &peer) noexcept
{
	if (size != sizeof (uint8_t))
		return nullptr;

	return PyBool_FromLong(port(*reinterpret_cast<const uint8_t *> (data)));
}

static int bool_unmarshal_init(PyObject *object, const void *data, Py_ssize_t size, PeerObject &peer) noexcept
{
	return 0;
}

const TypeHandler bool_type_handler = {
	BOOL_TYPE_ID,
	bool_traverse,
	bool_marshaled_size,
	bool_marshal,
	bool_unmarshal_alloc,
	bool_unmarshal_init,
};

} // namespace tap
