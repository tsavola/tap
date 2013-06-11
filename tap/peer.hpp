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

#ifndef TAP_PEER_HPP
#define TAP_PEER_HPP

#include <cstdint>
#include <map>

#include "instance.hpp"

struct TapPeer {
	class KeyState {
	public:
		KeyState() throw ()
		{
		}

		KeyState(uint32_t key, bool dirty) throw ():
			key(key),
			dirty(dirty)
		{
		}

		KeyState(const KeyState &other) throw ():
			key(other.key),
			dirty(other.dirty)
		{
		}

		KeyState &operator=(const KeyState &other) throw ()
		{
			key = other.key;
			dirty = other.dirty;
			return *this;
		}

		uint32_t key;
		bool dirty;
	};

	TapPeer(TapInstance &instance, bool secondary) throw ():
		instance(instance),
		next_key(secondary ? 2 : 1)
	{
		instance.peers.insert(this);
	}

	~TapPeer() throw ()
	{
		instance.peers.erase(this);
	}

	uint32_t insert(const void *ptr, bool dirty)
	{
		uint32_t key = next_key;
		next_key += 2;

		key_states[ptr] = TapPeer::KeyState(key, dirty);
		objects[key] = const_cast<void *> (ptr);

		return key;
	}

	TapInstance &instance;
	std::map<const void *, KeyState> key_states;
	std::map<uint32_t, void *> objects;
	uint32_t next_key;
};

#endif
