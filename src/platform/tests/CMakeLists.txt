#
#
#    Copyright (c) 2020 Project CHIP Authors
#
#    Licensed under the Apache License, Version 2.0 (the "License");
#    you may not use this file excegrep  in compliance with the License.
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
#    Description:
#      This file is the CMake template for the CHIP SDK Device Layer tests.
#

add_library(DeviceLayerTests
    TestPlatformMgr.cpp
    TestPlatformTime.cpp
    TestConfigurationMgr.cpp
)

target_include_directories(DeviceLayerTests PUBLIC
    ${PROJECT_BINARY_DIR}/src/include
    ${PROJECT_SOURCE_DIR}/src/include
    ${PROJECT_SOURCE_DIR}/src
    ${PROJECT_SOURCE_DIR}/src/lib
    ${PROJECT_SOURCE_DIR}/src/system
    ${PROJECT_SOURCE_DIR}/src/platform
)

target_link_libraries(DeviceLayerTests PUBLIC
    DeviceLayer
    InetLayer
    SystemLayer
    SupportLayer
    nlunit-test
    nlfaultinjection
    ${CHIP_PLATFORM_LIBS}
    chip-config
)

add_executable(TestPlatformTime TestPlatformTimeDriver.cpp)
target_link_libraries(TestPlatformTime PRIVATE DeviceLayerTests)
add_test(NAME TestPlatformTime COMMAND TestPlatformTime)
add_dependencies(check TestPlatformTime)

add_executable(TestPlatformMgr TestPlatformMgrDriver.cpp)
target_link_libraries(TestPlatformMgr PRIVATE DeviceLayerTests)
add_test(NAME TestPlatformMgr COMMAND TestPlatformMgr)
add_dependencies(check TestPlatformMgr)

add_executable(TestPlatformConfigurationMgr TestConfigurationMgrDriver.cpp)
target_link_libraries(TestPlatformConfigurationMgr PRIVATE DeviceLayerTests)
add_test(NAME TestPlatformConfigurationMgr COMMAND TestPlatformConfigurationMgr)
add_dependencies(check TestPlatformConfigurationMgr)
