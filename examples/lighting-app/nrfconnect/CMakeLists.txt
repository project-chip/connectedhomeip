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
get_filename_component(NRFCONNECT_COMMON ${CHIP_ROOT}/examples/platform/nrfconnect REALPATH)
get_filename_component(LIGHTING_COMMON ${CHIP_ROOT}/examples/lighting-app/lighting-common REALPATH)

set(CONF_FILE ${CHIP_ROOT}/config/nrfconnect/app/sample-defaults.conf prj.conf)

include(${CHIP_ROOT}/config/nrfconnect/app/app.cmake)

project(chip-nrf52840-lighting-example)

target_include_directories(app PRIVATE 
                           main/include 
                           ${LIGHTING_COMMON} 
                           ${NRFCONNECT_COMMON}/util/include 
                           ${NRFCONNECT_COMMON}/app/include)

target_sources(app PRIVATE
               main/AppTask.cpp
               main/LightingManager.cpp
               main/main.cpp
               main/ZclCallbacks.cpp
               ${LIGHTING_COMMON}/gen/call-command-handler.cpp
               ${LIGHTING_COMMON}/gen/callback-stub.cpp
               ${NRFCONNECT_COMMON}/util/LEDWidget.cpp
               ${NRFCONNECT_COMMON}/util/NFCWidget.cpp
               ${NRFCONNECT_COMMON}/util/ThreadUtil.cpp
               ${NRFCONNECT_COMMON}/app/Service.cpp
               ${CHIP_ROOT}/src/app/server/DataModelHandler.cpp
               ${CHIP_ROOT}/src/app/reporting/reporting-default-configuration.cpp
               ${CHIP_ROOT}/src/app/reporting/reporting.cpp
               ${CHIP_ROOT}/src/app/util/af-event.cpp
               ${CHIP_ROOT}/src/app/util/af-main-common.cpp
               ${CHIP_ROOT}/src/app/util/attribute-size.cpp
               ${CHIP_ROOT}/src/app/util/attribute-storage.cpp
               ${CHIP_ROOT}/src/app/util/attribute-table.cpp
               ${CHIP_ROOT}/src/app/util/binding-table.cpp
               ${CHIP_ROOT}/src/app/util/chip-message-send.cpp
               ${CHIP_ROOT}/src/app/util/client-api.cpp
               ${CHIP_ROOT}/src/app/util/ember-print.cpp
               ${CHIP_ROOT}/src/app/util/message.cpp
               ${CHIP_ROOT}/src/app/util/process-cluster-message.cpp
               ${CHIP_ROOT}/src/app/util/process-global-message.cpp
               ${CHIP_ROOT}/src/app/util/util.cpp
               ${CHIP_ROOT}/src/app/clusters/bindings/bindings.cpp
               ${CHIP_ROOT}/src/app/clusters/on-off-server/on-off.cpp
               ${CHIP_ROOT}/src/app/clusters/level-control/level-control.cpp
               )

if (CONFIG_CHIP_PW_RPC)

set(PIGWEED_ROOT "${CHIP_ROOT}/third_party/pigweed/repo")
include(${PIGWEED_ROOT}/pw_build/pigweed.cmake)
include(${PIGWEED_ROOT}/pw_protobuf_compiler/proto.cmake)

pw_set_backend(pw_log pw_log_basic)
pw_set_backend(pw_assert pw_assert_log)
pw_set_backend(pw_sys_io pw_sys_io.nrfconnect)
set(dir_pw_third_party_nanopb PRESENT CACHE STRING ${CHIP_ROOT}/third_party/nanopb/repo FORCE)

add_subdirectory(third_party/connectedhomeip/examples/platform/nrfconnect/pw_sys_io)
add_subdirectory(third_party/connectedhomeip/third_party/nanopb/repo)
add_subdirectory(third_party/connectedhomeip/third_party/pigweed/repo)

pw_proto_library(pigweed_lighting_protolib
  SOURCES
    main/pigweed_lighting.proto
)

target_sources(app PRIVATE
  main/Rpc.cpp
  ${NRFCONNECT_COMMON}/util/PigweedLogger.cpp
)

target_include_directories(app PRIVATE
  ${ZEPHYR_BASE}/subsys // required to get access to `log_output_std.h`
)

target_link_libraries(app PRIVATE
  pigweed_lighting_protolib.nanopb_rpc
  pw_hdlc_lite
  pw_log
  pw_rpc.server
)

endif(CONFIG_CHIP_PW_RPC)
