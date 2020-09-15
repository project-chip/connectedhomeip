#
#
#   Copyright (c) 2020 Project CHIP Authors
#   Copyright (c) 2019 Google LLC.
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
#         Makefile for building the CHIP nRF52840 Lock Example Application.
#

PROJECT_ROOT := $(realpath .)

CHIP_ROOT ?= $(realpath $(PROJECT_ROOT)/third_party/connectedhomeip)
BUILD_SUPPORT_DIR = $(CHIP_ROOT)/config/qpg6100
QORVO_PLATFORM_DIR = $(CHIP_ROOT)/examples/platform/qpg6100

include $(BUILD_SUPPORT_DIR)/qpg6100-app.mk
include $(BUILD_SUPPORT_DIR)/qpg6100-chip.mk

PREINCLUDE_HEADER:=$(QORVO_PLATFORM_DIR)/inc/qorvo_config.h

APP = chip-qpg6100-lock-example

# Application defines
SRC_APP:=
SRC_APP+=$(PROJECT_ROOT)/src/main.cpp
SRC+=$(SRC_APP)

#Compilation flags are defined in $(QORVO_PLATFORM_DIR)/Work/P236_CHIP/lock-app_QPG6100_dk/gen/qorvo_config.h

SRC_gpFreeRTOS:=
SRC_gpFreeRTOS+=$(QORVO_PLATFORM_DIR)/comps/gpFreeRTOS/config/hooks.c
SRC+=$(SRC_gpFreeRTOS)
INC_gpFreeRTOS:=
INC_gpFreeRTOS+=$(QORVO_PLATFORM_DIR)/comps/gpFreeRTOS/config
INC_gpFreeRTOS+=$(QORVO_PLATFORM_DIR)/comps/gpFreeRTOS/Source/include
INC_gpFreeRTOS+=$(QORVO_PLATFORM_DIR)/comps/gpFreeRTOS/Source/Portable/GCC/ARM_CM3
INC+=$(INC_gpFreeRTOS)

INC_qvCHIP:=
INC_qvCHIP+=$(QORVO_PLATFORM_DIR)/comps/qvCHIP/inc
INC+=$(INC_qvCHIP)
LIB_qvCHIP:=
LIB_qvCHIP+=$(QORVO_PLATFORM_DIR)/lib/libCHIP_qpg6100.a
LIBS+=$(LIB_qvCHIP)

# TODO - Add OpenThread library
# LIB_qvOT:=
# LIB_qvOT+=$(PROJECT_ROOT)/third_party/openthread/repo/third_party/Qorvo/qpg6100/libQorvoQPG6100.a
# LIBS+=$(LIB_qvOT)

# Defines handled in pre-include header
DEFINES = 

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
    CHIP_DEFINES += CHIP_CONFIG_LOG_LEVEL=OT_LOG_LEVEL_INFO
endif

CHIP_PROJECT_CONFIG = $(PROJECT_ROOT)/include/CHIPProjectConfig.h

LINKER_SCRIPT = $(QORVO_PLATFORM_DIR)/ldscripts/chip-qpg6100-example.ld

$(call GenerateBuildRules)
