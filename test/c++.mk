NAME		:= test-c++
SOURCES		:= test/test-c++.cpp
DEPENDS		+= $(O)/lib/libtap.a
LDFLAGS		+= -L$(O)/lib
LIBS		+= -ltap

include build/test.mk
