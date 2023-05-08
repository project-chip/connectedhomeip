#
#   Copyright (c) 2022-2023 Project CHIP Authors
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
#     CMake sub-project defining 'chip' target which represents CHIP library
#     Since CHIP doesn't provide native CMake support, ExternalProject
#     module is used to build the required artifacts with GN meta-build
#     system. It is assumed that find_package(Zephyr) has been called before
#     including this file.
#

if (CONFIG_CHIP)

include(ExternalProject)
include(../zephyr-util.cmake)

#
# ==============================================================================
# Prepare CHIP configuration based on the project Kconfig configuration
# ==============================================================================
# Set paths
if (NOT CHIP_ROOT)
    get_filename_component(CHIP_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../../.. REALPATH)
endif()
get_filename_component(GN_ROOT_TARGET ${CHIP_ROOT}/config/zephyr/chip-gn REALPATH)
get_filename_component(COMMON_CMAKE_SOURCE_DIR ${CHIP_ROOT}/config/common/cmake REALPATH)

# Additional configuration
if (CONFIG_CHIP_PW_RPC)
    set(CONFIG_CHIP_LIB_PW_RPC YES)
endif()

# Get common Cmake sources
include(${COMMON_CMAKE_SOURCE_DIR}/chip_gn_args.cmake)
include(${COMMON_CMAKE_SOURCE_DIR}/chip_gn.cmake)

# Prepare compiler flags
matter_add_flags(-isystem${ZEPHYR_BASE}/../modules/crypto/mbedtls/include/)

if (CONFIG_POSIX_API)
    matter_add_flags(-D_SYS__PTHREADTYPES_H_)
    matter_add_flags(-isystem${ZEPHYR_BASE}/include/zephyr/posix)
endif()

zephyr_get_compile_flags(ZEPHYR_CFLAGS_C C)
matter_add_cflags(${ZEPHYR_CFLAGS_C})
zephyr_get_compile_flags(ZEPHYR_CFLAGS_CC CXX)
matter_add_cxxflags(${ZEPHYR_CFLAGS_CC})
zephyr_get_gnu_cpp_standard(ZEPHYR_GNU_CPP_STD)
matter_add_cxxflags(${ZEPHYR_GNU_CPP_STD})

# Set up custom OpenThread configuration

if (CONFIG_CHIP_OPENTHREAD_CONFIG)
    get_filename_component(CHIP_OPENTHREAD_CONFIG
        ${CONFIG_CHIP_OPENTHREAD_CONFIG}
        REALPATH
        BASE_DIR ${CMAKE_SOURCE_DIR}
    )
    zephyr_set_openthread_config(${CHIP_OPENTHREAD_CONFIG})
endif()

# ==============================================================================
# Generate configuration for CHIP GN build system
# ==============================================================================
matter_common_gn_args(
    DEBUG           CONFIG_DEBUG
    LIB_SHELL       CONFIG_CHIP_LIB_SHELL
    LIB_TESTS       CONFIG_CHIP_BUILD_TESTS
    LIB_PW_RPC      CONFIG_CHIP_PW_RPC
    PROJECT_CONFIG  ${CONFIG_CHIP_PROJECT_CONFIG}
)
matter_add_gn_arg_string("zephyr_ar"                              ${CMAKE_AR})
matter_add_gn_arg_string("zephyr_cc"                              ${CMAKE_C_COMPILER})
matter_add_gn_arg_string("zephyr_cxx"                             ${CMAKE_CXX_COMPILER})
matter_add_gn_arg_bool  ("chip_enable_openthread"                 CONFIG_NET_L2_OPENTHREAD)
matter_add_gn_arg_bool  ("chip_openthread_ftd"                    CONFIG_OPENTHREAD_FTD)
matter_add_gn_arg_bool  ("chip_inet_config_enable_ipv4"           CONFIG_NET_IPV4)
matter_add_gn_arg_bool  ("chip_inet_config_enable_tcp_endpoint"   CONFIG_CHIP_BUILD_TESTS)

if (CONFIG_CHIP_ENABLE_DNSSD_SRP)
    matter_add_gn_arg_string("chip_mdns" "platform")
endif()

matter_generate_args_tmp_file()

# ==============================================================================
# Build chip library
# ==============================================================================
matter_build(chip
    LIB_SHELL   ${CONFIG_CHIP_LIB_SHELL}
    LIB_PW_RPC  ${CONFIG_CHIP_PW_RPC}
    LIB_TESTS   ${CONFIG_CHIP_BUILD_TESTS}
    GN_DEPENDENCIES kernel
)
set_property(GLOBAL APPEND PROPERTY ZEPHYR_INTERFACE_LIBS chip)

endif() # CONFIG_CHIP