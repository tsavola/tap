/*
 * Copyright (c) 2011-2013, Timo Savola
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef TAPY_TRACE_HPP
#define TAPY_TRACE_HPP

#ifdef TAPY_TRACE
# include <iostream>
# include <string>

# include <boost/format.hpp>
#endif

namespace tapy {

#ifdef TAPY_TRACE

void TracePrefix(std::string &prefix);

template <typename Format>
inline Format TraceFormat(Format format)
{
	return format;
}

template <typename Format, typename FirstArg, typename... OtherArgs>
inline Format TraceFormat(Format format, FirstArg first_arg,
                          OtherArgs... other_args)
{
	return TraceFormat(format % first_arg, other_args...);
}

template <typename... Args>
void Trace(const char *format, Args... args) throw ()
{
	try {
		std::string prefix;
		TracePrefix(prefix);

		std::cerr << (prefix + boost::str(TraceFormat(boost::format(format), args...)) + "\n");
	} catch (...) {
	}
}

#else

template <typename... Args>
inline void Trace(const char *, Args...) throw ()
{
}

#endif

} // namespace

#endif
