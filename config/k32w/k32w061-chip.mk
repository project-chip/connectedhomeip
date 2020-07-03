#
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
#         Component makefile for incorporating OpenWeave into a k32w061
#         application.
#
#   This makefile is intended to work in conjunction with the k32w061-app.mk
#   makefile to build the OpenWeave example applications on NXP platforms.
#   k32w061 applications should include this file in their top level Makefile
#   after including k32w061-app.mk.  E.g.:
#
#       PROJECT_ROOT = $(realpath .)
#
#       BUILD_SUPPORT_DIR = $(PROJECT_ROOT)/third_party/openweave-core/build/k32w061
#
#       include $(BUILD_SUPPORT_DIR)/k32w061-app.mk
#       include $(BUILD_SUPPORT_DIR)/k32w061-openweave.mk
#
#       PROJECT_ROOT := $(realpath .)
#
#       APP := openweave-k32w061-bringup
#
#       SRCS = \
#           $(PROJECT_ROOT)/main.cpp \
#           ...
#
#       $(call GenerateBuildRules)
#
# ==================================================
# General settings
# ==================================================

# Location of OpenWeave source tree
OPENWEAVE_ROOT ?= $(PROJECT_ROOT)/third_party/openweave-core

# Archtecture for which OpenWeave will be built.
OPENWEAVE_HOST_ARCH := armv7-unknown-linux-gnu

# Directory into which the OpenWeave build system will place its output.
OPENWEAVE_OUTPUT_DIR = $(OUTPUT_DIR)/openweave

# An optional file containing application-specific configuration overrides.
OPENWEAVE_PROJECT_CONFIG = $(wildcard $(PROJECT_ROOT)/WeaveProjectConfig.h)

# Architcture on which OpenWeave is being built.
OPENWEAVE_BUILD_ARCH = $(shell $(OPENWEAVE_ROOT)/third_party/nlbuild-autotools/repo/third_party/autoconf/config.guess | sed -e 's/[[:digit:].]*$$//g')


# ==================================================
# Compilation flags specific to building OpenWeave
# ==================================================

OPENWEAVE_DEFINES += \
    NEW_OPENTHREAD_API

OPENWEAVE_DEFINE_FLAGS = $(foreach def,$(OPENWEAVE_DEFINES),-D$(def))
OPENWEAVE_CPPFLAGS = $(STD_CFLAGS) $(CFLAGS) $(DEBUG_FLAGS) $(OPT_FLAGS) $(DEFINE_FLAGS) $(OPENWEAVE_DEFINE_FLAGS) $(INC_FLAGS)
OPENWEAVE_CXXFLAGS = $(STD_CXXFLAGS) $(CXXFLAGS)

# ==================================================
# OpenWeave configuration options
# ==================================================

OPENWEAVE_CONFIGURE_OPTIONS = \
    AR="$(AR)" AS="$(AS)" CC="$(CCACHE) $(CC)" CXX="$(CCACHE) $(CXX)" \
    LD="$(LD)" OBJCOPY="$(OBJCOPY)" RANLIB="$(RANLIB)" INSTALL="$(INSTALL) $(INSTALLFLAGS)" \
    CPPFLAGS="$(OPENWEAVE_CPPFLAGS)" \
    CXXFLAGS="$(OPENWEAVE_CXXFLAGS)" \
    --prefix=$(OPENWEAVE_OUTPUT_DIR) \
    --exec-prefix=$(OPENWEAVE_OUTPUT_DIR) \
    --host=$(OPENWEAVE_HOST_ARCH) \
    --build=$(OPENWEAVE_BUILD_ARCH) \
    --with-target-style=embedded \
    --with-device-layer=k32w \
    --with-network-layer=all \
    --with-target-network=lwip \
    --with-lwip=internal \
    --with-lwip-target=k32w \
    --with-inet-endpoint="tcp udp" \
    --with-openssl=no \
    --with-logging-style=external \
    --with-weave-project-includes=$(OPENWEAVE_PROJECT_CONFIG) \
    --with-weave-system-project-includes=$(OPENWEAVE_PROJECT_CONFIG) \
    --with-weave-inet-project-includes=$(OPENWEAVE_PROJECT_CONFIG) \
    --with-weave-ble-project-includes=$(OPENWEAVE_PROJECT_CONFIG) \
    --with-weave-warm-project-includes=$(OPENWEAVE_PROJECT_CONFIG) \
    --with-weave-device-project-includes=$(OPENWEAVE_PROJECT_CONFIG) \
    --disable-ipv4 \
    --disable-tests \
    --disable-tools \
    --disable-docs \
    --disable-java \
    --disable-device-manager

# Enable / disable optimization.
ifeq ($(OPT),1)
OPENWEAVE_CONFIGURE_OPTIONS += --enable-optimization=yes
else
OPENWEAVE_CONFIGURE_OPTIONS += --enable-optimization=no
endif

ifeq ($(DEBUG),1)
OPENWEAVE_CONFIGURE_OPTIONS += --enable-debug
endif
# ==================================================
# Adjustments to standard build settings to
#   incorporate OpenWeave
# ==================================================

# Add OpenWeave-specific paths to the standard include directories.
STD_INC_DIRS += \
    $(OPENWEAVE_OUTPUT_DIR)/include \
    $(OPENWEAVE_OUTPUT_DIR)/src/include \
    $(OPENWEAVE_ROOT)/src/adaptations/device-layer/trait-support \
    $(OPENWEAVE_ROOT)/third_party/lwip/repo/lwip/src/include \
    $(OPENWEAVE_ROOT)/src/lwip \
    $(OPENWEAVE_ROOT)/src/lwip/k32w \
    $(OPENWEAVE_ROOT)/src/lwip/freertos \
    $(K32W061_SDK_ROOT)/CMSIS/Include \
    $(K32W061_SDK_ROOT)/devices/K32W061 \
    $(K32W061_SDK_ROOT)/devices/K32W061/drivers \
    $(K32W061_SDK_ROOT)/devices/K32W061/utilities \
    $(K32W061_SDK_ROOT)/devices/K32W061/utilities/debug_console \
    $(K32W061_SDK_ROOT)/devices/K32W061/utilities/str \
    $(K32W061_SDK_ROOT)/middleware/wireless/framework/Common \
    $(K32W061_SDK_ROOT)/middleware/wireless/framework/Lists \
    $(K32W061_SDK_ROOT)/middleware/wireless/framework/PDM/Include \
    $(K32W061_SDK_ROOT)/middleware/wireless/framework/MemManager/Interface

# Add the location of OpenWeave libraries to application link action.
STD_LDFLAGS += -L$(OPENWEAVE_OUTPUT_DIR)/lib

# Add OpenWeave libraries to standard libraries list.
STD_LIBS += \
    -lDeviceLayer \
	-lWeave \
	-lWarm \
	-lInetLayer \
	-lmincrypt \
	-lnlfaultinjection \
	-lSystemLayer \
	-luECC \
	-llwip

# Add the appropriate OpenWeave target as a prerequisite to all application
# compilation targets to ensure that OpenWeave gets built and its header
# files installed prior to compiling any dependent source files.
STD_COMPILE_PREREQUISITES += install-weave

# Add the OpenWeave libraries as prerequisites for linking the application.
STD_LINK_PREREQUISITES += \
    $(OPENWEAVE_OUTPUT_DIR)/lib/libDeviceLayer.a \
	$(OPENWEAVE_OUTPUT_DIR)/lib/libWeave.a \
	$(OPENWEAVE_OUTPUT_DIR)/lib/libWarm.a \
	$(OPENWEAVE_OUTPUT_DIR)/lib/libInetLayer.a \
	$(OPENWEAVE_OUTPUT_DIR)/lib/libmincrypt.a \
	$(OPENWEAVE_OUTPUT_DIR)/lib/libnlfaultinjection.a \
	$(OPENWEAVE_OUTPUT_DIR)/lib/libSystemLayer.a \
	$(OPENWEAVE_OUTPUT_DIR)/lib/libuECC.a \
	$(OPENWEAVE_OUTPUT_DIR)/lib/liblwip.a


# ==================================================
# Late-bound build rules for OpenWeave
# ==================================================

# Add OpenWeaveBuildRules to the list of late-bound build rules that
# will be evaluated when GenerateBuildRules is called.
LATE_BOUND_RULES += OpenWeaveBuildRules

# Rules for configuring, building and installing OpenWeave.
define OpenWeaveBuildRules

.PHONY : config-weave .check-config-weave build-weave install-weave clean-weave

.check-config-weave : | $(OPENWEAVE_OUTPUT_DIR)
	$(NO_ECHO)echo $(OPENWEAVE_ROOT)/configure $(OPENWEAVE_CONFIGURE_OPTIONS) > $(OPENWEAVE_OUTPUT_DIR)/config.args.tmp; \
	(test -r $(OPENWEAVE_OUTPUT_DIR)/config.args && cmp -s $(OPENWEAVE_OUTPUT_DIR)/config.args.tmp $(OPENWEAVE_OUTPUT_DIR)/config.args) || \
	    mv $(OPENWEAVE_OUTPUT_DIR)/config.args.tmp $(OPENWEAVE_OUTPUT_DIR)/config.args; \
	 rm -f $(OPENWEAVE_OUTPUT_DIR)/config.args.tmp;

$(OPENWEAVE_OUTPUT_DIR)/config.args : .check-config-weave
	@: # Null action required to work around make's crazy timestamp caching behavior.

$(OPENWEAVE_ROOT)/configure : $(OPENWEAVE_ROOT)/configure.ac
	@echo "$(HDR_PREFIX)BOOTSTRAP OPENWEAVE..."
	$(NO_ECHO)(cd $(OPENWEAVE_ROOT) && ./bootstrap)

$(OPENWEAVE_OUTPUT_DIR)/config.status : $(OPENWEAVE_ROOT)/configure $(OPENWEAVE_OUTPUT_DIR)/config.args
	@echo "$(HDR_PREFIX)CONFIGURE OPENWEAVE..."
	$(NO_ECHO)(cd $(OPENWEAVE_OUTPUT_DIR) && $(OPENWEAVE_ROOT)/configure $(OPENWEAVE_CONFIGURE_OPTIONS))

config-weave : $(OPENWEAVE_OUTPUT_DIR)/config.status | $(OPENTHREAD_PREREQUISITE)

build-weave : config-weave
	@echo "$(HDR_PREFIX)BUILD OPENWEAVE..."
	$(NO_ECHO)MAKEFLAGS= make -C $(OPENWEAVE_OUTPUT_DIR) --no-print-directory all V=$(VERBOSE)

install-weave : | build-weave
	@echo "$(HDR_PREFIX)INSTALL OPENWEAVE..."
	$(NO_ECHO)MAKEFLAGS= make -C $(OPENWEAVE_OUTPUT_DIR) --no-print-directory install V=$(VERBOSE)

clean-weave:
	@echo "$(HDR_PREFIX)RM $(OPENWEAVE_OUTPUT_DIR)"
	$(NO_ECHO)rm -rf $(OPENWEAVE_OUTPUT_DIR)

$(OPENWEAVE_OUTPUT_DIR) :
	@echo "$(HDR_PREFIX)MKDIR $$@"
	$(NO_ECHO)mkdir -p "$$@"

endef
# ==================================================
# OpenWeave-specific help definitions
# ==================================================

define TargetHelp +=


  config-weave          Run the OpenWeave configure script.

  build-weave           Build the OpenWeave libraries.

  install-weave         Install OpenWeave libraries and headers in
                        build output directory for use by application.

  clean-weave           Clean all build outputs produced by the OpenWeave
                        build process.
endef
