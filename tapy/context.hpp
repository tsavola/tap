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

#ifndef TAPY_CONTEXT_HPP
#define TAPY_CONTEXT_HPP

#include "api.h"
#include "tapy.hpp"

#include <cassert>

#include <tap.hpp>

namespace tapy {

class BoolObject;
class NoneObject;
class Object;

TapyContext &GetContext() throw ();

} // namespace

class TapyContext {
public:
	class Internal;

	TapyContext();
	~TapyContext() throw ();

	tap::Instance &instance() throw ()
	{
		return m_instance;
	}

	tapy::NoneObject *none() throw ();

	tapy::BoolObject *false_() throw ();

	tapy::BoolObject *true_() throw ();

	tapy::Object *load_builtin_name(const tapy::Object *name);

	tapy::Object *import_builtin_module(const tapy::Object *name);

private:
	TapyContext(const TapyContext &);
	void operator=(const TapyContext &);

	Internal *m_internal;
	tap::Instance m_instance;
};

#endif
