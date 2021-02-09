#
#   Copyright (c) 2020 Project CHIP Authors
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
#     CMake utilities for managing and retrieving mbed build configuration
#

function(get_flags_for_lang lang input output)
  set(tmp_list "")

  list(LENGTH ${input} nb_elem)
  set(index 0)
  set(is_compile_lang_expression 0)

  while(${index} LESS ${nb_elem})
    list(GET ${input} ${index} value)
    if(value MATCHES "<COMPILE_LANGUAGE:${lang}>:([^>]+)")
      string(REGEX REPLACE "^[^:]*:" "" updated_flag ${value})
      list(APPEND tmp_list ${updated_flag})
      if(NOT value MATCHES ">$")
        set(is_compile_lang_expression 1)
      endif() 
    elseif(is_compile_lang_expression)
      if(value MATCHES ">$")
        set(is_compile_lang_expression 0)
      endif()
      string(REPLACE ">" "" updated_flag ${value})
      list(APPEND tmp_list ${updated_flag})
    endif()
    math(EXPR index "${index}+1")
  endwhile()

  set(${output} ${tmp_list} PARENT_SCOPE)
endfunction()


function(mbed_get_include_directories i)
  get_property(flags TARGET mbed-core PROPERTY INTERFACE_INCLUDE_DIRECTORIES)
  list(APPEND CFLAG_LIST)
  foreach(flag ${flags})
    list(APPEND CFLAG_LIST "\"-I${flag}\"")
  endforeach()
  set(${i} ${CFLAG_LIST} PARENT_SCOPE)
endfunction()

function(mbed_get_compile_definitions i)
  get_property(flags TARGET mbed-core PROPERTY INTERFACE_COMPILE_DEFINITIONS)

  #Temporary workaround for mbedtls - remove after switch to mbed source for mbedtls
  list(REMOVE_ITEM flags TARGET_LIKE_MBED)

  list(APPEND CFLAG_LIST)
  foreach(flag ${flags})
    # Replace each quote with a '\"' - format required for the GN arguments
    string(REPLACE "\""  "\\\\\""  output_flag ${flag})
    list(APPEND CFLAG_LIST "\"-D${output_flag}\"")
  endforeach()
  set(${i} ${CFLAG_LIST} PARENT_SCOPE)
endfunction()

function(mbed_get_compile_options_for_lang lang i)
  get_property(flags TARGET mbed-core PROPERTY INTERFACE_COMPILE_OPTIONS)
  get_flags_for_lang(${lang} flags output_list)
  set(${i} ${output_list} PARENT_SCOPE)
endfunction()


# Retrieve mbed common compiler flags
function(mbed_get_common_compile_flags VAR)
  mbed_get_include_directories(INCLUDES)
  mbed_get_compile_definitions(DEFINES)
  set(${VAR} ${INCLUDES} ${DEFINES} ${${VAR}} PARENT_SCOPE)
endfunction()

# Retrieve mbed compiler flags for the given language (C or CXX)
function(mbed_get_lang_compile_flags VAR LANG)
  mbed_get_compile_options_for_lang(${LANG} FLAGS)
  set(${VAR} ${FLAGS} ${${VAR}} PARENT_SCOPE)
endfunction()


function(convert_list_of_flags_to_string_of_flags ptr_list_of_flags string_of_flags separator)
  # Convert the list to a string so we can do string replace
  # operations on it and replace the ";" list separators with a
  # desirable one so the flags are spaced out
  string(REPLACE ";"  ${separator} locally_scoped_string_of_flags "${${ptr_list_of_flags}}")
  # Removing excess spaces
  string(REPLACE "  "  " "  locally_scoped_string_of_flags "${locally_scoped_string_of_flags}")

  # Set the output variable in the parent scope
  set(${string_of_flags} ${locally_scoped_string_of_flags} PARENT_SCOPE)
endfunction()