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
#      controls whether the package will be built for code coverage
#      reporting, using the LCOV package.
#

#
# NL_ENABLE_COVERAGE_REPORTS(default)
#
#   default - Whether the option should be automatic (auto), enabled
#             (yes), or disabled (no) by default.
#
# Adds an --enable-coverage-reports configuration option to the
# package with a default value of 'default' (should be 'auto', 'no' or
# 'yes') and controls whether the package will be built with or
# without code coverage reports, using the LCOV package.
#
# The value 'nl_cv_build_coverage_reports' will be set to the result. In
# addition, LCOV will be set to the path of the 'lcov' tool and GENHTML will  be set to the path of the 'genhtml' tool.
#
# NOTE: The behavior of this is influenced by nl_cv_build_coverage from
#       NL_ENABLE_COVERAGE.
#
#------------------------------------------------------------------------------
AC_DEFUN([NL_ENABLE_COVERAGE_REPORTS],
[
    # Check whether or not a default value has been passed in.

    m4_case([$1],
        [auto],[],
        [yes],[],
        [no],[],
        [m4_fatal([$0: invalid default value '$1'; must be 'auto', 'yes' or 'no'])])

    # Check for the presence of lcov and genhtml, required
    # to build and generate the coverage reports.

    AC_PATH_PROG(LCOV, lcov)
    AC_PATH_PROG(GENHTML, genhtml)

    AC_CACHE_CHECK([whether to build graphical code coverage reports],
        nl_cv_build_coverage_reports,
        [
            AC_ARG_ENABLE(coverage-reports,
                [AS_HELP_STRING([--enable-coverage-reports],[Enable the generation of code coverage reports (requires lcov)  @<:@default=$1@:>@.])],
                [
                    case "${enableval}" in 

                    auto|no|yes)
                        nl_cv_build_coverage_reports=${enableval}
                        ;;

                    *)
                        AC_MSG_ERROR([Invalid value ${enableval} for --enable-coverage])
                        ;;

                    esac
                ],
                [
                    nl_cv_build_coverage_reports=$1
                ])

            # If coverage is not enabled, then coverage reports
            # defaults to 'no' if it is 'auto' or fails if it is
            # 'yes'. Otherwise, availability of lcov and genhtml
            # condition behavior. Lack of availability for 'yes'
            # results in failure; however, for 'auto' then coverage
            # reports default to 'no'.

            case "${nl_cv_build_coverage}" in

            no)
                case "${nl_cv_build_coverage_reports}" in

                    auto)
                        nl_cv_build_coverage_reports="no"
                        ;;

                    yes)
                        AC_MSG_ERROR([--enable-coverage must be asserted to use --enable-coverage-reports.])
                        ;;

                    no)
                        ;;

                esac
                ;;

            yes)
                case "${nl_cv_build_coverage_reports}" in

                    auto)
                        # Both lcov and genhtml must exist to successfully
                        # enable coverage reports.

                        if test "x${LCOV}" = "x" || test "x${GENHTML}" = "x"; then
                            nl_cv_build_coverage_reports="no"

                        else
                            nl_cv_build_coverage_reports="yes"

                        fi
                        ;;

                    yes)
                        # Both lcov and genhtml must exist to successfully
                        # enable coverage reports. Since the default or user
                        # ask is 'yes', we must fail if lcov or genhtml cannot
                        # be found.

                        if test "x${LCOV}" = "x"; then
                            AC_MSG_ERROR([Cannot find 'lcov'. You must have the lcov package installed to use coverage reports.])

                        elif test "x${GENHTML}" = "x"; then
                            AC_MSG_ERROR([Cannot find 'genhtml'. You must have the lcov package installed to use coverage reports.])

                        elif test "${nl_cv_build_coverage_reports}" = "auto"; then
                            nl_cv_build_coverage_reports="yes"

                        fi
                        ;;

                    no)
                        ;;

                esac
                ;;

            esac
    ])
])
