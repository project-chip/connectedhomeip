#
#   Copyright (c) 2025 Project CHIP Authors
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

# =============================================================================
# CMake Setup
# =============================================================================

cmake_minimum_required(VERSION 3.15)

# =============================================================================
# Path Setup (must precede Python3_EXECUTABLE so it resolves to an absolute path)
# =============================================================================

get_filename_component(CHIP_ROOT        "${CHIP_ROOT}"             REALPATH)
get_filename_component(EXAMPLE_DIR      "${CHIP_ROOT}/examples/lighting-app/bouffalolab" REALPATH)
get_filename_component(EXAMPLES_PLAT_DIR "${CHIP_ROOT}/examples/platform/bouffalolab"    REALPATH)
set(GN_ROOT_TARGET      ${CMAKE_CURRENT_LIST_DIR})

set(Python3_EXECUTABLE "${CHIP_ROOT}/.environment/pigweed-venv/bin/python3"
    CACHE FILEPATH "Python3 executable" FORCE)

if(CONFIG_MFD STREQUAL "y")
  get_filename_component(_bl_sdk_base "${BL_SDK_BASE}" REALPATH)
  set(_matter_board_dir "${CMAKE_BINARY_DIR}/matter_board")
  set(_sdk_board_config_dir "${_bl_sdk_base}/bsp/board/${BOARD}/config")
  file(MAKE_DIRECTORY "${_matter_board_dir}/config")
  file(COPY "${_sdk_board_config_dir}/" DESTINATION "${_matter_board_dir}/config")
  file(COPY "${EXAMPLES_PLAT_DIR}/bflb/flash_config/partition_cfg_4M.toml"
       DESTINATION "${_matter_board_dir}/config")
endif()

# =============================================================================
# SDK Setup
# =============================================================================

find_package(bouffalo_sdk REQUIRED HINTS $ENV{BL_SDK_BASE} ${BL_SDK_BASE})
include(${EXAMPLES_PLAT_DIR}/bflb/bflb_common.cmake)

# Use the Matter partition table when factory data is enabled so the SDK
# post-processing step emits a partition.bin with the MFD partition.
if(CONFIG_MFD STREQUAL "y")
  set_property(GLOBAL PROPERTY FW_TOOL_CUSTOM_BOARD_DIR "${_matter_board_dir}")
endif()

set_property(TARGET app PROPERTY CXX_STANDARD 17)
set_property(TARGET app PROPERTY CXX_STANDARD_REQUIRED ON)

# Fix SDK C++11 standard and add custom compile options
get_target_property(_sdk_iface_opts sdk_intf_lib INTERFACE_COMPILE_OPTIONS)
if(_sdk_iface_opts)
  list(FILTER _sdk_iface_opts EXCLUDE REGEX "-std=c\\+\\+11")
  set_target_properties(sdk_intf_lib PROPERTIES INTERFACE_COMPILE_OPTIONS "${_sdk_iface_opts}")
endif()
sdk_add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-fno-use-cxa-atexit>)

# Fix SDK options that conflict with the Matter application.
get_target_property(_sdk_link_opts sdk_intf_lib INTERFACE_LINK_OPTIONS)
list(REMOVE_ITEM _sdk_link_opts -nostartfiles)
set_target_properties(sdk_intf_lib PROPERTIES INTERFACE_LINK_OPTIONS "${_sdk_link_opts}")

# =============================================================================
# Matter CMake Setup
# =============================================================================

include(ExternalProject)
include(${CHIP_ROOT}/config/common/cmake/chip_gn.cmake)
include(${CHIP_ROOT}/config/common/cmake/chip_gn_args.cmake)
include(${CHIP_ROOT}/src/app/chip_data_model.cmake)

# =============================================================================
# SDK Properties -> MATTER_CFLAGS
# =============================================================================

# Extract SDK properties
get_target_property(_sdk_opts sdk_intf_lib INTERFACE_COMPILE_OPTIONS)
get_target_property(_sdk_defs sdk_intf_lib INTERFACE_COMPILE_DEFINITIONS)
get_target_property(_sdk_incs sdk_intf_lib INTERFACE_INCLUDE_DIRECTORIES)

# Convert SDK options to MATTER_CFLAGS (filter generator expressions)
if(_sdk_opts)
  foreach(_flag ${_sdk_opts})
    if(_flag MATCHES "\\$<COMPILE_LANGUAGE:C>:(.+)>")
      list(APPEND MATTER_CFLAGS_C "${CMAKE_MATCH_1}")
    elseif(_flag MATCHES "\\$<COMPILE_LANGUAGE:CXX>:(.+)>")
      list(APPEND MATTER_CFLAGS_CC "${CMAKE_MATCH_1}")
    elseif(NOT _flag MATCHES "\\$<")
      list(APPEND MATTER_CFLAGS "${_flag}")
    endif()
  endforeach()
endif()

if(_sdk_defs)
  foreach(_def ${_sdk_defs})
    if(NOT _def MATCHES "\\$<")
      list(APPEND MATTER_CFLAGS "-D${_def}")
    endif()
  endforeach()
endif()

if(_sdk_incs)
  foreach(_inc ${_sdk_incs})
    if(NOT _inc MATCHES "\\$<")
      get_filename_component(_abs "${_inc}" ABSOLUTE)
      list(APPEND MATTER_CFLAGS "-I${_abs}")
    endif()
  endforeach()
endif()

# Filter out warning flags and std flags (handled elsewhere)
list(FILTER MATTER_CFLAGS    EXCLUDE REGEX "^-W[^n]|^-Wno-format")
list(FILTER MATTER_CFLAGS_C  EXCLUDE REGEX "^-W[^n]|^-Wno-format")
list(FILTER MATTER_CFLAGS_CC EXCLUDE REGEX "^-W[^n]|^-Wno-format")
list(FILTER MATTER_CFLAGS    EXCLUDE REGEX "^-std=")
list(FILTER MATTER_CFLAGS_C  EXCLUDE REGEX "^-std=")
list(FILTER MATTER_CFLAGS_CC EXCLUDE REGEX "^-std=")

list(FILTER MATTER_CFLAGS    EXCLUDE REGEX "^-Wno-unknown-warning-option$")
list(FILTER MATTER_CFLAGS_C  EXCLUDE REGEX "^-Wno-unknown-warning-option$")
list(FILTER MATTER_CFLAGS_CC EXCLUDE REGEX "^-Wno-unknown-warning-option$")

# =============================================================================
# Matter Flags
# =============================================================================

matter_add_flags(-DCONFIG_ARCH_RISCV)
matter_add_flags(-DOTA_AUTO_REBOOT_DELAY=5)
matter_add_flags(-Wno-conversion)
matter_add_flags(-Wno-maybe-uninitialized)
matter_add_flags(-Wno-shadow)
matter_add_flags(-Wno-sign-compare)

list(APPEND MATTER_CFLAGS_C
  -I${CHIP_ROOT}/src/platform/bouffalolab/BFLB)
list(APPEND MATTER_CFLAGS_CC
  -I${CHIP_ROOT}/src/platform/bouffalolab/BFLB)
list(APPEND MATTER_CFLAGS_C
  -I${CHIP_ROOT}/src/platform/bouffalolab/common)
list(APPEND MATTER_CFLAGS_CC
  -I${CHIP_ROOT}/src/platform/bouffalolab/common)

list(APPEND MATTER_CFLAGS_C  -I${EXAMPLE_DIR}/bflb)
list(APPEND MATTER_CFLAGS_CC -I${EXAMPLE_DIR}/bflb)

list(APPEND MATTER_CFLAGS_C  -include ${CMAKE_BINARY_DIR}/generated/autoconf.h)
list(APPEND MATTER_CFLAGS_CC -include ${CMAKE_BINARY_DIR}/generated/autoconf.h)

list(APPEND MATTER_CFLAGS_C  -UMBEDTLS_NET_C)
list(APPEND MATTER_CFLAGS_CC -UMBEDTLS_NET_C)

# =============================================================================
# Network Interface Configuration
# =============================================================================

if(NOT (CONFIG_WIFI STREQUAL "y" OR CONFIG_THREAD STREQUAL "y" OR CONFIG_ETHERNET STREQUAL "y"))
  message(FATAL_ERROR "No network interface selected. Set CONFIG_WIFI=y, CONFIG_THREAD=y, or CONFIG_ETHERNET=y.")
endif()

# Per-interface GN args and defines
if(CONFIG_WIFI STREQUAL "y")
  matter_add_gn_arg_bool("chip_enable_wifi" TRUE)
  matter_add_flags(-DCHIP_DEVICE_CONFIG_ENABLE_WIFI=1)
  list(APPEND MATTER_CFLAGS_C  -I${EXAMPLE_DIR}/bflb/wifi)
  list(APPEND MATTER_CFLAGS_CC -I${EXAMPLE_DIR}/bflb/wifi)
else()
  matter_add_gn_arg_bool("chip_enable_wifi" FALSE)
endif()

if(CONFIG_THREAD STREQUAL "y")
  matter_add_gn_arg_bool("chip_enable_thread" TRUE)
  sdk_add_compile_definitions(-DOPENTHREAD_PROJECT_CORE_CONFIG_FILE=\"openthread-core-proj-config.h\")
  sdk_add_compile_definitions(-DOPENTHREAD_CORE_CONFIG_PLATFORM_CHECK_FILE=\"openthread-core-platform-config-check.h\")
  list(APPEND MATTER_CFLAGS_C  -I${EXAMPLE_DIR}/bflb)
  list(APPEND MATTER_CFLAGS_CC -I${EXAMPLE_DIR}/bflb)
  if(CONFIG_SHELL STREQUAL "y")
    matter_add_flags(-DCHIP_DEVICE_CONFIG_THREAD_ENABLE_CLI=1)
    list(APPEND _COMMON_DEFS CHIP_DEVICE_CONFIG_THREAD_ENABLE_CLI=1)
  endif()
else()
  matter_add_gn_arg_bool("chip_enable_thread" FALSE)
endif()

if(CONFIG_ETHERNET STREQUAL "y")
  matter_add_gn_arg_bool("chip_enable_ethernet" TRUE)
  matter_add_flags(-DCHIP_DEVICE_CONFIG_ENABLE_ETHERNET=1)
  list(APPEND MATTER_CFLAGS_C  -I${EXAMPLE_DIR}/bflb/ethernet)
  list(APPEND MATTER_CFLAGS_CC -I${EXAMPLE_DIR}/bflb/ethernet)
else()
  matter_add_gn_arg_bool("chip_enable_ethernet" FALSE)
endif()

# Resolve GN arg conflicts for multi-interface combinations
if(CONFIG_THREAD STREQUAL "y")
  matter_add_gn_arg_string("chip_mdns" "platform")
  if(CONFIG_WIFI STREQUAL "y" OR CONFIG_ETHERNET STREQUAL "y")
    # Thread + WiFi/Ethernet: use lwIP for all interfaces, no OpenThread endpoints
    matter_add_gn_arg_bool("chip_with_lwip" TRUE)
    matter_add_gn_arg_bool("chip_inet_config_enable_ipv4" TRUE)
    matter_add_gn_arg_bool("chip_system_config_use_openthread_inet_endpoints" FALSE)
  else()
    # Thread-only: use OpenThread endpoints, no lwIP
    matter_add_gn_arg_bool("chip_with_lwip" FALSE)
    matter_add_gn_arg_bool("chip_inet_config_enable_ipv4" FALSE)
    matter_add_gn_arg_bool("chip_system_config_use_openthread_inet_endpoints" TRUE)
  endif()
else()
  matter_add_gn_arg_string("chip_mdns" "minimal")
endif()

if(CONFIG_ETHERNET STREQUAL "y" AND NOT CONFIG_WIFI STREQUAL "y")
  matter_add_gn_arg_bool("chip_config_network_layer_ble" FALSE)
endif()

# =============================================================================
# GN Arguments
# =============================================================================

matter_add_gn_arg_bool("chip_enable_factory_data" CONFIG_MFD STREQUAL "y")
matter_add_gn_arg_bool("chip_enable_additional_data_advertising" CONFIG_CHIP_ROTATING_DEVICE_ID STREQUAL "y")
matter_add_gn_arg_bool("chip_enable_rotating_device_id"          CONFIG_CHIP_ROTATING_DEVICE_ID STREQUAL "y")

matter_add_gn_arg_string("cross_compile" "${CROSS_COMPILE}")
matter_add_gn_arg_list("chip_project_config_include_dirs" "\"${EXAMPLE_DIR}/bflb\"")

bflb_remove_lto()

matter_common_gn_args(
  DEBUG     FALSE
  LIB_TESTS FALSE
  LIB_SHELL CONFIG_SHELL STREQUAL "y"
)

# =============================================================================
# GN Build
# =============================================================================

matter_generate_args_tmp_file()
matter_build(chip)

# =============================================================================
# SDK Include Directories
# =============================================================================

sdk_add_include_directories(
  ${CHIP_ROOT}/src/platform/bouffalolab/BFLB
  ${CHIP_ROOT}/src/platform/bouffalolab/common
  ${EXAMPLE_DIR}/bflb
  ${EXAMPLE_DIR}/common
  ${EXAMPLES_PLAT_DIR}/bflb
  ${EXAMPLES_PLAT_DIR}/common/bouffalo_sdk
  ${EXAMPLES_PLAT_DIR}/common/plat
)

if(CONFIG_ETHERNET STREQUAL "y")
  sdk_add_include_directories(${EXAMPLE_DIR}/bflb/ethernet)
endif()
if(CONFIG_WIFI STREQUAL "y")
  sdk_add_include_directories(${EXAMPLE_DIR}/bflb/wifi)
endif()

# =============================================================================
# Compile Definitions
# =============================================================================

set(_COMMON_DEFS
  APP_TASK_STACK_SIZE=4096
  ${CHIP}_ENABLE
  START_ENTRY=main
  ${CHIP}DK
  CHIP_HAVE_CONFIG_H
  OTA_PERIODIC_TIMEOUT=14400
  BOUFFALOLAB_APP_NAME="chip-lighting-example"
)
if(CONFIG_WIFI STREQUAL "y")
  list(APPEND _COMMON_DEFS CHIP_DEVICE_CONFIG_ENABLE_WIFI=1)
  list(APPEND _COMMON_DEFS CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE=1)
endif()
if(CONFIG_THREAD STREQUAL "y")
  list(APPEND _COMMON_DEFS CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE=1)
endif()
if(CONFIG_ETHERNET STREQUAL "y")
  list(APPEND _COMMON_DEFS CHIP_DEVICE_CONFIG_ENABLE_ETHERNET=1)
endif()
if(CONFIG_SHELL STREQUAL "y")
  list(APPEND _COMMON_DEFS CONFIG_ENABLE_CHIP_SHELL=1)
endif()
if(CONFIG_CHIP_HEAP_MONITOR STREQUAL "y")
  list(APPEND _COMMON_DEFS HEAP_MONITORING=1)
endif()
if(CONFIG_MFD STREQUAL "y")
  list(APPEND _COMMON_DEFS CONFIG_BOUFFALOLAB_FACTORY_DATA_ENABLE=1)
endif()

target_compile_definitions(app PRIVATE ${_COMMON_DEFS})
target_compile_options(app PRIVATE
  $<$<COMPILE_LANGUAGE:CXX>:-Wno-maybe-uninitialized>
  $<$<COMPILE_LANGUAGE:CXX>:-Wno-shadow>
  $<$<COMPILE_LANGUAGE:C>:-Wno-sign-compare>
  $<$<COMPILE_LANGUAGE:CXX>:-Wno-sign-compare>
)

# =============================================================================
# Zap / Data Model
# =============================================================================

file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/gen/app-zapgen/zapgen/app-templates/zap-generated")
set(_ZAP_FILE ${CHIP_ROOT}/examples/lighting-app/lighting-common/lighting-app.zap)
chip_configure_data_model(app
  INCLUDE_SERVER
  ZAP_FILE ${_ZAP_FILE}
)

# =============================================================================
# Target Include Directories
# =============================================================================

target_include_directories(app PRIVATE
  ${EXAMPLES_PLAT_DIR}/common/route_hook
  ${CHIP_ROOT}/examples/providers
  ${CHIP_ROOT}/zzz_generated/app-common
  ${CMAKE_CURRENT_BINARY_DIR}/gen/include
)

if(CONFIG_SHELL STREQUAL "y")
  target_include_directories(app PRIVATE
    ${CHIP_ROOT}/src/lib/shell
    ${CHIP_ROOT}/examples/shell/shell_common/include
  )
endif()

# =============================================================================
# Target Sources
# =============================================================================

target_sources(app PRIVATE
  ${EXAMPLE_DIR}/common/AppTask.cpp
  ${EXAMPLE_DIR}/common/ZclCallbacks.cpp
  ${EXAMPLES_PLAT_DIR}/common/bouffalo_sdk/demo_pwm.c
  ${EXAMPLES_PLAT_DIR}/common/bouffalo_sdk/platform_port.cpp
  ${EXAMPLES_PLAT_DIR}/common/plat/LEDWidget.cpp
  ${EXAMPLES_PLAT_DIR}/common/plat/OTAConfig.cpp
  ${EXAMPLES_PLAT_DIR}/common/plat/platform.cpp
  ${CHIP_ROOT}/examples/providers/DeviceInfoProviderImpl.cpp
)

if(CONFIG_WIFI STREQUAL "y" OR CONFIG_ETHERNET STREQUAL "y")
  target_sources(app PRIVATE
    ${EXAMPLES_PLAT_DIR}/common/route_hook/bflb_route_hook.c
    ${EXAMPLES_PLAT_DIR}/common/route_hook/bflb_route_table.c
  )
endif()

if(CONFIG_CHIP_HEAP_MONITOR STREQUAL "y")
  target_sources(app PRIVATE
    ${EXAMPLES_PLAT_DIR}/common/plat/MemMonitoring.cpp
  )
endif()

if(CONFIG_SHELL STREQUAL "y")
  target_sources(app PRIVATE
    ${CHIP_ROOT}/examples/shell/shell_common/cmd_misc.cpp
    ${CHIP_ROOT}/examples/shell/shell_common/cmd_otcli.cpp
    ${CHIP_ROOT}/examples/shell/shell_common/cmd_server.cpp
    ${CHIP_ROOT}/examples/shell/shell_common/globals.cpp
  )
endif()

# =============================================================================
# Link & Project
# =============================================================================

sdk_set_main_file(${EXAMPLES_PLAT_DIR}/common/plat/main.cpp)
sdk_add_link_libraries(chip)

set(CHIP_PROJECT_NAME "chip-bflb-lighting-example")
project(${CHIP_PROJECT_NAME} C CXX)

target_compile_options(${proj_name}.elf PRIVATE
  $<$<COMPILE_LANGUAGE:CXX>:-std=gnu++17>
)

target_compile_definitions(${proj_name}.elf PRIVATE ${_COMMON_DEFS})

add_custom_command(TARGET combine POST_BUILD
  COMMAND ${Python3_EXECUTABLE}
          ${CHIP_ROOT}/scripts/tools/bouffalolab/bouffalolab_ota_image.py
          --mode bouffalo-sdk
          --build-out-dir ${CMAKE_CURRENT_BINARY_DIR}/build_out
          --output-dir ${CMAKE_CURRENT_BINARY_DIR}/build_out
          --project-config ${EXAMPLE_DIR}/bflb/CHIPProjectConfig.h
  COMMENT "Generating Bouffalo Matter OTA image"
)
