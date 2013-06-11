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

#include "tuple.hpp"

#include <stdexcept>

#include <boost/format.hpp>

#include <tapy/objects/int.hpp>
#include <tapy/objects/str.hpp>

using namespace tap;

namespace tapy {

struct TupleObject::Portable {
	static size_t Size(uint32_t length) throw ()
	{
		return sizeof (Portable) + length * sizeof (uint32_t);
	}

	const uint32_t *items() const throw ()
	{
		return reinterpret_cast<const uint32_t *> (this + 1);
	}

	uint32_t *items() throw ()
	{
		return reinterpret_cast<uint32_t *> (this + 1);
	}

	uint32_t length;
} TAP_PACKED;

void *TupleObject::UnmarshalAlloc(const void *buf, size_t bufsize)
{
	if (bufsize < sizeof (Portable))
		throw std::out_of_range("unmarshal size");

	auto portable = reinterpret_cast<const Portable *> (buf);
	uint32_t length = Port(portable->length);

	if (bufsize != Portable::Size(length))
		throw std::out_of_range("unmarshal size");

	return AllocObject(type, ObjectSize(length));
}

TupleObject::TupleObject(Peer &peer, const void *buf, size_t)
{
	auto portable = reinterpret_cast<const Portable *> (buf);

	m_length = Port(portable->length);

	for (uint32_t i = 0; i < m_length; i++)
		items()[i] = Require<Object>(peer.object(Port(portable->items()[i])));
}

bool TupleObject::traverse(VisitFunc func, void *arg) const
{
	for (uint32_t i = 0; i < m_length; i++)
		if (items()[i] && !func(items()[i], arg))
			return false;

	return true;
}

size_t TupleObject::marshal_size(Peer &peer) const throw ()
{
	return Portable::Size(m_length);
}

bool TupleObject::marshal(Peer &peer, void *buf, size_t) const
{
	auto portable = reinterpret_cast<Portable *> (buf);

	portable->length = Port(m_length);

	for (uint32_t i = 0; i < m_length; i++)
		portable->items()[i] = Port(peer.key(items()[i]));

	return true;
}

Object *TupleObject::get(uint32_t index) const throw ()
{
	if (index >= length())
		throw std::range_error("tuple index out of range");

	assert(items()[index]);
	return items()[index];
}

StrObject *TupleObject::repr() const
{
	std::string s = "(";
	for (uint32_t i = 0; i < m_length; i++) {
		s += (boost::format("<%s object at %p>")
		      % ObjectType(items()[i])->name() % items()[i]).str();
		if (i < m_length - 1)
			s += ", ";
	}
	s += ")";
	return StrObject::New(s);
}

IntObject *TupleObject::len() const
{
	return IntObject::New(m_length);
}

TypeObject *TupleObject::type;

} // namespace
