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

#include "function.hpp"

#include <memory>

#include <tapy/executor.hpp>
#include <tapy/objects/code.hpp>
#include <tapy/objects/dict.hpp>
#include <tapy/objects/str.hpp>
#include <tapy/objects/tuple.hpp>
#include <tapy/trace.hpp>

using namespace tap;

namespace tapy {

struct FunctionObject::Portable {
	Key code;
	Key name;
} TAP_PACKED;

void *FunctionObject::UnmarshalAlloc(const void *buf, size_t bufsize)
{
	if (bufsize != sizeof (Portable))
		throw std::out_of_range("unmarshal size");

	return AllocObject(type, sizeof (FunctionObject));
}

FunctionObject::FunctionObject(Peer &peer, const void *buf, size_t)
{
	m_name = Require<StrObject>(peer.object(Port(reinterpret_cast<const Portable *> (buf)->name)));
	m_code = Require<CodeObject>(peer.object(Port(reinterpret_cast<const Portable *> (buf)->code)));
}

bool FunctionObject::traverse(VisitFunc func, void *arg) const
{
	return func(m_name, arg) && func(m_code, arg);
}

size_t FunctionObject::marshal_size(Peer &peer) const throw ()
{
	return sizeof (Portable);
}

bool FunctionObject::marshal(Peer &peer, void *buf, size_t) const
{
	auto portable = reinterpret_cast<Portable *> (buf);

	portable->name = Port(peer.key(m_name));
	portable->code = Port(peer.key(m_code));

	return true;
}

Object *FunctionObject::call(const TupleObject *args, DictObject *kwargs)
{
	auto dict = DictObject::New();

	for (unsigned int i = 0; i < args->length(); i++)
		dict->set(m_code->varnames()->get(i), args->get(i));

	dict->copy_from(kwargs);

	Executor executor(m_code, dict);
	return executor.execute();
}

TypeObject *FunctionObject::type;

} // namespace
