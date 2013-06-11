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

#ifndef TAPY_OBJECTS_OBJECT_HPP
#define TAPY_OBJECTS_OBJECT_HPP

#include <stdexcept>
#include <string>

#include <tap.hpp>

namespace tapy {

class BoolObject;
class BytesObject;
class DictObject;
class IntObject;
class StrObject;
class TupleObject;
class TypeObject;

class Object {
public:
	template <typename T>
	const T *dynamic() const
	{
		return dynamic_cast<const T *> (this);
	}

	template <typename T>
	T *dynamic()
	{
		return dynamic_cast<T *> (this);
	}

	const TypeObject *type() const throw ()
	{
		return reinterpret_cast<const TypeObject *> (tap::ObjectType(this));
	}

	IntObject *id() const;

	virtual StrObject *repr() const;

	virtual StrObject *str() const;

	virtual BytesObject *bytes() const;

	virtual BoolObject *lt(const Object *other) const;

	virtual BoolObject *le(const Object *other) const;

	virtual BoolObject *eq(const Object *other) const;

	virtual BoolObject *ne(const Object *other) const;

	virtual BoolObject *gt(const Object *other) const;

	virtual BoolObject *ge(const Object *other) const;

	virtual IntObject *hash() const;

	virtual BoolObject *bool_() const;

	// Attribute access

	virtual Object *getattribute(const Object *name);

	// Callable

	virtual Object *call(const TupleObject *args, DictObject *kwargs);

	// Container

	virtual IntObject *len() const;

	virtual BoolObject *contains(const Object *other) const;

	// Numeric

	virtual Object *add(const Object *other) const;

	// Iterable

	virtual Object *iter();

	// Iterator

	virtual Object *next();

protected:
	void modified() throw ();
};

class NotImplemented: public std::runtime_error {
public:
	explicit NotImplemented(const char *message) throw ():
		std::runtime_error(message)
	{
	}
};

class AttributeError: public std::runtime_error {
public:
	explicit AttributeError(const char *message) throw ():
		std::runtime_error(message)
	{
	}
};

class StopIteration: public std::runtime_error {
public:
	StopIteration() throw ():
		std::runtime_error("stop iteration")
	{
	}
};

template <typename T>
const T *Require(const void *ptr)
{
	if (ptr) {
		auto object = dynamic_cast<const T *>(
				reinterpret_cast<const Object *> (ptr));
		if (object)
			return object;
	}
	throw std::runtime_error("type mismatch");
}

template <typename T>
T *Require(void *ptr)
{
	if (ptr) {
		auto object = dynamic_cast<T *>(reinterpret_cast<Object *> (ptr));
		if (object)
			return object;
	}
	throw std::runtime_error("type mismatch");
}

} // namespace

#endif
