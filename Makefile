-include config.mk

export PYTHON	:= python3

CPPFLAGS	+= -I. -Iinclude -DBOOST_USE_SEGMENTED_STACKS -DTAPY_TRACE
CFLAGS		+= -g -Wall -Wextra -Wno-unused-parameter -pthread -fsplit-stack \
		   -O -fno-cprop-registers -fno-defer-pop -fno-split-wide-types -fno-tree-bit-ccp \
		      -fno-tree-ch -fno-tree-copyrename -fno-tree-dominator-opts -fno-tree-forwprop \
		      -fno-tree-fre -fno-tree-phiprop -fno-tree-pta -fno-tree-sra -fno-tree-ter \
		   -fcaller-saves -fdelete-null-pointer-checks -fdevirtualize -findirect-inlining \
		   -finline-functions -fipa-cp-clone -foptimize-sibling-calls -fpartial-inlining \
		   -fpeephole2
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
