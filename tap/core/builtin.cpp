#include "core.hpp"

#include <cstring>

namespace tap {

bool builtin_check(PyObject *object)
{
	if (!PyCFunction_Check(object))
		return false;

	PyCFunctionObject *builtin = reinterpret_cast<PyCFunctionObject *> (object);

	return builtin->m_module &&
	       PyUnicode_Check(builtin->m_module) &&
	       PyUnicode_READY(builtin->m_module) == 0;
}

static int builtin_traverse(PyObject *object, visitproc visit, void *arg)
{
	return 0;
}

static Py_ssize_t builtin_marshaled_size(PyObject *object)
{
	PyCFunctionObject *builtin = reinterpret_cast<PyCFunctionObject *> (object);

	// TODO: ensure UTF-8
	const char *module = reinterpret_cast<const char *> (PyUnicode_DATA(builtin->m_module));

	return strlen(module) + 1 + strlen(builtin->m_ml->ml_name) + 1;
}

static int builtin_marshal(PyObject *object, void *buf, Py_ssize_t size, PeerObject &peer)
{
	PyCFunctionObject *builtin = reinterpret_cast<PyCFunctionObject *> (object);
	char *portable = reinterpret_cast<char *> (buf);

	const char *module = reinterpret_cast<const char *> (PyUnicode_DATA(builtin->m_module));
	size_t modulesize = strlen(module) + 1;

	memcpy(portable, module, modulesize);
	strcpy(portable + modulesize, builtin->m_ml->ml_name);

	return 0;
}

static PyObject *builtin_unmarshal_alloc(const void *data, Py_ssize_t size, PeerObject &peer)
{
	if (size < 4)
		return NULL;

	const char *portable = reinterpret_cast<const char *> (data);
	if (portable[size - 1] != '\0')
		return NULL;

	const char *module = portable;
	size_t modulesize = strlen(module) + 1;

	const char *name = module + modulesize;
	size_t namesize = size - modulesize;

	if (namesize == 0 || strlen(name) != namesize - 1)
		return NULL;

	PyObject *mod = PyImport_ImportModule(module);
	if (mod == NULL) {
		fprintf(stderr, "builtin_unmarshal_alloc: failed to import module %s\n", module);
		return NULL;
	}

	PyObject *object = PyDict_GetItemString(PyModule_GetDict(mod), name);
	if (object == NULL)
		return NULL;

	Py_INCREF(object);
	Py_DECREF(mod);

	return object;
}

static int builtin_unmarshal_init(PyObject *object, const void *data, Py_ssize_t size, PeerObject &peer)
{
	return 0;
}

const TypeHandler builtin_type_handler = {
	BUILTIN_TYPE_ID,
	builtin_traverse,
	builtin_marshaled_size,
	builtin_marshal,
	builtin_unmarshal_alloc,
	builtin_unmarshal_init,
};

} // namespace tap
