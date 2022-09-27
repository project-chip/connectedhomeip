#
#    Copyright (c) 2021 Project CHIP Authors
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

# (Uses default behaviour of compiling all source files in directory, adding 'include' to include path.)
# The list of src and include dirs must be in sync with that in all-clusters-app/esp32/main/component.mk
set(PRIV_INCLUDE_DIRS_LIST
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/zzz_generated/all-clusters-app"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/examples/all-clusters-app/all-clusters-common/include"
                      "${CMAKE_CURRENT_LIST_DIR}/include"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/examples/lock-app/linux/include"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/examples/providers"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/examples/platform/esp32"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/third_party/nlfaultinjection/repo/include"                      
)
set(SRC_DIRS_LIST
                      "${CMAKE_CURRENT_LIST_DIR}"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/zzz_generated/all-clusters-app/zap-generated"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/zzz_generated/app-common/app-common/zap-generated/attributes"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/zzz_generated/app-common/app-common/zap-generated"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/examples/lock-app/linux/src"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/examples/providers"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/examples/platform/esp32/route_hook"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/examples/platform/esp32/ota"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/examples/platform/esp32/common"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/examples/platform/esp32/shell_extension"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/src/app/server"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/src/app/util"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/src/app/reporting"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/src/app/clusters/administrator-commissioning-server"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/src/app/clusters/application-basic-server"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/src/app/clusters/test-cluster-server"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/src/app/clusters/descriptor"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/src/app/clusters/fault-injection-server"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/src/app/clusters/fan-control-server"                      
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/src/app/clusters/fixed-label-server"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/src/app/clusters/user-label-server"                           
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/src/app/clusters/on-off-server"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/src/app/clusters/level-control"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/src/app/clusters/identify-server"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/src/app/clusters/access-control-server"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/src/app/clusters/account-login-server"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/src/app/clusters/application-launcher-server"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/src/app/clusters/audio-output-server"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/src/app/clusters/barrier-control-server"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/src/app/clusters/window-covering-server"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/src/app/clusters/general-commissioning-server"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/src/app/clusters/network-commissioning"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/src/app/clusters/groups-server"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/src/app/clusters/color-control-server"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/src/app/clusters/content-launch-server"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/src/app/clusters/operational-credentials-server"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/src/app/clusters/media-input-server"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/src/app/clusters/mode-select-server"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/src/app/clusters/low-power-server"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/src/app/clusters/keypad-input-server"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/src/app/clusters/media-playback-server"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/src/app/clusters/ota-requestor"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/src/app/clusters/target-navigator-server"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/src/app/clusters/thermostat-server"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/src/app/clusters/thermostat-user-interface-configuration-server"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/src/app/clusters/thread-network-diagnostics-server" 
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/src/app/clusters/channel-server"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/src/app/clusters/scenes"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/src/app/clusters/software-diagnostics-server"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/src/app/clusters/switch-server"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/src/app/clusters/general-diagnostics-server"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/src/app/clusters/group-key-mgmt-server"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/src/app/clusters/basic"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/src/app/clusters/bindings"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/src/app/clusters/diagnostic-logs-server"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/src/app/clusters/door-lock-server"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/src/app/clusters/occupancy-sensor-server"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/src/app/clusters/ethernet-network-diagnostics-server"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/src/app/clusters/localization-configuration-server"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/src/app/clusters/time-format-localization-server"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/src/app/clusters/wifi-network-diagnostics-server"                      
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/src/app/clusters/wake-on-lan-server"                      
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/src/app/clusters/pump-configuration-and-control-server"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/src/app/clusters/power-source-configuration-server"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/src/app/clusters/power-source-server"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/examples/all-clusters-app/all-clusters-common/src"
)

set(EXCLUDE_SRCS_LIST
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/examples/lock-app/linux/src/LockAppCommandDelegate.cpp"
)

if (CONFIG_ENABLE_PW_RPC)
# Append additional directories for RPC build
set(PRIV_INCLUDE_DIRS_LIST  "${PRIV_INCLUDE_DIRS_LIST}"
                     "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/examples/platform/esp32/pw_sys_io/public"
                     "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/examples/common"
                     "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/examples/common/pigweed"
                     "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/examples/common/pigweed/esp32"
                     "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/src/lib/support"
                     "${IDF_PATH}/components/freertos/include/freertos"
)
set(SRC_DIRS_LIST  "${SRC_DIRS_LIST}"
                     "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/examples/platform/esp32"
                     "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/examples/common/pigweed"
                     "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/examples/common/pigweed/esp32"
)
endif (CONFIG_ENABLE_PW_RPC)

if(("${CONFIG_DEVICE_TYPE_ESP32_DEVKITC}" STREQUAL "y") OR ("${CONFIG_DEVICE_TYPE_ESP32_C3_DEVKITM}" STREQUAL "y"))
    list(APPEND PRIV_INCLUDE_DIRS_LIST
                "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/examples/common/screen-framework/include")
    set(PRIV_REQUIRES_LIST chip QRCode bt)
elseif(("${CONFIG_DEVICE_TYPE_M5STACK}" STREQUAL "y") OR ("${CONFIG_DEVICE_TYPE_ESP32_WROVER_KIT}" STREQUAL "y"))
    set(PRIV_REQUIRES_LIST chip QRCode bt tft spidrier screen-framework)
endif()

if("${CONFIG_DEVICE_TYPE_ESP32_C3_DEVKITM}" STREQUAL "y")
    list(APPEND PRIV_REQUIRES_LIST led_strip)
endif()

if (CONFIG_OPENTHREAD_ENABLED)
    list(APPEND PRIV_REQUIRES_LIST openthread)
endif()

idf_component_register(PRIV_INCLUDE_DIRS ${PRIV_INCLUDE_DIRS_LIST}
                       SRC_DIRS ${SRC_DIRS_LIST}
                       EXCLUDE_SRCS ${EXCLUDE_SRCS_LIST}
                       PRIV_REQUIRES ${PRIV_REQUIRES_LIST})

set_property(TARGET ${COMPONENT_LIB} PROPERTY CXX_STANDARD 17)
target_compile_options(${COMPONENT_LIB} PRIVATE "-DCHIP_HAVE_CONFIG_H")
target_compile_options(${COMPONENT_LIB} PUBLIC
           "-DCHIP_ADDRESS_RESOLVE_IMPL_INCLUDE_HEADER=<lib/address_resolve/AddressResolve_DefaultImpl.h>"
)

if (CONFIG_ENABLE_PW_RPC)

get_filename_component(CHIP_ROOT ${CMAKE_SOURCE_DIR}/third_party/connectedhomeip REALPATH)

set(PIGWEED_ROOT "${CHIP_ROOT}/third_party/pigweed/repo")
include(${PIGWEED_ROOT}/pw_build/pigweed.cmake)
include(${PIGWEED_ROOT}/pw_protobuf_compiler/proto.cmake)
set(dir_pw_third_party_nanopb "${CHIP_ROOT}/third_party/nanopb/repo" CACHE STRING "" FORCE)

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
  PREFIX
    lighting_service
  STRIP_PREFIX
    ${CHIP_ROOT}/examples/common/pigweed/protos
  DEPS
    pw_protobuf.common_proto
)

pw_proto_library(locking_service
  SOURCES
    ${CHIP_ROOT}/examples/common/pigweed/protos/locking_service.proto
  PREFIX
    locking_service
  STRIP_PREFIX
    ${CHIP_ROOT}/examples/common/pigweed/protos
  DEPS
    pw_protobuf.common_proto
)

pw_proto_library(wifi_service
  SOURCES
    ${CHIP_ROOT}/examples/common/pigweed/protos/wifi_service.proto
  INPUTS
    ${CHIP_ROOT}/examples/common/pigweed/protos/wifi_service.options
  PREFIX
    wifi_service
  DEPS
    pw_protobuf.common_proto
  STRIP_PREFIX
    ${CHIP_ROOT}/examples/common/pigweed/protos
)

target_link_libraries(${COMPONENT_LIB} PUBLIC
  attributes_service.nanopb_rpc
  button_service.nanopb_rpc
  descriptor_service.nanopb_rpc
  device_service.nanopb_rpc
  lighting_service.nanopb_rpc
  locking_service.nanopb_rpc
  wifi_service.nanopb_rpc
  pw_checksum
  pw_hdlc
  pw_log
  pw_rpc.server
  pw_trace_tokenized
  pw_trace_tokenized.trace_buffer
  pw_trace_tokenized.rpc_service
  pw_trace_tokenized.protos.nanopb_rpc
)

target_link_options(${COMPONENT_LIB}
  PUBLIC
    "-T${PIGWEED_ROOT}/pw_tokenizer/pw_tokenizer_linker_sections.ld"
)

target_compile_options(${COMPONENT_LIB} PRIVATE
                       "-DPW_RPC_ATTRIBUTE_SERVICE=1"
                       "-DPW_RPC_BUTTON_SERVICE=1"
                       "-DPW_RPC_DESCRIPTOR_SERVICE=1"
                       "-DPW_RPC_DEVICE_SERVICE=1"
                       "-DPW_RPC_LIGHTING_SERVICE=1"
                       "-DPW_RPC_LOCKING_SERVICE=1"
                       "-DPW_RPC_TRACING_SERVICE=1"
                       "-DPW_RPC_WIFI_SERVICE=1"
                       "-DPW_TRACE_BACKEND_SET=1")

endif (CONFIG_ENABLE_PW_RPC)
