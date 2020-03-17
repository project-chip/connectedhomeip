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
#      the presence and viability of both required and optional
#      dependent packages, which may be internal or external to the
#      current package.
#
#      Five (5) macros are made available:
#
#        * NL_WITH_REQUIRED_EXTERNAL_PACKAGE
#        * NL_WITH_OPTIONAL_EXTERNAL_PACKAGE
#        * NL_WITH_REQUIRED_INTERNAL_PACKAGE
#        * NL_WITH_OPTIONAL_INTERNAL_PACKAGE
#        * NL_WITH_PACKAGE
#
#      Note, however, that NL_WITH_PACKAGE is deprecated and maps to
#      NL_WITH_REQUIRED_INTERNAL_PACKAGE.
#

# _NL_ARG_WITH_INCLUDES_OR_LIBS(PACKAGE-DISPOSITION,
#                               PACKAGE-SOURCE,
#                               PACKAGE-PRETTY-NAME,
#                               PACKAGE-VARIABLE-PREFIX,
#                               PACKAGE-SUCCINCT-NAME,
#                               PACKAGE-WITH-OPTION,
#                               PLURAL-WITH-OPTION-HELP-WORD,
#                               WITH-DIR-VARIABLE-MODIFIER)
# ----------------------------------------------------------------------------
# This is a wrapper around AC_ARG_WITH that provides the ability to
# optionally specify a dependent package include and link library
# directories independently as
# --with-<PACKAGE-SUCCINCT-NAME>-<PACKAGE-WITH-OPTION>=<DIR>.
#
# If the package is specified as required, the use of
# --without-<PACKAGE-SUCCINCT-NAME> or the use of
# --with-<PACKAGE-SUCCINCT-NAME>=no will result in a fatal error.
#
# At the successful conclusion of the execution of the macro, 
# two variables will be defined:
#
#   * nl_with_<PACKAGE-SUCCINCT-NAME>
#   * <PACKAGE-SUCCINT-NAME>_<WITH-DIR-VARIABLE-MODIFIER>_dir
#
# The package disposition and source are specified by:
#
#   PACKAGE-DISPOSITION     : This may be either 'required' if the dependent
#                             package is required or 'optional' if
#                             not.
#   PACKAGE-SOURCE          : This may be either 'internal' if the dependent
#                             package may be provided either
#                             internally or externally to the current
#                             package or 'external' if the dependent
#                             package may only be provided outside of
#                             the current package. This also serves as
#                             the default value for where the
#                             configuration script expects to find the
#                             package.
#
# The dependent package is specified in three forms:
#
#   PACKAGE-PRETTY-NAME     : The human-readable name by which the package
#                             will be referred for any diagnostic output. For
#                             example, "My Great Software Package".
#   PACKAGE-VARIABLE-PREFIX : The package-specific prefix applied to variables
#                             defined as a result of running this macro for the
#                             packages. For example, "MGSP" here is transformed
#                             into MGSP_CPPFLAGS.
#   PACKAGE-SUCCINCT-NAME   : The package-specific name used for pkg-config,
#                             in temporary variables and on the
#                             configure command line.  For example,
#                             "mgsp" here is used for --with-mgsp=DIR
#                             or 'pkg-config --cflags mgsp'.
# ----------------------------------------------------------------------------
AC_DEFUN([_NL_ARG_WITH_INCLUDES_OR_LIBS],
[
    dnl Check whether or not the package is required

    m4_case([$1],
        [required],[],
        [optional],[],
        [m4_fatal([$0: invalid disposition value '$1'; must be 'required' or 'optional'])])

    dnl Check whether or not the package is internal

    m4_case([$2],
        [internal],[],
        [external],[],
        [m4_fatal([$0: invalid source value '$2'; must be 'internal' or 'external'])])

    AC_ARG_WITH($5-$6,
	AS_HELP_STRING([--with-$5-$6=DIR],
		   [Specify location of $1 $3 $7 @<:@default=$2@:>@.]),
	[
	    if test "${withval}" = "no"; then
                m4_if([$1],
                      [required],
                      [AC_MSG_ERROR([${PACKAGE_NAME} requires the $3 package.])],
                      [nl_with_$5=${withval}])
	    fi
	
	    if test "x${$5_dir}" != "x"; then
		AC_MSG_WARN([overriding --with-$5=${$5_dir}])
	    fi

	    if test "${withval}" = "internal"; then
		$5_$8_dir=${withval}
		nl_with_$5=${withval}
	    else
		$5_$8_dir=${withval}
		nl_with_$5=external
	    fi
	],
	[
	    $5_$8_dir=;
	    if test "x${nl_with_$5}" = "x"; then
		nl_with_$5=$2
	    fi
	])
])

# _NL_ARG_WITH_INCLUDES(PACKAGE-DISPOSITION,
#                       PACKAGE-SOURCE,
#                       PACKAGE-PRETTY-NAME,
#                       PACKAGE-VARIABLE-PREFIX,
#                       PACKAGE-SUCCINCT-NAME)
# ----------------------------------------------------------------------------
# This is a wrapper around AC_ARG_WITH that provides the ability to
# optionally specify a dependent package include directory
# independently as --with-<PACKAGE-SUCCINCT-NAME>-includes=<DIR>.
#
# If the package is specified as required, the use of
# --without-<PACKAGE-SUCCINCT-NAME>-includes or the use of
# --with-<PACKAGE-SUCCINCT-NAME>-includes=no will result in a fatal error.
#
# At the successful conclusion of the execution of the macro, 
# two variables will be defined:
#
#   * nl_with_<PACKAGE-SUCCINCT-NAME>
#   * <PACKAGE-SUCCINT-NAME>_header_dir
#
# The package disposition and source are specified by:
#
#   PACKAGE-DISPOSITION     : This may be either 'required' if the dependent
#                             package is required or 'optional' if
#                             not.
#   PACKAGE-SOURCE          : This may be either 'internal' if the dependent
#                             package may be provided either
#                             internally or externally to the current
#                             package or 'external' if the dependent
#                             package may only be provided outside of
#                             the current package. This also serves as
#                             the default value for where the
#                             configuration script expects to find the
#                             package.
#
# The dependent package is specified in three forms:
#
#   PACKAGE-PRETTY-NAME     : The human-readable name by which the package
#                             will be referred for any diagnostic output. For
#                             example, "My Great Software Package".
#   PACKAGE-VARIABLE-PREFIX : The package-specific prefix applied to variables
#                             defined as a result of running this macro for the
#                             packages. For example, "MGSP" here is transformed
#                             into MGSP_CPPFLAGS.
#   PACKAGE-SUCCINCT-NAME   : The package-specific name used for pkg-config,
#                             in temporary variables and on the
#                             configure command line.  For example,
#                             "mgsp" here is used for --with-mgsp=DIR
#                             or 'pkg-config --cflags mgsp'.
# ----------------------------------------------------------------------------
AC_DEFUN([_NL_ARG_WITH_INCLUDES],
[
    _NL_ARG_WITH_INCLUDES_OR_LIBS([$1], [$2], [$3], [$4], [$5], [includes], [headers], [header])dnl
])

# _NL_ARG_WITH_LIBS(PACKAGE-DISPOSITION,
#                   PACKAGE-SOURCE,
#                   PACKAGE-PRETTY-NAME,
#                   PACKAGE-VARIABLE-PREFIX,
#                   PACKAGE-SUCCINCT-NAME)
# ----------------------------------------------------------------------------
# This is a wrapper around AC_ARG_WITH that provides the ability to
# optionally specify a dependent package link library directory
# independently as --with-<PACKAGE-SUCCINCT-NAME>-libs=<DIR>.
#
# If the package is specified as required, the use of
# --without-<PACKAGE-SUCCINCT-NAME>-libs or the use of
# --with-<PACKAGE-SUCCINCT-NAME>-libs=no will result in a fatal error.
#
# At the successful conclusion of the execution of the macro, 
# two variables will be defined:
#
#   * nl_with_<PACKAGE-SUCCINCT-NAME>
#   * <PACKAGE-SUCCINT-NAME>_library_dir
#
# The package disposition and source are specified by:
#
#   PACKAGE-DISPOSITION     : This may be either 'required' if the dependent
#                             package is required or 'optional' if
#                             not.
#   PACKAGE-SOURCE          : This may be either 'internal' if the dependent
#                             package may be provided either
#                             internally or externally to the current
#                             package or 'external' if the dependent
#                             package may only be provided outside of
#                             the current package. This also serves as
#                             the default value for where the
#                             configuration script expects to find the
#                             package.
#
# The dependent package is specified in three forms:
#
#   PACKAGE-PRETTY-NAME     : The human-readable name by which the package
#                             will be referred for any diagnostic output. For
#                             example, "My Great Software Package".
#   PACKAGE-VARIABLE-PREFIX : The package-specific prefix applied to variables
#                             defined as a result of running this macro for the
#                             packages. For example, "MGSP" here is transformed
#                             into MGSP_CPPFLAGS.
#   PACKAGE-SUCCINCT-NAME   : The package-specific name used for pkg-config,
#                             in temporary variables and on the
#                             configure command line.  For example,
#                             "mgsp" here is used for --with-mgsp=DIR
#                             or 'pkg-config --cflags mgsp'.
# ----------------------------------------------------------------------------
AC_DEFUN([_NL_ARG_WITH_LIBS],
[
    _NL_ARG_WITH_INCLUDES_OR_LIBS([$1], [$2], [$3], [$4], [$5], [libs], [libraries], [library])dnl
])

# _NL_ARG_WITH_PACKAGE(PACKAGE-DISPOSITION,
#                      PACKAGE-SOURCE,
#                      PACKAGE-PRETTY-NAME,
#                      PACKAGE-VARIABLE-PREFIX,
#                      PACKAGE-SUCCINCT-NAME)
# ----------------------------------------------------------------------------
# This is a wrapper around AC_ARG_WITH that provides the ability to
# optionally specify a dependent package as
# --with-<PACKAGE-SUCCINCT-NAME>=<DIR> or to independently specify the
# include and link library directories independently as
# --with-<PACKAGE-SUCCINCT-NAME>-includes=<DIR> and
# --with-<PACKAGE-SUCCINCT-NAME>-libs=<DIR>.
#
# If the package is specified as required, the use of
# --without-<PACKAGE-SUCCINCT-NAME>* or the use of
# --with-<PACKAGE-SUCCINCT-NAME>*=no will result in a fatal error.
#
# At the successful conclusion of the execution of the macro, 
# two or more variables will be defined:
#
#   * nl_with_<PACKAGE-SUCCINCT-NAME>
#   * <PACKAGE-SUCCINT-NAME>_dir
#   * <PACKAGE-SUCCINT-NAME>_header_dir
#   * <PACKAGE-SUCCINT-NAME>_library_dir
#
# The package disposition and source are specified by:
#
#   PACKAGE-DISPOSITION     : This may be either 'required' if the dependent
#                             package is required or 'optional' if
#                             not.
#   PACKAGE-SOURCE          : This may be either 'internal' if the dependent
#                             package may be provided either
#                             internally or externally to the current
#                             package or 'external' if the dependent
#                             package may only be provided outside of
#                             the current package. This also serves as
#                             the default value for where the
#                             configuration script expects to find the
#                             package.
#
# The dependent package is specified in three forms:
#
#   PACKAGE-PRETTY-NAME     : The human-readable name by which the package
#                             will be referred for any diagnostic output. For
#                             example, "My Great Software Package".
#   PACKAGE-VARIABLE-PREFIX : The package-specific prefix applied to variables
#                             defined as a result of running this macro for the
#                             packages. For example, "MGSP" here is transformed
#                             into MGSP_CPPFLAGS.
#   PACKAGE-SUCCINCT-NAME   : The package-specific name used for pkg-config,
#                             in temporary variables and on the
#                             configure command line.  For example,
#                             "mgsp" here is used for --with-mgsp=DIR
#                             or 'pkg-config --cflags mgsp'.
# ----------------------------------------------------------------------------
AC_DEFUN([_NL_ARG_WITH_PACKAGE],
[
    AC_ARG_WITH($5,
	AS_HELP_STRING([--with-$5=DIR],
		   [Specify location of the $1 $3 headers and libraries @<:@default=$2@:>@.]),
	[
	    if test "${withval}" = "no"; then
                m4_if([$1],
                      [required],
                      [AC_MSG_ERROR([${PACKAGE_NAME} requires the $3 package.])],
                      [nl_with_$5=${withval}])
	    elif test "${withval}" = "internal"; then
		$5_dir=${withval}
		nl_with_$5=${withval}
	    else
		$5_dir=${withval}
		nl_with_$5=external
	    fi
	],
	[$5_dir=; nl_with_$5=$2])

    # Allow users to specify external headers and libraries independently.

    _NL_ARG_WITH_INCLUDES([$1], [$2], [$3], [$4], [$5])dnl

    _NL_ARG_WITH_LIBS([$1], [$2], [$3], [$4], [$5])dnl
])

# _NL_WITH_PACKAGE(PACKAGE-DISPOSITION,
#                  PACKAGE-SOURCE,
#                  PACKAGE-PRETTY-NAME,
#                  PACKAGE-VARIABLE-PREFIX,
#                  PACKAGE-SUCCINCT-NAME,
#                  [DEFAULT-PACKAGE-LIBS],
#                  [ACTIONS-TO-RUN-IF-NOT-EXTERNAL],
#                  [ACTIONS-TO-RUN-IF-NOT-INTERNAL])
# ----------------------------------------------------------------------------
# This macro is used to test for the presence, with pkg-config if it
# is available, of the specified, optional or required dependent
# package. The dependent package may be provided externally or may
# exist within the current package itself.
#
# If the package is specified as required, failure to find the
# dependent package will result in a fatal error.
#
# At the successful conclusion of the execution of the macro, three
# variables will be defined:
#
#   * <PACKAGE-VARIABLE-PREFIX>_CPPFLAGS
#   * <PACKAGE-VARIABLE-PREFIX>_LDFLAGS
#   * <PACKAGE-VARIABLE-PREFIX>_LIBS
#
# In addition, the variable:
#
#   * nl_with_<PACKAGE-SUCCINCT-NAME>
#
# will unconditionally be set to the source of the package if it is to
# be used and is found; otherwise, 'no' if it is not to be used.
#
# The package disposition and source are specified by:
#
#   PACKAGE-DISPOSITION     : This may be either 'required' if the dependent
#                             package is required or 'optional' if
#                             not.
#   PACKAGE-SOURCE          : This may be either 'internal' if the dependent
#                             package may be provided either
#                             internally or externally to the current
#                             package or 'external' if the dependent
#                             package may only be provided outside of
#                             the current package. This also serves as
#                             the default value for where the
#                             configuration script expects to find the
#                             package.
#
# The dependent package is specified in three forms:
#
#   PACKAGE-PRETTY-NAME     : The human-readable name by which the package
#                             will be referred for any diagnostic output. For
#                             example, "My Great Software Package".
#   PACKAGE-VARIABLE-PREFIX : The package-specific prefix applied to variables
#                             defined as a result of running this macro for the
#                             packages. For example, "MGSP" here is transformed
#                             into MGSP_CPPFLAGS.
#   PACKAGE-SUCCINCT-NAME   : The package-specific name used for pkg-config,
#                             in temporary variables and on the
#                             configure command line.  For example,
#                             "mgsp" here is used for --with-mgsp=DIR
#                             or 'pkg-config --cflags mgsp'.
#
# In addition, if any additional, default link libraries are required
# for use with the package, these are specified as:
#
#   DEFAULT-PACKAGE-LIBS    : Default link libraries required for use with
#                             the package. These are used if pkg-config is
#                             not available or cannot identify any
#                             such libraries. For example, '-lmgsp'.
#
# If the package is specified, either internally or externally, optional
# actions are run.
#
#   ACTIONS-TO-RUN-IF-NOT-EXTERNAL : Optional actions to run if the package
#                                    is not external.
#   ACTIONS-TO-RUN-IF-NOT-INTERNAL : Optional actions to run if the package
#                                    is not internal.
# ----------------------------------------------------------------------------
AC_DEFUN([_NL_WITH_PACKAGE],
[
    # Influential external variables for the package support

    AC_ARG_VAR($4_CPPFLAGS, [$3 C preprocessor flags])
    AC_ARG_VAR($4_LDFLAGS,  [$3 linker flags])
    AC_ARG_VAR($4_LIBS,     [$3 linker libraries])

    # Allow the user to specify both external headers and libraries
    # together (or internal).

    _NL_ARG_WITH_PACKAGE([$1], [$2], [$3], [$4], [$5])dnl

    if test "${nl_with_$5}" == "no"; then
	AC_MSG_CHECKING([whether to use the $3 package])
	AC_MSG_RESULT([${nl_with_$5}])

    else
	AC_MSG_CHECKING([source of the $3 package])
	AC_MSG_RESULT([${nl_with_$5}])

	# If the user has selected or has defaulted into the internal $3
	# package, set the values appropriately. Otherwise, run through the
	# usual routine.

	if test "${nl_with_$5}" = "internal"; then
	    $7

	else
	    # We always prefer checking the values of the various '--with-$5-...' 
	    # options first to using pkg-config because the former might be used
	    # in a cross-compilation environment on a system that also contains
	    # pkg-config. In such a case, the user wants what he/she specified
	    # rather than what pkg-config indicates.

	    if test "x${$5_dir}" != "x" -o "x${$5_header_dir}" != "x" -o "x${$5_library_dir}" != "x"; then
			if test "x${$5_dir}" != "x"; then
		    if test -d "${$5_dir}"; then
			if test -d "${$5_dir}/include"; then
			    $4_CPPFLAGS="-I${$5_dir}/include"
			else
			    $4_CPPFLAGS="-I${$5_dir}"
			fi

			if test -d "${$5_dir}/lib"; then
			    $4_LDFLAGS="-L${$5_dir}/lib"
			else
			    $4_LDFLAGS="-L${$5_dir}"
			fi
		    else
			AC_MSG_ERROR([No such directory ${$5_dir}])
		    fi
		fi

		if test "x${$5_header_dir}" != "x"; then
		    if test -d "${$5_header_dir}"; then
			$4_CPPFLAGS="-I${$5_header_dir}"
		    else
			AC_MSG_ERROR([No such directory ${$5_header_dir}])
		    fi
		fi

		if test "x${$5_library_dir}" != "x"; then
		    if test -d "${$5_library_dir}"; then
			$4_LDFLAGS="-L${$5_library_dir}"
		    else
			AC_MSG_ERROR([No such directory ${$5_library_dir}])
		    fi
		fi

		$4_LIBS="${$4_LDFLAGS} $6"

	    elif test "x${PKG_CONFIG}" != "x"; then
                if ${PKG_CONFIG} --exists "$5"; then
		    $4_CPPFLAGS="`${PKG_CONFIG} --cflags $5`"
		    $4_LDFLAGS="`${PKG_CONFIG} --libs-only-L $5`"
		    $4_LIBS="`${PKG_CONFIG} --libs-only-l $5`"
                else
                    m4_if([$1],
                          [required],
                          [AC_MSG_ERROR([Cannot find the $5 package with ${PKG_CONFIG}. ${PACKAGE_NAME} requires the $5 package. Try installing the package or use the relevant --with options to configure.])],
                          [nl_with_$5="no"])
                fi

	    else
                m4_if([$1],
                      [required],
                      [AC_MSG_ERROR([Cannot find the $3 package. ${PACKAGE_NAME} requires the $3 package.])],
                      [nl_with_$5="no"])
	    fi
	fi

	AC_SUBST($4_CPPFLAGS)
	AC_SUBST($4_LDFLAGS)
	AC_SUBST($4_LIBS)

	if test "${nl_with_$5}" != "internal" -a "${nl_with_$5}" != "no"; then
	    nl_saved_CPPFLAGS="${CPPFLAGS}"
	    nl_saved_LDFLAGS="${LDFLAGS}"
	    nl_saved_LIBS="${LIBS}"

	    CPPFLAGS="${CPPFLAGS} ${$4_CPPFLAGS}"
	    LDFLAGS="${LDFLAGS} ${$4_LDFLAGS}"
	    LIBS="${LIBS} ${$4_LIBS}"

		$8

	    CPPFLAGS="${nl_saved_CPPFLAGS}"
	    LDFLAGS="${nl_saved_LDFLAGS}"
	    LIBS="${nl_saved_LIBS}"
	fi
    fi
])

# NL_WITH_REQUIRED_EXTERNAL_PACKAGE(PACKAGE-PRETTY-NAME,
#                                   PACKAGE-VARIABLE-PREFIX,
#                                   PACKAGE-SUCCINCT-NAME,
#                                   [DEFAULT-PACKAGE-LIBS],
#                                   [ACTIONS-TO-RUN])
# ----------------------------------------------------------------------------
# This macro is used to test for the presence, with pkg-config if it
# is available, of the specified, required external dependent package.
#
# Failure to find the dependent package will result in a fatal error.
#
# The dependent package is specified in three forms:
#
#   PACKAGE-PRETTY-NAME     : The human-readable name by which the package
#                             will be referred for any diagnostic output. For
#                             example, "My Great Software Package".
#   PACKAGE-VARIABLE-PREFIX : The package-specific prefix applied to variables
#                             defined as a result of running this macro for the
#                             packages. For example, "MGSP" here is transformed
#                             into MGSP_CPPFLAGS.
#   PACKAGE-SUCCINCT-NAME   : The package-specific name used for pkg-config,
#                             in temporary variables and on the
#                             configure command line.  For example,
#                             "mgsp" here is used for --with-mgsp=DIR
#                             or 'pkg-config --cflags mgsp'.
#
# In addition, if any additional, default link libraries are required
# for use with the package, these are specified as:
#
#   DEFAULT-PACKAGE-LIBS    : Default link libraries required for use with
#                             the package. These are used if pkg-config is
#                             not available or cannot identify any
#                             such libraries. For example, '-lmgsp'.
#
# If the package is specified, optional actions are run.
#
#   ACTIONS-TO-RUN-IF-FOUND : Optional actions to run if the package is found.
# ----------------------------------------------------------------------------
AC_DEFUN([NL_WITH_REQUIRED_EXTERNAL_PACKAGE],
[
    _NL_WITH_PACKAGE([required], [external], [$1], [$2], [$3], [$4], [:], [$5])dnl
])

# NL_WITH_OPTIONAL_EXTERNAL_PACKAGE(PACKAGE-PRETTY-NAME,
#                                   PACKAGE-VARIABLE-PREFIX,
#                                   PACKAGE-SUCCINCT-NAME,
#                                   [DEFAULT-PACKAGE-LIBS],
#                                   [ACTIONS-TO-RUN])
# ----------------------------------------------------------------------------
# This macro is used to test for the presence, with pkg-config if it
# is available, of the specified, optional external dependent package.
#
# Failure to find the dependent package will NOT result in a fatal error.
#
# The dependent package is specified in three forms:
#
#   PACKAGE-PRETTY-NAME     : The human-readable name by which the package
#                             will be referred for any diagnostic output. For
#                             example, "My Great Software Package".
#   PACKAGE-VARIABLE-PREFIX : The package-specific prefix applied to variables
#                             defined as a result of running this macro for the
#                             packages. For example, "MGSP" here is transformed
#                             into MGSP_CPPFLAGS.
#   PACKAGE-SUCCINCT-NAME   : The package-specific name used for pkg-config,
#                             in temporary variables and on the
#                             configure command line.  For example,
#                             "mgsp" here is used for --with-mgsp=DIR
#                             or 'pkg-config --cflags mgsp'.
#
# In addition, if any additional, default link libraries are required
# for use with the package, these are specified as:
#
#   DEFAULT-PACKAGE-LIBS    : Default link libraries required for use with
#                             the package. These are used if pkg-config is
#                             not available or cannot identify any
#                             such libraries. For example, '-lmgsp'.
#
# If the package is specified, optional actions are run.
#
#   ACTIONS-TO-RUN-IF-FOUND : Optional actions to run if the package is found.
# ----------------------------------------------------------------------------
AC_DEFUN([NL_WITH_OPTIONAL_EXTERNAL_PACKAGE],
[
    _NL_WITH_PACKAGE([optional], [external], [$1], [$2], [$3], [$4], [:], [$5])dnl
])

# NL_WITH_REQUIRED_INTERNAL_PACKAGE(PACKAGE-PRETTY-NAME,
#                                   PACKAGE-VARIABLE-PREFIX,
#                                   PACKAGE-SUCCINCT-NAME,
#                                   [DEFAULT-PACKAGE-LIBS],
#                                   [ACTIONS-TO-RUN-IF-NOT-EXTERNAL],
#                                   [ACTIONS-TO-RUN-IF-NOT-INTERNAL])
# ----------------------------------------------------------------------------
# This macro is used to test for the presence, with pkg-config if it
# is available, of the specified, required dependent package. The dependent
# package may be provided externally or may exist within the current
# package itself.
#
# Failure to find the dependent package will result in a fatal error.
#
# The dependent package is specified in three forms:
#
#   PACKAGE-PRETTY-NAME     : The human-readable name by which the package
#                             will be referred for any diagnostic output. For
#                             example, "My Great Software Package".
#   PACKAGE-VARIABLE-PREFIX : The package-specific prefix applied to variables
#                             defined as a result of running this macro for the
#                             packages. For example, "MGSP" here is transformed
#                             into MGSP_CPPFLAGS.
#   PACKAGE-SUCCINCT-NAME   : The package-specific name used for pkg-config,
#                             in temporary variables and on the
#                             configure command line.  For example,
#                             "mgsp" here is used for --with-mgsp=DIR
#                             or 'pkg-config --cflags mgsp'.
#
# In addition, if any additional, default link libraries are required
# for use with the package, these are specified as:
#
#   DEFAULT-PACKAGE-LIBS    : Default link libraries required for use with
#                             the package. These are used if pkg-config is
#                             not available or cannot identify any
#                             such libraries. For example, '-lmgsp'.
#
# If the package is specified, either internally or externally, optional
# actions are run.
#
#   ACTIONS-TO-RUN-IF-NOT-EXTERNAL : Optional actions to run if the package
#                                    is not external.
#   ACTIONS-TO-RUN-IF-NOT-INTERNAL : Optional actions to run if the package
#                                    is not internal.
# ----------------------------------------------------------------------------
AC_DEFUN([NL_WITH_REQUIRED_INTERNAL_PACKAGE],
[
    _NL_WITH_PACKAGE([required], [internal], [$1], [$2], [$3], [$4], [$5], [$6])dnl
])

# NL_WITH_OPTIONAL_INTERNAL_PACKAGE(PACKAGE-PRETTY-NAME,
#                                   PACKAGE-VARIABLE-PREFIX,
#                                   PACKAGE-SUCCINCT-NAME,
#                                   [DEFAULT-PACKAGE-LIBS],
#                                   [ACTIONS-TO-RUN-IF-NOT-EXTERNAL],
#                                   [ACTIONS-TO-RUN-IF-NOT-INTERNAL])
# ----------------------------------------------------------------------------
# This macro is used to test for the presence, with pkg-config if it
# is available, of the specified, optional dependent package. The dependent
# package may be provided externally or may exist within the current
# package itself.
#
# Failure to find the dependent package will NOT result in a fatal error.
#
# The dependent package is specified in three forms:
#
#   PACKAGE-PRETTY-NAME     : The human-readable name by which the package
#                             will be referred for any diagnostic output. For
#                             example, "My Great Software Package".
#   PACKAGE-VARIABLE-PREFIX : The package-specific prefix applied to variables
#                             defined as a result of running this macro for the
#                             packages. For example, "MGSP" here is transformed
#                             into MGSP_CPPFLAGS.
#   PACKAGE-SUCCINCT-NAME   : The package-specific name used for pkg-config,
#                             in temporary variables and on the
#                             configure command line.  For example,
#                             "mgsp" here is used for --with-mgsp=DIR
#                             or 'pkg-config --cflags mgsp'.
#
# In addition, if any additional, default link libraries are required
# for use with the package, these are specified as:
#
#   DEFAULT-PACKAGE-LIBS    : Default link libraries required for use with
#                             the package. These are used if pkg-config is
#                             not available or cannot identify any
#                             such libraries. For example, '-lmgsp'.
#
# If the package is specified, either internally or externally, optional
# actions are run.
#
#   ACTIONS-TO-RUN-IF-NOT-EXTERNAL : Optional actions to run if the package
#                                    is not external.
#   ACTIONS-TO-RUN-IF-NOT-INTERNAL : Optional actions to run if the package
#                                    is not internal.
# ----------------------------------------------------------------------------
AC_DEFUN([NL_WITH_OPTIONAL_INTERNAL_PACKAGE],
[
    _NL_WITH_PACKAGE([optional], [internal], [$1], [$2], [$3], [$4], [$5], [$6])dnl
])

# NL_WITH_PACKAGE(PACKAGE-PRETTY-NAME, PACKAGE-VARIABLE-PREFIX,
#                 PACKAGE-SUCCINCT-NAME, [DEFAULT-PACKAGE-LIBS],
#                 [ACTIONS-TO-RUN-IF-NOT-EXTERNAL],
#                 [ACTIONS-TO-RUN-IF-NOT-INTERNAL])
# ----------------------------------------------------------------------------
# This macro is used to test for the presence, with pkg-config if it
# is available, of the specified dependent package. The dependent
# package may be provided externally or may exist within the current
# package itself.
#
# The dependent package is specified in three forms:
#
#   PACKAGE-PRETTY-NAME     : The human-readable name by which the package
#                             will be referred for any diagnostic output. For
#                             example, "My Great Software Package".
#   PACKAGE-VARIABLE-PREFIX : The package-specific prefix applied to variables
#                             defined as a result of running this macro for the
#                             packages. For example, "MGSP" here is transformed
#                             into MGSP_CPPFLAGS.
#   PACKAGE-SUCCINCT-NAME   : The package-specific name used for pkg-config,
#                             in temporary variables and on the
#                             configure command line.  For example,
#                             "mgsp" here is used for --with-mgsp=DIR
#                             or 'pkg-config --cflags mgsp'.
#
# In addition, if any additional, default link libraries are required
# for use with the package, these are specified as:
#
#   DEFAULT-PACKAGE-LIBS    : Default link libraries required for use with
#                             the package. These are used if pkg-config is
#                             not available or cannot identify any
#                             such libraries. For example, '-lmgsp'.
#
# If the package is specified, either internally or externally, optional
# actions are run.
#
#   ACTIONS-TO-RUN-IF-NOT-EXTERNAL : Optional actions to run if the package
#                                    is not external.
#   ACTIONS-TO-RUN-IF-NOT-INTERNAL : Optional actions to run if the package
#                                    is not internal.
# ----------------------------------------------------------------------------
AC_DEFUN([NL_WITH_PACKAGE],
[
    m4_warn(obsolete, [$0: this macro has been deprecated. Consider using NL_WITH_REQUIRED_INTERNAL_PACKAGE instead.])

    NL_WITH_REQUIRED_INTERNAL_PACKAGE([$1], [$2], [$3], [$4], [$5], [$6])dnl
])


