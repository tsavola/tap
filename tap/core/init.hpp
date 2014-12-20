#ifndef TAP_CORE_INIT_HPP
#define TAP_CORE_INIT_HPP

#include "visibility.hpp"

extern "C" {

PyMODINIT_FUNC PyInit_core() noexcept TAP_VISIBLE;

} // extern "C"

#endif
