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
#      --enable-coverage configuration option to the package and
#      controls whether the package will be built for code coverage.
#

#
# NL_ENABLE_COVERAGE(default)
#
#   default - Whether the option should be enabled (yes) or disabled (no)
#             by default.
#
# Adds an --enable-coverage configuration option to the package with a
# default value of 'default' (should be either 'no' or 'yes') and controls
# whether the package will be built with or without code coverage.
#
# The value 'nl_cv_build_coverage' will be set to the result. In
# addition, NL_COVERAGE_CPPFLAGS and NL_COVERAGE_LIBS will be set
# to the appropriate values to pass to the compiler and linker,
# respectively.
#
# NOTE: This is only supported at present for GCC or GCC-compatible
#       toolchains.
#
# NOTE: The behavior of this is influenced by nl_cv_build_optimized from
#       NL_DISABLE_OPTIMIZATION
#
#------------------------------------------------------------------------------
AC_DEFUN([NL_ENABLE_COVERAGE],
[
    # Check whether or not a default value has been passed in.

    m4_case([$1],
        [yes],[],
        [no],[],
        [m4_fatal([$0: invalid default value '$1'; must be 'yes' or 'no'])])

    AC_CACHE_CHECK([whether to build code-coverage instances of programs and libraries],
        nl_cv_build_coverage,
        [
            AC_ARG_ENABLE(coverage,
                [AS_HELP_STRING([--enable-coverage],[Enable the generation of code-coverage instances @<:@default=$1@:>@.])],
                [
                    case "${enableval}" in

                    no|yes)
                        nl_cv_build_coverage=${enableval}

                        if test "${nl_cv_build_optimized}" = "yes"; then
                            AC_MSG_ERROR([both --enable-optimization and --enable-coverage cannot used. Please, choose one or the other to enable.])
                        fi
                        ;;

                    *)
                        AC_MSG_ERROR([Invalid value ${enableval} for --enable-coverage])
                        ;;

                    esac
                ],
                [
                    if test "${nl_cv_build_optimized}" = "yes"; then
                        AC_MSG_WARN([--enable-optimization was specified, coverage disabled])
                        nl_cv_build_coverage=no

                    else
                        nl_cv_build_coverage=$1

                    fi
                ])

            if test "${nl_cv_build_coverage}" = "yes"; then
                if test "${GCC}" != "yes"; then
                    AC_MSG_ERROR([GCC or a GCC-compatible toolchain is required for --enable-coverage])
                else
                    NL_COVERAGE_CPPFLAGS="--coverage"
                    if ${CC} --version | grep -q clang; then
                        NL_COVERAGE_LDFLAGS="--coverage"
                    else
                        NL_COVERAGE_LIBS="-lgcov"
                    fi
                fi
            fi
    ])
])








