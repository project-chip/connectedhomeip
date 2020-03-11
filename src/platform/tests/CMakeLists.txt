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

add_library(DeviceLayerTests
    TestPlatformMgr.cpp
    TestPlatformTime.cpp
    TestConfigurationMgr.cpp
)

set(COMMON_INCLUDES
    ${PROJECT_BINARY_DIR}/src/include
    ${PROJECT_SOURCE_DIR}/src/include
    ${PROJECT_SOURCE_DIR}/src
    ${PROJECT_SOURCE_DIR}/src/lib
    ${PROJECT_SOURCE_DIR}/src/system
    ${PROJECT_SOURCE_DIR}/src/platform
    ${PROJECT_SOURCE_DIR}/third_party/nlassert/repo/src
    ${PROJECT_SOURCE_DIR}/third_party/nlunit-test/repo/src
    ${PROJECT_SOURCE_DIR}/third_party/nlfaultinjection/repo/include
)

target_include_directories(DeviceLayerTests PRIVATE ${COMMON_INCLUDES})
target_link_libraries(DeviceLayerTests PRIVATE chip-config)

set(COMMON_LIBS
    DeviceLayerTests
    DeviceLayer
    InetLayer
    SystemLayer
    SupportLayer
    nlunit-test
    nlfaultinjection
    ${CHIP_PLATFORM_LIBS}
    chip-config
)

set(COMMON_LDPATH)

add_executable(TestPlatformTime TestPlatformTimeDriver.cpp)
target_include_directories(TestPlatformTime PRIVATE ${COMMON_INCLUDES})
target_link_libraries(TestPlatformTime PRIVATE ${COMMON_LIBS})
target_link_directories(TestPlatformTime PRIVATE ${COMMON_LDPATH})
add_test(NAME TestPlatformTime COMMAND TestPlatformTime)

add_executable(TestPlatformMgr TestPlatformMgrDriver.cpp)
target_include_directories(TestPlatformMgr PRIVATE ${COMMON_INCLUDES})
target_link_libraries(TestPlatformMgr PRIVATE ${COMMON_LIBS})
target_link_directories(TestPlatformMgr PRIVATE ${COMMON_LDPATH})
add_test(NAME TestPlatformMgr COMMAND TestPlatformMgr)

add_executable(TestPlatformConfigurationMgr TestConfigurationMgrDriver.cpp)
target_include_directories(TestPlatformConfigurationMgr PRIVATE ${COMMON_INCLUDES})
target_link_libraries(TestPlatformConfigurationMgr PRIVATE ${COMMON_LIBS})
target_link_directories(TestPlatformConfigurationMgr PRIVATE ${COMMON_LDPATH})
add_test(NAME TestPlatformConfigurationMgr COMMAND TestPlatformConfigurationMgr)
