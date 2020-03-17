#
#    Copyright 2016 Nest Labs Inc. All Rights Reserved.
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
#      This file defines a number of GNU autoconf M4-style macros
#      for checking language-specific preprocessor options.
#

#
# _AX_CHECK_PREPROCESSOR_OPTION_WITH_VAR(language, variable, option)
#
#   language - The autoconf language (C, C++, Objective C, Objective C++,
#              etc.).
#   variable - The variable to add the checked preprocessor option to.
#   option   - The preprocessor flag to check.
#
# Add, if supported, the specified preprocessor flag for the preprocessor
# selected for the specified language to the provided variable.
# ----------------------------------------------------------------------------
AC_DEFUN([_AX_CHECK_PREPROCESSOR_OPTION_WITH_VAR],
[
    AC_LANG_PUSH($1)
    AC_MSG_CHECKING([whether the _AC_LANG preprocessor understands $3])
    SAVE_CPPFLAGS=${CPPFLAGS}
    SAVE_$2=${$2}
    CPPFLAGS=$3
    AC_TRY_CPP(,AC_MSG_RESULT([yes]); CPPFLAGS="${SAVE_CPPFLAGS}"; $2="${SAVE_$2} $3",AC_MSG_RESULT([no]); CPPFLAGS=${SAVE_CPPFLAGS}; $2=${SAVE_$2});
    unset SAVE_CPPFLAGS
    unset SAVE_$2
    AC_LANG_POP($1)
])

#
# _AX_CHECK_PREPROCESSOR_OPTION(language, option)
#
#   language - The autoconf language (C, C++, Objective C, Objective C++,
#              etc.).
#   option   - The preprocessor flag to check.
#
# Add, if supported, the specified preprocessor flag for the preprocessor
# selected for the specified language.
# ----------------------------------------------------------------------------
AC_DEFUN([_AX_CHECK_PREPROCESSOR_OPTION],
[
    AC_LANG_PUSH($1)
    AC_MSG_CHECKING([whether the _AC_LANG preprocessor understands $2])
    SAVE_CPPFLAGS=${CPPFLAGS}
    CPPFLAGS=$2
    AC_TRY_CPP(,AC_MSG_RESULT([yes]); CPPFLAGS="${SAVE_CPPFLAGS} $2",AC_MSG_RESULT([no]); CPPFLAGS=${SAVE_CPPFLAGS});
    unset SAVE_CPPFLAGS
    AC_LANG_POP($1)
])

#
# AX_CHECK_PREPROCESSOR_OPTION(language, [variable,] option)
#
#   language - The autoconf language (C, C++, Objective C, Objective C++,
#              etc.).
#   variable - If supplied, the variable to add the checked preprocessor option
#              to.
#   option   - The preprocessor flag to check.
#
# Add, if supported, the specified preprocessor flag for the preprocessor
# selected for the specified language, optionally saving it to the specified
# variable.
# ----------------------------------------------------------------------------
AC_DEFUN([AX_CHECK_PREPROCESSOR_OPTION],
[
    ifelse($#,
        3,
        [_AX_CHECK_PREPROCESSOR_OPTION_WITH_VAR($1, $2, $3)],
        [_AX_CHECK_PREPROCESSOR_OPTION($1, $2)])
])

#
# AX_CHECK_PREPROCESSOR_OPTIONS(language, [variable,] option ...)
#
#   language - The autoconf language (C, C++, Objective C, Objective C++,
#              etc.).
#   variable - If supplied, the variable to add the checked preprocessor option
#              to.
#   options  - The preprocessor flags to check.
#
# Add, if supported, the specified preprocessor flags for the preprocessor
# selected for the specified language, optionally saving it to the specified
# variable.
# ----------------------------------------------------------------------------
AC_DEFUN([AX_CHECK_PREPROCESSOR_OPTIONS],
[
    ifelse($#,
        3,
        [
            for ax_preprocessor_option in [$3]; do
                _AX_CHECK_PREPROCESSOR_OPTION_WITH_VAR([$1], [$2], $ax_preprocessor_option)
            done
	],
        [
            for ax_preprocessor_option in [$2]; do
                _AX_CHECK_PREPROCESSOR_OPTION([$1], $ax_preprocessor_option)
            done
	])
])
