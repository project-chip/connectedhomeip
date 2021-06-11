#
#   Copyright (c) 2021 Project CHIP Authors
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
#     CMake utilities for managing and retrieving Zephyr build configuration
#

# Retrieve Zephyr compiler flags for the given language (C or CXX)
function(zephyr_get_compile_flags VAR LANG)
    # We want to treat all zephyr-provided headers as system headers, so
    # warnings in them don't trigger -Werror.  That means that for the headers
    # zephyr returns as "non-system" headers (the ones from
    # zephyr_get_include_directories_for_lang) we need to manually replace "-I"
    # with "-isystem".
    zephyr_get_include_directories_for_lang_as_string(${LANG} INCLUDES)
    string(REGEX REPLACE "(^| )-I" "\\1-isystem" INCLUDES ${INCLUDES})
    zephyr_get_system_include_directories_for_lang_as_string(${LANG} SYSTEM_INCLUDES)
    zephyr_get_compile_definitions_for_lang_as_string(${LANG} DEFINES)
    zephyr_get_compile_options_for_lang_as_string(${LANG} FLAGS)
    set(${VAR} ${INCLUDES} ${SYSTEM_INCLUDES} ${DEFINES} ${FLAGS} ${${VAR}} PARENT_SCOPE)
endfunction()

# Select gnu++<YY> standard based on Kconfig configuration
macro(zephyr_get_gnu_cpp_standard VAR)
    if (CONFIG_STD_CPP11)
        list(APPEND ${VAR} -std=gnu++11)
    elseif (CONFIG_STD_CPP14)
        list(APPEND ${VAR} -std=gnu++14)
    elseif (CONFIG_STD_CPP17)
        list(APPEND ${VAR} -std=gnu++17)
    elseif (CONFIG_STD_CPP2A)
        list(APPEND ${VAR} -std=gnu++20)
    else()
        message(FATAL_ERROR "Building with unsupported C++ standard")
    endif()
endmacro()

function(zephyr_set_openthread_config_impl OT_DIR CONFIG_FILE)
    get_property(DEFINES DIRECTORY ${OT_DIR} PROPERTY COMPILE_DEFINITIONS)
    get_property(SUBDIRS DIRECTORY ${OT_DIR} PROPERTY SUBDIRECTORIES)

    list(TRANSFORM DEFINES REPLACE 
         OPENTHREAD_PROJECT_CORE_CONFIG_FILE=.*
         OPENTHREAD_PROJECT_CORE_CONFIG_FILE="${CONFIG_FILE}")

    set_property(DIRECTORY ${OT_DIR} PROPERTY COMPILE_DEFINITIONS ${DEFINES})

    foreach(SUBDIR ${SUBDIRS})
    zephyr_set_openthread_config_impl(${SUBDIR} ${CONFIG_FILE})
    endforeach()
endfunction()

# Replace Zephyr-supplied configuration file with a custom one
function(zephyr_set_openthread_config CONFIG_PATH)
    get_filename_component(CONFIG_DIR ${CONFIG_PATH} DIRECTORY)
    get_filename_component(CONFIG_FILE ${CONFIG_PATH} NAME)
    target_include_directories(ot-config INTERFACE ${CONFIG_DIR})
    zephyr_set_openthread_config_impl(${ZEPHYR_OPENTHREAD_MODULE_DIR} ${CONFIG_FILE})
endfunction()
