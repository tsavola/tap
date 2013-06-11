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

#include "module.hpp"

#include <stdexcept>

#include <tapy/objects/dict.hpp>
#include <tapy/objects/internal.hpp>
#include <tapy/objects/str.hpp>
#include <tapy/objects/type.hpp>

using namespace tap;

namespace tapy {

struct ModuleObject::Portable {
	uint32_t name;
	uint32_t dict;
} TAP_PACKED;

void *ModuleObject::UnmarshalAlloc(const void *buf, size_t bufsize)
{
	if (bufsize != sizeof (Portable))
		throw std::out_of_range("unmarshal size");

	return AllocObject(type, sizeof (Portable));
}

ModuleObject::ModuleObject(const char *name) throw ():
	m_name(StrObject::New(name)),
	m_dict(DictObject::New())
{
}

ModuleObject::ModuleObject(Peer &peer, const void *buf, size_t bufsize)
{
	auto portable = reinterpret_cast<const Portable *> (buf);

	m_name = Require<StrObject>(peer.object(Port(portable->name)));
	m_dict = Require<DictObject>(peer.object(Port(portable->dict)));
}

bool ModuleObject::traverse(VisitFunc func, void *arg) const
{
	return func(m_name, arg) && func(m_dict, arg);
}

size_t ModuleObject::marshal_size(Peer &peer) const throw ()
{
	return sizeof (Portable);
}

bool ModuleObject::marshal(Peer &peer, void *buf, size_t) const
{
	auto portable = reinterpret_cast<Portable *> (buf);

	portable->name = Port(peer.key(m_name));
	portable->dict = Port(peer.key(m_dict));

	return true;
}

void ModuleObject::init(const char *name, Object *value)
{
	m_dict->set(StrObject::New(name), value);
}

void ModuleObject::init(const char *name, InternalFunc func)
{
	auto key = StrObject::New(name);
	m_dict->set(key, InternalObject::New(this, key, func));
}

Object *ModuleObject::getattribute(const Object *name)
{
	return m_dict->get(name);
}

TypeObject *ModuleObject::type;

} // namespace
