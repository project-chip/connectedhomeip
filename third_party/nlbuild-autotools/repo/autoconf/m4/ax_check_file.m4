#
#    Copyright 2014-2016 Nest Labs Inc. All Rights Reserved.
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
#      This file implements a GNU M4 autoconf macro for checking for
#      the existence of files.
#
#      The autoconf version of AC_CHECK_FILE is absolutely broken in
#      that it cannot check for files when cross-compiling even though
#      the only thing it relies upon is a shell file readability
#      check.
#

# AX_CHECK_FILE(FILE, [ACTION-IF-FOUND], [ACTION-IF-NOT-FOUND])
# -------------------------------------------------------------
#
# Check for the existence of FILE.
AC_DEFUN([AX_CHECK_FILE],
[AS_VAR_PUSHDEF([ac_File], [ac_cv_file_$1])dnl
AC_CACHE_CHECK([for $1], [ac_File],
[if test -r "$1"; then
  AS_VAR_SET([ac_File], [yes])
else
  AS_VAR_SET([ac_File], [no])
fi])
AS_VAR_IF([ac_File], [yes], [$2], [$3])
AS_VAR_POPDEF([ac_File])dnl
])# AX_CHECK_FILE
