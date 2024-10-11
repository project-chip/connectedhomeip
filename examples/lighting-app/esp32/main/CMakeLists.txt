#
#    Copyright (c) 2021-2023 Project CHIP Authors
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

set(PRIV_INCLUDE_DIRS_LIST
                      "${APP_GEN_DIR}"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/examples/providers"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/examples/lighting-app/lighting-common/include"
                      "${CMAKE_CURRENT_LIST_DIR}/include"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/examples/platform/esp32"
)
set(SRC_DIRS_LIST
                      "${CMAKE_CURRENT_LIST_DIR}"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/zzz_generated/app-common/app-common/zap-generated/attributes"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/zzz_generated/app-common/app-common/zap-generated"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/examples/providers"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/examples/lighting-app/lighting-common/src"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/examples/platform/esp32/ota"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/examples/platform/esp32/common"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/examples/platform/esp32/shell_extension"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/src/app/server"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/src/app/util"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/src/app/reporting"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/src/app/clusters/access-control-server"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/src/app/clusters/administrator-commissioning-server"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/src/app/clusters/basic-information"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/src/app/clusters/bindings"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/src/app/clusters/descriptor"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/src/app/clusters/diagnostic-logs-server"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/src/app/clusters/color-control-server"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/src/app/clusters/ethernet-network-diagnostics-server"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/src/app/clusters/fixed-label-server"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/src/app/clusters/general-commissioning-server"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/src/app/clusters/general-diagnostics-server"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/src/app/clusters/identify-server"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/src/app/clusters/localization-configuration-server"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/src/app/clusters/time-format-localization-server"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/src/app/clusters/level-control"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/src/app/clusters/network-commissioning"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/src/app/clusters/on-off-server"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/src/app/clusters/operational-credentials-server"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/src/app/clusters/ota-requestor"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/src/app/clusters/power-source-configuration-server"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/src/app/clusters/software-diagnostics-server"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/src/app/clusters/switch-server"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/src/app/clusters/thread-network-diagnostics-server"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/src/app/clusters/user-label-server"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/src/app/clusters/wifi-network-diagnostics-server"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/src/app/clusters/ota-requestor"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/src/app/clusters/groups-server"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/src/app/clusters/group-key-mgmt-server"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/src/app/clusters/scenes-server"
)

if (CONFIG_ENABLE_PW_RPC)
# Append additional directories for RPC build
set(PRIV_INCLUDE_DIRS_LIST  "${PRIV_INCLUDE_DIRS_LIST}"
                     "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/examples/platform/esp32/pw_sys_io/public"
                     "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/examples/common"
                     "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/examples/common/pigweed"
                     "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/examples/common/pigweed/esp32"
                     "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/src/lib/support"
)

if (${IDF_VERSION_MAJOR} LESS 5)
    list(APPEND PRIV_INCLUDE_DIRS_LIST "${IDF_PATH}/components/freertos/include/freertos")
else()
    list(APPEND PRIV_INCLUDE_DIRS_LIST "${IDF_PATH}/components/freertos/FreeRTOS-Kernel/include/freertos")
endif()

set(SRC_DIRS_LIST  "${SRC_DIRS_LIST}"
                     "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/examples/platform/esp32"
                     "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/examples/common/pigweed"
                     "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/examples/common/pigweed/esp32"
)
endif (CONFIG_ENABLE_PW_RPC)

idf_component_register(PRIV_INCLUDE_DIRS ${PRIV_INCLUDE_DIRS_LIST}
                       SRC_DIRS ${SRC_DIRS_LIST})
get_filename_component(CHIP_ROOT ${CMAKE_SOURCE_DIR}/third_party/connectedhomeip REALPATH)

include("${CHIP_ROOT}/build/chip/esp32/esp32_codegen.cmake")

chip_app_component_codegen("${CHIP_ROOT}/examples/lighting-app/lighting-common/lighting-app.matter")
chip_app_component_zapgen("${CHIP_ROOT}/examples/lighting-app/lighting-common/lighting-app.zap")

if (CONFIG_ENABLE_ESP_INSIGHTS_TRACE)
    target_add_binary_data(${COMPONENT_TARGET} "insights_auth_key.txt" TEXT)
endif()

if (CONFIG_ENABLE_SET_CERT_DECLARATION_API)
    target_add_binary_data(${COMPONENT_TARGET} "certification_declaration.der" BINARY)
endif()

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
  pw_sys_io
  pw_trace
  pw_trace_tokenized
  pw_trace_tokenized.trace_buffer
  pw_trace_tokenized.rpc_service
  pw_trace_tokenized.protos.nanopb_rpc
)

target_link_options(${COMPONENT_LIB}
  PUBLIC
    "-T${PIGWEED_ROOT}/pw_tokenizer/pw_tokenizer_linker_sections.ld"
    "-L${PIGWEED_ROOT}/pw_tokenizer"
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
