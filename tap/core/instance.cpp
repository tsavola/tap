#include "core.hpp"

#include <new>

namespace tap {

struct Instance {
	std::unordered_set<PeerObject *> peers;
};

static Instance *instance;

int instance_init()
{
	instance = new (std::nothrow) Instance;
	if (instance == nullptr)
		return -1;

	return 0;
}

std::unordered_set<PeerObject *> &instance_peers()
{
	return instance->peers;
}

} // namespace tap
