#
#    Copyright (c) 2020 Project CHIP Authors
#    Copyright (c) 2018 Google LLC
#
#    Licensed under the Apache License, Version 2.0 (the "License");
#    you may not use this file except in compliance with the License.
#    You may obtain a copy of the License at
#
#        http://www.apache.org/licenses/LICENSE-2.0
#
#    Unless required by applicable law or agreed to in writing, software
#    distributed under the License is distributed on an "AS IS" BASIS,
#    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#    See the License for the specific language governing permissions and
#    limitations under the License.
#

# Makefile

PROJ_ROOT = ../../..

### ===== Host System Variants =====

UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Linux)
endif

### ===== Toolchain =====

CROSS_COMPILE ?=

ifeq ($(UNAME_S),Darwin)
CC  = ccache $(CROSS_COMPILE)llvm-gcc
CPP = ccache $(CROSS_COMPILE)llvm-g++
LD  = $(CROSS_COMPILE)ld
AR  = $(CROSS_COMPILE)ar
else
CC  = ccache $(CROSS_COMPILE)gcc
CPP = ccache $(CROSS_COMPILE)g++
LD  = $(CROSS_COMPILE)g++
AR  = $(CROSS_COMPILE)ar
endif


### ===== Compiler Flags =====

INCLUDES =                                            \
    -I.                                               \
    -I$(PROJ_ROOT)/src/include                        \
    -I$(PROJ_ROOT)/src/osal/include                   \
    -I$(PROJ_ROOT)/src/osal/posix                     \
    -I$(PROJ_ROOT)/third_party/nlassert/repo/include  \
    $(NULL)

DEBUG ?= 0

DEFINES =                   \
	-DDEBUG=${DEBUG}        \
    -D_GNU_SOURCE           \
    $(NULL)

CFLAGS =                    \
    $(INCLUDES) $(DEFINES)  \
    -g                      \
    -O0                     \
    $(NULL)

LIBS = -lpthread -lstdc++
ifeq ($(UNAME_S),Linux)
LIBS += -lrt
endif

LDFLAGS =

### ===== Sources =====

OSAL_PATH = $(PROJ_ROOT)/src/osal/posix

SRCS  = $(shell find $(OSAL_PATH) -maxdepth 1 -name '*.c')
SRCS += $(shell find $(OSAL_PATH) -maxdepth 1 -name '*.cc')
SRCS += $(shell find $(OSAL_PATH) -maxdepth 1 -name '*.cpp')

OBJS  = $(patsubst %.c, %.o,$(filter %.c,  $(SRCS)))
OBJS += $(patsubst %.cc,%.o,$(filter %.cc, $(SRCS)))
OBJS += $(patsubst %.cpp,%.o,$(filter %.cpp, $(SRCS)))

TEST_SRCS  = $(shell find . -maxdepth 1 -name '*.c')
TEST_SRCS += $(shell find . -maxdepth 1 -name '*.cc')
TEST_SRCS += $(shell find . -maxdepth 1 -name '*.cpp')

TEST_OBJS  = $(patsubst %.c, %.o,$(filter %.c,  $(SRCS)))
TEST_OBJS += $(patsubst %.cc,%.o,$(filter %.cc, $(SRCS)))
TEST_OBJS += $(patsubst %.cpp,%.o,$(filter %.cpp, $(SRCS)))

### ===== Rules =====

all: depend                  \
     test_os_task.exe        \
     test_os_queue.exe       \
     test_os_mutex.exe       \
     test_os_sem.exe         \
     test_os_timer.exe       \
     test_ring.exe           \
     $(NULL)

test_os_task.exe: test_os_task.o $(OBJS)
	$(LD) -o $@ $^ $(LDFLAGS) $(LIBS)

test_os_queue.exe: test_os_queue.o $(OBJS)
	$(LD) -o $@ $^ $(LDFLAGS) $(LIBS)

test_os_mutex.exe: test_os_mutex.o $(OBJS)
	$(LD) -o $@ $^ $(LDFLAGS) $(LIBS)

test_os_sem.exe: test_os_sem.o $(OBJS)
	$(LD) -o $@ $^ $(LDFLAGS) $(LIBS)

test_os_timer.exe: test_os_timer.o $(OBJS)
	$(LD) -o $@ $^ $(LDFLAGS) $(LIBS)

test_ring.exe: test_ring.o $(OBJS)
	$(LD) -o $@ $^ $(LDFLAGS) $(LIBS)

test: all
	./test_os_task.exe
	./test_os_queue.exe
	./test_os_mutex.exe
	./test_os_sem.exe
	./test_os_timer.exe
	./test_ring.exe

show_objs:
	@echo $(OBJS)

### ===== Clean =====
clean:
	@echo "Cleaning artifacts."
	rm *~ .depend $(OBJS) *.o *.exe

### ===== Dependencies =====
### Rebuild if headers change
depend: .depend

.depend: $(SRCS) $(TEST_SRCS)
	@echo "Building dependencies."
	rm -f ./.depend
	$(CC) $(CFLAGS) -MM $^ > ./.depend;

include .depend

### Generic rules based on extension
%.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@

%.o: %.cc
	$(CPP) -c $(CFLAGS) $< -o $@

%.o: %.cpp
	$(CPP) -c $(CFLAGS) $< -o $@
