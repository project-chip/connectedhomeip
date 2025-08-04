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

if (${IDF_VERSION_MAJOR} LESS 5)
    set(freertos_include_path "${IDF_PATH}/components/freertos/include/freertos")
else()
    set(freertos_include_path "${IDF_PATH}/components/freertos/FreeRTOS-Kernel/include/freertos")
endif()

idf_component_register(INCLUDE_DIRS
                     "${CMAKE_CURRENT_LIST_DIR}"
                     "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/examples/platform/esp32/pw_sys_io/public"
                     "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/examples/platform/esp32"
                     "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/examples/common/pigweed"
                     "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/examples/common/pigweed/esp32"
                     "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/src/lib/support"
                     "${freertos_include_path}"

                      SRC_DIRS
                     "${CMAKE_CURRENT_LIST_DIR}"
                     "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/examples/platform/esp32"
                     "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/examples/common/pigweed"
                     "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/examples/common/pigweed/esp32")

get_filename_component(CHIP_ROOT ${CMAKE_SOURCE_DIR}/third_party/connectedhomeip REALPATH)
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
    pw_sys_io
)
