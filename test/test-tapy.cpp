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

#include <iostream>

#include <tapy/context.hpp>
#include <tapy/executor.hpp>
#include <tapy/objects/bytes.hpp>
#include <tapy/objects/float.hpp>
#include <tapy/objects/int.hpp>
#include <tapy/objects/none.hpp>
#include <tapy/objects/str.hpp>
#include <tapy/objects/tuple.hpp>
#include <tapy/system.hpp>

using namespace tap;
using namespace tapy;

int main(int argc, char **argv)
{
	Context context;

	auto i = IntObject::New(5739);
	auto j = FloatObject::New(429.832);
	auto k = BytesObject::New((uint8_t *) "hello", 5);
	auto l = StrObject::New("world");

	std::cout << i->repr() << std::endl;
	std::cout << j->repr() << std::endl;
	std::cout << k->repr() << std::endl;
	std::cout << l->repr() << std::endl;
	std::cout << None()->repr() << std::endl;

	auto t = TupleObject::New(i, j, k, l, None());

	std::cout << t->repr() << std::endl;

	Peer peer(context.instance(), false);
	Buffer buffer;
	Marshal(peer, buffer, t);

	std::cout << BytesObject::New((uint8_t *) buffer.data(), buffer.size())->repr() << std::endl;

	return 0;
}
