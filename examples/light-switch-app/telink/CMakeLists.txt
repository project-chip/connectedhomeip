#
#    Copyright (c) 2022 Project CHIP Authors
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

set(BOARD tlsr9518adk80d)

get_filename_component(CHIP_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/third_party/connectedhomeip REALPATH)
get_filename_component(NLIO_ROOT ${CHIP_ROOT}/third_party/nlio/repo/include REALPATH)
get_filename_component(TELINK_COMMON ${CHIP_ROOT}/examples/platform/telink REALPATH)
get_filename_component(GEN_DIR ${CHIP_ROOT}/zzz_generated/ REALPATH)

set(CONF_FILE ${CHIP_ROOT}/config/telink/app/zephyr.conf prj.conf)

# Load NCS/Zephyr build system
list(APPEND ZEPHYR_EXTRA_MODULES ${CHIP_ROOT}/config/telink/chip-module)
find_package(Zephyr HINTS $ENV{ZEPHYR_BASE})

project(chip-telink-light-switch-example)

include(${CHIP_ROOT}/config/telink/app/enable-gnu-std.cmake)

target_compile_options(app PRIVATE -fpermissive)

target_include_directories(app PRIVATE
                           include
                           ${GEN_DIR}/app-common
                           ${GEN_DIR}/light-switch-app
                           ${NLIO_ROOT}
                           ${TELINK_COMMON}/util/include
                           ${TELINK_COMMON}/app/include)

add_definitions(
    "-DCHIP_ADDRESS_RESOLVE_IMPL_INCLUDE_HEADER=<lib/address_resolve/AddressResolve_DefaultImpl.h>"
)

# TODO - re-use chip_data_model.cmake to add cluster implementations.
target_sources(app PRIVATE
               src/AppTask.cpp
               src/main.cpp
               src/ZclCallbacks.cpp
               src/binding-handler.cpp
               ${GEN_DIR}/light-switch-app/zap-generated/callback-stub.cpp
               ${GEN_DIR}/light-switch-app/zap-generated/IMClusterCommandHandler.cpp
               ${TELINK_COMMON}/util/src/LEDWidget.cpp
               ${TELINK_COMMON}/util/src/ButtonManager.cpp
               ${TELINK_COMMON}/util/src/ThreadUtil.cpp
               ${CHIP_ROOT}/zzz_generated/app-common/app-common/zap-generated/attributes/Accessors.cpp
               ${CHIP_ROOT}/zzz_generated/app-common/app-common/zap-generated/cluster-objects.cpp
               ${CHIP_ROOT}/src/app/util/DataModelHandler.cpp
               ${CHIP_ROOT}/src/app/util/af-event.cpp
               ${CHIP_ROOT}/src/app/util/attribute-size-util.cpp
               ${CHIP_ROOT}/src/app/util/attribute-storage.cpp
               ${CHIP_ROOT}/src/app/util/attribute-table.cpp
               ${CHIP_ROOT}/src/app/util/binding-table.cpp
               ${CHIP_ROOT}/src/app/util/ember-compatibility-functions.cpp
               ${CHIP_ROOT}/src/app/util/ember-print.cpp
               ${CHIP_ROOT}/src/app/util/error-mapping.cpp
               ${CHIP_ROOT}/src/app/util/message.cpp
               ${CHIP_ROOT}/src/app/util/privilege-storage.cpp
               ${CHIP_ROOT}/src/app/util/util.cpp
               ${CHIP_ROOT}/src/app/server/EchoHandler.cpp
               ${CHIP_ROOT}/src/app/server/Dnssd.cpp
               ${CHIP_ROOT}/src/app/server/OnboardingCodesUtil.cpp
               ${CHIP_ROOT}/src/app/server/Server.cpp
               ${CHIP_ROOT}/src/app/server/CommissioningWindowManager.cpp
               ${CHIP_ROOT}/src/app/clusters/access-control-server/access-control-server.cpp
               ${CHIP_ROOT}/src/app/clusters/administrator-commissioning-server/administrator-commissioning-server.cpp
               ${CHIP_ROOT}/src/app/clusters/basic/basic.cpp
               ${CHIP_ROOT}/src/app/clusters/bindings/BindingManager.cpp
               ${CHIP_ROOT}/src/app/clusters/bindings/bindings.cpp
               ${CHIP_ROOT}/src/app/clusters/bindings/PendingNotificationMap.cpp
               ${CHIP_ROOT}/src/app/clusters/descriptor/descriptor.cpp
               ${CHIP_ROOT}/src/app/clusters/identify-server/identify-server.cpp
               ${CHIP_ROOT}/src/app/clusters/diagnostic-logs-server/diagnostic-logs-server.cpp
               ${CHIP_ROOT}/src/app/clusters/ethernet-network-diagnostics-server/ethernet-network-diagnostics-server.cpp
               ${CHIP_ROOT}/src/app/clusters/localization-configuration-server/localization-configuration-server.cpp
               ${CHIP_ROOT}/src/app/clusters/time-format-localization-server/time-format-localization-server.cpp
               ${CHIP_ROOT}/src/app/clusters/fixed-label-server/fixed-label-server.cpp
               ${CHIP_ROOT}/src/app/clusters/user-label-server/user-label-server.cpp               
               ${CHIP_ROOT}/src/app/clusters/thread-network-diagnostics-server/thread-network-diagnostics-server.cpp
               ${CHIP_ROOT}/src/app/clusters/wifi-network-diagnostics-server/wifi-network-diagnostics-server.cpp
               ${CHIP_ROOT}/src/app/clusters/software-diagnostics-server/software-diagnostics-server.cpp
               ${CHIP_ROOT}/src/app/clusters/switch-server/switch-server.cpp
               ${CHIP_ROOT}/src/app/clusters/general-diagnostics-server/general-diagnostics-server.cpp               	       
               ${CHIP_ROOT}/src/app/clusters/group-key-mgmt-server/group-key-mgmt-server.cpp
               ${CHIP_ROOT}/src/app/clusters/general-commissioning-server/general-commissioning-server.cpp
               ${CHIP_ROOT}/src/app/clusters/operational-credentials-server/operational-credentials-server.cpp
               ${CHIP_ROOT}/src/app/clusters/occupancy-sensor-server/occupancy-sensor-server.cpp
               ${CHIP_ROOT}/src/app/clusters/network-commissioning/network-commissioning.cpp
               ${CHIP_ROOT}/src/app/clusters/ota-requestor/ota-requestor-server.cpp
               ${CHIP_ROOT}/src/app/clusters/ota-requestor/BDXDownloader.cpp
               ${CHIP_ROOT}/src/app/clusters/ota-requestor/DefaultOTARequestor.cpp
               ${CHIP_ROOT}/src/app/clusters/ota-requestor/DefaultOTARequestorDriver.cpp
               ${CHIP_ROOT}/src/app/clusters/groups-server/groups-server.cpp
               )
