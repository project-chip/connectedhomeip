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

#
# Makefile for building CHIP OSAL library and tests in isolation.
#
# Usage:
#   make -f Makefile.mk          # Build a local POSIX (linux or darwin) version of tests
#   make -f Makefile.mk test     # Build and run all tests
#   make -f Makefile.mk clean    # Clean up all built code, tests, and dependecies
#
#   # Build FreeRTOS version of tests (for nRF52840)
#   DEBUG=1 V=1 PLATFORM=nrf52840 make -f Makefile.mk
#

PROJ_ROOT = ../../..

### ===== Platforms =====

# Map supported platforms to dedicated build scripts

ifeq ($(PLATFORM),nrf52840)
include Makefile.freertos-nrf52840
endif

### ===== Host System Variants =====

# Default to PLATFORM=posix if no platform was specified
PLATFORM ?= posix

# Hook to allow platforms to add helpers for alternate output targets like .hex
TARGET_EXT ?= exe

ifeq ($(PLATFORM),posix)
TARGET_OSAL ?= posix

# Linux-specific POSIX build
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Linux)
TARGET_LDFLAGS = -lpthread -lstdc++ -lrt
endif

# Darwin-specific POSIX build
ifeq ($(UNAME_S),Darwin)
TARGET_LDFLAGS = -lpthread -lstdc++
endif

endif # PLATFORM == posix

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
    -I$(PROJ_ROOT)/src/osal/$(TARGET_OSAL)            \
    -I$(PROJ_ROOT)/third_party/nlassert/repo/include  \
    $(TARGET_INCLUDES)                                \
    $(NULL)

DEBUG ?= 0

DEFINES =                   \
	-DDEBUG=${DEBUG}        \
    -D_GNU_SOURCE           \
    $(NULL)

CFLAGS =                    \
    $(INCLUDES) $(DEFINES)  \
    $(TARGET_CCFLAGS)       \
    $(NULL)

LDFLAGS = $(TARGET_LDFLAGS)

ifeq ($(DEBUG),1)
CFLAGS += -g -O0
else
CFLAGS += -Os
endif


### ===== Sources =====

OSAL_PATH = $(PROJ_ROOT)/src/osal/$(TARGET_OSAL)

SRCS  = $(shell find $(OSAL_PATH) -maxdepth 1 -name '*.c')
SRCS += $(shell find $(OSAL_PATH) -maxdepth 1 -name '*.cc')
SRCS += $(shell find $(OSAL_PATH) -maxdepth 1 -name '*.cpp')
SRCS += $(TARGET_SRCS)

OBJS  = $(patsubst %.c, %.o,$(filter %.c,  $(SRCS)))
OBJS += $(patsubst %.cc,%.o,$(filter %.cc, $(SRCS)))
OBJS += $(patsubst %.cpp,%.o,$(filter %.cpp, $(SRCS)))
OBJS += $(patsubst %.S, %.o,$(filter %.S,  $(SRCS)))
OBJS += $(TARGET_OBJS)

TEST_SRCS  = $(shell find . -maxdepth 1 -name '*.c')
TEST_SRCS += $(shell find . -maxdepth 1 -name '*.cc')
TEST_SRCS += $(shell find . -maxdepth 1 -name '*.cpp')
TEST_SRCS += $(shell find . -maxdepth 1 -name '*.S')

TEST_OBJS  = $(patsubst %.c, %.o,$(filter %.c,  $(SRCS)))
TEST_OBJS += $(patsubst %.cc,%.o,$(filter %.cc, $(SRCS)))
TEST_OBJS += $(patsubst %.cpp,%.o,$(filter %.cpp, $(SRCS)))
TEST_OBJS += $(patsubst %.S, %.o,$(filter %.S,  $(SRCS)))

### ===== Rules =====

all: depend                       \
     test_os_task.$(TARGET_EXT)   \
     test_os_queue.$(TARGET_EXT)  \
     test_os_mutex.$(TARGET_EXT)  \
     test_os_sem.$(TARGET_EXT)    \
     test_os_timer.$(TARGET_EXT)  \
     test_ring.$(TARGET_EXT)      \
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

%.o: %.S
	$(CPP) -c $(CFLAGS) $< -o $@