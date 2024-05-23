#
#   Copyright (c) 2022 Project CHIP Authors
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
#     CMake sub-project to configure and build the chip library.
#

include(ExternalProject)

# ==============================================================================
# Prepare CHIP configuration based on the project configuration
# ==============================================================================
# Set paths
if (NOT CHIP_ROOT)
    get_filename_component(CHIP_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../.. REALPATH)
endif()
get_filename_component(GN_ROOT_TARGET ${CHIP_ROOT}/config/openiotsdk/chip-gn REALPATH)
get_filename_component(COMMON_CMAKE_SOURCE_DIR ${CHIP_ROOT}/config/common/cmake REALPATH)

# Get common Cmake sources
include(${COMMON_CMAKE_SOURCE_DIR}/util.cmake)
include(${COMMON_CMAKE_SOURCE_DIR}/chip_gn_args.cmake)
include(${COMMON_CMAKE_SOURCE_DIR}/chip_gn.cmake)

# Prepare compiler flags
matter_add_cflags(${CMAKE_C_FLAGS})
matter_add_cxxflags(${CMAKE_CXX_FLAGS})

matter_get_compiler_flags_from_targets("${CONFIG_CHIP_EXTERNAL_TARGETS}")

# ==============================================================================
# Generate configuration for CHIP GN build system
# ==============================================================================
matter_common_gn_args(
    DEBUG           CONFIG_CHIP_DEBUG
    LIB_SHELL       CONFIG_CHIP_LIB_SHELL
    LIB_TESTS       CONFIG_CHIP_LIB_TESTS
    PROJECT_CONFIG  ${CONFIG_CHIP_PROJECT_CONFIG}
    PROJECT_CONFIG_INC_DIR ${CONFIG_CHIP_PROJECT_CONFIG_INCLUDE_DIRS}
)
matter_add_gn_arg_string("target_cpu"                           "${CMAKE_SYSTEM_PROCESSOR}")
matter_add_gn_arg_string("openiotsdk_ar"                        ${CMAKE_AR})
matter_add_gn_arg_string("openiotsdk_cc"                        ${CMAKE_C_COMPILER})
matter_add_gn_arg_string("openiotsdk_cxx"                       ${CMAKE_CXX_COMPILER})
matter_add_gn_arg_bool  ("chip_monolithic_tests"                CONFIG_CHIP_LIB_TESTS)
matter_add_gn_arg_bool  ("chip_detail_logging"                  CONFIG_CHIP_DETAIL_LOGGING)
matter_add_gn_arg_bool  ("chip_progress_logging"                CONFIG_CHIP_PROGRESS_LOGGING)
matter_add_gn_arg_bool  ("chip_automation_logging"              CONFIG_CHIP_AUTOMATION_LOGGING)
matter_add_gn_arg_bool  ("chip_error_logging"                   CONFIG_CHIP_ERROR_LOGGING)
matter_add_gn_arg_string("chip_crypto"                          "${CONFIG_CHIP_CRYPTO}")
matter_add_gn_arg_string("chip_openiotsdk_software_version"           "${CONFIG_CHIP_OPEN_IOT_SDK_SOFTWARE_VERSION}")
matter_add_gn_arg_string("chip_openiotsdk_software_version_string"    "${CONFIG_CHIP_OPEN_IOT_SDK_SOFTWARE_VERSION_STRING}")
matter_add_gn_arg_bool  ("chip_enable_ota_requestor"            CONFIG_CHIP_OPEN_IOT_SDK_OTA_ENABLE)
if (TARGET cmsis-rtos-api)
    matter_add_gn_arg_string("target_os"                        "cmsis-rtos")
endif()
matter_add_gn_arg_string("optimize_debug_level"                 "s")

matter_generate_args_tmp_file()

# ==============================================================================
# Build chip library
# ==============================================================================
matter_build(chip
    LIB_SHELL   ${CONFIG_CHIP_LIB_SHELL}
    LIB_TESTS   ${CONFIG_CHIP_LIB_TESTS}
    GN_DEPENDENCIES ${CONFIG_GN_DEPENDENCIES}
)

target_link_libraries(chip INTERFACE
    ${CONFIG_CHIP_EXTERNAL_TARGETS}
)
