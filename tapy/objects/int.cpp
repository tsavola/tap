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

#include "int.hpp"

#include <stdexcept>

#include <boost/format.hpp>

#include <tapy/objects/bool.hpp>
#include <tapy/objects/str.hpp>

using namespace tap;

namespace tapy {

struct IntObject::Portable {
	int64_t value;
} TAP_PACKED;

void *IntObject::UnmarshalAlloc(const void *buf, size_t bufsize)
{
	if (bufsize != sizeof (Portable))
		throw std::out_of_range("unmarshal size");

	return AllocObject(type, sizeof (IntObject));
}

IntObject::IntObject(Peer &peer, const void *buf, size_t)
{
	m_value = Port(reinterpret_cast<const Portable *> (buf)->value);
}

size_t IntObject::marshal_size(Peer &peer) const throw ()
{
	return sizeof (Portable);
}

bool IntObject::marshal(Peer &peer, void *buf, size_t) const
{
	reinterpret_cast<Portable *> (buf)->value = Port(m_value);
	return true;
}

StrObject *IntObject::repr() const
{
	return StrObject::New((boost::format("%1%") % m_value).str());
}

BoolObject *IntObject::bool_() const
{
	return Bool(m_value);
}

TypeObject *IntObject::type;

} // namespace
