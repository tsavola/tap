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

#include "builtins.hpp"

#include <cstddef>
#include <iostream>
#include <string>

#include <tapy/objects/bool.hpp>
#include <tapy/objects/dict.hpp>
#include <tapy/objects/int.hpp>
#include <tapy/objects/module.hpp>
#include <tapy/objects/none.hpp>
#include <tapy/objects/range.hpp>
#include <tapy/objects/str.hpp>
#include <tapy/objects/tuple.hpp>

namespace tapy {

static Object *Id(const TupleObject *args, DictObject *kwargs)
{
	args->get(0)->id();
	return None();
}

static Object *Print(const TupleObject *args, DictObject *kwargs)
{
	for (unsigned int i = 0; i < args->length(); i++) {
		if (i > 0)
			std::cout << " ";

		auto so = args->get(i)->str();
		std::string s(so->data(), so->length());
		std::cout << s;
	}

	std::cout << "\n";

	return None();
}

static Object *Range(const TupleObject *args, DictObject *kwargs)
{
	int64_t start = 0;
	int64_t stop;
	int64_t step = 1;

	switch (args->length()) {
	default:
		step = Require<IntObject>(args->get(2))->value();
	case 2:
		stop = Require<IntObject>(args->get(1))->value();
		start = Require<IntObject>(args->get(0))->value();
		break;

	case 1:
		stop = Require<IntObject>(args->get(0))->value();
		break;
	}

	return RangeObject::New(start, stop, step);
}

static Object *Repr(const TupleObject *args, DictObject *kwargs)
{
	return args->get(0)->repr();
}

static Object *Str(const TupleObject *args, DictObject *kwargs)
{
	return args->get(0)->str();
}

static Object *Len(const TupleObject *args, DictObject *kwargs)
{
	return args->get(0)->len();
}

ModuleObject *CreateBuiltinsModule(Object *none, Object *false_, Object *true_)
{
	auto module = ModuleObject::New("builtins");

	module->init("None", none);
	module->init("False", false_);
	module->init("True", true_);
	module->init("id", Id);
	module->init("print", Print);
	module->init("range", Range);
	module->init("repr", Repr);
	module->init("str", Str);
	module->init("len", Len);

	return module;
}

} // namespace
