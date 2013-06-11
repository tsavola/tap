NAME		:= test-glib
SOURCES		:= test/test-glib.c
DEPENDS		+= $(O)/lib/libtap.a
LDFLAGS		+= -L$(O)/lib
LIBS		+= -ltap -lstdc++
PKGS		+= glib-2.0

include build/test.mk
