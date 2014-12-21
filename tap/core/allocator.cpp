#include "core.hpp"

namespace tap {

static void (*original_object_free)(void *ctx, void *ptr) noexcept;

static void object_free_wrapper(void *ctx, void *ptr) noexcept
{
	for (PeerObject *peer: instance_peers())
		peer->object_freed(ptr);

	original_object_free(ctx, ptr);
}

void allocator_init() noexcept
{
	PyMemAllocator allocator;

	PyMem_GetAllocator(PYMEM_DOMAIN_OBJ, &allocator);

	original_object_free = allocator.free;
	allocator.free = object_free_wrapper;

	PyMem_SetAllocator(PYMEM_DOMAIN_OBJ, &allocator);
}

} // namespace tap
