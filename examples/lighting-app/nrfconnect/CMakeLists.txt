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
get_filename_component(NLIO_ROOT ${CHIP_ROOT}/third_party/nlio/repo/include REALPATH)
get_filename_component(NRFCONNECT_COMMON ${CHIP_ROOT}/examples/platform/nrfconnect REALPATH)
get_filename_component(LIGHTING_COMMON ${CHIP_ROOT}/examples/lighting-app/lighting-common REALPATH)

include(${CHIP_ROOT}/config/nrfconnect/app/check-nrfconnect-version.cmake)

# Load NCS/Zephyr build system
set(CONF_FILE ${CHIP_ROOT}/config/nrfconnect/app/sample-defaults.conf prj.conf)

if (EXISTS boards/${BOARD}.conf)
  list(APPEND CONF_FILE boards/${BOARD}.conf)
endif()

# TODO: temporary fix to remove after solving static addressing problem on nrf5340
if(BOARD STREQUAL "nrf5340dk_nrf5340_cpuapp")
    list(INSERT OVERLAY_CONFIG 0 ${CHIP_ROOT}/config/nrfconnect/app/overlay-bt_private_addresses.conf)
endif()

list(APPEND ZEPHYR_EXTRA_MODULES ${CHIP_ROOT}/config/nrfconnect/chip-module)
find_package(Zephyr HINTS $ENV{ZEPHYR_BASE})

project(chip-nrfconnect-lighting-example)

include(${CHIP_ROOT}/config/nrfconnect/app/enable-gnu-std.cmake)

target_compile_options(app PRIVATE -Werror)

target_include_directories(app PRIVATE
                           main/include
                           ${LIGHTING_COMMON}
                           ${NLIO_ROOT}
                           ${NRFCONNECT_COMMON}/util/include
                           ${NRFCONNECT_COMMON}/app/include)

target_sources(app PRIVATE
               main/AppTask.cpp
               main/LightingManager.cpp
               main/main.cpp
               main/ZclCallbacks.cpp
               ${LIGHTING_COMMON}/gen/attribute-size.cpp
               ${LIGHTING_COMMON}/gen/callback-stub.cpp
               ${LIGHTING_COMMON}/gen/IMClusterCommandHandler.cpp
               ${NRFCONNECT_COMMON}/util/LEDWidget.cpp
               ${NRFCONNECT_COMMON}/util/ThreadUtil.cpp
               ${CHIP_ROOT}/src/app/util/DataModelHandler.cpp
               ${CHIP_ROOT}/src/app/reporting/reporting-default-configuration.cpp
               ${CHIP_ROOT}/src/app/reporting/reporting.cpp
               ${CHIP_ROOT}/src/app/util/af-event.cpp
               ${CHIP_ROOT}/src/app/util/af-main-common.cpp
               ${CHIP_ROOT}/src/app/util/attribute-list-byte-span.cpp
               ${CHIP_ROOT}/src/app/util/attribute-size-util.cpp
               ${CHIP_ROOT}/src/app/util/attribute-storage.cpp
               ${CHIP_ROOT}/src/app/util/attribute-table.cpp
               ${CHIP_ROOT}/src/app/util/binding-table.cpp
               ${CHIP_ROOT}/src/app/util/chip-message-send.cpp
               ${CHIP_ROOT}/src/app/util/client-api.cpp
               ${CHIP_ROOT}/src/app/util/ember-compatibility-functions.cpp
               ${CHIP_ROOT}/src/app/util/ember-print.cpp
               ${CHIP_ROOT}/src/app/util/error-mapping.cpp
               ${CHIP_ROOT}/src/app/util/message.cpp
               ${CHIP_ROOT}/src/app/util/process-cluster-message.cpp
               ${CHIP_ROOT}/src/app/util/process-global-message.cpp
               ${CHIP_ROOT}/src/app/util/util.cpp
               ${CHIP_ROOT}/src/app/server/EchoHandler.cpp
               ${CHIP_ROOT}/src/app/server/Mdns.cpp
               ${CHIP_ROOT}/src/app/server/OnboardingCodesUtil.cpp
               ${CHIP_ROOT}/src/app/server/RendezvousServer.cpp
               ${CHIP_ROOT}/src/app/server/Server.cpp
               ${CHIP_ROOT}/src/app/server/StorablePeerConnection.cpp
               ${CHIP_ROOT}/src/app/clusters/basic/basic.cpp
               ${CHIP_ROOT}/src/app/clusters/bindings/bindings.cpp
               ${CHIP_ROOT}/src/app/clusters/diagnostic-logs-server/diagnostic-logs-server.cpp
               ${CHIP_ROOT}/src/app/clusters/ethernet_network_diagnostics_server/ethernet_network_diagnostics_server.cpp
               ${CHIP_ROOT}/src/app/clusters/general-commissioning-server/general-commissioning-server.cpp               
               ${CHIP_ROOT}/src/app/clusters/on-off-server/on-off-server.cpp
               ${CHIP_ROOT}/src/app/clusters/operational-credentials-server/operational-credentials-server.cpp
               ${CHIP_ROOT}/src/app/clusters/level-control/level-control.cpp
               ${CHIP_ROOT}/src/app/clusters/color-control-server/color-control-server.cpp
               ${CHIP_ROOT}/src/app/clusters/occupancy-sensor-server/occupancy-sensor-server.cpp
               ${CHIP_ROOT}/src/app/clusters/network-commissioning/network-commissioning-ember.cpp
               ${CHIP_ROOT}/src/app/clusters/network-commissioning/network-commissioning.cpp
               )

if (BOARD STREQUAL "nrf52840dongle_nrf52840")
  target_sources(app PRIVATE ${NRFCONNECT_COMMON}/util/DFUTrigger.cpp)
endif()

if (CONFIG_CHIP_PW_RPC)

set(PIGWEED_ROOT "${CHIP_ROOT}/third_party/pigweed/repo")
include(${PIGWEED_ROOT}/pw_build/pigweed.cmake)
include(${PIGWEED_ROOT}/pw_protobuf_compiler/proto.cmake)

pw_set_backend(pw_log pw_log_basic)
pw_set_backend(pw_assert pw_assert_log)
pw_set_backend(pw_sys_io pw_sys_io.nrfconnect)
set(dir_pw_third_party_nanopb "${CHIP_ROOT}/third_party/nanopb/repo" CACHE STRING "" FORCE)

add_subdirectory(third_party/connectedhomeip/examples/platform/nrfconnect/pw_sys_io)
add_subdirectory(third_party/connectedhomeip/third_party/nanopb/repo)
add_subdirectory(third_party/connectedhomeip/third_party/pigweed/repo)

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

pw_proto_library(lighting_service
  SOURCES
    ${CHIP_ROOT}/examples/lighting-app/lighting-common/lighting_service/lighting_service.proto
  STRIP_PREFIX
    ${CHIP_ROOT}/examples/lighting-app/lighting-common/
  DEPS
    pw_protobuf.common_protos
)

target_sources(app PRIVATE
  main/Rpc.cpp
  ../../common/pigweed/RpcService.cpp
  ../../common/pigweed/nrfconnect/PigweedLoggerMutex.cpp
  ${NRFCONNECT_COMMON}/util/PigweedLogger.cpp
)

target_include_directories(app PRIVATE
  ${PIGWEED_ROOT}/pw_sys_io/public
  ${CHIP_ROOT}/src/lib/support
  ${CHIP_ROOT}/src/system
  ../../common
  ../../common/pigweed/nrfconnect)

target_link_libraries(app PRIVATE
  button_service.nanopb_rpc
  device_service.nanopb_rpc
  lighting_service.nanopb_rpc
  pw_checksum
  pw_hdlc
  pw_log
  pw_rpc.server
)

target_link_libraries(pw_build INTERFACE zephyr_interface)

endif(CONFIG_CHIP_PW_RPC)

include(${CHIP_ROOT}/config/nrfconnect/app/flashing.cmake)
