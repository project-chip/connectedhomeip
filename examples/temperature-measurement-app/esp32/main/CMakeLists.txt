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
#
# (Uses default behaviour of compiling all source files in directory, adding 'include' to include path.)
idf_component_register(PRIV_INCLUDE_DIRS 
                      "${CMAKE_CURRENT_LIST_DIR}"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/third_party/nlio/repo/include"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/src"
                      "${CMAKE_CURRENT_LIST_DIR}/include"
                      SRC_DIRS
                      "${CMAKE_CURRENT_LIST_DIR}"
                      "${CMAKE_CURRENT_LIST_DIR}/gen"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/src/app/server"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/src/app/util"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/src/app/reporting"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/src/app/clusters/basic"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/src/app/clusters/bindings"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/src/app/clusters/diagnostic-logs-server"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/src/app/clusters/general-commissioning-server"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/src/app/clusters/network-commissioning"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/src/app/clusters/operational-credentials-server"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/src/app/clusters/temperature-measurement-server"
                      PRIV_REQUIRES chip QRCode tft spidriver bt screen-framework)

set_property(TARGET ${COMPONENT_LIB} PROPERTY CXX_STANDARD 14) 
target_compile_options(${COMPONENT_LIB} PRIVATE "-DLWIP_IPV6_SCOPES=0" "-DCHIP_HAVE_CONFIG_H")
