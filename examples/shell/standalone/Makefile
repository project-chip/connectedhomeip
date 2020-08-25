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
#         Makefile for building the CHIP Standalone Example Application.
#

PROJECT_ROOT := $(realpath ..)

CHIP_ROOT = $(realpath $(PROJECT_ROOT)/third_party/connectedhomeip)
BUILD_SUPPORT_DIR = $(CHIP_ROOT)/config/standalone

include $(BUILD_SUPPORT_DIR)/standalone-app.mk
include $(BUILD_SUPPORT_DIR)/standalone-chip.mk

APP = chip-shell

LIBS = -lChipShell

SRCS = \
    $(PROJECT_ROOT)/main.cpp \
    $(PROJECT_ROOT)/cmd_misc.cpp \
    $(PROJECT_ROOT)/cmd_base64.cpp \
    $(PROJECT_ROOT)/cmd_device.cpp \
    $(NULL)

INC_DIRS = \
    $(PROJECT_ROOT) \
    $(PROJECT_ROOT)/include \
    $(CHIP_ROOT)/src/ \
    $(CHIP_ROOT)/src/lib \
    $(CHIP_ROOT)/src/system \
    $(CHIP_ROOT)/config/standalone \
    $(NULL)

DEFINES = \
    USE_APP_CONFIG \
    $(NULL)

CXXFLAGS += -std=c++11

ifdef BUILD_RELEASE
    DEFINES += BUILD_RELEASE=1
else
    DEFINES += BUILD_RELEASE=0
endif

CHIP_PROJECT_CONFIG = $(PROJECT_ROOT)/include/CHIPProjectConfig.h

$(call GenerateBuildRules)
