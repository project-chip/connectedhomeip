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
#      --disable-docs configuration option to the package and controls
#      whether the package will be built with or without documentation.
#

#
# NL_ENABLE_DOCS(default, dot_default)
#
#   default     - Whether the option should be automatic (auto), enabled
#                 (yes), disabled (no) by default.
#   dot_default - Whether Doxygen should use (YES) or not use (NO)
#                 GraphViz dot.
#
# Adds an --disable-docs configuration option to the package with a
# default value of 'default' (should be 'auto', 'no' or 'yes') and
# controls whether the package will be built with or without Doxygen-based
# documentation.
#
# The value 'nl_cv_build_docs' will be set to the result. In addition:
#
#   DOXYGEN         - Will be set to the path of the Doxygen executable.
#   DOT             - Will be set to the path of the GraphViz dot
#                     executable.
#   DOXYGEN_USE_DOT - Will be set to 'NO' or 'YES' depending on whether
#                     GraphViz dot is available.
#
#------------------------------------------------------------------------------
AC_DEFUN([NL_ENABLE_DOCS],
[
    # Check whether or not the 'default' value is sane.

    m4_case([$1],
        [auto],[],
        [yes],[],
        [no],[],
        [m4_fatal([$0: invalid default value '$1'; must be 'auto', 'yes' or 'no'])])

    # Check whether or not the 'dot_default' value is sane.

    m4_case([$2],
        [YES],[],
        [NO],[],
        [m4_fatal([$0: invalid default value '$2'; must be 'YES' or 'NO'])])

    DOXYGEN_USE_DOT=$2

    AC_ARG_VAR(DOXYGEN, [Doxygen executable])
    AC_ARG_VAR(DOT,     [GraphViz 'dot' executable, which may be used, when present, to generate Doxygen class graphs])

    AC_PATH_PROG(DOXYGEN, doxygen)
    AC_PATH_PROG(DOT, dot)

    AC_CACHE_CHECK([whether to build documentation],
        nl_cv_build_docs,
        [
	    AC_ARG_ENABLE(docs,
		[AS_HELP_STRING([--disable-docs],[Enable building documentation (requires Doxygen) @<:@default=$1@:>@.])],
		[
		    case "${enableval}" in 

		    auto|no|yes)
			nl_cv_build_docs=${enableval}
			;;

		    *)
			AC_MSG_ERROR([Invalid value ${enableval} for --disable-docs])
			;;

		    esac
		],
		[nl_cv_build_docs=$1])

	    if test "x${DOXYGEN}" != "x"; then
		nl_cv_have_doxygen=yes
	    else
		nl_cv_have_doxygen=no
	    fi

	    if test "${nl_cv_build_docs}" = "auto"; then
		if test "${nl_cv_have_doxygen}" = "no"; then
		    nl_cv_build_docs=no
		else
		    nl_cv_build_docs=yes
		fi
	    fi

	    if test "${nl_cv_build_docs}" = "yes"; then
		if test "${nl_cv_have_doxygen}" = "no"; then
		    AC_MSG_ERROR([Building docs was explicitly requested but Doxygen cannot be found])
		elif test "${nl_cv_have_doxygen}" = "yes"; then
		    if test "x${DOT}" != "x"; then
			DOXYGEN_USE_DOT=YES
		    fi
		fi
	    fi
    ])

    AC_SUBST(DOXYGEN_USE_DOT)
])
