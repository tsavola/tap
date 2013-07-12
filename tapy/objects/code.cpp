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

#include "code.hpp"

#include <stdexcept>

#include <boost/utility.hpp>

#include <tapy/objects/bool.hpp>
#include <tapy/objects/bytes.hpp>
#include <tapy/objects/float.hpp>
#include <tapy/objects/int.hpp>
#include <tapy/objects/none.hpp>
#include <tapy/objects/str.hpp>
#include <tapy/objects/tuple.hpp>

using namespace tap;

namespace tapy {

// TODO: merge Loader, CodeLoader and CodeLoaderState

template <typename State>
class Loader: boost::noncopyable {
public:
	Loader(State &state) throw ():
		m_state(state)
	{
	}

	bool empty() const
	{
		return m_state.position() >= m_state.size();
	}

	template <typename T>
	T load()
	{
		return Port(*load_raw<T>(1));
	}

	const char *load_chars(size_t count)
	{
		return load_raw<char>(count);
	}

	template <typename T>
	const T *load_raw(size_t count)
	{
		auto pos = m_state.position();

		if (pos + sizeof (T) * count > m_state.size())
			throw std::runtime_error("code loader ran out of data");

		auto data = m_state.data() + pos;
		m_state.advance(sizeof (T) * count);

		return reinterpret_cast<const T *> (data);
	}

private:
	State &m_state;
};

class CodeLoaderState: boost::noncopyable {
public:
	CodeLoaderState(const uint8_t *data, size_t size):
		m_data(data),
		m_size(size),
		m_position(0)
	{
	}

	const uint8_t *data() const
	{
		return m_data;
	}

	size_t size() const
	{
		return m_size;
	}

	size_t position() const
	{
		return m_position;
	}

	void advance(size_t offset)
	{
		m_position += offset;
	}

private:
	const uint8_t *const m_data;
	const size_t m_size;
	size_t m_position;
};

class CodeLoader: Loader<CodeLoaderState> {
public:
	explicit CodeLoader(CodeLoaderState &state):
		Loader<CodeLoaderState>(state)
	{
	}

	template <typename T>
	T *load_next()
	{
		return Require<T>(load_next());
	}

	Object *load_next()
	{
		switch (load<char>()) {
		case 'N': return None();
		case 'F': return False();
		case 'T': return True();
		case '(': return load_tuple();
		case 'c': return load_code();
		case 'g': return load_float();
		case 'i': return load_int();
		case 's': return load_bytes();
		case 'u': return load_str();
		default: throw std::runtime_error("encountered unknown type while loading code");
		}
	}

private:
	IntObject *load_int()
	{
		return IntObject::New(load<int32_t>());
	}

	FloatObject *load_float()
	{
		union {
			uint64_t raw;
			double value;
		} pun;

		pun.raw = *load_raw<uint64_t>(1);
		return FloatObject::New(pun.value);
	}

	BytesObject *load_bytes()
	{
		auto length = load<uint32_t>();
		return BytesObject::New(load_raw<uint8_t>(length), length);
	}

	StrObject *load_str()
	{
		auto size = load<uint32_t>();
		return StrObject::New(load_chars(size), size);
	}

	TupleObject *load_tuple()
	{
		auto length = load<uint32_t>();
		auto tuple = TupleObject::NewUninit(length);

		for (uint32_t i = 0; i < length; i++)
			tuple->init(i, load_next());

		return tuple;
	}

	CodeObject *load_code()
	{
		/* argcount = */ load<uint32_t>();
		/* kwonlyargcount = */ load<uint32_t>();
		/* nlocals = */ load<uint32_t>();
		auto stacksize = load<uint32_t>();
		/* flags = */ load<uint32_t>();
		auto code = load_next<BytesObject>();
		auto consts = load_next<TupleObject>();
		auto names = load_next<TupleObject>();
		auto varnames = load_next<TupleObject>();
		/* freevars = */ load_next();
		/* cellvars = */ load_next();
		/* filename = */ load_next();
		/* name = */ load_next();
		/* firstlineno = */ load<uint32_t>();
		/* lnotab = */ load_next();

		return CodeObject::New(stacksize, code, consts, names, varnames);
	}
};

struct CodeObject::Portable {
	uint32_t stacksize;
	Key bytecode;
	Key consts;
	Key names;
	Key varnames;
} TAP_PACKED;

CodeObject *CodeObject::Load(const void *void_data, size_t size)
{
	auto byte_data = reinterpret_cast<const uint8_t *> (void_data);

	CodeLoaderState state(byte_data, size);
	CodeLoader loader(state);

	return loader.load_next<CodeObject>();
}

void *CodeObject::UnmarshalAlloc(const void *buf, size_t bufsize)
{
	if (bufsize != sizeof (Portable))
		throw std::out_of_range("unmarshal size");

	return AllocObject(type, sizeof (CodeObject));
}

CodeObject::CodeObject(Peer &peer, const void *buf, size_t)
{
	auto portable = reinterpret_cast<const Portable *> (buf);

	m_stacksize = Port(portable->stacksize);
	m_bytecode = Require<BytesObject>(peer.object(Port(portable->bytecode)));
	m_consts = Require<TupleObject>(peer.object(Port(portable->consts)));
	m_names = Require<TupleObject>(peer.object(Port(portable->names)));
	m_varnames = Require<TupleObject>(peer.object(Port(portable->varnames)));
}

bool CodeObject::traverse(VisitFunc func, void *arg) const
{
	return func(m_bytecode, arg) &&
	       func(m_consts, arg) &&
	       func(m_names, arg) &&
	       func(m_varnames, arg);
}

size_t CodeObject::marshal_size(Peer &peer) const throw ()
{
	return sizeof (Portable);
}

bool CodeObject::marshal(Peer &peer, void *buf, size_t) const
{
	auto portable = reinterpret_cast<Portable *> (buf);

	portable->stacksize = Port(m_stacksize);
	portable->bytecode = Port(peer.key(m_bytecode));
	portable->consts = Port(peer.key(m_consts));
	portable->names = Port(peer.key(m_names));
	portable->varnames = Port(peer.key(m_varnames));

	return true;
}

TypeObject *CodeObject::type;

} // namespace
