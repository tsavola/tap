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

#include "object.hpp"

#include <cstdint>

#include <boost/format.hpp>

#include <tapy/context.hpp>
#include <tapy/objects/bool.hpp>
#include <tapy/objects/bytes.hpp>
#include <tapy/objects/int.hpp>
#include <tapy/objects/str.hpp>

using namespace tap;

namespace tapy {

void Object::modified() throw ()
{
	ObjectModified(GetContext().instance(), this);
}

IntObject *Object::id() const
{
	return IntObject::New(intptr_t(this));
}

StrObject *Object::repr() const
{
	return StrObject::New((boost::format("<%s object at %p>") % ObjectType(this)->name() % this).str());
}

StrObject *Object::str() const
{
	return repr();
}

BytesObject *Object::bytes() const
{
	auto s = str();
	return BytesObject::New(reinterpret_cast<const uint8_t *> (s->data()), s->size());
}

BoolObject *Object::lt(const Object *other) const
{
	return Bool(uintptr_t(this) < uintptr_t(other));
}

BoolObject *Object::le(const Object *other) const
{
	return Bool(uintptr_t(this) <= uintptr_t(other));
}

BoolObject *Object::eq(const Object *other) const
{
	return Bool(uintptr_t(this) == uintptr_t(other));
}

BoolObject *Object::ne(const Object *other) const
{
	return Bool(uintptr_t(this) != uintptr_t(other));
}

BoolObject *Object::gt(const Object *other) const
{
	return Bool(uintptr_t(this) > uintptr_t(other));
}

BoolObject *Object::ge(const Object *other) const
{
	return Bool(uintptr_t(this) >= uintptr_t(other));
}

IntObject *Object::hash() const
{
	return IntObject::New(intptr_t(this));
}

BoolObject *Object::bool_() const
{
	return True();
}

Object *Object::getattribute(const Object *name)
{
	throw NotImplemented("getattribute");
}

Object *Object::call(const TupleObject *args, DictObject *kwargs)
{
	throw NotImplemented("call");
}

IntObject *Object::len() const
{
	throw NotImplemented("len");
}

BoolObject *Object::contains(const Object *other) const
{
	throw NotImplemented("contains");
}

Object *Object::add(const Object *other) const
{
	throw NotImplemented("add");
}

Object *Object::iter()
{
	throw NotImplemented("iter");
}

Object *Object::next()
{
	throw NotImplemented("next");
}

} // namespace
