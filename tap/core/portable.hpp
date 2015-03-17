#ifndef TAP_CORE_PORTABLE_HPP
#define TAP_CORE_PORTABLE_HPP

#include <boost/endian/conversion.hpp>

#define TAP_PACKED  __attribute__ ((packed))

namespace tap {

template <typename T>
inline T port(T x) noexcept
{
	return boost::endian::conditional_reverse<boost::endian::order::little, boost::endian::order::native>(x);
}

} // namespace tap

#endif
