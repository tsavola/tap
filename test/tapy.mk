NAME		:= test-tapy
SOURCES		:= test/test-tapy.cpp
DEPENDS		+= $(O)/lib/libtap.a $(O)/lib/libtapy.a
LDFLAGS		+= -L$(O)/lib
LIBS		+= -Wl,-Bstatic -ltapy -ltap -lboost_coroutine -lboost_context -Wl,-Bdynamic

include build/test.mk
