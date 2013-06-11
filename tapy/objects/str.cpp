/*
 * Copyright (c) 2006, 2011-2013, Timo Savola
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

#include "str.hpp"

#include <cstring>
#include <functional>
#include <stdexcept>

#include <tapy/objects/bool.hpp>
#include <tapy/objects/int.hpp>

using namespace tap;

namespace tapy {

struct StrObject::Portable {
	static size_t Size(uint32_t size) throw ()
	{
		return sizeof (Portable) + size;
	}

	const char *data() const throw ()
	{
		return reinterpret_cast<const char *> (this + 1);
	}

	char *data() throw ()
	{
		return reinterpret_cast<char *> (this + 1);
	}

	uint32_t size;
} TAP_PACKED;

uint32_t StrObject::CalculateLength(const char *string, uint32_t size)
{
	const uint8_t *data = reinterpret_cast<const uint8_t *> (string);
	uint32_t length = 0;

	for (uint32_t i = 0; i < size; length++) {
		uint8_t byte = data[i++];

		if ((byte & 0x80) != 0) {
			uint32_t skip;
			if ((byte & 0xe0) == 0xc0)
				skip = 1;
			else if ((byte & 0xf0) == 0xe0)
				skip = 2;
			else if ((byte & 0xf8) == 0xf0)
				skip = 3;
			else
				throw std::runtime_error("invalid UTF-8 string");

			uint32_t next = i + skip;
			if (next > size)
				throw std::runtime_error("invalid UTF-8 string");

			for (; i < next; i++) {
				byte = data[i];

				if ((byte & 0xc0) != 0x80)
					throw std::runtime_error("invalid UTF-8 string");
			}
		}
	}

	return length;
}

void *StrObject::UnmarshalAlloc(const void *buf, size_t bufsize)
{
	if (bufsize < sizeof (Portable))
		throw std::out_of_range("unmarshal size");

	auto portable = reinterpret_cast<const Portable *> (buf);
	uint32_t size = Port(portable->size);

	if (bufsize != Portable::Size(size))
		throw std::out_of_range("unmarshal size");

	return AllocObject(type, ObjectSize(size));
}

StrObject *StrObject::New(const char *s)
{
	return New(s, std::strlen(s));
}

StrObject::StrObject(const char *data_, uint32_t size_) throw ():
	m_size(size_)
{
	std::memcpy(uninit_data(), data_, m_size);
	uninit_data()[m_size] = '\0';
	m_length = CalculateLength(data(), m_size);
}

StrObject::StrObject(Peer &peer, const void *buf, size_t)
{
	auto portable = reinterpret_cast<const Portable *> (buf);

	m_size = Port(portable->size);
	std::memcpy(uninit_data(), portable->data(), m_size);
	uninit_data()[m_size] = '\0';
	m_length = CalculateLength(data(), m_size);
}

size_t StrObject::marshal_size(Peer &peer) const throw ()
{
	return Portable::Size(m_size);
}

bool StrObject::marshal(Peer &peer, void *buf, size_t) const
{
	auto portable = reinterpret_cast<Portable *> (buf);

	portable->size = Port(m_size);
	std::memcpy(portable->data(), data(), m_size);

	return true;
}

StrObject *StrObject::repr() const
{
	std::string s = "'";
	s += data();
	s += "'";
	return StrObject::New(s);
}

StrObject *StrObject::str() const
{
	return const_cast<StrObject *> (this);
}

template <typename Predicate>
static BoolObject *Compare(const StrObject *lso, const Object *ro)
{
	auto rso = Require<StrObject>(ro);

	std::string ls(lso->data(), lso->length());
	std::string rs(rso->data(), rso->length());

	Predicate pred;

	return Bool(pred(ls, rs));
}

BoolObject *StrObject::lt(const Object *other) const
{
	if (this == other)
		return False();

	return Compare<std::less<std::string>>(this, other);
}

BoolObject *StrObject::le(const Object *other) const
{
	if (this == other)
		return True();

	return Compare<std::less_equal<std::string>>(this, other);
}

BoolObject *StrObject::eq(const Object *other) const
{
	if (this == other)
		return True();

	return Compare<std::equal_to<std::string>>(this, other);
}

BoolObject *StrObject::ne(const Object *other) const
{
	if (this == other)
		return False();

	return Compare<std::not_equal_to<std::string>>(this, other);
}

BoolObject *StrObject::gt(const Object *other) const
{
	if (this == other)
		return False();

	return Compare<std::greater<std::string>>(this, other);
}

BoolObject *StrObject::ge(const Object *other) const
{
	if (this == other)
		return True();

	return Compare<std::greater_equal<std::string>>(this, other);
}

IntObject *StrObject::hash() const
{
	std::hash<std::string> h;
	std::string s(data(), length());
	return IntObject::New(h(s));
}

BoolObject *StrObject::bool_() const
{
	return Bool(m_length);
}

IntObject *StrObject::len() const
{
	return IntObject::New(m_length);
}

Object *StrObject::add(const Object *other) const
{
	auto other_string = Require<StrObject>(other);

	std::string tmp(data(), size());
	tmp.append(other_string->data(), other_string->size());

	return StrObject::New(tmp);
}

TypeObject *StrObject::type;

} // namespace
