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
#      This file is the CMake template for the CHIP SDK public headers.
#

add_custom_command(
    OUTPUT OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/CHIPVersion.h
    COMMAND ${PROJECT_SOURCE_DIR}/scripts/gen-chip-version.sh -o CHIPVersion.h `cat ${PROJECT_SOURCE_DIR}/.default-version`
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    DEPENDS ${PROJECT_SOURCE_DIR}/.default-version
)

add_custom_target(chip-includes DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/CHIPVersion.h)