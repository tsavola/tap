NAME		:= tapy
SOURCES		:= $(wildcard tapy/*.cpp tapy/*/*.cpp)
DEPENDS		+= $(O)/lib/libtap.so
CFLAGS		+= -fvisibility=hidden -fvisibility-inlines-hidden

include build/library.mk
