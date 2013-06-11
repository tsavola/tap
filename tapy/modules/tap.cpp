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

#include "tap.hpp"

#include <exception>
#include <iostream>

#include <boost/coroutine/all.hpp>
#include <boost/tuple/tuple.hpp>

#include <tapy/objects/dict.hpp>
#include <tapy/objects/module.hpp>
#include <tapy/objects/none.hpp>
#include <tapy/objects/tuple.hpp>

namespace tapy {

typedef boost::coroutines::coroutine<
			void(boost::tuple<Object *, const TupleObject *, DictObject *>)
		> SpawnType;

static void Spawnee(SpawnType::caller_type &c) throw ()
{
	try {
		auto &params = c.get();
		params.get<0>()->call(params.get<1>(), params.get<2>());
	} catch (const std::exception &e) {
		std::cerr << "spawn exception: " << e.what() << std::endl;
	} catch (...) {
		std::cerr << "spawn: other exception" << std::endl;
	}
}

static Object *Spawn(const TupleObject *args, DictObject *kwargs)
{
	auto callable = args->get(0);
	auto callargs = TupleObject::NewUninit(args->length() - 1);

	for (unsigned int i = 1; i < args->length(); i++)
		callargs->init(i - 1, args->get(i));

	auto c = new SpawnType(Spawnee, boost::make_tuple(callable, callargs, kwargs));
	// TODO: register 'c' somewhere somehow

	return None();
}

ModuleObject *CreateTapModule()
{
	auto module = ModuleObject::New("tap");

	module->init("spawn", Spawn);

	return module;
}

} // namespace
