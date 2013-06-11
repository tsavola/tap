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

#ifndef TAPY_OPCODES_HPP
#define TAPY_OPCODES_HPP

namespace tapy {

/**
 * Python opcode numbers.
 */
enum Opcode {
	OpPopTop              = 1,
	OpRotTwo              = 2,
	OpRotThree            = 3,
	OpRotFour             = 4,
	OpDupTop              = 5,
	OpNop                 = 9,

	OpBinaryAdd           = 23,

	OpGetIter             = 68,

	OpReturnValue         = 83,

	OpPopBlock            = 87,

	OpStoreName           = 90,
	OpForIter             = 93,

	OpLoadConst           = 100,
	OpLoadName            = 101,
	OpLoadAttr            = 106,
	OpCompareOp           = 107,
	OpImportName          = 108,
	OpImportFrom          = 109,

	OpJumpForward         = 110,
	OpJumpIfFalseOrPop    = 111,
	OpJumpIfTrueOrPop     = 112,
	OpJumpAbsolute        = 113,
	OpPopJumpIfFalse      = 114,
	OpPopJumpIfTrue       = 115,

	OpLoadGlobal          = 116,

	OpSetupLoop           = 120,

	OpLoadFast            = 124,
	OpStoreFast           = 125,

	OpCallFunction        = 131,
	OpMakeFunction        = 132,
};

/**
 * Python comparison types.
 */
enum CompareOp {
	CompareLT, CompareLE,
	CompareEQ, CompareNE,
	CompareGT, CompareGE,
	CompareIn, CompareNotIn,
	CompareIs, CompareIsNot,
	CompareException,
};

} // namespace

#endif
