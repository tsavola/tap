#include "core.hpp"
#include "portable.hpp"

#include <stdexcept>
#include <vector>

namespace tap {

static int (*list_py_ass_subscript_orig)(PyObject *, PyObject *, PyObject *) noexcept;

static int list_py_ass_subscript_wrap(PyObject *self, PyObject *key, PyObject *value) noexcept
{
	int ret = list_py_ass_subscript_orig(self, key, value);
	if (ret == 0)
		peers_touch(self);

	return ret;
}

int list_py_type_init() noexcept
{
	list_py_ass_subscript_orig = PyList_Type.tp_as_mapping->mp_ass_subscript;
	PyList_Type.tp_as_mapping->mp_ass_subscript = list_py_ass_subscript_wrap;

	return 0;
}

static int list_traverse(PyObject *object, visitproc visit, void *arg) noexcept
{
	for (Py_ssize_t i = 0; i < PyList_GET_SIZE(object); ++i)
		Py_VISIT(PyList_GET_ITEM(object, i));

	return 0;
}

static Py_ssize_t list_marshaled_size(PyObject *object) noexcept
{
	return sizeof (Key) * PyList_GET_SIZE(object);
}

static int list_marshal(PyObject *object, void *buf, Py_ssize_t size, PeerObject &peer) noexcept
{
	Key *portable = reinterpret_cast<Key *> (buf);

	for (Py_ssize_t i = 0; i < PyList_GET_SIZE(object); ++i) {
		Key remote_key = peer.key_for_remote(PyList_GET_ITEM(object, i));
		if (remote_key < 0)
			return -1;

		portable[i] = port(remote_key);
	}

	return 0;
}

static PyObject *list_unmarshal_alloc(const void *data, Py_ssize_t size, PeerObject &peer) noexcept
{
	if (size % sizeof (Key))
		return nullptr;

	return PyList_New(size / sizeof (Key));
}

static int list_unmarshal_init(PyObject *object, const void *data, Py_ssize_t size, PeerObject &peer) noexcept
{
	const Key *portable = reinterpret_cast<const Key *> (data);

	for (Py_ssize_t i = 0; i < PyList_GET_SIZE(object); ++i) {
		PyObject *item = peer.object(port(portable[i]));
		if (item == nullptr)
			return -1;

		Py_INCREF(item);
		PyList_SET_ITEM(object, i, item);
	}

	return 0;
}

static int list_unmarshal_update(PyObject *object, const void *data, Py_ssize_t size, PeerObject &peer) noexcept
{
	if (size % sizeof (Key))
		return -1;

	const Key *portable = reinterpret_cast<const Key *> (data);
	std::vector<PyObject *> refs;

	try {
		refs.reserve(PyList_GET_SIZE(object));
	} catch (...) {
		return -1;
	}

	for (Py_ssize_t i = 0; i < PyList_GET_SIZE(object); ++i) {
		PyObject *item = PyList_GET_ITEM(object, i);
		Py_INCREF(item);
		refs.push_back(item);
	}

	PyList_SetSlice(object, 0, PyList_GET_SIZE(object), nullptr);

	Py_ssize_t length = size / sizeof (Key);

	for (Py_ssize_t i = 0; i < length; ++i) {
		PyObject *item = peer.object(port(portable[i]));
		if (item == nullptr)
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
