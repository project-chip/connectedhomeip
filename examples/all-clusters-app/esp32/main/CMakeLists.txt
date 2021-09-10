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
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/zzz_generated/app-common"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/examples/all-clusters-app/all-clusters-common"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/examples/platform/esp32/"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/third_party/nlio/repo/include"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/src"
                      "${CMAKE_CURRENT_LIST_DIR}/include"
)
set(SRC_DIRS_LIST
                      "${CMAKE_CURRENT_LIST_DIR}"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/zzz_generated/all-clusters-app/zap-generated"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/zzz_generated/app-common/app-common/zap-generated/attributes"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/examples/platform/esp32/shell_extension"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/src/app/server"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/src/app/util"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/src/app/clusters/administrator-commissioning-server"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/src/app/clusters/test-cluster-server"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/src/app/clusters/descriptor"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/src/app/clusters/on-off-server"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/src/app/clusters/level-control"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/src/app/clusters/identify"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/src/app/clusters/account-login-server"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/src/app/clusters/application-launcher-server"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/src/app/clusters/audio-output-server"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/src/app/clusters/barrier-control-server"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/src/app/clusters/general-commissioning-server"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/src/app/clusters/network-commissioning"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/src/app/clusters/groups-server"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/src/app/clusters/color-control-server"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/src/app/clusters/content-launch-server"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/src/app/clusters/operational-credentials-server"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/src/app/clusters/media-input-server"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/src/app/clusters/low-power-server"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/src/app/clusters/keypad-input-server"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/src/app/clusters/media-playback-server"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/src/app/clusters/ota-provider"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/src/app/clusters/binary-input-server"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/src/app/clusters/target-navigator-server"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/src/app/clusters/temperature-measurement-server"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/src/app/clusters/thread_network_diagnostics_server"		                           
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/src/app/clusters/tv-channel-server"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/src/app/clusters/scenes"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/src/app/clusters/software_diagnostics_server" 		      
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/src/app/clusters/basic"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/src/app/clusters/bindings"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/src/app/reporting"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/src/app/clusters/diagnostic-logs-server"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/src/app/clusters/door-lock-server"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/src/app/clusters/occupancy-sensor-server"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/src/app/clusters/ias-zone-server"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/src/app/clusters/relative-humidity-measurement-server"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/src/app/clusters/electrical-measurement-server"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/src/app/clusters/ethernet_network_diagnostics_server"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/src/app/clusters/wifi_network_diagnostics_server"                      
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/src/app/clusters/pump-configuration-and-control-server"
                      #${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/src/app/clusters/ias-zone-client
)

if (CONFIG_ENABLE_PW_RPC)
# Append additional directories for RPC build
set(PRIV_INCLUDE_DIRS_LIST  "${PRIV_INCLUDE_DIRS_LIST}"
                     "${CMAKE_SOURCE_DIR}/../../platform/esp32/pw_sys_io/public"
                     "${CMAKE_SOURCE_DIR}/../../common/pigweed"
                     "${CMAKE_SOURCE_DIR}/../../common/pigweed/esp32"
                     "${CMAKE_SOURCE_DIR}/../../../src/lib/support"
                     "${IDF_PATH}/components/freertos/include/freertos"
)
set(SRC_DIRS_LIST  "${SRC_DIRS_LIST}"
                     "${CMAKE_SOURCE_DIR}/../../platform/esp32"
                     "${CMAKE_SOURCE_DIR}/../../common/pigweed"
                     "${CMAKE_SOURCE_DIR}/../../common/pigweed/esp32"
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

idf_component_register(PRIV_INCLUDE_DIRS ${PRIV_INCLUDE_DIRS_LIST}
                       SRC_DIRS ${SRC_DIRS_LIST}
                       PRIV_REQUIRES ${PRIV_REQUIRES_LIST})

set_property(TARGET ${COMPONENT_LIB} PROPERTY CXX_STANDARD 17)
target_compile_options(${COMPONENT_LIB} PRIVATE "-DLWIP_IPV6_SCOPES=0" "-DCHIP_HAVE_CONFIG_H")

if (CONFIG_ENABLE_PW_RPC)

idf_component_get_property(chip_lib chip COMPONENT_LIB)

set(WRAP_FUNCTIONS esp_log_write)
target_link_libraries(${chip_lib} INTERFACE "-Wl,--wrap=${WRAP_FUNCTIONS}")

get_filename_component(CHIP_ROOT ../third_party/connectedhomeip REALPATH)

set(PIGWEED_ROOT "${CHIP_ROOT}/third_party/pigweed/repo")
include(${PIGWEED_ROOT}/pw_build/pigweed.cmake)
include(${PIGWEED_ROOT}/pw_protobuf_compiler/proto.cmake)
set(dir_pw_third_party_nanopb "${CHIP_ROOT}/third_party/nanopb/repo" CACHE STRING "" FORCE)

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

pw_proto_library(wifi_service
  SOURCES
    ${CHIP_ROOT}/examples/ipv6only-app/common/wifi_service/wifi_service.proto
  INPUTS
    ${CHIP_ROOT}/examples/ipv6only-app/common/wifi_service/wifi_service.options
  PREFIX
    wifi_service
  DEPS
    pw_protobuf.common_protos
  STRIP_PREFIX
    ${CHIP_ROOT}/examples/ipv6only-app/common/wifi_service
)

target_link_libraries(${COMPONENT_LIB} PUBLIC
  button_service.nanopb_rpc
  device_service.nanopb_rpc
  wifi_service.nanopb_rpc
  pw_checksum
  pw_hdlc
  pw_log
  pw_rpc.server
)

set_property(TARGET ${chip_lib} APPEND PROPERTY LINK_LIBRARIES ${COMPONENT_LIB})
target_include_directories(${chip_lib} PUBLIC
                          "$<TARGET_FILE_DIR:${chip_lib}>/protocol_buffer/gen/third_party/connectedhomeip/third_party/pigweed/repo/pw_rpc/protos.proto_library/nanopb_rpc"
                          "$<TARGET_FILE_DIR:${chip_lib}>/protocol_buffer/gen/third_party/connectedhomeip/third_party/pigweed/repo/pw_rpc/protos.proto_library/nanopb"
                          "$<TARGET_FILE_DIR:${chip_lib}>/protocol_buffer/gen/third_party/connectedhomeip/third_party/pigweed/repo/pw_rpc/protos.proto_library/pwpb")

endif (CONFIG_ENABLE_PW_RPC)
