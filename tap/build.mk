NAME		:= tap
SOURCES		:= $(wildcard tap/*.cpp)
CFLAGS		+= -fvisibility=hidden -fvisibility-inlines-hidden

include build/library.mk
