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

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <glib.h>

#include <tap.h>

#include "test.h"

/* Long */

struct TestLong {
	int64_t value;
};

struct TestLongMarshaled {
	int64_t portable_value;
} TAP_PACKED;

typedef struct TestLong TestLong;
typedef struct TestLongMarshaled TestLongMarshaled;
 
static void test_long_type_destroy(const TapType *type, void *ptr)
{
}
 
static bool test_long_type_traverse(const TapType *type, const void *ptr, TapVisitFunc func, void *arg)
{
	return true;
}

static size_t test_long_type_marshal_size(const TapType *type, const void *ptr, TapPeer *peer)
{
	return sizeof (TestLongMarshaled);
}

static bool test_long_type_marshal(const TapType *type, const void *ptr, TapPeer *peer, void *buf,
                                   size_t size)
{
	const TestLong *object = ptr;
	TestLongMarshaled *marshaled = buf;

	marshaled->portable_value = tap_port_64(object->value);

	return true;
}

static void *test_long_type_unmarshal_alloc(const TapType *type, const void *data, size_t size)
{
	if (size != sizeof (TestLongMarshaled))
		return NULL;

	return tap_object_alloc(type, sizeof (TestLong));
}

static bool test_long_type_unmarshal(const TapType *type, void *ptr, TapPeer *peer, const void *data,
                                     size_t size)
{
	TestLong *object = ptr;
	const TestLongMarshaled *marshaled = data;

	object->value = tap_port_64(marshaled->portable_value);

	return true;
}

static TapType test_long_type = {
	.id               = TEST_LONG_TYPE_ID,
	.name             = "long",
	.destroy          = test_long_type_destroy,
	.traverse         = test_long_type_traverse,
	.marshal_size     = test_long_type_marshal_size,
	.marshal          = test_long_type_marshal,
	.unmarshal_alloc  = test_long_type_unmarshal_alloc,
	.unmarshal        = test_long_type_unmarshal,
};

TestLong *test_long_new(int64_t value)
{
	TestLong *object = tap_object_alloc(&test_long_type, sizeof (TestLong));

	if (object)
		object->value = value;

	return object;
}

/* Tuple */

struct TestTuple {
	uint32_t length;
	void *items[0];
};

struct TestTupleMarshaled {
	uint32_t portable_length;
	uint32_t portable_item_keys[0];
} TAP_PACKED;

typedef struct TestTuple TestTuple;
typedef struct TestTupleMarshaled TestTupleMarshaled;
 
static void test_tuple_type_destroy(const TapType *type, void *ptr)
{
}
 
static bool test_tuple_type_traverse(const TapType *type, const void *ptr, TapVisitFunc func, void *arg)
{
	const TestTuple *object = ptr;

	for (uint32_t i = 0; i < object->length; i++)
		if (object->items[i] && !func(object->items[i], arg))
			return false;

	return true;
}

static size_t test_tuple_type_marshal_size(const TapType *type, const void *ptr, TapPeer *peer)
{
	const TestTuple *object = ptr;

	return sizeof (TestTupleMarshaled) + object->length * sizeof (uint32_t);
}

static bool test_tuple_type_marshal(const TapType *type, const void *ptr, TapPeer *peer, void *buf,
                                    size_t size)
{
	const TestTuple *object = ptr;
	TestTupleMarshaled *marshaled = buf;

	marshaled->portable_length = tap_port_32(object->length);

	for (uint32_t i = 0; i < object->length; i++)
		if (object->items[i]) {
			uint32_t key = tap_peer_key(peer, object->items[i]);
			if (key == 0)
				return false;

			marshaled->portable_item_keys[i] = tap_port_32(key);
		}

	return true;
}

static void *test_tuple_type_unmarshal_alloc(const TapType *type, const void *data, size_t size)
{
	if (size < sizeof (TestTupleMarshaled))
		return NULL;

	const TestTupleMarshaled *marshaled = data;
	uint32_t length = tap_port_32(marshaled->portable_length);

	if (size != sizeof (TestTupleMarshaled) + length * sizeof (uint32_t))
		return NULL;

	return tap_object_alloc(type, sizeof (TestTuple) + length * sizeof (void *));
}

static bool test_tuple_type_unmarshal(const TapType *type, void *ptr, TapPeer *peer, const void *data,
                                      size_t size)
{
	TestTuple *object = ptr;
	const TestTupleMarshaled *marshaled = data;

	object->length = tap_port_32(marshaled->portable_length);

	for (uint32_t i = 0; i < object->length; i++) {
		uint32_t key = tap_port_32(marshaled->portable_item_keys[i]);

		if (key) {
			void *item = tap_peer_object(peer, key);
			if (item == NULL)
				return false;

			object->items[i] = item;
		}
	}

	return true;
}

static TapType test_tuple_type = {
	.id               = TEST_TUPLE_TYPE_ID,
	.name             = "tuple",
	.destroy          = test_tuple_type_destroy,
	.traverse         = test_tuple_type_traverse,
	.marshal_size     = test_tuple_type_marshal_size,
	.marshal          = test_tuple_type_marshal,
	.unmarshal_alloc  = test_tuple_type_unmarshal_alloc,
	.unmarshal        = test_tuple_type_unmarshal,
};

TestTuple *test_tuple_new(uint32_t length)
{
	TestTuple *object = tap_object_alloc(&test_tuple_type, sizeof (TestTuple) + length * sizeof (void *));

	if (object)
		object->length = length;

	return object;
}

void test_tuple_set_item(TapInstance *instance, TestTuple *object, uint32_t i, void *item)
{
	assert(i < object->length);
	object->items[i] = item;
	tap_object_modified(instance, object);
}

void *test_tuple_get_item(const TestTuple *object, uint32_t i)
{
	assert(i < object->length);
	return object->items[i];
}

/* Visitor */

static bool test_visit(const void *ptr, void *arg)
{
	GHashTable *refs = arg;

	if (g_hash_table_contains(refs, ptr))
		return true;

	g_hash_table_add(refs, (void *) ptr);

	const TapType *type = tap_object_type(ptr);

	printf("%s object at %p\n", type->name, ptr);

	return type->traverse(type, ptr, test_visit, arg);
}

int main(int argc, char **argv)
{
	TapSystem *system = tap_system_new(TEST_SYSTEM_NAME);
	assert(system);

	tap_type_init(&test_long_type, system);
	tap_type_init(&test_tuple_type, system);

	size_t bufsize = 0;
	void *bufdata = NULL;

	{
		TapInstance *instance = tap_instance_new(system);
		assert(instance);

		{
			TestLong *long1 = test_long_new(300);
			assert(long1);

			TestLong *long2 = test_long_new(400);
			assert(long2);

			TestTuple *tuple1 = test_tuple_new(2);
			assert(tuple1);
			test_tuple_set_item(instance, tuple1, 0, long1);
			test_tuple_set_item(instance, tuple1, 1, long2);

			TestTuple *tuple2 = test_tuple_new(3);
			assert(tuple2);
			test_tuple_set_item(instance, tuple2, 0, long2);

			TestTuple *root = test_tuple_new(2);
			assert(root);
			test_tuple_set_item(instance, root, 0, tuple1);
			test_tuple_set_item(instance, root, 1, tuple2);

			{
				GHashTable *refs = g_hash_table_new(NULL, NULL);
				assert(refs);

				test_visit(root, refs);

				printf("%u reachable objects\n", g_hash_table_size(refs));

				g_hash_table_destroy(refs);
			}

			{
				TapPeer *peer = tap_peer_new(instance, false);
				assert(peer);

				TapBuffer *buffer = tap_buffer_new();
				assert(buffer);

				bool ok = tap_marshal(peer, buffer, root);
				assert(ok);

				for (size_t i = 0; i < tap_buffer_size(buffer); i++)
					printf("%02x%c",
					       ((const uint8_t *) tap_buffer_data(buffer))[i], (i % 16) < 15 ? ' ' : '\n');

				printf("\n");

				bufsize = tap_buffer_size(buffer);
				bufdata = malloc(bufsize);
				assert(bufdata);
				memcpy(bufdata, tap_buffer_data(buffer), bufsize);

				tap_buffer_destroy(buffer);

				tap_peer_destroy(peer);
			}
		}

		tap_instance_destroy(instance);
	}

	printf("--\n");

	{
		TapInstance *instance = tap_instance_new(system);
		assert(instance);

		TapPeer *peer = tap_peer_new(instance, true);
		assert(peer);

		void *root = tap_unmarshal(peer, bufdata, bufsize);
		assert(root);

		test_tuple_set_item(instance, (TestTuple *) root, 1, root);

		{
			GHashTable *refs = g_hash_table_new(NULL, NULL);
			assert(refs);

			test_visit(root, refs);

			printf("%u reachable objects\n", g_hash_table_size(refs));

			g_hash_table_destroy(refs);
		}

		TapBuffer *buffer = tap_buffer_new();
		assert(buffer);

		bool ok = tap_marshal(peer, buffer, root);
		assert(ok);

		for (size_t i = 0; i < tap_buffer_size(buffer); i++)
			printf("%02x%c", ((const uint8_t *) tap_buffer_data(buffer))[i], (i % 16) < 15 ? ' ' : '\n');

		printf("\n");

		tap_buffer_destroy(buffer);

		tap_peer_destroy(peer);

		tap_instance_destroy(instance);
	}

	return 0;
}
