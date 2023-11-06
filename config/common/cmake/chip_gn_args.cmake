#
#   Copyright (c) 2023 Project CHIP Authors
#
#   Licensed under the Apache License, Version 2.0 (the "License");
#   you may not use this file except in compliance with the License.
#   You may obtain a copy of the License at
#
#       http://www.apache.org/licenses/LICENSE-2.0
#
#   Unless required by applicable law or agreed to in writing, software
#   distributed under the License is distributed on an "AS IS" BASIS,
#   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#   See the License for the specific language governing permissions and
#   limitations under the License.
#

#
#   @file
#     CMake file that allows collecting C/C++ compiler flags passed to
#     the Matter build system.
#

include(${CMAKE_CURRENT_LIST_DIR}/util.cmake)

# ==============================================================================
# Configuration variables and define constants
# ==============================================================================

# C/C++ compiler flags passed to the Matter build system
if (NOT MATTER_CFLAGS)
    set(MATTER_CFLAGS PARENT_SCOPE)
endif()

# C compiler flags passed to the Matter build system
if (NOT MATTER_CFLAGS_C)
    set(MATTER_CFLAGS_C PARENT_SCOPE)
endif()

# C++ compiler flags passed to the Matter build system
if (NOT MATTER_CFLAGS_CC)
    set(MATTER_CFLAGS_CC PARENT_SCOPE)
endif()

# GN meta-build system arguments in the form of 'key1 = value1\nkey2 = value2...' string
if (NOT MATTER_GN_ARGS)
    set(MATTER_GN_ARGS PARENT_SCOPE)
endif()

# ==============================================================================
# Macros
# ==============================================================================
# Add import GN argument
# [Args]:
#   file - path to file that should be imported
macro(matter_add_gn_arg_import file)
    string(APPEND MATTER_GN_ARGS "--module\n${file}\n")
endmacro()

# Add string  GN argument
# [Args]:
#   key - key name
#   value - string value
macro(matter_add_gn_arg_string key value)
    string(APPEND MATTER_GN_ARGS "--arg-string\n${key}\n${value}\n")
endmacro()

# Add bool GN argument
# [Args]:
#   key - bool variable
macro(matter_add_gn_arg_bool key)
    if (${ARGN})
        string(APPEND MATTER_GN_ARGS "--arg\n${key}\ntrue\n")
    else()
        string(APPEND MATTER_GN_ARGS "--arg\n${key}\nfalse\n")
    endif()
endmacro()

# Add compiler flag GN argument
# [Args]:
#   key - key name
#   value - compiler flag value
macro(matter_add_gn_arg_cflags key value)
    string(APPEND MATTER_GN_ARGS "--arg-cflags\n${key}\n${value}\n")
endmacro()

# Add simple variable GN argument
# [Args]:
#   key - variable name
#   value - variable value
macro(matter_add_gn_arg key value)
    string(APPEND MATTER_GN_ARGS "--arg\n${key}\n${value}\n")
endmacro()

# Add list variable GN argument
# [Args]:
#   key - variable name
#   value - list variable
macro(matter_add_gn_arg_list key value)
    set(_value_list ${value} ${ARGN})
    if (_value_list)
        string(REPLACE ";" "," _list_str "${_value_list}")
        string(APPEND MATTER_GN_ARGS "--arg\n${key}\n[${_list_str}]\n")
    endif()
endmacro()

# Add items to Matter common compiler flags
# [Args]:
#   flags - flags to add
macro(matter_add_flags flags)
    list(APPEND MATTER_CFLAGS ${flags})
endmacro()

# Add items to Matter C compiler flags
# [Args]:
#   flags - flags to add
macro(matter_add_cflags flags)
    list(APPEND MATTER_CFLAGS_C ${flags})
endmacro()

# Add items to Matter CXX compiler flags
# [Args]:
#   flags - flags to add
macro(matter_add_cxxflags flags)
    list(APPEND MATTER_CFLAGS_CC ${flags})
endmacro()

# Add GNU CPP standard flag to Matter CXX compiler flags
# [Args]:
#   std_version - standard version number e.g. 17 for C++17
macro(matter_add_gnu_cpp_standard std_version)
    list(APPEND MATTER_CFLAGS_CC -std=gnu++${std_version})
endmacro()

# Get compiler flags from listed targets.
# Collect common compile flags and save them in MATTER_CFLAGS
# Collect C/CXX compile flags and save them in MATTER_CFLAGS_C/MATTER_CFLAGS_CXX
# [Args]:
#   targets - list of targets
macro(matter_get_compiler_flags_from_targets targets)
    foreach(target ${targets})
        get_target_common_compile_flags(EXTERNAL_TARGET_CFLAGS ${target})
        get_lang_compile_flags(EXTERNAL_TARGET_CFLAGS_C ${target} C)
        get_lang_compile_flags(EXTERNAL_TARGET_CFLAGS_CXX ${target} CXX)
        list(APPEND MATTER_CFLAGS ${EXTERNAL_TARGET_CFLAGS})
        list(APPEND MATTER_CFLAGS_C ${EXTERNAL_TARGET_CFLAGS_C})
        list(APPEND MATTER_CFLAGS_CC ${EXTERNAL_TARGET_CFLAGS_CXX})
        # Reset between targets
        set(EXTERNAL_TARGET_CFLAGS "")
        set(EXTERNAL_TARGET_CFLAGS_C "")
        set(EXTERNAL_TARGET_CFLAGS_CXX "")
    endforeach()
endmacro()

# Generate the common Matter GN arguments.
# Pass all compilation flags to GN build.
# Available options are:
#   DEBUG           Debug build
#   LIB_SHELL       Add Matter shell library
#   LIB_PW_RPC      Add Matter PW RPC library
#   LIB_TESTS       Add Matter unit tests library
#   DEVICE_INFO_EXAMPLE_PROVIDER Add example device info provider support
#   PROJECT_CONFIG  Path to the project-specific configuration file
#   PROJECT_CONFIG_INC_DIR  Extra include dirs for project configurations
#
macro(matter_common_gn_args)
    set(options)
    set(oneValueArgs
        DEBUG
        LIB_TESTS
        LIB_SHELL
        LIB_PW_RPC
        DEVICE_INFO_EXAMPLE_PROVIDER
        PROJECT_CONFIG
        
    )
    set(multiValueArgs
        PROJECT_CONFIG_INC_DIR
    )

    cmake_parse_arguments(ARG "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    convert_list_of_flags_to_string_of_flags(MATTER_CFLAGS MATTER_CFLAGS)
    convert_list_of_flags_to_string_of_flags(MATTER_CFLAGS_C MATTER_CFLAGS_C)
    convert_list_of_flags_to_string_of_flags(MATTER_CFLAGS_CC MATTER_CFLAGS_CC)

    if (MATTER_CFLAGS)
        matter_add_gn_arg_cflags    ("target_cflags"           ${MATTER_CFLAGS})
    endif() # MATTER_CFLAGS
    if (MATTER_CFLAGS_C)
        matter_add_gn_arg_cflags    ("target_cflags_c"         ${MATTER_CFLAGS_C})
    endif() # MATTER_CFLAGS_C
    if (MATTER_CFLAGS_CC)
        matter_add_gn_arg_cflags    ("target_cflags_cc"        ${MATTER_CFLAGS_CC})
    endif() # MATTER_CFLAGS_CC

    matter_add_gn_arg_bool          ("is_debug"                 ${ARG_DEBUG})
    matter_add_gn_arg_bool          ("chip_build_tests"         ${ARG_LIB_TESTS})
    matter_add_gn_arg_bool          ("chip_build_libshell"      ${ARG_LIB_SHELL})

    if (ARG_LIB_PW_RPC)
        matter_add_gn_arg_bool      ("chip_build_pw_rpc_lib"    ${ARG_LIB_PW_RPC})
    endif() # ARG_LIB_PW_RPC
    if (ARG_DEVICE_INFO_EXAMPLE_PROVIDER)
        matter_add_gn_arg_bool      ("chip_build_example_providers" ${ARG_DEVICE_INFO_EXAMPLE_PROVIDER})
    endif() # ARG_DEVICE_INFO_EXAMPLE_PROVIDER
    if (ARG_PROJECT_CONFIG)
        get_filename_component(PROJECT_CONFIG
            ${ARG_PROJECT_CONFIG}
            REALPATH
            BASE_DIR ${CMAKE_SOURCE_DIR}
        )
        matter_add_gn_arg_string("chip_project_config_include"               "<${PROJECT_CONFIG}>")
        matter_add_gn_arg_string("chip_system_project_config_include"        "<${PROJECT_CONFIG}>")
    endif() # CHIP_PROJECT_CONFIG
    if (ARG_PROJECT_CONFIG_INC_DIR)
        matter_add_gn_arg_list("chip_project_config_include_dirs"             ${ARG_PROJECT_CONFIG_INC_DIR})
    endif()
endmacro()

# Generate the temporary GN arguments file from the settings
macro(matter_generate_args_tmp_file)
    file(GENERATE OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/args.tmp" CONTENT ${MATTER_GN_ARGS})
endmacro()