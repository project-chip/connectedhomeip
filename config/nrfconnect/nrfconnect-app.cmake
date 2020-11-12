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

# Set Kconfig root
if (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/Kconfig)
    set(KCONFIG_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/Kconfig)
else()
    set(KCONFIG_ROOT ${CHIP_ROOT}/config/nrfconnect/Kconfig)
endif()

# ==================================================
# Load NCS/Zephyr build system
# ==================================================
find_package(Zephyr HINTS $ENV{ZEPHYR_BASE})

# TODO: temporary fix forcing time_t size to be equal long size - remove it after merging fix from Zephyr project.
zephyr_compile_definitions(_USE_LONG_TIME_T)

# ==================================================
# General settings
# ==================================================

include(chip-lib)

set(CHIP_COMMON_FLAGS
    -D_SYS__PTHREADTYPES_H_
    -DMBEDTLS_CONFIG_FILE=<nrf-config.h>
    -DMBEDTLS_PK_WRITE_C
    -DMBEDTLS_X509_CREATE_C
    -DMBEDTLS_X509_CSR_WRITE_C
    -isystem${ZEPHYR_BASE}/include/posix
    -isystem${ZEPHYR_BASE}/../mbedtls/include
    -I${CMAKE_CURRENT_SOURCE_DIR}
    -I${CMAKE_CURRENT_SOURCE_DIR}/main/include
)

set(CHIP_LIBRARIES ${CHIP_OUTPUT_DIR}/lib/libCHIP.a)
if (CONFIG_CHIP_LIB_SHELL)
    list(APPEND CHIP_LIBRARIES ${CHIP_OUTPUT_DIR}/obj/third_party/connectedhomeip/src/lib/shell/lib/libCHIPShell.a)
endif()
if (CONFIG_CHIP_PW_RPC)
    list(APPEND CHIP_LIBRARIES ${CHIP_OUTPUT_DIR}/lib/libPwRpc.a)
endif()

find_file(CHIP_PROJECT_CONFIG 
    CHIPProjectConfig.h
    PATHS ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/main/include
    NO_DEFAULT_PATH
)

# ==================================================
# Configure Zephyr
# ==================================================

function(set_openthread_config OT_DIR CONFIG_FILE)
    get_property(DEFINES DIRECTORY ${OT_DIR} PROPERTY COMPILE_DEFINITIONS)
    get_property(SUBDIRS DIRECTORY ${OT_DIR} PROPERTY SUBDIRECTORIES)

    list(TRANSFORM DEFINES REPLACE 
         OPENTHREAD_PROJECT_CORE_CONFIG_FILE=.*
         OPENTHREAD_PROJECT_CORE_CONFIG_FILE="${CONFIG_FILE}")

    set_property(DIRECTORY ${OT_DIR} PROPERTY COMPILE_DEFINITIONS ${DEFINES})

    foreach(SUBDIR ${SUBDIRS})
        set_openthread_config(${SUBDIR} ${CONFIG_FILE})
    endforeach()
endfunction()

# Add dependency on nRF crypto library.
# This is temporary solution and should be removed after fixing it in nRF Connect SDK.
zephyr_include_directories(${ZEPHYR_BASE}/../nrfxlib/crypto/nrf_cc310_platform/include)

# Override Zephyr-supplied OpenThread configuration
if (CHIP_OPENTHREAD_CONFIG)
    get_filename_component(CONFIG_DIR ${CHIP_OPENTHREAD_CONFIG} DIRECTORY)
    get_filename_component(CONFIG_FILE ${CHIP_OPENTHREAD_CONFIG} NAME)
    target_include_directories(ot-config INTERFACE ${CONFIG_DIR})
    set_openthread_config(${ZEPHYR_BASE}/../modules/lib/openthread ${CONFIG_FILE})
endif()

# ==================================================
# Setup CHIP build
# ==================================================

chip_configure(ChipConfig
    ARCH arm-none-eabi
    CFLAGS ${CHIP_COMMON_FLAGS} --specs=nosys.specs
    CXXFLAGS ${CHIP_COMMON_FLAGS}
    PROJECT_CONFIG ${CHIP_PROJECT_CONFIG}
)

chip_build(ChipLib ChipConfig
    BUILD_COMMAND ninja
    BUILD_ARTIFACTS ${CHIP_LIBRARIES}
)

# ==================================================
# Configure application
# ==================================================

target_link_libraries(app PUBLIC ChipLib)
target_compile_definitions(app PRIVATE CHIP_HAVE_CONFIG_H)
