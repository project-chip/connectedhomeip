#
#    Copyright (c) 2020 Project CHIP Authors
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
#      This file is the GNU automake template for the Project CHIP Inet
#      layer library unit tests.
#

include $(abs_top_nlbuild_autotools_dir)/automake/pre.am

#
# Local headers to build against and distribute but not to install
# since they are not part of the package.
#
noinst_HEADERS                                        = \
    TapAddrAutoconf.h                                   \
    TestInetCommon.h                                    \
    TestInetCommonOptions.h                             \
    TestInetLayerCommon.hpp                             \
    $(NULL)

#
# Other files we do want to distribute with the package.
#
EXTRA_DIST                                            = \
    $(NULL)

if CHIP_BUILD_TESTS
lib_LIBRARIES                                         = \
    libInetLayerTests.a                                 \
    $(NULL)

libInetLayerTests_a_SOURCES                           = \
    TestInetAddress.cpp                                 \
    TestInetErrorStr.cpp                                \
    $(NULL)

libInetLayerTests_adir                                = $(includedir)/inet

dist_libInetLayerTests_a_HEADERS                      = \
    TestInetLayer.h                                     \
    $(NULL)

noinst_LIBRARIES                                      = \
    libTestInetCommon.a                                 \
    $(NULL)

# C/C++ preprocessor option flags that will apply to all compiled
# objects in this makefile.

AM_CPPFLAGS                                           = \
    -I$(top_srcdir)/src                                 \
    -I$(top_srcdir)/src/include                         \
    -I$(top_srcdir)/src/lib                             \
    $(NLASSERT_CPPFLAGS)                                \
    $(NLFAULTINJECTION_CPPFLAGS)                        \
    $(NLIO_CPPFLAGS)                                    \
    $(NLUNIT_TEST_CPPFLAGS)                             \
    $(LWIP_CPPFLAGS)                                    \
    $(SOCKETS_CPPFLAGS)                                 \
    $(PTHREAD_CFLAGS)                                   \
    $(NULL)

libTestInetCommon_a_SOURCES                           = \
    TestInetCommon.cpp                                  \
    TestInetCommonOptions.cpp                           \
    TestInetLayerCommon.cpp                             \
    $(NULL)

if CHIP_LWIP_TARGET_STANDALONE

# TapAddrAutoconf is only needed for LwIP on sockets simulation in
# which a host tap/tun interface is used to proxy the LwIP stack onto
# a host native network interface.

libTestInetCommon_a_SOURCES                          += \
    TapAddrAutoconf.cpp                                 \
    $(NULL)

endif # CHIP_LWIP_TARGET_STANDALONE

CHIP_LDADD                                            = \
    $(top_builddir)/src/inet/libInetLayer.a             \
    $(top_builddir)/src/system/libSystemLayer.a         \
    $(top_builddir)/src/lib/support/libSupportLayer.a   \
    $(NULL)

COMMON_LDADD                                          = \
    libInetLayerTests.a                                 \
    $(COMMON_LDFLAGS)                                   \
    $(CHIP_LDADD)                                       \
    $(NLFAULTINJECTION_LDFLAGS) $(NLFAULTINJECTION_LIBS)\
    $(NLUNIT_TEST_LDFLAGS) $(NLUNIT_TEST_LIBS)          \
    $(LWIP_LDFLAGS) $(LWIP_LIBS)                        \
    $(SOCKETS_LDFLAGS) $(SOCKETS_LIBS)                  \
    $(PTHREAD_CFLAGS) $(PTHREAD_LIBS)                   \
    $(NULL)

# Test applications that should be run when the 'check' target is run.

check_PROGRAMS                                        = \
    $(NULL)

check_SCRIPTS                                         = \
    $(NULL)

# The additional environment variables and their values that will be
# made available to all programs and scripts in TESTS.
TESTS_ENVIRONMENT                                     = \
    $(NULL)

if CHIP_DEVICE_LAYER_TARGET_ESP32

check_SCRIPTS                                        += \
    qemu_inet_tests.sh                                  \
    $(NULL)

TESTS_ENVIRONMENT                                    += \
    abs_top_srcdir='$(abs_top_srcdir)'                  \
    abs_top_builddir='$(abs_top_builddir)'              \
    QEMU_TEST_TARGET=libInetLayerTests.a                \
    $(NULL)

else # CHIP_DEVICE_LAYER_TARGET_ESP32

# Test executables that should be built but not installed and should
# always be built to ensure overall "build sanity".
#
# These will NOT be part of the externally-consumable binary SDK.
#
# XXX - At this point TestInetEndPoint and TestInetLayerDNS are stranded
#       here until a solution for enabling IPv6 on Linux containers for
#       Docker on macOS can be resolved as that test involves
#       executing APIs that exercise bind, listen, connect, accept,
#       etc. on IPv4 and IPv6 IP end points. When that issue has been
#       resolved, move these tests to check_PROGRAMS.

# Build and run these test targets only on standalone device targets
noinst_PROGRAMS                                       = \
    TestLwIPDNS                                         \
    TestInetEndPoint                                    \
    TestInetLayer                                       \
    TestInetLayerDNS                                    \
    TestInetLayerMulticast                              \
    $(NULL)

check_PROGRAMS                                       += \
    TestInetAddress                                     \
    TestInetErrorStr                                    \
    $(NULL)

endif # CHIP_DEVICE_LAYER_TARGET_ESP32

# Test applications and scripts that should be built and run when the
# 'check' target is run.

TESTS                                                 = \
    $(check_PROGRAMS)                                   \
    $(check_SCRIPTS)                                    \
    $(NULL)

# Source, compiler, and linker options for test programs.

TestLwIPDNS_SOURCES                                   = TestLwIPDNS.cpp
TestLwIPDNS_LDADD                                     = libTestInetCommon.a $(COMMON_LDADD)

TestInetAddress_SOURCES                               = TestInetAddressDriver.cpp
TestInetAddress_LDADD                                 = $(COMMON_LDADD)

TestInetEndPoint_SOURCES                              = TestInetEndPoint.cpp    \
                                                        $(NULL)
TestInetEndPoint_LDADD                                = libTestInetCommon.a $(COMMON_LDADD)

TestInetErrorStr_SOURCES                              = TestInetErrorStrDriver.cpp    \
                                                        $(NULL)
TestInetErrorStr_LDADD                                = $(COMMON_LDADD)

TestInetLayerDNS_SOURCES                              = TestInetLayerDNS.cpp
TestInetLayerDNS_LDADD                                = libTestInetCommon.a $(COMMON_LDADD)

TestInetLayer_SOURCES                                 = TestInetLayer.cpp    \
                                                        $(NULL)
TestInetLayer_LDADD                                   = libTestInetCommon.a $(COMMON_LDADD)

TestInetLayerMulticast_SOURCES                        = TestInetLayerMulticast.cpp    \
                                                        $(NULL)
TestInetLayerMulticast_LDADD                          = libTestInetCommon.a $(COMMON_LDADD)

#
# Foreign make dependencies
#

NLFOREIGN_FILE_DEPENDENCIES                           = \
   $(CHIP_LDADD)                                        \
   $(NULL)

NLFOREIGN_SUBDIR_DEPENDENCIES                         = \
   $(top_builddir)/src/include                          \
   $(LWIP_FOREIGN_SUBDIR_DEPENDENCY)                    \
   $(NLFAULTINJECTION_FOREIGN_SUBDIR_DEPENDENCY)        \
   $(NLUNIT_TEST_FOREIGN_SUBDIR_DEPENDENCY)             \
   $(NULL)

if CHIP_BUILD_COVERAGE
CLEANFILES                                            = $(wildcard *.gcda *.gcno)

if CHIP_BUILD_COVERAGE_REPORTS
# The bundle should positively be qualified with the absolute build
# path. Otherwise, VPATH will get auto-prefixed to it if there is
# already such a directory in the non-colocated source tree.

CHIP_COVERAGE_BUNDLE                                  = ${abs_builddir}/${PACKAGE}${NL_COVERAGE_BUNDLE_SUFFIX}
CHIP_COVERAGE_INFO                                    = ${CHIP_COVERAGE_BUNDLE}/${PACKAGE}${NL_COVERAGE_INFO_SUFFIX}

$(CHIP_COVERAGE_BUNDLE):
	$(call create-directory)

$(CHIP_COVERAGE_INFO): check-local | $(CHIP_COVERAGE_BUNDLE)
	$(call generate-coverage-report,${top_builddir},*/usr/include/* */third_party/*)

coverage-local: $(CHIP_COVERAGE_INFO)

clean-local: clean-local-coverage

.PHONY: clean-local-coverage
clean-local-coverage:
	-$(AM_V_at)rm -rf $(CHIP_COVERAGE_BUNDLE)
endif # CHIP_BUILD_COVERAGE_REPORTS
endif # CHIP_BUILD_COVERAGE
endif # CHIP_BUILD_TESTS

include $(abs_top_nlbuild_autotools_dir)/automake/post.am
