#include "core.hpp"

#include <cstring>

namespace tap {

struct Portable {
	int32_t argcount;
	int32_t kwonlyargcount;
	int32_t nlocals;
	int32_t stacksize;
	int32_t flags;
	Key code;
	Key consts;
	Key names;
	Key varnames;
	Key freevars;
	Key cellvars;
	Key filename;
	Key name;
	int32_t firstlineno;
	Key lnotab;

	const uint8_t *cell2arg() const
	{
		return reinterpret_cast<const uint8_t *> (this + 1);
	}

	uint8_t *cell2arg()
	{
		return reinterpret_cast<uint8_t *> (this + 1);
	}
} TAP_PACKED;

static int code_traverse(PyObject *object, visitproc visit, void *arg)
{
	const PyCodeObject *self = reinterpret_cast<PyCodeObject *> (object);

	Py_VISIT(self->co_code);
	Py_VISIT(self->co_consts);
	Py_VISIT(self->co_names);
	Py_VISIT(self->co_varnames);
	Py_VISIT(self->co_freevars);
	Py_VISIT(self->co_cellvars);
	Py_VISIT(self->co_filename);
	Py_VISIT(self->co_name);

	Py_VISIT(self->co_lnotab);

	return 0;
}

static Py_ssize_t code_marshaled_size(PyObject *object)
{
	const PyCodeObject *codeobject = reinterpret_cast<PyCodeObject *> (object);
	Py_ssize_t size = sizeof (Portable);

	if (codeobject->co_cell2arg)
		size += PyTuple_GET_SIZE(codeobject->co_cellvars);

	return size;
}

#define TAP_CODE_MARSHAL_OBJECT(NAME) \
	do { \
		Key key = peer.key(codeobject->co_##NAME); \
		if (key < 0) \
			return -1; \
		portable->NAME = port(key); \
	} while (0)

#define TAP_CODE_MARSHAL_VALUE(NAME) \
	portable->NAME = port(codeobject->co_##NAME)

static int code_marshal(PyObject *object, void *buf, Py_ssize_t size, PeerObject &peer)
{
	const PyCodeObject *codeobject = reinterpret_cast<PyCodeObject *> (object);
	Portable *portable = reinterpret_cast<Portable *> (buf);

	TAP_CODE_MARSHAL_VALUE(argcount);
	TAP_CODE_MARSHAL_VALUE(kwonlyargcount);
	TAP_CODE_MARSHAL_VALUE(nlocals);
	TAP_CODE_MARSHAL_VALUE(stacksize);
	TAP_CODE_MARSHAL_VALUE(flags);
	TAP_CODE_MARSHAL_OBJECT(code);
	TAP_CODE_MARSHAL_OBJECT(consts);
	TAP_CODE_MARSHAL_OBJECT(names);
	TAP_CODE_MARSHAL_OBJECT(varnames);
	TAP_CODE_MARSHAL_OBJECT(freevars);
	TAP_CODE_MARSHAL_OBJECT(cellvars);
	TAP_CODE_MARSHAL_OBJECT(filename);
	TAP_CODE_MARSHAL_OBJECT(name);
	TAP_CODE_MARSHAL_VALUE(firstlineno);
	TAP_CODE_MARSHAL_OBJECT(lnotab);

	if (codeobject->co_cell2arg)
		memcpy(portable->cell2arg(), codeobject->co_cell2arg, PyTuple_GET_SIZE(codeobject->co_cellvars));

	return 0;
}

#undef TAP_CODE_MARSHAL_OBJECT
#undef TAP_CODE_MARSHAL_VALUE

static PyObject *code_unmarshal_alloc(const void *data, Py_ssize_t size, PeerObject &peer)
{
	if (size < Py_ssize_t(sizeof (Portable)))
		return NULL;

	return reinterpret_cast<PyObject *> (PyObject_NEW(PyCodeObject, &PyCode_Type));
}

#define TAP_CODE_UNMARSHAL_KEY(NAME) \
	codeobject->co_##NAME = peer.object(port(portable->NAME)); \
	if (codeobject->co_##NAME == NULL) \
		return -1; \
	Py_INCREF(codeobject->co_##NAME)

#define TAP_CODE_UNMARSHAL_VALUE(NAME) \
	codeobject->co_##NAME = port(portable->NAME)

static int code_unmarshal_init(PyObject *object, const void *data, Py_ssize_t size, PeerObject &peer)
{
	PyCodeObject *codeobject = reinterpret_cast<PyCodeObject *> (object);
	const Portable *portable = reinterpret_cast<const Portable *> (data);

	TAP_CODE_UNMARSHAL_VALUE(argcount);
	TAP_CODE_UNMARSHAL_VALUE(kwonlyargcount);
	TAP_CODE_UNMARSHAL_VALUE(nlocals);
	TAP_CODE_UNMARSHAL_VALUE(stacksize);
	TAP_CODE_UNMARSHAL_VALUE(flags);
	TAP_CODE_UNMARSHAL_KEY(code);
	TAP_CODE_UNMARSHAL_KEY(consts);
	TAP_CODE_UNMARSHAL_KEY(names);
	TAP_CODE_UNMARSHAL_KEY(varnames);
	TAP_CODE_UNMARSHAL_KEY(freevars);
	TAP_CODE_UNMARSHAL_KEY(cellvars);
	codeobject->co_cell2arg = NULL;
	TAP_CODE_UNMARSHAL_KEY(filename);
	TAP_CODE_UNMARSHAL_KEY(name);
	TAP_CODE_UNMARSHAL_VALUE(firstlineno);
	TAP_CODE_UNMARSHAL_KEY(lnotab);
	codeobject->co_zombieframe = NULL;
	codeobject->co_weakreflist = NULL;

	if (codeobject->co_argcount < 0 ||
	    codeobject->co_kwonlyargcount < 0 ||
	    codeobject->co_nlocals < 0 ||
	    !PyObject_CheckReadBuffer(codeobject->co_code) ||
	    !PyTuple_Check(codeobject->co_consts) ||
	    !PyTuple_Check(codeobject->co_names) ||
	    !PyTuple_Check(codeobject->co_varnames) ||
	    !PyTuple_Check(codeobject->co_freevars) ||
	    !PyTuple_Check(codeobject->co_cellvars) ||
	    !PyUnicode_Check(codeobject->co_filename) || PyUnicode_READY(codeobject->co_filename) < 0 ||
	    !PyUnicode_Check(codeobject->co_name) ||
	    !PyBytes_Check(codeobject->co_lnotab)) {
		return -1;
	}

	Py_ssize_t cell2arg_size = Py_ssize_t(size) - Py_ssize_t(sizeof (Portable));
	if (cell2arg_size > 0) {
		if (cell2arg_size != PyTuple_GET_SIZE(codeobject->co_cellvars))
			return -1;

		codeobject->co_cell2arg = reinterpret_cast<uint8_t *> (PyMem_MALLOC(cell2arg_size));
		if (codeobject->co_cell2arg == NULL)
			return -1;

		memcpy(codeobject->co_cell2arg, portable->cell2arg(), cell2arg_size);
	}

	return 0;
}

#undef TAP_CODE_UNMARSHAL_KEY
#undef TAP_CODE_UNMARSHAL_VALUE

const TypeHandler code_type_handler = {
	CODE_TYPE_ID,
	code_traverse,
	code_marshaled_size,
	code_marshal,
	code_unmarshal_alloc,
	code_unmarshal_init,
};

} // namespace tap
