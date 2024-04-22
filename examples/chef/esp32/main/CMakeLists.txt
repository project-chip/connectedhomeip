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

get_filename_component(CHIP_ROOT ../third_party/connectedhomeip REALPATH)
get_filename_component(CHEF ${CMAKE_CURRENT_SOURCE_DIR}/../../ REALPATH)

# include(${CMAKE_CURRENT_LIST_DIR}/../../project_include.cmake)

# message(STATUS "Product ID " ${CONFIG_DEVICE_PRODUCT_ID})
# message(STATUS "Vendor ID " ${CONFIG_DEVICE_VENDOR_ID})

set(CHIP_SHELL_DIR "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/examples/shell")

set(PRIV_INCLUDE_DIRS_LIST
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/zzz_generated/"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/zzz_generated/app-common"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/examples/platform/esp32/"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/third_party/nlio/repo/include"
                      "${CMAKE_SOURCE_DIR}/../"
                      "${CMAKE_SOURCE_DIR}/../common"
                      "${CMAKE_SOURCE_DIR}/../common/clusters"
                      "${CMAKE_SOURCE_DIR}/main/include/"
)


# find_package(Python3 REQUIRED)
set(ZAP_FILE ${CMAKE_CURRENT_LIST_DIR}/../../devices/${SAMPLE_NAME}.zap)
execute_process(
  COMMAND ${Python3_EXECUTABLE} ${CHIP_ROOT}/src/app/zap_cluster_list.py --zap_file ${ZAP_FILE}
  OUTPUT_VARIABLE CLUSTER_LIST
  ERROR_VARIABLE ERROR_MESSAGE
  RESULT_VARIABLE RC
)
# if (NOT RC EQUAL 0)
#   message(FATAL_ERROR "Failed to execute zap_cluster_list.py: ${ERROR_MESSAGE}")
# endif()

message(STATUS "** Start of Cluster List **")
string(REPLACE "\n" ";" CLUSTER_LIST "${CLUSTER_LIST}")
foreach(CLUSTER ${CLUSTER_LIST})
  set(SRC_DIRS_LIST  "${SRC_DIRS_LIST}"
                     "${CHIP_ROOT}/src/app/clusters/${CLUSTER}")
  message(STATUS "src/app/clusters/${CLUSTER}")
endforeach()
message(STATUS "** End of Cluster List **")

set(SRC_DIRS_LIST
                      ${SRC_DIRS_LIST}
                      "${CMAKE_CURRENT_LIST_DIR}"
                      "${CMAKE_SOURCE_DIR}/../common"
                      "${CMAKE_SOURCE_DIR}/../common/clusters/audio-output/"
                      "${CMAKE_SOURCE_DIR}/../common/clusters/channel/"
                      "${CMAKE_SOURCE_DIR}/../common/clusters/door-lock/"
                      "${CMAKE_SOURCE_DIR}/../common/clusters/keypad-input/"
                      "${CMAKE_SOURCE_DIR}/../common/clusters/low-power/"
                      "${CMAKE_SOURCE_DIR}/../common/clusters/media-input/"
                      "${CMAKE_SOURCE_DIR}/../common/clusters/media-playback/"
                      "${CMAKE_SOURCE_DIR}/../common/clusters/target-navigator/"
                      "${CMAKE_SOURCE_DIR}/../common/clusters/wake-on-lan/"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/zzz_generated/app-common/app-common/zap-generated/attributes"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/src/app/server"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/src/app/util"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/src/app/reporting"
)

if (CONFIG_ENABLE_CHIP_SHELL)
set(SRC_DIRS_LIST
                      ${SRC_DIRS_LIST}
                      "${CHIP_SHELL_DIR}/shell_common/"
)
endif()


if (CONFIG_ENABLE_PW_RPC)
# Append additional directories for RPC build
set(PRIV_INCLUDE_DIRS_LIST  "${PRIV_INCLUDE_DIRS_LIST}"
                     "${CMAKE_SOURCE_DIR}/../../platform/esp32/pw_sys_io/public"
                     "${CMAKE_SOURCE_DIR}/../../common"
                     "${CMAKE_SOURCE_DIR}/../../common/pigweed"
                     "${CMAKE_SOURCE_DIR}/../../common/pigweed/esp32"
                     "${CMAKE_SOURCE_DIR}/../../common/screen-framework/include/"
                     "${CMAKE_SOURCE_DIR}/../../../src/lib/support"
)

if (${IDF_VERSION_MAJOR} LESS 5)
    list(APPEND PRIV_INCLUDE_DIRS_LIST "${IDF_PATH}/components/freertos/include/freertos")
else()
    list(APPEND PRIV_INCLUDE_DIRS_LIST "${IDF_PATH}/components/freertos/FreeRTOS-Kernel/include/freertos")
endif()

set(SRC_DIRS_LIST  "${SRC_DIRS_LIST}"
                     "${CMAKE_SOURCE_DIR}/../../platform/esp32"
                     "${CMAKE_SOURCE_DIR}/../../common/pigweed"
                     "${CMAKE_SOURCE_DIR}/../../common/pigweed/esp32"
)
else()
set(SRC_DIRS_LIST
                      ${SRC_DIRS_LIST}
                      "${CHIP_SHELL_DIR}/shell_common/"
)
endif (CONFIG_ENABLE_PW_RPC)

idf_component_register(PRIV_INCLUDE_DIRS
                      "${CHIP_SHELL_DIR}/shell_common/include"
                      "${PRIV_INCLUDE_DIRS_LIST}"
                      PRIV_REQUIRES chip nvs_flash bt console mbedtls QRCode tft screen-framework spidriver
                      SRC_DIRS ${SRC_DIRS_LIST})

include("${CHIP_ROOT}/build/chip/esp32/esp32_codegen.cmake")
chip_app_component_codegen("${CHEF}/devices/${SAMPLE_NAME}.matter")
chip_app_component_zapgen("${CHEF}/devices/${SAMPLE_NAME}.zap")

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

pw_proto_library(boolean_state_service
  SOURCES
    ${CHIP_ROOT}/examples/common/pigweed/protos/boolean_state_service.proto
  PREFIX
    boolean_state_service
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
  boolean_state_service.nanopb_rpc
  button_service.nanopb_rpc
  descriptor_service.nanopb_rpc
  device_service.nanopb_rpc
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
  wifi_service.nanopb_rpc
)

target_link_options(${COMPONENT_LIB}
  PUBLIC
    "-T${PIGWEED_ROOT}/pw_tokenizer/pw_tokenizer_linker_sections.ld"
    "-L${PIGWEED_ROOT}/pw_tokenizer"
)

target_compile_options(${COMPONENT_LIB} PRIVATE
                       "-DPW_RPC_ATTRIBUTE_SERVICE=1"
                       "-DPW_RPC_BOOLEAN_STATE_SERVICE=1"
                       "-DPW_RPC_BUTTON_SERVICE=1"
                       "-DPW_RPC_DEVICE_SERVICE=1"
                       "-DPW_RPC_DESCRIPTOR_SERVICE=1"
                       "-DPW_RPC_WIFI_SERVICE=1"
                       "-DPW_RPC_TRACING_SERVICE=1")

endif (CONFIG_ENABLE_PW_RPC)
