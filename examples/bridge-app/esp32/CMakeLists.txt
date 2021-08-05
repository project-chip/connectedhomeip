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

cmake_minimum_required(VERSION 3.5)
include($ENV{IDF_PATH}/tools/cmake/project.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/../../common/cmake/idf_flashing.cmake)

set(EXTRA_COMPONENT_DIRS
    "${CMAKE_CURRENT_LIST_DIR}/third_party/connectedhomeip/config/esp32/components"
    "${CMAKE_CURRENT_LIST_DIR}/../../common/QRCode"
)

project(chip-bridge-app)
idf_build_set_property(CXX_COMPILE_OPTIONS "-std=gnu++14;-Os;-DLWIP_IPV6_SCOPES=0;-DCHIP_HAVE_CONFIG_H;-DDYNAMIC_ENDPOINT_COUNT=16" APPEND)
idf_build_set_property(C_COMPILE_OPTIONS "-Os;-DLWIP_IPV6_SCOPES=0" APPEND)

flashing_script()

