#ifndef TAP_CORE_MAPPING_HPP
#define TAP_CORE_MAPPING_HPP

#include "core.hpp"

namespace tap {

struct MappingOrig {
	int (*ass_subscript)(PyObject *, PyObject *, PyObject *) noexcept;
};

template <typename T>
struct MappingWrap {
	static int ass_subscript(PyObject *self, PyObject *key, PyObject *value) noexcept
	{
		int ret = orig.ass_subscript(self, key, value);
		if (ret == 0)
			peers_touch(self);

		return ret;
	}

	static void init(PyTypeObject *type) noexcept
	{
		orig.ass_subscript = type->tp_as_mapping->mp_ass_subscript;
		type->tp_as_mapping->mp_ass_subscript = ass_subscript;
	}

	static MappingOrig orig;
};

template<typename T> MappingOrig MappingWrap<T>::orig;

} // namespace tap

#endif
