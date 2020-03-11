#
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

#
#    Description:
#      This file is the CMake template for the CHIP SDK support library tests.
#

add_library(SupportLayerTests
    TestErrorStr.cpp
    TestTimeUtils.cpp
    TestCHIPArgParser.cpp
)

target_include_directories(SupportLayerTests PUBLIC
    ${PROJECT_SOURCE_DIR}/src
    ${PROJECT_SOURCE_DIR}/src/lib
    ${PROJECT_SOURCE_DIR}/src/include
    ${NLUNIT_TEST_INCLUDE}
)

target_link_libraries(SupportLayerTests PUBLIC 
    SupportLayer
    nlunit-test
    ${CHIP_PLATFORM_LIBS}
    chip-config
)

target_compile_definitions(SupportLayerTests PUBLIC
     _GNU_SOURCE  # enable vasprintf
)

target_link_directories(SupportLayerTests PUBLIC
    ${NLUNIT_TEST_LDPATH}
)

#### TestSupportErrorStr
add_executable(TestSupportErrorStr TestErrorStrDriver.cpp)
target_link_libraries(TestSupportErrorStr PRIVATE SupportLayerTests)
add_test(NAME TestSupportErrorStr COMMAND TestSupportErrorStr)
add_dependencies(check TestSupportErrorStr)

#### TestSupportTimeUtils
add_executable(TestSupportTimeUtils TestTimeUtilsDriver.cpp)
target_link_libraries(TestSupportTimeUtils PRIVATE SupportLayerTests)
add_test(NAME TestSupportTimeUtils COMMAND TestSupportTimeUtils)
add_dependencies(check TestSupportTimeUtils)

#### TestSupportCHIPArgParser
add_executable(TestSupportCHIPArgParser TestCHIPArgParserDriver.cpp)
target_link_libraries(TestSupportCHIPArgParser PRIVATE SupportLayerTests)
add_test(NAME TestSupportCHIPArgParser COMMAND TestSupportCHIPArgParser)
add_dependencies(check TestSupportCHIPArgParser)
