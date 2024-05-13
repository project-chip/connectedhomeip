#
# Copyright (c) 2020 Project CHIP Authors
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

cmake_minimum_required(VERSION 3.4.1)

add_library(native-lib SHARED src/main/cpp/native-lib.cpp)
find_library(log-lib log)

target_link_libraries(native-lib ${log-lib}  ${CMAKE_HOME_DIRECTORY}/../../../setup_payload/libSetupPayload.a)
target_include_directories(native-lib PRIVATE ${CMAKE_HOME_DIRECTORY}/../../../lib ${CMAKE_HOME_DIRECTORY}/../../..)
add_definitions(-DCHIP_SYSTEM_CONFIG_USE_SOCKETS)
