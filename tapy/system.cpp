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
#include "system.hpp"
#include "tapy.hpp"

#include <cstddef>

#include <tapy/objects/bool.hpp>
#include <tapy/objects/bytes.hpp>
#include <tapy/objects/code.hpp>
#include <tapy/objects/dict.hpp>
#include <tapy/objects/float.hpp>
#include <tapy/objects/function.hpp>
#include <tapy/objects/int.hpp>
#include <tapy/objects/internal.hpp>
#include <tapy/objects/module.hpp>
#include <tapy/objects/none.hpp>
#include <tapy/objects/range.hpp>
#include <tapy/objects/str.hpp>
#include <tapy/objects/tuple.hpp>

using namespace tap;

namespace tapy {

static System *system;

static void InitTypes(System &system)
{
	uint32_t id = 0;

	InitType<TypeObject>     (system, ++id, "type");
	InitType<NoneObject>     (system, ++id, "none");

	InitType<IntObject>      (system, ++id, "int");
	InitType<BoolObject>     (system, ++id, "bool");
	InitType<FloatObject>    (system, ++id, "float");

	InitType<BytesObject>    (system, ++id, "bytes");
	InitType<StrObject>      (system, ++id, "str");
	InitType<TupleObject>    (system, ++id, "tuple");
	InitType<DictObject>     (system, ++id, "dict");

	InitType<FunctionObject> (system, ++id, "function");
	InitType<ModuleObject>   (system, ++id, "module");
	InitType<CodeObject>     (system, ++id, "code");

	InitType<RangeObject>    (system, ++id, "range");
	InitType<InternalObject> (system, ++id, "internal");
}

System &InitSystem()
{
	if (system == NULL) {
		system = new System(TAPY_SYSTEM_NAME);
		InitTypes(*system);
	}

	return *system;
}

const System &GetSystem() throw ()
{
	assert(system);
	return *system;
}

} // namespace

using namespace tapy;

int tapy_init(void)
{
	try {
		InitSystem();
		return 0;
	} catch (...) {
		return -1;
	}
}
