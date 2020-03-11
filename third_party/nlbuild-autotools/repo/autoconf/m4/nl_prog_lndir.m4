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
#      This file defines a GNU autoconf M4-style macro for checking
#      for a build host-based tool that can shadow a directory using
#      symbolic links, ostensibly either GNU cp or X11 lndir.
#

AC_DEFUN([_NL_CHECK_CP_RS],
[
    $1 -Rs 2>&1 | grep 'missing file operand' > /dev/null
])

AC_DEFUN([_NL_CHECK_LNDIR],
[
    $1 -silent 2>&1 | grep 'usage: lndir' > /dev/null
])

#
# NL_PROG_LNDIR([fallback GNU cp path to test, fallback GNU cp path to set])
#
#   test path    - The fallback GNU cp path and arguments to test if a system
#                  GNU cp cannot be found.
#   set path     - The fallback GNU cp path and arguments to set to LNDIR if
#                  the test path succeeds.
#
# Determine and assign to LNDIR, a build host-based tool that can shadow
# a directory using symbolic links, attempting either GNU cp or X11 lndir
# as preferred defaults.
#
# If the host doesn't have GNU cp natively, the caller can specify
# both a GNU cp path to test and a GNU cp path to set if the test path
# was successful.
#
# ----------------------------------------------------------------------------
AC_DEFUN([NL_PROG_LNDIR],
[
    AC_ARG_VAR(LNDIR, [Program and arguments to create a shadow directory of symbolic links to another directory tree (e.g. 'cp -Rs')])

    AC_MSG_CHECKING([how to shadow a directory tree])

    if test "x${LNDIR}" = "x"; then
	if `_NL_CHECK_CP_RS(cp)`; then
	    LNDIR="cp -Rs"

	elif `_NL_CHECK_LNDIR(lndir)`; then
	    LNDIR="lndir -silent"

	elif `_NL_CHECK_CP_RS($1)`; then
	    LNDIR="$2 -Rs"

	else
	    AC_MSG_ERROR([Cannot determine how to shadow a directory tree. Neither 'cp -Rs' nor 'lndir -silent' appear to be available or functional. Please consider installing or making available in your PATH one of: GNU coreutils <http://www.gnu.org/software/coreutils/>, XQuartz (Mac OS X-only) <http://xquartz.macosforge.org/>, or lndir <http://www.mit.edu/afs/sipb/project/sipbsrc/rt/lndir/>.])

	fi
    fi

    AC_MSG_RESULT(${LNDIR})

    AC_SUBST(LNDIR)
])
