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
#         on the Silicon Labs EFR32 SDK.
#

#
#   This makefile is primarily intended to support building the
#   CHIP example applications on Silicon Labs platforms.  However
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
#       BUILD_SUPPORT_DIR = $(PROJECT_ROOT)/third_party/connectedhomeip/config/efr32
#
#       include $(BUILD_SUPPORT_DIR)/efr32-app.mk
#       include $(BUILD_SUPPORT_DIR)/efr32-chip.mk
#
#       APP := chip-efr32-bringup
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

ifndef BOARD
$(error ENVIRONMENT ERROR: BOARD not set)
endif
ifndef EFR32_SDK_ROOT
$(error ENVIRONMENT ERROR: EFR32_SDK_ROOT not set)
endif
ifndef EFR32_TOOLS_ROOT
$(error ENVIRONMENT ERROR: EFR32_TOOLS_ROOT not set)
endif
ifndef FREERTOS_ROOT
$(error ENVIRONMENT ERROR: FREERTOS_ROOT not set)
endif

uname_S := $(shell sh -c 'uname -s 2>/dev/null || echo not')

# Differentiate between boards
# - BRD4304A / SLWSTK6000B / MGM12P Module / 2.4GHz@19dBm
# - BRD4161A / SLWSTK6000B / Wireless Starter Kit / 2.4GHz@19dBm
# - BRD4166A / SLTB004A    / Thunderboard Sense 2 / 2.4GHz@10dBm
# - BRD4170A / SLWSTK6000B / Multiband Wireless Starter Kit / 2.4GHz@19dBm, 915MHz@19dBm, 868MHz@19dBm
# - BRD4180A / SLWSTK6006A / MG21 Module / 2.4GHz@20dBm

ifeq ($(BOARD),BRD4304A)
EFR32FAMILY := efr32mg12
MCU = EFR32MG12P432F1024GM48
else
ifeq ($(BOARD),BRD4161A)
EFR32FAMILY := efr32mg12
MCU = EFR32MG12P432F1024GL125
else
ifeq ($(BOARD),BRD4166A)
EFR32FAMILY := efr32mg12
MCU = EFR32MG12P332F1024GL125
else
ifeq ($(BOARD),BRD4170A)
EFR32FAMILY := efr32mg12
MCU = EFR32MG12P433F1024GM68
else
ifeq ($(BOARD),BRD4180A)
EFR32FAMILY := efr32mg21
MCU = EFR32MG21A020F1024IM32
else
$(error Please provide a value for BOARD variable e.g BOARD=BRD4161A (currently supported BRD4304A, BRD4161A, BRD4166A, BRD4170A or BRD4180A))
endif
endif
endif
endif
endif

BOARD_LC := $(shell echo $(BOARD) | tr A-Z a-z)

EXTRA_SRCS += \
    $(EFR32_SDK_ROOT)/platform/emdrv/nvm3/src/nvm3_lock.c                                   \
    $(EFR32_SDK_ROOT)/platform/emdrv/nvm3/src/nvm3_default.c                                \
    $(EFR32_SDK_ROOT)/platform/emdrv/nvm3/src/nvm3_hal_flash.c                              \
    $(EFR32_SDK_ROOT)/platform/emdrv/sleep/src/sleep.c                                      \
    $(EFR32_SDK_ROOT)/util/third_party/segger/systemview/SEGGER/SEGGER_RTT.c

STD_INC_DIRS += \
    $(CHIP_ROOT)/src/include/platform                                                       \
    $(CHIP_ROOT)/src/platform/EFR32                                                         \
    $(FREERTOS_ROOT)/Source/include                                                         \
    $(EFR32_SDK_ROOT)                                                                       \
    $(EFR32_SDK_ROOT)/hardware/kit/common/bsp                                               \
    $(EFR32_SDK_ROOT)/hardware/kit/common/drivers                                           \
    $(EFR32_SDK_ROOT)/platform/base/                                                        \
    $(EFR32_SDK_ROOT)/platform/base/hal                                                     \
    $(EFR32_SDK_ROOT)/platform/base/hal/micro/cortexm3/compiler                             \
    $(EFR32_SDK_ROOT)/platform/base/hal/micro/cortexm3/efm32                                \
    $(EFR32_SDK_ROOT)/platform/base/hal/micro/cortexm3/efm32/config                         \
    $(EFR32_SDK_ROOT)/platform/base/hal/plugin                                              \
    $(EFR32_SDK_ROOT)/platform/CMSIS/Include                                                \
    $(EFR32_SDK_ROOT)/platform/emdrv/common/inc                                             \
    $(EFR32_SDK_ROOT)/platform/emdrv/gpiointerrupt/inc                                      \
    $(EFR32_SDK_ROOT)/platform/emdrv/uartdrv/inc                                            \
    $(EFR32_SDK_ROOT)/platform/emdrv/uartdrv/config                                         \
    $(EFR32_SDK_ROOT)/platform/emdrv/ustimer/inc                                            \
    $(EFR32_SDK_ROOT)/platform/emdrv/dmadrv/config                                          \
    $(EFR32_SDK_ROOT)/platform/emdrv/dmadrv/inc                                             \
    $(EFR32_SDK_ROOT)/platform/emdrv/nvm3/inc                                               \
    $(EFR32_SDK_ROOT)/platform/emdrv/rtcdrv/inc                                             \
    $(EFR32_SDK_ROOT)/platform/emdrv/sleep/inc                                              \
    $(EFR32_SDK_ROOT)/platform/emlib/inc                                                    \
    $(EFR32_SDK_ROOT)/platform/halconfig/inc/hal-config                                     \
    $(EFR32_SDK_ROOT)/platform/radio/rail_lib/common                                        \
    $(EFR32_SDK_ROOT)/platform/radio/rail_lib/chip/efr32                                    \
    $(EFR32_SDK_ROOT)/platform/radio/rail_lib/protocol/ieee802154                           \
    $(EFR32_SDK_ROOT)/platform/radio/rail_lib/chip/efr32/rf/common/cortex                   \
    $(EFR32_SDK_ROOT)/platform/radio/rail_lib/hal                                           \
    $(EFR32_SDK_ROOT)/platform/radio/rail_lib/hal/efr32                                     \
    $(EFR32_SDK_ROOT)/platform/radio/rail_lib/plugin/pa-conversions                         \
    $(EFR32_SDK_ROOT)/util/third_party/segger/systemview/SEGGER                             \
    $(EFR32_SDK_ROOT)/util/third_party/segger/systemview/Config/                            \
    $(EFR32_SDK_ROOT)/util/plugin/plugin-common/fem-control

ifeq ($(EFR32FAMILY), efr32mg12)
STD_INC_DIRS += \
    $(FREERTOS_ROOT)/Source/portable/GCC/ARM_CM4F                                           \
    $(EFR32_SDK_ROOT)/hardware/kit/EFR32MG12_$(BOARD)/config                                \
    $(EFR32_SDK_ROOT)/platform/Device/SiliconLabs/EFR32MG12P/Include                        \
    $(EFR32_SDK_ROOT)/platform/radio/rail_lib/chip/efr32/efr32xg1x
else
ifeq ($(EFR32FAMILY), efr32mg21)
STD_INC_DIRS += \
    $(FREERTOS_ROOT)/Source/portable/GCC/ARM_CM3                                            \
    $(EFR32_SDK_ROOT)/hardware/kit/EFR32MG21_$(BOARD)/config                                \
    $(EFR32_SDK_ROOT)/platform/Device/SiliconLabs/EFR32MG21/Include                         \
    $(EFR32_SDK_ROOT)/platform/radio/rail_lib/chip/efr32/efr32xg2x
endif
endif

# ==================================================
# General definitions
# ==================================================

.DEFAULT_GOAL := all

APP_EXE = $(OUTPUT_DIR)/$(APP).out
APP_SREC = $(OUTPUT_DIR)/$(APP).s37

SRCS =
ALL_SRCS = $(SRCS) $(EXTRA_SRCS)
OBJS = $(foreach file,$(ALL_SRCS),$(call ObjFileName,$(file)))

OUTPUT_DIR = $(PROJECT_ROOT)/build
OBJS_DIR = $(OUTPUT_DIR)/objs
DEPS_DIR = $(OUTPUT_DIR)/deps

ifeq ($(EFR32FAMILY), efr32mg12)
FLOAT_ABI = -mfloat-abi=softfp
FLOAT_FPU = -mfpu=fpv4-sp-d16
else
ifeq ($(EFR32FAMILY), efr32mg21)
FLOAT_ABI = -mfloat-abi=hard
FLOAT_FPU = -mfpu=fpv5-sp-d16
endif
endif

STD_CFLAGS = \
    -mcpu=cortex-m4 \
    -mthumb \
    -mabi=aapcs \
    $(FLOAT_ABI) \
    $(FLOAT_FPU) \
    -Wall \
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
    -mcpu=cortex-m4 \
    -mthumb \
    -mabi=aapcs \
    $(FLOAT_ABI) \
    $(FLOAT_FPU)

STD_LDFLAGS = \
    -mcpu=cortex-m4 \
    -mthumb \
    -mabi=aapcs \
    $(FLOAT_ABI) \
    -fdata-sections \
    -ffunction-sections \
    -Wl,--gc-sections \
    -specs=nosys.specs \
    $(foreach dir,$(LINKER_SCRIPT_INC_DIRS),-L$(dir)) \
    -T$(LINKER_SCRIPT)

ifeq ($(EFR32FAMILY), efr32mg21)
STD_LDFLAGS += \
    $(FLOAT_FPU)
endif

STD_LIBS = \
    -lc \
    -lstdc++ \
    -lfreertos \
    -lnosys \
    -lm

ifeq ($(EFR32FAMILY), efr32mg12)
STD_LIBS += \
    $(EFR32_SDK_ROOT)/protocol/bluetooth/lib/EFR32MG12P/GCC/libbluetooth.a \
    $(EFR32_SDK_ROOT)/platform/radio/rail_lib/autogen/librail_release/librail_multiprotocol_efr32xg12_gcc_release.a \
    $(EFR32_SDK_ROOT)/platform/emdrv/nvm3/lib/libnvm3_CM4_gcc.a \
    $(EFR32_SDK_ROOT)/protocol/bluetooth/lib/EFR32MG12P/GCC/libmbedtls.a
else
ifeq ($(EFR32FAMILY), efr32mg21)
STD_LIBS += \
    $(EFR32_SDK_ROOT)/protocol/bluetooth/lib/EFR32MG21/GCC/libbluetooth.a \
    $(EFR32_SDK_ROOT)/platform/radio/rail_lib/autogen/librail_release/librail_multiprotocol_efr32xg21_gcc_release.a \
    $(EFR32_SDK_ROOT)/platform/emdrv/nvm3/lib/libnvm3_CM33_gcc.a \
    $(EFR32_SDK_ROOT)/protocol/bluetooth/lib/EFR32MG21/GCC/libmbedtls.a
endif
endif

STD_DEFINES = \
    HAVE_CONFIG_H \
    __STARTUP_CLEAR_BSS \
    $(MCU)

STD_COMPILE_PREREQUISITES =

STD_LINK_PREREQUISITES += $(OUTPUT_DIR)/freertos/libfreertos.a

DEFINE_FLAGS = $(foreach def,$(STD_DEFINES) $(DEFINES),-D$(def))

INC_FLAGS = $(foreach dir,$(INC_DIRS) $(STD_INC_DIRS),-I$(dir))

LINKER_SCRIPT_INC_DIRS = $(PROJECT_ROOT) $(OUTPUT_DIR)/freertos

# Default EFR32 linker script defines a section at top of Flash for NVM3 support.
ifeq ($(EFR32FAMILY), efr32mg12)
ifndef LINKER_SCRIPT
LINKER_SCRIPT = $(APP)-MG12P.ld
endif
else
ifeq ($(EFR32FAMILY), efr32mg21)
ifndef LINKER_SCRIPT
LINKER_SCRIPT = $(APP)-MG21.ld
endif
endif
endif


# ==================================================
# Toolchain and external utilities / files
# ==================================================

TARGET_TUPLE = arm-none-eabi

CC      = $(TARGET_TUPLE)-gcc
CXX     = $(TARGET_TUPLE)-c++
CPP     = $(TARGET_TUPLE)-cpp
AS      = $(TARGET_TUPLE)-as
AR      = $(TARGET_TUPLE)-ar
LD      = $(TARGET_TUPLE)-ld
NM      = $(TARGET_TUPLE)-nm
OBJDUMP = $(TARGET_TUPLE)-objdump
OBJCOPY = $(TARGET_TUPLE)-objcopy
SIZE    = $(TARGET_TUPLE)-size
RANLIB  = $(TARGET_TUPLE)-ranlib

INSTALL = /usr/bin/install
INSTALLFLAGS = -C -v

ifneq (, $(shell which ccache))
CCACHE = ccache
endif

ifeq ($(uname_S),Darwin)
COMMANDER = $(EFR32_TOOLS_ROOT)/developer/adapter_packs/commander/Commander.app/Contents/MacOS/commander
else
COMMANDER = $(EFR32_TOOLS_ROOT)/developer/adapter_packs/commander/commander
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
OPT_FLAGS = -Og
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

.PHONY : $(APP) flash flash-app flash_app erase clean help

# Convert executable to S-Record s37 format
%.s37 : %.out
	$(NO_ECHO)$(OBJCOPY) -O srec $< $@

# Clean build output
clean ::
	@echo "RM $(OUTPUT_DIR)"
	$(NO_ECHO)rm -rf $(OUTPUT_DIR)

# Print help
export HelpText
help :
	@echo "$${HelpText}"


# ==================================================
# Late-bound rules for building the application
# ==================================================

define AppBuildRules

# Default all rule
all : $(APP)

# General target for building the application
$(APP) : $(APP_SREC)

# Rule to link the application executable
$(APP_EXE) : $(OBJS) $(STD_LINK_PREREQUISITES) | $(OUTPUT_DIR)
	@echo "$$(HDR_PREFIX)LD $$@"
	$$(CC) $$(STD_LDFLAGS) $$(LDFLAGS) $$(DEBUG_FLAGS) $$(OPT_FLAGS) -Wl,-Map=$$(@:.out=.map) $$(OBJS) -Wl,--start-group $$(LIBS) $$(STD_LIBS) -Wl,--end-group -o $$@
	$(NO_ECHO)$$(SIZE) $$@

# Individual build rules for each application source file
$(foreach file,$(filter %.c,$(ALL_SRCS)),$(call CCRule,$(file)))
$(foreach file,$(filter %.cpp,$(ALL_SRCS)),$(call CXXRule,$(file)))
$(foreach file,$(filter %.S,$(ALL_SRCS)),$(call ASRule,$(file)))
$(foreach file,$(filter %.s,$(ALL_SRCS)),$(call ASRule,$(file)))

# Rule to build and flash the application
flash : $(APP_SREC)
ifndef SERIALNO
	@echo "FLASH $$(APP_SREC)"
	$(COMMANDER) flash $(APP_SREC)
else
	@echo "FLASH $$(APP_SREC), serial: $$(SERIALNO)"
	$(COMMANDER) flash $(APP_SREC) --serialno $(SERIALNO)
endif

# Rule to flash a pre-built application
flash-app flash_app :
ifndef SERIALNO
	@echo "FLASH APP $$(APP_SREC)"
	$(COMMANDER) flash $(APP_SREC)
else
	@echo "FLASH APP $$(APP_SREC), serial: $$(SERIALNO)"
	$(COMMANDER) flash $(APP_SREC) --serialno $(SERIALNO)
endif

# Erase device
erase :
ifndef SERIALNO
	@echo "FLASH ERASE"
	$(COMMANDER) device masserase
else
	@echo "FLASH ERASE, serial: $$(SERIALNO)"
	$(COMMANDER) device masserase --serialno $(SERIALNO)
endif


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

  erase                 Wipe the device's flash memory.

  help                  Print this help message.
endef

# Desciptions of build options
define OptionHelp
  BOARD=board           Build image for a particular module.
                        Supported boards are BRD4161A, BRD4166A, BRD4170A, BRD4304A and BRD4180A.

  SERIALNO=serial       Serial number of board to flash or erase when multiple
                        devices are connected.

  DEBUG=[1|0]           Build the application and all libraries with symbol
                        information.

  OPT=[1|0]             Build the application with optimization.

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
