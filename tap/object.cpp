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

#include "api.h"
#include "tap.hpp"

#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <new>

#include "instance.hpp"
#include "peer.hpp"

struct TapObject {
	const TapType *type;
};

static TapObject *tap_object(const void *ptr) throw ()
{
	return reinterpret_cast<TapObject *> (const_cast<void *> (ptr)) - 1;
}

void *tap_object_alloc(const TapType *type, size_t size)
{
	char *buf = new (std::nothrow) char[sizeof (TapObject) + size];
	if (buf == NULL)
		return NULL;

	TapObject *object = reinterpret_cast<TapObject *> (buf);
	object->type = type;

	void *ptr = buf + sizeof (TapObject);
	std::memset(ptr, 0, size);

	return ptr;
}

void tap_object_free(void *ptr)
{
	if (ptr)
		delete[] reinterpret_cast<char *> (tap_object(ptr));
}

const TapType *tap_object_type(const void *ptr)
{
	return tap_object(ptr)->type;
}

void tap_object_modified(TapInstance *instance, void *ptr)
{
	auto &peers = instance->peers;
	for (auto i = peers.begin(); i != peers.end(); ++i) {
		auto &key_states = (*i)->key_states;
		auto j = key_states.find(ptr);
		if (j != key_states.end())
			j->second.dirty = true;
	}
}
