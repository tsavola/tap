#include "core.hpp"

namespace tap {

struct Portable {
	int32_t type_id;
} TAP_PACKED;

static int type_traverse(PyObject *object, visitproc visit, void *arg)
{
	return 0;
}

static Py_ssize_t type_marshaled_size(PyObject *object)
{
	return sizeof (Portable);
}

static int type_marshal(PyObject *object, void *buf, Py_ssize_t size, PeerObject &peer)
{
	const TypeHandler *handler = type_handler_for_object(object);
	Portable *portable = reinterpret_cast<Portable *> (buf);
	portable->type_id = port(handler->type_id);
	return 0;
}

static PyObject *type_unmarshal_alloc(const void *data, Py_ssize_t size, PeerObject &peer)
{
	if (size != sizeof (Portable))
		return NULL;

	const Portable *portable = reinterpret_cast<const Portable *> (data);
	return reinterpret_cast<PyObject *> (type_object_for_id(port(portable->type_id)));
}

static int type_unmarshal_init(PyObject *object, const void *data, Py_ssize_t size, PeerObject &peer)
{
	return 0;
}

const TypeHandler type_type_handler = {
	TYPE_TYPE_ID,
	type_traverse,
	type_marshaled_size,
	type_marshal,
	type_unmarshal_alloc,
	type_unmarshal_init,
};

const TypeHandler *type_handler_for_object(PyObject *object)
{
	PyTypeObject *type = object->ob_type;

	if (object == Py_None) return &none_type_handler;
	if (type == &PyType_Type) return &type_type_handler;
	if (type == &PyBool_Type) return &bool_type_handler;
	if (type == &PyLong_Type) return &long_type_handler;
	if (type == &PyTuple_Type) return &tuple_type_handler;
	if (type == &PyList_Type) return &list_type_handler;
	if (type == &PyDict_Type) return &dict_type_handler;
	if (type == &PyBytes_Type) return &bytes_type_handler;
	if (type == &PyUnicode_Type) return &unicode_type_handler;
	if (type == &PyCode_Type) return &code_type_handler;
	if (type == &PyFunction_Type) return &function_type_handler;
	if (type == &PyModule_Type) return &module_type_handler;
	if (type == &PyCFunction_Type && builtin_check(object)) return &builtin_type_handler;

	//fprintf(stderr, "type_handler_for_object: %s -> opaque\n", type->tp_name);

	return &opaque_type_handler;
}

const TypeHandler *type_handler_for_id(int32_t type_id)
{
	if (type_id >= 0 && type_id < TYPE_ID_COUNT) {
		switch (TypeId(type_id)) {
		case NONE_TYPE_ID: return &none_type_handler;
		case TYPE_TYPE_ID: return &type_type_handler;
		case OPAQUE_TYPE_ID: return &opaque_type_handler;
		case BOOL_TYPE_ID: return &bool_type_handler;
		case LONG_TYPE_ID: return &long_type_handler;
		case TUPLE_TYPE_ID: return &tuple_type_handler;
		case LIST_TYPE_ID: return &list_type_handler;
		case DICT_TYPE_ID: return &dict_type_handler;
		case BYTES_TYPE_ID: return &bytes_type_handler;
		case UNICODE_TYPE_ID: return &unicode_type_handler;
		case CODE_TYPE_ID: return &code_type_handler;
		case FUNCTION_TYPE_ID: return &function_type_handler;
		case MODULE_TYPE_ID: return &module_type_handler;
		case BUILTIN_TYPE_ID: return &builtin_type_handler;
		case TYPE_ID_COUNT: break;
		}
	}

	return NULL;
}

PyTypeObject *type_object_for_id(int32_t type_id)
{
	if (type_id >= 0 && type_id < TYPE_ID_COUNT) {
		switch (TypeId(type_id)) {
		case NONE_TYPE_ID: return reinterpret_cast<PyTypeObject *> (Py_None->ob_type);
		case TYPE_TYPE_ID: return &PyType_Type;
		case OPAQUE_TYPE_ID: return &opaque_type;
		case BOOL_TYPE_ID: return &PyBool_Type;
		case LONG_TYPE_ID: return &PyLong_Type;
		case TUPLE_TYPE_ID: return &PyTuple_Type;
		case LIST_TYPE_ID: return &PyList_Type;
		case DICT_TYPE_ID: return &PyDict_Type;
		case BYTES_TYPE_ID: return &PyBytes_Type;
		case UNICODE_TYPE_ID: return &PyUnicode_Type;
		case CODE_TYPE_ID: return &PyCode_Type;
		case FUNCTION_TYPE_ID: return &PyFunction_Type;
		case MODULE_TYPE_ID: return &PyModule_Type;
		case BUILTIN_TYPE_ID: return &PyCFunction_Type;
		case TYPE_ID_COUNT: break;
		}
	}

	return NULL;
}

} // namespace tap
