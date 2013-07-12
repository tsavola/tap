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

#include <cassert>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <set>

#include "buffer.hpp"
#include "peer.hpp"

using namespace tap;

struct TapMarshalHeader {
	uint32_t size;
	uint32_t type_id;
	TapKey key;
} TAP_PACKED;

struct TapMarshalVisitor {
	static bool Visit(const void *ptr, void *arg) throw ()
	{
		return reinterpret_cast<TapMarshalVisitor *> (arg)->visit(ptr);
	}

	TapMarshalVisitor(TapPeer *peer, TapBuffer *buffer) throw ():
		peer(peer),
		buffer(buffer)
	{
	}

	bool visit(const void *ptr) throw ()
	{
		if (seen.find(ptr) != seen.end())
			return true;

		seen.insert(ptr);

		const TapType *type = tap_object_type(ptr);
		TapKey key = 0;

		auto i = peer->key_states.find(ptr);
		if (i != peer->key_states.end()) {
			if (i->second.dirty) {
				key = i->second.key;
				i->second.dirty = false;
			}
		} else {
			key = peer->insert(ptr, false);
		}

		if (key) {
			size_t marshal_size = type->marshal_size(type, ptr, peer);
			size_t extent_size = sizeof (TapMarshalHeader) + marshal_size;

			auto header = reinterpret_cast<TapMarshalHeader *> (
					buffer->extend(extent_size));
			if (header == NULL)
				return false;

			header->size = Port(extent_size);
			header->type_id = Port(type->id);
			header->key = Port(key);

			if (!type->marshal(type, ptr, peer, header + 1, marshal_size))
				return false;
		}

		return type->traverse(type, ptr, Visit, this);
	}

	TapPeer *const peer;
	TapBuffer *const buffer;
	std::set<const void *> seen;
};

bool tap_marshal(TapPeer *peer, TapBuffer *buffer, const void *ptr)
{
	TapMarshalVisitor visitor(peer, buffer);
	return visitor.visit(ptr);
}

static void *tap_unmarshal_pass1(TapPeer *peer, const void *data,
                                 size_t size) throw ()
{
	void *root_ptr = NULL;

	while (size >= sizeof (TapMarshalHeader)) {
		auto header = reinterpret_cast<const TapMarshalHeader *> (data);
		uint32_t item_size = Port(header->size);
		uint32_t item_type_id = Port(header->type_id);
		TapKey item_key = Port(header->key);

		if (item_size > size) {
			std::fprintf(stderr, "unmarshal: header size out of bounds\n");
			return NULL;
		}
		size_t marshal_size = item_size - sizeof (TapMarshalHeader);
		const void *marshal_data = header + 1;

		auto i = peer->instance.system.types.find(item_type_id);
		if (i == peer->instance.system.types.end()) {
			std::fprintf(stderr, "unmarshal: object type id is unknown\n");
			return NULL;
		}
		const TapType *type = i->second;

		void *ptr;

		auto j = peer->objects.find(item_key);
		if (j != peer->objects.end()) {
			ptr = j->second;
			type->destroy(type, ptr);

			peer->key_states[ptr].dirty = false;
		} else {
			ptr = type->unmarshal_alloc(type, marshal_data, marshal_size);
			if (ptr == NULL) {
				std::fprintf(stderr, "unmarshal: allocation failed\n");
				return NULL;
			}

			peer->key_states[ptr] = TapPeer::KeyState(item_key, false);
			peer->objects[item_key] = ptr;
		}

		if (root_ptr == NULL)
			root_ptr = ptr;

		data = reinterpret_cast<const char *> (data) + item_size;
		size -= item_size;
	}

	if (size > 0) {
		std::fprintf(stderr,
		             "unmarshal: trailing garbage or truncated data\n");
		return NULL;
	}

	return root_ptr;
}

static bool tap_unmarshal_pass2(TapPeer *peer, const void *data,
                                size_t size) throw ()
{
	while (size >= sizeof (TapMarshalHeader)) {
		auto header = reinterpret_cast<const TapMarshalHeader *> (data);
		uint32_t item_size = Port(header->size);
		TapKey item_key = Port(header->key);

		size_t marshal_size = item_size - sizeof (TapMarshalHeader);
		const void *marshal_data = header + 1;
		void *ptr = peer->objects.find(item_key)->second;
		const TapType *type = tap_object_type(ptr);

		if (!type->unmarshal(type, ptr, peer, marshal_data, marshal_size))
			return false;

		data = reinterpret_cast<const char *> (data) + item_size;
		size -= item_size;
	}

	return true;
}

void *tap_unmarshal(TapPeer *peer, const void *data, size_t size)
{
	void *ptr = tap_unmarshal_pass1(peer, data, size);

	if (ptr && !tap_unmarshal_pass2(peer, data, size))
		ptr = NULL;

	return ptr;
}
