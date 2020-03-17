#
#    Copyright 2019 Google LLC. All Rights Reserved.
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
#      This file defines a GNU autoconf M4-style macro for filtering
#      the autoconf canonical build, host, or target.
#
#      Mac OS X / Darwin ends up putting some versioning cruft on the
#      end of its tuples that most users of these variables rarely
#      care about.
#

#
# _NL_FILTERED_CANONICAL_SPLIT(name)
#
#   name - The existing autoconf variable to split
#
#   This splits, by CPU architecture, vendor, and OS, the filtered
#   tuples otherwise created by autotools, creating:
#
#     nl_filtered_<name>
#     nl_filtered_<name>_cpu
#     nl_filtered_<name>_os
#     nl_filtered_<name>_vendor
#
#   filtered of the versioning cruft on the vendor component that most
#   users of these variables rarely care about.
#
#   The resulting values are available both in configure.ac as well
#   as in makefiles.
#
_NL_FILTERED_CANONICAL_SPLIT(name)
AC_DEFUN([_NL_FILTERED_CANONICAL_SPLIT],
[
    case ${nl_cv_filtered_$1} in
    
    *-*-*) ;;
    *) AC_MSG_ERROR([invalid value of canonical $1]);;
    
    esac
    
    AC_SUBST([nl_filtered_$1], [$nl_cv_filtered_$1])
    
    nl_save_IFS=$IFS; IFS='-'
    set x $nl_cv_filtered_$1
    shift
    
    AC_SUBST([nl_filtered_$1_cpu], [$[1]])
    AC_SUBST([nl_filtered_$1_vendor], [$[2]])
    
    shift; shift
    [# Remember, the first character of IFS is used to create $]*,
    # except with old shells:
    nl_filtered_$1_os=$[*]
    IFS=$nl_save_IFS
    
    case nl_filtered_$$1_os in
    
    *\ *) nl_filtered_$1_os=`echo "$$1_os" | sed 's/ /-/g'`;;
    
    esac
    
    AC_SUBST([nl_filtered_$1_os])
])

#
# _NL_FILTERED_CANONICAL(name)
#
#   name - The existing autoconf variable to filter
#
#   Mac OS X / Darwin ends up putting some versioning cruft on the end
#   of its tuples that most users of these variables rarely care about.
#
#   This filters such versioning cruft from the variable 'name'
#   generated from AC_CANONICAL_<NAME> and saves it in
#   'nl_filtered_<name>'.
#
_NL_FILTERED_CANONICAL(name)
AC_DEFUN([_NL_FILTERED_CANONICAL],
[
    AC_CACHE_CHECK([filtered $1 system type],
        [nl_cv_filtered_$1],
        [nl_cv_filtered_$1=`echo ${$1} | sed -e 's/[[[:digit:].]]*$//g'`
        nl_filtered_$1=${nl_cv_filtered_$1}])

    _NL_FILTERED_CANONICAL_SPLIT($1)
])

#
# NL_FILTERED_CANONICAL_BUILD
#
#   Mac OS X / Darwin ends up putting some versioning cruft on the end
#   of its tuples that most users of these variables rarely care about.
#
#   This filters such versioning cruft from the variable 'build'
#   generated from AC_CANONICAL_BUILD and saves it in
#   'nl_filtered_build'.
#
NL_FILTERED_CANONICAL_BUILD
AC_DEFUN([NL_FILTERED_CANONICAL_BUILD],
[
    AC_REQUIRE([AC_CANONICAL_BUILD])
    _NL_FILTERED_CANONICAL(build)
])

#
# NL_FILTERED_CANONICAL_HOST
#
#   Mac OS X / Darwin ends up putting some versioning cruft on the end
#   of its tuples that most users of these variables rarely care about.
#
#   This filters such versioning cruft from the variable 'host'
#   generated from AC_CANONICAL_HOST and saves it in
#   'nl_filtered_build'.
#
NL_FILTERED_CANONICAL_HOST
AC_DEFUN([NL_FILTERED_CANONICAL_HOST],
[
    AC_REQUIRE([AC_CANONICAL_HOST])
    _NL_FILTERED_CANONICAL(host)
])

#
# NL_FILTERED_CANONICAL_TARGET
#
#   Mac OS X / Darwin ends up putting some versioning cruft on the end
#   of its tuples that most users of these variables rarely care about.
#
#   This filters such versioning cruft from the variable 'target'
#   generated from AC_CANONICAL_TARGET and saves it in
#   'nl_filtered_target'.
#
NL_FILTERED_CANONICAL_TARGET
AC_DEFUN([NL_FILTERED_CANONICAL_TARGET],
[
    AC_REQUIRE([AC_CANONICAL_TARGET])
    _NL_FILTERED_CANONICAL(target)
])
