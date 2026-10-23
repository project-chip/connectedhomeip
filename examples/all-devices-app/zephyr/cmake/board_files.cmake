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
    # Add other manufacturer folders here
)

# Strips the revision and qualifiers from BOARD.
function(all_devices_board_name board out_var)
    if(NOT "${board}" MATCHES "^([^@/]+)(@[^@/]+)?(/.*)?$")
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

# Appends to a sysbuild image variable while preserving caller-provided values
# and cache metadata.
function(all_devices_append_image_files var)
    set(_owned_var "_ALL_DEVICES_OWNED_${var}")
    set(_value ${${var}})

    if(DEFINED CACHE{${var}})
        get_property(_cache_type CACHE ${var} PROPERTY TYPE)
        get_property(_cache_help CACHE ${var} PROPERTY HELPSTRING)
    else()
        set(_cache_type INTERNAL)
        set(_cache_help "All-devices sysbuild image files")
    endif()

    foreach(_owned_file IN LISTS ${_owned_var})
        list(REMOVE_ITEM _value "${_owned_file}")
    endforeach()

    list(APPEND _value ${ARGN})
    if(_value)
        list(REMOVE_DUPLICATES _value)
    endif()

    set(${_owned_var} "${ARGN}" CACHE INTERNAL "All-devices sysbuild image files" FORCE)
    set(${var} "${_value}" CACHE ${_cache_type} "${_cache_help}" FORCE)
    set(${var} "${_value}" PARENT_SCOPE)
endfunction()
