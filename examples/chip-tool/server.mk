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

PROJECT_ROOT := $(realpath .)

CHIP_ROOT = $(realpath $(PROJECT_ROOT)/third_party/connectedhomeip)
BUILD_SUPPORT_DIR = $(CHIP_ROOT)/config/standalone

include $(BUILD_SUPPORT_DIR)/standalone-app.mk
include $(BUILD_SUPPORT_DIR)/standalone-chip.mk

APP = chip-tool-server

SRCS = \
    $(PROJECT_ROOT)/server.cpp \
    $(CHIP_ROOT)/src/app/util/attribute-size.c \
    $(CHIP_ROOT)/src/app/util/attribute-storage.c \
    $(CHIP_ROOT)/src/app/util/attribute-table.c \
    $(CHIP_ROOT)/src/app/util/chip-response.cpp \
    $(CHIP_ROOT)/src/app/util/client-api.c \
    $(CHIP_ROOT)/src/app/util/ember-print.cpp \
    $(CHIP_ROOT)/src/app/util/message.c \
    $(CHIP_ROOT)/src/app/util/process-cluster-message.c \
    $(CHIP_ROOT)/src/app/util/process-global-message.c \
    $(CHIP_ROOT)/src/app/util/util.c \
    $(CHIP_ROOT)/src/app/clusters/on-off-server/on-off.c \
    $(PROJECT_ROOT)/gen/call-command-handler.c \
    $(PROJECT_ROOT)/gen/callback-stub.c \
    $(PROJECT_ROOT)/gen/znet-bookkeeping.c \
    $(NULL)

INC_DIRS = \
    $(PROJECT_ROOT) \
    $(PROJECT_ROOT)/include \
    $(CHIP_ROOT)/src/lib \
    $(CHIP_ROOT)/src/ \
    $(CHIP_ROOT)/src/system \
    $(CHIP_ROOT)/src/controller \
    $(CHIP_ROOT)/config/standalone \
    $(CHIP_ROOT)/src/app \
    $(CHIP_ROOT)/src/app/util \
    $(NULL)

DEFINES = \
    USE_APP_CONFIG \
    $(NULL)

# Ignore the dead code
CXXFLAGS += -std=c++11 -O3 -flto -ffunction-sections
CFLAGS += -O3 -flto -ffunction-sections
OPT_FLAGS += -O3 -flto

ifdef BUILD_RELEASE
    DEFINES += BUILD_RELEASE=1
else
    DEFINES += BUILD_RELEASE=0
endif

CHIP_PROJECT_CONFIG = $(PROJECT_ROOT)/include/CHIPProjectConfig.h

$(call GenerateBuildRules)
