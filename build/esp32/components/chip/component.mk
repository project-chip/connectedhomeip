#
#    Copyright (c) 2018 Nest Labs, Inc.
#    All rights reserved.
#
#    Licensed under the Apache License, Version 2.0 (the "License");
#    you may not use this file except in compliance with the License.
#    You may obtain a copy of the License at
#
#        http://www.apache.org/licenses/LICENSE-2.0
#
#    Unless required by applicable law or agreed to in writing, software
#    distributed under the License is distributed on an "AS IS" BASIS,
#    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#    See the License for the specific language governing permissions and
#    limitations under the License.
#
#    Description:
#      Component makefile for building OpenWeave within the ESP32 ESP-IDF environment.
#

# ==================================================
# General settings
# ==================================================

# OpenWeave source root directory
OPENWEAVE_ROOT              ?= $(realpath $(COMPONENT_PATH)/../../../..)

# Archtecture for which Weave will be built.
HOST_ARCH                   := xtensa-unknown-linux-gnu

# Directory into which the Weave build system will place its output. 
OUTPUT_DIR					:= $(BUILD_DIR_BASE)/openweave
REL_OUTPUT_DIR              := $(shell perl -e 'use File::Spec; use Cwd; print File::Spec->abs2rel(Cwd::realpath($$ARGV[0]), Cwd::realpath($$ARGV[1])) . "\n"' $(OUTPUT_DIR) $(COMPONENT_PATH))

# Directory containing esp32-specific Weave project configuration files.
PROJECT_CONFIG_DIR          := $(OPENWEAVE_ROOT)/build/config/esp32

# Architcture on which Weave is being built.
BUILD_ARCH                  := $(shell $(OPENWEAVE_ROOT)/third_party/nlbuild-autotools/repo/third_party/autoconf/config.guess | sed -e 's/[[:digit:].]*$$//g')

# Directory containing the esp32-specific LwIP component sources.
LWIP_COMPONENT_DIR      	?= $(PROJECT_PATH)/components/lwip


# ==================================================
# Compilation flags specific to building OpenWeave
# ==================================================

# Include directories to be searched when building OpenWeave.
INCLUDES                    := $(OUTPUT_DIR)/src/include \
                               $(OUTPUT_DIR)/src/include/Weave/DeviceLayer/ESP32 \
                               $(OPENWEAVE_ROOT)/src/adaptations/device-layer/trait-support \
                               $(COMPONENT_INCLUDES)

# Compiler flags for building OpenWeave
CFLAGS                      += $(addprefix -I,$(INCLUDES))
CPPFLAGS                    += $(addprefix -I,$(INCLUDES))
CXXFLAGS                    += $(addprefix -I,$(INCLUDES))

INSTALL                     := /usr/bin/install
INSTALLFLAGS                := -C -v


# ==================================================
# Utility Functions
# ==================================================

QuoteChar = "

DoubleQuoteStr = $(QuoteChar)$(subst $(QuoteChar),\$(QuoteChar),$(subst \,\\,$(1)))$(QuoteChar)


# ==================================================
# OpenWeave configuration options
# ==================================================

# ESP-IDF's project.mk fails to define RANLIB appropriately, so we define it here.
RANLIB                      := $(call dequote,$(CONFIG_TOOLPREFIX))ranlib

CONFIGURE_OPTIONS       	:= AR="$(AR)" CC="$(CC)" CXX="$(CXX)" LD="$(LD)" OBJCOPY="$(OBJCOPY)" RANLIB="$(RANLIB)" \
                               INSTALL="$(INSTALL) $(INSTALLFLAGS)" \
                               CFLAGS=$(call DoubleQuoteStr, $(CFLAGS)) \
                               CPPFLAGS=$(call DoubleQuoteStr, $(CPPFLAGS)) \
                               CXXFLAGS=$(call DoubleQuoteStr, $(CXXFLAGS)) \
                               --prefix=$(OUTPUT_DIR) \
                               --exec-prefix=$(OUTPUT_DIR) \
                               --host=$(HOST_ARCH) \
                               --build=$(BUILD_ARCH) \
                               --with-device-layer=esp32 \
                               --with-network-layer=all \
                               --with-target-network=lwip \
                               --with-lwip=$(LWIP_COMPONENT_DIR) \
                               --with-inet-endpoint="tcp udp tun dns" \
                               --with-openssl=no \
                               --with-logging-style=external \
                               --with-weave-project-includes= \
                               --with-weave-system-project-includes= \
                               --with-weave-inet-project-includes= \
                               --with-weave-ble-project-includes= \
                               --with-weave-warm-project-includes= \
                               --disable-tests \
                               --disable-tools \
                               --disable-docs \
                               --disable-java \
                               --disable-device-manager

# Enable debug and disable optimization if ESP-IDF Optimization Level is set to Debug.
ifeq ($(CONFIG_OPTIMIZATION_LEVEL_DEBUG),y)
CONFIGURE_OPTIONS           += --enable-debug --enable-optimization=no
else
CONFIGURE_OPTIONS           +=  --enable-optimization=yes
endif


# ==================================================
# Configuration for the OpenWeave ESF-IDF Component
# ==================================================

# Header directories to be included when building other components that use OpenWeave.
# Note that these must be relative to the component source directory.
COMPONENT_ADD_INCLUDEDIRS 	 = project-config \
                               $(REL_OUTPUT_DIR)/include

# Linker flags to be included when building other components that use Weave. 
COMPONENT_ADD_LDFLAGS        = -L$(OUTPUT_DIR)/lib \
					           -lWeave \
					           -lInetLayer \
					           -lmincrypt \
					           -lnlfaultinjection \
					           -lSystemLayer \
					           -luECC \
					           -lWarm \
					           -lDeviceLayer

# Tell the ESP-IDF build system that the OpenWeave component defines its own build
# and clean targets.
COMPONENT_OWNBUILDTARGET 	 = 1
COMPONENT_OWNCLEANTARGET 	 = 1


# ==================================================
# Build Rules
# ==================================================

.PHONY : check-config-args-updated
check-config-args-updated : | $(OUTPUT_DIR)
	echo $(OPENWEAVE_ROOT)/configure $(CONFIGURE_OPTIONS) > $(OUTPUT_DIR)/config.args.tmp; \
	(test -r $(OUTPUT_DIR)/config.args && cmp -s $(OUTPUT_DIR)/config.args.tmp $(OUTPUT_DIR)/config.args) || \
	    mv $(OUTPUT_DIR)/config.args.tmp $(OUTPUT_DIR)/config.args; \
	rm -f $(OUTPUT_DIR)/config.args.tmp;

$(OUTPUT_DIR)/config.args : check-config-args-updated
	@: # Null action required to work around make's crazy timestamp caching behavior.

$(OPENWEAVE_ROOT)/configure : $(OPENWEAVE_ROOT)/configure.ac
	echo "BOOTSTRAP OPENWEAVE..."
	(cd $(OPENWEAVE_ROOT) && ./bootstrap)

$(OUTPUT_DIR)/config.status : $(OPENWEAVE_ROOT)/configure $(OUTPUT_DIR)/config.args
	echo "CONFIGURE OPENWEAVE..."
	(cd $(OUTPUT_DIR) && $(OPENWEAVE_ROOT)/configure $(CONFIGURE_OPTIONS))

configure-weave : $(OUTPUT_DIR)/config.status

$(OUTPUT_DIR) :
	echo "MKDIR $@"
	@mkdir -p "$@"

build-weave : configure-weave
	echo "BUILD OPENWEAVE..."
	MAKEFLAGS= make -C $(OUTPUT_DIR) --no-print-directory all

install-weave : | build-weave
	echo "INSTALL OPENWEAVE..."
	MAKEFLAGS= make -C $(OUTPUT_DIR) --no-print-directory install

build : build-weave install-weave

clean:
	echo "RM $(OUTPUT_DIR)"
	rm -rf $(OUTPUT_DIR)
