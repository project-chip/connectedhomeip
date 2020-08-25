#
#   Copyright (c) 2020 Project CHIP Authors
#   Copyright (c) 2020 Google LLC.
#   All rights reserved.
#
#   Licensed under the Apache License, Version 2.0 (the "License");
#   you may not use this file except in compliance with the License.
#   You may obtain a copy of the License at
#
#       http://www.apache.org/licenses/LICENSE-2.0
#
#   Unless required by applicable law or agreed to in writing, software
#   distributed under the License is distributed on an "AS IS" BASIS,
#   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#   See the License for the specific language governing permissions and
#   limitations under the License.
#

#
#   @file
#         Common makefile definitions for building applications based
#         on the NXP K32W061 SDK.
#
#
#   This makefile is primarily intended to support building the
#   CHIP example applications on NXP platforms.  However
#   external developers should feel free to use it if they find
#   it useful.
#
#   To build an application using this makefile, include the file
#   in a project-specific Makefile, define make variables describing
#   the application and how it should be built, and then call the
#   GenerateBuildRules function.  E.g.:
#
#       PROJECT_ROOT = $(realpath .)
#
#       BUILD_SUPPORT_DIR = $(PROJECT_ROOT)/third_party/connectedhomeip/config/k32w061
#
#       include $(BUILD_SUPPORT_DIR)/k32w061-app.mk
#
#       APP := chip-k32w061-bringup
#
#       SRCS = \
#           $(PROJECT_ROOT)/main.cpp \
#           ...
#
#       INC_DIRS = \
#           $(PROJECT_ROOT) \
#           ...
#
#       LIBS = \
#           ...
#
#       DEFINES = \
#           ...
#
#       $(call GenerateBuildRules)
#
# ==================================================
# Sanity Checks
# ==================================================

ifndef K32W061_SDK_ROOT
$(error ENVIRONMENT ERROR: K32W061_SDK_ROOT not set)
endif
ifndef GNU_INSTALL_ROOT
$(error ENVIRONMENT ERROR: GNU_INSTALL_ROOT not set)
endif

# ==================================================
# General definitions
# ==================================================

.DEFAULT_GOAL := all

APP_EXE = $(OUTPUT_DIR)/$(APP).out
APP_ELF = $(OUTPUT_DIR)/$(APP).elf

SRCS =
ALL_SRCS = $(SRCS) $(EXTRA_SRCS)
OBJS = $(foreach file,$(ALL_SRCS),$(call ObjFileName,$(file)))

OUTPUT_DIR = $(PROJECT_ROOT)/build
OBJS_DIR = $(OUTPUT_DIR)/objs
DEPS_DIR = $(OUTPUT_DIR)/deps

FLOAT_ABI = -mfloat-abi=soft

STD_CFLAGS = \
    -Wall \
     $(FLOAT_ABI) \
    -mcpu=cortex-m4 \
    -mthumb \
    -mabi=aapcs \
    -ffunction-sections \
    -fdata-sections \
    -fno-strict-aliasing \
    -fshort-enums \
    --specs=nosys.specs

STD_CXXFLAGS = \
    -fno-rtti \
    -fno-exceptions \
    -fno-unwind-tables

STD_ASFLAGS = \
     $(FLOAT_ABI) \
    -g3 \
    -mcpu=cortex-m4 \
    -mthumb \
    -mabi=aapcs \
    -x assembler-with-cpp

STD_LDFLAGS = \
     $(FLOAT_ABI) \
    -mthumb \
    -mabi=aapcs \
    -mcpu=cortex-m4 \
    -Wl,--gc-sections \
    --specs=nosys.specs \
    $(foreach dir,$(LINKER_SCRIPT_INC_DIRS),-L$(dir)) \
    -T$(LINKER_SCRIPT)

STD_LIBS = \
    -lc \
    -lstdc++ \
    -lnosys \
    -lm

STD_INC_DIRS += \
    $(K32W061_SDK_ROOT)/CMSIS/Include                                                       \
    $(K32W061_SDK_ROOT)/devices/K32W061                                                     \
    $(K32W061_SDK_ROOT)/devices/K32W061/drivers                                             \
    $(K32W061_SDK_ROOT)/devices/K32W061/utilities                                           \
    $(K32W061_SDK_ROOT)/devices/K32W061/utilities/debug_console                             \
    $(K32W061_SDK_ROOT)/devices/K32W061/utilities/str                                       \
    $(CHIP_ROOT)/src/platform/K32W

STD_DEFINES = \
    HAVE_CONFIG_H

STD_COMPILE_PREREQUISITES = k32w061-sdk-check

STD_LINK_PREREQUISITES = k32w061-sdk-check

DEFINE_FLAGS = $(foreach def,$(STD_DEFINES) $(DEFINES),-D$(def))

INC_FLAGS = $(foreach dir,$(INC_DIRS) $(STD_INC_DIRS),-I$(dir))

LINKER_SCRIPT = $(realpath $(PROJECT_ROOT))/$(APP).ld

LINKER_SCRIPT_INC_DIRS =

K32W061_SDK_TITLE = K32W061 SDK for Thread

# ==================================================
# Toolchain and external utilities / files
# ==================================================

TARGET_TUPLE = arm-none-eabi

CC      = $(GNU_INSTALL_ROOT)/$(TARGET_TUPLE)-gcc
CXX     = $(GNU_INSTALL_ROOT)/$(TARGET_TUPLE)-c++
CPP     = $(GNU_INSTALL_ROOT)/$(TARGET_TUPLE)-cpp
AS      = $(GNU_INSTALL_ROOT)/$(TARGET_TUPLE)-as
AR      = $(GNU_INSTALL_ROOT)/$(TARGET_TUPLE)-ar
LD      = $(GNU_INSTALL_ROOT)/$(TARGET_TUPLE)-ld
NM      = $(GNU_INSTALL_ROOT)/$(TARGET_TUPLE)-nm
OBJDUMP = $(GNU_INSTALL_ROOT)/$(TARGET_TUPLE)-objdump
OBJCOPY = $(GNU_INSTALL_ROOT)/$(TARGET_TUPLE)-objcopy
SIZE    = $(GNU_INSTALL_ROOT)/$(TARGET_TUPLE)-size
RANLIB  = $(GNU_INSTALL_ROOT)/$(TARGET_TUPLE)-ranlib

INSTALL = /usr/bin/install
INSTALLFLAGS = -C -v

ifneq (, $(shell which ccache))
CCACHE = ccache
endif

K32W061_PROG = $(K32W061_SDK_ROOT)/tools/imagetool/DK6Programmer.exe

# ==================================================
# Build options
# ==================================================

DEBUG ?= 1
OPT ?= 1
VERBOSE = 0

ifeq ($(VERBOSE),1)
  NO_ECHO :=
  HDR_PREFIX := ====================
else
  NO_ECHO := @
  HDR_PREFIX :=
endif

ifeq ($(DEBUG),1)
DEBUG_FLAGS = -g3 -ggdb3
else
DEBUG_FLAGS = -g0
endif

ifeq ($(OPT),1)
OPT_FLAGS = -Os
else
OPT_FLAGS = -Og
endif

K32W061_PROG_FLAGS ?= -P 1000000 -V 0

# ==================================================
# Utility definitions
# ==================================================

# Convert source file name to object file name
define ObjFileName
$(OBJS_DIR)/$(notdir $1).o
endef

# Convert source file name to dependency file name
define DepFileName
$(DEPS_DIR)/$(notdir $1).d
endef

# Newline
define NL

endef
# ==================================================
# General build rules
# ==================================================
.PHONY : $(APP) flash flash-app flash_app erase clean help k32w061-sdk-check

%.elf : %.out
	$(K32W061_SDK_ROOT)/tools/imagetool/sign_images.sh $(OUTPUT_DIR)
	$(NO_ECHO)mv $< $@
	rm -rf $<.bin
	rm -rf $@.bin

# Flash the SoftDevice
flash-softdevice flash_softdevice :
	@echo "FLASH $$(APP_ELF)"
	$(NO_ECHO)$(K32W061_PROG) -l
	$(NO_ECHO)read -p "Please type a COM from the above list:" COM; \
	$(K32W061_PROG) $(K32W061_PROG_FLAGS) -s $COM -p $(APP_ELF) || true

# Erase device
erase :
	@echo "ERASE DEVICE"
	$(NO_ECHO)$(K32W061_PROG) -l
	$(NO_ECHO)read -p "Please type a COM from the above list:" COM; \
	$(K32W061_PROG) -s $COM -e || true

# Clean build output
clean ::
	@echo "RM $(OUTPUT_DIR)"
	$(NO_ECHO)rm -rf $(OUTPUT_DIR)

# Print help
export HelpText
help :
	@echo "$${HelpText}"

# Verify the K32W061 SDK is found
k32w061-sdk-check :
	@test -d $(K32W061_SDK_ROOT) || { \
	    echo "ENVIRONMENT ERROR: ${K32W061_SDK_TITLE} not found at $(K32W061_SDK_ROOT)"; \
	    exit 1; \
	};
# ==================================================
# Late-bound rules for building the application
# ==================================================

define AppBuildRules

# Default all rule
all : $(APP)

# General target for building the application
$(APP) : $(APP_ELF)

# Rule to link the application executable
$(APP_EXE) : $(OBJS) $(STD_LINK_PREREQUISITES) | $(OUTPUT_DIR)
	@echo "$$(HDR_PREFIX)LD $$@"
	$(NO_ECHO)$$(CC) $$(STD_LDFLAGS) $$(LDFLAGS) $$(DEBUG_FLAGS) $$(OPT_FLAGS) -Wl,-Map=$$(@:.out=.map) $$(OBJS) -Wl,--start-group $$(LIBS) $$(STD_LIBS) -Wl,--end-group -o $$@
	$(NO_ECHO)$$(SIZE) $$@

# Individual build rules for each application source file
$(foreach file,$(filter %.c,$(ALL_SRCS)),$(call CCRule,$(file)))
$(foreach file,$(filter %.cpp,$(ALL_SRCS)),$(call CXXRule,$(file)))
$(foreach file,$(filter %.S,$(ALL_SRCS)),$(call ASRule,$(file)))
$(foreach file,$(filter %.s,$(ALL_SRCS)),$(call ASRule,$(file)))

# Rule to build and flash the application
flash : $(APP_ELF)
	@echo "FLASH $$(APP_ELF)"
	$(NO_ECHO)$(K32W061_PROG) -l
	$(NO_ECHO)read -p "Please type a COM from the above list:" COM; \
	$(K32W061_PROG) $(K32W061_PROG_FLAGS) -s $COM -p $(APP_ELF) || true

# Rule to flash a pre-built application
flash-app flash_app :
	@echo "FLASH $$(APP_ELF)"
	$(NO_ECHO)$(K32W061_PROG) -l
	$(NO_ECHO)read -p "Please type a COM from the above list:" COM; \
	$(K32W061_PROG) $(K32W061_PROG_FLAGS) -s $COM -p $(APP_ELF) || true

# Rule to create the output directory / subdirectories
$(OUTPUT_DIR) $(OBJS_DIR) $(DEPS_DIR) :
	@echo "MKDIR $$@"
	$(NO_ECHO)mkdir -p "$$@"

# Include generated dependency files
include $$(wildcard $(DEPS_DIR)/*.d)

endef

# Generates late-bound rule for building an object file from a C file
define CCRule
$(call DepFileName,$1) : ;
$(call ObjFileName,$1): $1 $(call DepFileName,$1) | $(OBJS_DIR) $(DEPS_DIR) $(STD_COMPILE_PREREQUISITES)
	@echo "$$(HDR_PREFIX)CC $1"
	$(NO_ECHO) $$(CCACHE) $$(CC) -c $$(STD_CFLAGS) $$(CFLAGS) $$(DEBUG_FLAGS) $$(OPT_FLAGS) $$(DEFINE_FLAGS) $$(INC_FLAGS) -MT $$@ -MMD -MP -MF $(call DepFileName,$1).tmp -o $$@ $1
	$(NO_ECHO)mv $(call DepFileName,$1).tmp $(call DepFileName,$1)
$(NL)
endef

# Generates late-bound rule for building an object file from a C++ file
define CXXRule
$(call DepFileName,$1) : ;
$(call ObjFileName,$1): $1 $(call DepFileName,$1) | $(OBJS_DIR) $(DEPS_DIR) $(STD_COMPILE_PREREQUISITES)
	@echo "$$(HDR_PREFIX)CXX $1"
	$(NO_ECHO) $$(CCACHE) $$(CXX) -c $$(AUTODEP_FLAGS) $$(STD_CFLAGS) $$(STD_CXXFLAGS) $$(CFLAGS) $$(CXXFLAGS) $$(DEBUG_FLAGS) $$(OPT_FLAGS) $$(DEFINE_FLAGS) $$(INC_FLAGS) -MT $$@ -MMD -MP -MF $(call DepFileName,$1).tmp -o $$@ $1
	$(NO_ECHO)mv $(call DepFileName,$1).tmp $(call DepFileName,$1)
$(NL)
endef

# Generates late-bound rule for building an object file from an assembly file
define ASRule
$(call ObjFileName,$1): $1 | $(OBJS_DIR) $(STD_COMPILE_PREREQUISITES)
	@echo "$$(HDR_PREFIX)AS $1"
	$(NO_ECHO)$$(CC) -c -x assembler-with-cpp $$(STD_ASFLAGS) $$(ASFLAGS) $$(DEBUG_FLAGS) $$(OPT_FLAGS) $$(DEFINE_FLAGS) $$(INC_FLAGS) -o $$@ $1
$(NL)
endef
# ==================================================
# Function for generating all late-bound rules
# ==================================================

# List of variables containing late-bound rules that should
# be evaluated when GenerateBuildRules is called.
LATE_BOUND_RULES = AppBuildRules

define GenerateBuildRules
$(foreach rule,$(LATE_BOUND_RULES),$(eval $($(rule))))
endef
# ==================================================
# Help Definitions
# ==================================================

# Desciptions of make targets
define TargetHelp
  all                   Build the $(APP) application.

  clean                 Clean all build outputs.

  flash                 Build and flash the application onto the device.

  flash-app             Flash the application onto the device without building
                        it first.

  flash-softdevice      Flash the NXP image onto the device.

  erase                 Wipe the device's flash memory.

  help                  Print this help message.
endef

# Desciptions of build options
define OptionHelp
  DEBUG=[1|0]           Build the application and all libraries with symbol
                        information.

  VERBOSE=[1|0]         Show commands as they are being executed.
endef

# Overall help text
define HelpText
Makefile for building the $(APP) application.

Available targets:

$(TargetHelp)

Build options:

$(OptionHelp)

endef
