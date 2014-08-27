#include "core.hpp"

#include <stdexcept>
#include <vector>

namespace tap {

static int list_traverse(PyObject *object, visitproc visit, void *arg)
{
	for (Py_ssize_t i = 0; i < PyList_GET_SIZE(object); ++i)
		Py_VISIT(PyList_GET_ITEM(object, i));

	return 0;
}

static Py_ssize_t list_marshaled_size(PyObject *object)
{
	return sizeof (Key) * PyList_GET_SIZE(object);
}

static int list_marshal(PyObject *object, void *buf, Py_ssize_t size, PeerObject &peer)
{
	Key *portable = reinterpret_cast<Key *> (buf);

	for (Py_ssize_t i = 0; i < PyList_GET_SIZE(object); ++i) {
		Key key = peer.key(PyList_GET_ITEM(object, i));
		if (key < 0)
			return -1;

		portable[i] = port(key);
	}

	return 0;
}

static PyObject *list_unmarshal_alloc(const void *data, Py_ssize_t size, PeerObject &peer)
{
	if (size % sizeof (Key))
		return NULL;

	return PyList_New(size / sizeof (Key));
}

static int list_unmarshal_init(PyObject *object, const void *data, Py_ssize_t size, PeerObject &peer)
{
	const Key *portable = reinterpret_cast<const Key *> (data);

	for (Py_ssize_t i = 0; i < PyList_GET_SIZE(object); ++i) {
		PyObject *item = peer.object(port(portable[i]));
		if (item == NULL)
			return -1;

		Py_INCREF(item);
		PyList_SET_ITEM(object, i, item);
	}

	return 0;
}

static int list_unmarshal_update(PyObject *object, const void *data, Py_ssize_t size, PeerObject &peer)
{
	if (size % sizeof (Key))
		return -1;

	const Key *portable = reinterpret_cast<const Key *> (data);
	std::vector<PyObject *> refs;

	try {
		refs.reserve(PyList_GET_SIZE(object));
	} catch (std::bad_alloc) {
		return -1;
	}

	for (Py_ssize_t i = 0; i < PyList_GET_SIZE(object); ++i) {
		PyObject *item = PyList_GET_ITEM(object, i);
		Py_INCREF(item);
		refs.push_back(item);
	}

	PyList_SetSlice(object, 0, PyList_GET_SIZE(object), NULL);

	Py_ssize_t length = size / sizeof (Key);

	for (Py_ssize_t i = 0; i < length; ++i) {
		PyObject *item = peer.object(port(portable[i]));
		if (item == NULL)
			return -1;

		if (PyList_Append(object, item) < 0)
			return -1;
	}

	for (PyObject *item: refs)
		Py_DECREF(item);

	return 0;
}

const TypeHandler list_type_handler = {
	LIST_TYPE_ID,
	list_traverse,
	list_marshaled_size,
	list_marshal,
	list_unmarshal_alloc,
	list_unmarshal_init,
	list_unmarshal_update,
};

} // namespace tap
