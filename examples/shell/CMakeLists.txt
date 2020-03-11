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
    ${PROJECT_SOURCE_DIR}/src/crypto
)

set(COMMON_LIBS
    ChipShell
    ChipCore
    SupportLayer
    ${CHIP_PLATFORM_LIBS}
    chip-config
)

set(COMMON_LDPATH)

add_executable(chip-shell
    cmd_base64.cpp
    cmd_misc.cpp
    main.cpp
)

target_include_directories(chip-shell PRIVATE ${COMMON_INCLUDES})
target_link_libraries(chip-shell PRIVATE ${COMMON_LIBS})
target_link_directories(chip-shell PRIVATE ${COMMON_LDPATH})
