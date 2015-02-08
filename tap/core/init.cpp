#include "core.hpp"
#include "init.hpp"

using namespace tap;

namespace tap {

static PyObject *marshal_py(PyObject *self, PyObject *args) noexcept
{
	PyObject *result = nullptr;
	PyObject *peer;
	PyObject *bytearray;
	PyObject *object = nullptr;

	if (PyArg_ParseTuple(args, "O!O!|O", &peer_type, &peer, &PyByteArray_Type, &bytearray, &object)) {
		if (marshal(*reinterpret_cast <PeerObject *>(peer), bytearray, object) == 0) {
			Py_INCREF(Py_None);
			result = Py_None;
		}
	}

	return result;
}

static PyObject *unmarshal_py(PyObject *self, PyObject *args) noexcept
{
	PyObject *result = nullptr;
	PyObject *peer;
	Py_buffer buffer;

	if (PyArg_ParseTuple(args, "O!y*", &peer_type, &peer, &buffer)) {
		result = unmarshal(*reinterpret_cast <PeerObject *>(peer), buffer.buf, buffer.len);
		PyBuffer_Release(&buffer);
	}

	return result;
}

static PyMethodDef method_defs[] = {
	{ "marshal", marshal_py, METH_VARARGS },
	{ "unmarshal", unmarshal_py, METH_VARARGS },
	{}
};

static PyModuleDef module_def = {
	PyModuleDef_HEAD_INIT,
	"tap.core",
	nullptr,
	-1,
	method_defs,
};

} // namespace tap

PyMODINIT_FUNC PyInit_core() noexcept
{
	if (instance_init() < 0)
		return nullptr;

	if (peer_type_init() < 0)
		return nullptr;

	list_py_type_init();
	dict_py_type_init();

	allocator_init();

	PyObject *module_obj = PyModule_Create(&module_def);
	if (module_obj == nullptr)
		return nullptr;

	Py_INCREF(&peer_type);
	PyModule_AddObject(module_obj, "Peer", (PyObject *) &peer_type);

	return module_obj;
}
