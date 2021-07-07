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

# Get compilation flags for specific lang
# The flags might contains compile language generator expressions that
# look like this:
# $<$<COMPILE_LANGUAGE:CXX>:-fno-exceptions>
# Applying a regex to extract the flag and also to find out if the language matches.
# [Args]:
#   input - list of flags to parse
#   output - list of flags set to specific language
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

# Get include directory of target build
# Get target property of includes directories
# For each flag add -I prefix and put it in quotation marks
# [Args]:
#   target - target name
#   output - output variable name
function(mbed_get_include_directories target output)
  get_property(flags TARGET ${target} PROPERTY INTERFACE_INCLUDE_DIRECTORIES)
  list(APPEND CFLAG_LIST)
  foreach(flag ${flags})
    list(APPEND CFLAG_LIST "\"-isystem${flag}\"")
  endforeach()
  set(${output} ${CFLAG_LIST} PARENT_SCOPE)
endfunction()

# Get compile definitions of target build
# Get target property of compile definitions
# For each flag change format, add -D prefix and put it in quotation marks
# [Args]:
#   target - target name
#   output - output variable name
function(mbed_get_compile_definitions target output)
  get_property(flags TARGET ${target} PROPERTY INTERFACE_COMPILE_DEFINITIONS)

  list(APPEND CFLAG_LIST)
  foreach(flag ${flags})
    # Replace each quote with a '\"' - format required for the GN arguments
    string(REPLACE "\""  "\\\\\""  output_flag ${flag})
    list(APPEND CFLAG_LIST "\"-D${output_flag}\"")
  endforeach()
  set(${output} ${CFLAG_LIST} PARENT_SCOPE)
endfunction()

# Get compile options of mbed build for specific language
# Get mbed-core property of compile options
# [Args]:
#   lang - compilation languge (C, C++ or ASM)
#   target - target name
#   output - output variable name
function(mbed_get_compile_options_for_lang lang target output)
  get_property(flags TARGET ${target} PROPERTY INTERFACE_COMPILE_OPTIONS)
  get_flags_for_lang(${lang} flags output_list)
  set(${output} ${output_list} PARENT_SCOPE)
endfunction()


# Retrieve common compilation flags specific for target
# [Args]:
#   VAR - flags variable name
#   TARGET - target name
function(mbed_get_target_common_compile_flags VAR TARGET)
  mbed_get_include_directories(${TARGET} INCLUDES)
  mbed_get_compile_definitions(${TARGET} DEFINES)
  set(${VAR} ${INCLUDES} ${DEFINES} ${${VAR}} PARENT_SCOPE)
endfunction()

# Retrieve target compiler flags for the specific language (C or CXX)
# [Args]:
#   VAR - flags variable name
#   TARGET - target name
#   LANG - compilation languge (C, C++ or ASM) 
function(mbed_get_lang_compile_flags VAR TARGET LANG)
  mbed_get_compile_options_for_lang(${LANG} ${TARGET} FLAGS)
  set(${VAR} ${FLAGS} ${${VAR}} PARENT_SCOPE)
endfunction()

# Convert list of flags to string format
# [Args]:
#   ptr_list_of_flags - list fo flags
#   string_of_flags - output string
#   separator - flags separator
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