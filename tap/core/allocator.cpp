#include "core.hpp"

namespace tap {

static void (*object_free_orig)(void *ctx, void *ptr) noexcept;

static void object_free_wrap(void *ctx, void *ptr) noexcept
{
	for (PeerObject *peer: instance_peers())
		peer->object_freed(ptr);

	object_free_orig(ctx, ptr);
}

void allocator_init() noexcept
{
	PyMemAllocator allocator;

	PyMem_GetAllocator(PYMEM_DOMAIN_OBJ, &allocator);

	object_free_orig = allocator.free;
	allocator.free = object_free_wrap;

	PyMem_SetAllocator(PYMEM_DOMAIN_OBJ, &allocator);
}

} // namespace tap
