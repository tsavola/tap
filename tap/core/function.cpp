#include "core.hpp"

namespace tap {

struct Portable {
	Key code;
	Key globals;
	Key defaults;
	Key kwdefaults;
	Key closure;
	Key doc;
	Key name;
	Key dict;
	Key weakreflist;
	Key module;
	Key annotations;
	Key qualname;
} TAP_PACKED;

static int function_traverse(PyObject *object, visitproc visit, void *arg)
{
	const PyFunctionObject *self = reinterpret_cast<PyFunctionObject *> (object);

	Py_VISIT(self->func_code);
	Py_VISIT(self->func_globals);
	Py_VISIT(self->func_defaults);
	Py_VISIT(self->func_kwdefaults);
	Py_VISIT(self->func_closure);
	Py_VISIT(self->func_doc);
	Py_VISIT(self->func_name);
	Py_VISIT(self->func_dict);
	Py_VISIT(self->func_weakreflist);
	Py_VISIT(self->func_module);
	Py_VISIT(self->func_annotations);
	Py_VISIT(self->func_qualname);

	return 0;
}

static Py_ssize_t function_marshaled_size(PyObject *object)
{
	return sizeof (Portable);
}

#define TAP_FUNCTION_MARSHAL_OBJECT(NAME) \
	do { \
		Key key = -1; \
		if (function->func_##NAME) { \
			key = peer.key(function->func_##NAME); \
			if (key < 0) \
				return -1; \
		} \
		portable->NAME = port(key); \
	} while (0)

static int function_marshal(PyObject *object, void *buf, Py_ssize_t size, PeerObject &peer)
{
	const PyFunctionObject *function = reinterpret_cast<PyFunctionObject *> (object);
	Portable *portable = reinterpret_cast<Portable *> (buf);

	TAP_FUNCTION_MARSHAL_OBJECT(code);
	TAP_FUNCTION_MARSHAL_OBJECT(globals);
	TAP_FUNCTION_MARSHAL_OBJECT(defaults);
	TAP_FUNCTION_MARSHAL_OBJECT(kwdefaults);
	TAP_FUNCTION_MARSHAL_OBJECT(closure);
	TAP_FUNCTION_MARSHAL_OBJECT(doc);
	TAP_FUNCTION_MARSHAL_OBJECT(name);
	TAP_FUNCTION_MARSHAL_OBJECT(dict);
	TAP_FUNCTION_MARSHAL_OBJECT(weakreflist);
	TAP_FUNCTION_MARSHAL_OBJECT(module);
	TAP_FUNCTION_MARSHAL_OBJECT(annotations);
	TAP_FUNCTION_MARSHAL_OBJECT(qualname);

	return 0;
}

#undef TAP_FUNCTION_MARSHAL_OBJECT

static PyObject *function_unmarshal_alloc(const void *data, Py_ssize_t size, PeerObject &peer)
{
	if (size != sizeof (Portable))
		return nullptr;

	return reinterpret_cast<PyObject *> (PyObject_GC_New(PyFunctionObject, &PyFunction_Type));
}

#define TAP_FUNCTION_UNMARSHAL_KEY(NAME) \
	do { \
		PyObject *ptr = nullptr; \
		Key key = port(portable->NAME); \
		if (key != -1) { \
			ptr = peer.object(key); \
			if (ptr == nullptr) { \
				fprintf(stderr, "tap function unmarshal: " #NAME " lookup error\n"); \
				return -1; \
			} \
			Py_INCREF(ptr); \
		} \
		function->func_##NAME = ptr; \
	} while (0)

static int function_unmarshal_init(PyObject *object, const void *data, Py_ssize_t size, PeerObject &peer)
{
	const Portable *portable = reinterpret_cast<const Portable *> (data);
	PyFunctionObject *function = reinterpret_cast<PyFunctionObject *> (object);

	TAP_FUNCTION_UNMARSHAL_KEY(code);
	TAP_FUNCTION_UNMARSHAL_KEY(globals);
	TAP_FUNCTION_UNMARSHAL_KEY(defaults);
	TAP_FUNCTION_UNMARSHAL_KEY(kwdefaults);
	TAP_FUNCTION_UNMARSHAL_KEY(closure);
	TAP_FUNCTION_UNMARSHAL_KEY(doc);
	TAP_FUNCTION_UNMARSHAL_KEY(name);
	TAP_FUNCTION_UNMARSHAL_KEY(dict);
	TAP_FUNCTION_UNMARSHAL_KEY(weakreflist);
	TAP_FUNCTION_UNMARSHAL_KEY(module);
	TAP_FUNCTION_UNMARSHAL_KEY(annotations);
	TAP_FUNCTION_UNMARSHAL_KEY(qualname);

    _PyObject_GC_TRACK(function);
	return 0;
}

#undef TAP_FUNCTION_UNMARSHAL_KEY

const TypeHandler function_type_handler = {
	FUNCTION_TYPE_ID,
	function_traverse,
	function_marshaled_size,
	function_marshal,
	function_unmarshal_alloc,
	function_unmarshal_init,
};

} // namespace tap
