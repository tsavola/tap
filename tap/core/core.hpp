#ifndef TAP_CORE_TAP_HPP
#define TAP_CORE_TAP_HPP

#include <Python.h>

#include <cstddef>
#include <cstdint>
#include <map>
#include <stdexcept>
#include <unordered_set>
#include <utility>

#ifndef TAP_PORTABLE_BYTEORDER
# ifdef __linux__
#  include <endian.h>
# else
#  include <sys/types.h>
# endif
# if defined(BYTE_ORDER)
#  if BYTE_ORDER == LITTLE_ENDIAN
#   define TAP_PORTABLE_BYTEORDER true
#  else
#   define TAP_PORTABLE_BYTEORDER false
#  endif
# elif defined(__BYTE_ORDER)
#  if __BYTE_ORDER == __LITTLE_ENDIAN
#   define TAP_PORTABLE_BYTEORDER true
#  else
#   define TAP_PORTABLE_BYTEORDER false
#  endif
# elif defined(__APPLE__)
#  if defined(__LITTLE_ENDIAN__)
#   define TAP_PORTABLE_BYTEORDER true
#  else
#   define TAP_PORTABLE_BYTEORDER false
#  endif
# else
#  error cannot figure out byteorder
# endif
#endif

#define TAP_PACKED  __attribute__ ((packed))

namespace tap {

typedef int32_t Key;

enum TypeId {
	NONE_TYPE_ID      = 0,
	TYPE_TYPE_ID      = 1,
	OPAQUE_TYPE_ID    = 2,
	BOOL_TYPE_ID      = 3,
	LONG_TYPE_ID      = 4,
	TUPLE_TYPE_ID     = 5,
	LIST_TYPE_ID      = 6,
	DICT_TYPE_ID      = 7,
	BYTES_TYPE_ID     = 8,
	UNICODE_TYPE_ID   = 9,
	CODE_TYPE_ID      = 10,
	FUNCTION_TYPE_ID  = 11,
	MODULE_TYPE_ID    = 12,
	BUILTIN_TYPE_ID   = 13,
	TYPE_ID_COUNT
};

struct PeerObject {
	PyObject_HEAD

	PeerObject() throw (std::bad_alloc);
	~PeerObject();

	int insert(PyObject *object, Key key, bool dirty = false);
	void clear(PyObject *object);
	std::pair<Key, bool> insert_or_clear(PyObject *object);
	Key key(PyObject *object);
	PyObject *object(Key key);
	void object_freed(void *ptr);

private:
	class State;

	PeerObject(const PeerObject &);
	void operator=(const PeerObject &);

	Key insert_new(PyObject *object, bool dirty);

	std::map<PyObject *, State> states;
	std::map<Key, PyObject *> objects;
	Key next_key;
};

int peer_type_init();
extern PyTypeObject peer_type;

struct TypeHandler {
	int32_t type_id;
	int (*traverse)(PyObject *object, visitproc visit, void *arg);
	Py_ssize_t (*marshaled_size)(PyObject *object);
	int (*marshal)(PyObject *object, void *buf, Py_ssize_t size, PeerObject &peer);
	PyObject *(*unmarshal_alloc)(const void *marshal_data, Py_ssize_t marshal_size, PeerObject &peer);
	int (*unmarshal_init)(PyObject *object, const void *marshal_data, Py_ssize_t marshal_size, PeerObject &peer);
	int (*unmarshal_update)(PyObject *object, const void *marshal_data, Py_ssize_t marshal_size, PeerObject &peer);
};

int instance_init();
std::unordered_set<PeerObject *> &instance_peers();

extern const TypeHandler none_type_handler;
extern const TypeHandler type_type_handler;
extern const TypeHandler opaque_type_handler;
extern const TypeHandler bool_type_handler;
extern const TypeHandler long_type_handler;
extern const TypeHandler tuple_type_handler;
extern const TypeHandler list_type_handler;
extern const TypeHandler dict_type_handler;
extern const TypeHandler bytes_type_handler;
extern const TypeHandler unicode_type_handler;
extern const TypeHandler code_type_handler;
extern const TypeHandler function_type_handler;
extern const TypeHandler module_type_handler;
bool builtin_check(PyObject *object);
extern const TypeHandler builtin_type_handler;

const TypeHandler *type_handler_for_object(PyObject *object);
const TypeHandler *type_handler_for_id(int32_t type_id);
PyTypeObject *type_object_for_id(int32_t type_id);

int opaque_type_init();
extern PyTypeObject opaque_type;

int marshal(PeerObject &peer, PyObject *bytearray, PyObject *object);
PyObject *unmarshal(PeerObject &peer, const void *data, Py_ssize_t size);

void allocator_init();

#if TAP_PORTABLE_BYTEORDER

template <typename T> inline T port(T x) throw () { return x; }

#else

template <typename T, unsigned int N> struct Porter;

template <typename T> struct Porter<T, 1> {
	static inline T port(T x) throw () { return x; }
};

template <typename T> struct Porter<T, 2> {
	static inline T port(T x) throw () { return __bswap_16(x); }
};

template <typename T> struct Porter<T, 4> {
	static inline T port(T x) throw () { return __bswap_32(x); }
};

template <typename T> struct Porter<T, 8> {
	static inline T port(T x) throw () { return __bswap_64(x); }
};

template <typename T> inline T port(T x) throw () { return Porter<T, sizeof (T)>::port(x); }

#endif

} // namespace tap

#endif
