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

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <set>
#include <stdexcept>

#include <tap.hpp>

#include "test.h"

using namespace tap;

namespace test {

class Long: public ObjectBase<Long> {
	struct Marshaled {
		int64_t portable_value;
	} TAP_PACKED;

public:
	explicit Long(int64_t value):
		value(value)
	{
	}

	static void *UnmarshalAlloc(const void *data, size_t size)
	{
		if (size != sizeof (Marshaled))
			throw std::out_of_range("unmarshal size");

		return AllocObject(type, sizeof (Marshaled));
	}

	Long(Peer &peer, const void *data, size_t)
	{
		value = Port(reinterpret_cast<const Marshaled *> (data)->portable_value);
	}

	size_t marshal_size(Peer &peer) const throw ()
	{
		return sizeof (Marshaled);
	}

	bool marshal(Peer &peer, void *buf, size_t size) const
	{
		reinterpret_cast<Marshaled *> (buf)->portable_value = Port(value);
		return true;
	}

	static Type *type;

	int64_t value;
};

Type *Long::type;

/* Tuple */

class Tuple: public ObjectBase<Tuple> {
	struct Marshaled {
		static size_t Size(uint32_t length) throw ()
		{
			return sizeof (Marshaled) + length * sizeof (uint32_t);
		}

		uint32_t portable_length;
		uint32_t portable_item_keys[0];
	} TAP_PACKED;

public:
	static Tuple *New(uint32_t length)
	{
		return NewObject<Tuple>(ObjectSize(length), length);
	}

	static void *UnmarshalAlloc(const void *data, size_t size)
	{
		if (size < sizeof (Marshaled))
			throw std::out_of_range("unmarshal size");

		auto marshaled = reinterpret_cast<const Marshaled *> (data);
		uint32_t length = Port(marshaled->portable_length);

		if (size != Marshaled::Size(length))
			throw std::out_of_range("unmarshal size");

		return AllocObject(type, ObjectSize(length));
	}

	explicit Tuple(uint32_t length):
		m_length(length)
	{
	}

	Tuple(Peer &peer, const void *data, size_t size)
	{
		auto marshaled = reinterpret_cast<const Marshaled *> (data);

		m_length = Port(marshaled->portable_length);

		for (uint32_t i = 0; i < m_length; i++)
			m_items[i] = peer.object(Port(marshaled->portable_item_keys[i]));
	}

	bool traverse(VisitFunc func, void *arg) const
	{
		for (uint32_t i = 0; i < m_length; i++)
			if (m_items[i] && !func(m_items[i], arg))
				return false;

		return true;
	}

	size_t marshal_size(Peer &peer) const throw ()
	{
		return Marshaled::Size(m_length);
	}

	bool marshal(Peer &peer, void *buf, size_t size) const
	{
		auto marshaled = reinterpret_cast<Marshaled *> (buf);

		marshaled->portable_length = Port(m_length);

		for (uint32_t i = 0; i < m_length; i++)
			marshaled->portable_item_keys[i] = Port(peer.key(m_items[i]));

		return true;
	}

	uint32_t length() const throw ()
	{
		return m_length;
	}

	void set_item(Instance &instance, uint32_t i, void *ptr) throw ()
	{
		assert(i < m_length);
		m_items[i] = ptr;
		ObjectModified(instance, this);
	}

	void *get_item(uint32_t i) const throw ()
	{
		assert(i < m_length);
		return m_items[i];
	}

	static Type *type;

private:
	static size_t ObjectSize(uint32_t length) throw ()
	{
		return sizeof (Tuple) + length * sizeof (void *);
	}

	Tuple(const Tuple &);
	void operator=(const Tuple &);

	uint32_t m_length;
	void *m_items[0];
};

Type *Tuple::type;

/* Visitor */

static bool Visit(const void *ptr, void *arg) throw ()
{
	try {
		std::set<const void *> *refs = reinterpret_cast<std::set<const void *> *> (arg);

		if (refs->find(ptr) != refs->end())
			return true;

		refs->insert(ptr);

		auto type = ObjectType(ptr);

		std::printf("%s object at %p\n", type->name(), ptr);

		return type->c_ptr()->traverse(type->c_ptr(), ptr, Visit, arg);
	} catch (...) {
		return false;
	}
}

static void Main()
{
	System system(TEST_SYSTEM_NAME);
	InitType<Long>(system, TEST_LONG_TYPE_ID, "long");
	InitType<Tuple>(system, TEST_TUPLE_TYPE_ID, "tuple");

	size_t bufsize = 0;
	void *bufdata = NULL;

	{
		Instance instance(system);

		{
			Long *long1 = Long::New(300);

			Long *long2 = Long::New(400);

			Tuple *tuple1 = Tuple::New(2);
			tuple1->set_item(instance, 0, long1);
			tuple1->set_item(instance, 1, long2);

			Tuple *tuple2 = Tuple::New(3);
			tuple2->set_item(instance, 0, long2);

			Tuple *root = Tuple::New(2);
			root->set_item(instance, 0, tuple1);
			root->set_item(instance, 1, tuple2);

			{
				std::set<const void *> refs;
				Visit(root, &refs);
				std::printf("%lu reachable objects\n", refs.size());
			}

			{
				Peer peer(instance, false);
				Buffer buffer;

				Marshal(peer, buffer, root);

				for (size_t i = 0; i < buffer.size(); ++i)
					std::printf("%02x%c",
					            ((const uint8_t *) buffer.data())[i], (i % 16) < 15 ? ' ' : '\n');

				std::printf("\n");

				bufsize = buffer.size();
				bufdata = new char[bufsize];
				std::memcpy(bufdata, buffer.data(), bufsize);
			}
		}
	}

	std::printf("--\n");

	{
		Instance instance(system);
		Peer peer(instance, true);

		void *root = Unmarshal(peer, bufdata, bufsize);

		reinterpret_cast<Tuple *> (root)->set_item(instance, 1, root);

		{
			std::set<const void *> refs;
			Visit(root, &refs);
			std::printf("%lu reachable objects\n", refs.size());
		}

		Buffer buffer;

		Marshal(peer, buffer, root);

		for (size_t i = 0; i < buffer.size(); ++i)
			std::printf("%02x%c", ((const uint8_t *) buffer.data())[i], (i % 16) < 15 ? ' ' : '\n');

		std::printf("\n");
	}
}

} // namespace

int main(int argc, char **argv)
{
	test::Main();
	return 0;
}
