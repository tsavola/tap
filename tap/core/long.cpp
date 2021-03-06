#include "core.hpp"
#include "portable.hpp"

namespace tap {

static int long_traverse(PyObject *object, visitproc visit, void *arg) noexcept
{
	return 0;
}

static Py_ssize_t long_marshaled_size(PyObject *object) noexcept
{
	return sizeof (int64_t);
}

static int long_marshal(PyObject *object, void *buf, Py_ssize_t size, PeerObject &peer) noexcept
{
	int overflow;
	int64_t value = PyLong_AsLongLongAndOverflow(object, &overflow);
	if (overflow != 0)
		return -1;

	*reinterpret_cast<int64_t *> (buf) = port(value);
	return 0;
}

static PyObject *long_unmarshal_alloc(const void *data, Py_ssize_t size, PeerObject &peer) noexcept
{
	if (size != sizeof (int64_t))
		return nullptr;

	return PyLong_FromLongLong(port(*reinterpret_cast<const int64_t *> (data)));
}

static int long_unmarshal_init(PyObject *object, const void *data, Py_ssize_t size, PeerObject &peer) noexcept
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
