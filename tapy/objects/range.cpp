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

#include "range.hpp"

#include <stdexcept>

#include <tapy/objects/int.hpp>
#include <tapy/objects/type.hpp>

using namespace tap;

namespace tapy {

struct RangeObject::Portable {
	int64_t value;
	int64_t stop;
	int64_t step;
} TAP_PACKED;

void *RangeObject::UnmarshalAlloc(const void *buf, size_t bufsize)
{
	if (bufsize != sizeof (Portable))
		throw std::out_of_range("unmarshal size");

	return AllocObject(type, sizeof (Portable));
}

RangeObject::RangeObject(int64_t start, int64_t stop, int64_t step) throw ():
	m_value(start),
	m_stop(stop),
	m_step(step)
{
	if (step == 0)
		throw std::runtime_error("bad range step");
}

RangeObject::RangeObject(Peer &peer, const void *buf, size_t bufsize)
{
	auto portable = reinterpret_cast<const Portable *> (buf);

	m_value = Port(portable->value);
	m_stop = Port(portable->stop);
	m_step = Port(portable->step);

	if (m_step == 0)
		throw std::runtime_error("bad unmarshal range step");
}

size_t RangeObject::marshal_size(Peer &peer) const throw ()
{
	return sizeof (Portable);
}

bool RangeObject::marshal(Peer &peer, void *buf, size_t) const
{
	auto portable = reinterpret_cast<Portable *> (buf);

	portable->value = Port(m_value);
	portable->stop = Port(m_stop);
	portable->step = Port(m_step);

	return true;
}

Object *RangeObject::iter()
{
	return this;
}

Object *RangeObject::next()
{
	if (m_step > 0) {
		if (m_value >= m_stop)
			throw StopIteration();
	} else {
		if (m_value <= m_stop)
			throw StopIteration();
	}

	auto result = IntObject::New(m_value);
	m_value += m_step;

	// TODO: handle wrap-around problems etc.

	return result;
}

TypeObject *RangeObject::type;

} // namespace
