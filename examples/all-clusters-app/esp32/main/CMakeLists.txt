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

get_filename_component(CHIP_ROOT ${CMAKE_SOURCE_DIR}/third_party/connectedhomeip REALPATH)
get_filename_component(ALL_CLUSTERS_COMMON_DIR ${CHIP_ROOT}/examples/all-clusters-app/all-clusters-common REALPATH)
get_filename_component(ENERGY_MANAGEMENT_COMMON_DIR ${CHIP_ROOT}/examples/energy-management-app/energy-management-common/ REALPATH)
get_filename_component(APP_COMMON_GEN_DIR ${CHIP_ROOT}/zzz_generated/app-common/app-common/zap-generated REALPATH)

set(PRIV_INCLUDE_DIRS_LIST "${ALL_CLUSTERS_COMMON_DIR}/include"
                           "${ENERGY_MANAGEMENT_COMMON_DIR}/common/include"
                           "${ENERGY_MANAGEMENT_COMMON_DIR}/device-energy-management/include"
                           "${ENERGY_MANAGEMENT_COMMON_DIR}/energy-evse/include"
                           "${ENERGY_MANAGEMENT_COMMON_DIR}/energy-reporting/include"
                           "${ENERGY_MANAGEMENT_COMMON_DIR}/water-heater/include"
                           "${CMAKE_CURRENT_LIST_DIR}/include"
                           "${CHIP_ROOT}/examples/providers"
                           "${CHIP_ROOT}/examples/platform/esp32"
                           "${CHIP_ROOT}/third_party/nlfaultinjection/include"
)
set(SRC_DIRS_LIST     "${CMAKE_CURRENT_LIST_DIR}"
                      "${APP_COMMON_GEN_DIR}/attributes"
                      "${APP_COMMON_GEN_DIR}"
                      "${ALL_CLUSTERS_COMMON_DIR}/src"
                      "${ENERGY_MANAGEMENT_COMMON_DIR}/common/src"
                      "${ENERGY_MANAGEMENT_COMMON_DIR}/device-energy-management/src"
                      "${ENERGY_MANAGEMENT_COMMON_DIR}/energy-evse/src"
                      "${ENERGY_MANAGEMENT_COMMON_DIR}/energy-reporting/src"
                      "${ENERGY_MANAGEMENT_COMMON_DIR}/water-heater/src"
                      "${CHIP_ROOT}/examples/providers"
                      "${CHIP_ROOT}/examples/platform/esp32/ota"
                      "${CHIP_ROOT}/examples/platform/esp32/common"
                      "${CHIP_ROOT}/examples/platform/esp32/shell_extension"
)


if (CONFIG_ENABLE_PW_RPC)
# Append additional directories for RPC build
set(PRIV_INCLUDE_DIRS_LIST  "${PRIV_INCLUDE_DIRS_LIST}"
                     "${CHIP_ROOT}/examples/platform/esp32/pw_sys_io/public"
                     "${CHIP_ROOT}/examples/common"
                     "${CHIP_ROOT}/examples/common/pigweed"
                     "${CHIP_ROOT}/examples/common/pigweed/esp32"
                     "${CHIP_ROOT}/src/lib/support"
)

if (${IDF_VERSION_MAJOR} LESS 5)
    list(APPEND PRIV_INCLUDE_DIRS_LIST "${IDF_PATH}/components/freertos/include/freertos")
else()
    list(APPEND PRIV_INCLUDE_DIRS_LIST "${IDF_PATH}/components/freertos/FreeRTOS-Kernel/include/freertos")
endif()

set(SRC_DIRS_LIST  "${SRC_DIRS_LIST}"
                     "${CHIP_ROOT}/examples/platform/esp32"
                     "${CHIP_ROOT}/examples/common/pigweed"
                     "${CHIP_ROOT}/examples/common/pigweed/esp32"
)
endif (CONFIG_ENABLE_PW_RPC)

if (CONFIG_ENABLE_ICD_SERVER)
    list(APPEND PRIV_INCLUDE_DIRS_LIST "${CHIP_ROOT}/examples/platform/esp32/icd")
    list(APPEND SRC_DIRS_LIST "${CHIP_ROOT}/examples/platform/esp32/icd")
endif()

idf_component_register(PRIV_INCLUDE_DIRS ${PRIV_INCLUDE_DIRS_LIST}
                       SRC_DIRS ${SRC_DIRS_LIST})

include(${CHIP_ROOT}/src/app/chip_data_model.cmake)
chip_configure_data_model(${COMPONENT_LIB}
    ZAP_FILE ${ALL_CLUSTERS_COMMON_DIR}/all-clusters-app.zap
)

set_property(TARGET ${COMPONENT_LIB} PROPERTY CXX_STANDARD 17)
target_compile_options(${COMPONENT_LIB} PRIVATE "-DCHIP_HAVE_CONFIG_H")
target_compile_options(${COMPONENT_LIB} PUBLIC
           "-DCHIP_ADDRESS_RESOLVE_IMPL_INCLUDE_HEADER=<lib/address_resolve/AddressResolve_DefaultImpl.h>"
)

if (CONFIG_ENABLE_PW_RPC)


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
