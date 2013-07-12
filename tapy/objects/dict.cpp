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

#include "dict.hpp"

#include <stdexcept>

#include <boost/format.hpp>

#include <tapy/objects/int.hpp>
#include <tapy/objects/type.hpp>

using namespace tap;

namespace tapy {

struct DictObject::Portable {
	struct Item {
		Key name;
		Key value;
	} TAP_PACKED;

	static size_t Size(uint32_t length) throw ()
	{
		return length * sizeof (Item);
	}

	static bool Check(size_t bufsize) throw ()
	{
		return (bufsize % sizeof (Item)) == 0;
	}

	static size_t Length(size_t bufsize) throw ()
	{
		return bufsize / sizeof (Item);
	}

	const Item *items() const throw ()
	{
		return reinterpret_cast<const Item *> (this);
	}

	Item *items() throw ()
	{
		return reinterpret_cast<Item *> (this);
	}
};

size_t DictObject::Hash::operator()(const Object *object) const
{
	return object->hash()->value();
}

bool DictObject::KeyEqual::operator()(const Object *a, const Object *b) const
{
	return a->eq(b);
}

void *DictObject::UnmarshalAlloc(const void *buf, size_t bufsize)
{
	if (!Portable::Check(bufsize))
		throw std::out_of_range("unmarshal size");

	return AllocObject(type, sizeof (DictObject));
}

DictObject::DictObject(Peer &peer, const void *buf, size_t bufsize)
{
	auto portable = reinterpret_cast<const Portable *> (buf);

	for (size_t i = 0; i < Portable::Length(bufsize); i++) {
		auto &item = portable->items()[i];

		m_map[Require<Object>(peer.object(Port(item.name)))]
				= Require<Object>(peer.object(Port(item.value)));
	}
}

bool DictObject::traverse(VisitFunc func, void *arg) const
{
	for (auto i = m_map.begin(); i != m_map.end(); ++i) {
		if (i->first && !func(i->first, arg))
			return false;

		if (i->second && !func(i->second, arg))
			return false;
	}

	return true;
}

size_t DictObject::marshal_size(Peer &peer) const throw ()
{
	return Portable::Size(m_map.size());
}

bool DictObject::marshal(Peer &peer, void *buf, size_t) const
{
	auto portable = reinterpret_cast<Portable *> (buf);
	size_t n = 0;

	for (auto i = m_map.begin(); i != m_map.end(); ++i) {
		auto &item = portable->items()[n++];
		item.name = Port(peer.key(i->first));
		item.value = Port(peer.key(i->second));
	}

	return true;
}

void DictObject::set(const Object *name, Object *value)
{
	m_map[name] = value;
	modified();
}

Object *DictObject::get(const Object *name) const
{
	auto i = m_map.find(name);
	if (i == m_map.end())
		throw AttributeError("key not found");
	return i->second;
}

void DictObject::copy_from(const DictObject *other_dict)
{
	auto &other_map = other_dict->m_map;

	for (auto i = other_map.begin(); i != other_map.end(); ++i)
		m_map[i->first] = i->second;
}

IntObject *DictObject::len() const
{
	return IntObject::New(m_map.size());
}

TypeObject *DictObject::type;

} // namespace
