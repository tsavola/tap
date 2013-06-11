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

#ifndef TAPY_OBJECTS_BYTES_HPP
#define TAPY_OBJECTS_BYTES_HPP

#include <tapy/objects/object.hpp>
#include <tapy/objects/type.hpp>

namespace tapy {

class BytesObject: public Object, public tap::ObjectBase<BytesObject> {
public:
	static BytesObject *New(const uint8_t *data, uint32_t length)
	{
		return tap::NewObject<BytesObject>(ObjectSize(length), data, length);
	}

	static void *UnmarshalAlloc(const void *buf, size_t bufsize);

	BytesObject(const uint8_t *data_, uint32_t length_) throw ();

	BytesObject(tap::Peer &peer, const void *buf, size_t bufsize);

	size_t marshal_size(tap::Peer &peer) const throw ();

	bool marshal(tap::Peer &peer, void *buf, size_t bufsize) const;

	uint32_t length() const throw ()
	{
		return m_length;
	}

	const uint8_t *data() const throw ()
	{
		return reinterpret_cast<const uint8_t *> (this + 1);
	}

	virtual StrObject *repr() const;

	virtual BytesObject *bytes() const;

	virtual IntObject *len() const;

	static TypeObject *type;

private:
	struct Portable;

	static size_t ObjectSize(uint32_t length) throw ()
	{
		return sizeof (BytesObject) + length;
	}

	uint8_t *uninit_data() throw ()
	{
		return reinterpret_cast<uint8_t *> (this + 1);
	}

	uint32_t m_length;
};

} // namespace

#endif
