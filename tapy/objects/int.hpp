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

#ifndef TAPY_OBJECTS_INT_HPP
#define TAPY_OBJECTS_INT_HPP

#include <tapy/objects/number.hpp>

namespace tapy {

class Int: public Number {
public:
	int64_t value() const throw ()
	{
		return m_value;
	}

protected:
	Int() throw ()
	{
	}

	Int(int64_t value) throw ():
		m_value(value)
	{
	}

	int64_t m_value;
};

class IntObject: public Int, public tap::ObjectBase<IntObject> {
public:
	static void *UnmarshalAlloc(const void *buf, size_t bufsize);

	explicit IntObject(int64_t value) throw ():
		Int(value)
	{
	}

	IntObject(tap::Peer &peer, const void *buf, size_t bufsize);

	size_t marshal_size(tap::Peer &peer) const throw ();

	bool marshal(tap::Peer &peer, void *buf, size_t bufsize) const;

	virtual StrObject *repr() const;

	virtual BoolObject *bool_() const;

	static TypeObject *type;

private:
	struct Portable;
};

} // namespace

#endif
