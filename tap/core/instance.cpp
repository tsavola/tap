#include "core.hpp"

namespace tap {

struct Instance {
	std::unordered_set<PeerObject *> peers;
};

static Instance *instance;

int instance_init() noexcept
{
	try {
		instance = new Instance;
	} catch (...) {
		return -1;
	}

	return 0;
}

std::unordered_set<PeerObject *> &instance_peers() noexcept
{
	return instance->peers;
}

} // namespace tap
