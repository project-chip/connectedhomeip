#
#    Copyright (c) 2020 Project CHIP Authors
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
cmake_minimum_required(VERSION 3.13.1)

get_filename_component(CHIP_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/third_party/connectedhomeip REALPATH)
get_filename_component(NRFCONNECT_COMMON ${CHIP_ROOT}/examples/platform/nrfconnect REALPATH)
get_filename_component(CHEF ${CMAKE_CURRENT_SOURCE_DIR}/../ REALPATH)

include(${CHIP_ROOT}/config/nrfconnect/app/check-nrfconnect-version.cmake)
include(${CHEF}/project_include.cmake)

get_filename_component(GEN_DIR ${CHEF}/out/${SAMPLE_NAME}/zap-generated REALPATH)

set(CONF_FILE prj.conf)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CHIP_CFLAGS "${CHIP_CFLAGS} -DCHIP_DEVICE_CONFIG_DEVICE_PRODUCT_ID=${CONFIG_DEVICE_PRODUCT_ID}")
set(CHIP_CFLAGS "${CHIP_CFLAGS} -DCHIP_DEVICE_CONFIG_DEVICE_VENDOR_ID=${CONFIG_DEVICE_VENDOR_ID}")
set(CHIP_CFLAGS "${CHIP_CFLAGS} -DCHIP_PLATFORM_NRFCONNECT=1")
if(NOT ${CHIP_DEVICE_CONFIG_DEVICE_SOFTWARE_VERSION_STRING} STREQUAL "")
  set(CHIP_CFLAGS "${CHIP_CFLAGS} -DCHIP_DEVICE_CONFIG_DEVICE_SOFTWARE_VERSION_STRING=\"${CHIP_DEVICE_CONFIG_DEVICE_SOFTWARE_VERSION_STRING}\"")
endif()
message(STATUS "Product ID " ${CONFIG_DEVICE_PRODUCT_ID})
message(STATUS "Vendor ID " ${CONFIG_DEVICE_VENDOR_ID})
message(STATUS "SW Version String" ${CHIP_DEVICE_CONFIG_DEVICE_SOFTWARE_VERSION_STRING})

# Set Kconfig root files that will be processed as a first Kconfig for used child images.
set(mcuboot_KCONFIG_ROOT ${CHIP_ROOT}/config/nrfconnect/chip-module/Kconfig.mcuboot.root)
set(multiprotocol_rpmsg_KCONFIG_ROOT ${CHIP_ROOT}/config/nrfconnect/chip-module/Kconfig.multiprotocol_rpmsg.root)

list(APPEND ZEPHYR_EXTRA_MODULES ${CHIP_ROOT}/config/nrfconnect/chip-module)

if (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/boards/${BOARD}.conf)
  list(APPEND CONF_FILE boards/${BOARD}.conf)
endif()

find_package(Zephyr HINTS $ENV{ZEPHYR_BASE})

project(chip-nrfconnect-chef-example)

include(${CHIP_ROOT}/config/nrfconnect/app/enable-gnu-std.cmake)
include(${CHIP_ROOT}/src/app/chip_data_model.cmake)

# -Wmaybe-uninitialized has too many false positives, including on std::optional
# and chip::Optional.  Make it nonfatal.
#
# See https://gcc.gnu.org/bugzilla/show_bug.cgi?id=80635
target_compile_options(app PRIVATE -Werror -Wno-error=maybe-uninitialized)

target_include_directories(app PRIVATE
                           ${CMAKE_CURRENT_SOURCE_DIR}
                           ${CMAKE_CURRENT_SOURCE_DIR}/third_party/connectedhomeip/zzz_generated/app-common
                           ${GEN_DIR}
                           ${CHEF}
                           ${GEN_DIR}/../
                           ${CHIP_ROOT}/src
                           ${NRFCONNECT_COMMON}/util/include
                           ${NRFCONNECT_COMMON}/app/include
)

if (CONFIG_ENABLE_CHIP_SHELL)
  target_sources(app PRIVATE
    ${CHEF}/shell_common/globals.cpp
    ${CHEF}/shell_common/cmd_misc.cpp
    ${CHEF}/shell_common/cmd_otcli.cpp
  )

  target_include_directories(app PRIVATE
    ${CHEF}/shell_common/include
  )
endif (CONFIG_ENABLE_CHIP_SHELL)

target_sources(app PRIVATE
    ${CHEF}/nrfconnect/main.cpp
    ${CHEF}/common/stubs.cpp
    ${GEN_DIR}/callback-stub.cpp
    ${GEN_DIR}/IMClusterCommandHandler.cpp
)

message(STATUS ${CHEF}/devices/${SAMPLE_NAME}.zap)
chip_configure_data_model(app
    INCLUDE_SERVER
    ZAP_FILE ${CHEF}/devices/${SAMPLE_NAME}.zap
    GEN_DIR ${GEN_DIR}
)

include(${CHIP_ROOT}/config/nrfconnect/app/flashing.cmake)


if (CONFIG_ENABLE_PW_RPC)

target_compile_options(app PRIVATE
                       "-DCONFIG_ENABLE_PW_RPC=1"
)

# Make all targets created below depend on zephyr_interface to inherit MCU-related compilation flags
link_libraries($<BUILD_INTERFACE:zephyr_interface>)

set(PIGWEED_ROOT "${CHIP_ROOT}/third_party/pigweed/repo")
include(${PIGWEED_ROOT}/pw_build/pigweed.cmake)
include(${PIGWEED_ROOT}/pw_protobuf_compiler/proto.cmake)

pw_set_module_config(pw_rpc_CONFIG pw_rpc.disable_global_mutex_config)
pw_set_backend(pw_log pw_log_basic)
pw_set_backend(pw_assert.check pw_assert_log.check_backend)
pw_set_backend(pw_assert.assert pw_assert.assert_compatibility_backend)
pw_set_backend(pw_sys_io pw_sys_io.nrfconnect)
pw_set_backend(pw_trace pw_trace_tokenized)
set(dir_pw_third_party_nanopb "${CHIP_ROOT}/third_party/nanopb/repo" CACHE STRING "" FORCE)

add_subdirectory(third_party/connectedhomeip/examples/platform/nrfconnect/pw_sys_io)
add_subdirectory(third_party/connectedhomeip/third_party/nanopb/repo)
add_subdirectory(third_party/connectedhomeip/third_party/pigweed/repo)

pw_proto_library(attributes_service
  SOURCES
    ${CHIP_ROOT}/examples/common/pigweed/protos/attributes_service.proto
  INPUTS
    ${CHIP_ROOT}/examples/common/pigweed/protos/attributes_service.options
  PREFIX
    attributes_service
  STRIP_PREFIX
    ${CHIP_ROOT}/examples/common/pigweed/protos
  DEPS
    pw_protobuf.common_proto
)

pw_proto_library(descriptor_service
  SOURCES
    ${CHIP_ROOT}/examples/common/pigweed/protos/descriptor_service.proto
  PREFIX
    descriptor_service
  STRIP_PREFIX
    ${CHIP_ROOT}/examples/common/pigweed/protos
  DEPS
    pw_protobuf.common_proto
)

pw_proto_library(device_service
  SOURCES
    ${CHIP_ROOT}/examples/common/pigweed/protos/device_service.proto
  INPUTS
    ${CHIP_ROOT}/examples/common/pigweed/protos/device_service.options
  PREFIX
    device_service
  STRIP_PREFIX
    ${CHIP_ROOT}/examples/common/pigweed/protos
  DEPS
    pw_protobuf.common_proto
)

pw_proto_library(ot_cli_service
  SOURCES
    ${CHIP_ROOT}/examples/common/pigweed/protos/ot_cli_service.proto
  INPUTS
    ${CHIP_ROOT}/examples/common/pigweed/protos/ot_cli_service.options
  STRIP_PREFIX
    ${CHIP_ROOT}/examples/common/pigweed/protos
  PREFIX
    ot_cli_service
  DEPS
    pw_protobuf.common_proto
)

pw_proto_library(thread_service
  SOURCES
    ${CHIP_ROOT}/examples/common/pigweed/protos/thread_service.proto
  INPUTS
    ${CHIP_ROOT}/examples/common/pigweed/protos/thread_service.options
  STRIP_PREFIX
    ${CHIP_ROOT}/examples/common/pigweed/protos
  PREFIX
    thread_service
  DEPS
    pw_protobuf.common_proto
)

target_sources(app PRIVATE
  ../../common/pigweed/RpcService.cpp
  ../../common/pigweed/nrfconnect/PigweedLoggerMutex.cpp
  ${NRFCONNECT_COMMON}/Rpc.cpp
  ${NRFCONNECT_COMMON}/util/PigweedLogger.cpp
)

target_include_directories(app PRIVATE
  ${PIGWEED_ROOT}/pw_sys_io/public
  ${CHIP_ROOT}/src/lib/support
  ${CHIP_ROOT}/src/system
  ${NRFCONNECT_COMMON}
  ${NRFCONNECT_COMMON}/util/include
  ../../common
  ../../common/pigweed
  ../../common/pigweed/nrfconnect)

target_compile_options(app PRIVATE
                       "-DPW_RPC_ATTRIBUTE_SERVICE=1"
                       "-DPW_RPC_DESCRIPTOR_SERVICE=1"
                       "-DPW_RPC_DEVICE_SERVICE=1"
                       "-DPW_RPC_THREAD_SERVICE=1"
                       "-DPW_RPC_TRACING_SERVICE=1"
)

target_link_libraries(app PRIVATE
  attributes_service.nanopb_rpc
  descriptor_service.nanopb_rpc
  device_service.nanopb_rpc
  thread_service.nanopb_rpc
  pw_checksum
  pw_hdlc
  pw_log
  pw_rpc.server
  pw_trace_tokenized
  pw_trace_tokenized.trace_buffer
  pw_trace_tokenized.rpc_service
  pw_trace_tokenized.protos.nanopb_rpc
)

target_link_options(app
  PUBLIC
    "-T${PIGWEED_ROOT}/pw_tokenizer/pw_tokenizer_linker_sections.ld"
)

endif(CONFIG_ENABLE_PW_RPC)
