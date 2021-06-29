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

if (CONFIG_ENABLE_PW_RPC)
idf_component_register(INCLUDE_DIRS
                     "${CMAKE_CURRENT_LIST_DIR}"
                     "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/third_party/pigweed/repo/pw_sys_io/public"
                     "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/third_party/pigweed/repo/pw_assert/public"
                     "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/third_party/pigweed/repo/pw_assert_log/public"
                     "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/third_party/pigweed/repo/pw_assert_log/public_overrides"
                     "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/third_party/pigweed/repo/pw_bytes/public"
                     "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/third_party/pigweed/repo/pw_checksum/public"
                     "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/third_party/pigweed/repo/pw_containers/public"
                     "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/third_party/pigweed/repo/pw_hdlc/public"
                     "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/third_party/pigweed/repo/pw_log/public"
                     "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/third_party/pigweed/repo/pw_log_basic/public"
                     "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/third_party/pigweed/repo/pw_log_basic/public_overrides"
                     "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/third_party/pigweed/repo/pw_span/public_overrides"
                     "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/third_party/pigweed/repo/pw_span/public"
                     "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/third_party/pigweed/repo/pw_polyfill/public"
                     "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/third_party/pigweed/repo/pw_polyfill/standard_library_public"
                     "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/third_party/pigweed/repo/pw_polyfill/public_overrides"
                     "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/third_party/pigweed/repo/pw_rpc/public"
                     "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/third_party/pigweed/repo/pw_rpc/nanopb/public"
                     "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/third_party/pigweed/repo/pw_rpc/raw/public"
                     "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/third_party/pigweed/repo/pw_protobuf/public"
                     "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/third_party/pigweed/repo/pw_status/public"
                     "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/third_party/pigweed/repo/pw_stream/public"
                     "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/third_party/pigweed/repo/pw_result/public"
                     "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/third_party/pigweed/repo/pw_varint/public"
                     "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/third_party/pigweed/repo/pw_preprocessor/public"
                     "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/third_party/pigweed/repo/pw_rpc/system_server/public"
                     "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/third_party/nanopb/repo"
                     "${CMAKE_SOURCE_DIR}/../../platform/esp32/pw_sys_io/public"
                     "${CMAKE_SOURCE_DIR}/../../platform/esp32"
                     "${CMAKE_SOURCE_DIR}/../../common/pigweed"
                     "${CMAKE_SOURCE_DIR}/../../common/pigweed/esp32"
                     "${CMAKE_SOURCE_DIR}/../../../src/lib/support"
                     "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/src"
                     "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/examples/lock-app/lock-common"
                     "${CMAKE_CURRENT_LIST_DIR}/include"
                     "${IDF_PATH}/components/freertos/include/freertos"
                      SRC_DIRS
                     "${CMAKE_CURRENT_LIST_DIR}"
                     "${CMAKE_SOURCE_DIR}/../../platform/esp32"
                     "${CMAKE_SOURCE_DIR}/../../common/pigweed"
                     "${CMAKE_SOURCE_DIR}/../../common/pigweed/esp32"
                     "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/examples/lock-app/lock-common/gen"
                     "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/src/app/server"
                     "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/src/app/util"
                     "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/src/app/reporting"
                     "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/src/app/clusters/basic"
                     "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/src/app/clusters/bindings"
                     "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/src/app/clusters/network-commissioning"
                     "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/src/app/clusters/on-off-server"
                     "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/src/app/clusters/operational-credentials-server"
                     "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/src/app/clusters/general-commissioning-server"
                     PRIV_REQUIRES bt chip QRCode tft spidriver screen-framework)

idf_component_get_property(chip_lib chip COMPONENT_LIB)

set(WRAP_FUNCTIONS esp_log_write)
target_link_libraries(${chip_lib} INTERFACE "-Wl,--wrap=${WRAP_FUNCTIONS}")

get_filename_component(CHIP_ROOT ../third_party/connectedhomeip REALPATH)

set(PIGWEED_ROOT "${CHIP_ROOT}/third_party/pigweed/repo")
include(${PIGWEED_ROOT}/pw_build/pigweed.cmake)
include(${PIGWEED_ROOT}/pw_protobuf_compiler/proto.cmake)
set(dir_pw_third_party_nanopb "${CHIP_ROOT}/third_party/nanopb/repo" CACHE STRING "" FORCE)

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
    pw_protobuf.common_protos
)

pw_proto_library(button_service
  SOURCES
    ${CHIP_ROOT}/examples/common/pigweed/protos/button_service.proto
  PREFIX
    button_service
  STRIP_PREFIX
    ${CHIP_ROOT}/examples/common/pigweed/protos
  DEPS
    pw_protobuf.common_protos
)

pw_proto_library(locking_service
  SOURCES
    ${CHIP_ROOT}/examples/lock-app/esp32/main/locking_service.proto
  PREFIX
    locking_service
  STRIP_PREFIX
    ${CHIP_ROOT}/examples/lock-app/esp32/main
  DEPS
    pw_protobuf.common_protos
)

target_link_libraries(${COMPONENT_LIB} PUBLIC
  device_service.nanopb_rpc
  button_service.nanopb_rpc
  locking_service.nanopb_rpc
)

set_property(TARGET ${chip_lib} APPEND PROPERTY LINK_LIBRARIES ${COMPONENT_LIB})
target_include_directories(${chip_lib} PUBLIC
                          "$<TARGET_FILE_DIR:${chip_lib}>/protocol_buffer/gen/third_party/connectedhomeip/third_party/pigweed/repo/pw_rpc/protos.proto_library/nanopb_rpc"
                          "$<TARGET_FILE_DIR:${chip_lib}>/protocol_buffer/gen/third_party/connectedhomeip/third_party/pigweed/repo/pw_rpc/protos.proto_library/nanopb"
                          "$<TARGET_FILE_DIR:${chip_lib}>/protocol_buffer/gen/third_party/connectedhomeip/third_party/pigweed/repo/pw_rpc/protos.proto_library/pwpb")

else (CONFIG_ENABLE_PW_RPC)
idf_component_register(PRIV_INCLUDE_DIRS
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/src"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/examples/lock-app/lock-common"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/examples/platform/esp32"
                      "${CMAKE_CURRENT_LIST_DIR}/include"
                      SRC_DIRS
                      "${CMAKE_CURRENT_LIST_DIR}"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/examples/lock-app/lock-common/gen"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/examples/platform/esp32/shell_extension"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/src/app/server"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/src/app/util"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/src/app/reporting"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/src/app/clusters/basic"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/src/app/clusters/bindings"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/src/app/clusters/diagnostic-logs-server"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/src/app/clusters/network-commissioning"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/src/app/clusters/on-off-server"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/src/app/clusters/operational-credentials-server"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/src/app/clusters/general-commissioning-server"
                      PRIV_REQUIRES chip QRCode tft spidriver bt screen-framework)

set_property(TARGET ${COMPONENT_LIB} PROPERTY CXX_STANDARD 17)
target_compile_options(${COMPONENT_LIB} PRIVATE "-DLWIP_IPV6_SCOPES=0" "-DCHIP_HAVE_CONFIG_H")

endif (CONFIG_ENABLE_PW_RPC)
