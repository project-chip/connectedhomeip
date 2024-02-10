#
#    Copyright (c) 2022 Project CHIP Authors
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
get_filename_component(TELINK_COMMON ${CHIP_ROOT}/examples/platform/telink REALPATH)
get_filename_component(GEN_DIR ${CHIP_ROOT}/zzz_generated/ REALPATH)

set(ignoreMe "${TLNK_USB_DONGLE}")

if(${TLNK_MARS_BOARD} MATCHES y)
  if(EXISTS "${CHIP_ROOT}/src/platform/telink/${BOARD}_mars.overlay")
    set(MARS_DTC_OVERLAY_FILE "${CHIP_ROOT}/src/platform/telink/${BOARD}_mars.overlay")
  else()
    unset(MARS_DTC_OVERLAY_FILE)
  endif()
  if(EXISTS "${CHIP_ROOT}/src/platform/telink/${BOARD}_mars.conf")
    set(MARS_CONF_OVERLAY_FILE "${CHIP_ROOT}/src/platform/telink/${BOARD}_mars.conf")
  else()
    unset(MARS_CONF_OVERLAY_FILE)
  endif()
else()
  unset(MARS_CONF_OVERLAY_FILE)
endif()

if(${TLNK_USB_DONGLE} MATCHES y)
  if(EXISTS "${CHIP_ROOT}/src/platform/telink/${BOARD}_usb.overlay")
    set(USB_DTC_OVERLAY_FILE "${CHIP_ROOT}/src/platform/telink/${BOARD}_usb.overlay")
  else()
    unset(USB_DTC_OVERLAY_FILE)
  endif()
  if(EXISTS "${CHIP_ROOT}/src/platform/telink/${BOARD}_usb.conf")
    set(USB_CONF_OVERLAY_FILE "${CHIP_ROOT}/src/platform/telink/${BOARD}_usb.conf")
  else()
    unset(USB_CONF_OVERLAY_FILE)
  endif()
else()
  unset(USB_CONF_OVERLAY_FILE)
endif()

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/boards/${BOARD}.overlay")
  set(LOCAL_DTC_OVERLAY_FILE "${CMAKE_CURRENT_SOURCE_DIR}/boards/${BOARD}.overlay")
else()
  unset(LOCAL_DTC_OVERLAY_FILE)
endif()

if(EXISTS "${CHIP_ROOT}/src/platform/telink/${BOARD}.overlay")
  set(GLOBAL_DTC_OVERLAY_FILE "${CHIP_ROOT}/src/platform/telink/${BOARD}.overlay")
else()
  unset(GLOBAL_DTC_OVERLAY_FILE)
endif()

if(EXISTS "${CHIP_ROOT}/src/platform/telink/${FLASH_SIZE}_flash.overlay")
  set(FLASH_DTC_OVERLAY_FILE "${CHIP_ROOT}/src/platform/telink/${FLASH_SIZE}_flash.overlay")
  message(STATUS "Flash memory size is set to: " ${FLASH_SIZE} "b")
else()
  set(FLASH_DTC_OVERLAY_FILE "${CHIP_ROOT}/src/platform/telink/2m_flash.overlay")
  message(STATUS "Flash memory size is set to: 2mb")
endif()

if(DTC_OVERLAY_FILE)
  set(DTC_OVERLAY_FILE
    "${DTC_OVERLAY_FILE} ${GLOBAL_DTC_OVERLAY_FILE} ${USB_DTC_OVERLAY_FILE} ${MARS_DTC_OVERLAY_FILE} ${FLASH_DTC_OVERLAY_FILE} ${LOCAL_DTC_OVERLAY_FILE}"
    CACHE STRING "" FORCE
  )
else()
  set(DTC_OVERLAY_FILE ${GLOBAL_DTC_OVERLAY_FILE} ${USB_DTC_OVERLAY_FILE} ${MARS_DTC_OVERLAY_FILE} ${FLASH_DTC_OVERLAY_FILE} ${LOCAL_DTC_OVERLAY_FILE})
endif()

set(CONF_FILE ${USB_CONF_OVERLAY_FILE} ${MARS_CONF_OVERLAY_FILE} prj.conf)

# Load NCS/Zephyr build system
list(APPEND ZEPHYR_EXTRA_MODULES ${CHIP_ROOT}/config/telink/chip-module)
find_package(Zephyr HINTS $ENV{ZEPHYR_BASE})

project(chip-telink-lighting-example)

include(${CHIP_ROOT}/config/telink/app/enable-gnu-std.cmake)
include(${CHIP_ROOT}/src/app/chip_data_model.cmake)

target_compile_options(app PRIVATE -fpermissive)

target_include_directories(app PRIVATE
                           include
                           ${GEN_DIR}/app-common
                           ${GEN_DIR}/lighting-app
                           ${TELINK_COMMON}/common/include
                           ${TELINK_COMMON}/util/include
                           ${TELINK_COMMON}/app/include)

target_sources(app PRIVATE
               src/AppTask.cpp
               src/ZclCallbacks.cpp
               ${TELINK_COMMON}/common/src/mainCommon.cpp
               ${TELINK_COMMON}/common/src/AppTaskCommon.cpp
               ${TELINK_COMMON}/util/src/LEDWidget.cpp
               ${TELINK_COMMON}/util/src/ButtonManager.cpp
               ${TELINK_COMMON}/util/src/ThreadUtil.cpp
               ${TELINK_COMMON}/util/src/PWMDevice.cpp
               ${TELINK_COMMON}/util/src/WS2812Device.cpp
               ${TELINK_COMMON}/util/src/ColorFormat.cpp)

chip_configure_data_model(app
    INCLUDE_SERVER
    ZAP_FILE ${CMAKE_CURRENT_SOURCE_DIR}/../lighting-common/lighting-app.zap
)

if(CONFIG_BOOTLOADER_MCUBOOT)
  target_sources(app PRIVATE ${TELINK_COMMON}/util/src/OTAUtil.cpp)
endif()

# Fix for unused swap parameter in: zephyr/include/zephyr/arch/riscv/irq.h:70
add_compile_options(-Wno-error=unused-parameter)

if (CONFIG_CHIP_PW_RPC)

# Make all targets created below depend on zephyr_interface to inherit MCU-related compilation flags
link_libraries($<BUILD_INTERFACE:zephyr_interface>)

set(PIGWEED_ROOT "${CHIP_ROOT}/third_party/pigweed/repo")
include(${PIGWEED_ROOT}/pw_build/pigweed.cmake)
include(${PIGWEED_ROOT}/pw_protobuf_compiler/proto.cmake)

include($ENV{PW_ROOT}/pw_assert/backend.cmake)
include($ENV{PW_ROOT}/pw_log/backend.cmake)
include($ENV{PW_ROOT}/pw_sys_io/backend.cmake)

pw_set_module_config(pw_rpc_CONFIG pw_rpc.disable_global_mutex_config)
pw_set_backend(pw_log pw_log_basic)
pw_set_backend(pw_assert.check pw_assert_log.check_backend)
pw_set_backend(pw_assert.assert pw_assert.assert_compatibility_backend)
pw_set_backend(pw_sys_io pw_sys_io.telink)

set(dir_pw_third_party_nanopb "${CHIP_ROOT}/third_party/nanopb/repo" CACHE STRING "" FORCE)

add_subdirectory(third_party/connectedhomeip/third_party/pigweed/repo)
add_subdirectory(third_party/connectedhomeip/third_party/nanopb/repo)
add_subdirectory(third_party/connectedhomeip/examples/platform/telink/pw_sys_io)

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

pw_proto_library(button_service
  SOURCES
    ${CHIP_ROOT}/examples/common/pigweed/protos/button_service.proto
  PREFIX
    button_service
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

pw_proto_library(lighting_service
  SOURCES
    ${CHIP_ROOT}/examples/common/pigweed/protos/lighting_service.proto
  STRIP_PREFIX
    ${CHIP_ROOT}/examples/common/pigweed/protos
  PREFIX
    lighting_service
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
  ../../common/pigweed/telink/PigweedLoggerMutex.cpp
  ${TELINK_COMMON}/Rpc.cpp
  ${TELINK_COMMON}/util/src/PigweedLogger.cpp
)

target_include_directories(app PRIVATE
  ${PIGWEED_ROOT}/pw_sys_io/public
  ${CHIP_ROOT}/src/lib/support
  ${CHIP_ROOT}/src/system
  ${TELINK_COMMON}
  ../../common
  ../../common/pigweed
  ../../common/pigweed/telink)

target_compile_options(app PRIVATE
                       "-DPW_RPC_ATTRIBUTE_SERVICE=1"
                       "-DPW_RPC_BUTTON_SERVICE=1"
                       "-DPW_RPC_DESCRIPTOR_SERVICE=1"
                       "-DPW_RPC_DEVICE_SERVICE=1"
                       "-DPW_RPC_LIGHTING_SERVICE=1"
                       "-DPW_RPC_THREAD_SERVICE=1")

target_link_libraries(app PRIVATE
  attributes_service.nanopb_rpc
  button_service.nanopb_rpc
  descriptor_service.nanopb_rpc
  device_service.nanopb_rpc
  lighting_service.nanopb_rpc
  thread_service.nanopb_rpc
  pw_checksum
  pw_hdlc
  pw_log
  pw_rpc.server
  pw_sys_io
)

endif(CONFIG_CHIP_PW_RPC)
