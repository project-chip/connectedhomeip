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

SHELL = /bin/bash

# CHIP source root directory
CHIP_ROOT              ?= $(realpath $(COMPONENT_PATH)/../../../..)

# Directory into which the CHIP build system will place its output.
OUTPUT_DIR					:= $(BUILD_DIR_BASE)/chip
REL_OUTPUT_DIR              := $(shell perl -e 'use File::Spec; use Cwd; print File::Spec->abs2rel(Cwd::realpath($$ARGV[0]), Cwd::realpath($$ARGV[1])) . "\n"' $(OUTPUT_DIR) $(COMPONENT_PATH))

REL_CHIP_ROOT               := $(shell perl -e 'use File::Spec; use Cwd; print File::Spec->abs2rel(Cwd::realpath($$ARGV[0]), Cwd::realpath($$ARGV[1])) . "\n"' $(CHIP_ROOT) $(COMPONENT_PATH))

COMPONENT_LIBRARIES         :=


# ==================================================
# Compilation flags specific to building CHIP
# ==================================================

# Include directories to be searched when building CHIP.  Make sure
# that anything starting with $(IDF_PATH) ends up being included with
# -isystem, not -I, so warnings in those headers don't cause the build
# to fail.
INCLUDES                    := $(OUTPUT_DIR)/src/include \
                               $(OUTPUT_DIR)/src/include/platform/ESP32 \
                               $(filter-out $(IDF_PATH)/%, $(COMPONENT_INCLUDES))

SYSTEM_INCLUDES             := $(IDF_PATH)/components/lwip/lwip/src/include \
                               $(IDF_PATH)/components/freertos/include/freertos/ \
                               $(IDF_PATH)/components/mbedtls/mbedtls/include \
                               $(filter $(IDF_PATH)/%, $(COMPONENT_INCLUDES))


# Compiler flags for building CHIP
ALL_INCLUDES                := $(addprefix -I,$(INCLUDES)) $(addprefix -isystem,$(SYSTEM_INCLUDES))
CFLAGS                      += $(ALL_INCLUDES)
CPPFLAGS                    += $(ALL_INCLUDES)
CXXFLAGS                    += $(ALL_INCLUDES)


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
                               $(REL_CHIP_ROOT)/src/app

# Linker flags to be included when building other components that use CHIP.
COMPONENT_ADD_LDFLAGS        = -L$(OUTPUT_DIR)/lib/ \
                               -lCHIP

ifdef CONFIG_ENABLE_CHIP_SHELL
COMPONENT_ADD_LDFLAGS        += -lCHIPShell
endif

ifdef CONFIG_ENABLE_PW_RPC
COMPONENT_ADD_LDFLAGS        += -lPwRpc
endif

COMPONENT_ADD_INCLUDEDIRS +=   $(REL_OUTPUT_DIR)/src/include \
                               $(REL_CHIP_ROOT)/third_party/nlassert/repo/include \
                               $(REL_OUTPUT_DIR)/gen/third_party/connectedhomeip/src/app/include \
                               $(REL_OUTPUT_DIR)/gen/include

# Tell the ESP-IDF build system that the CHIP component defines its own build
# and clean targets.
COMPONENT_OWNBUILDTARGET 	 = 1
COMPONENT_OWNCLEANTARGET 	 = 1

is_debug ?= true

# ==================================================
# Build Rules
# ==================================================

$(OUTPUT_DIR) :
	echo "MKDIR $@"
	@mkdir -p "$@"


fix_cflags = $(filter-out -DHAVE_CONFIG_H,\
                $(filter-out -D,\
                  $(filter-out IDF_VER%,\
                      $(1) -D$(filter IDF_VER%,$(1))\
               )))
CHIP_CFLAGS = $(call fix_cflags,$(CFLAGS) $(CPPFLAGS))
CHIP_CXXFLAGS = $(call fix_cflags,$(CXXFLAGS) $(CPPFLAGS))

install-chip : $(OUTPUT_DIR)
	echo "INSTALL CHIP..."
	echo                                   > $(OUTPUT_DIR)/args.gn
	echo "import(\"//args.gni\")"          >> $(OUTPUT_DIR)/args.gn
	echo target_cflags_c  = [$(foreach word,$(CHIP_CFLAGS),\"$(word)\",)] | sed -e 's/=\"/=\\"/g;s/\"\"/\\"\"/g;'  >> $(OUTPUT_DIR)/args.gn
	echo target_cflags_cc = [$(foreach word,$(CHIP_CXXFLAGS),\"$(word)\",)] | sed -e 's/=\"/=\\"/g;s/\"\"/\\"\"/g;'   >> $(OUTPUT_DIR)/args.gn
	echo esp32_ar = \"$(AR)\"                >> $(OUTPUT_DIR)/args.gn
	echo esp32_cc = \"$(CC)\"                >> $(OUTPUT_DIR)/args.gn
	echo esp32_cxx = \"$(CXX)\"              >> $(OUTPUT_DIR)/args.gn
	echo esp32_cpu = \"esp32\"               >> $(OUTPUT_DIR)/args.gn
ifeq ($(is_debug),false)
	@echo "is_debug = false" >> $(OUTPUT_DIR)/args.gn
endif
	if [[ "$(CONFIG_ENABLE_PW_RPC)" = "y" ]]; then                        \
	  echo "chip_build_pw_rpc_lib = true" >> $(OUTPUT_DIR)/args.gn       ;\
	  echo "remove_default_configs = [\"//third_party/connectedhomeip/third_party/pigweed/repo/pw_build:cpp17\"]" >> $(OUTPUT_DIR)/args.gn		;\
	  echo "pw_log_BACKEND = \"//third_party/connectedhomeip/third_party/pigweed/repo/pw_log_basic\"" >> $(OUTPUT_DIR)/args.gn     ;\
	  echo "pw_assert_BACKEND = \"//third_party/connectedhomeip/third_party/pigweed/repo/pw_assert_log\"" >> $(OUTPUT_DIR)/args.gn ;\
	  echo "pw_sys_io_BACKEND = \"//third_party/connectedhomeip/examples/platform/esp32/pw_sys_io:pw_sys_io_esp32\"" >> $(OUTPUT_DIR)/args.gn      ;\
	  echo "dir_pw_third_party_nanopb = \"//third_party/connectedhomeip/third_party/nanopb/repo\"" >>$(OUTPUT_DIR)/args.gn         ;\
	fi
	if [[ "$(CONFIG_ENABLE_CHIP_SHELL)" = "y" ]]; then \
	  echo "chip_build_libshell = true" >> $(OUTPUT_DIR)/args.gn ;\
	fi
	if [[ "$(CONFIG_USE_MINIMAL_MDNS)" = "n" ]]; then \
	  echo "chip_mdns = platform" >> $(OUTPUT_DIR)/args.gn ;\
	fi
	echo "Written file $(OUTPUT_DIR)/args.gn"
	cd $(CHIP_ROOT) && PW_ENVSETUP_QUIET=1 . scripts/activate.sh && cd $(COMPONENT_PATH) && gn gen --check --fail-on-unused-args $(OUTPUT_DIR)
	cd $(COMPONENT_PATH); ninja $(subst 1,-v,$(filter 1,$(V))) -C $(OUTPUT_DIR) esp32


build : install-chip
	echo "CHIP built and installed..."
	cp -a ${OUTPUT_DIR}/lib/libCHIP.a ${OUTPUT_DIR}/libchip.a

clean:
	echo "RM $(OUTPUT_DIR)"
	rm -rf $(OUTPUT_DIR)
