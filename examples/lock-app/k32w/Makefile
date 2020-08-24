#
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
#         Makefile for building the CHIP K32W Lock Example Application.
#

PROJECT_ROOT := $(realpath .)

CHIP_ROOT ?= $(realpath $(PROJECT_ROOT)/third_party/connectedhomeip)
BUILD_SUPPORT_DIR = $(CHIP_ROOT)/config/k32w

include $(BUILD_SUPPORT_DIR)/k32w061-app.mk
include $(BUILD_SUPPORT_DIR)/k32w061-chip.mk

APP = chip-k32w061-lock-example

SRCS = \
    $(PROJECT_ROOT)/main/main.cpp \
    $(PROJECT_ROOT)/main/AppTask.cpp \
    $(PROJECT_ROOT)/main/BoltLockManager.cpp \
    $(PROJECT_ROOT)/main/LEDWidget.cpp \
    $(PROJECT_ROOT)/main/support/FreeRtosMbedtlsMutex.c \
    $(K32W061_SDK_ROOT)/rtos/amazon-freertos/lib/FreeRTOS/tasks.c \
    $(K32W061_SDK_ROOT)/rtos/amazon-freertos/lib/FreeRTOS/portable/MemMang/heap_3.c \
    $(K32W061_SDK_ROOT)/rtos/amazon-freertos/lib/FreeRTOS/list.c \
    $(K32W061_SDK_ROOT)/rtos/amazon-freertos/lib/FreeRTOS/portable/GCC/ARM_CM3/port.c \
    $(K32W061_SDK_ROOT)/rtos/amazon-freertos/lib/FreeRTOS/queue.c \
    $(K32W061_SDK_ROOT)/rtos/amazon-freertos/lib/FreeRTOS/timers.c \
    $(K32W061_SDK_ROOT)/rtos/amazon-freertos/lib/FreeRTOS/event_groups.c \
    $(K32W061_SDK_ROOT)/middleware/wireless/framework/PDM/pdm_port.c \
    $(K32W061_SDK_ROOT)/middleware/wireless/framework/FunctionLib/FunctionLib.c \
    $(K32W061_SDK_ROOT)/middleware/wireless/framework/Flash/Internal/Flash_Adapter.c \
    $(K32W061_SDK_ROOT)/middleware/wireless/framework/Panic/Source/Panic.c \
    $(K32W061_SDK_ROOT)/middleware/wireless/framework/OSAbstraction/Source/fsl_os_abstraction_free_rtos.c \
    $(K32W061_SDK_ROOT)/middleware/wireless/framework/Reset/Reset.c \
    $(K32W061_SDK_ROOT)/middleware/wireless/framework/LED/Source/LED.c \
    $(K32W061_SDK_ROOT)/middleware/wireless/framework/Keyboard/Source/Keyboard.c \
    $(K32W061_SDK_ROOT)/middleware/wireless/framework/GPIO/GPIO_Adapter.c \
    $(K32W061_SDK_ROOT)/middleware/wireless/framework/TimersManager/Source/TimersManager.c \
    $(K32W061_SDK_ROOT)/middleware/wireless/framework/TimersManager/Source/TMR_Adapter.c \
    $(K32W061_SDK_ROOT)/boards/k32w061dk6/wireless_examples/openthread/reed/bm/board_utility.c \
    $(K32W061_SDK_ROOT)/boards/k32w061dk6/wireless_examples/openthread/reed/bm/board.c \
    $(K32W061_SDK_ROOT)/boards/k32w061dk6/wireless_examples/openthread/reed/bm/hardware_init.c \
    $(K32W061_SDK_ROOT)/devices/K32W061/drivers/fsl_common.c \
    $(K32W061_SDK_ROOT)/devices/K32W061/drivers/fsl_inputmux.c \
    $(K32W061_SDK_ROOT)/devices/K32W061/drivers/fsl_pint.c \
    $(K32W061_SDK_ROOT)/devices/K32W061/drivers/fsl_reset.c

INC_DIRS = \
    $(K32W061_SDK_ROOT)/rtos/amazon-freertos/lib/include \
    $(K32W061_SDK_ROOT)/rtos/amazon-freertos/lib/include/private \
    $(K32W061_SDK_ROOT)/rtos/amazon-freertos/lib/FreeRTOS/portable/GCC/ARM_CM3 \
    $(K32W061_SDK_ROOT)/rtos/amazon-freertos/lib/third_party/unity/src \
    $(K32W061_SDK_ROOT)/rtos/amazon-freertos/lib/include/ \
    $(K32W061_SDK_ROOT)/middleware/wireless/framework/OSAbstraction/Interface \
    $(K32W061_SDK_ROOT)/middleware/wireless/framework/FunctionLib/ \
    $(K32W061_SDK_ROOT)/middleware/wireless/framework/Flash/Internal/ \
    $(K32W061_SDK_ROOT)/middleware/wireless/framework/Panic/Interface/ \
    $(K32W061_SDK_ROOT)/middleware/wireless/framework/LED/Interface/ \
    $(K32W061_SDK_ROOT)/middleware/wireless/framework/Keyboard/Interface/ \
    $(K32W061_SDK_ROOT)/middleware/wireless/framework/TimersManager/Interface/ \
    $(K32W061_SDK_ROOT)/middleware/wireless/framework/RNG/Interface/ \
    $(K32W061_SDK_ROOT)/middleware/wireless/framework/GPIO/ \
    $(K32W061_SDK_ROOT)/middleware/wireless/framework/XCVR/DK6/ \
    $(K32W061_SDK_ROOT)/boards/k32w061dk6/wireless_examples/openthread/reed/bm/ \
    $(K32W061_SDK_ROOT)/devices/K32W061/drivers/ \
    $(K32W061_SDK_ROOT)/components/serial_manager/ \
    $(PROJECT_ROOT) \
    $(PROJECT_ROOT)/main \
    $(PROJECT_ROOT)/main/include \
    $(CHIP_ROOT)/src/include/ \
    $(CHIP_ROOT)/src/lib \
    $(CHIP_ROOT)/src/ \
    $(CHIP_ROOT)/src/system

LIBS = \
    -lopenthread-cli-ftd \
    -lopenthread-ftd \
    -lopenthread-k32w061_plat \
    -lopenthread-k32w061_sdk \
    -lmbedcrypto \
    $(K32W061_SDK_ROOT)/middleware/wireless/ieee-802.15.4/lib/libMiniMac.a \
    $(K32W061_SDK_ROOT)/middleware/wireless/framework/XCVR/lib/libRadio.a \
    $(K32W061_SDK_ROOT)/middleware/wireless/framework/PDM/Library/libPDM.a \

DEFINES = \
    CPU_K32W061HN \
    CPU_JN518X \
    CPU_JN518X_REV=2 \
    JENNIC_CHIP_FAMILY_NAME=_JN518x \
    SDK_DEBUGCONSOLE=0 \
    USE_RTOS=1 \
    FSL_RTOS_FREE_RTOS=1 \
    gUartDebugConsole_d=0 \
    gUsePdm_d=1 \
    gLEDSupported_d \
    gKeyBoardSupported_d \
    gPdmNbSegments=64 \
    osCustomStartup=1 \
    OM15082=1\
    gKBD_KeysCount_c=3 \
    gLEDsOnTargetBoardCnt_c=2 \
    gLED_InvertedMode_d=1 \
    ENABLE_RAM_VECTOR_TABLE=1 \
    gTMR_Enabled_d=1 \
    gTimestamp_Enabled_d=0 \
    gMainThreadStackSize_c=2048 \
    K32W_LOG_ENABLED=1 \
    CHIP_ENABLE_OPENTHREAD=1

CFLAGS = \
    --specs=nano.specs

LDFLAGS = \
    --specs=nano.specs

ifdef DEVICE_FIRMWARE_REVISION
DEFINES += \
    CHIP_DEVICE_CONFIG_DEVICE_FIRMWARE_REVISION=\"$(DEVICE_FIRMWARE_REVISION)\"
endif

# A product's firmware usually comes with two build "flavors":
# 1) a "development" build which typically enables debugging artifacts along with logging,
# and disables security and optimizations and
# 2) a "release" build which targets end-user devices
# and disables debugging artifacts and logging, and enables security and optimizations.
#
# To facilitate supporting these two build flavors, the sample app source code supports
# the build configuration option BUILD_RELEASE. By default, BUILD_RELEASE is not defined
# and a development build is produced. If make is invoked with BUILD_RELEASE=1, then
# a "pseudo-release" build is produced.
#
# We say "pseudo-release" build because the sample app is never to be used as-is
# as the firmware for an end-user device. However, since real products may start off with
# the sample app source code, we show how the BUILD_RELEASE build configuration option
# can be used to produce two build flavors (development and release) of the firmware.
#
# IMPORTANT: By no means does the use of BUILD_RELEASE in the sample app cover all key aspects that
# one should be concerned with (e.g. security, performance) when targeting real world end-user
# devices. This is simply provided for guidance on how to support two build flavors.

ifdef BUILD_RELEASE
    DEFINES += BUILD_RELEASE=1
    # Disable Thread logging.
    CHIP_DEFINES += CHIP_CONFIG_LOG_LEVEL=OT_LOG_LEVEL_NONE
else
    DEFINES += BUILD_RELEASE=0
    # Increase Tread logging level to INFO.
    CHIP_DEFINES += CHIP_CONFIG_LOG_LEVEL=OT_LOG_LEVEL_DEBG
endif

CHIP_PROJECT_CONFIG = $(PROJECT_ROOT)/main/include/CHIPProjectConfig.h

OPENTHREAD_CONFIGURE_OPTIONS += \
    --enable-child-supervision \
    --enable-jam-detection

LINKER_SCRIPT = $(PROJECT_ROOT)/main/ldscripts/chip-k32w061-lock-example.ld

$(call GenerateBuildRules)
