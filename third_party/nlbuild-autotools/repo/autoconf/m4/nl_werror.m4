#
#    Copyright 2015-2018 Nest Labs Inc. All Rights Reserved.
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
#      This file defines GNU autoconf M4-style macros that ensure the
#      -Werror (or -Werror=<...>) compiler option(s) for GCC-based or
#      -compatible compilers do not break some autoconf tests (see
#      http://lists.gnu.org/archive/html/autoconf-patches/2008-09/msg00014.html).
#
#      If -Werror (or -Werror=<...>) has/have been passed transform it
#      into -Wno-error (or -Wno-error=<...>) for CPPFLAGS, CFLAGS,
#      CXXFLAGS, OBJCFLAGS, and OBJCXXFLAGS with
#      NL_SAVE_WERROR. Transform them back again with
#      NL_RESTORE_WERROR.
#

# 
# _NL_SAVE_WERROR_FOR_VAR(variable)
#
#   variable - The compiler flags variable to scan for the presence of
#              -Werror (or -Werror=<...>) and, if present, transform
#              to -Wno-error (or -Wno-error=<...>).
#
# This transforms, for the specified compiler flags variable, -Werror
# (or -Werror=<...>) to -Wno-error (or -Wno-error=<...>), if it was it
# present. The original state may be restored by invoking
# _NL_RESTORE_WERROR_FOR_VAR([variable]).
#
#------------------------------------------------------------------------------
AC_DEFUN([_NL_SAVE_WERROR_FOR_VAR],
[
    if echo "${$1}" | grep -q '\-Werror'; then
	$1="`echo ${$1} | sed -e 's,-Werror\(=[[[:alnum:]_-]]\+\)*\([[[:space:]]]\),-Wno-error\1\2,g'`"
	nl_had_$1_werror=yes
    else
	nl_had_$1_werror=no
    fi
])

#
# _NL_RESTORE_WERROR_FOR_VAR(variable)
#
#   variable - The compiler flag for which to restore -Wno-error back
#              to -Werror if it was originally passed in by the user as
#              such.
#
# This restores, for the specified compiler flags variable, -Werror
# (or -Werror=<...>) from -Wno-error (or -Wno-error=<...>), if it was
# initially set as -Werror (or -Werror=<...>) at the time
# _NL_SAVE_WERROR_FOR_VAR([variable]) was invoked.
#
#------------------------------------------------------------------------------
AC_DEFUN([_NL_RESTORE_WERROR_FOR_VAR],
[
    if test "${nl_had_$1_werror}" = "yes"; then
	$1="`echo ${$1} | sed -e 's,-Wno-error\(=[[[:alnum:]_-]]\+\)*\([[[:space:]]]\),-Werror\1\2,g'`"
    fi

    unset nl_had_$1_werror
])

# 
# NL_SAVE_WERROR
#
# This transforms, for each of CFLAGS, CXXFLAGS, OBJCFLAGS, and
# OBJCXXFLAGS, -Werror (or -Werror=<...>) to -Wno-error (or
# -Wno-error=<...>), if it was it present. The original state may be
# restored by invoking NL_RESTORE_WERROR.
#
#------------------------------------------------------------------------------
AC_DEFUN([NL_SAVE_WERROR],
[
    _NL_SAVE_WERROR_FOR_VAR([CPPFLAGS])
    _NL_SAVE_WERROR_FOR_VAR([CFLAGS])
    _NL_SAVE_WERROR_FOR_VAR([CXXFLAGS])
    _NL_SAVE_WERROR_FOR_VAR([OBJCFLAGS])
    _NL_SAVE_WERROR_FOR_VAR([OBJCXXFLAGS])
])

#
# NL_RESTORE_WERROR
#
# This restores, for each of OBJCXXFLAGS, OBJCFLAGS, CXXFLAGS, and
# CFLAGS, -Werror (or -Werror=<...>) from -Wno-error (or
# -Wno-error=<...>), if it was initially set as -Werror (or
# -Werror=<...>) at the time NL_SAVE_WERROR was invoked.
#
#------------------------------------------------------------------------------
AC_DEFUN([NL_RESTORE_WERROR],
[
    _NL_RESTORE_WERROR_FOR_VAR([OBJCXXFLAGS])
    _NL_RESTORE_WERROR_FOR_VAR([OBJCFLAGS])
    _NL_RESTORE_WERROR_FOR_VAR([CXXFLAGS])
    _NL_RESTORE_WERROR_FOR_VAR([CFLAGS])
    _NL_RESTORE_WERROR_FOR_VAR([CPPFLAGS])
])
