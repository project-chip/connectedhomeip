#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

#
#    Copyright (c) 2020 Project CHIP Authors
#    Copyright (c) 2018-2019 Google LLC
#    Copyright (c) 2014-2018 Nest Labs, Inc.
#
#    Licensed under the Apache License, Version 2.0 (the "License");
#    you may not use this file except in compliance with the License.
#    You may obtain a copy of the License at
#
#        http://www.apache.org/licenses/LICENSE-2.0
#
#    Unless required by applicable law or agreed to in writing, software
#    distributed under the License is distributed on an "AS IS" BASIS,
#    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#    See the License for the specific language governing permissions and
#    limitations under the License.
#

#
#    Description:
#      This file is the GNU autoconf input source file for the
#      Connected Home over IP package.
#

#
# Declare autoconf version requirements
#
AC_PREREQ([2.68])

#
# Initialize autoconf for the package
#
AC_INIT([Chip],
        m4_esyscmd([third_party/nlbuild-autotools/repo/scripts/mkversion -b `cat .default-version` .]),
        [chip.support@zigbee.com],
        [chip],
        [https://zigbee.com/])

# Tell the rest of the build system the absolute path where the
# nlbuild-autotools repository is rooted at.

AC_SUBST(nlbuild_autotools_stem,[third_party/nlbuild-autotools/repo])
AC_SUBST(abs_top_nlbuild_autotools_dir,[\${abs_top_srcdir}/\${nlbuild_autotools_stem}])

#
# CHIP interface current, revision, and age versions.
#
# NOTE: At present, the CHIP SDK makes NO ABI compatibility
#       commitments. Consequently, these simply serve as documentation
#       for how the interfaces have evolved.
#
# Maintainters: Please manage these fields as follows:
#
#   Interfaces removed:    CURRENT++, AGE = 0, REVISION = 0
#   Interfaces added:      CURRENT++, AGE++,   REVISION = 0
#   No interfaces changed:                     REVISION++
#
#
AC_SUBST(LIBCHIP_VERSION_CURRENT,  [1])
AC_SUBST(LIBCHIP_VERSION_AGE,      [0])
AC_SUBST(LIBCHIP_VERSION_REVISION, [0])
AC_SUBST(LIBCHIP_VERSION_INFO,     [${LIBCHIP_VERSION_CURRENT}:${LIBCHIP_VERSION_REVISION}:${LIBCHIP_VERSION_AGE}])

#
# Check the sanity of the source directory by checking for the
# presence of a key watch file
#
AC_CONFIG_SRCDIR([src/lib/core/CHIPConfig.h])

#
# Tell autoconf where to find auxilliary build tools (e.g. config.guess,
# install-sh, missing, etc.)
#
AC_CONFIG_AUX_DIR([build/autoconf])

#
# Tell autoconf where to find auxilliary M4 macros
# Note that nlbuild-autotools has macros in two subdirs, for local vs third-party scripts
#
AC_CONFIG_MACRO_DIRS([build/autoconf/m4 autoconf/m4 third_party/nlbuild-autotools/repo/third_party/autoconf/m4 third_party/nlbuild-autotools/repo/autoconf/m4])

#
# Tell autoconf what file the package is using to aggregate C preprocessor
# defines.
#
AC_CONFIG_HEADERS([src/include/BuildConfig.h])

#
# Guards against multiple inclusion of the BuildConfig.h
#

AH_TOP([#ifndef CHIP_BUILD_CONFIG_H
#define CHIP_BUILD_CONFIG_H])

AH_BOTTOM([#endif // CHIP_BUILD_CONFIG_H])

#
# Figure out what the canonical build, host and target tuples are.
#
AC_CANONICAL_BUILD
AC_CANONICAL_HOST
AC_CANONICAL_TARGET

#
# Mac OS X / Darwin ends up putting some versioning cruft on the end of its
# tuple that we don't care about in this script. Create "clean" variables
# devoid of it.
#

NL_FILTERED_CANONICAL_BUILD
NL_FILTERED_CANONICAL_HOST
NL_FILTERED_CANONICAL_TARGET

#
# Configure automake with the desired options, indicating that this is not
# a native GNU package, that we want "silent" build rules, and that we want
# objects built in the same subdirectory as their source rather than collapsed
# together at the top-level directory.
#
# Disable silent build rules by either passing --disable-silent-rules to
# configure or passing V=1 to make
#
AM_INIT_AUTOMAKE([1.14 foreign silent-rules subdir-objects tar-pax no-define])

#
# Silent build rules requires at least automake-1.11. Employ
# techniques for not breaking earlier versions of automake.
#
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])
AM_SILENT_RULES([yes])

#
# Disable maintainer mode to enable the package to automatically rebuild
# configure, Makefile.in, etc. when the files on which they depend change (for
# example, configure.ac, Makefile.am, etc).
#
# For those that do not desire this behavior, run configure with
# `--enable-maintainer-mode` and run the top-level `bootstrap` script manually.
#
AM_MAINTAINER_MODE([disable])

#
# Check for the target style
#
CHIP_TARGET_STYLE=
CHIP_TARGET_STYLE_UNIX=0
CHIP_TARGET_STYLE_EMBEDDED=0

AC_MSG_CHECKING([for target style])

# The user may have attempted to explicitly specify the target
# style. Sanity check it or default to 'auto'.

AC_ARG_WITH(target-style,
    [AS_HELP_STRING([--with-target-style=TARGET],
        [Specify the target style from one of: auto, embedded or unix @<:@default=auto@:>@.])],
    [
        case "${with_target_style}" in

        auto|embedded|unix)
            ;;

        *)
            AC_MSG_ERROR([Invalid value ${with_target_style} for --with-target-style])
            ;;

        esac
    ],
    [with_target_style=auto])

# At this point, the target style is one of the allowed
# values. If it's 'auto' we autodetect it.

if test "${with_target_style}" = "auto"; then
    case ${target_os} in

	*cygwin*|*darwin*|*linux*|*freebsd*|*netbsd*|*openbsd*)
            with_target_style=unix
	    ;;

	*freertos*)
            with_target_style=embedded
	    ;;

	*)
	    AC_MSG_RESULT([unknown])
	    AC_MSG_ERROR([Unsupported target OS ${target_os}])
	    ;;

    esac
fi

CHIP_TARGET_STYLE=${with_target_style}

case ${with_target_style} in

    embedded)
        CHIP_TARGET_STYLE_EMBEDDED=1
        ;;

    unix)
        CHIP_TARGET_STYLE_UNIX=1
        ;;

esac

AC_MSG_RESULT(${CHIP_TARGET_STYLE})

AC_SUBST(CHIP_TARGET_STYLE)
AC_DEFINE_UNQUOTED([CHIP_TARGET_STYLE],[${CHIP_TARGET_STYLE}],[CHIP target style])

AC_SUBST(CHIP_TARGET_STYLE_EMBEDDED)
AM_CONDITIONAL([CHIP_TARGET_STYLE_EMBEDDED], [test "${CHIP_TARGET_STYLE}" = "embedded"])
AC_DEFINE_UNQUOTED([CHIP_TARGET_STYLE_EMBEDDED],[${CHIP_TARGET_STYLE_EMBEDDED}],[Define to 1 if you want to use CHIP with an embedded (i.e. non-Unix-based) system])

AC_SUBST(CHIP_TARGET_STYLE_UNIX)
AM_CONDITIONAL([CHIP_TARGET_STYLE_UNIX], [test "${CHIP_TARGET_STYLE}" = "unix"])
AC_DEFINE_UNQUOTED([CHIP_TARGET_STYLE_UNIX],[${CHIP_TARGET_STYLE_UNIX}],[Define to 1 if you want to use CHIP with a Unix-based system])

#
# Checks for build host programs
#

# If we are cross-compiling and we are on an embedded target that
# doesn't support independent, standalone executables, then all
# compiler tests that attempt to create an executable will fail. In
# such circumstances, set AC_NO_EXECUTABLES (see http://sourceware.org/
# ml/newlib/2006/msg00353.html).

AC_MSG_CHECKING([whether to disable executable checking])
if test "$cross_compiling" = yes -a "${CHIP_TARGET_STYLE}" = "embedded"; then
#    AC_NO_EXECUTABLES
    AC_MSG_RESULT([yes])
else
    AC_MSG_RESULT([no])
fi

# Passing -Werror to GCC-based or -compatible compilers breaks some
# autoconf tests (see
# http://lists.gnu.org/archive/html/autoconf-patches/2008-09/msg00014.html).
#
# If -Werror has been passed transform it into -Wno-error. We'll
# transform it back later with NL_RESTORE_WERROR.

NL_SAVE_WERROR

# Check for compilers.
#
# These should be checked BEFORE we check for and, implicitly,
# initialize libtool such that libtool knows what languages it has to
# work with.

AC_PROG_CPP
AC_PROG_CPP_WERROR

AC_PROG_CC
AC_PROG_CC_C_O

AC_PROG_CXXCPP

AC_PROG_CXX
AC_PROG_CXX_C_O

AC_PROG_OBJC
AC_PROG_OBJCXX

# Check for other compiler toolchain tools.

AC_CHECK_TOOL(AR, ar)
AC_CHECK_TOOL(RANLIB, ranlib)
AC_CHECK_TOOL(OBJCOPY, objcopy)
AC_CHECK_TOOL(STRIP, strip)

# Check for other host tools.

AC_PROG_INSTALL
AC_PROG_LN_S

AC_PATH_PROG(CLANG_FORMAT, clang-format)
AC_PATH_PROG(CMP, cmp)
AC_PATH_PROG(PERL, perl)
AC_PATH_PROG(VALGRIND, valgrind)

#
# Python
#
# At minimum, a Python interpreter is absolutely required to
# auto-generate a number of header files (e.g., ASN1OID.h).
# Additionally, Python support is used for one binding of the
# Device Manager as well as to run and orchestrate several
# unit and functional tests.
#

# In a normal "build-and-install on 'this' machine" context,
# placing Python packages in the normal pythondir and pyexecdir
# (typically
# lib/pythonPYTHON_VERSION/{dist,site}-packages/PACKAGE) makes
# sense.
#
# However, in our case, we're building and 'installing' for
# standalone distrubution to any number of machines for which we
# don't know the Python version. In those cases, override
# pythondir and pyexec dir.

case "${prefix}" in

    /opt|/usr|/usr/local|NONE)
        ;;

    *)
        am_cv_python_pythondir="\${prefix}/lib/python"
        am_cv_python_pyexecdir="\${exec_prefix}/lib/python"
        ;;

esac

# Check for Python

AM_PATH_PYTHON

# If a Python interpreter of any color cannot be found, error out.

if test "${PYTHON}" = ""; then
    AC_MSG_ERROR([could not find python: python is required to build this package.])
fi

# Check for specific Python packages

AC_MSG_CHECKING([for Python setuptools package])
if ${PYTHON} -c 'import setuptools' >/dev/null 2>&1; then
    have_python_package_setuptools=yes
else
    have_python_package_setuptools=no
fi
AC_MSG_RESULT(${have_python_package_setuptools})

AC_MSG_CHECKING([for Python wheel package])
if ${PYTHON} -c 'import wheel' >/dev/null 2>&1; then
    have_python_package_wheel=yes
else
    have_python_package_wheel=no
fi
AC_MSG_RESULT(${have_python_package_wheel})


#
# Checks for specific compiler characteristics
#

#
# Common compiler flags we would like to have.
#
#   -Wall -Wshadow -Wframe-larger-than=9472 -Wtype-limits   CC, CXX, OBJC, OBJCXX [unix, embedded]
#   -fPIC                                                   CC, CXX, OBJC, OBJCXX [unix]
#   -fno-stack-protector                                    CC, CXX, OBJC, OBJCXX [unix]
#   -fno-exceptions                                             CXX,       OBJCXX [unix]
#   -fobjc-call-cxx-cdtors                                           OBJC, OBJCXX [cocoa]
#   -fobjc-arc                                                       OBJC, OBJCXX [cocoa]
#
#

PROSPECTIVE_CFLAGS="-Wall -Wshadow -Wframe-larger-than=9472 -Wtype-limits"
PROSPECTIVE_CXXFLAGS=""
PROSPECTIVE_OBJCFLAGS="-fobjc-call-cxx-cdtors -fobjc-arc"
PROSPECTIVE_OBJCXXFLAGS=""

# Suppress clang-specific warnings about -Wnonportable-include-path.
#
# -Wnonportable-include-path warns about include paths that can be
# problematic. This creates false positives for CHIP when compiling
# the CHIP Addressing and Routing Module (CHIPARM) in which source
# directories (e.g. src/warm/...) and public include directories
# (e.g. src/include/Warm/...) have identical, and consequently
# ambiguous, naming on case-insensitive file systems.

case "`basename ${CC}`" in

    *clang)
        AX_CHECK_COMPILER_OPTIONS([C],   -Wno-nonportable-include-path)
        ;;

esac

case "`basename ${CXX}`" in

    *clang++)
        AX_CHECK_COMPILER_OPTIONS([C++], -Wno-nonportable-include-path)
        ;;

esac

# For the flags that are desired for the "unix" target style, add
# those to the set to be checked. For the "embedded" target style, the
# CHIP system integrator knows best as to what compiler flags should
# be added and will have passed them in.

if test "${CHIP_TARGET_STYLE}" = "unix"; then
    PROSPECTIVE_CFLAGS="${PROSPECTIVE_CFLAGS} -fPIC -fno-stack-protector"
    PROSPECTIVE_CXXFLAGS="${PROSPECTIVE_CXXFLAGS} -fno-exceptions"
fi

# CHIP intentionally uses derived classes that do not use an explicit
# destructor and a virtual one at that. Consequently, do not emit
# compiler warnings or errors about it when deletion is performed on
# such objects.

PROSPECTIVE_CXXFLAGS="${PROSPECTIVE_CXXFLAGS} -Wno-delete-non-virtual-dtor"

# Check whether the C++ compiler supports C++11 with GNU
# extensions. If it is supported, enable it.  Otherwise, proceed
# without error or warning.  While the CHIP code itself does not rely
# on the GNU extensions, Android build targets crash when the standard
# is set to strict C++11

AX_CXX_COMPILE_STDCXX_11(noext, optional)

AM_CONDITIONAL([HAVE_CXX11], [test "x${HAVE_CXX11}" == "x1"])

if test "x${HAVE_CXX11}" == "x1"; then
PROSPECTIVE_CXXFLAGS="${PROSPECTIVE_CXXFLAGS} -std=gnu++11"
fi

AX_CHECK_COMPILER_OPTIONS([C],   ${PROSPECTIVE_CFLAGS})
AX_CHECK_COMPILER_OPTIONS([C++], ${PROSPECTIVE_CFLAGS} ${PROSPECTIVE_CXXFLAGS})

# Check for and initialize libtool

LT_INIT

#
# Android support
#
with_android=no

AC_MSG_CHECKING([for an Android target])

        case "${target}" in

        *android*)
            with_android=yes
            ;;

        *)
            AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[]], [[
            #if !defined(__ANDROID__) || !__ANDROID__
            #error "We are not compiling with Android."
            #endif
            ]])],
            [with_android=yes],
            [with_android=no])
            ;;

        esac

AC_MSG_RESULT(${with_android})

AM_CONDITIONAL([CHIP_WITH_ANDROID], [test "${with_android}" = "yes"])

AC_SUBST(CROSS_SYSROOT, [$lt_sysroot])
#
# CHIP package-specific build customization options
#

#
# Check for logging style
#

CHIP_LOGGING_STYLE=
CHIP_LOGGING_STYLE_ANDROID=0
CHIP_LOGGING_STYLE_EXTERNAL=0
CHIP_LOGGING_STYLE_STDIO=0
CHIP_LOGGING_STYLE_STDIO_WEAK=0

AC_MSG_CHECKING([for logging style])

AC_ARG_WITH(logging-style,
    [AS_HELP_STRING([--with-logging-style=STYLE],
        [Specify the logging style from one of: auto, android, stdio, stdio-weak, or external @<:@default=auto@:>@.])],
    [
        case "${with_logging_style}" in

        android|auto|external|stdio|stdio-weak)
            ;;

        *)
            AC_MSG_ERROR([Invalid value ${with_logging_style} for --with-logging-style])
            ;;

        esac
    ],
    [with_logging_style=auto])

# At this point, the logging style is one of the allowed values. If
# it's 'auto' we autodetect it. Otherwise, sanity check the specified
# value.

if test "${with_logging_style}" = "auto"; then

    if test "${with_android}" = "yes"; then
        with_logging_style=android

    elif test "${with_target_style}" = "unix"; then
        with_logging_style=stdio

    else
        with_logging_style=external

    fi

else
    if test "${with_android}" != "yes"; then
        if test "${with_logging_style}" = "android"; then
            AC_MSG_ERROR([Android-style logging may only be specified when building for Android.])
        fi
    fi
fi

CHIP_LOGGING_STYLE=${with_logging_style}

case ${with_logging_style} in

    android)
        CHIP_LOGGING_STYLE_ANDROID=1
        ;;

    external)
        CHIP_LOGGING_STYLE_EXTERNAL=1
        ;;

    stdio)
        CHIP_LOGGING_STYLE_STDIO=1
        ;;

    stdio-weak)
        CHIP_LOGGING_STYLE_STDIO_WEAK=1
        ;;

esac

AC_MSG_RESULT(${CHIP_LOGGING_STYLE})

AC_SUBST(CHIP_LOGGING_STYLE)
AC_DEFINE_UNQUOTED([CHIP_LOGGING_STYLE],[${CHIP_LOGGING_STYLE}],[CHIP logging style])

AC_SUBST(CHIP_LOGGING_STYLE_ANDROID)
AM_CONDITIONAL([CHIP_LOGGING_STYLE_ANDROID], [test "${CHIP_LOGGING_STYLE}" = "android"])
AC_DEFINE_UNQUOTED([CHIP_LOGGING_STYLE_ANDROID],[${CHIP_LOGGING_STYLE_ANDROID}],[Define to 1 if you want to use CHIP with the Android logging system])

AC_SUBST(CHIP_LOGGING_STYLE_EXTERNAL)
AM_CONDITIONAL([CHIP_LOGGING_STYLE_EXTERNAL], [test "${CHIP_LOGGING_STYLE}" = "external"])
AC_DEFINE_UNQUOTED([CHIP_LOGGING_STYLE_EXTERNAL],[${CHIP_LOGGING_STYLE_EXTERNAL}],[Define to 1 if you want to use CHIP with an external (i.e. platform- and integrator-defined) logging system])

AC_SUBST(CHIP_LOGGING_STYLE_STDIO)
AM_CONDITIONAL([CHIP_LOGGING_STYLE_STDIO], [test "${CHIP_LOGGING_STYLE}" = "stdio"])
AC_DEFINE_UNQUOTED([CHIP_LOGGING_STYLE_STDIO],[${CHIP_LOGGING_STYLE_STDIO}],[Define to 1 if you want to use CHIP with a C Standard I/O Library-based logging system])

AC_SUBST(CHIP_LOGGING_STYLE_STDIO_WEAK)
AM_CONDITIONAL([CHIP_LOGGING_STYLE_STDIO_WEAK], [test "${CHIP_LOGGING_STYLE}" = "stdio-weak"])
AC_DEFINE_UNQUOTED([CHIP_LOGGING_STYLE_STDIO_WEAK],[${CHIP_LOGGING_STYLE_STDIO_WEAK}],[Define to 1 if you want to use CHIP with a C Standard I/O Library-based logging system,but override, at link-time an external (i.e. platform- and integrator-defined) logging system])

#
# If C Standard I/O logging is used, prepending timestamps before log
# lines may be enabled to support test automation and other debugging
# activities.
#

CHIP_LOGGING_STYLE_STDIO_WITH_TIMESTAMPS=0

AC_MSG_CHECKING([whether to enable stdio logging timestamps])
AC_ARG_ENABLE(stdio-logging-timestamps,
    [AS_HELP_STRING([--enable-stdio-logging-timestamps],[Enable timestamps in stdio- or stdio-weak- style log output @<:@default=no@:>@.])],
    [
        case "${enableval}" in

        no|yes)
            if test "${enableval}" = "yes"; then
                if test "${CHIP_LOGGING_STYLE}" != "stdio"; then
                    AC_MSG_ERROR([The CHIP logging style must be 'stdio' to use logging timestamps. Use --with-logging-style=stdio Use --with-logging-style=stdio-weak to enable this logging style.])
                else
                    CHIP_LOGGING_STYLE_STDIO_WITH_TIMESTAMPS=1
                fi
            fi

            enable_stdio_logging_timestamps=${enableval}
            ;;

        *)
            AC_MSG_ERROR([Invalid value ${enableval} for --enable-stdio-logging-timestamps])
            ;;

        esac
    ],
    [enable_stdio_logging_timestamps=no])
AC_MSG_RESULT(${enable_stdio_logging_timestamps})

AC_DEFINE_UNQUOTED([CHIP_LOGGING_STYLE_STDIO_WITH_TIMESTAMPS],[${CHIP_LOGGING_STYLE_STDIO_WITH_TIMESTAMPS}],[Define to 1 if you want to use CHIP with a C Standard I/O Library-based logging system with timestamped log entries])

#
# Cocoa support
#
AC_MSG_CHECKING([whether to build Cocoa support])
AC_ARG_ENABLE(cocoa,
    [AS_HELP_STRING([--enable-cocoa],[Enable Cocoa support, required for the iOS device manager @<:@default=no@:>@.])],
    [
        case "${enableval}" in

        no|yes)
            enable_cocoa=${enableval}
            ;;

        *)
            AC_MSG_ERROR([Invalid value ${enableval} for --enable-cocoa])
            ;;

        esac
    ],
    [enable_cocoa=no])
AC_MSG_RESULT(${enable_cocoa})

AM_CONDITIONAL([CHIP_WITH_COCOA], [test "${enable_cocoa}" = "yes"])

if test "${enable_cocoa}" = "yes" ; then

    # Check if the Objective C and Objective C++ compilers support
    # some overall options we want for those sources.

    AX_CHECK_COMPILER_OPTIONS([Objective C],   ${PROSPECTIVE_CFLAGS} ${PROSPECTIVE_OBJCFLAGS})
    AX_CHECK_COMPILER_OPTIONS([Objective C++], ${PROSPECTIVE_CFLAGS} ${PROSPECTIVE_CXXFLAGS} ${PROSPECTIVE_OBJCFLAGS} ${PROSPECTIVE_OBJCXXFLAGS})

    AC_LANG_PUSH([Objective C++])

        # Check for required Cocoa headers.

        AC_CHECK_HEADERS([Foundation/Foundation.h],
        [],
        [
            AC_MSG_ERROR(The Cocoa header "$ac_header" is required but cannot be found.)
        ])

    AC_LANG_POP([Objective C++])
fi

#
# Java support
#

# Influential external variables for Java support

AC_ARG_VAR(JAVA_HOME,    [Java SDK root directory])
AC_ARG_VAR(JNI_CPPFLAGS, [Java Native Interface (JNI) C preprocessor flags])
AC_ARG_VAR(JAVAC,        [Java class compiler command])
AC_ARG_VAR(JAR,          [Java archiver command])

AC_MSG_CHECKING([whether to build Java support])
AC_ARG_ENABLE(java,
    [AS_HELP_STRING([--disable-java],[Disable Java support, otherwise required for the Android device manager and CHIP service support @<:@default=auto@:>@.])],
    [
        case "${enableval}" in

        auto|no|yes)
            enable_java=${enableval}
            ;;

        *)
            AC_MSG_ERROR([Invalid value ${enableval} for --enable-java])
            ;;

        esac
    ],
    [enable_java=auto])

if test "${enable_java}" = "auto"; then
    if test "x${JAVA_HOME}" != "x"; then
        enable_java=yes
    else
        enable_java=no
    fi
fi

AC_MSG_RESULT(${enable_java})

# If Java support has been requested, then check for the relevant Java
# headers and tools.

if test "${enable_java}" = "yes" ; then
    # Check for Java

    # If a specific set of JNI_INCLUDE_DIRS has *not* been given, attempt
    # to locate the JNI include dirs from JAVA_HOME or the location of
    # javac.
    if test "x${JNI_INCLUDE_DIRS}" = "x" ; then
        AC_JNI_INCLUDE_DIR
    fi

    if test "x${JNI_INCLUDE_DIRS}" != "x" ; then
        for dir in ${JNI_INCLUDE_DIRS} ; do
            JNI_CPPFLAGS="${JNI_CPPFLAGS} -I${dir}"
        done

        AC_SUBST(JNI_CPPFLAGS)
    fi

    AC_PATH_PROG([JAVAC], [javac], [], [${JAVA_HOME:+${JAVA_HOME}/bin}${PATH_SEPARATOR}${PATH}])
    AC_PATH_PROG([JAR],   [jar],   [], [${JAVA_HOME:+${JAVA_HOME}/bin}${PATH_SEPARATOR}${PATH}])
fi

AM_CONDITIONAL([CHIP_WITH_JAVA], [test "${enable_java}" = "yes"])

#
# Debug and profile instances
#
AC_MSG_NOTICE([checking whether to build debug and profile instances])

# Debug

NL_ENABLE_DEBUG([no])

AM_CONDITIONAL([CHIP_BUILD_DEBUG], [test "${nl_cv_build_debug}" = "yes"])

# Profile

AC_MSG_CHECKING([whether to build profile instances of programs and libraries])
AC_ARG_ENABLE(profile,
    [AS_HELP_STRING([--disable-profile],[Disable the generation of profile instances @<:@default=yes@:>@.])],
    [
        case "${enableval}" in

        no|yes)
            build_profile=${enableval}
            ;;

        *)
            AC_MSG_ERROR([Invalid value ${enableval} for --enable-profile])
            ;;

        esac
    ],
    [build_profile=no])
AC_MSG_RESULT(${build_profile})
AM_CONDITIONAL([CHIP_BUILD_PROFILE], [test "${build_profile}" = "yes"])

#
# Code coverage and compiler optimization
#

# Coverage

NL_ENABLE_COVERAGE([no])

AM_CONDITIONAL([CHIP_BUILD_COVERAGE], [test "${nl_cv_build_coverage}" = "yes"])

NL_ENABLE_COVERAGE_REPORTS([auto])

AM_CONDITIONAL([CHIP_BUILD_COVERAGE_REPORTS], [test "${nl_cv_build_coverage_reports}" = "yes"])

# Optimization

NL_ENABLE_OPTIMIZATION([yes])

AM_CONDITIONAL([CHIP_BUILD_OPTIMIZED], [test "${nl_cv_build_optimized}" = "yes"])

#
# Treat Compiler Warnings as Errors
#
NL_ENABLE_WERROR([no])

#
# Tests and tools
#
AC_MSG_NOTICE([checking whether to build tools and tests])

# Tests

NL_ENABLE_TESTS([yes])

AM_CONDITIONAL([CHIP_BUILD_TESTS], [test "${nl_cv_build_tests}" = "yes"])

# Tools

AC_MSG_CHECKING([whether to build tools])
AC_ARG_ENABLE(tools,
    [AS_HELP_STRING([--disable-tools],[Disable building of tools @<:@default=no@:>@.])],
    [
        case "${enableval}" in

        no|yes)
            build_tools=${enableval}
            ;;

        *)
            AC_MSG_ERROR([Invalid value ${enableval} for --enable-tools])
            ;;

        esac
    ],
    [build_tools=yes])
AC_MSG_RESULT(${build_tools})
AM_CONDITIONAL([CHIP_BUILD_TOOLS], [test "${build_tools}" = "yes"])

# Happy

AC_MSG_CHECKING([checking whether to run functional tests with Happy])
AC_ARG_WITH(happy,
    [AS_HELP_STRING([--with-happy=HAPPY],
        [Run CHIP functional tests using the Happy network functional test framework: yes, no, auto, or a path to the Happy network functional test framework [default=auto].])],
    [
        case "${with_happy}" in

        no|yes)
            ;;

        *)
            configure_happy_path=${with_happy}
            happy_path="${with_happy}/lib/python${am_cv_python_version}/site-packages/"
            with_happy=yes
            ;;
        esac
    ],
    [with_happy=auto])

if test "${nl_cv_build_tests}" != "yes"; then
    with_happy=no
    happy_path="-"
else
    if test -n ${happy_path}; then
        grep "Nest Labs" ${configure_happy_path}/setup.py &> /dev/null
        if test $? = 0; then
            AC_MSG_ERROR(["You have specified a valid path, ${configure_happy_path} to a Nest Labs network functional test framework (Happy) source directory; however, it does not appear to be built and installed. Please build and install Happy from the specified path, ${configure_happy_path}, and then try again."])
        fi
        if ! test -d ${happy_path}; then
            AC_MSG_ERROR(["No Nest Labs network functional test framework (Happy) installation could be found at ${configure_happy_path}. Please provide a path to a valid Happy installation."])
        fi
    fi
fi

if test "${with_happy}" != "no"; then
    PYTHONPATH=${happy_path}:${PYTHONPATH} python -c 'import happy' 2> /dev/null
    if test $? != 0; then
        if test "${with_happy}" = "yes"; then
            if test -z ${happy_path}; then
                AC_MSG_ERROR(["The Happy network functional test framework was configured with the defaults; however, Happy could not be found in the Python default package locations. Please either install Happy in a default location or invoke 'configure' with an explicit path using '--with-happy=<path to happy>'."])
            else
                AC_MSG_ERROR(["The Happy network functional test framework was configured with the path ${happy_path}; however, Happy could neither be found in the Python default package locations and nor at ${happy_path}."])
            fi
        else
            with_happy=no
            happy_path="-"
        fi
    else
        with_happy=yes
    fi
fi

if test "${with_happy}" = "yes"; then
    HAPPY_MINIMUM_VERSION="1.1.36"
    happy_version=`PYTHONPATH=${happy_path}:${PYTHONPATH} ${PYTHON} -c 'import happy; print happy.__version__ if "__version__" in dir(happy) else "0.0.0"'`
    AX_COMPARE_VERSION([${HAPPY_MINIMUM_VERSION}],[gt],[${happy_version}],[AC_MSG_ERROR(["The Happy network functional test framework is installed with version ${happy_version}; however, Weave test apps require at least version ${HAPPY_MINIMUM_VERSION}."])],[])
fi

AC_MSG_RESULT(${with_happy})
AM_CONDITIONAL([WEAVE_RUN_HAPPY], [test "${with_happy}" = "yes"])
AC_SUBST(HAPPY_PATH,[${happy_path}])
#
# CHIPoBLE Control Path and Throughput Test  (CHIPoBLE Test)
#

AC_MSG_CHECKING([whether to build CHIPoBLE Test])
AC_ARG_ENABLE(chipoble-test,
    [AS_HELP_STRING([--enable-chipoble-test],[Enable CHIPoBLE Test @<:@default=no@:>@.])],
    [
        case "${enableval}" in

        no|yes)
            enable_chipoble_test=${enableval}
            ;;

        *)
            AC_MSG_ERROR([Invalid value ${enableval} for --enable-chipoble-test])
            ;;

        esac
    ],
    [enable_chipoble_test=no])
AC_MSG_RESULT(${enable_chipoble_test})
AM_CONDITIONAL([CHIP_ENABLE_CHIPOBLE_TEST], [test "${enable_chipoble_test}" = "yes"])
if test "${enable_chipoble_test}" = "yes"; then
    CHIP_ENABLE_CHIPOBLE_TEST=1
else
    CHIP_ENABLE_CHIPOBLE_TEST=0
fi
AC_DEFINE_UNQUOTED([CHIP_ENABLE_CHIPOBLE_TEST],[${CHIP_ENABLE_CHIPOBLE_TEST}],[Define to 1 if you want to enable CHIPoBle Control Path and Throughput Test.])
#
# Documentation
#

# Determine whether or not documentation (via Doxygen) should be built
# or not, with 'auto' as the default and establish a default support
# value for GraphViz 'dot' support.

NL_ENABLE_DOCS([auto],[NO])

AM_CONDITIONAL(CHIP_BUILD_DOCS, [test "${nl_cv_build_docs}" = "yes"])

#
# Network Technology Layer
#
AC_MSG_CHECKING([network layer])
AC_ARG_WITH(network-layer,
    [AS_HELP_STRING([--with-network-layer=LAYER],
        [Specify the target network layers from one of: ble, inet, or all @<:@default=all@:>@.])],
    [
        case "${with_network_layer}" in

        ble|inet|all)
            ;;

        *)
            AC_MSG_ERROR([Invalid value ${with_network_layer} for --with-network-layer])
            ;;

        esac
    ],
    [with_network_layer=all])
AC_MSG_RESULT(${with_network_layer})

case "${with_network_layer}" in

ble)
      CONFIG_NETWORK_LAYER_BLE=1
      CONFIG_NETWORK_LAYER_INET=0
      ;;

inet)
      CONFIG_NETWORK_LAYER_BLE=0
      CONFIG_NETWORK_LAYER_INET=1
      ;;

all)
      CONFIG_NETWORK_LAYER_BLE=1
      CONFIG_NETWORK_LAYER_INET=1
      ;;

esac

# NOTE WELL: The following logic is a transitional patch (BLE layer is currently dependent on INET layer)
if test ${CONFIG_NETWORK_LAYER_INET} = 0 && test ${CONFIG_NETWORK_LAYER_BLE} = 1; then
    with_network_layer=all
    CONFIG_NETWORK_LAYER_INET=1
    AC_MSG_NOTICE([NOTE WELL: the BLE layer currently requires the INET layer.])
fi

AC_SUBST(CONFIG_NETWORK_LAYER_BLE)
AM_CONDITIONAL([CONFIG_NETWORK_LAYER_BLE],    [test "${CONFIG_NETWORK_LAYER_BLE}" = 1])
AC_DEFINE_UNQUOTED([CONFIG_NETWORK_LAYER_BLE],[${CONFIG_NETWORK_LAYER_BLE}],[Define to 1 if you want to use CHIP with the Bluetooth Low Energy network stack.])

AC_SUBST(CONFIG_NETWORK_LAYER_INET)
AM_CONDITIONAL([CONFIG_NETWORK_LAYER_INET],    [test "${CONFIG_NETWORK_LAYER_INET}" = 1])
AC_DEFINE_UNQUOTED([CONFIG_NETWORK_LAYER_INET],[${CONFIG_NETWORK_LAYER_INET}],[Define to 1 if you want to use CHIP with the Internet Protocol network stack.])

#
# CHIP Device Layer
#
AC_MSG_CHECKING([device layer])
AC_ARG_WITH(device-layer,
    [AS_HELP_STRING([--with-device-layer=LAYER],
        [Specify the target environment for the CHIP Device Layer.  Choose one of: auto, darwin, efr32, esp32, nrf5, k32w, nrfconnect, linux, or none @<:@default=auto@:>@.])],
    [
        case "${with_device_layer}" in

        auto|darwin|efr32|esp32|nrf5|k32w|nrfconnect|linux|none)
            ;;

        *)
            AC_MSG_ERROR([Invalid value ${with_device_layer} for --with-device-layer])
            ;;

        esac
    ],
    [with_device_layer=auto])
AC_MSG_RESULT(${with_device_layer})

# Disable all device layer targets by default
CHIP_DEVICE_LAYER_TARGET_DARWIN=0
CHIP_DEVICE_LAYER_TARGET_EFR32=0
CHIP_DEVICE_LAYER_TARGET_ESP32=0
CHIP_DEVICE_LAYER_TARGET_NRF5=0
CHIP_DEVICE_LAYER_TARGET_K32W=0
CHIP_DEVICE_LAYER_TARGET_LINUX=0
CHIP_DEVICE_LAYER_TARGET_NRFCONNECT=0

if test "${with_device_layer}" = "auto"; then
    case ${target_os} in

	*darwin*)
            with_device_layer=darwin
	    ;;

	*linux*)
            with_device_layer=linux
	    ;;

	*)
            with_device_layer=none
	    ;;

    esac
fi

case "${with_device_layer}" in

darwin)
      CONFIG_DEVICE_LAYER=1
      CHIP_DEVICE_LAYER_TARGET=Darwin
      CHIP_DEVICE_LAYER_TARGET_DARWIN=1
      ;;

efr32)
      CONFIG_DEVICE_LAYER=1
      CHIP_DEVICE_LAYER_TARGET=EFR32
      CHIP_DEVICE_LAYER_TARGET_EFR32=1
      ;;

esp32)
      CONFIG_DEVICE_LAYER=1
      CHIP_DEVICE_LAYER_TARGET=ESP32
      CHIP_DEVICE_LAYER_TARGET_ESP32=1
      ;;

nrf5)
      CONFIG_DEVICE_LAYER=1
      CHIP_DEVICE_LAYER_TARGET=nRF5
      CHIP_DEVICE_LAYER_TARGET_NRF5=1
      CFLAGS="${CFLAGS} -mcpu=cortex-m4 -mfloat-abi=hard -mfpu=fpv4-sp-d16 -mthumb -mabi=aapcs"
      CXXFLAGS="${CXXFLAGS} -mcpu=cortex-m4 -mfloat-abi=hard -mfpu=fpv4-sp-d16 -mthumb -mabi=aapcs"
      ;;
      
       
k32w)
      CONFIG_DEVICE_LAYER=1
      CHIP_DEVICE_LAYER_TARGET=K32W
      CHIP_DEVICE_LAYER_TARGET_K32W=1
      CFLAGS="${CFLAGS} -mcpu=cortex-m4 -mfloat-abi=soft -mthumb"
      CXXFLAGS="${CXXFLAGS} -mcpu=cortex-m4 -mfloat-abi=soft -mthumb"
      ;;

nrfconnect)
      CONFIG_DEVICE_LAYER=1
      CHIP_DEVICE_LAYER_TARGET=nrfconnect
      CHIP_DEVICE_LAYER_TARGET_NRFCONNECT=1
      ;;

linux)
      CONFIG_DEVICE_LAYER=1
      CHIP_DEVICE_LAYER_TARGET=Linux
      CHIP_DEVICE_LAYER_TARGET_LINUX=1
      ;;

none)
      CONFIG_DEVICE_LAYER=0
      CHIP_DEVICE_LAYER_TARGET=NONE
      ;;
esac

AC_SUBST(CONFIG_DEVICE_LAYER)
AM_CONDITIONAL([CONFIG_DEVICE_LAYER],    [test "${CONFIG_DEVICE_LAYER}" = 1])
AC_DEFINE_UNQUOTED([CONFIG_DEVICE_LAYER],[${CONFIG_DEVICE_LAYER}],[Define to 1 if you want to use the CHIP Device Layer.])

AC_SUBST(CHIP_DEVICE_LAYER_TARGET)
AC_DEFINE_UNQUOTED([CHIP_DEVICE_LAYER_TARGET],[${CHIP_DEVICE_LAYER_TARGET}],[Target platform name for CHIP Device Layer.])

AC_SUBST(CHIP_DEVICE_LAYER_TARGET_DARWIN)
AM_CONDITIONAL([CHIP_DEVICE_LAYER_TARGET_DARWIN],    [test "${CHIP_DEVICE_LAYER_TARGET_DARWIN}" = 1])
AC_DEFINE_UNQUOTED([CHIP_DEVICE_LAYER_TARGET_DARWIN],[${CHIP_DEVICE_LAYER_TARGET_DARWIN}],[Define to 1 if you want to build the CHIP Device Layer for Darwin platforms.])

AC_SUBST(CHIP_DEVICE_LAYER_TARGET_EFR32)
AM_CONDITIONAL([CHIP_DEVICE_LAYER_TARGET_EFR32],    [test "${CHIP_DEVICE_LAYER_TARGET_EFR32}" = 1])
AC_DEFINE_UNQUOTED([CHIP_DEVICE_LAYER_TARGET_EFR32],[${CHIP_DEVICE_LAYER_TARGET_EFR32}],[Define to 1 if you want to build the CHIP Device Layer for Silicon Labs EFR32 platforms.])

AC_SUBST(CHIP_DEVICE_LAYER_TARGET_ESP32)
AM_CONDITIONAL([CHIP_DEVICE_LAYER_TARGET_ESP32],    [test "${CHIP_DEVICE_LAYER_TARGET_ESP32}" = 1])
AC_DEFINE_UNQUOTED([CHIP_DEVICE_LAYER_TARGET_ESP32],[${CHIP_DEVICE_LAYER_TARGET_ESP32}],[Define to 1 if you want to build the CHIP Device Layer for the Espressif ESP32.])

AC_SUBST(CHIP_DEVICE_LAYER_TARGET_NRF5)
AM_CONDITIONAL([CHIP_DEVICE_LAYER_TARGET_NRF5],    [test "${CHIP_DEVICE_LAYER_TARGET_NRF5}" = 1])
AC_DEFINE_UNQUOTED([CHIP_DEVICE_LAYER_TARGET_NRF5],[${CHIP_DEVICE_LAYER_TARGET_NRF5}],[Define to 1 if you want to build the CHIP Device Layer for Nordic nRF5* platforms.])

AC_SUBST(CHIP_DEVICE_LAYER_TARGET_K32W)
AM_CONDITIONAL([CHIP_DEVICE_LAYER_TARGET_K32W],    [test "${CHIP_DEVICE_LAYER_TARGET_K32W}" = 1])
AC_DEFINE_UNQUOTED([CHIP_DEVICE_LAYER_TARGET_K32W],[${CHIP_DEVICE_LAYER_TARGET_K32W}],[Define to 1 if you want to build the CHIP Device Layer for NXP K32W platforms.])

AC_SUBST(CHIP_DEVICE_LAYER_TARGET_NRFCONNECT)
AM_CONDITIONAL([CHIP_DEVICE_LAYER_TARGET_NRFCONNECT],    [test "${CHIP_DEVICE_LAYER_TARGET_NRFCONNECT}" = 1])
AC_DEFINE_UNQUOTED([CHIP_DEVICE_LAYER_TARGET_NRFCONNECT],[${CHIP_DEVICE_LAYER_TARGET_NRFCONNECT}],[Define to 1 if you want to build the CHIP Device Layer for Nordic nRF Connect platforms.])

AC_SUBST(CHIP_DEVICE_LAYER_TARGET_LINUX)
AM_CONDITIONAL([CHIP_DEVICE_LAYER_TARGET_LINUX],    [test "${CHIP_DEVICE_LAYER_TARGET_LINUX}" = 1])
AC_DEFINE_UNQUOTED([CHIP_DEVICE_LAYER_TARGET_LINUX],[${CHIP_DEVICE_LAYER_TARGET_LINUX}],[Define to 1 if you want to build the CHIP Device Layer for Linux platforms.])

if test "${CONFIG_DEVICE_LAYER}" = 1; then
    AC_DEFINE_UNQUOTED([BLE_PLATFORM_CONFIG_INCLUDE],
                       [<platform/${CHIP_DEVICE_LAYER_TARGET}/BlePlatformConfig.h>],
                       [Path to BLE platform config header file])
    AC_DEFINE_UNQUOTED([INET_PLATFORM_CONFIG_INCLUDE],
                       [<platform/${CHIP_DEVICE_LAYER_TARGET}/InetPlatformConfig.h>],
                       [Path to Inet Layer platform config header file])
    AC_DEFINE_UNQUOTED([SYSTEM_PLATFORM_CONFIG_INCLUDE],
                       [<platform/${CHIP_DEVICE_LAYER_TARGET}/SystemPlatformConfig.h>],
                       [Path to System Layer platform config header file])
    AC_DEFINE_UNQUOTED([WARM_PLATFORM_CONFIG_INCLUDE],
                       [<platform/${CHIP_DEVICE_LAYER_TARGET}/WarmPlatformConfig.h>],
                       [Path to WARM platform config header file])
    AC_DEFINE_UNQUOTED([CHIP_PLATFORM_CONFIG_INCLUDE],
                       [<platform/${CHIP_DEVICE_LAYER_TARGET}/CHIPPlatformConfig.h>],
                       [Path to CHIP platform config header file])
    AC_DEFINE_UNQUOTED([CHIP_DEVICE_PLATFORM_CONFIG_INCLUDE],
                       [<platform/${CHIP_DEVICE_LAYER_TARGET}/CHIPDevicePlatformConfig.h>],
                       [Path to CHIP Device Layer platform config header file])
fi

if test ${CHIP_DEVICE_LAYER_TARGET_DARWIN} = 1 && test ${CONFIG_NETWORK_LAYER_BLE} = 1; then
    AC_CHECK_HEADER([CoreBluetooth/CoreBluetooth.h])
    LDFLAGS="${LDFLAGS} -framework Foundation -framework CoreBluetooth"
fi

#
# CHIP target network stack(s)
#
CONFIG_TARGET_NETWORKS=sockets

AC_MSG_CHECKING([target network])
AC_ARG_WITH(target-network,
    [AS_HELP_STRING([--with-target-network=NETWORK],
        [Specify the target network stack from one or more of, separated by commas: sockets, lwip or Network.framework @<:@default=sockets@:>@.])],
    [
        CONFIG_TARGET_NETWORKS=`echo ${with_target_network} | sed -e 's/,/ /g'`

        for target_network in ${CONFIG_TARGET_NETWORKS}; do
            case "${target_network}" in

            lwip|sockets|Network.framework)
                ;;

            *)
                AC_MSG_ERROR([Invalid value ${with_target_network} for --with-target-network])
                ;;

            esac
        done
    ],
    [with_target_network=${CONFIG_TARGET_NETWORKS}])
AC_MSG_RESULT(${with_target_network})
AC_SUBST(CONFIG_TARGET_NETWORKS)
AC_DEFINE_UNQUOTED([CONFIG_TARGET_NETWORKS], "${CONFIG_TARGET_NETWORKS}", [CHIP target network stack(s)])

CHIP_SYSTEM_CONFIG_USE_LWIP=0
CHIP_SYSTEM_CONFIG_USE_SOCKETS=0
CHIP_SYSTEM_CONFIG_USE_NETWORK_FRAMEWORK=0

for target_network in ${CONFIG_TARGET_NETWORKS}; do
    case ${target_network} in

    lwip)
        CHIP_SYSTEM_CONFIG_USE_LWIP=1
        ;;

    sockets)
        CHIP_SYSTEM_CONFIG_USE_SOCKETS=1
        ;;

    Network.framework)
        CHIP_SYSTEM_CONFIG_USE_NETWORK_FRAMEWORK=1
        ;;

    esac
done

AC_SUBST(CHIP_SYSTEM_CONFIG_USE_LWIP)
AM_CONDITIONAL([CHIP_SYSTEM_CONFIG_USE_LWIP], [test "${CHIP_SYSTEM_CONFIG_USE_LWIP}" = 1])
AC_DEFINE_UNQUOTED([CHIP_SYSTEM_CONFIG_USE_LWIP], [${CHIP_SYSTEM_CONFIG_USE_LWIP}],
    [Define to 1 if you want to use LwIP with CHIP System Layer.])

AC_SUBST(CHIP_SYSTEM_CONFIG_USE_SOCKETS)
AM_CONDITIONAL([CHIP_SYSTEM_CONFIG_USE_SOCKETS], [test "${CHIP_SYSTEM_CONFIG_USE_SOCKETS}" = 1])
AC_DEFINE_UNQUOTED([CHIP_SYSTEM_CONFIG_USE_SOCKETS], [${CHIP_SYSTEM_CONFIG_USE_SOCKETS}],
    [Define to 1 if you want to use BSD sockets with CHIP System Layer.])

AC_SUBST(CHIP_SYSTEM_CONFIG_USE_NETWORK_FRAMEWORK)
AM_CONDITIONAL([CHIP_SYSTEM_CONFIG_USE_NETWORK_FRAMEWORK], [test "${CHIP_SYSTEM_CONFIG_USE_NETWORK_FRAMEWORK}" = 1])
AC_DEFINE_UNQUOTED([CHIP_SYSTEM_CONFIG_USE_NETWORK_FRAMEWORK], [${CHIP_SYSTEM_CONFIG_USE_NETWORK_FRAMEWORK}],
    [Define to 1 if you want to use Network.framework with CHIP System Layer.])

#
#
# Internet Protocol Network Endpoints
#
if test ${CONFIG_NETWORK_LAYER_INET} = 1; then
    AC_MSG_CHECKING([IPv4 enabled])
    AC_ARG_ENABLE(ipv4,
        [AS_HELP_STRING([--disable-ipv4],[Disable the inclusion of IPv4 networking @<:@default=yes@:>@.])],
        [
            case "${enableval}" in

            no|yes)
                enable_ivp4=${enableval}
                ;;

            *)
                AC_MSG_ERROR([Invalid value ${enableval} for --enable-ivp4])
                ;;

            esac
        ],
        [enable_ivp4=yes])
    AC_MSG_RESULT(${enable_ivp4})

    INET_WANT_IPV4=1

    if test "${enable_ipv4}" = no; then
        INET_WANT_IPV4=0
    fi

    AM_CONDITIONAL([INET_WANT_IPV4], [test "${INET_WANT_IPV4}" = 1])
    AC_DEFINE_UNQUOTED([INET_WANT_IPV4],[${INET_WANT_IPV4}], [Define to 1 if you want to use CHIP with IPv4.])

    if test "${INET_WANT_IPV4}" = 1; then
        AC_DEFINE(INET_CONFIG_ENABLE_IPV4, 1, [Define to 1 for enabling IPv4])
    fi

    AC_MSG_CHECKING([inet network endpoint])
    AC_ARG_WITH(inet-endpoint,
        [AS_HELP_STRING([--with-inet-endpoint=ENDPOINT],
            [Specify the inet network endpoint from one or more of: raw, tcp, tun, udp @<:@default=all@:>@.])],
        [
            INET_ENDPOINTS=`echo ${with_inet_endpoint} | sed -e 's/,/ /g'`

            for inet_endpoint in ${INET_ENDPOINTS}; do

                 case "${inet_endpoint}" in

                 dns|raw|tcp|tun|udp|all)
                     ;;

                 *)
                     AC_MSG_ERROR([Invalid value ${with_inet_endpoint} for --with-inet-endpoint])
                     ;;

                 esac

            done
        ],
        [INET_ENDPOINTS=all])
    AC_MSG_RESULT(${with_inet_endpoint})

    AC_SUBST(INET_ENDPOINTS)
    AC_DEFINE_UNQUOTED([INET_ENDPOINTS], "${INET_ENDPOINTS}", [CHIP target inet network endpoint(s)])

    INET_WANT_ENDPOINT_DNS=0
    INET_WANT_ENDPOINT_RAW=0
    INET_WANT_ENDPOINT_TCP=0
    INET_WANT_ENDPOINT_TUN=0
    INET_WANT_ENDPOINT_UDP=0

    for wanted_endpoint in ${INET_ENDPOINTS}; do

        case "${wanted_endpoint}" in

        dns)
             INET_WANT_ENDPOINT_DNS=1
             ;;

        raw)
             INET_WANT_ENDPOINT_RAW=1
             ;;

        tcp)
             INET_WANT_ENDPOINT_TCP=1
             ;;

        tun)
             INET_WANT_ENDPOINT_TUN=1
             ;;

        udp)
             INET_WANT_ENDPOINT_UDP=1
             ;;

        all)
             INET_WANT_ENDPOINT_DNS=1
             INET_WANT_ENDPOINT_RAW=1
             INET_WANT_ENDPOINT_TCP=1
             INET_WANT_ENDPOINT_TUN=1
             INET_WANT_ENDPOINT_UDP=1
             ;;
        esac

    done

    if test "${INET_WANT_ENDPOINT_TUN}" = 1; then
        AC_CHECK_HEADERS(linux/if_tun.h,[AC_SUBST(HAVE_LINUX_IF_TUN_H, [1])], [], [])
	    AC_CHECK_TYPE(struct rtentry, , AC_CHECK_HEADERS(linux/route.h, [AC_SUBST(HAVE_LINUX_ROUTE_H, [1])], [], []), [[#include <net/route.h>]])
	    AC_CHECK_TYPE(struct in6_rtmsg, , AC_CHECK_HEADERS(linux/ipv6_route.h, [AC_SUBST(HAVE_LINUX_IPV6_ROUTE_H, [1])], [], []), [[#include <net/route.h>]])
        # Check whether linux/if_tun.h defines TUNGETIFF.
        AC_MSG_CHECKING([whether linux/if_tun.h declares TUNGETIFF])
        AC_COMPILE_IFELSE([
            AC_LANG_PROGRAM(
        [[
        #if HAVE_LINUX_IF_TUN_H
        # include <linux/if_tun.h>
        #endif
        ]],
        [[
        #if !defined(TUNGETIFF)
        #error "TUNGETIFF is not defined"
        #endif
        ]])],
        [
        AC_MSG_RESULT([yes])
        AC_DEFINE(HAVE_TUNGETIFF, 1, [Define to 1 if <linux/if_tun.h> header file defines the TUNGETIFF ioctl command.])
        ],
        [
        AC_MSG_RESULT([no])
        ])
        AC_CHECK_TYPES(struct in6_rtmsg, [AC_SUBST(HAVE_IN6_RTMSG, [1])],[],[#include<net/route.h>])
        if [[[ "${CHIP_SYSTEM_CONFIG_USE_LWIP}" -eq 0 ]]] &&
                [[[ "${HAVE_LINUX_IF_TUN_H}" -eq 0 || "${HAVE_IN6_RTMSG}" -eq 0 ]]]; then
            AC_MSG_NOTICE([Tun EndPoint cannot be enabled for current target
                CHIP_SYSTEM_CONFIG_USE_LWIP=$CHIP_SYSTEM_CONFIG_USE_LWIP, HAVE_LINUX_IF_TUN_H=$HAVE_LINUX_IF_TUN_H,
                HAVE_IN6_RTMSG=$HAVE_IN6_RTMSG])
            INET_WANT_ENDPOINT_TUN=0
        fi
    fi

    AM_CONDITIONAL([INET_WANT_ENDPOINT_DNS],    [test "${INET_WANT_ENDPOINT_DNS}" = 1])
    AC_DEFINE_UNQUOTED([INET_WANT_ENDPOINT_DNS],[${INET_WANT_ENDPOINT_DNS}],
                       [Define to 1 if you want to use CHIP with the inet dns resolver.])

    AM_CONDITIONAL([INET_WANT_ENDPOINT_RAW],    [test "${INET_WANT_ENDPOINT_RAW}" = 1])
    AC_DEFINE_UNQUOTED([INET_WANT_ENDPOINT_RAW],[${INET_WANT_ENDPOINT_RAW}],
                       [Define to 1 if you want to use CHIP with the inet raw endpoint.])

    AM_CONDITIONAL([INET_WANT_ENDPOINT_TCP],    [test "${INET_WANT_ENDPOINT_TCP}" = 1])
    AC_DEFINE_UNQUOTED([INET_WANT_ENDPOINT_TCP],[${INET_WANT_ENDPOINT_TCP}],
                       [Define to 1 if you want to use CHIP with the inet tcp endpoint.])

    AM_CONDITIONAL([INET_WANT_ENDPOINT_TUN],    [test "${INET_WANT_ENDPOINT_TUN}" = 1])
    AC_DEFINE_UNQUOTED([INET_WANT_ENDPOINT_TUN],[${INET_WANT_ENDPOINT_TUN}],
                       [Define to 1 if you want to use CHIP with the inet tun endpoint.])

    AM_CONDITIONAL([INET_WANT_ENDPOINT_UDP],    [test "${INET_WANT_ENDPOINT_UDP}" = 1])
    AC_DEFINE_UNQUOTED([INET_WANT_ENDPOINT_UDP],[${INET_WANT_ENDPOINT_UDP}],
                       [Define to 1 if you want to use CHIP with the inet udp endpoint.])

    if test "${INET_WANT_ENDPOINT_DNS}" = 1; then
        AC_DEFINE(INET_CONFIG_ENABLE_DNS_RESOLVER, 1, [Define to 1 for enabling DNS Resolver])
    fi

    if test "${INET_WANT_ENDPOINT_RAW}" = 1; then
        AC_DEFINE(INET_CONFIG_ENABLE_RAW_ENDPOINT, 1, [Define to 1 for enabling Raw Endpoint])
    fi

    if test "${INET_WANT_ENDPOINT_TCP}" = 1; then
        AC_DEFINE(INET_CONFIG_ENABLE_TCP_ENDPOINT, 1, [Define to 1 for enabling TCP Endpoint])
    fi

    if test "${INET_WANT_ENDPOINT_TUN}" = 1; then
        AC_DEFINE(INET_CONFIG_ENABLE_TUN_ENDPOINT, 1, [Define to 1 for enabling TUN Endpoint (only for LWIP and Linux w/ support)])
    fi

    if test "${INET_WANT_ENDPOINT_UDP}" = 1; then
        AC_DEFINE(INET_CONFIG_ENABLE_UDP_ENDPOINT, 1, [Define to 1 for enabling UDP Endpoint])
    fi
fi

# Asynchronous DNS
AC_MSG_CHECKING([whether to build with asynchronous DNS resolution support])
AC_ARG_ENABLE(adns,
    [AS_HELP_STRING([--disable-adns],[Disable building of adns @<:@default=no@:>@.])],
    [
        case "${enableval}" in

        no|yes)
            build_adns=${enableval}
            ;;

        *)
            AC_MSG_ERROR([Invalid value ${enableval} for --disable-adns])
            ;;

        esac
    ],
    [build_adns=yes])
AC_MSG_RESULT(${build_adns})

AM_CONDITIONAL([INET_CONFIG_ENABLE_ASYNC_DNS_SOCKETS], [test "${build_adns}" = "yes"])

if test ${build_adns} = "yes" && test ${INET_WANT_ENDPOINT_DNS} = 1 && test ${CHIP_SYSTEM_CONFIG_USE_SOCKETS} = 1; then
    AC_DEFINE(INET_CONFIG_ENABLE_ASYNC_DNS_SOCKETS, 1, [Define to 1 for enabling ASYNC DNS])
else
    AC_DEFINE(INET_CONFIG_ENABLE_ASYNC_DNS_SOCKETS, 0, [Define to 0 for disabling ASYNC DNS])
fi

#
# Options for the user to specify project-specific configuration files
# for various CHIP components.
#

AC_LANG_PUSH([C++])

# SystemLayer

CHIP_CHECK_PROJECT_CONFIG_INCLUDES(chip-system-project-includes, SYSTEM_PROJECT_CONFIG_INCLUDE, SystemProjectConfig.h, CHIP System Layer, ${ac_abs_confdir}/config/standalone)

# BleLayer

if test ${CONFIG_NETWORK_LAYER_BLE} = 1; then
    CHIP_CHECK_PROJECT_CONFIG_INCLUDES(chip-ble-project-includes, BLE_PROJECT_CONFIG_INCLUDE, BleProjectConfig.h, CHIP BLE Layer, ${ac_abs_confdir}/config/standalone)
fi

# InetLayer

if test ${CONFIG_NETWORK_LAYER_INET} = 1; then
    CHIP_CHECK_PROJECT_CONFIG_INCLUDES(chip-inet-project-includes, INET_PROJECT_CONFIG_INCLUDE, InetProjectConfig.h, CHIP Inet Layer, ${ac_abs_confdir}/config/standalone)
fi

# Core

CHIP_CHECK_PROJECT_CONFIG_INCLUDES(chip-project-includes, CHIP_PROJECT_CONFIG_INCLUDE, CHIPProjectConfig.h, CHIP Core, ${ac_abs_confdir}/config/standalone)

# Device Layer

if test "${CONFIG_DEVICE_LAYER}" = 1; then
    CHIP_CHECK_PROJECT_CONFIG_INCLUDES(chip-device-project-includes, CHIP_DEVICE_PROJECT_CONFIG_INCLUDE, CHIPDeviceProjectConfig.h, CHIP Device Layer,)
fi

AC_LANG_POP([C++])

#
# Check for crypto implementation
#
CHIP_CRYPTO=
CHIP_CRYPTO_MBEDTLS=0
CHIP_CRYPTO_OPENSSL=0

AC_MSG_CHECKING([for crypto implementation])

# The user may have attempted to explicitly specify the crypto
# implementation. Sanity check it or default to 'auto'.

AC_ARG_WITH(crypto,
    [AS_HELP_STRING([--with-crypto=CRYPTO],
        [Specify the crypto implementation from one of: auto, mbedtls, or openssl @<:@default=auto@:>@.])],
    [
        case "${with_crypto}" in
        auto|mbedtls|openssl)
            ;;
        *)
            AC_MSG_ERROR([Invalid value ${with_crypto} for --with-crypto])
            ;;
        esac
    ],
    [with_crypto=auto])

# At this point, the crypto implementation is one of the allowed
# values. If it's 'auto' we autodetect it.

if test "${with_crypto}" = "auto"; then
    case ${target_os} in

    *cygwin*|*darwin*|*linux*|*freebsd*|*netbsd*|*openbsd*)
            with_crypto=openssl
        ;;

    *freertos*)
            with_crypto=mbedtls
        ;;

    *)
        AC_MSG_RESULT([unknown])
        AC_MSG_ERROR([Unsupported target OS ${target_os}])
        ;;

    esac
fi

CHIP_CRYPTO=${with_crypto}

case ${with_crypto} in

    mbedtls)
        CHIP_CRYPTO_MBEDTLS=1
        ;;

    openssl)
        CHIP_CRYPTO_OPENSSL=1
        ;;

esac

AC_MSG_RESULT(${CHIP_CRYPTO})

AC_SUBST(CHIP_CRYPTO)
AC_DEFINE_UNQUOTED([CHIP_CRYPTO],[${CHIP_CRYPTO}],[CHIP crypto implementation])

AC_SUBST(CHIP_CRYPTO_MBEDTLS)
AM_CONDITIONAL([CHIP_CRYPTO_MBEDTLS], [test "${CHIP_CRYPTO}" = "mbedtls"])
AC_DEFINE_UNQUOTED([CHIP_CRYPTO_MBEDTLS],[${CHIP_CRYPTO_MBEDTLS}],[Define to 1 if you want to use CHIP with a mbedTLS crypto implementation])

AC_SUBST(CHIP_CRYPTO_OPENSSL)
AM_CONDITIONAL([CHIP_CRYPTO_OPENSSL], [test "${CHIP_CRYPTO}" = "openssl"])
AC_DEFINE_UNQUOTED([CHIP_CRYPTO_OPENSSL],[${CHIP_CRYPTO_OPENSSL}],[Define to 1 if you want to use CHIP with an OpenSSL crypto implementation])

#
# Checks for libraries and packages.
#
# At minimum, the following are required:
#
#   * nlassert
#   * nlio
#
# The following are optional, depending on configuration:
#
#   * lwip
#   * mbedtls
#   * nlunit-test
#   * nlfaultinjection
#   * openssl
#
# Most of these are supplied "in package"; however, they may be also
# supplied out of package.
#
AC_MSG_NOTICE([checking package dependencies])

# Check if the build host has pkg-config

AC_PATH_PROG([PKG_CONFIG],[pkg-config])

#
# Chip over Ble over Bluez Peripheral support
#

# Only a concern when the BLE layer is enabled.

CONFIG_BLE_PLATFORM_BLUEZ=0
enable_chipoble_bluez="no"

NL_WITH_OPTIONAL_INTERNAL_PACKAGE(
  [BlueZ],
  [BLUEZ],
  [bluez],
  [-lgdbus-internal -lshared-glib],
  [
  # actions if not external
  # At this time, enable this only for linux, and only if the tests are enabled
      if test "${nl_cv_build_tests}" = "yes" && test "${CONFIG_NETWORK_LAYER_BLE}" = 1 && test "${with_device_layer}" = "linux"; then
      case "${target}" in

          *linux*)
              if test "x${HAVE_CXX11}" == "0"; then
                AC_MSG_ERROR([BlueZ support requires C++11 compiler])
              fi
              CONFIG_BLE_PLATFORM_BLUEZ=1
              enable_chipoble_bluez="yes"
              ;;

          *)
              CONFIG_BLE_PLATFORM_BLUEZ=0
              enable_chipoble_bluez="no"
              ;;

          esac
      fi
  ]
)

# Depending on whether bluez has been configured for an internal
# location, its directory stem within this package needs to be set
# accordingly. In addition, if the location is internal, then we need
# to attempt to pull it down using the bootstrap makefile.

if test "${nl_with_bluez}" = "internal" && test "${enable_chipoble_bluez}" = "yes"; then
    maybe_bluez_dirstem="bluez/repo"
    bluez_dirstem="third_party/${maybe_bluez_dirstem}"

    AC_MSG_NOTICE([attempting to create internal ${bluez_dirstem}])

    ${MAKE-make} --no-print-directory -C ${srcdir} -f Makefile-bootstrap ${bluez_dirstem}

    if test $? -ne 0; then
        AC_MSG_ERROR([failed to create ${bluez_dirstem}. Please check your network connection or the correctness of 'repos.conf'])
    fi

    echo "  BOOTSTRAP    ${bluez_dirstem}"

    (cd ${srcdir}/${bluez_dirstem} && ./bootstrap)

else
    maybe_bluez_dirstem=""
fi

AC_SUBST(BLUEZ_SUBDIRS, [${maybe_bluez_dirstem}])
AM_CONDITIONAL([CHIP_WITH_BLUEZ_INTERNAL], [test "${nl_with_bluez}" = "internal"])

AM_CONDITIONAL([CONFIG_BLE_PLATFORM_BLUEZ], [test "${enable_chipoble_bluez}" = "yes"])
AC_DEFINE_UNQUOTED([CONFIG_BLE_PLATFORM_BLUEZ],[${CONFIG_BLE_PLATFORM_BLUEZ}],[Define to 1 if you want to enable Chip over Ble over bluez.])

if test "${enable_chipoble_bluez}" = "yes"; then
    PKG_CHECK_MODULES(GLIB, glib-2.0 >= 2.28, dummy=yes,
				AC_MSG_ERROR(GLib >= 2.28 is required))
    AC_SUBST(GLIB_CFLAGS)
    AC_SUBST(GLIB_LIBS)

    PKG_CHECK_MODULES(DBUS, dbus-1 >= 1.6, dummy=yes,
				AC_MSG_ERROR(D-Bus >= 1.6 is required))
    AC_SUBST(DBUS_CFLAGS)
    AC_SUBST(DBUS_LIBS)

    AX_CHECK_COMPILER_OPTIONS([C],   ${GLIB_CFLAGS} ${DBUS_CFLAGS})
    AX_CHECK_COMPILER_OPTIONS([C++], ${GLIB_CFLAGS} ${DBUS_CFLAGS})
fi

#
# OpenSSL
#

if test "${with_crypto}" = "openssl"; then
    NL_WITH_REQUIRED_EXTERNAL_PACKAGE([OpenSSL],
	[OPENSSL],
	[openssl],
	[-lcrypto],
	[
	    # Check for required OpenSSL headers.
	    AC_CHECK_HEADERS([openssl/aes.h] [openssl/bn.h] [openssl/crypto.h] [openssl/ec.h] [openssl/err.h] [openssl/evp.h] [openssl/hmac.h] [openssl/kdf.h] [openssl/rand.h] [openssl/sha.h] [openssl/srp.h],
		[],
		[
		    AC_MSG_ERROR(The OpenSSL header "$ac_header" is required but cannot be found.)
		]
	    )
	]
    )
fi

AM_CONDITIONAL([CHIP_WITH_OPENSSL], [test "${nl_with_openssl}" != "no"])

if test "${nl_with_openssl}" = "no"; then
    AC_DEFINE([CHIP_WITH_OPENSSL], [0], [Define to 0 to build CHIP without OpenSSL])
else
    AC_DEFINE([CHIP_WITH_OPENSSL], [1], [Define to 1 to build CHIP with OpenSSL])
fi

#
# mbedTLS
#

if test "${with_crypto}" = "mbedtls"; then
    NL_WITH_REQUIRED_INTERNAL_PACKAGE(
	[mbedTLS],
	[MBEDTLS],
	[mbedtls],
	[-lmbedtls],
	[
	    # At this point, the internal mbedTLS package will be neither
	    # configured nor built, so the normal checks we undertake for an
	    # external package cannot be run here. Simply set the appropriate
	    # variables and trust all will be well.

	    MBEDTLS_CPPFLAGS="-I\${abs_top_srcdir}/third_party/mbedtls/repo/include"
	    MBEDTLS_LDFLAGS="-L${ac_pwd}/third_party/mbedtls"
	    MBEDTLS_LIBS="-lmbedtls"
	],
	[
	    # Check for required mbedTLS headers.
	    AC_CHECK_HEADERS([mbedtls/sha1.h] [mbedtls/sha256.h] [mbedtls/sha512.h] [mbedtls/md.h] [mbedtls/hkdf.h] [mbedtls/pkcs5.h] [mbedtls/chachapoly.h] [mbedtls/aes.h] [mbedtls/ecdh.h] [mbedtls/bignum.h],
		[],
		[
		    AC_MSG_ERROR(The mbedTLS header "$ac_header" is required but cannot be found.)
		],
        [
// Undefine MBEDTLS_CONFIG_FILE and MBEDTLS_USER_CONFIG_FILE to prevent
// including these files at configure stage.  This is necessary because
// CPPFLAGS is shell expanded during configure stage, which is not
// compatible with the expansion of make.
#undef MBEDTLS_CONFIG_FILE
#undef MBEDTLS_USER_CONFIG_FILE
		])
	]
    )
fi

# Depending on whether mbedTLS has been configured for an internal
# location, its directory stem within this package needs to be set
# accordingly. In addition, if the location is internal, then we need
# to attempt to pull it down using the bootstrap makefile.

if test "${nl_with_mbedtls}" = "internal"; then
    maybe_mbedtls_dirstem="mbedtls"
    mbedtls_dirstem="third_party/${maybe_mbedtls_dirstem}/repo"

    AC_MSG_NOTICE([attempting to create internal ${mbedtls_dirstem}])

    ${MAKE-make} --no-print-directory -C ${srcdir} -f Makefile-bootstrap ${mbedtls_dirstem}

    if test $? -ne 0; then
        AC_MSG_ERROR([failed to create ${mbedtls_dirstem}. Please check your network connection or the correctness of 'repos.conf'])
    fi
else
    maybe_mbedtls_dirstem=""
fi

AC_SUBST(MBEDTLS_SUBDIRS, [${maybe_mbedtls_dirstem}])
AM_CONDITIONAL([CHIP_WITH_MBEDTLS_INTERNAL], [test "${nl_with_mbedtls}" = "internal"])

AM_CONDITIONAL([CHIP_WITH_MBEDTLS], [test "${nl_with_mbedtls}" != "no"])

if test "${nl_with_mbedtls}" = "no"; then
    AC_DEFINE([CHIP_WITH_MBEDTLS], [0], [Define to 0 to build CHIP without mbedTLS])
else
    AC_DEFINE([CHIP_WITH_MBEDTLS], [1], [Define to 1 to build CHIP with mbedTLS])
fi

#
# OpenThread
#

NL_WITH_OPTIONAL_INTERNAL_PACKAGE(
    [OpenThread],
    [OPENTHREAD],
    [openthread],
    [],
    [
        # use absolute path to support build out of source tree
        chip_srcdir="$(cd ${srcdir} && pwd)"

        # bootstrap
        test -f ${chip_srcdir}/third_party/openthread/repo/configure || (cd ${chip_srcdir}/third_party/openthread/repo && ./bootstrap)

        case "${with_device_layer}" in
        nrf5)
            OPENTHREAD_CPPFLAGS="-I\${abs_top_srcdir}/third_party/openthread/repo/include"
            OPENTHREAD_LDFLAGS="-L${ac_pwd}/third_party/openthread"
            OPENTHREAD_LIBS="-lopenthread-cli-ftd \
                -lopenthread-ftd \
                -lopenthread-platform-utils \
                -lopenthread-nrf52840-transport \
                -lopenthread-nrf52840-softdevice-sdk \
                -lnordicsemi-nrf52840-radio-driver-softdevice \
                \${NRF5_SDK_ROOT}/external/openthread/nrf_security/lib/libmbedcrypto_glue.a \
                \${NRF5_SDK_ROOT}/external/openthread/nrf_security/lib/libmbedcrypto_glue_cc310.a \
                \${NRF5_SDK_ROOT}/external/openthread/nrf_security/lib/libmbedcrypto_glue_vanilla.a \
                \${NRF5_SDK_ROOT}/external/openthread/nrf_security/lib/libmbedcrypto_cc310_backend.a \
                \${NRF5_SDK_ROOT}/external/openthread/nrf_security/lib/libmbedcrypto_vanilla_backend.a \
                \${NRF5_SDK_ROOT}/external/openthread/nrf_security/lib/libmbedtls_base_vanilla.a \
                \${NRF5_SDK_ROOT}/external/openthread/nrf_security/lib/libmbedtls_tls_vanilla.a \
                \${NRF5_SDK_ROOT}/external/openthread/nrf_security/lib/libmbedtls_x509_vanilla.a \
                \${NRF5_SDK_ROOT}/external/openthread/nrf_security/lib/libnrf_cc310_platform_0.9.1.a"

            # private definitions
            ot_cppflags="-Wno-expansion-to-defined \
                -I${chip_srcdir}/third_party/openthread/repo/examples/platforms/nrf528xx/nrf52840 \
                -I${chip_srcdir}/third_party/openthread/repo/third_party/NordicSemiconductor/libraries/nrf_security/config \
                -I${chip_srcdir}/third_party/openthread/repo/third_party/NordicSemiconductor/libraries/nrf_security/include \
                -I${chip_srcdir}/third_party/openthread/repo/third_party/NordicSemiconductor/libraries/nrf_security/mbedtls_plat_config \
                -I${chip_srcdir}/third_party/openthread/repo/third_party/NordicSemiconductor/libraries/nrf_security/nrf_cc310_plat/include \
                -I${chip_srcdir}/third_party/mbedtls/repo/include \
                -DMBEDTLS_CONFIG_FILE=\<nrf-config.h\> \
                -DMBEDTLS_USER_CONFIG_FILE=\<nrf52840-mbedtls-config.h\> \
                -DOPENTHREAD_PROJECT_CORE_CONFIG_FILE=\<openthread-core-nrf52840-config.h\> \
                -DOPENTHREAD_CORE_CONFIG_PLATFORM_CHECK_FILE=\<openthread-core-nrf52840-config-check.h\> \
                -DOPENTHREAD_CONFIG_HEAP_EXTERNAL_ENABLE=1 \
                -DOPENTHREAD_CONFIG_JOINER_ENABLE=1 \
                -DOPENTHREAD_CONFIG_NCP_UART_ENABLE=1 \
                -DOPENTHREAD_CONFIG_IP6_SLAAC_ENABLE=1 \
                -DUART_AS_SERIAL_TRANSPORT=1"

            (mkdir -p ${ac_pwd}/third_party/openthread \
             && cd ${ac_pwd}/third_party/openthread \
             && CPPFLAGS="${ot_cppflags}" CFLAGS="${CFLAGS}" CXXFLAGS="${CXXFLAGS}" ${chip_srcdir}/third_party/openthread/repo/configure \
             --disable-builtin-mbedtls \
             --disable-docs \
             --disable-executable \
             --disable-tools \
             --enable-cli \
             --enable-ftd \
             --host=arm-none-eabi \
             --prefix=${prefix} \
             --exec-prefix=${exec_prefix} \
             --with-examples=nrf52840)

            if test $? != 0; then
                AC_MSG_ERROR([Failed to configure OpenThread!])
            fi

            maybe_openthread_dirstem="openthread"
            ;;

        k32w)
            OPENTHREAD_CPPFLAGS="-I\${abs_top_srcdir}/third_party/openthread/repo/include"
            OPENTHREAD_LDFLAGS="-L${ac_pwd}/third_party/openthread"
            OPENTHREAD_LIBS="-lopenthread-cli-ftd \
                -lopenthread-ftd \
                -lopenthread-platform-utils \
                -lopenthread-mtd \
                -lopenthread-k32w061_plat\
                -lopenthread-k32w061_sdk \
                -lmbedcrypto \
                \${K32W061_SDK_ROOT}/middleware/wireless/ieee-802.15.4/lib/libMiniMac.a \
                \${K32W061_SDK_ROOT}/middleware/wireless/framework/XCVR/lib/libRadio.a \
                \${K32W061_SDK_ROOT}/middleware/wireless/framework/PDM/Library/libPDM.a "

            # private definitions
            ot_cppflags="-Wno-expansion-to-defined \
                -I${chip_srcdir}/third_party/openthread/repo/examples/platforms/k32w/k32w061 \
                -I${chip_srcdir}/third_party/openthread/repo/examples/platforms \
                -I${chip_srcdir}/third_party/openthread/repo/include \
                -I${chip_srcdir}/third_party/openthread/repo/src/core \
                -I${chip_srcdir}/third_party/mbedtls/repo/include \
                -I${chip_srcdir}/third_party/mbedtls/repo/ \
                -I${K32W061_SDK_ROOT}/middleware/mbedtls/port/ksdk \
                -I${K32W061_SDK_ROOT}/middleware/mbedtls/repo/include/mbedtls \
                -I${K32W061_SDK_ROOT}/middleware/wireless/ieee-802.15.4/uMac/Include \
                -I${K32W061_SDK_ROOT}/middleware/wireless/framework/Common \
                -I${K32W061_SDK_ROOT}/middleware/wireless/framework/XCVR/DK6/Build/Include \
                -I${K32W061_SDK_ROOT}/middleware/wireless/framework/XCVR/DK6 \
                -I${K32W061_SDK_ROOT}/middleware/wireless/framework/SerialManager/Source \
                -I${K32W061_SDK_ROOT}/middleware/wireless/framework/TimersManager/Source \
                -I${K32W061_SDK_ROOT}/components/serial_manager \
                -I${K32W061_SDK_ROOT}/components/uart \
                -I${K32W061_SDK_ROOT}/devices/K32W061 \
                -I${K32W061_SDK_ROOT}/devices/K32W061/drivers \
                -I${K32W061_SDK_ROOT}/devices/K32W061/utilities \
                -I${K32W061_SDK_ROOT}/devices/K32W061/utilities/debug_console \
                -I${K32W061_SDK_ROOT}/devices/K32W061/utilities/str \
                -I${K32W061_SDK_ROOT}/boards/k32w061dk6/wireless_examples/openthread/enablement \
                -I${K32W061_SDK_ROOT}/CMSIS/Include \
                -I${K32W061_SDK_ROOT}/rtos/amazon-freertos/lib/include \
                -I${K32W061_SDK_ROOT}/rtos/amazon-freertos/lib/include/private \
                -I${K32W061_SDK_ROOT}/rtos/amazon-freertos/template_application/ARM_CM3 \
                -I${K32W061_SDK_ROOT}/rtos/amazon-freertos/lib/FreeRTOS/portable/GCC/ARM_CM3 \
                -I${K32W061_SDK_ROOT}/rtos/amazon-freertos/lib/third_party/unity/src \
                -DCPU_K32W061HN \
                -DCPU_JN518X \
                -DCPU_JN518X_REV=2 \
                -DJENNIC_CHIP_FAMILY_JN518x \
                -DJENNIC_CHIP_FAMILY_NAME=_JN518x \
                -DSDK_DEBUGCONSOLE=0 \
                -DUSE_RTOS=1 \
                -DOPENTHREAD_CONFIG_HEAP_EXTERNAL_ENABLE=1 \
                -DOPENTHREAD_CONFIG_HEAP_INTERNAL_SIZE=16 \
                -DOPENTHREAD_CONFIG_JOINER_ENABLE=1 \
                -DOPENTHREAD_CONFIG_IP6_SLAAC_ENABLE=1 \
                -DMBEDTLS_THREADING_C=1 \
                -DMBEDTLS_THREADING_ALT=1 \
                -DOPENTHREAD_PROJECT_CORE_CONFIG_FILE=\<openthread-core-k32w061-config.h\> \
                -DMBEDTLS_USER_CONFIG_FILE=\<k32w061-mbedtls-config.h\> "

            (mkdir -p ${ac_pwd}/third_party/openthread \
             && cd ${ac_pwd}/third_party/openthread \
             && CPPFLAGS="${ot_cppflags}" CFLAGS="${CFLAGS}" CXXFLAGS="${CXXFLAGS}" ${chip_srcdir}/third_party/openthread/repo/configure \
             --disable-docs \
             --disable-executable \
             --disable-tools \
             --disable-tests \
             --enable-cli \
             --enable-ftd \
             --enable-mtd \
             --host=arm-none-eabi \
             --prefix=${prefix} \
             --exec-prefix=${exec_prefix} \
             --with-examples=k32w061)

            if test $? != 0; then
                AC_MSG_ERROR([Failed to configure OpenThread!])
            fi

            maybe_openthread_dirstem="openthread"
            ;;

        *)
            AC_MSG_NOTICE([No internal OpenThread support yet!])
            nl_with_openthread=no
            with_openthread=no
            ;;
        esac
    ],
    [
        # Check for required OpenThread headers.
        AC_CHECK_HEADERS([openthread/dataset.h] [openthread/dataset_ftd.h] [openthread/error.h] [openthread/icmp6.h] [openthread/instance.h] [openthread/ip6.h] [openthread/link.h] [openthread/message.h] [openthread/netdata.h] [openthread/tasklet.h] [openthread/thread.h],
        [],
        [
            AC_MSG_ERROR(The OpenThread header "$ac_header" is required but cannot be found.)
        ])
        maybe_openthread_dirstem=""
    ]
)
AC_SUBST(OPENTHREAD_SUBDIRS, [${maybe_openthread_dirstem}])
AM_CONDITIONAL([CHIP_ENABLE_OPENTHREAD], [test "${nl_with_openthread}" != "no"])
if test "${nl_with_openthread}" != "no"; then
    CHIP_ENABLE_OPENTHREAD=1
else
    CHIP_ENABLE_OPENTHREAD=0
fi
AC_DEFINE_UNQUOTED([CHIP_ENABLE_OPENTHREAD],[${CHIP_ENABLE_OPENTHREAD}],[Define to 1 if you want to enable OpenThread.])

#
# LwIP
#

NL_WITH_LWIP(${CHIP_SYSTEM_CONFIG_USE_LWIP})

#
# Nlio
#

NL_WITH_PACKAGE(
    [Nlio],
    [NLIO],
    [nlio],
    [],
    [
        # At this point, the internal Nlio package will be neither
        # configured nor built, so the normal checks we undertake for an
        # external package cannot be run here. Simply set the appropriate
        # variables and trust all will be well.

        NLIO_CPPFLAGS="-I\${abs_top_srcdir}/third_party/nlio/repo/include"
        NLIO_LDFLAGS=
        NLIO_LIBS=
    ],
    [
        # Check for required Nlio headers.

        AC_CHECK_HEADERS([nlbyteorder.h],
        [],
        [
            AC_MSG_ERROR(The nlio header "$ac_header" is required but cannot be found.)
        ])
    ]
)

# Depending on whether nlio has been configured for an internal
# location, its directory stem within this package needs to be set
# accordingly. In addition, if the location is internal, then we need
# to attempt to pull it down using the bootstrap makefile.

if test "${nl_with_nlio}" = "internal"; then
    maybe_nlio_dirstem="nlio/repo"
    nlio_dirstem="third_party/${maybe_nlio_dirstem}"

    AC_MSG_NOTICE([attempting to create internal ${nlio_dirstem}])

    ${MAKE-make} --no-print-directory -C ${srcdir} -f Makefile-bootstrap ${nlio_dirstem}

    if test $? -ne 0; then
        AC_MSG_ERROR([failed to create ${nlio_dirstem}. Please check your network connection or the correctness of 'repos.conf'])
    fi
else
    maybe_nlio_dirstem=""
fi

AC_SUBST(NLIO_SUBDIRS, [${maybe_nlio_dirstem}])
AM_CONDITIONAL([CHIP_WITH_NLIO_INTERNAL], [test "${nl_with_nlio}" = "internal"])

#
# Nlassert
#

NL_WITH_PACKAGE(
    [Nlassert],
    [NLASSERT],
    [nlassert],
    [],
    [
        # At this point, the internal Nlassert package will be neither
        # configured nor built, so the normal checks we undertake for an
        # external package cannot be run here. Simply set the appropriate
        # variables and trust all will be well.

        NLASSERT_CPPFLAGS="-I\${abs_top_srcdir}/third_party/nlassert/repo/include"
        NLASSERT_LDFLAGS=
        NLASSERT_LIBS=
    ],
    [
        # Check for required Nlassert headers.

        AC_CHECK_HEADERS([nlassert.h],
        [],
        [
            AC_MSG_ERROR(The nlassert header "$ac_header" is required but cannot be found.)
        ])
    ]
)

# Depending on whether nlassert has been configured for an internal
# location, its directory stem within this package needs to be set
# accordingly. In addition, if the location is internal, then we need
# to attempt to pull it down using the bootstrap makefile.

if test "${nl_with_nlassert}" = "internal"; then
    maybe_nlassert_dirstem="nlassert/repo"
    nlassert_dirstem="third_party/${maybe_nlassert_dirstem}"

    AC_MSG_NOTICE([attempting to create internal ${nlassert_dirstem}])

    ${MAKE-make} --no-print-directory -C ${srcdir} -f Makefile-bootstrap ${nlassert_dirstem}

    if test $? -ne 0; then
        AC_MSG_ERROR([failed to create ${nlassert_dirstem}. Please check your network connection or the correctness of 'repos.conf'])
    fi
else
    maybe_nlassert_dirstem=""
fi

AC_SUBST(NLASSERT_SUBDIRS, [${maybe_nlassert_dirstem}])
AM_CONDITIONAL([CHIP_WITH_NLASSERT_INTERNAL], [test "${nl_with_nlassert}" = "internal"])

#
# Nlfaultinjection
#

NL_WITH_OPTIONAL_INTERNAL_PACKAGE(
    [Nlfaultinjection],
    [NLFAULTINJECTION],
    [nlfaultinjection],
    [-lnlfaultinjection],
    [
        # At this point, the internal nlfaultinjection package will be neither
        # configured nor built, so the normal checks we undertake for an
        # external package cannot be run here. Simply set the appropriate
        # variables and trust all will be well.

        NLFAULTINJECTION_CPPFLAGS="-I\${abs_top_srcdir}/third_party/nlfaultinjection/repo/include"
        NLFAULTINJECTION_LDFLAGS="-L${ac_pwd}/third_party/nlfaultinjection/repo/src"
        NLFAULTINJECTION_LIBS="-lnlfaultinjection"
    ],
    [
        # Check for required nlfaultinjection headers.

        AC_LANG_PUSH([C++])
        AC_CHECK_HEADERS([nlfaultinjection.hpp],
        [],
        [
            AC_MSG_ERROR(The nlfaultinjection header "$ac_header" is required but cannot be found.)
        ])
        AC_LANG_POP([C++])
    ]
)

# Depending on whether nlfaultinjection has been configured for an internal
# location, its directory stem within this package needs to be set
# accordingly. In addition, if the location is internal, then we need
# to attempt to pull it down using the bootstrap makefile.

if test "${nl_with_nlfaultinjection}" = "internal"; then
    maybe_nlfaultinjection_dirstem="nlfaultinjection/repo"
    nlfaultinjection_dirstem="third_party/${maybe_nlfaultinjection_dirstem}"

    AC_MSG_NOTICE([attempting to create internal ${nlfaultinjection_dirstem}])

    ${MAKE-make} --no-print-directory -C ${srcdir} -f Makefile-bootstrap ${nlfaultinjection_dirstem}

    if test $? -ne 0; then
        AC_MSG_ERROR([failed to create ${nlfaultinjection_dirstem}. Please check your network connection or the correctness of 'repos.conf'])
    fi
else
    maybe_nlfaultinjection_dirstem=""
fi

AC_SUBST(NLFAULTINJECTION_SUBDIRS, [${maybe_nlfaultinjection_dirstem}])
AM_CONDITIONAL([CHIP_WITH_NLFAULTINJECTION], [test "${nl_with_nlfaultinjection}" != "no"])
AM_CONDITIONAL([CHIP_WITH_NLFAULTINJECTION_INTERNAL], [test "${nl_with_nlfaultinjection}" = "internal"])

if test "${nl_with_nlfaultinjection}" = "no"; then
    AC_DEFINE([CHIP_WITH_NLFAULTINJECTION], [0], [Define to 0 to build CHIP without nlFaultInjection features])
else
    AC_DEFINE([CHIP_WITH_NLFAULTINJECTION], [1], [Define to 1 to build CHIP with nlFaultInjection features])
fi

if test "${nl_cv_build_tests}" = "yes"; then
    if test "${nl_with_nlfaultinjection}" = "no"; then
        AC_MSG_ERROR(CHIP tests require nlfaultinjection.)
    fi
fi

#
# otbr-client
#

NL_WITH_OPTIONAL_INTERNAL_PACKAGE(
    [ot_br_posix],
    [OT_BR_POSIX],
    [ot_br_posix],
    [-lot_br_client],
    [
        OT_BR_POSIX_CPPFLAGS="-I\${abs_top_srcdir}/third_party/ot-br-posix/repo/include/ -I\${abs_top_srcdir}/third_party/ot-br-posix/repo/src/"
        OT_BR_POSIX_LDFLAGS="-L${ac_pwd}/third_party/ot-br-posix/"
        OT_BR_POSIX_LIBS="-lot_br_client"
    ],
    [
        # Check for required nlfaultinjection headers.
        AC_LANG_PUSH([C++])
        AC_CHECK_HEADERS([dbus/client/thread_api_dbus.hpp],
        [],
        [
            AC_MSG_ERROR(The OpenThread border router header "$ac_header" is required but cannot be found.)
        ])
        AC_LANG_POP([C++])
    ]
)

AC_MSG_NOTICE("nl_with_ot_br_posix=${nl_with_ot_br_posix}")

if test "${nl_with_ot_br_posix}" = "internal" && test "${with_device_layer}" = "linux"; then
    maybe_ot_br_posix_dirstem="ot-br-posix"
    ot_br_posix_dirstem="third_party/${maybe_ot_br_posix_dirstem}/repo"

    AC_MSG_NOTICE([attempting to create internal ${ot_br_posix_dirstem}])

    ${MAKE-make} --no-print-directory -C ${srcdir} -f Makefile-bootstrap ${ot_br_posix_dirstem}

    if test $? -ne 0; then
        AC_MSG_ERROR([failed to create ${ot_br_posix_dirstem}. Please check your network connection or the correctness of 'repos.conf'])
    fi

    PKG_CHECK_MODULES(DBUS, dbus-1 >= 1.4)
    AC_SUBST([DBUS_CFLAGS])
    AC_SUBST([DBUS_LIBS])
else
    maybe_ot_br_posix_dirstem=
fi

AC_SUBST(OT_BR_POSIX_SUBDIRS, [${maybe_ot_br_posix_dirstem}])
AM_CONDITIONAL([CHIP_WITH_OT_BR_POSIX], [test "${nl_with_ot_br_posix}" != "no"])

#
# Nlunit-test
#

if test "${nl_cv_build_tests}" = "yes"; then
    NL_WITH_PACKAGE(
        [Nlunit-test],
        [NLUNIT_TEST],
        [nlunit_test],
        [-lnlunit-test],
        [
	    # At this point, the internal Nlunit-test package will be neither
	    # configured nor built, so the normal checks we undertake for an
	    # external package cannot be run here. Simply set the appropriate
	    # variables and trust all will be well.

            NLUNIT_TEST_CPPFLAGS="-I\${abs_top_srcdir}/third_party/nlunit-test/repo/src"
            NLUNIT_TEST_LDFLAGS="-L${ac_pwd}/third_party/nlunit-test/repo/src"
            NLUNIT_TEST_LIBS="-lnlunit-test"
        ],
        [
            # Check for required nlunit-test headers.

            AC_CHECK_HEADERS([nlunit-test.h],
            [],
            [
                AC_MSG_ERROR(The nlunit-test header "$ac_header" is required but cannot be found.)
            ])
        ])
fi

# Depending on whether nlunit-test has been configured for an internal
# location, its directory stem within this package needs to be set
# accordingly. In addition, if the location is internal, then we need
# to attempt to pull it down using the bootstrap makefile.

if test "${nl_with_nlunit_test}" = "internal"; then
    maybe_nlunit_test_dirstem="nlunit-test/repo"
    nlunit_test_dirstem="third_party/${maybe_nlunit_test_dirstem}"

    AC_MSG_NOTICE([attempting to create internal ${nlunit_test_dirstem}])

    ${MAKE-make} --no-print-directory -C ${srcdir} -f Makefile-bootstrap ${nlunit_test_dirstem}

    if test $? -ne 0; then
        AC_MSG_ERROR([failed to create ${nlunit_test_dirstem}. Please check your network connection or the correctness of 'repos.conf'])
    fi
else
    maybe_nlunit_test_dirstem=""
fi

AC_SUBST(NLUNIT_TEST_SUBDIRS, [${maybe_nlunit_test_dirstem}])
AM_CONDITIONAL([CHIP_WITH_NLUNIT_TEST_INTERNAL], [test "${nl_with_nlunit_test}" = "internal"])

#
#
# IniPP
#

if test "${CHIP_DEVICE_LAYER_TARGET_LINUX}" = 1; then
    NL_WITH_PACKAGE(
        [IniPP],
        [INIPP],
        [],
        [],
        [
            INIPP_CPPFLAGS="-I\${abs_top_srcdir}/third_party/inipp/repo/inipp"
        ],
        [
            # Check for required IniPP headers.

            AC_CHECK_HEADERS([inipp.h],
            [],
            [
                AC_MSG_ERROR(The IniPP header "inipp.h" is required but cannot be found.)
            ])
        ]
    )
fi

#
#
# GIO
#

CHIP_ENABLE_GIO=0

if test "${CHIP_DEVICE_LAYER_TARGET_LINUX}" = 1; then
    PKG_CHECK_MODULES([GIO], [gio-2.0])

    # Check for GIO library is available.
    AC_CHECK_LIB([gio-2.0], [g_bus_get_sync],
    [
        CHIP_ENABLE_GIO=1
        AC_DEFINE([CHIP_WITH_GIO], [1], [Define to 1 to build CHIP with GIO])
    ],
    [
        CHIP_ENABLE_GIO=0
        AC_DEFINE([CHIP_WITH_GIO], [0], [Define to 0 to build CHIP without GIO])
    ]
    )

    PKG_CHECK_MODULES([GIO_UNIX], [gio-unix-2.0])

    AC_CHECK_PROG(GDBUSCODEGEN, gdbus-codegen, gdbus-codegen)
fi

#
#
# WPA
#

CONFIG_WIFI_PLATFORM_WPA=0
enable_chipwifi_wpa="no"

# At this time, enable this only for linux, and only if the GIO package are installed. Currently, WiFi and Thread
# are depending on the gdbus-internal configured by internal BLuez package.
# TODO: Refacrot BLuez to use common gdbus lib from GIO.

if test "${with_device_layer}" = "linux" && test "${CHIP_ENABLE_GIO}" = 1 && test "${CONFIG_BLE_PLATFORM_BLUEZ}" = 1; then
case "${target}" in
    *linux*)
        CONFIG_WIFI_PLATFORM_WPA=1
        enable_chipwifi_wpa="yes"
        ;;
    *)
        CONFIG_WIFI_PLATFORM_WPA=0
        enable_chipwifi_wpa="no"
        ;;
    esac
fi

AM_CONDITIONAL([CONFIG_WIFI_PLATFORM_WPA], [test "${enable_chipwifi_wpa}" = "yes"])
AC_DEFINE_UNQUOTED([CHIP_DEVICE_CONFIG_ENABLE_WPA],[${CONFIG_WIFI_PLATFORM_WPA}],[Define to 1 if you want to enable WPA support for wpa_supplicant.])

#
# Sockets
#

# Influential external variables for BSD Sockets support

AC_ARG_VAR(SOCKETS_CPPFLAGS, [BSD Sockets C preprocessor flags])
AC_ARG_VAR(SOCKETS_LDFLAGS,  [BSD Sockets linker flags])
AC_ARG_VAR(SOCKETS_LIBS,     [BSD Sockets linker libraries])


#
# Check for headers
#
AC_HEADER_STDBOOL
AC_HEADER_STDC

AC_CHECK_HEADERS([ctype.h])
AC_CHECK_HEADERS([ifaddrs.h])
AC_CHECK_HEADERS([stdint.h])
AC_CHECK_HEADERS([stdlib.h])
AC_CHECK_HEADERS([string.h])

# Check for netinet/icmp6.h for ICMP6_FILTER support. Both Android and
# Mac OS X fail to properly include both netinet/in.h and sys/types.h
# in netinet/icmp6.h, making the check fail unless it exists and these
# are both also included.

AC_CHECK_HEADERS([netinet/in.h])
AC_CHECK_HEADERS([sys/types.h])
AC_CHECK_HEADERS([netinet/icmp6.h], [], [],
[
#if HAVE_SYS_TYPES_H
# include <sys/types.h>
#endif
#if HAVE_NETINET_IN_H
# include <netinet/in.h>
#endif
])

# But, wait, this still isn't enough. Even with all of that, Android
# may still not have the ICMP6_FILTER socket option defined. Check for
# that too.

AC_MSG_CHECKING([whether netinet/icmp6.h declares ICMP6_FILTER])
AC_COMPILE_IFELSE([
          AC_LANG_PROGRAM(
[[
#if HAVE_SYS_TYPES_H
# include <sys/types.h>
#endif
#if HAVE_NETINET_IN_H
# include <netinet/in.h>
#endif
#if HAVE_NETINET_ICMP6_H
# include <netinet/icmp6.h>
#endif
]],
[[
#if !defined(ICMP6_FILTER)
# error "ICMP6_FILTER is not defined"
#endif
]])],
[
AC_MSG_RESULT([yes])
AC_DEFINE(HAVE_ICMP6_FILTER, 1, [Define to 1 if your <netinet/icmp6.h> header file defines the ICMP6_FILTER socket option.])
],
[
AC_MSG_RESULT([no])
])

# Check for sys/socket.h for SO_BINDTODEVICE support.

AC_CHECK_HEADERS([sys/socket.h])

AC_MSG_CHECKING([whether sys/socket.h declares SO_BINDTODEVICE])
AC_COMPILE_IFELSE([
          AC_LANG_PROGRAM(
[[
#if HAVE_SYS_SOCKET_H
# include <sys/socket.h>
#endif
]],
[[
#if !defined(SO_BINDTODEVICE)
# error "SO_BINDTODEVICE is not defined"
#endif
]])],
[
AC_MSG_RESULT([yes])
AC_DEFINE(HAVE_SO_BINDTODEVICE, 1, [Define to 1 if your <sys/socket.h> header file defines the SO_BINDTODEVICE socket option.])
],
[
AC_MSG_RESULT([no])
])

# Check for sys/sockio.h
AC_CHECK_HEADERS([sys/sockio.h])

#
# Check for types and structures
#
AC_TYPE_INT8_T
AC_TYPE_INT16_T
AC_TYPE_INT32_T
AC_TYPE_INT64_T
AC_TYPE_UINT8_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T

#
# Checks for library functions
#

CONFIG_HAVE_HEAP=0

if test "${ac_no_link}" != "yes"; then
    # Check for stdlib memory management functions.
    AC_CHECK_FUNCS([malloc free realloc strdup], [CONFIG_HAVE_HEAP=1], [CONFIG_HAVE_HEAP=0])
    AC_CHECK_FUNCS([memchr memcmp memmove memset])
    AC_CHECK_FUNCS([strchr strcmp strlen])
    AC_CHECK_FUNCS([snprintf])

    # Some versions of Android are missing getifaddrs and
    # freeifaddrs. Check to see if the target system is such a version.

    AC_CHECK_FUNCS([getifaddrs freeifaddrs])

    # Check for clock_gettime, gettimeofday, settimeofday and localtime.
    # In some target environments, clock_gettime exists in librt.

    AC_SEARCH_LIBS([clock_gettime], [rt])
    AC_CHECK_FUNCS([clock_gettime gettimeofday localtime])
    AC_CHECK_HEADERS([time.h sys/time.h])
    AC_CHECK_TYPES([clockid_t], [], [], [[#include <time.h>]])

    # Check for clock_settime.  This function exists on iOS, however the compiler prevents
    # reference to it via the use of an "unavailable" attribute.  Unfortunately, this goes
    # unnoticed by autoconf's AC_CHECK_FUNCS macro, hence the need for custom detection code.

    AC_SEARCH_LIBS([clock_settime], [rt])
    AC_MSG_CHECKING([clock_settime])
    AC_LINK_IFELSE(
        [AC_LANG_SOURCE([[
            #include <stdlib.h>
            #include <time.h>
            int main(int argc, char *argv[])
            {
                clock_settime(CLOCK_REALTIME, NULL);
                return 0;
            }
        ]])],
        [AC_DEFINE([HAVE_CLOCK_SETTIME], [1], [Define to 1 if you have the `clock_settime' function.]) AC_MSG_RESULT([yes])],
        [AC_MSG_RESULT([no])]
    )

    # Note that different platforms have different APIs for monotonic timer with sleep time compensation.
    # CLOCK_BOOTTIME is available in later versions of linux and Android

    AC_CHECK_DECLS([CLOCK_MONOTONIC, CLOCK_BOOTTIME], [], [], [[#include <time.h>]])
fi

AC_SUBST(CONFIG_HAVE_HEAP)
AM_CONDITIONAL([CONFIG_HAVE_HEAP], [test "${CONFIG_HAVE_HEAP}" = "1"])

#
# Check for POSIX thread (pthread) support
#
AX_PTHREAD([], [])

AC_CHECK_DECL([PTHREAD_NULL], [], [AC_DEFINE([PTHREAD_NULL],[0], [Approximation of PTHREAD_NULL since pthread.h does not define one])],
              [[#include <pthread.h>]])

#
# Check for Network.framework
#
if test "${CHIP_SYSTEM_CONFIG_USE_NETWORK_FRAMEWORK}" = "1"; then
    AC_CHECK_HEADER([Network/Network.h])
    LDFLAGS="${LDFLAGS} -lnetwork"
fi

#
# Check for <new>
#
AC_LANG_PUSH([C++])

AC_CHECK_HEADERS([new])

AC_LANG_POP([C++])


# Add test enabled macro
if test "${nl_cv_build_tests}" = "yes"; then
    CPPFLAGS="${CPPFLAGS} -DNL_CHIP_WITH_TESTS"
    CPPFLAGS="${CPPFLAGS} -DCHIP_CONFIG_TEST=1"
    CPPFLAGS="${CPPFLAGS} -DCHIP_SYSTEM_CONFIG_TEST=1"
    CPPFLAGS="${CPPFLAGS} -DCHIP_SYSTEM_CONFIG_PROVIDE_STATISTICS=1"
    if test ${CONFIG_NETWORK_LAYER_INET} = 1; then
        CPPFLAGS="${CPPFLAGS} -DINET_CONFIG_TEST=1"
    fi
fi

# Add any crypto-implementation CPPFLAGS, LDFLAGS, and LIBS

CRYPTO_CPPFLAGS="${CRYPTO_CPPFLAGS} ${MBEDTLS_CPPFLAGS} ${OPENSSL_CPPFLAGS}"
CRYPTO_LDFLAGS="${CRYPTO_LDFLAGS} ${MBEDTLS_LDFLAGS} ${OPENSSL_LDFLAGS}"
CRYPTO_LIBS="${CRYPTO_LIBS} ${MBEDTLS_LIBS} ${OPENSSL_LIBS}"

AC_SUBST(CRYPTO_CPPFLAGS)
AC_SUBST(CRYPTO_LDFLAGS)
AC_SUBST(CRYPTO_LIBS)

CPPFLAGS="${CPPFLAGS} ${CRYPTO_CPPFLAGS} ${OPENTHREAD_CPPFLAGS}"
LDFLAGS="${LDFLAGS} ${CRYPTO_LDFLAGS} ${OPENTHREAD_LDFLAGS}"
LIBS="${LIBS} ${CRYPTO_LIBS} ${OPENTHREAD_LIBS}"

# Add any code coverage CPPFLAGS, LDFLAGS, and LIBS

CPPFLAGS="${CPPFLAGS} ${NL_COVERAGE_CPPFLAGS}"
LDFLAGS="${LDFLAGS} ${NL_COVERAGE_LDFLAGS}"
LIBS="${LIBS} ${NL_COVERAGE_LIBS}"

# Fold in the warning-as-errors flag, unless it was already in effect.

if test "${nl_had_CPPFLAGS_werror}" = "no"; then
    CPPFLAGS="${CPPFLAGS} ${NL_WERROR_CPPFLAGS}"
fi


# At this point, we can restore the compiler flags to whatever the
# user passed in, now that we're clear of an -Werror issues by
# transforming -Wno-error back to -Werror.

NL_RESTORE_WERROR

#
# Code style
#

AC_SUBST(PRETTY, ["\${abs_top_srcdir}/scripts/helpers/clang-format.sh"])
AC_SUBST(PRETTY_ARGS, ["-style=file -i"])
AC_SUBST(PRETTY_CHECK, ["\${abs_top_srcdir}/scripts/helpers/clang-format-check.sh"])
AC_SUBST(PRETTY_CHECK_ARGS, [""])

# Configure any autotools-based subdirectories
if test "${nl_with_nlunit_test}" = "internal"; then
AC_CONFIG_SUBDIRS([third_party/nlunit-test/repo])
AC_SUBST(NLUNIT_TEST_FOREIGN_SUBDIR_DEPENDENCY,["${ac_pwd}/third_party/nlunit-test/repo/src"])
fi

if test "${nl_with_nlio}" = "internal"; then
AC_CONFIG_SUBDIRS([third_party/nlio/repo])
fi

if test "${nl_with_nlassert}" = "internal"; then
AC_CONFIG_SUBDIRS([third_party/nlassert/repo])
fi

if test "${nl_with_nlfaultinjection}" = "internal"; then
AC_CONFIG_SUBDIRS([third_party/nlfaultinjection/repo])
AC_SUBST(NLFAULTINJECTION_FOREIGN_SUBDIR_DEPENDENCY,["${ac_pwd}/third_party/nlfaultinjection/repo/src"])
fi

if test "${nl_with_bluez}" = "internal" && test "${enable_chipoble_bluez}" = "yes"; then
AC_CONFIG_SUBDIRS([third_party/bluez/repo])
fi

if test "${nl_with_mbedtls}" = "internal"; then
AC_CONFIG_SUBDIRS([third_party/mbedtls/repo])
fi

if test "${nl_with_lwip}" = "internal"; then
AC_SUBST(LWIP_FOREIGN_SUBDIR_DEPENDENCY,["${ac_pwd}/src/lwip"])
fi

#
# Identify the various makefiles and auto-generated files for the package
#
AC_CONFIG_FILES([
Makefile
docs/Makefile
examples/Makefile
third_party/Makefile
third_party/lwip/Makefile
third_party/mbedtls/Makefile
third_party/ot-br-posix/Makefile
src/Makefile
src/include/Makefile
src/app/Makefile
src/app/clusters/Makefile
src/app/util/Makefile
src/ble/Makefile
src/ble/tests/Makefile
src/controller/java/Makefile
src/crypto/Makefile
src/crypto/tests/Makefile
src/lwip/Makefile
src/lwip/tests/Makefile
src/system/Makefile
src/system/tests/Makefile
src/setup_payload/Makefile
src/setup_payload/java/Makefile
src/setup_payload/tests/Makefile
src/inet/Makefile
src/inet/tests/Makefile
src/lib/Makefile
src/lib/core/tests/Makefile
src/lib/shell/Makefile
src/lib/shell/tests/Makefile
src/lib/support/Makefile
src/lib/support/tests/Makefile
src/platform/Makefile
src/platform/tests/Makefile
src/qrcodetool/Makefile
src/transport/Makefile
src/transport/tests/Makefile
])

#
# Generate the auto-generated files for the package
#
AC_OUTPUT

#
# Summarize the package configuration
#

AC_MSG_NOTICE([

  Configuration Summary
  ---------------------
  Package                                          : ${PACKAGE_NAME}
  Version                                          : ${PACKAGE_VERSION}
  Interface                                        : ${LIBCHIP_VERSION_INFO//:/.}
  Build system                                     : ${build}
  Host system                                      : ${host}
  Target system                                    : ${target}
  Target architecture                              : ${target_cpu}
  Target OS                                        : ${target_os}
  Target style                                     : ${CHIP_TARGET_STYLE}
  Device layer                                     : ${CHIP_DEVICE_LAYER_TARGET}
  Cryptographic implementation                     : ${CHIP_CRYPTO}
  Target network layer                             : ${with_network_layer}
  Target network system(s)                         : ${CONFIG_TARGET_NETWORKS}
  IPv4 enabled                                     : ${enable_ipv4}
  Internet endpoint(s)                             : ${INET_ENDPOINTS}
  Printf enhancements                              : ${CHIP_ENHANCED_PRINTF}
  Android support                                  : ${with_android}
  Logging style                                    : ${CHIP_LOGGING_STYLE}
  Cross compiling                                  : ${cross_compiling}
  Build shared libraries                           : ${enable_shared}
  Build static libraries                           : ${enable_static}
  Build debug libraries                            : ${nl_cv_build_debug}
  Build profile libraries                          : ${build_profile}
  Build optimized libraries                        : ${nl_cv_build_optimized}
  Build coverage libraries                         : ${nl_cv_build_coverage}
  Build coverage reports                           : ${nl_cv_build_coverage_reports}
  Lcov                                             : ${LCOV:--}
  Genhtml                                          : ${GENHTML:--}
  Happy                                            : ${with_happy}
  Happy Path                                       : ${happy_path}
  Treat warnings as errors                         : ${nl_cv_warnings_as_errors}
  Build tests                                      : ${nl_cv_build_tests}
  Build long running tests                         : ${nl_cv_build_long_tests}
  Build tools                                      : ${build_tools}
  Tunnel Failover support                          : ${build_tunnel_failover}
  Prefix                                           : ${prefix}
  Shadow directory program                         : ${LNDIR}
  Cocoa support                                    : ${enable_cocoa}
  Java support                                     : ${enable_java}
  Java home                                        : ${JAVA_HOME:--}
  Java Compiler                                    : ${JAVAC:--}
  Java Native Interface (JNI) compile flags        : ${JNI_CPPFLAGS:--}
  Java Archiver                                    : ${JAR:--}
  Python executable                                : ${PYTHON:--}
  Python version                                   : ${PYTHON_VERSION:--}
  Python script directory                          : ${pythondir:--}
  Python extension directory                       : ${pyexecdir:--}
  Verhoeff support                                 : ${enable_verhoeff}
  Pairing code decoding support                    : ${enable_pairing_code_decoding}
  Documentation support                            : ${nl_cv_build_docs}
  Clang-format                                     : ${CLANG_FORMAT:--}
  Doxygen                                          : ${DOXYGEN:--}
  GraphViz dot                                     : ${DOT:--}
  PERL                                             : ${PERL:--}
  CHIP over BlueZ support                          : ${enable_chipoble_bluez:--}
  BlueZ source                                     : ${nl_with_bluez:--}
  Valgrind                                         : ${VALGRIND:--}
  LwIP source                                      : ${nl_with_lwip:--}
  LwIP compile flags                               : ${LWIP_CPPFLAGS:--}
  LwIP link flags                                  : ${LWIP_LDFLAGS:--}
  LwIP link libraries                              : ${LWIP_LIBS:--}
  LwIP foreign subdirectory dependency             : ${LWIP_FOREIGN_SUBDIR_DEPENDENCY:--}
  mbedTLS source                                   : ${nl_with_mbedtls:--}
  mbedTLS compile flags                            : ${MBEDTLS_CPPFLAGS:--}
  mbedTLS link flags                               : ${MBEDTLS_LDFLAGS:--}
  mbedTLS link libraries                           : ${MBEDTLS_LIBS:--}
  OpenThread source                                : ${nl_with_openthread}
  OpenThread compile flags                         : ${OPENTHREAD_CPPFLAGS:--}
  OpenThread link flags                            : ${OPENTHREAD_LDFLAGS:--}
  OpenThread link libraries                        : ${OPENTHREAD_LIBS:--}
  OpenSSL source                                   : ${nl_with_openssl}
  OpenSSL compile flags                            : ${OPENSSL_CPPFLAGS:--}
  OpenSSL link flags                               : ${OPENSSL_LDFLAGS:--}
  OpenSSL link libraries                           : ${OPENSSL_LIBS:--}
  Crypto implementation compile flags              : ${CRYPTO_CPPFLAGS:--}
  Crypto implementation link flags                 : ${CRYPTO_LDFLAGS:--}
  Crypto implementation link libraries             : ${CRYPTO_LIBS:--}
  Nlunit-test source                               : ${nl_with_nlunit_test:--}
  Nlunit-test compile flags                        : ${NLUNIT_TEST_CPPFLAGS:--}
  Nlunit-test link flags                           : ${NLUNIT_TEST_LDFLAGS:--}
  Nlunit-test link libraries                       : ${NLUNIT_TEST_LIBS:--}
  Nlunit-test foreign subdirectory dependency      : ${NLUNIT_TEST_FOREIGN_SUBDIR_DEPENDENCY:--}
  Nlio source                                      : ${nl_with_nlio:--}
  Nlio compile flags                               : ${NLIO_CPPFLAGS:--}
  Nlio link flags                                  : ${NLIO_LDFLAGS:--}
  Nlio link libraries                              : ${NLIO_LIBS:--}
  Nlassert source                                  : ${nl_with_nlassert:--}
  Nlassert compile flags                           : ${NLASSERT_CPPFLAGS:--}
  Nlassert link flags                              : ${NLASSERT_LDFLAGS:--}
  Nlassert link libraries                          : ${NLASSERT_LIBS:--}
  Nlfaultinjection source                          : ${nl_with_nlfaultinjection:--}
  Nlfaultinjection compile flags                   : ${NLFAULTINJECTION_CPPFLAGS:--}
  Nlfaultinjection link flags                      : ${NLFAULTINJECTION_LDFLAGS:--}
  Nlfaultinjection link libraries                  : ${NLFAULTINJECTION_LIBS:--}
  Nlfaultinjection foreign subdirectory dependency : ${NLFAULTINJECTION_FOREIGN_SUBDIR_DEPENDENCY:--}
  Sockets compile flags                            : ${SOCKETS_CPPFLAGS:--}
  Sockets link flags                               : ${SOCKETS_LDFLAGS:--}
  Sockets link libraries                           : ${SOCKETS_LIBS:--}
  PThreads compile flags                           : ${PTHREAD_CFLAGS:--}
  PThreads link libraries                          : ${PTHREAD_LIBS:--}
  IniPP compile flags                              : ${INIPP_CPPFLAGS:--}
  GIO compile flags                                : ${GIO_CFLAGS:--}
  GIO link flags                                   : ${GIO_LIBS:--}
  GIO UNIX compile flags                           : ${GIO_UNIX_CFLAGS:--}
  GIO UNIX link flags                              : ${GIO_UNIX_LIBS:--}
  C Preprocessor                                   : ${CPP}
  C Compiler                                       : ${CC}
  C++ Preprocessor                                 : ${CXXCPP}
  C++ Compiler                                     : ${CXX}
  Objective C Compiler                             : ${OBJC:--}
  Objective C++ Compiler                           : ${OBJCXX:--}
  Archiver                                         : ${AR}
  Archive Indexer                                  : ${RANLIB}
  Symbol Stripper                                  : ${STRIP}
  Object Copier                                    : ${OBJCOPY}
  C Preprocessor flags                             : ${CPPFLAGS:--}
  C Compile flags                                  : ${CFLAGS:--}
  C++ Compile flags                                : ${CXXFLAGS:--}
  Objective C Compile flags                        : ${OBJCFLAGS:--}
  Objective C++ Compile flags                      : ${OBJCXXFLAGS:--}
  Symbol Visibility C Compile flags                : ${VISIBILITY_CFLAGS:--}
  Symbol Visibility C++ Compile flags              : ${VISIBILITY_CXXFLAGS:--}
  Symbol Visibility Objective C Compile flags      : ${VISIBILITY_OBJCFLAGS:--}
  Symbol Visibility Objective C++ Compile flags    : ${VISIBILITY_OBJCXXFLAGS:--}
  Link flags                                       : ${LDFLAGS:--}
  Link libraries                                   : ${LIBS}
  Fuzzing Enabled                                  : ${enable_fuzzing}
])
