#include "core.hpp"

namespace tap {

struct Instance {
	std::unordered_set<PeerObject *> peers;
	std::unordered_map<std::string, PyTypeObject *> opaque_types;
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

std::unordered_map<std::string, PyTypeObject *> &instance_opaque_types() noexcept
{
	return instance->opaque_types;
}

} // namespace tap
