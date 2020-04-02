#
#    Copyright (c) 2020 Project CHIP Authors
#    Copyright (c) 2014-2017 Nest Labs, Inc.
#    All rights reserved.
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
#      This file defines a GNU autoconf M4-style macro for checking
#      project-specific header search paths.
#

#
# CHIP_CHECK_PROJECT_CONFIG_INCLUDES(option, header, description, default)
#
#   option          - The stem of the option advertised via AC_ARG_WITH.
#   header-define   - The pre-processor macro that will be defined with the
#                     with the name of the header file.
#   header          - The name of the header file to search for when a directory
#                     is given. 
#   description     - The short description of the project-specific configuration
#                     header.
#   default-dir     - [optional] The default directory to search for project-specific
#                     configuration headers.
#
# Create a configuration option that allows the user to specify the place to
# search for the named project-specific configuration header.
# ----------------------------------------------------------------------------
AC_DEFUN([CHIP_CHECK_PROJECT_CONFIG_INCLUDES],
[
AC_ARG_WITH($1,
    [AS_HELP_STRING([--with-$1=FILE|DIR],
                    [Specify a project-specific configuration header for $4. If a directory is given, it is searched for the file $3. @<:@default=$5@:>@])],
    [
        # If a file or directory was specified...
        if test "x${withval}" != "x" -a "x${withval}" != "xno"; then
        
            # Attempt to find the specified file, or a directory containing the default 
            # file name.  NOTE: We do not use AC_CHECK_HEADERS here because the supplied
            # file may not be compilable out of context of the full CHIP build.
            if test -f "${withval}"; then
                _chip_tmp_project_file="${withval}"
                _chip_tmp_project_dir=
            elif test -f "${withval}/include/$3"; then
                _chip_tmp_project_file="$3"
                _chip_tmp_project_dir="${withval}/include/"
            elif test -f "${withval}/$3"; then
                _chip_tmp_project_file="$3"
                _chip_tmp_project_dir="${withval}/"
            else
                AC_MSG_ERROR([Project-specific configuration for %4 not found: ${withval}".])
            fi

            # If a directory was given, include it in the header search paths.
            if test "x${_chip_tmp_project_dir}" != "x"; then
                CPPFLAGS="-I${_chip_tmp_project_dir} ${CPPFLAGS}"
            fi

            # Define a pre-processor macro containing the name of the located header file in angle brackets
            # (e.g. <CHIPProjectConfig.h>).
            AC_DEFINE_UNQUOTED([$2], [<${_chip_tmp_project_file}>], [Path to $4 platform config header file])

            # Accumulate a list of the project config files that have been selected.
            CHIP_PROJECT_CONFIG_INCLUDES="$CHIP_PROJECT_CONFIG_INCLUDES ${_chip_tmp_project_dir}${_chip_tmp_project_file}"
        fi
    ],
    [
        # If a default location was specified and it contains the expected header file...
        if test "x$5" != "x" -a -f "$5/$3"; then
            
            # Include the default directory in the header search paths.
            CPPFLAGS="-I$5 ${CPPFLAGS}"
            
            # Define a pre-processor macro containing the default header name in angle brackets
            # (e.g. <CHIPProjectConfig.h>).
            AC_DEFINE_UNQUOTED([$2], [<$3>], [Path to $4 platform config header file])

            # Accumulate a list of the project config files that have been selected.
            CHIP_PROJECT_CONFIG_INCLUDES="$CHIP_PROJECT_CONFIG_INCLUDES $5/$3"
        fi
    ])
])

# A list of the project configuration headers that have been selected
# for inclusion.
#
AC_SUBST(CHIP_PROJECT_CONFIG_INCLUDES)
