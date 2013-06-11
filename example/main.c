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

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <tapy.h>

int main(int argc, char **argv)
{
	const char *filename = EXAMPLE_BYTECODE;

	if (argc > 1)
		filename = argv[1];

	FILE *file = fopen(filename, "r");
	if (file == NULL)
		return 1;

	if (fseek(file, 0, SEEK_END) < 0)
		return 2;

	long size = ftell(file);
	if (size < 0)
		return 3;

	if (fseek(file, 0, SEEK_SET) < 0)
		return 4;

	uint8_t *data = malloc(size);
	if (data == NULL)
		return 5;

	if (fread(data, 1, size, file) != (size_t) size)
		return 6;

	fclose(file);

	if (size <= 12)
		return 7;

	data += 12;
	size -= 12;

	if (tapy_init() < 0)
		return 10;

	TapyContext *context = tapy_context_new();
	if (context == NULL)
		return 11;

	if (tapy_execute(data, size) < 0)
		return 12;

	tapy_context_destroy(context);

	return 0;
}
