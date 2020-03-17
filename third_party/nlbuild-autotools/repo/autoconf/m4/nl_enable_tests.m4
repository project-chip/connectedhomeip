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
#      --enable-tests configuration option to the package and controls
#      whether the package will be built with or without unit and
#      integration tests.
#

#
# NL_ENABLE_TESTS(default)
#
#   default - Whether the option should be enabled (yes) or disabled (no)
#             by default.
#
# Adds an --enable-tests configuration option to the package with a
# default value of 'default' (should be either 'no' or 'yes') and
# controls whether the package will be built with or without unit and
# integration tests.
#
# The value 'nl_cv_build_tests' will be set to the result.
#
#------------------------------------------------------------------------------

AC_DEFUN([NL_ENABLE_TESTS],
[
    # Check whether or not a default value has been passed in.

    m4_case([$1],
        [yes],[],
        [no],[],
        [m4_fatal([$0: invalid default value '$1'; must be 'yes' or 'no'])])

    AC_CACHE_CHECK([whether to build tests],
        nl_cv_build_tests,
        [
            AC_ARG_ENABLE(tests,
                [AS_HELP_STRING([--enable-tests],[Enable building of tests @<:@default=$1@:>@.])],
                [
                    case "${enableval}" in 

                    no|yes)
                        nl_cv_build_tests=${enableval}
                        ;;

                    *)
                        AC_MSG_ERROR([Invalid value ${enableval} for --enable-tests])
                        ;;

                    esac
                ],
                [
                    nl_cv_build_tests=$1
                ])
    ])
])
