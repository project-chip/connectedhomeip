#
#    Copyright 2015-2016 Nest Labs Inc. All Rights Reserved.
#
#    Licensed under the Apache License, Version 2.0 (the "License");
#    you may not use this file except in compliance with the License.
#    You may obtain a copy of the License at
#
#    http://www.apache.org/licenses/LICENSE-2.0
#
#    Unless required by applicable law or agreed to in writing, software
#    distributed under the License is distributed on an "AS IS" BASIS,
#    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#    See the License for the specific language governing permissions and
#    limitations under the License.
#

#
#    Description:
#      This file defines a GNU autoconf M4-style macro that adds an
#      --enable-warnings-as-errors configuration option to the package
#      and controls whether the package will be built to treat all
#      compilation warnings as errors.  #

#
# NL_ENABLE_WERROR(default)
#
#   default - Whether the option should be enabled (yes) or disabled (no)
#             by default.
#
# Adds an --enable-warnings-as-errors configuration option to the
# package with a default value of 'default' (should be either 'no' or
# 'yes') and controls whether the package will be built with or
# without -Werror enabled.
#
# The value 'nl_cv_warnings_as_errors' will be set to the result. In
# addition, the variable NL_WERROR_CPPFLAGS will be set to the
# compiler-specific flag necessary to assert this option.
#
#------------------------------------------------------------------------------
AC_DEFUN([NL_ENABLE_WERROR],
[
    # Check whether or not a default value has been passed in.

    m4_case([$1],
        [yes],[],
        [no],[],
        [m4_fatal([$0: invalid default value '$1'; must be 'yes' or 'no'])])

    AC_CACHE_CHECK([whether to treat all compilation warnings as errors],
        nl_cv_warnings_as_errors,
        [
            AC_ARG_ENABLE(warnings-as-errors,
                [AS_HELP_STRING([--enable-warnings-as-errors],[Treat all compilation warnings as errors @<:@default=$1@:>@.])],
                [
                    case "${enableval}" in 

                    no|yes)
                        nl_cv_warnings_as_errors=${enableval}
                        ;;

                    *)
                        AC_MSG_ERROR([Invalid value ${enableval} for --enable-warnings-as-errors])
                        ;;

                    esac
                ],
                [
                    nl_cv_warnings_as_errors=$1
                ])
    ])

    if test "${nl_cv_warnings_as_errors}" = "yes"; then
        AX_CHECK_COMPILER_OPTION([C], NL_WERROR_CPPFLAGS, [-Werror])
        if test "x${NL_WERROR_CPPFLAGS}" = "x"; then
            AC_MSG_ERROR([Could not determine how to treat warnings as errors for your compiler ${CC}])
        fi
    fi
])
