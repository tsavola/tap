/*
 * Copyright (c) 2013, Timo Savola
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

#include "api.h"
#include "tapy.hpp"

#include <exception>
#include <iostream>

#include <boost/coroutine/all.hpp>

#include <tapy/executor.hpp>
#include <tapy/objects/code.hpp>

using namespace tapy;

typedef boost::coroutines::coroutine<void(const CodeObject *)> ExecuteType;

static void execution(ExecuteType::caller_type &c) throw ()
{
	try {
		Executor executor(c.get());
		executor.execute();
	} catch (const std::exception &e) {
		std::cerr << "execute exception: " << e.what() << std::endl;
	} catch (...) {
		std::cerr << "execute: other exception" << std::endl;
	}
}

int tapy_execute(const void *data, size_t size)
{
	CodeObject *code;

	try {
		code = CodeObject::Load(data, size);
	} catch (const std::exception &e) {
		std::cerr << "execute: " << e.what() << std::endl;
		return -1;
	} catch (...) {
		std::cerr << "execute: other exception" << std::endl;
		return -1;
	}

	try {
		ExecuteType c(execution, code);
		return 0;
	} catch (...) {
		std::cerr << "execute: coroutine error" << std::endl;
		return -1;
	}
}
