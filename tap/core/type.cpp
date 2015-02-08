#include "core.hpp"
#include "portable.hpp"

namespace tap {

const TypeHandler *type_handler_for_object(PyObject *object) noexcept
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
	if (type == &PyFrame_Type) return &frame_type_handler;

	return &opaque_type_handler;
}

const TypeHandler *type_handler_for_id(int32_t type_id) noexcept
{
	if (type_id >= 0 && type_id < TYPE_ID_COUNT) {
		switch (TypeId(type_id)) {
		case OPAQUE_TYPE_ID: return &opaque_type_handler;
		case NONE_TYPE_ID: return &none_type_handler;
		case TYPE_TYPE_ID: return &type_type_handler;
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
		case FRAME_TYPE_ID: return &frame_type_handler;

		case TYPE_ID_COUNT: break;
		}
	}

	return nullptr;
}

struct Portable {
	int32_t type_id;
	char opaque_name[];
} TAP_PACKED;

static int type_traverse(PyObject *object, visitproc visit, void *arg) noexcept
{
	return 0;
}

static Py_ssize_t type_marshaled_size(PyObject *object) noexcept
{
	auto handler = type_handler_for_object(object);
	Py_ssize_t size = sizeof (Portable);

	if (handler->type_id == OPAQUE_TYPE_ID)
		size += strlen(Py_TYPE(object)->tp_name);

	return size;
}

static int type_marshal(PyObject *object, void *buf, Py_ssize_t size, PeerObject &peer) noexcept
{
	auto handler = type_handler_for_object(object);
	auto portable = reinterpret_cast<Portable *> (buf);

	portable->type_id = port(handler->type_id);

	if (handler->type_id == OPAQUE_TYPE_ID) {
		const char *name = Py_TYPE(object)->tp_name;
		memcpy(portable->opaque_name, name, strlen(name));
	}

	return 0;
}

static PyObject *type_unmarshal_alloc(const void *data, Py_ssize_t size, PeerObject &peer) noexcept
{
	if (size < Py_ssize_t(sizeof (Portable)))
		return nullptr;

	const Portable *portable = reinterpret_cast<const Portable *> (data);
	auto type_id = port(portable->type_id);
	auto opaque_name_len = size - sizeof (Portable);

	if (type_id == OPAQUE_TYPE_ID) {
		if (opaque_name_len == 0)
			return nullptr;
	} else {
		if (opaque_name_len > 0)
			return nullptr;
	}

	PyTypeObject *type = nullptr;

	if (type_id >= 0 && type_id < TYPE_ID_COUNT) {
		switch (TypeId(type_id)) {
		case OPAQUE_TYPE_ID:
			if (!unicode_verify_utf8(portable->opaque_name, opaque_name_len)) {
				fprintf(stderr, "tap type unmarshal: bad UTF-8 in opaque name\n");
				return nullptr;
			}

			try {
				const std::string name(portable->opaque_name, opaque_name_len);
				type = opaque_type_for_name(name);
			} catch (...) {
				return nullptr;
			}

			break;

		case NONE_TYPE_ID: type = Py_TYPE(Py_None); break;
		case TYPE_TYPE_ID: type = &PyType_Type; break;
		case BOOL_TYPE_ID: type = &PyBool_Type; break;
		case LONG_TYPE_ID: type = &PyLong_Type; break;
		case TUPLE_TYPE_ID: type = &PyTuple_Type; break;
		case LIST_TYPE_ID: type = &PyList_Type; break;
		case DICT_TYPE_ID: type = &PyDict_Type; break;
		case BYTES_TYPE_ID: type = &PyBytes_Type; break;
		case UNICODE_TYPE_ID: type = &PyUnicode_Type; break;
		case CODE_TYPE_ID: type = &PyCode_Type; break;
		case FUNCTION_TYPE_ID: type = &PyFunction_Type; break;
		case MODULE_TYPE_ID: type = &PyModule_Type; break;
		case BUILTIN_TYPE_ID: type = &PyCFunction_Type; break;
		case FRAME_TYPE_ID: type = &PyFrame_Type; break;

		case TYPE_ID_COUNT: break;
		}
	}

	return reinterpret_cast<PyObject *> (type);
}

static int type_unmarshal_init(PyObject *object, const void *data, Py_ssize_t size, PeerObject &peer) noexcept
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

} // namespace tap
