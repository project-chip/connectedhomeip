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
idf_component_register(PRIV_INCLUDE_DIRS 
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/examples/all-clusters-app/all-clusters-common"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/examples/platform/esp32/"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/third_party/nlio/repo/include"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/src"
                      "${CMAKE_CURRENT_LIST_DIR}/include"
                      SRC_DIRS
                      "${CMAKE_CURRENT_LIST_DIR}"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/examples/all-clusters-app/all-clusters-common/gen"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/examples/platform/esp32/shell_extension"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/src/app/server"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/src/app/util"
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
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/src/app/clusters/ota-server"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/src/app/clusters/binary-input-server"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/src/app/clusters/target-navigator-server"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/src/app/clusters/temperature-measurement-server"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/src/app/clusters/tv-channel-server"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/src/app/clusters/scenes"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/src/app/clusters/basic"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/src/app/clusters/bindings"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/src/app/reporting"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/src/app/clusters/door-lock-server"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/src/app/clusters/occupancy-sensor-server"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/src/app/clusters/ias-zone-server"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/src/app/clusters/relative-humidity-measurement-server"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/src/app/clusters/pump-configuration-and-control-server"
                      #${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/src/app/clusters/ias-zone-client
                      PRIV_REQUIRES chip QRCode tft spidriver bt screen-framework)

set_property(TARGET ${COMPONENT_LIB} PROPERTY CXX_STANDARD 14)
target_compile_options(${COMPONENT_LIB} PRIVATE "-DLWIP_IPV6_SCOPES=0" "-DCHIP_HAVE_CONFIG_H")
