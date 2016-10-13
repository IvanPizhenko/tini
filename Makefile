#########################################################################################
#
# TinyINI - small and simple open-source library for loading, saving and 
# managing INI file data structures in the memory.
#
# TinyINI is distributed under following terms and conditions:
#
# Copyright (c) 2015-2016, Ivan Pizhenko.
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#     * Redistributions of source code must retain the above copyright
#       notice, this list of conditions and the following disclaimer.
#     * Redistributions in binary form must reproduce the above copyright
#       notice, this list of conditions and the following disclaimer in the
#       documentation and/or other materials provided with the distribution.
#     * Neither the name of the copyright holder nor the names of its contributors
#       may be used to endorse or promote products derived from this software
#       without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ''AS IS''
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL BEN HOYT BE LIABLE FOR ANY
# DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
# (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
# ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
# SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
# 
# SPECIAL NOTICE
# TinyINI library relies on the open-source INIH library 
# (https://github.com/benhoyt/inih) for parsing text of INI file.
# Source code of INIH library and information about it, including 
# licensing conditions, is included in the subfolder inih.
#
#########################################################################################

# Makefile for TINI library

.PHONY: all build clean

TARGET=libtini.a
SRC:=inih/ini.c tini.c
OBJ:=$(SRC:.c=.o)
DEP:=$(OBJ:.o=.d)

CC:=gcc
AR:=ar

INCLUDES:=
DEFS:=-DINI_MAX_LINE=1024 -DINI_USE_STACK=0 -DTINI_FEATURE_GET_PARAMETERS_STORAGE
CFLAGS:=-std=gnu90 -fPIC -fmax-errors=3 -Wall -Wextra -Werror $(DEFS) $(INCLUDES)
CPPFLAGS:=-MMD -MP
ARFLAGS:=

ifeq ("$(DEBUG)", "1")
CFLAGS+=-g3 -Og -DDEBUG -D_DEBUG
else
CFLAGS+=-O2 -ffast-math -DNDEBUG
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

