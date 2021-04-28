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
set(PRIV_INCLUDE_DIRS_GROUPS 
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/examples/all-clusters-app/all-clusters-common"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/third_party/nlio/repo/include"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/src"
                      "${CMAKE_CURRENT_LIST_DIR}/include"
)
set(SRC_DIRS_GROUPS
                      "${CMAKE_CURRENT_LIST_DIR}"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/examples/all-clusters-app/all-clusters-common/gen"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/src/app/server"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/src/app/util"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/src/app/clusters/test-cluster-server"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/src/app/clusters/descriptor"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/src/app/clusters/on-off-server"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/src/app/clusters/level-control"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/src/app/clusters/identify"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/src/app/clusters/barrier-control-server"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/src/app/clusters/general-commissioning-server"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/src/app/clusters/network-commissioning"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/src/app/clusters/groups-server"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/src/app/clusters/color-control-server"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/src/app/clusters/content-launch-server"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/src/app/clusters/operational-credentials-server"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/src/app/clusters/trusted-root-certificates-server"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/src/app/clusters/low-power-server"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/src/app/clusters/media-playback-server"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/src/app/clusters/ota-server"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/src/app/clusters/temperature-measurement-server"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/src/app/clusters/scenes"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/src/app/clusters/basic"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/src/app/clusters/bindings"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/src/app/reporting"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/src/app/clusters/door-lock-server"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/src/app/clusters/occupancy-sensor-server"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/src/app/clusters/ias-zone-server"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/src/app/clusters/relative-humidity-measurement-server"
                      #${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/src/app/clusters/ias-zone-client
)

if(${CONFIG_DEVICE_TYPE_ESP32_C3_DEVKITM})
set(PRIV_INCLUDE_DIRS_GROUPS ${PRIV_INCLUDE_DIRS_GROUPS}
                             "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/examples/common/screen-framework/include")
set(PRIV_REQUIRES_GROUPS chip QRCode bt)
endif()

if(${CONFIG_DEVICE_TYPE_ESP32_DEVKITC})
set(PRIV_INCLUDE_DIRS_GROUPS ${PRIV_INCLUDE_DIRS_GROUPS}
                             "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/examples/common/screen-framework/include")
set(PRIV_REQUIRES_GROUPS chip QRCode bt)
endif()

if(${CONFIG_DEVICE_TYPE_ESP32_WROVER_KIT})
    set(PRIV_REQUIRES_GROUPS chip QRCode bt tft spidrier screen-framework)
endif()

if(${CONFIG_DEVICE_TYPE_M5STACK})
    set(PRIV_REQUIRES_GROUPS chip QRCode bt tft spidrier screen-framework)
endif()

idf_component_register(PRIV_INCLUDE_DIRS ${PRIV_INCLUDE_DIRS_GROUPS} 
                       SRC_DIRS ${SRC_DIRS_GROUPS}
                       PRIV_REQUIRES ${PRIV_REQUIRES_GROUPS})

set_property(TARGET ${COMPONENT_LIB} PROPERTY CXX_STANDARD 14)
target_compile_options(${COMPONENT_LIB} PRIVATE "-DLWIP_IPV6_SCOPES=0" "-DCHIP_HAVE_CONFIG_H")
