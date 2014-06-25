#include "core.hpp"

using namespace tap;

extern "C" {

static PyObject *tap_marshal(PyObject *self, PyObject *args)
{
	PyObject *result = NULL;
	PyObject *peer;
	PyObject *bytearray;
	PyObject *object;

	if (PyArg_ParseTuple(args, "O!O!O", &peer_type, &peer, &PyByteArray_Type, &bytearray, &object)) {
		if (marshal(*reinterpret_cast <PeerObject *>(peer), bytearray, object) == 0) {
			Py_INCREF(Py_None);
			result = Py_None;
		}
	}

	return result;
}

static PyObject *tap_unmarshal(PyObject *self, PyObject *args)
{
	PyObject *result = NULL;
	PyObject *peer;
	Py_buffer buffer;

	if (PyArg_ParseTuple(args, "O!y*", &peer_type, &peer, &buffer)) {
		result = unmarshal(*reinterpret_cast <PeerObject *>(peer), buffer.buf, buffer.len);
		PyBuffer_Release(&buffer);
	}

	return result;
}

static PyMethodDef tap_methods[] = {
	{ "marshal", tap_marshal, METH_VARARGS, NULL },
	{ "unmarshal", tap_unmarshal, METH_VARARGS, NULL },
	{ NULL, NULL, 0, NULL }
};

static PyModuleDef tap_module = {
	PyModuleDef_HEAD_INIT,
	"tap.core",
	NULL,
	-1,
	tap_methods
};

PyMODINIT_FUNC PyInit_core()
{
	if (instance_init() < 0)
		return NULL;

	if (peer_type_init() < 0)
		return NULL;

	if (opaque_type_init() < 0)
		return NULL;

	allocator_init();

	PyObject *m = PyModule_Create(&tap_module);
	if (m == NULL)
		return NULL;

	Py_INCREF(&peer_type);
	PyModule_AddObject(m, "Peer", (PyObject *) &peer_type);

	Py_INCREF(&opaque_type);
	PyModule_AddObject(m, "Opaque", (PyObject *) &opaque_type);

	return m;
}

} // extern "C"
