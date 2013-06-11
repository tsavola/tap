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

#ifndef TAPY_OBJECTS_CODE_HPP
#define TAPY_OBJECTS_CODE_HPP

#include <tapy/objects/object.hpp>

namespace tapy {

class BytesObject;
class TupleObject;

class CodeObject: public Object, public tap::ObjectBase<CodeObject> {
public:
	static CodeObject *Load(const void *data, size_t size);

	static void *UnmarshalAlloc(const void *buf, size_t bufsize);

	CodeObject(uint32_t stacksize, BytesObject *bytecode, TupleObject *consts,
	           TupleObject *names, TupleObject *varnames) throw ():
		m_stacksize(stacksize),
		m_bytecode(bytecode),
		m_consts(consts),
		m_names(names),
		m_varnames(varnames)
	{
	}

	CodeObject(tap::Peer &peer, const void *buf, size_t bufsize);

	bool traverse(tap::VisitFunc func, void *arg) const;

	size_t marshal_size(tap::Peer &peer) const throw ();

	bool marshal(tap::Peer &peer, void *buf, size_t bufsize) const;

	uint32_t stacksize() const throw ()
	{
		return m_stacksize;
	}

	BytesObject *bytecode() const throw ()
	{
		return m_bytecode;
	}

	TupleObject *consts() const throw ()
	{
		return m_consts;
	}

	TupleObject *names() const throw ()
	{
		return m_names;
	}

	TupleObject *varnames() const throw ()
	{
		return m_varnames;
	}

	static TypeObject *type;

private:
	struct Portable;

	uint32_t m_stacksize;
	BytesObject *m_bytecode;
	TupleObject *m_consts;
	TupleObject *m_names;
	TupleObject *m_varnames;
};

} // namespace

#endif
