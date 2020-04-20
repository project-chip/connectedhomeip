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
#         on the Nordic nRF5 SDK.
#

#
#   This makefile is primarily intended to support building the
#   CHIP example applications on Nordic platforms.  However
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
#       BUILD_SUPPORT_DIR = $(PROJECT_ROOT)/third_party/connectedhomeip/config/nrf5
#       
#       include $(BUILD_SUPPORT_DIR)/nrf5-app.mk
#       
#       APP := chip-nrf52840-bringup
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
#           -lnrf_cc310_0.9.10
#       
#       DEFINES = \
#           NRF52840_XXAA \
#           BOARD_PCA10056 \
#           BSP_DEFINES_ONLY \
#           CONFIG_GPIO_AS_PINRESET \
#           FLOAT_ABI_HARD \
#           USE_APP_CONFIG \
#           __HEAP_SIZE=0 \
#           __STACK_SIZE=8192 \
#           SOFTDEVICE_PRESENT
#       
#       $(call GenerateBuildRules)
#


# ==================================================
# Sanity Checks
# ==================================================

ifndef NRF5_SDK_ROOT
$(error ENVIRONMENT ERROR: NRF5_SDK_ROOT not set)
endif
ifndef ARM_GCC_INSTALL_ROOT
$(error ENVIRONMENT ERROR: ARM_GCC_INSTALL_ROOT not set)
endif
ifndef NRF5_TOOLS_ROOT
$(error ENVIRONMENT ERROR: NRF5_TOOLS_ROOT not set)
endif


# ==================================================
# General definitions
# ==================================================

.DEFAULT_GOAL := all

APP_EXE = $(OUTPUT_DIR)/$(APP).out
APP_HEX = $(OUTPUT_DIR)/$(APP).hex

SRCS =
ALL_SRCS = $(SRCS) $(EXTRA_SRCS)
OBJS = $(foreach file,$(ALL_SRCS),$(call ObjFileName,$(file)))

OUTPUT_DIR = $(PROJECT_ROOT)/build
OBJS_DIR = $(OUTPUT_DIR)/objs
DEPS_DIR = $(OUTPUT_DIR)/deps

STD_CFLAGS = \
    -mcpu=cortex-m4 \
    -mthumb \
    -mabi=aapcs \
    -Wall \
    -mfloat-abi=hard \
    -mfpu=fpv4-sp-d16 \
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
    -g3 \
    -mcpu=cortex-m4 \
    -mthumb \
    -mabi=aapcs \
    -mfloat-abi=hard \
    -mfpu=fpv4-sp-d16 \
    -x assembler-with-cpp

STD_LDFLAGS = \
    -mthumb \
    -mabi=aapcs \
    -mcpu=cortex-m4 \
    -mfloat-abi=hard \
    -mfpu=fpv4-sp-d16 \
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

STD_DEFINES =

STD_COMPILE_PREREQUISITES = nrf5-sdk-check

STD_LINK_PREREQUISITES = nrf5-sdk-check

DEFINE_FLAGS = $(foreach def,$(STD_DEFINES) $(DEFINES),-D$(def))

INC_FLAGS = $(foreach dir,$(INC_DIRS) $(STD_INC_DIRS),-I$(dir))

LINKER_SCRIPT = $(realpath $(PROJECT_ROOT))/$(APP).ld

LINKER_SCRIPT_INC_DIRS = $(NRF5_SDK_ROOT)/modules/nrfx/mdk

NRF5_SDK_VERSION_FILE = $(NRF5_SDK_ROOT)/documentation/release_notes.txt

NRF5_SDK_TITLE = nRF5 SDK for Thread and Zigbee

NRF5_SDK_VERSION_SED_EXP = /^${NRF5_SDK_TITLE} v[0-9.]+.*$$/ { s/.*v([0-9.]+).*/\1/; s/[.]/ /g; p; q; }

NRF5_SDK_MIN_VERSION = 3.1.0


# ==================================================
# Toolchain and external utilities / files
# ==================================================

TARGET_TUPLE = arm-none-eabi

CC      = $(ARM_GCC_INSTALL_ROOT)/$(TARGET_TUPLE)-gcc
CXX     = $(ARM_GCC_INSTALL_ROOT)/$(TARGET_TUPLE)-c++
CPP     = $(ARM_GCC_INSTALL_ROOT)/$(TARGET_TUPLE)-cpp
AS      = $(ARM_GCC_INSTALL_ROOT)/$(TARGET_TUPLE)-as
AR      = $(ARM_GCC_INSTALL_ROOT)/$(TARGET_TUPLE)-ar
LD      = $(ARM_GCC_INSTALL_ROOT)/$(TARGET_TUPLE)-ld
NM      = $(ARM_GCC_INSTALL_ROOT)/$(TARGET_TUPLE)-nm
OBJDUMP = $(ARM_GCC_INSTALL_ROOT)/$(TARGET_TUPLE)-objdump
OBJCOPY = $(ARM_GCC_INSTALL_ROOT)/$(TARGET_TUPLE)-objcopy
SIZE    = $(ARM_GCC_INSTALL_ROOT)/$(TARGET_TUPLE)-size
RANLIB  = $(ARM_GCC_INSTALL_ROOT)/$(TARGET_TUPLE)-ranlib

INSTALL = /usr/bin/install
INSTALLFLAGS = -C -v

ifneq (, $(shell which ccache))
CCACHE = ccache
endif

NRFJPROG = $(NRF5_TOOLS_ROOT)/nrfjprog/nrfjprog

SOFTDEVICE_IMAGE_DIR = $(NRF5_SDK_ROOT)/components/softdevice/s140/hex

SOFTDEVICE_IMAGE = $(wildcard $(SOFTDEVICE_IMAGE_DIR)/s140_nrf52_*_softdevice.hex)


# ==================================================
# Build options
# ==================================================

DEBUG ?= 1
OPT ?= 1
VERBOSE ?= 0
MMD ?= 0

ifeq ($(VERBOSE),1)
  NO_ECHO :=
  HDR_PREFIX := ==================== 
else
  NO_ECHO := @
  HDR_PREFIX :=
endif

ifeq ($(MMD),1)
STD_DEFINES += JLINK_MMD
EXTRA_SRCS += $(PROJECT_ROOT)/JLINK_MONITOR_ISR_SES.s
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

NRFJPROG_FLAGS ?= -f nrf52
ifneq (,$(JLINK_SN))
NRFJPROG_FLAGS += -s $(JLINK_SN)
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

.PHONY : $(APP) flash flash-app flash_app flash-softdevice flash_softdevice erase clean help nrf5-sdk-check

# Convert executable to Intel hex format
%.hex : %.out
	$(NO_ECHO)$(OBJCOPY) -O ihex $< $@

# Flash the SoftDevice
flash-softdevice flash_softdevice :
	@if test -z "$(SOFTDEVICE_IMAGE)"; then echo "SoftDevice image not found in $(SOFTDEVICE_IMAGE_DIR)"; false; fi
	@echo "FLASH $(SOFTDEVICE_IMAGE)"
	$(NO_ECHO)$(NRFJPROG) -f nrf52 --program $(SOFTDEVICE_IMAGE) --sectorerase
	@echo "RESET DEVICE"
	$(NO_ECHO)$(NRFJPROG) -f nrf52 --reset

# Erase device
erase :
	@echo "ERASE DEVICE"
	$(NO_ECHO)$(NRFJPROG) -f nrf52 --eraseall

# Clean build output
clean ::
	@echo "RM $(OUTPUT_DIR)"
	$(NO_ECHO)rm -rf $(OUTPUT_DIR)

# Print help
export HelpText
help :
	@echo "$${HelpText}"

# Verify the nRF5 SDK is found and meets the required minimum version
nrf5-sdk-check :
	@test -d $(NRF5_SDK_ROOT) || { \
	    echo "ENVIRONMENT ERROR: ${NRF5_SDK_TITLE} not found at $(NRF5_SDK_ROOT)"; \
	    exit 1; \
	}; \
	test -f $(NRF5_SDK_VERSION_FILE) || { \
	    echo "ENVIRONMENT ERROR: Version file for $(NRF5_SDK_TITLE) not found at $(NRF5_SDK_VERSION_FILE)"; \
	    exit 1; \
	}; \
	SDK_VERSION=`sed -nE '$(NRF5_SDK_VERSION_SED_EXP)' $(NRF5_SDK_VERSION_FILE)`; \
	test "$${SDK_VERSION}x" != "x" || { \
	    echo "ENVIRONMENT ERROR: Unable to determine version of $(NRF5_SDK_TITLE) from $(NRF5_SDK_VERSION_FILE)"; \
	    exit 1; \
	}; \
	MIN_VERSION=`echo $(NRF5_SDK_MIN_VERSION) | sed -E 's/\./ /g'`; \
	HIGHEST_VERSION=`(echo $${SDK_VERSION}; echo $${MIN_VERSION}) | sort -r -n -k1 -k2 -k3 | head -1`; \
	test "$${HIGHEST_VERSION}" = "$${SDK_VERSION}" || { \
	    echo "ENVIRONMENT ERROR: Unsupported version of $(NRF5_SDK_TITLE); Please use $(NRF5_SDK_MIN_VERSION) or greater"; \
	    exit 1; \
	}	


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
	@echo "FLASH $$(APP_HEX)"
	$(NO_ECHO)$$(NRFJPROG) $$(NRFJPROG_FLAGS) --program $$(APP_HEX) --sectorerase
	@echo "RESET DEVICE"
	$(NO_ECHO)$$(NRFJPROG) $$(NRFJPROG_FLAGS) --reset

# Rule to flash a pre-built application
flash-app flash_app :
	@echo "FLASH $$(APP_HEX)"
	$(NO_ECHO)$$(NRFJPROG) $$(NRFJPROG_FLAGS) --program $$(APP_HEX) --sectorerase
	@echo "RESET DEVICE"
	$(NO_ECHO)$$(NRFJPROG) $$(NRFJPROG_FLAGS) --reset

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
 
  MMD=[1|0]             Enable J-Link monitor mode debugging.  Monitor mode
                        debugging requires the JLINK_MONITOR_ISR_SES.s source
                        file to be placed in the application root directory.
                        This file is available from SEGGER at the following URL:
      
                        https://www.segger.com/products/debug-probes/j-link/technology/monitor-mode-debugging/
endef

# Overall help text
define HelpText
Makefile for building the $(APP) application.

Available targets:

$(TargetHelp)

Build options:

$(OptionHelp)

endef
