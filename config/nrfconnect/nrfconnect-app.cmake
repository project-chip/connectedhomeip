#
#   Copyright (c) 2020 Project CHIP Authors
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
#         CMake module for incorporating CHIP into an nRF Connect SDK
#         application for Nordic platforms.
#

#
#   The top-level CMakeLists.txt of an application should define CHIP_ROOT variable,
#   include this module and configure `app` build target properly. E.g.:
#
#   cmake_minimum_required(VERSION 3.13.1)
#
#   set(CHIP_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/third_party/connectedhomeip)
#   get_filename_component(CHIP_ROOT ${CHIP_ROOT} REALPATH)
#   set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CHIP_ROOT}/config/nrfconnect/)
#
#   include(nrfconnect-app)
#
#   project(chip-nrf52840-lock-example)
#   target_sources(app PRIVATE main/main.cpp ...)
#

# Set DTC overlay before finding the Zephyr package.
if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/dts.overlay")
    set(DTC_OVERLAY_FILE "${CMAKE_CURRENT_SOURCE_DIR}/dts.overlay")
endif()

# ==================================================
# Load NCS/Zephyr build system
# ==================================================
find_package(Zephyr HINTS $ENV{ZEPHYR_BASE})

# ==================================================
# General settings
# ==================================================

include(chip-lib)

set(CHIP_COMMON_FLAGS
    -D_SYS__PTHREADTYPES_H_
    -isystem${ZEPHYR_BASE}/include/posix
    -isystem${ZEPHYR_BASE}/../modules/crypto/mbedtls/configs
)

set(CHIP_OUTPUT_LIBRARIES
    ${CHIP_OUTPUT_DIR}/lib/libCHIP.a
    ${CHIP_OUTPUT_DIR}/lib/libInetLayer.a
    ${CHIP_OUTPUT_DIR}/lib/libSystemLayer.a
    ${CHIP_OUTPUT_DIR}/lib/libSupportLayer.a
    ${CHIP_OUTPUT_DIR}/lib/libBleLayer.a
    ${CHIP_OUTPUT_DIR}/lib/libDeviceLayer.a
    ${CHIP_OUTPUT_DIR}/lib/libCHIPDataModel.a
)

# ==================================================
# Setup CHIP build
# ==================================================

chip_configure(ChipConfig 
    ARCH arm-none-eabi
    CFLAGS ${CHIP_COMMON_FLAGS} --specs=nosys.specs
    CXXFLAGS ${CHIP_COMMON_FLAGS}
)

chip_build(ChipLib ChipConfig
    BUILD_COMMAND make --no-print-directory install V=${CMAKE_AUTOGEN_VERBOSE}
    BUILD_ARTIFACTS ${CHIP_OUTPUT_LIBRARIES}
)

# ==================================================
# Configure application
# ==================================================

target_compile_definitions(app PRIVATE HAVE_CONFIG_H)
target_link_libraries(app PUBLIC -Wl,--start-group ChipLib -Wl,--end-group)
