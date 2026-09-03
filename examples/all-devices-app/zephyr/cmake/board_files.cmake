#
#    Copyright (c) 2026 Project CHIP Authors
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

# Vendor subdirectories containing board-specific application files.
set(ALL_DEVICES_BOARD_VENDOR_SUBDIRECTORIES
    silabs
)

# Strips the revision and qualifiers from BOARD.
function(all_devices_board_name board out_var)
    if(NOT "${board}" MATCHES "^([^@/]+)(@[^@/]+)?(/[^@]+)?$")
        message(FATAL_ERROR "Invalid BOARD value: ${board}")
    endif()
    set(${out_var} "${CMAKE_MATCH_1}" PARENT_SCOPE)
endfunction()

# Locates a board-specific file in boards/ or a registered vendor subdirectory.
function(all_devices_find_board_file boards_dir filename out_var)
    cmake_parse_arguments(ARG "EXCLUDE_FLAT" "" "" ${ARGN})

    set(_directories "")
    if(NOT ARG_EXCLUDE_FLAT)
        list(APPEND _directories "${boards_dir}")
    endif()
    foreach(_vendor IN LISTS ALL_DEVICES_BOARD_VENDOR_SUBDIRECTORIES)
        list(APPEND _directories "${boards_dir}/${_vendor}")
    endforeach()

    set(_match "")
    foreach(_directory IN LISTS _directories)
        set(_candidate "${_directory}/${filename}")
        if(EXISTS "${_candidate}")
            if(_match)
                message(FATAL_ERROR "Multiple board files named ${filename}: ${_match};${_candidate}")
            endif()
            set(_match "${_candidate}")
        endif()
    endforeach()
    set(${out_var} "${_match}" PARENT_SCOPE)
endfunction()
