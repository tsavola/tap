#include "core.hpp"

#include <cstring>

#define TAP_OPAQUE_PREFIX  "tap.core.opaque."

namespace tap {

struct OpaqueObject {
	PyObject_HEAD
};

static PyObject *opaque_new(PyTypeObject *type, PyObject *args, PyObject *kwargs) noexcept
{
	return type->tp_alloc(type, 0);
}

static void opaque_dealloc(PyObject *opaque) noexcept
{
	Py_TYPE(opaque)->tp_free(opaque);
}

struct OpaqueTypeObject: public PyTypeObject {
	OpaqueTypeObject(const std::string &name)
	{
		size_t prefix_len = strlen(TAP_OPAQUE_PREFIX);
		auto opaque_name_len = prefix_len + name.length();
		auto opaque_name = new char[opaque_name_len + 1];
		memcpy(opaque_name, TAP_OPAQUE_PREFIX, prefix_len);
		memcpy(opaque_name + prefix_len, name.data(), name.length());
		opaque_name[opaque_name_len] = '\0';

		memset(static_cast<PyTypeObject *> (this), 0, sizeof (PyTypeObject));
		ob_base.ob_base.ob_refcnt = 1;
		tp_name = opaque_name;
		tp_basicsize = sizeof (OpaqueTypeObject);
		tp_dealloc = opaque_dealloc;
		tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE;
		tp_new = opaque_new;
	}

	~OpaqueTypeObject() noexcept
	{
		delete[] tp_name;
	}

private:
	OpaqueTypeObject(const OpaqueTypeObject &);
	void operator=(const OpaqueTypeObject &);
};

PyTypeObject *opaque_type_for_name(const std::string &name) noexcept
{
	auto &types = instance_opaque_types();
	PyTypeObject *type = nullptr;

	auto i = types.find(name);
	if (i != types.end()) {
		type = i->second;
	} else {
		OpaqueTypeObject *opaque_type;

		try {
			opaque_type = new OpaqueTypeObject(name);

			if (PyType_Ready(opaque_type) < 0) {
				delete opaque_type;
				return nullptr;
			}

			try {
				types.insert(std::make_pair(name, opaque_type));
			} catch (...) {
				delete opaque_type;
				return nullptr;
			}
		} catch (...) {
			return nullptr;
		}

		type = opaque_type;
	}

	return type;
}

static int opaque_traverse(PyObject *object, visitproc visit, void *arg) noexcept
{
	return 0;
}

static Py_ssize_t opaque_marshaled_size(PyObject *object) noexcept
{
	return strlen(Py_TYPE(object)->tp_name);
}

static int opaque_marshal(PyObject *object, void *buf, Py_ssize_t size, PeerObject &peer) noexcept
{
	const char *name = Py_TYPE(object)->tp_name;
	memcpy(buf, name, strlen(name));
	return 0;
}

static PyObject *opaque_unmarshal_alloc(const void *data, Py_ssize_t size, PeerObject &peer) noexcept
{
	if (size == 0)
		return nullptr;

	auto text = reinterpret_cast <const char *> (data);

	for (int i = 0; i < size; i++) {
		if (text[i] == '\0')
			return nullptr;
	}

	if (!unicode_verify_utf8(text, size)) {
		fprintf(stderr, "tap opaque unmarshal: bad UTF-8\n");
		return nullptr;
	}

	PyTypeObject *type = nullptr;

	try {
		type = opaque_type_for_name(std::string(text, size));
	} catch (...) {
	}

	if (type == nullptr)
		return nullptr;

	return type->tp_alloc(type, 0);
}

static int opaque_unmarshal_init(PyObject *object, const void *data, Py_ssize_t size, PeerObject &peer) noexcept
{
	return 0;
}

const TypeHandler opaque_type_handler = {
	OPAQUE_TYPE_ID,
	opaque_traverse,
	opaque_marshaled_size,
	opaque_marshal,
	opaque_unmarshal_alloc,
	opaque_unmarshal_init,
};

} // namespace tap
