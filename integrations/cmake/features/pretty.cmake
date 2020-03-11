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
#      This file is the CHIP SDK CMake template for adding the `pretty`
#      and `pretty-check` targets for enforcing code style guidelines.
#

option(BUILD_PRETTY "Pretty and pretty-check code styling with clang-format" ON)

if ( BUILD_PRETTY )

file(GLOB_RECURSE ALL_CXX_SOURCE_FILES
    ${CHIP_PROJECT_SOURCE_DIR}/src/*.[ch]pp
)

add_custom_target(pretty
    COMMAND ${CHIP_PROJECT_SOURCE_DIR}/scripts/helpers/clang-format.sh
    ${ALL_CXX_SOURCE_FILES}

    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Pretty formatting ..."
)

add_custom_target(pretty-check
    COMMAND ${CHIP_PROJECT_SOURCE_DIR}/scripts/helpers/clang-format-check.sh
    ${ALL_CXX_SOURCE_FILES}

    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Pretty check ..."
)

endif ( BUILD_PRETTY )
