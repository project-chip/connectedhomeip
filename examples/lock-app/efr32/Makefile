#
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
#         Makefile for building the CHIP SiLabs EFR32 Lock Example Application.
#

#
# Differentiate between boards
#
# MG12
# - BRD4304A / SLWSTK6000B / MGM12P Module / 2.4GHz@19dBm
# - BRD4161A / SLWSTK6000B / Wireless Starter Kit / 2.4GHz@19dBm
# - BRD4166A / SLTB004A / Thunderboard Sense 2 / 2.4GHz@10dBm
# - BRD4170A / SLWSTK6000B / Multiband Wireless Starter Kit / 2.4GHz@19dBm, 915MHz@19dBm
#
# MG21
# - BRD4180A / SLWSTK6006A / Wireless Starter Kit / 2.4GHz@20dBm

PROJECT_ROOT := $(realpath .)

#
# Location of the EFR32 SDK
#
ifndef EFR32_SDK_ROOT
$(error ENVIRONMENT ERROR: EFR32_SDK_ROOT not set)
endif

CHIP_ROOT ?= $(realpath $(PROJECT_ROOT)/third_party/connectedhomeip)
EFR32_TOOLS_ROOT ?= $(realpath $(EFR32_SDK_ROOT)/../../../..)
FREERTOS_ROOT ?= $(realpath $(EFR32_SDK_ROOT)/util/third_party/freertos)

# Do not enable -Wconversion for Silicon Labs SDK sources
override CFLAGS                              := $(filter-out -Wconversion,$(CFLAGS))
override CXXFLAGS                            := $(filter-out -Wconversion,$(CXXFLAGS))

# Do not enable -pedantic-errors for Silicon Labs SDK sources
override CFLAGS                              := $(filter-out -pedantic-errors,$(CFLAGS))
override CXXFLAGS                            := $(filter-out -pedantic-errors,$(CXXFLAGS))

# Do not enable -Wshadow for Silicon Labs SDK sources
override CFLAGS                              := $(filter-out -Wshadow,$(CFLAGS))
override CXXFLAGS                            := $(filter-out -Wshadow,$(CXXFLAGS))

# Do not enable -Wundef for Silicon Labs SDK sources
override CFLAGS                              := $(filter-out -Wundef,$(CFLAGS))
override CXXFLAGS                            := $(filter-out -Wundef,$(CXXFLAGS))

FREERTOSCONFIG_DIR := $(PROJECT_ROOT)/include

GECKO_SDK_SUITE_DIR    := $(EFR32_SDK_ROOT)/..

all : |

BUILD_SUPPORT_DIR := $(CHIP_ROOT)/config/efr32

include $(BUILD_SUPPORT_DIR)/efr32-app.mk
include $(BUILD_SUPPORT_DIR)/efr32-chip.mk
include $(BUILD_SUPPORT_DIR)/efr32-freertos.mk
include $(PROJECT_ROOT)/src/display/efr32-lcd.mk

APP = efr32-lock-example

SRCS = \
    $(PROJECT_ROOT)/src/main.cpp                                                             \
    $(PROJECT_ROOT)/src/LEDWidget.cpp                                                        \
    $(PROJECT_ROOT)/src/AppTask.cpp                                                          \
    $(PROJECT_ROOT)/src/ButtonHandler.cpp                                                    \
    $(PROJECT_ROOT)/src/platform/$(EFR32FAMILY)/$(BOARD)/init_mcu.c                          \
    $(PROJECT_ROOT)/src/platform/$(EFR32FAMILY)/$(BOARD)/init_board.c                        \
    $(EFR32_SDK_ROOT)/hardware/kit/common/bsp/bsp_init.c                                     \
    $(EFR32_SDK_ROOT)/hardware/kit/common/bsp/bsp_stk.c                                      \
    $(EFR32_SDK_ROOT)/hardware/kit/common/bsp/bsp_stk_leds.c                                 \
    $(EFR32_SDK_ROOT)/platform/emdrv/dmadrv/src/dmadrv.c                                     \
    $(EFR32_SDK_ROOT)/platform/emdrv/gpiointerrupt/src/gpiointerrupt.c                       \
    $(EFR32_SDK_ROOT)/platform/emdrv/uartdrv/src/uartdrv.c                                   \
    $(EFR32_SDK_ROOT)/platform/emdrv/ustimer/src/ustimer.c                                   \
    $(EFR32_SDK_ROOT)/platform/emlib/src/em_adc.c                                            \
    $(EFR32_SDK_ROOT)/platform/emlib/src/em_cmu.c                                            \
    $(EFR32_SDK_ROOT)/platform/emlib/src/em_core.c                                           \
    $(EFR32_SDK_ROOT)/platform/emlib/src/em_crypto.c                                         \
    $(EFR32_SDK_ROOT)/platform/emlib/src/em_emu.c                                            \
    $(EFR32_SDK_ROOT)/platform/emlib/src/em_gpio.c                                           \
    $(EFR32_SDK_ROOT)/platform/emlib/src/em_ldma.c                                           \
    $(EFR32_SDK_ROOT)/platform/emlib/src/em_leuart.c                                         \
    $(EFR32_SDK_ROOT)/platform/emlib/src/em_msc.c                                            \
    $(EFR32_SDK_ROOT)/platform/emlib/src/em_rmu.c                                            \
    $(EFR32_SDK_ROOT)/platform/emlib/src/em_rtcc.c                                           \
    $(EFR32_SDK_ROOT)/platform/emlib/src/em_system.c                                         \
    $(EFR32_SDK_ROOT)/platform/emlib/src/em_timer.c                                          \
    $(EFR32_SDK_ROOT)/platform/emlib/src/em_usart.c                                          \
    $(EFR32_SDK_ROOT)/platform/radio/rail_lib/hal/efr32/hal_efr.c                            \
    $(EFR32_SDK_ROOT)/platform/radio/rail_lib/hal/hal_common.c                               \
    $(EFR32_SDK_ROOT)/platform/service/mpu/src/sl_mpu.c                                      \
    $(EFR32_SDK_ROOT)/platform/service/sleeptimer/src/sl_sleeptimer.c                        \
    $(EFR32_SDK_ROOT)/platform/service/sleeptimer/src/sl_sleeptimer_hal_rtcc.c               \
    $(EFR32_SDK_ROOT)/hardware/kit/common/drivers/retargetio.c                               \
    $(EFR32_SDK_ROOT)/hardware/kit/common/drivers/retargetserial.c                           \

INC_DIRS = \
    $(PROJECT_ROOT)                                                                       \
    $(PROJECT_ROOT)/src/platform/$(EFR32FAMILY)/$(BOARD)                                  \
    $(PROJECT_ROOT)/include                                                               \
    $(PROJECT_ROOT)/traits/include                                                        \
    $(PROJECT_ROOT)/schema/include                                                        \
    $(CHIP_ROOT)/src/include/                                                             \
    $(CHIP_ROOT)/src/lib                                                                  \
    $(CHIP_ROOT)/src/                                                                     \
    $(CHIP_ROOT)/src/system                                                               \
    $(EFR32_SDK_ROOT)                                                                     \
    $(EFR32_SDK_ROOT)/platform/radio/rail_lib/common                                      \
    $(EFR32_SDK_ROOT)/platform/radio/rail_lib/chip/efr32                                  \
    $(EFR32_SDK_ROOT)/platform/radio/rail_lib/protocol/ieee802154                         \
    $(EFR32_SDK_ROOT)/platform/radio/rail_lib/chip/efr32/rf/common/cortex                 \
    $(EFR32_SDK_ROOT)/platform/radio/rail_lib/hal                                         \
    $(EFR32_SDK_ROOT)/platform/radio/rail_lib/hal/efr32                                   \
    $(EFR32_SDK_ROOT)/platform/radio/rail_lib/plugin/pa-conversions                       \
    $(EFR32_SDK_ROOT)/hardware/kit/common/bsp                                             \
    $(EFR32_SDK_ROOT)/platform/CMSIS/Include                                              \
    $(EFR32_SDK_ROOT)/platform/emdrv/common/inc                                           \
    $(EFR32_SDK_ROOT)/platform/emdrv/gpiointerrupt/inc                                    \
    $(EFR32_SDK_ROOT)/platform/emdrv/uartdrv/inc                                          \
    $(EFR32_SDK_ROOT)/platform/emdrv/uartdrv/config                                       \
    $(EFR32_SDK_ROOT)/platform/emdrv/ustimer/inc                                          \
    $(EFR32_SDK_ROOT)/platform/emdrv/dmadrv/inc                                           \
    $(EFR32_SDK_ROOT)/platform/emdrv/dmadrv/config                                        \
    $(EFR32_SDK_ROOT)/platform/emlib/inc                                                  \
    $(EFR32_SDK_ROOT)/platform/halconfig/inc/hal-config                                   \
    $(EFR32_SDK_ROOT)/platform/common/inc                                                 \
    $(EFR32_SDK_ROOT)/platform/service/mpu/inc                                            \
    $(EFR32_SDK_ROOT)/platform/service/sleeptimer/inc                                     \
    $(EFR32_SDK_ROOT)/platform/service/sleeptimer/config                                  \
    $(EFR32_SDK_ROOT)/util/plugin/plugin-common/fem-control                               \
    $(EFR32_SDK_ROOT)/hardware/kit/common/drivers/                                        \
    $(EFR32_SDK_ROOT)/hardware/kit/common/halconfig/                                      \
    $(EFR32_SDK_ROOT)/util/third_party/mbedtls/sl_crypto/include                          \

# Board specfic SRCs and INCs
ifeq ($(EFR32FAMILY), efr32mg12)
SRCS += \
    $(EFR32_SDK_ROOT)/hardware/kit/common/drivers/mx25flash_spi.c                            \
    $(EFR32_SDK_ROOT)/platform/Device/SiliconLabs/EFR32MG12P/Source/system_efr32mg12p.c      \
    $(EFR32_SDK_ROOT)/platform/Device/SiliconLabs/EFR32MG12P/Source/GCC/startup_efr32mg12p.c
INC_DIRS += \
    $(EFR32_SDK_ROOT)/hardware/kit/EFR32MG12_$(BOARD)/config                                 \
    $(EFR32_SDK_ROOT)/platform/Device/SiliconLabs/EFR32MG12P/Include
LINKER_SCRIPT=./ldscripts/$(APP)-MG12P.ld
else
ifeq ($(EFR32FAMILY), efr32mg21)
SRCS += \
    $(EFR32_SDK_ROOT)/platform/Device/SiliconLabs/EFR32MG21/Source/system_efr32mg21.c      \
    $(EFR32_SDK_ROOT)/platform/Device/SiliconLabs/EFR32MG21/Source/GCC/startup_efr32mg21.c \
    $(EFR32_SDK_ROOT)/platform/emlib/src/em_se.c                                           
INC_DIRS += \
    $(EFR32_SDK_ROOT)/util/third_party/mbedtls/include                                     \
    $(EFR32_SDK_ROOT)/hardware/kit/EFR32MG21_$(BOARD)/config                               \
    $(EFR32_SDK_ROOT)/platform/Device/SiliconLabs/EFR32MG21/Include                    
LINKER_SCRIPT=./ldscripts/$(APP)-MG21.ld
endif
endif

# The mainboard for 4166A does not have a display
ifeq ($(BOARD),BRD4166A)
    SHOW_QR_CODE=0
else
    SHOW_QR_CODE=1
endif

# Add LCD sources if allowed
ifeq ($(SHOW_QR_CODE), 1)
    SRCS += $(LCD_SRCS)
    INC_DIRS += $(LCD_INCLUDES)
    DEFINES += DISPLAY_ENABLED
endif

DEFINES += \
  $(BOARD)=1                                            \
  BOARD_ID=$(BOARD)                                     \
  HARD_FAULT_LOG_ENABLE                                 \
  RETARGET_VCOM                                         \
  RETARGET_USART0                                       \
  PLATFORM_HEADER='<hal/micro/cortexm3/compiler/gcc.h>' \
  CORTEXM3_EFM32_MICRO                                  \
  CHIP_ERROR_LOGGING=1                                  \
  CHIP_PROGRESS_LOGGING=1                               \
  CHIP_DETAIL_LOGGING=1                                 \
  EFR32_LOG_ENABLED=1                                   \
  NVM3_DEFAULT_NVM_SIZE=40960

CFLAGS = \
    -specs=nano.specs

LDFLAGS = \
    -specs=nano.specs

ifdef DEVICE_FIRMWARE_REVISION
DEFINES += \
    CHIP_DEVICE_CONFIG_DEVICE_FIRMWARE_REVISION=\"$(DEVICE_FIRMWARE_REVISION)\"
endif

CHIP_PROJECT_CONFIG = $(PROJECT_ROOT)/include/CHIPProjectConfig.h

clean ::

$(call GenerateBuildRules)
