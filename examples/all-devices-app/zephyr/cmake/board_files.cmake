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

# Strips qualifiers from BOARD
function(all_devices_board_name board out_var)
    string(REGEX REPLACE "/.*$" "" _name "${board}")
    set(${out_var} "${_name}" PARENT_SCOPE)
endfunction()

# Locates a board-specific file under boards/, whether it sits directly in
# boards/ or is grouped under a vendor subfolder (e.g. boards/vendor/).
function(all_devices_find_board_file boards_dir filename out_var)
    cmake_parse_arguments(ARG "EXCLUDE_FLAT" "" "" ${ARGN})

    set(${out_var} "" PARENT_SCOPE)
    file(GLOB_RECURSE _candidates "${boards_dir}/${filename}")
    foreach(_candidate IN LISTS _candidates)
        if(ARG_EXCLUDE_FLAT)
            get_filename_component(_candidate_dir "${_candidate}" DIRECTORY)
            if(_candidate_dir STREQUAL "${boards_dir}")
                continue()
            endif()
        endif()
        set(${out_var} "${_candidate}" PARENT_SCOPE)
        return()
    endforeach()
endfunction()

# Locates a soc-specific file under socs/, whether it sits directly in
# socs/ or is grouped under a vendor subfolder (e.g. socs/vendor/).
function(all_devices_find_soc_for_board board out_var)
    set(${out_var} "" PARENT_SCOPE)

    set(_roots "")
    if(DEFINED ENV{ZEPHYR_BASE})
        list(APPEND _roots "$ENV{ZEPHYR_BASE}/boards")
    endif()
    foreach(_extra_root IN LISTS BOARD_ROOT)
        list(APPEND _roots "${_extra_root}/boards")
    endforeach()

    foreach(_root IN LISTS _roots)
        file(GLOB_RECURSE _board_ymls "${_root}/board.yml")
        foreach(_yml IN LISTS _board_ymls)
            file(STRINGS "${_yml}" _lines)
            set(_in_board FALSE)
            set(_in_socs FALSE)
            foreach(_line IN LISTS _lines)
                if(_line MATCHES "^  - name: +([A-Za-z0-9_]+)")
                    set(_in_board FALSE)
                    set(_in_socs FALSE)
                    if(CMAKE_MATCH_1 STREQUAL board)
                        set(_in_board TRUE)
                    endif()
                elseif(_in_board AND _line MATCHES "^ +socs:")
                    set(_in_socs TRUE)
                elseif(_in_board AND _in_socs AND _line MATCHES "^ +- name: +([A-Za-z0-9_]+)")
                    set(${out_var} "${CMAKE_MATCH_1}" PARENT_SCOPE)
                    return()
                endif()
            endforeach()
        endforeach()
    endforeach()
endfunction()
