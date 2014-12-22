#include "core.hpp"
#include "portable.hpp"

#include <cassert>
#include <cstring>

namespace tap {

static int unicode_traverse(PyObject *object, visitproc visit, void *arg) noexcept
{
	return 0;
}

static Py_ssize_t unicode_marshaled_size(PyObject *object) noexcept
{
	Py_ssize_t size;

	if (PyUnicode_AsUTF8AndSize(object, &size) == nullptr)
		return -1;

	return size;
}

static int unicode_marshal(PyObject *object, void *buf, Py_ssize_t size, PeerObject &peer) noexcept
{
	const char *data = PyUnicode_AsUTF8(object);
	if (data == nullptr)
		return -1;

	memcpy(buf, data, size);
	return 0;
}

static PyObject *unicode_unmarshal_alloc(const void *data, Py_ssize_t size, PeerObject &peer) noexcept
{
	if (!unicode_verify_utf8(data, size)) {
		fprintf(stderr, "tap unicode unmarshal: bad UTF-8\n");
		return nullptr;
	}

	return PyUnicode_FromStringAndSize(reinterpret_cast<const char *> (data), size);
}

static int unicode_unmarshal_init(PyObject *object, const void *data, Py_ssize_t size, PeerObject &peer) noexcept
{
	return 0;
}

bool unicode_verify_utf8(const void *data, Py_ssize_t size) noexcept
{
	const uint8_t *bytes = reinterpret_cast<const uint8_t *> (data);
	Py_ssize_t i = 0;

	while (i < size) {
		uint8_t byte = bytes[i++];

		if (byte & 0x80) {
			Py_ssize_t skip;

			if ((byte & 0xe0) == 0xc0) {
				skip = 1;
			} else if ((byte & 0xf0) == 0xe0) {
				skip = 2;
			} else if ((byte & 0xf8) == 0xf0) {
				skip = 3;
			} else {
				return false;
			}

			Py_ssize_t next = i + skip;
			if (next > size)
				return false;

			for (; i < next; i++) {
				byte = bytes[i];
				if ((byte & 0xc0) != 0x80)
					return false;
			}
		}
	}

	return true;
}

const TypeHandler unicode_type_handler = {
	UNICODE_TYPE_ID,
	unicode_traverse,
	unicode_marshaled_size,
	unicode_marshal,
	unicode_unmarshal_alloc,
	unicode_unmarshal_init,
};

} // namespace tap
