#include "core.hpp"

#include <stdexcept>
#include <unordered_set>

namespace tap {

struct Item {
	Key key;
	Key value;
} TAP_PACKED;

static int dict_traverse(PyObject *object, visitproc visit, void *arg)
{
	Py_ssize_t pos = 0;
	PyObject *key;
	PyObject *value;

	for (Py_ssize_t i = 0; PyDict_Next(object, &pos, &key, &value); ++i) {
		Py_VISIT(key);
		Py_VISIT(value);
	}

	return 0;
}

static Py_ssize_t dict_marshaled_size(PyObject *object)
{
	return sizeof (Item) * PyDict_Size(object);
}

static int dict_marshal(PyObject *object, void *buf, Py_ssize_t size, PeerObject &peer)
{
	Item *portable = reinterpret_cast<Item *> (buf);
	Py_ssize_t pos = 0;
	PyObject *key_o;
	PyObject *value_o;

	for (Py_ssize_t i = 0; PyDict_Next(object, &pos, &key_o, &value_o); ++i) {
		Key key_k = peer.key(key_o);
		if (key_k < 0)
			return -1;

		Key value_k = peer.key(value_o);
		if (value_k < 0)
			return -1;

		Item &item = portable[i];
		item.key = port(key_k);
		item.value = port(value_k);
	}

	return 0;
}

static PyObject *dict_unmarshal_alloc(const void *data, Py_ssize_t size, PeerObject &peer)
{
	if (size % sizeof (Item))
		return nullptr;

	return PyDict_New();
}

static int dict_unmarshal_init(PyObject *object, const void *data, Py_ssize_t size, PeerObject &peer)
{
	const Item *portable = reinterpret_cast<const Item *> (data);
	Py_ssize_t length = size / sizeof (Item);

	for (Py_ssize_t i = 0; i < length; ++i) {
		const Item &item = portable[i];

		PyObject *key = peer.object(port(item.key));
		if (key == nullptr)
			return -1;

		PyObject *value = peer.object(port(item.value));
		if (value == nullptr)
			return -1;

		if (PyDict_SetItem(object, key, value) < 0)
			return -1;
	}

	return 0;
}

static int dict_unmarshal_update(PyObject *object, const void *data, Py_ssize_t size, PeerObject &peer)
{
	if (size % sizeof (Item))
		return -1;

	const Item *portable = reinterpret_cast<const Item *> (data);
	Py_ssize_t length = size / sizeof (Item);
	std::unordered_set<PyObject *> included_keys;

	for (Py_ssize_t i = 0; i < length; ++i) {
		const Item &item = portable[i];

		PyObject *key = peer.object(port(item.key));
		if (key == nullptr)
			return -1;

		PyObject *value = peer.object(port(item.value));
		if (value == nullptr)
			return -1;

		if (PyDict_SetItem(object, key, value) < 0)
			return -1;

		try {
			included_keys.insert(key);
		} catch (std::bad_alloc) {
			return -1;
		}
	}

	Py_ssize_t pos = 0;
	PyObject *key;
	PyObject *value;
	std::unordered_set<PyObject *> excluded_keys;

	while (PyDict_Next(object, &pos, &key, &value)) {
		if (included_keys.find(key) == included_keys.end()) {
			try {
				excluded_keys.insert(key);
			} catch (std::bad_alloc) {
				return -1;
			}
		}
	}

	for (PyObject *key: excluded_keys)
		PyDict_DelItem(object, key);

	return 0;
}

const TypeHandler dict_type_handler = {
	DICT_TYPE_ID,
	dict_traverse,
	dict_marshaled_size,
	dict_marshal,
	dict_unmarshal_alloc,
	dict_unmarshal_init,
	dict_unmarshal_update,
};

} // namespace tap
