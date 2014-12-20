#include "core.hpp"

#include <cstring>

// copied from cpython/Objects/moduleobject.c
typedef struct {
    PyObject_HEAD
    PyObject *md_dict;
    struct PyModuleDef *md_def;
    void *md_state;
    PyObject *md_weaklist;
    PyObject *md_name;  /* for logging purposes after md_dict is cleared */
} PyModuleObject;

namespace tap {

struct Portable {
	Key dict;

	const char *name() const noexcept
	{
		return reinterpret_cast<const char *> (this + 1);
	}

	char *name() noexcept
	{
		return reinterpret_cast<char *> (this + 1);
	}
} TAP_PACKED;

static int module_traverse(PyObject *object, visitproc visit, void *arg) noexcept
{
	Py_VISIT(PyModule_GetDict(object));

	return 0;
}

static Py_ssize_t module_marshaled_size(PyObject *object) noexcept
{
	return sizeof (Portable) + strlen(PyModule_GetName(object));
}

static int module_marshal(PyObject *object, void *buf, Py_ssize_t size, PeerObject &peer) noexcept
{
	Key dict = peer.key(PyModule_GetDict(object));
	if (dict < 0)
		return -1;

	const char *name = PyModule_GetName(object);

	Portable *portable = reinterpret_cast<Portable *> (buf);

	portable->dict = port(dict);
	memcpy(portable->name(), name, strlen(name));

	return 0;
}

static PyObject *module_unmarshal_alloc(const void *data, Py_ssize_t size, PeerObject &peer) noexcept
{
	if (size <= Py_ssize_t(sizeof (Portable)))
		return nullptr;

	const Portable *portable = reinterpret_cast<const Portable *> (data);

	PyObject *name = PyUnicode_FromStringAndSize(portable->name(), size - sizeof (Portable));
	if (name == nullptr)
		return nullptr;

	PyObject *object = PyModule_NewObject(name);
	Py_DECREF(name);

	return object;
}

static int module_unmarshal_init(PyObject *object, const void *data, Py_ssize_t size, PeerObject &peer) noexcept
{
	PyModuleObject *module = reinterpret_cast<PyModuleObject *> (object);
	const Portable *portable = reinterpret_cast<const Portable *> (data);

	PyObject *new_dict = peer.object(port(portable->dict));
	if (new_dict == nullptr)
		return -1;

	PyObject *old_dict = module->md_dict;
	Py_INCREF(new_dict);
	module->md_dict = new_dict;
	Py_DECREF(old_dict);

	return 0;
}

const TypeHandler module_type_handler = {
	MODULE_TYPE_ID,
	module_traverse,
	module_marshaled_size,
	module_marshal,
	module_unmarshal_alloc,
	module_unmarshal_init,
};

} // namespace tap
