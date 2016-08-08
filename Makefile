# Our custom Makefile for INIH library

.PHONY: all build clean

TARGET=libtini.a
C_SOURCES:=inih/ini.c tini.c
OBJECTS:=$(patsubst %.c,%.o,$(C_SOURCES))

CC:=gcc
AR:=ar

CFLAGS:=-std=gnu90 -fPIC -fmax-errors=3 -Wall -Wextra -Werror
INCLUDES:=-I./include
DEFS:=-DINI_MAX_LINE=1024 -DINI_USE_STACK=0
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
	-rm -f *.o
	-rm -f inih/*.o

$(TARGET): $(OBJECTS)
	ar rvs $(ARFLAGS) $@ $^

%.o: %.c
	$(CC) -o $@ $(INCLUDES) $(CFLAGS) $(DEFS) -c $<
