/*
 * Copyright (c) 2013, Timo Savola
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef TAP_TAP_HPP
#define TAP_TAP_HPP

#include <cassert>
#include <cstddef>
#include <cstring>
#include <memory>
#include <new>
#include <stdexcept>

#include "tap.h"

namespace tap {

template <typename T>
class Resource {
public:
	const T *c_ptr() const throw ()
	{
		return m_resource;
	}

	T *c_ptr() throw ()
	{
		return m_resource;
	}

protected:
	Resource(T *resource):
		m_resource(resource)
	{
		if (m_resource == NULL)
			throw std::bad_alloc();
	}

	T *m_resource;

private:
	Resource(const Resource &);
	void operator=(const Resource &);
};

class System: public Resource<TapSystem> {
public:
	explicit System(const char *name):
		Resource<TapSystem>(::tap_system_new(name))
	{
	}

	~System() throw ()
	{
		::tap_system_destroy(m_resource);
	}

	const char *name() const throw ()
	{
		return ::tap_system_name(m_resource);
	}

	const ::TapType *type(uint32_t type_id) const
	{
		auto type = ::tap_system_type(m_resource, type_id);
		if (type == NULL)
			throw std::runtime_error("unknown type");
		return type;
	}
};

class Instance: public Resource<TapInstance> {
public:
	explicit Instance(const System &system):
		Resource<TapInstance>(::tap_instance_new(system.c_ptr()))
	{
	}

	explicit Instance(const ::TapSystem *system):
		Resource<TapInstance>(::tap_instance_new(system))
	{
	}

	~Instance() throw ()
	{
		::tap_instance_destroy(m_resource);
	}
};

typedef ::TapKey Key;

class Peer: public Resource<TapPeer> {
public:
	Peer(Instance &instance, bool secondary):
		Resource<TapPeer>(::tap_peer_new(instance.c_ptr(), secondary)),
		m_own(true)
	{
	}

	Peer(::TapInstance *instance, bool secondary):
		Resource<TapPeer>(::tap_peer_new(instance, secondary)),
		m_own(true)
	{
	}

	explicit Peer(::TapPeer *resource):
		Resource<TapPeer>(resource),
		m_own(false)
	{
	}

	~Peer() throw ()
	{
		if (m_own)
			::tap_peer_destroy(m_resource);
	}

	Key key(const void *ptr)
	{
		Key key = 0;
		if (ptr) {
			key = ::tap_peer_key(m_resource, ptr);
			if (key == 0)
				throw std::range_error("peer object not found");
		}
		return key;
	}

	void *object(Key key)
	{
		void *ptr = NULL;
		if (key) {
			ptr = ::tap_peer_object(m_resource, key);
			if (ptr == NULL)
				throw std::range_error("peer key not found");
		}
		return ptr;
	}

private:
	const bool m_own;
};

typedef ::TapVisitFunc VisitFunc;

template <typename T>
struct TypeFunc;

class Type {
public:
	static const Type *Of(const void *object_ptr) throw ()
	{
		auto type_m_ptr = ::tap_object_type(object_ptr);
		auto type_ptr = reinterpret_cast<const char *> (type_m_ptr) -
		                offsetof(Type, m);
		return reinterpret_cast<const Type *> (type_ptr);
	}

	Type(uint32_t id, const char *name) throw ()
	{
		std::memset(&m, 0, sizeof (m));
		m.id = id;
		m.name = name;
	}

	template <typename T>
	void init(System &system, T *) throw ()
	{
		m.destroy = TypeFunc<T>::Destroy;
		m.traverse = TypeFunc<T>::Traverse;
		m.marshal_size = TypeFunc<T>::MarshalSize;
		m.marshal = TypeFunc<T>::Marshal;
		m.unmarshal_alloc = TypeFunc<T>::UnmarshalAlloc;
		m.unmarshal = TypeFunc<T>::Unmarshal;

		::tap_type_init(&m, system.c_ptr());
	}

	uint32_t id() const throw ()
	{
		return m.id;
	}

	const char *name() const throw ()
	{
		return m.name;
	}

	const ::TapType *c_ptr() const throw ()
	{
		return &m;
	}

protected:
	Type() throw ()
	{
	}

private:
	Type(const Type &);
	void operator=(const Type &);

	::TapType m;
};

template <typename T>
struct TypeFunc {
	friend class Type;

private:
	static void Destroy(const ::TapType *type, void *ptr) throw ()
	{
		reinterpret_cast<T *> (ptr)->~T();
	}

	static bool Traverse(const ::TapType *type, const void *ptr,
	                     VisitFunc func, void *arg) throw ()
	{
		try {
			return reinterpret_cast<const T *> (ptr)->traverse(func, arg);
		} catch (...) {
			return false;
		}
	}

	static size_t MarshalSize(const ::TapType *type, const void *ptr,
	                          ::TapPeer *peer) throw ()
	{
		Peer p(peer);
		return reinterpret_cast<const T *> (ptr)->marshal_size(p);
	}

	static bool Marshal(const ::TapType *type, const void *ptr,
	                    ::TapPeer *peer, void *buf, size_t size) throw ()
	{
		try {
			Peer p(peer);
			return reinterpret_cast<const T *> (ptr)->marshal(p, buf, size);
		} catch (...) {
			return false;
		}
	}

	static void *UnmarshalAlloc(const ::TapType *type, const void *data,
	                            size_t size) throw ()
	{
		try {
			return T::UnmarshalAlloc(data, size);
		} catch (...) {
			return NULL;
		}
	}

	static bool Unmarshal(const ::TapType *type, void *ptr, ::TapPeer *peer,
	                      const void *data, size_t size) throw ()
	{
		try {
			Peer p(peer);
			new (ptr) T(p, data, size);
			return true;
		} catch (...) {
			return false;
		}
	}
};

template <typename T>
T *CreateType(uint32_t id, const char *name, T *)
{
	return new T(id, name);
}

template <typename T>
void InitType(System &system, uint32_t id, const char *name)
{
	assert(T::type == NULL);
	auto t = CreateType(id, name, reinterpret_cast<decltype(T::type)> (NULL));
	// TODO: delete type on exception
	t->init(system, reinterpret_cast<T *> (NULL));
	T::type = t;
}

inline void *AllocObject(const Type *type, size_t size)
{
	void *ptr = ::tap_object_alloc(type->c_ptr(), size);
	if (ptr == NULL)
		throw std::bad_alloc();
	return ptr;
}

template <typename T, typename... Args>
T *NewObject(size_t size, Args... args)
{
	assert(size >= sizeof (T));
	void *ptr = AllocObject(T::type, size);
	new (ptr) T(args...);
	return reinterpret_cast<T *> (ptr);
}

inline const Type *ObjectType(const void *ptr) throw ()
{
	return Type::Of(ptr);
}

inline void ObjectModified(Instance &instance, void *ptr) throw ()
{
	::tap_object_modified(instance.c_ptr(), ptr);
}

template <typename T>
class ObjectBase {
public:
	template <typename... Args>
	static T *New(Args... args)
	{
		return NewObject<T>(sizeof (T), args...);
	}

#if 0
	void *operator new(size_t size, void *ptr) throw ()
	{
		return ptr;
	}

	void operator delete(void *) throw ()
	{
		assert(false);
	}

	void operator delete(void *, void *) throw ()
	{
	}
#endif

	bool traverse(VisitFunc func, void *arg) const throw ()
	{
		return true;
	}

#if 0
private:
	void *operator new(size_t size);
	void *operator new(size_t size, const std::nothrow_t &) throw ();
	void operator delete(void *, const std::nothrow_t &) throw ();
#endif
};

class Buffer: public Resource<TapBuffer> {
public:
	Buffer():
		Resource<TapBuffer>(::tap_buffer_new())
	{
	}

	~Buffer() throw ()
	{
		::tap_buffer_destroy(m_resource);
	}

	size_t size() const throw ()
	{
		return ::tap_buffer_size(m_resource);
	}

	const void *data() const throw ()
	{
		return ::tap_buffer_data(m_resource);
	}
};

inline void Marshal(::TapPeer *peer, ::TapBuffer *buffer, const void *ptr)
{
	if (!::tap_marshal(peer, buffer, ptr))
		throw std::bad_alloc();
}

inline void Marshal(Peer &peer, ::TapBuffer *buffer, const void *ptr)
{
	Marshal(peer.c_ptr(), buffer, ptr);
}

inline void Marshal(::TapPeer *peer, Buffer &buffer, const void *ptr)
{
	Marshal(peer, buffer.c_ptr(), ptr);
}

inline void Marshal(Peer &peer, Buffer &buffer, const void *ptr)
{
	Marshal(peer.c_ptr(), buffer.c_ptr(), ptr);
}

inline void *Unmarshal(::TapPeer *peer, const void *data, size_t size)
{
	void *ptr = ::tap_unmarshal(peer, data, size);
	if (ptr == NULL)
		throw std::runtime_error("unmarshal failed");
	return ptr;
}

inline void *Unmarshal(Peer &peer, const void *data, size_t size)
{
	return Unmarshal(peer.c_ptr(), data, size);
}

#if TAP_PORTABLE_BYTEORDER

template <typename T>
T Port(T x) throw ()
{
	return x;
}

#else

template <typename T, unsigned int N>
struct Porter;

template <typename T>
struct Porter<T, 1> {
	static T Port(T x) throw ()
	{
		return x;
	}
};

template <typename T>
struct Porter<T, 2> {
	static T Port(T x) throw ()
	{
		return __bswap_16(x);
	}
};

template <typename T>
struct Porter<T, 4> {
	static T Port(T x) throw ()
	{
		return __bswap_32(x);
	}
};

template <typename T>
struct Porter<T, 8> {
	static T Port(T x) throw ()
	{
		return __bswap_64(x);
	}
};

template <typename T>
T Port(T x) throw ()
{
	return Porter<T, sizeof (T)>::Port(x);
}

#endif

} // namespace

#endif
