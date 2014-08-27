#include "core.hpp"

namespace tap {

extern "C" {
	static void (*original_object_free)(void *ctx, void *ptr);

	static void object_free_wrapper(void *ctx, void *ptr)
	{
		for (PeerObject *peer: instance_peers())
			peer->object_freed(ptr);

		original_object_free(ctx, ptr);
	}
}

void allocator_init()
{
	PyMemAllocator allocator;

	PyMem_GetAllocator(PYMEM_DOMAIN_OBJ, &allocator);

	original_object_free = allocator.free;
	allocator.free = object_free_wrapper;

	PyMem_SetAllocator(PYMEM_DOMAIN_OBJ, &allocator);
}

} // namespace tap
