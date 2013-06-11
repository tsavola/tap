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

#include "internal.hpp"

#include <tapy/objects/module.hpp>
#include <tapy/objects/str.hpp>
#include <tapy/objects/type.hpp>

using namespace tap;

namespace tapy {

struct InternalObject::Portable {
	uint32_t module;
	uint32_t name;
} TAP_PACKED;

void *InternalObject::UnmarshalAlloc(const void *buf, size_t bufsize)
{
	if (bufsize != sizeof (Portable))
		throw std::out_of_range("unmarshal size");

	return AllocObject(type, sizeof (InternalObject));
}

InternalObject::InternalObject(Peer &peer, const void *buf, size_t)
{
	auto portable = reinterpret_cast<const Portable *> (buf);

	m_module = Require<ModuleObject>(peer.object(Port(portable->module)));
	m_name = Require<StrObject>(peer.object(Port(portable->name)));
}

bool InternalObject::traverse(VisitFunc func, void *arg) const
{
	return func(m_module, arg) && func(m_name, arg);
}

size_t InternalObject::marshal_size(Peer &peer) const throw ()
{
	return sizeof (Portable);
}

bool InternalObject::marshal(Peer &peer, void *buf, size_t) const
{
	auto portable = reinterpret_cast<Portable *> (buf);

	portable->module = Port(peer.key(m_module));
	portable->name = Port(peer.key(m_name));

	return true;
}

Object *InternalObject::call(const TupleObject *args, DictObject *kwargs)
{
	return m_func(args, kwargs);
}

TypeObject *InternalObject::type;

} // namespace
