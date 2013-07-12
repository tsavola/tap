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

#ifndef TAPY_EXECUTOR_HPP
#define TAPY_EXECUTOR_HPP

#include <vector>

#include <tap.hpp>

class TapyContext;

namespace tapy {

class Callable;
class CodeObject;
class DictObject;
class Executor;
class Object;
class TupleObject;

/**
 * Executor for a function call stack.  Implements a Python bytecode
 * interpreter.
 */
class Executor {
public:
	/**
	 * Create an executor for a Python module.
	 */
	Executor(const CodeObject *code);

	/**
	 * Create an executor for a Python function.
	 */
	Executor(const CodeObject *code, DictObject *dict);

	/**
	 * Create an executor for a Python callable.
	 * @param args contains a Callable followed by its arguments
	 */
	Executor(const TupleObject *args);

	/**
	 * Execute the bytecode.
	 * @return result
	 */
	Object *execute();

	/**
	 * Visit all object references in the call stack.
	 */
	template <typename Visitor>
	void visit(Visitor &v) const;

private:
	/**
	 * @struct tapy::Executor::Block
	 * Represents an active control block within a function.
	 */
	struct Block {
		unsigned int position;
		unsigned int delta;
	};

	/**
	 * Read from and advance the current bytecode position.
	 */
	template <typename T>
	T load_bytecode();

	/**
	 * Replace the current bytecode position.
	 */
	void jump_absolute(unsigned int target);

	/**
	 * Advance the current bytecode position.
	 */
	void jump_forward(unsigned int delta);

	/**
	 * Add an object at the top of the object stack.
	 */
	void push(Object *object);

	/**
	 * Refer to the object at the top of the object stack.
	 */
	Object *peek() const;

	/**
	 * Remove and return the object at the top of the object stack.
	 */
	Object *pop();

	/**
	 * Add a Block referring to the current bytecode position at the top of
	 * the block stack.
	 */
	void push_block(unsigned int delta);

	/**
	 * Direct access to the Block at the top of the block stack.
	 * @return short-term reference to Arena memory
	 */
	const Block &peek_block() const;

	/**
	 * Remove the Block at the top of the block stack.
	 */
	void pop_block();

	/**
	 * Execute the next bytecode instruction.
	 */
	void execute_op();

	void op_pop_top();
	void op_nop();
	void op_rot_two();
	void op_rot_three();
	void op_rot_four();
	void op_dup_top();
	void op_binary_add();
	void op_get_iter();
	void op_pop_block();
	void op_return_value();
	void op_store_name(unsigned int namei);
	void op_for_iter(unsigned int delta);
	void op_load_const(unsigned int consti);
	void op_load_name(unsigned int namei);
	void op_load_attr(unsigned int namei);
	void op_compare_op(unsigned int opname);
	void op_import_name(unsigned int namei);
	void op_import_from(unsigned int namei);
	void op_jump_forward(unsigned int delta);
	void op_jump_if_false_or_pop(unsigned int target);
	void op_jump_if_true_or_pop(unsigned int target);
	void op_jump_absolute(unsigned int target);
	void op_pop_jump_if_false(unsigned int target);
	void op_pop_jump_if_true(unsigned int target);
	void op_load_global(unsigned int delta);
	void op_setup_loop(unsigned int delta);
	void op_load_fast(unsigned int var_num);
	void op_store_fast(unsigned int var_num);
	void op_call_function(uint16_t argc);
	void op_make_function(uint16_t argc);

	Executor(const Executor &);
	void operator=(const Executor &);

	TapyContext &m_context;
	const CodeObject *m_code;
	DictObject *const m_dict;
	bool m_running;
	unsigned int m_position;
	std::vector<Object *> m_stack;
	std::vector<Block> m_blocks;
};

} // namespace

#endif
