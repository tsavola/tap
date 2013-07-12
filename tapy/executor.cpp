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


#include "executor.hpp"

#include <cassert>
#include <cstddef>

#include <tapy/context.hpp>
#include <tapy/objects/bool.hpp>
#include <tapy/objects/bytes.hpp>
#include <tapy/objects/code.hpp>
#include <tapy/objects/dict.hpp>
#include <tapy/objects/function.hpp>
#include <tapy/objects/module.hpp>
#include <tapy/objects/none.hpp>
#include <tapy/objects/object.hpp>
#include <tapy/objects/tuple.hpp>
#include <tapy/opcodes.hpp>
#include <tapy/trace.hpp>

using namespace tap;

#define MAX_BLOCKS 20

namespace tapy {

Executor::Executor(const CodeObject *code):
	m_context(GetContext()),
	m_code(code),
	m_dict(DictObject::New()),
	m_running(true),
	m_position(0)
{
	m_stack.reserve(m_code->stacksize());
}

Executor::Executor(const CodeObject *code, DictObject *dict):
	m_context(GetContext()),
	m_code(code),
	m_dict(dict),
	m_running(true),
	m_position(0)
{
	m_stack.reserve(m_code->stacksize());
}

Executor::Executor(const TupleObject *args):
	m_context(GetContext()),
	m_code(NULL),
	m_dict(DictObject::New()),
	m_running(true),
	m_position(0)
{
	const uint8_t bytecode[] = {
		OpCallFunction,
		uint8_t(args->length() - 1),
		0,
		OpReturnValue,
	};

	auto bytes = BytesObject::New(bytecode, sizeof (bytecode));
	m_code = CodeObject::New(3, bytes, TupleObject::New(), TupleObject::New(), TupleObject::New());

	for (uint32_t i = 0; i < args->length(); i++)
		push(args->get(i));

	m_stack.reserve(m_code->stacksize());
}

Object *Executor::execute()
{
	do {
		assert(&GetContext() == &m_context);
		execute_op();
	} while (m_running);

	return pop();
}

#ifdef TAPY_TRACE
# define CASE(op) case op: Trace(#op);
#else
# define CASE(op) case op:
#endif

void Executor::execute_op()
{
	auto op = load_bytecode<uint8_t>();

	switch (op) {
	CASE(OpPopTop)              return op_pop_top();
	CASE(OpNop)                 return op_nop();
	CASE(OpBinaryAdd)           return op_binary_add();
	CASE(OpGetIter)             return op_get_iter();
	CASE(OpPopBlock)            return op_pop_block();
	CASE(OpReturnValue)         return op_return_value();
	}

	auto arg = load_bytecode<uint16_t>();

	switch (op) {
	CASE(OpStoreName)           return op_store_name(arg);
	CASE(OpForIter)             return op_for_iter(arg);
	CASE(OpLoadConst)           return op_load_const(arg);
	CASE(OpLoadName)            return op_load_name(arg);
	CASE(OpLoadAttr)            return op_load_attr(arg);
	CASE(OpCompareOp)           return op_compare_op(arg);
	CASE(OpImportName)          return op_import_name(arg);
	CASE(OpImportFrom)          return op_import_from(arg);
	CASE(OpJumpForward)         return op_jump_forward(arg);
	CASE(OpJumpIfFalseOrPop)    return op_jump_if_false_or_pop(arg);
	CASE(OpJumpIfTrueOrPop)     return op_jump_if_true_or_pop(arg);
	CASE(OpJumpAbsolute)        return op_jump_absolute(arg);
	CASE(OpPopJumpIfFalse)      return op_pop_jump_if_false(arg);
	CASE(OpPopJumpIfTrue)       return op_pop_jump_if_true(arg);
	CASE(OpLoadGlobal)          return op_load_global(arg);
	CASE(OpSetupLoop)           return op_setup_loop(arg);
	CASE(OpLoadFast)            return op_load_fast(arg);
	CASE(OpStoreFast)           return op_store_fast(arg);
	CASE(OpCallFunction)        return op_call_function(arg);
	CASE(OpMakeFunction)        return op_make_function(arg);
	}
	
	Trace("%1%: unsupported opcode: %2%", __func__, int(op));

	throw std::runtime_error("unsupported opcode");
}

#undef CASE

template <typename T>
T Executor::load_bytecode()
{
	auto bytecode = m_code->bytecode();

	if (m_position + sizeof (T) > bytecode->length())
		throw std::runtime_error("Executor frame ran out of bytecode");

	const uint8_t *ptr = bytecode->data() + m_position;
	m_position += sizeof (T);

	return *reinterpret_cast<const T *> (ptr);
}

void Executor::jump_absolute(unsigned int target)
{
	if (target > m_code->bytecode()->length())
		throw std::runtime_error("Jump target outside executor frame bytecode");

	m_position = target;
}

void Executor::jump_forward(unsigned int delta)
{
	jump_absolute(m_position + delta);
}

void Executor::push(Object *object)
{
	if (m_stack.size() >= m_code->stacksize())
		throw std::runtime_error("stack overflow");

	m_stack.push_back(object);
}

Object *Executor::peek() const
{
	if (m_stack.empty())
		throw std::runtime_error("empty stack");

	return m_stack.back();
}

Object *Executor::pop()
{
	if (m_stack.empty())
		throw std::runtime_error("stack underflow");

	auto object = m_stack.back();
	m_stack.pop_back();

	return object;
}

void Executor::push_block(unsigned int delta)
{
	if (m_blocks.size() >= MAX_BLOCKS)
		throw std::runtime_error("too many nested blocks");

	Block block = { m_position, delta };
	m_blocks.push_back(block);
}

const Executor::Block &Executor::peek_block() const
{
	if (m_blocks.empty())
		throw std::runtime_error("no active block");

	return m_blocks.back();
}

void Executor::pop_block()
{
	if (m_blocks.empty())
		throw std::runtime_error("no block to pop");

	m_blocks.pop_back();
}

void Executor::op_pop_top()
{
	pop();
}

void Executor::op_nop()
{
}

void Executor::op_rot_two()
{
	// TODO: optimize

	auto top0 = pop();
	auto top1 = pop();

	push(top0);
	push(top1);
}

void Executor::op_rot_three()
{
	// TODO: optimize

	auto top0 = pop();
	auto top1 = pop();
	auto top2 = pop();

	push(top0);
	push(top2);
	push(top1);
}

void Executor::op_rot_four()
{
	// TODO: optimize

	auto top0 = pop();
	auto top1 = pop();
	auto top2 = pop();
	auto top3 = pop();

	push(top0);
	push(top3);
	push(top2);
	push(top1);
}

void Executor::op_dup_top()
{
	push(peek());
}

void Executor::op_binary_add()
{
	auto b = pop();
	auto a = pop();
	push(a->add(b));
}

void Executor::op_get_iter()
{
	auto iterable = pop();
	push(iterable->iter());
}

void Executor::op_pop_block()
{
	pop_block();
}

void Executor::op_return_value()
{
	m_running = false;
}

void Executor::op_store_name(unsigned int namei)
{
	auto name = m_code->names()->get(namei);
	auto value = pop();
	m_dict->set(name, value);
}

void Executor::op_for_iter(unsigned int delta)
{
	try {
		push(peek()->next());
	} catch (const StopIteration &) {
		pop();
		jump_forward(delta);
	}
}

void Executor::op_load_const(unsigned int consti)
{
	push(m_code->consts()->get(consti));
}

void Executor::op_load_name(unsigned int namei)
{
	auto name = m_code->names()->get(namei);
	Object *value;

	try {
		value = m_dict->get(name);
	} catch (...) {
		value = m_context.load_builtin_name(name);
	}

	push(value);
}

void Executor::op_load_attr(unsigned int namei)
{
	push(pop()->getattribute(m_code->names()->get(namei)));
}

void Executor::op_compare_op(unsigned int opname)
{
	auto right = pop();
	auto left = pop();

	switch (opname) {
	case CompareLT:
		return push(left->lt(right));

	case CompareLE:
		return push(left->le(right));

	case CompareEQ:
		return push(left->eq(right));

	case CompareNE:
		return push(left->ne(right));

	case CompareGT:
		return push(left->gt(right));

	case CompareGE:
		return push(left->ge(right));

	case CompareIn:
		return push(left->contains(right));

	case CompareNotIn:
		return push(Bool(!left->contains(right)->bool_()->value()));

	case CompareIs:
		return push(Bool(left == right));

	case CompareIsNot:
		return push(Bool(left != right));

	case CompareException:
		throw std::runtime_error("CompareException not implemented");

	default:
		throw std::runtime_error("unknown comparison");
	}
}

void Executor::op_import_name(unsigned int namei)
{
	auto from = pop();
	auto level = Require<IntObject>(pop());
	auto name = m_code->names()->get(namei);
	auto module = m_context.import_builtin_module(name);

	if (!from->dynamic<NoneObject>()) {
		auto fromlist = Require<TupleObject>(from);
		assert(fromlist->length() == 1);
		push(module->getattribute(fromlist->get(0)));
	}

	push(module);
}

void Executor::op_import_from(unsigned int namei)
{
	push(pop()->getattribute(m_code->names()->get(namei)));
}

void Executor::op_jump_forward(unsigned int delta)
{
	jump_forward(delta);
}

void Executor::op_jump_if_false_or_pop(unsigned int target)
{
	if (peek()->bool_()->value())
		pop();
	else
		jump_absolute(target);
}

void Executor::op_jump_if_true_or_pop(unsigned int target)
{
	if (peek()->bool_()->value())
		jump_absolute(target);
	else
		pop();
}

void Executor::op_jump_absolute(unsigned int target)
{
	jump_absolute(target);
}

void Executor::op_pop_jump_if_false(unsigned int target)
{
	if (!pop()->bool_()->value())
		jump_absolute(target);
}

void Executor::op_pop_jump_if_true(unsigned int target)
{
	if (pop()->bool_()->value())
		jump_absolute(target);
}

void Executor::op_load_global(unsigned int namei)
{
	auto name = m_code->names()->get(namei);
	push(m_context.load_builtin_name(name));
}

void Executor::op_setup_loop(unsigned int delta)
{
	push_block(delta);
}

void Executor::op_load_fast(unsigned int var_num)
{
	push(m_dict->get(m_code->varnames()->get(var_num)));
}

void Executor::op_store_fast(unsigned int var_num)
{
	m_dict->set(m_code->varnames()->get(var_num), pop());
}

void Executor::op_call_function(uint16_t portable_argc)
{
	unsigned int argc = Port(portable_argc) & 0xff;
	unsigned int kwargc = Port(portable_argc) >> 8;

	auto args = TupleObject::NewUninit(argc);
	auto kwargs = DictObject::New();

	for (unsigned int i = 0; i < kwargc; i++) {
		auto value = pop();
		auto key = pop();

		kwargs->set(key, value);
	}

	for (unsigned int i = argc; i-- > 0; )
		args->init(i, pop());

	push(pop()->call(args, kwargs));
}

void Executor::op_make_function(uint16_t portable_argc)
{
	if (portable_argc)
		throw std::runtime_error("default function arguments not supported");

	auto name = Require<StrObject>(pop());
	auto code = Require<CodeObject>(pop());
	push(FunctionObject::New(name, code));
}

} // namespace
