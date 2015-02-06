#include "core.hpp"
#include "portable.hpp"

#include <algorithm>

#define TAP_FRAME_MAXBLOCKS  20
// XXX: assert(TAP_FRAME_MAXBLOCKS == CO_MAXBLOCKS)

namespace tap {

struct PortableTryBlock {
	int32_t type;
	int32_t handler;
	int32_t level;
} TAP_PACKED;

struct Portable {
	Key back;
	Key code;
	Key builtins;
	Key globals;
	Key locals;
	int32_t valuestack;
	int32_t stacktop;
	Key trace;
	Key exc_type;
	Key exc_value;
	Key exc_traceback;
	Key gen;
	int32_t lasti;
	int32_t lineno;
	int32_t iblock;
	uint8_t executing;
	PortableTryBlock blockstack[TAP_FRAME_MAXBLOCKS];
	Key localsplus[];
} TAP_PACKED;

static int frame_traverse(PyObject *object, visitproc visit, void *arg) noexcept
{
	const PyFrameObject *self = reinterpret_cast<PyFrameObject *> (object);

	Py_VISIT(self->f_back);
	Py_VISIT(self->f_code);
	Py_VISIT(self->f_builtins);
	Py_VISIT(self->f_globals);
	Py_VISIT(self->f_locals);

	// XXX: are all the local pointers valid?
	for (PyObject *const *p = self->f_localsplus; p < self->f_valuestack; p++)
		Py_VISIT(*p);

	if (self->f_stacktop) {
		for (PyObject **p = self->f_valuestack; p < self->f_stacktop; p++)
			Py_VISIT(*p);
    } else {
		fprintf(stderr, "tap frame traverse: stacktop is null\n");
	}

	Py_VISIT(self->f_trace);
	Py_VISIT(self->f_exc_type);
	Py_VISIT(self->f_exc_value);
	Py_VISIT(self->f_exc_traceback);
	Py_VISIT(self->f_gen);

	return 0;
}

static Py_ssize_t frame_marshaled_size(PyObject *object) noexcept
{
	const PyFrameObject *frameobject = reinterpret_cast<PyFrameObject *> (object);
	auto localsplus = const_cast<PyObject **> (frameobject->f_localsplus);
	size_t localsplus_num;

	if (frameobject->f_stacktop) {
		localsplus_num = frameobject->f_stacktop - localsplus;
    } else {
		fprintf(stderr, "tap frame marshal: stacktop is null\n");
		localsplus_num = frameobject->f_valuestack - localsplus;
	}

	// XXX: how much over the stacktop do we need to allocate?

	return sizeof (Portable) + localsplus_num * sizeof (Key);
}

#define TAP_FRAME_MARSHAL_OBJECT(NAME) \
	do { \
		Key remote_key = -1; \
		PyObject *object = reinterpret_cast<PyObject *> (frameobject->f_##NAME); \
		if (object) { \
			remote_key = peer.key_for_remote(object); \
			if (remote_key < 0) \
				return -1; \
		} \
		portable->NAME = port(remote_key); \
	} while (0)

#define TAP_FRAME_MARSHAL_VALUE(NAME) \
	portable->NAME = port(frameobject->f_##NAME)

static int frame_marshal(PyObject *object, void *buf, Py_ssize_t size, PeerObject &peer) noexcept
{
	const PyFrameObject *frameobject = reinterpret_cast<PyFrameObject *> (object);
	auto localsplus = const_cast<PyObject **> (frameobject->f_localsplus);
	Portable *portable = reinterpret_cast<Portable *> (buf);

	TAP_FRAME_MARSHAL_OBJECT(back);
	TAP_FRAME_MARSHAL_OBJECT(code);
	TAP_FRAME_MARSHAL_OBJECT(builtins);
	TAP_FRAME_MARSHAL_OBJECT(globals);
	TAP_FRAME_MARSHAL_OBJECT(locals);

	int32_t valuestack = frameobject->f_valuestack - localsplus;
	portable->valuestack = port(valuestack);

	int32_t stacktop = -1;
	if (frameobject->f_stacktop)
		stacktop = frameobject->f_stacktop - localsplus;
	portable->stacktop = port(stacktop);

	TAP_FRAME_MARSHAL_OBJECT(trace);
	TAP_FRAME_MARSHAL_OBJECT(exc_type);
	TAP_FRAME_MARSHAL_OBJECT(exc_value);
	TAP_FRAME_MARSHAL_OBJECT(exc_traceback);
	TAP_FRAME_MARSHAL_OBJECT(gen);
	TAP_FRAME_MARSHAL_VALUE(lasti);
	TAP_FRAME_MARSHAL_VALUE(lineno);
	TAP_FRAME_MARSHAL_VALUE(iblock);
	TAP_FRAME_MARSHAL_VALUE(executing);

	for (int i = 0; i < TAP_FRAME_MAXBLOCKS; i++) {
		const PyTryBlock &block = frameobject->f_blockstack[i];
		PortableTryBlock &portable_block = portable->blockstack[i];

		portable_block.type = port(int32_t(block.b_type));
		portable_block.handler = port(int32_t(block.b_handler));
		portable_block.level = port(int32_t(block.b_level));
	}

	for (int i = 0; i < std::max(valuestack, stacktop); i++) {
		Key remote_key = -1;
		PyObject *object = frameobject->f_localsplus[i];
		if (object) {
			remote_key = peer.key_for_remote(object);
			if (remote_key < 0)
				return -1;
		}
		portable->localsplus[i] = port(remote_key);
	}

	return 0;
}

#undef TAP_FRAME_MARSHAL_OBJECT
#undef TAP_FRAME_MARSHAL_VALUE

static PyObject *frame_unmarshal_alloc(const void *data, Py_ssize_t size, PeerObject &peer) noexcept
{
	if (size < Py_ssize_t(sizeof (Portable)))
		return nullptr;

	Py_ssize_t localsplus_size = size - Py_ssize_t(sizeof (Portable));
	if (localsplus_size % sizeof (Key))
		return nullptr;
	int localsplus_num = localsplus_size / sizeof (Key);

	// XXX: to we need to subtract sizeof (PyObject *) from PyFrameObject size?
	return reinterpret_cast<PyObject *> (PyObject_NEW_VAR(PyFrameObject, &PyFrame_Type, localsplus_num));
}

#define TAP_FRAME_UNMARSHAL_KEY(NAME) \
	do { \
		PyObject *object = nullptr; \
		Key key = port(portable->NAME); \
		if (key >= 0) { \
			object = peer.object(key); \
			if (object == nullptr) { \
				fprintf(stderr, "tap frame unmarshal error: %s\n", #NAME); \
				return -1; \
			} \
			Py_INCREF(object); \
		} \
		frameobject->f_##NAME = object; \
	} while (0)

#define TAP_FRAME_UNMARSHAL_KEY_TYPE(NAME, TYPE) \
	do { \
		TYPE##Object *typed_object = nullptr; \
		Key key = port(portable->NAME); \
		if (key >= 0) { \
			PyObject *object = peer.object(key); \
			if (object == nullptr) { \
				fprintf(stderr, "tap frame unmarshal error: %s\n", #NAME); \
				return -1; \
			} \
			if (!TYPE##_Check(object)) { \
				fprintf(stderr, "tap frame unmarshal error: %s is not a %s\n", #NAME, #TYPE); \
				return -1; \
			} \
			Py_INCREF(object); \
			typed_object = reinterpret_cast<TYPE##Object *> (object); \
		} \
		frameobject->f_##NAME = typed_object; \
	} while (0)

#define TAP_FRAME_UNMARSHAL_VALUE(NAME) \
	frameobject->f_##NAME = port(portable->NAME)

static int frame_unmarshal_init(PyObject *object, const void *data, Py_ssize_t size, PeerObject &peer) noexcept
{
	PyFrameObject *frameobject = reinterpret_cast<PyFrameObject *> (object);
	const Portable *portable = reinterpret_cast<const Portable *> (data);
	int localsplus_num = (size - sizeof (Portable)) / sizeof (Key);

	TAP_FRAME_UNMARSHAL_KEY_TYPE(back, PyFrame);
	TAP_FRAME_UNMARSHAL_KEY_TYPE(code, PyCode);
	TAP_FRAME_UNMARSHAL_KEY(builtins);
	TAP_FRAME_UNMARSHAL_KEY(globals);
	TAP_FRAME_UNMARSHAL_KEY(locals);

	int32_t valuestack = port(portable->valuestack);
	frameobject->f_valuestack = frameobject->f_localsplus + valuestack;

	int32_t stacktop = port(portable->valuestack);
	if (stacktop >= 0) {
		frameobject->f_stacktop = frameobject->f_localsplus + stacktop;
	} else {
		frameobject->f_stacktop = nullptr;
		fprintf(stderr, "tap frame unmarshal: stacktop is null\n");
	}

	TAP_FRAME_UNMARSHAL_KEY(trace);
	TAP_FRAME_UNMARSHAL_KEY(exc_type);
	TAP_FRAME_UNMARSHAL_KEY(exc_value);
	TAP_FRAME_UNMARSHAL_KEY(exc_traceback);

	// don't Py_INCREF the borrowed gen reference
	PyObject *gen_object = nullptr;
	Key gen_key = port(portable->gen);
	if (gen_key >= 0) {
		gen_object = peer.object(gen_key);
		if (gen_object == nullptr) {
			fprintf(stderr, "tap frame unmarshal error: gen\n");
			return -1;
		}
	}
	frameobject->f_gen = gen_object;

	TAP_FRAME_UNMARSHAL_VALUE(lasti);
	TAP_FRAME_UNMARSHAL_VALUE(lineno);
	TAP_FRAME_UNMARSHAL_VALUE(iblock);
	TAP_FRAME_UNMARSHAL_VALUE(executing);

	for (int i = 0; i < TAP_FRAME_MAXBLOCKS; i++) {
		const PortableTryBlock &portable_block = portable->blockstack[i];
		PyTryBlock &block = frameobject->f_blockstack[i];

		block.b_type = port(portable_block.type);
		block.b_handler = port(portable_block.handler);
		block.b_level = port(portable_block.level);
	}

	for (int i = 0; i < localsplus_num; i++) {
		PyObject *object = nullptr;
		Key key = port(portable->localsplus[i]);
		if (key >= 0) {
			object = peer.object(key);
			if (object == nullptr) {
				fprintf(stderr, "tap frame unmarshal error: localsplus[%d] (num=%d)\n", i, localsplus_num);
				return -1;
			}
			Py_INCREF(object);
		}
		frameobject->f_localsplus[i] = object;
	}

	return 0;
}

#undef TAP_FRAME_UNMARSHAL_KEY
#undef TAP_FRAME_UNMARSHAL_KEY_TYPE
#undef TAP_FRAME_UNMARSHAL_VALUE

const TypeHandler frame_type_handler = {
	FRAME_TYPE_ID,
	frame_traverse,
	frame_marshaled_size,
	frame_marshal,
	frame_unmarshal_alloc,
	frame_unmarshal_init,
};

} // namespace tap
