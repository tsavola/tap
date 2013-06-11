-include config.mk

export PYTHON	:= python3

CPPFLAGS	+= -I. -Iinclude -DBOOST_USE_SEGMENTED_STACKS -DTAPY_TRACE
CFLAGS		+= -g -Wall -Wextra -Wno-unused-parameter -pthread -fsplit-stack
CCFLAGS		+= -std=c99
CXXFLAGS	+= -std=c++0x

ifneq ($(BOOST),)
CPPFLAGS	+= -I$(BOOST)/include
LDFLAGS		+= -L$(BOOST)/lib
endif

LIBRARIES	:= tap tapy
TESTS		:= test/glib test/c++ test/tapy example

build: $(TESTS)

test/glib: tap-static
test/c++: tap-static
test/tapy: tapy-static

example: tap-static tapy-static

include build/project.mk
