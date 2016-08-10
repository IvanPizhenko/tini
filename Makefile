# Our custom Makefile for INIH library

.PHONY: all build clean

TARGET=libtini.a
SRC:=inih/ini.c tini.c
OBJ:=$(SRC:.c=.o)
DEP:=$(OBJ:.o=.d)

CC:=gcc
AR:=ar

INCLUDES:=-I./include
DEFS:=-DINI_MAX_LINE=1024 -DINI_USE_STACK=0 -DTINI_FEATURE_GET_PARAMETERS_STORAGE
CFLAGS:=-std=gnu90 -fPIC -fmax-errors=3 -Wall -Wextra -Werror $(DEFS) $(INCLUDES)
CPPFLAGS:=-MMD -MP
ARFLAGS:=

ifeq ("$(DEBUG)", "1")
CFLAGS+=-g3 -Og -DDEBUG -D_DEBUG
else
CFLAGS+=-Os -DNDEBUG
endif

all: build

build: $(TARGET)

clean:
	echo Cleaning $(TARGET)...
	-rm -f $(TARGET)
	-rm -f *.o inih/*.o
	-rm -f *.d inih/*.o

-include $(DEP)

$(TARGET): $(OBJ)
	ar rvs $(ARFLAGS) $@ $^

