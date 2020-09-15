#
#   Copyright (c) 2020 Project CHIP Authors
#   Copyright (c) 2019 Google LLC.
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
#         on the Qorvo QPG6100.
#

#
#   This makefile is primarily intended to support building the
#   CHIP example applications on Qorvo platforms.  However
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
#       BUILD_SUPPORT_DIR = $(PROJECT_ROOT)/third_party/connectedhomeip/config/qpg6100
#       
#       include $(BUILD_SUPPORT_DIR)/qpg6100-app.mk
#       
#       APP := chip-qpg6100-bringup
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
#           -lCHIP_qpg6100.a
#       
#       DEFINES = \
#           ...
#       
#       $(call GenerateBuildRules)
#


# ==================================================
# Sanity Checks
# ==================================================

#ifndef QPG6100_SDK_ROOT
#$(error ENVIRONMENT ERROR: QPG6100_SDK_ROOT not set)
#endif
ifndef ARM_GCC_INSTALL_ROOT
$(error ENVIRONMENT ERROR: ARM_GCC_INSTALL_ROOT not set)
endif

# ==================================================
# General definitions
# ==================================================

.DEFAULT_GOAL := all

APP_EXE = $(OUTPUT_DIR)/$(APP).out
APP_HEX = $(OUTPUT_DIR)/$(APP).hex

SRC =
ALL_SRCS = $(SRC) $(EXTRA_SRCS)
OBJS = $(foreach file,$(ALL_SRCS),$(call ObjFileName,$(file)))

OUTPUT_DIR = $(PROJECT_ROOT)/build
OBJS_DIR = $(OUTPUT_DIR)/objs
DEPS_DIR = $(OUTPUT_DIR)/deps

STD_CFLAGS = \
    -mcpu=cortex-m4 \
    -mthumb \
    -mabi=aapcs \
    -Wall \
    -mfloat-abi=soft \
    -ffunction-sections \
    -fdata-sections \
    -fno-strict-aliasing \
    -fshort-enums \
    --specs=nosys.specs
#    -mfpu=fpv4-sp-d16 \

STD_CXXFLAGS = \
    -fno-rtti \
    -fno-exceptions \
    -fno-unwind-tables

STD_ASFLAGS = \
    -g3 \
    -mcpu=cortex-m4 \
    -mthumb \
    -mabi=aapcs \
    -mfloat-abi=soft \
    -x assembler-with-cpp

STD_LDFLAGS = \
    -mthumb \
    -mabi=aapcs \
    -mcpu=cortex-m4 \
    -mfloat-abi=soft \
    -Wl,--gc-sections \
    --specs=nosys.specs \
    $(foreach dir,$(LINKER_SCRIPT_INC_DIRS),-L$(dir)) \
    -T$(LINKER_SCRIPT)

STD_LIBS = \
    -lc \
    -lstdc++ \
    -lnosys \
    -lm

STD_INC_DIRS =

STD_DEFINES = HAVE_CONFIG_H

#STD_COMPILE_PREREQUISITES = qpg6100-sdk-check
STD_COMPILE_PREREQUISITES =

#STD_LINK_PREREQUISITES = qpg6100-sdk-check
STD_LINK_PREREQUISITES =

DEFINE_FLAGS = $(foreach def,$(STD_DEFINES) $(DEFINES),-D$(def))

INC_FLAGS = $(foreach dir,$(INC) $(INC_DIRS) $(STD_INC_DIRS),-I$(dir))

LINKER_SCRIPT = $(realpath $(PROJECT_ROOT))/$(APP).ld

LINKER_SCRIPT_INC_DIRS =

QPG6100_SDK_TITLE = "QPG6100 SDK"

# ==================================================
# Toolchain and external utilities / files
# ==================================================

ARCH            ?=
CROSS_COMPILE   ?=arm-none-eabi-

# Define programs and commands.
CC          = $(ARM_GCC_INSTALL_ROOT)/$(CROSS_COMPILE)gcc
CXX         = $(ARM_GCC_INSTALL_ROOT)/$(CROSS_COMPILE)g++
CPP         = $(ARM_GCC_INSTALL_ROOT)/$(CROSS_COMPILE)cpp
AS          = $(ARM_GCC_INSTALL_ROOT)/$(CROSS_COMPILE)as
AR          = $(ARM_GCC_INSTALL_ROOT)/$(CROSS_COMPILE)ar
LD          = $(ARM_GCC_INSTALL_ROOT)/$(CROSS_COMPILE)ld
NM          = $(ARM_GCC_INSTALL_ROOT)/$(CROSS_COMPILE)nm
OBJCOPY     = $(ARM_GCC_INSTALL_ROOT)/$(CROSS_COMPILE)objcopy
OBJDUMP     = $(ARM_GCC_INSTALL_ROOT)/$(CROSS_COMPILE)objdump
SIZE        = $(ARM_GCC_INSTALL_ROOT)/$(CROSS_COMPILE)size -A
RANLIB      = $(ARM_GCC_INSTALL_ROOT)/$(CROSS_COMPILE)ranlib

INSTALL = /usr/bin/install
INSTALLFLAGS = -C -v

ifneq (, $(shell which ccache))
CCACHE = ccache
endif

# ==================================================
# Build options
# ==================================================

DEBUG ?= 1
OPT ?= 1
VERBOSE ?= 0

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
OPT_FLAGS = -O0
endif

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

.PHONY : $(APP) flash flash-app flash_app erase clean help qpg6100-sdk-check

# Convert executable to Intel hex format
%.hex : %.out
	$(NO_ECHO)$(OBJCOPY) -O ihex $< $@

# Erase device
erase :
	@echo "ERASE DEVICE - not supported"

# Clean build output
clean ::
	@echo "RM $(OUTPUT_DIR)"
	$(NO_ECHO)rm -rf $(OUTPUT_DIR)

# Print help
export HelpText
help :
	@echo "$${HelpText}"

# Verify the QPG6100 SDK is found
qpg6100-sdk-check :
	@test -d $(QPG6100_SDK_ROOT) || { \
	    echo "ENVIRONMENT ERROR: ${QPG6100_SDK_TITLE} not found at $(QPG6100_SDK_ROOT)"; \
	    exit 1; \
	};

# ==================================================
# Late-bound rules for building the application
# ==================================================

define AppBuildRules

# Default all rule
all : $(APP)

# General target for building the application
$(APP) : $(APP_HEX)

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
flash : $(APP_HEX)
	@echo "FLASH $$(APP_HEX) - not implemented"

# Rule to flash a pre-built application
flash-app flash_app :
	@echo "FLASH $$(APP_HEX) - not implemented"

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
  
  flash-softdevice      Flash the Nordic SoftDevice image onto the device.
  
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
