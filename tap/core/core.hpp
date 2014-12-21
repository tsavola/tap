#ifndef TAP_CORE_CORE_HPP
#define TAP_CORE_CORE_HPP

#include <Python.h>

#include <cstdint>
#include <map>
#include <unordered_set>
#include <utility>
#include <vector>

namespace tap {

typedef int64_t Key;

enum TypeId {
	NONE_TYPE_ID,
	TYPE_TYPE_ID,
	OPAQUE_TYPE_ID,
	BOOL_TYPE_ID,
	LONG_TYPE_ID,
	TUPLE_TYPE_ID,
	LIST_TYPE_ID,
	DICT_TYPE_ID,
	BYTES_TYPE_ID,
	UNICODE_TYPE_ID,
	CODE_TYPE_ID,
	FUNCTION_TYPE_ID,
	MODULE_TYPE_ID,
	BUILTIN_TYPE_ID,

	TYPE_ID_COUNT
};

struct PeerObject {
	PyObject_HEAD

	PeerObject();
	~PeerObject();

	int insert(PyObject *object, Key key) noexcept;
	void clear(PyObject *object) noexcept;
	std::pair<Key, bool> insert_or_clear_for_remote(PyObject *object) noexcept;
	Key key_for_remote(PyObject *object) noexcept;
	PyObject *object(Key key) noexcept;
	void touch(PyObject *object) noexcept;
	void set_references(const std::unordered_set<PyObject *> &referenced) noexcept;
	void object_freed(void *ptr) noexcept;

	std::vector<Key> freed;

private:
	struct State;

	PeerObject(const PeerObject &);
	void operator=(const PeerObject &);

	int insert(PyObject *object, Key key, unsigned int flags) noexcept;
	Key insert_new(PyObject *object, unsigned int flags) noexcept;
	Key key_for_remote(Key key) noexcept;

	std::map<void *, State> states;
	std::map<Key, PyObject *> objects;
	uint32_t next_object_id;
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
void peers_touch(PyObject *object) noexcept;

int opaque_type_init() noexcept;

int list_py_type_init() noexcept;

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
