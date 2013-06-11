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

#include "type.hpp"

#include <stdexcept>

#include <boost/format.hpp>

#include <tapy/objects/str.hpp>
#include <tapy/system.hpp>

using namespace tap;

namespace tapy {

struct TypeObject::Portable {
	uint32_t id;
} TAP_PACKED;

void *TypeObject::UnmarshalAlloc(const void *buf, size_t bufsize)
{
	if (bufsize != sizeof (Portable))
		throw std::out_of_range("unmarshal size");

	auto tap_type = GetSystem().type(Port(
			reinterpret_cast<const Portable *> (buf)->id));

	return const_cast<TypeObject *> (
			reinterpret_cast<const TypeObject *> (tap_type));
}

size_t TypeObject::marshal_size(Peer &peer) const throw ()
{
	return sizeof (Portable);
}

bool TypeObject::marshal(Peer &peer, void *buf, size_t) const
{
	reinterpret_cast<Portable *> (buf)->id = Port(Type::id());
	return true;
}

StrObject *TypeObject::repr() const
{
	return StrObject::New((boost::format("<type '%s'>")
	                       % name() % this).str());
}

TypeObject *TypeObject::type;

} // namespace
