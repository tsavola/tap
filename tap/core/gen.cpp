#include "core.hpp"
#include "portable.hpp"

namespace tap {

struct Portable {
	Key frame;
	uint8_t running;
	Key code;
	Key weakreflist;
} TAP_PACKED;

static int gen_traverse(PyObject *object, visitproc visit, void *arg) noexcept
{
	const PyGenObject *self = reinterpret_cast<PyGenObject *> (object);

	Py_VISIT(self->gi_frame);
	Py_VISIT(self->gi_code);
	Py_VISIT(self->gi_weakreflist);

	return 0;
}

static Py_ssize_t gen_marshaled_size(PyObject *object) noexcept
{
	return sizeof (Portable);
}

#define TAP_GEN_MARSHAL_OBJECT(NAME) \
	do { \
		Key remote_key = -1; \
		PyObject *object = reinterpret_cast<PyObject *> (genobject->gi_##NAME); \
		if (object) { \
			remote_key = peer.key_for_remote(object); \
			if (remote_key < 0) \
				return -1; \
		} \
		portable->NAME = port(remote_key); \
	} while (0)

#define TAP_GEN_MARSHAL_VALUE(NAME) \
	portable->NAME = port(genobject->gi_##NAME)

static int gen_marshal(PyObject *object, void *buf, Py_ssize_t size, PeerObject &peer) noexcept
{
	const PyGenObject *genobject = reinterpret_cast<PyGenObject *> (object);
	Portable *portable = reinterpret_cast<Portable *> (buf);

	TAP_GEN_MARSHAL_OBJECT(frame);
	TAP_GEN_MARSHAL_VALUE(running);
	TAP_GEN_MARSHAL_OBJECT(code);
	TAP_GEN_MARSHAL_OBJECT(weakreflist);

	return 0;
}

#undef TAP_GEN_MARSHAL_OBJECT
#undef TAP_GEN_MARSHAL_VALUE

static PyObject *gen_unmarshal_alloc(const void *data, Py_ssize_t size, PeerObject &peer) noexcept
{
	if (size != sizeof (Portable))
	    return nullptr;

	return reinterpret_cast<PyObject *> (PyObject_GC_New(PyGenObject, &PyGen_Type));
}

#define TAP_GEN_UNMARSHAL_KEY(NAME) \
	do { \
		PyObject *object = nullptr; \
		Key key = port(portable->NAME); \
		if (key >= 0) { \
			object = peer.object(key); \
			if (object == nullptr) { \
				fprintf(stderr, "tap gen unmarshal error: %s\n", #NAME); \
				return -1; \
			} \
			Py_INCREF(object); \
		} \
		genobject->gi_##NAME = object; \
	} while (0)

#define TAP_GEN_UNMARSHAL_KEY_TYPE(NAME, TYPE) \
	do { \
		TYPE##Object *typed_object = nullptr; \
		Key key = port(portable->NAME); \
		if (key >= 0) { \
			PyObject *object = peer.object(key); \
			if (object == nullptr) { \
				fprintf(stderr, "tap gen unmarshal error: %s\n", #NAME); \
				return -1; \
			} \
			if (!TYPE##_Check(object)) { \
				fprintf(stderr, "tap gen unmarshal error: %s is not a %s\n", #NAME, #TYPE); \
				return -1; \
			} \
			Py_INCREF(object); \
			typed_object = reinterpret_cast<TYPE##Object *> (object); \
		} \
		genobject->gi_##NAME = typed_object; \
	} while (0)

#define TAP_GEN_UNMARSHAL_VALUE(NAME) \
	genobject->gi_##NAME = port(portable->NAME)

static int gen_unmarshal_init(PyObject *object, const void *data, Py_ssize_t size, PeerObject &peer) noexcept
{
	PyGenObject *genobject = reinterpret_cast<PyGenObject *> (object);
	const Portable *portable = reinterpret_cast<const Portable *> (data);

	TAP_GEN_UNMARSHAL_KEY_TYPE(frame, PyFrame);
	TAP_GEN_UNMARSHAL_KEY(code);
	TAP_GEN_UNMARSHAL_VALUE(running);
	TAP_GEN_UNMARSHAL_KEY(weakreflist);

	_PyObject_GC_TRACK(genobject);

	return 0;
}

#undef TAP_GEN_UNMARSHAL_KEY
#undef TAP_GEN_UNMARSHAL_KEY_TYPE
#undef TAP_GEN_UNMARSHAL_VALUE

const TypeHandler gen_type_handler = {
	GEN_TYPE_ID,
	gen_traverse,
	gen_marshaled_size,
	gen_marshal,
	gen_unmarshal_alloc,
	gen_unmarshal_init,
};

} // namespace tap
