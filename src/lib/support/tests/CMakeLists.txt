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

set(COMMON_INCLUDES
    ${PROJECT_SOURCE_DIR}/src
    ${PROJECT_SOURCE_DIR}/src/include
    ${PROJECT_SOURCE_DIR}/src/lib
    ${PROJECT_SOURCE_DIR}/third_party/nlunit-test/repo/src
)

set(COMMON_LIBS
    SupportLayer
    nlunit-test
    ${CHIP_PLATFORM_LIBS}
    chip-config
)

set(COMMON_LDPATH)


#### TestSupportErrorStr

add_executable(TestSupportErrorStr
    TestErrorStr.cpp
    TestErrorStrDriver.cpp
)

target_include_directories(TestSupportErrorStr PRIVATE ${COMMON_INCLUDES})
target_link_libraries(TestSupportErrorStr PRIVATE ${COMMON_LIBS})
target_link_directories(TestSupportErrorStr PRIVATE ${COMMON_LDPATH})

add_test(NAME TestSupportErrorStr COMMAND TestSupportErrorStr)


#### TestSupportTimeUtils

add_executable(TestSupportTimeUtils
    TestTimeUtils.cpp
    TestTimeUtilsDriver.cpp
)

target_include_directories(TestSupportTimeUtils PRIVATE ${COMMON_INCLUDES})
target_link_libraries(TestSupportTimeUtils PRIVATE ${COMMON_LIBS})
target_link_directories(TestSupportTimeUtils PRIVATE ${COMMON_LDPATH})

add_test(NAME TestSupportTimeUtils COMMAND TestSupportTimeUtils)


#### TestSupportCHIPArgParser

add_executable(TestSupportCHIPArgParser
    TestCHIPArgParser.cpp
    TestCHIPArgParserDriver.cpp
)

target_include_directories(TestSupportCHIPArgParser PRIVATE ${COMMON_INCLUDES})
target_link_libraries(TestSupportCHIPArgParser PRIVATE ${COMMON_LIBS})
target_link_directories(TestSupportCHIPArgParser PRIVATE ${COMMON_LDPATH})

add_test(NAME TestSupportCHIPArgParser COMMAND TestSupportCHIPArgParser)
