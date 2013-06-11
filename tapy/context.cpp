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
#include "context.hpp"
#include "tapy.hpp"

#include <cassert>
#include <cstddef>
#include <string>

#include <boost/utility.hpp>

#include <tapy/modules/builtins.hpp>
#include <tapy/modules/tap.hpp>
#include <tapy/objects/bool.hpp>
#include <tapy/objects/module.hpp>
#include <tapy/objects/none.hpp>
#include <tapy/objects/str.hpp>
#include <tapy/system.hpp>

using namespace tap;

#ifndef TAPY_THREAD_LOCAL
#define TAPY_THREAD_LOCAL __thread
#endif

namespace tapy {

static TAPY_THREAD_LOCAL Context *context;

class Context::Internal: boost::noncopyable {
public:
	Internal():
		none(NULL),
		builtins(NULL)
	{
		none = NoneObject::New();
		false_ = BoolObject::New(false);
		true_ = BoolObject::New(true);

		builtins = CreateBuiltinsModule(none, false_, true_);
		add(builtins);

		add(CreateTapModule());
	}

	~Internal()
	{
		// TODO: delete modules, or something
	}

	NoneObject *none;
	BoolObject *false_;
	BoolObject *true_;
	ModuleObject *builtins;
	std::map<std::string, ModuleObject *> modules;

private:
	void add(ModuleObject *module)
	{
		modules[module->name()] = module;
	}
};

Context::Context():
	m_instance(InitSystem())
{
	assert(context == NULL);
	context = this;

	m_internal = new Internal;
}

Context::~Context() throw ()
{
	delete m_internal;

	assert(context == this);
	context = NULL;
}

NoneObject *Context::none() throw ()
{
	return m_internal->none;
}

BoolObject *Context::false_() throw ()
{
	return m_internal->false_;
}

BoolObject *Context::true_() throw ()
{
	return m_internal->true_;
}

Object *Context::load_builtin_name(const Object *name)
{
	return m_internal->builtins->getattribute(name);
}

Object *Context::import_builtin_module(const Object *name)
{
	auto i = m_internal->modules.find(Require<StrObject>(name)->data());
	if (i == m_internal->modules.end())
		throw std::runtime_error("unknown module");
	return i->second;
}

Context &GetContext() throw ()
{
	assert(context);
	return *context;
}

// declared in tapy/objects/none.hpp

NoneObject *None() throw ()
{
	return GetContext().none();
}

// declared in tapy/objects/bool.hpp

BoolObject *False() throw ()
{
	return GetContext().false_();
}

BoolObject *True() throw ()
{
	return GetContext().true_();
}

} // namespace

using namespace tapy;

TapyContext *tapy_context_new(void)
{
	return reinterpret_cast<TapyContext *> (new (std::nothrow) Context);
}

void tapy_context_destroy(TapyContext *context)
{
	delete reinterpret_cast<Context *> (context);
}
