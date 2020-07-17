#
#    Copyright (c) 2020 Project CHIP Authors
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
#      Component makefile for building CHIP within the ESP32 ESP-IDF environment.
#

# ==================================================
# General settings
# ==================================================

# CHIP source root directory
CHIP_ROOT              ?= $(realpath $(COMPONENT_PATH)/../../../..)

# Archtecture for which CHIP will be built.
HOST_ARCH                   := xtensa-unknown-linux-gnu

# Directory into which the CHIP build system will place its output.
OUTPUT_DIR					:= $(BUILD_DIR_BASE)/chip
REL_OUTPUT_DIR              := $(shell perl -e 'use File::Spec; use Cwd; print File::Spec->abs2rel(Cwd::realpath($$ARGV[0]), Cwd::realpath($$ARGV[1])) . "\n"' $(OUTPUT_DIR) $(COMPONENT_PATH))

REL_CHIP_ROOT               := $(shell perl -e 'use File::Spec; use Cwd; print File::Spec->abs2rel(Cwd::realpath($$ARGV[0]), Cwd::realpath($$ARGV[1])) . "\n"' $(CHIP_ROOT) $(COMPONENT_PATH))

# Directory containing esp32-specific CHIP project configuration files.
PROJECT_CONFIG_DIR          := $(CHIP_ROOT)/build/config/esp32

# Architcture on which CHIP is being built.
BUILD_ARCH                  := $(shell $(CHIP_ROOT)/third_party/nlbuild-autotools/repo/third_party/autoconf/config.guess | sed -e 's/[[:digit:].]*$$//g')

# Directory containing the esp32-specific LwIP component sources.
LWIP_COMPONENT_DIR      	?= $(IDF_PATH)/components/lwip

COMPONENT_LIBRARIES         :=


# ==================================================
# Compilation flags specific to building CHIP
# ==================================================

# Include directories to be searched when building CHIP.
INCLUDES                    := $(OUTPUT_DIR)/src/include \
                               $(OUTPUT_DIR)/src/include/platform/ESP32 \
                               $(IDF_PATH)/components/lwip/lwip/src/include \
                               $(IDF_PATH)/components/freertos/include/freertos/ \
                               $(IDF_PATH)/components/mbedtls/mbedtls/include \
                               $(COMPONENT_INCLUDES)

# Compiler flags for building CHIP
CFLAGS                      += $(addprefix -I,$(INCLUDES))
CPPFLAGS                    += $(addprefix -I,$(INCLUDES))
CXXFLAGS                    += $(addprefix -I,$(INCLUDES))

INSTALL                     := /usr/bin/install
INSTALLFLAGS                := -C -v


# ==================================================
# Utility Functions
# ==================================================

QuoteChar = "# " # the '# "' added here to allow syntax coloring to work in emacs ;)
MBEDTLS_CONFIG_PATH = -DMBEDTLS_CONFIG_FILE='"mbedtls/esp_config.h"'

DoubleQuoteStr = $(QuoteChar)$(subst $(QuoteChar),\$(QuoteChar),$(subst $(MBEDTLS_CONFIG_PATH),,$(subst \,\\,$(1))))$(QuoteChar)


# ==================================================
# CHIP configuration options
# ==================================================

# ESP-IDF's project.mk fails to define RANLIB appropriately, so we define it here.
RANLIB                      := $(call dequote,$(CONFIG_TOOLPREFIX))ranlib

CONFIGURE_OPTIONS       	:= -C AR="$(AR)" CC="$(CC)" CXX="$(CXX)" LD="$(LD)" OBJCOPY="$(OBJCOPY)" RANLIB="$(RANLIB)" \
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
                               --with-inet-endpoint="tcp udp" \
                               --with-logging-style=external \
                               --with-chip-project-includes=$(CONFIG_CHIP_PROJECT_CONFIG) \
                               --with-chip-system-project-includes= \
                               --with-chip-inet-project-includes= \
                               --with-chip-ble-project-includes= \
                               --with-chip-warm-project-includes= \
                               --with-crypto=mbedtls \
                               --with-mbedtls-includes=$(IDF_PATH)/components/mbedtls/mbedtls/include \
                               --with-mbedtls-libs=$(BUILD_DIR_BASE)/mbedtls \
                               --with-target-style=embedded \
                               --disable-tools \
                               --disable-docs \
                               --disable-java \
                               --disable-device-manager

ifneq (,$(findstring CHIP_SUPPORT_FOREIGN_TEST_DRIVERS,$(CXXFLAGS)))
CONFIGURE_OPTIONS           += --enable-tests --enable-nlfaultinjection
else
CONFIGURE_OPTIONS           += --disable-tests
endif

# Enable debug and disable optimization if ESP-IDF Optimization Level is set to Debug.
ifeq ($(CONFIG_OPTIMIZATION_LEVEL_DEBUG),y)
CONFIGURE_OPTIONS           += --enable-debug --enable-optimization=no
else
CONFIGURE_OPTIONS           +=  --enable-optimization=yes
endif


# ==================================================
# Configuration for the CHIP ESF-IDF Component
# ==================================================

# Header directories to be included when building other components that use CHIP.
# Note that these must be relative to the component source directory.
# TODO Boot the CHIP_ROOT includedirs
COMPONENT_ADD_INCLUDEDIRS 	 = project-config \
                               $(REL_OUTPUT_DIR)/include \
                               $(REL_CHIP_ROOT)/src/include/platform/ESP32 \
                               $(REL_CHIP_ROOT)/src/include/ \
                               $(REL_CHIP_ROOT)/src/lib \
                               $(REL_CHIP_ROOT)/src/ \
                               $(REL_CHIP_ROOT)/src/system \
                               $(IDF_PATH)/components/mbedtls/mbedtls/include \
                               $(REL_CHIP_ROOT)/src/app \


# Linker flags to be included when building other components that use CHIP.
COMPONENT_ADD_LDFLAGS        = -L$(OUTPUT_DIR)/lib/ \
                               -lCHIP \
                               -lInetLayer \
                               -lSystemLayer \
                               -lDeviceLayer \
                               -lChipCrypto \
                               -lSetupPayload

ifneq (,$(findstring CHIP_SUPPORT_FOREIGN_TEST_DRIVERS,$(CXXFLAGS)))
COMPONENT_ADD_LDFLAGS       += -lnlfaultinjection
endif

# Tell the ESP-IDF build system that the CHIP component defines its own build
# and clean targets.
COMPONENT_OWNBUILDTARGET 	 = 1
COMPONENT_OWNCLEANTARGET 	 = 1


# ==================================================
# Build Rules
# ==================================================

.PHONY : check-config-args-updated
check-config-args-updated : | $(OUTPUT_DIR)
	echo $(CHIP_ROOT)/configure $(CONFIGURE_OPTIONS) > $(OUTPUT_DIR)/config.args.tmp; \
	(test -r $(OUTPUT_DIR)/config.args && cmp -s $(OUTPUT_DIR)/config.args.tmp $(OUTPUT_DIR)/config.args) || \
	    mv $(OUTPUT_DIR)/config.args.tmp $(OUTPUT_DIR)/config.args; \
	rm -f $(OUTPUT_DIR)/config.args.tmp;

$(OUTPUT_DIR)/config.args : check-config-args-updated
	@: # Null action required to work around make's crazy timestamp caching behavior.

$(CHIP_ROOT)/configure : $(CHIP_ROOT)/configure.ac
	echo "BOOTSTRAP CHIP..."
	(cd $(CHIP_ROOT) && ./bootstrap)

$(OUTPUT_DIR)/config.status : $(CHIP_ROOT)/configure $(OUTPUT_DIR)/config.args
	echo "CONFIGURE CHIP..."
	(cd $(OUTPUT_DIR) && $(CHIP_ROOT)/configure $(CONFIGURE_OPTIONS))

configure-chip : $(OUTPUT_DIR)/config.status

$(OUTPUT_DIR) :
	echo "MKDIR $@"
	@mkdir -p "$@"

install-chip : configure-chip
	echo "INSTALL CHIP..."
	MAKEFLAGS= make -C $(OUTPUT_DIR) --no-print-directory install

build : install-chip
	echo "CHIP built and installed..."
	cp ${OUTPUT_DIR}/lib/libCHIP.a ${OUTPUT_DIR}/libchip.a

clean:
	echo "RM $(OUTPUT_DIR)"
	rm -rf $(OUTPUT_DIR)
