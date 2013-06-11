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

#ifndef TAPY_OBJECTS_INTERNAL_HPP
#define TAPY_OBJECTS_INTERNAL_HPP

#include <tapy/objects/object.hpp>

namespace tapy {

class DictObject;
class ModuleObject;
class StrObject;
class TupleObject;

typedef Object *(*InternalFunc)(const TupleObject *args, DictObject *kwargs);

class InternalObject: public Object, public tap::ObjectBase<InternalObject> {
public:
	static void *UnmarshalAlloc(const void *buf, size_t bufsize);

	InternalObject(const ModuleObject *module, const StrObject *name, InternalFunc func) throw ():
		m_module(module),
		m_name(name),
		m_func(func)
	{
	}

	InternalObject(tap::Peer &peer, const void *buf, size_t bufsize);

	bool traverse(tap::VisitFunc func, void *arg) const;

	size_t marshal_size(tap::Peer &peer) const throw ();

	bool marshal(tap::Peer &peer, void *buf, size_t bufsize) const;

	virtual Object *call(const TupleObject *args, DictObject *kwargs);

	static TypeObject *type;

private:
	struct Portable;

	const ModuleObject *m_module;
	const StrObject *m_name;
	InternalFunc m_func;
};

} // namespace

#endif
