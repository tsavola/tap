#include "core.hpp"

namespace tap {

struct OpaqueObject {
	PyObject_HEAD
};

static PyObject *opaque_new(PyTypeObject *type, PyObject *args, PyObject *kwargs) noexcept
{
	return type->tp_alloc(type, 0);
}

static void opaque_dealloc(PyObject *opaque) noexcept
{
	Py_TYPE(opaque)->tp_free(opaque);
}

int opaque_type_init() noexcept
{
	return PyType_Ready(&opaque_type);
}

PyTypeObject opaque_type = {
	PyVarObject_HEAD_INIT(nullptr, 0)
	"tap.core.Opaque",              /* tp_name */
	sizeof (OpaqueObject),          /* tp_basicsize */
	0,                              /* tp_itemsize */
	opaque_dealloc,                 /* tp_dealloc */
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
	opaque_new,                     /* tp_new */
};

static int opaque_traverse(PyObject *object, visitproc visit, void *arg) noexcept
{
	return 0;
}

static Py_ssize_t opaque_marshaled_size(PyObject *object) noexcept
{
	return 0;
}

static int opaque_marshal(PyObject *object, void *buf, Py_ssize_t size, PeerObject &peer) noexcept
{
	return 0;
}

static PyObject *opaque_unmarshal_alloc(const void *data, Py_ssize_t size, PeerObject &peer) noexcept
{
	return opaque_type.tp_alloc(&opaque_type, 0);
}

static int opaque_unmarshal_init(PyObject *object, const void *data, Py_ssize_t size, PeerObject &peer) noexcept
{
	return 0;
}

const TypeHandler opaque_type_handler = {
	OPAQUE_TYPE_ID,
	opaque_traverse,
	opaque_marshaled_size,
	opaque_marshal,
	opaque_unmarshal_alloc,
	opaque_unmarshal_init,
};

} // namespace tap
