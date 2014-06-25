#include "core.hpp"

#include <cassert>
#include <cstring>

namespace tap {

static int unicode_traverse(PyObject *object, visitproc visit, void *arg)
{
	return 0;
}

static Py_ssize_t unicode_marshaled_size(PyObject *object)
{
	if (PyUnicode_KIND(object) != PyUnicode_1BYTE_KIND) {
		fprintf(stderr, "tap unicode marshal: unsupported kind of unicode (%d)\n", PyUnicode_KIND(object));
		return -1;
	}

	return PyUnicode_GET_LENGTH(object);
}

static int unicode_marshal(PyObject *object, void *buf, Py_ssize_t size, PeerObject &peer)
{
	memcpy(buf, PyUnicode_1BYTE_DATA(object), size);
	return 0;
}

static PyObject *unicode_unmarshal_alloc(const void *data, Py_ssize_t size, PeerObject &peer)
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
				fprintf(stderr, "tap unicode unmarshal: bad UTF-8\n");
				return NULL;
			}

			Py_ssize_t next = i + skip;

			if (next > size) {
				fprintf(stderr, "tap unicode unmarshal: bad UTF-8\n");
				return NULL;
			}

			for (; i < next; i++) {
				byte = bytes[i];

				if ((byte & 0xc0) != 0x80) {
					fprintf(stderr, "tap unicode unmarshal: bad UTF-8\n");
					return NULL;
				}
			}
		}
	}

	return PyUnicode_FromKindAndData(PyUnicode_1BYTE_KIND, data, size);
}

static int unicode_unmarshal_init(PyObject *object, const void *data, Py_ssize_t size, PeerObject &peer)
{
	return 0;
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