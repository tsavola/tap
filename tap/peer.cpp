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
#include "peer.hpp"
#include "tap.hpp"

#include <cassert>
#include <new>

#include "instance.hpp"

TapPeer *tap_peer_new(TapInstance *instance, bool secondary)
{
	return new (std::nothrow) TapPeer(*instance, secondary);
}

void tap_peer_destroy(TapPeer *peer)
{
	delete peer;
}

TapKey tap_peer_key(TapPeer *peer, const void *ptr)
{
	assert(ptr);

	TapKey key = 0;

	if (ptr) {
		auto i = peer->key_states.find(ptr);
		if (i != peer->key_states.end()) {
			key = i->second.key;
		} else {
			key = peer->insert(ptr, true);
		}
	}

	return key;
}

void *tap_peer_object(TapPeer *peer, TapKey key)
{
	assert(key);

	void *ptr = NULL;

	auto i = peer->objects.find(key);
	if (i != peer->objects.end())
		ptr = i->second;

	return ptr;
}
