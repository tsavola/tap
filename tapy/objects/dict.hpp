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

#ifndef TAPY_OBJECTS_DICT_HPP
#define TAPY_OBJECTS_DICT_HPP

#include <unordered_map>

#include <tapy/objects/object.hpp>

namespace tapy {

class DictObject: public Object, public tap::ObjectBase<DictObject> {
public:
	static void *UnmarshalAlloc(const void *buf, size_t bufsize);

	DictObject()
	{
	}

	DictObject(tap::Peer &peer, const void *buf, size_t bufsize);

	bool traverse(tap::VisitFunc func, void *arg) const;

	size_t marshal_size(tap::Peer &peer) const throw ();

	bool marshal(tap::Peer &peer, void *buf, size_t bufsize) const;

	uint32_t length() const throw ()
	{
		return m_map.size();
	}

	void set(const Object *name, Object *value);

	Object *get(const Object *name) const;

	void copy_from(const DictObject *other);

	virtual IntObject *len() const;

	static TypeObject *type;

private:
	struct Portable;

	struct Hash {
		size_t operator()(const Object *object) const;
	};

	struct KeyEqual {
		bool operator()(const Object *a, const Object *b) const;
	};

	std::unordered_map<const Object *, Object *, Hash, KeyEqual> m_map;
};

} // namespace

#endif
