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

idf_component_register(INCLUDE_DIRS 
                     "${CMAKE_CURRENT_LIST_DIR}"
                     "${CMAKE_SOURCE_DIR}/../../platform/esp32/pw_sys_io/public"
                     "${CMAKE_SOURCE_DIR}/../../platform/esp32"
                     "${CMAKE_SOURCE_DIR}/../../common/pigweed"
                     "${CMAKE_SOURCE_DIR}/../../common/pigweed/esp32"
                     "${CMAKE_SOURCE_DIR}/../../../src/lib/support"
                     "${IDF_PATH}/components/freertos/include/freertos"
                    
                      SRC_DIRS
                     "${CMAKE_CURRENT_LIST_DIR}"
                     "${CMAKE_SOURCE_DIR}/../../platform/esp32"
                     "${CMAKE_SOURCE_DIR}/../../common/pigweed"
                     "${CMAKE_SOURCE_DIR}/../../common/pigweed/esp32"
                      PRIV_REQUIRES bt chip)

idf_component_get_property(chip_lib chip COMPONENT_LIB)

get_filename_component(CHIP_ROOT ../third_party/connectedhomeip REALPATH)
set(PIGWEED_ROOT "${CHIP_ROOT}/third_party/pigweed/repo")

include(${PIGWEED_ROOT}/pw_build/pigweed.cmake)
include(${PIGWEED_ROOT}/pw_protobuf_compiler/proto.cmake)
set(dir_pw_third_party_nanopb "${CHIP_ROOT}/third_party/nanopb/repo" CACHE STRING "" FORCE)

target_link_libraries(${COMPONENT_LIB} PUBLIC
    pw_checksum
    pw_hdlc
    pw_log
    pw_rpc.nanopb.echo_service
    pw_rpc.server
)

set(WRAP_FUNCTIONS esp_log_write)
target_link_libraries(${chip_lib} INTERFACE "-Wl,--wrap=${WRAP_FUNCTIONS}")

set_property(TARGET ${chip_lib} APPEND PROPERTY LINK_LIBRARIES ${COMPONENT_LIB})
target_include_directories(${chip_lib} PUBLIC
                          "$<TARGET_FILE_DIR:${chip_lib}>/protocol_buffer/gen/third_party/connectedhomeip/third_party/pigweed/repo/pw_rpc/protos.proto_library/nanopb_rpc"
                          "$<TARGET_FILE_DIR:${chip_lib}>/protocol_buffer/gen/third_party/connectedhomeip/third_party/pigweed/repo/pw_rpc/protos.proto_library/nanopb"
                          "$<TARGET_FILE_DIR:${chip_lib}>/protocol_buffer/gen/third_party/connectedhomeip/third_party/pigweed/repo/pw_rpc/protos.proto_library/pwpb")
