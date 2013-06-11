/*
 * Copyright (c) 2011-2013, Timo Savola
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

#ifndef TAPY_OBJECTS_TUPLE_HPP
#define TAPY_OBJECTS_TUPLE_HPP

#include <cassert>

#include <tapy/objects/object.hpp>
#include <tapy/objects/type.hpp>

namespace tapy {

class TupleObject: public Object, public tap::ObjectBase<TupleObject> {
public:
	static TupleObject *New()
	{
		return NewUninit(0);
	}

	template <typename Item, typename... Tail>
	static TupleObject *New(Item first_item, Tail... other_items)
	{
		TupleObject *object = NewUninit(1 + sizeof...(Tail));
		object->init_items(0, first_item, other_items...);
		return object;
	}

	static TupleObject *NewUninit(uint32_t length)
	{
		return tap::NewObject<TupleObject>(ObjectSize(length), length);
	}

	static void *UnmarshalAlloc(const void *buf, size_t bufsize);

	explicit TupleObject(uint32_t length) throw ():
		m_length(length)
	{
	}

	TupleObject(tap::Peer &peer, const void *buf, size_t bufsize);

	bool traverse(tap::VisitFunc func, void *arg) const;

	size_t marshal_size(tap::Peer &peer) const throw ();

	bool marshal(tap::Peer &peer, void *buf, size_t bufsize) const;

	uint32_t length() const throw ()
	{
		return m_length;
	}

	void init(uint32_t index, Object *ptr) throw ()
	{
		assert(index < length());
		items()[index] = ptr;
	}

	Object *get(uint32_t index) const throw ();

	virtual StrObject *repr() const;

	virtual IntObject *len() const;

	static TypeObject *type;

private:
	struct Portable;

	static size_t ObjectSize(uint32_t length) throw ()
	{
		return sizeof (TupleObject) + length * sizeof (Object *);
	}

	Object *const *items() const throw ()
	{
		return reinterpret_cast<Object *const *> (this + 1);
	}

	Object **items() throw ()
	{
		return reinterpret_cast<Object **> (this + 1);
	}

	template <typename Item, typename... Tail>
	void init_items(uint32_t first_index, Item first_item, Tail... other_items)
	{
		init(first_index, first_item);
		init_items(first_index + 1, other_items...);
	}

	void init_items(unsigned int index)
	{
	}

	uint32_t m_length;
};

} // namespace

#endif
