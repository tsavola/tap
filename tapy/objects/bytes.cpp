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

#include "bytes.hpp"

#include <cstring>
#include <stdexcept>

#include <boost/format.hpp>

#include <tapy/objects/int.hpp>
#include <tapy/objects/str.hpp>

using namespace tap;

namespace tapy {

struct BytesObject::Portable {
	static size_t Size(uint32_t length) throw ()
	{
		return sizeof (Portable) + length;
	}

	const uint8_t *data() const throw ()
	{
		return reinterpret_cast<const uint8_t *> (this + 1);
	}

	uint8_t *data() throw ()
	{
		return reinterpret_cast<uint8_t *> (this + 1);
	}

	uint32_t length;
} TAP_PACKED;

void *BytesObject::UnmarshalAlloc(const void *buf, size_t bufsize)
{
	if (bufsize < sizeof (Portable))
		throw std::out_of_range("unmarshal size");

	auto portable = reinterpret_cast<const Portable *> (buf);
	uint32_t length = Port(portable->length);

	if (bufsize != Portable::Size(length))
		throw std::out_of_range("unmarshal size");

	return AllocObject(type, ObjectSize(length));
}

BytesObject::BytesObject(const uint8_t *data_, uint32_t length_) throw ():
	m_length(length_)
{
	std::memcpy(uninit_data(), data_, length_);
}

BytesObject::BytesObject(Peer &peer, const void *buf, size_t)
{
	auto portable = reinterpret_cast<const Portable *> (buf);

	m_length = Port(portable->length);
	std::memcpy(uninit_data(), portable->data(), m_length);
}

size_t BytesObject::marshal_size(Peer &peer) const throw ()
{
	return Portable::Size(m_length);
}

bool BytesObject::marshal(Peer &peer, void *buf, size_t) const
{
	auto portable = reinterpret_cast<Portable *> (buf);

	portable->length = Port(m_length);
	std::memcpy(portable->data(), data(), m_length);

	return true;
}

StrObject *BytesObject::repr() const
{
	std::string s = "b'";
	for (uint32_t i = 0; i < m_length; i++)
		s += (boost::format("\\x%02x") % int(data()[i])).str();
	s += "'";
	return StrObject::New(s);
}

BytesObject *BytesObject::bytes() const
{
	return const_cast<BytesObject *> (this);
}

IntObject *BytesObject::len() const
{
	return IntObject::New(m_length);
}

TypeObject *BytesObject::type;

} // namespace
