#ifndef TAP_CORE_CORE_HPP
#define TAP_CORE_CORE_HPP

#include <Python.h>

#include <cstdint>
#include <map>
#include <unordered_set>
#include <utility>

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

	PeerObject();
	~PeerObject();

	int insert(PyObject *object, Key key, bool dirty = false) noexcept;
	void clear(PyObject *object) noexcept;
	std::pair<Key, bool> insert_or_clear(PyObject *object) noexcept;
	Key key(PyObject *object) noexcept;
	PyObject *object(Key key) noexcept;
	void object_freed(void *ptr) noexcept;

private:
	class State;

	PeerObject(const PeerObject &);
	void operator=(const PeerObject &);

	Key insert_new(PyObject *object, bool dirty) noexcept;

	std::map<void *, State> states;
	std::map<Key, PyObject *> objects;
	Key next_key;
};

struct TypeHandler {
	int32_t type_id;
	int (*traverse)(PyObject *object, visitproc visit, void *arg) noexcept;
	Py_ssize_t (*marshaled_size)(PyObject *object) noexcept;
	int (*marshal)(PyObject *object, void *buf, Py_ssize_t size, PeerObject &peer) noexcept;
	PyObject *(*unmarshal_alloc)(const void *marshal_data, Py_ssize_t marshal_size, PeerObject &peer) noexcept;
	int (*unmarshal_init)(PyObject *object, const void *marshal_data, Py_ssize_t marshal_size, PeerObject &peer) noexcept;
	int (*unmarshal_update)(PyObject *object, const void *marshal_data, Py_ssize_t marshal_size, PeerObject &peer) noexcept;
};

int instance_init() noexcept;
std::unordered_set<PeerObject *> &instance_peers() noexcept;

void allocator_init() noexcept;

int peer_type_init() noexcept;

int opaque_type_init() noexcept;

bool unicode_verify_utf8(const void *data, Py_ssize_t size) noexcept;

bool builtin_check(PyObject *object) noexcept;

const TypeHandler *type_handler_for_object(PyObject *object) noexcept;
const TypeHandler *type_handler_for_id(int32_t type_id) noexcept;
PyTypeObject *type_object_for_id(int32_t type_id) noexcept;

int marshal(PeerObject &peer, PyObject *bytearray, PyObject *object) noexcept;
PyObject *unmarshal(PeerObject &peer, const void *data, Py_ssize_t size) noexcept;

extern PyTypeObject peer_type;
extern PyTypeObject opaque_type;

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
extern const TypeHandler builtin_type_handler;

} // namespace tap

#endif
