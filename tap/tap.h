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

#ifndef TAP_TAP_H
#define TAP_TAP_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifndef TAP_PACKED
#define TAP_PACKED __attribute__ ((packed))
#endif

#ifndef TAP_PORTABLE_BYTEORDER
# ifdef __linux__
#  include <endian.h>
# else
#  include <sys/types.h>
# endif
# if defined(BYTE_ORDER)
#  if BYTE_ORDER == LITTLE_ENDIAN
#   define TAP_PORTABLE_BYTEORDER true
#  else
#   define TAP_PORTABLE_BYTEORDER false
#  endif
# elif defined(__BYTE_ORDER)
#  if __BYTE_ORDER == __LITTLE_ENDIAN
#   define TAP_PORTABLE_BYTEORDER true
#  else
#   define TAP_PORTABLE_BYTEORDER false
#  endif
# elif defined(__APPLE__)
#  if defined(__LITTLE_ENDIAN__)
#   define TAP_PORTABLE_BYTEORDER true
#  else
#   define TAP_PORTABLE_BYTEORDER false
#  endif
# else
#  error cannot figure out byteorder
# endif
#endif

#ifdef __cplusplus
extern "C" {
#endif

struct TapBuffer;
struct TapInstance;
struct TapPeer;
struct TapSystem;
struct TapType;

typedef struct TapBuffer TapBuffer;
typedef struct TapInstance TapInstance;
typedef struct TapPeer TapPeer;
typedef struct TapSystem TapSystem;
typedef struct TapType TapType;

// TODO: typedef uint32_t TapKey;

typedef bool (*TapVisitFunc)(const void *ptr, void *arg);

struct TapType {
	TapSystem *system;
	uint32_t id;
	const char *name;
	void (*destroy)(const TapType *type, void *ptr);
	bool (*traverse)(const TapType *type, const void *ptr, TapVisitFunc func,
	                 void *arg);
	size_t (*marshal_size)(const TapType *type, const void *ptr,
	                       TapPeer *peer);
	bool (*marshal)(const TapType *type, const void *ptr, TapPeer *peer,
	                void *buf, size_t bufsize);
	void *(*unmarshal_alloc)(const TapType *type, const void *data,
	                         size_t size);
	bool (*unmarshal)(const TapType *type, void *ptr, TapPeer *peer,
	                  const void *buf, size_t bufsize);
};

TAP_API TapSystem *tap_system_new(const char *name);
TAP_API void tap_system_destroy(TapSystem *system);
TAP_API const char *tap_system_name(const TapSystem *system);
TAP_API const TapType *tap_system_type(const TapSystem *system,
                                       uint32_t type_id);

TAP_API TapInstance *tap_instance_new(const TapSystem *system);
TAP_API void tap_instance_destroy(TapInstance *instance);

TAP_API TapPeer *tap_peer_new(TapInstance *instance, bool secondary);
TAP_API void tap_peer_destroy(TapPeer *peer);
TAP_API uint32_t tap_peer_key(TapPeer *peer, const void *ptr);
TAP_API void *tap_peer_object(TapPeer *peer, uint32_t key);

TAP_API void tap_type_init(TapType *type, TapSystem *system);

TAP_API void *tap_object_alloc(const TapType *type, size_t size);
TAP_API void tap_object_free(void *ptr);
TAP_API const TapType *tap_object_type(const void *ptr);
TAP_API void tap_object_modified(TapInstance *instance, void *ptr);

TAP_API TapBuffer *tap_buffer_new(void);
TAP_API void tap_buffer_destroy(TapBuffer *buffer);
TAP_API size_t tap_buffer_size(const TapBuffer *buffer);
TAP_API const void *tap_buffer_data(const TapBuffer *buffer);

TAP_API bool tap_marshal(TapPeer *peer, TapBuffer *buffer, const void *ptr);
TAP_API void *tap_unmarshal(TapPeer *peer, const void *buf, size_t bufsize);

#if TAP_PORTABLE_BYTEORDER
static inline uint16_t tap_port_16(uint16_t x) { return x; }
static inline uint32_t tap_port_32(uint32_t x) { return x; }
static inline uint64_t tap_port_64(uint64_t x) { return x; }
#else
static inline uint16_t tap_port_16(uint16_t x) { return __bswap_16(x); }
static inline uint32_t tap_port_32(uint32_t x) { return __bswap_32(x); }
static inline uint64_t tap_port_64(uint64_t x) { return __bswap_64(x); }
#endif

#ifdef __cplusplus
}
#endif

#endif
