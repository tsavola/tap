#ifndef TAP_CORE_PORTABLE_HPP
#define TAP_CORE_PORTABLE_HPP

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

#define TAP_PACKED  __attribute__ ((packed))

namespace tap {

#if TAP_PORTABLE_BYTEORDER

template <typename T> inline T port(T x) noexcept { return x; }

#else

template <typename T, unsigned int N> struct Porter;

template <typename T> struct Porter<T, 1> {
	static inline T port(T x) noexcept { return x; }
};

template <typename T> struct Porter<T, 2> {
	static inline T port(T x) noexcept { return __bswap_16(x); }
};

template <typename T> struct Porter<T, 4> {
	static inline T port(T x) noexcept { return __bswap_32(x); }
};

template <typename T> struct Porter<T, 8> {
	static inline T port(T x) noexcept { return __bswap_64(x); }
};

template <typename T> inline T port(T x) noexcept { return Porter<T, sizeof (T)>::port(x); }

#endif

} // namespace tap

#endif
