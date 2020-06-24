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
#      This file is the GNU automake template for the Project CHIP
#      transport layer library unit tests.
#

include $(abs_top_nlbuild_autotools_dir)/automake/pre.am

#
# Local headers to build against and distribute but not to install
# since they are not part of the package.
#
noinst_HEADERS                                        = \
    $(NULL)

#
# Other files we do want to distribute with the package.
#
EXTRA_DIST                                            = \
    $(NULL)

if CHIP_BUILD_TESTS
lib_LIBRARIES                                         = \
    libTransportLayerTests.a                            \
    $(NULL)

libTransportLayerTests_a_SOURCES                      = \
    NetworkTestHelpers.cpp                              \
    TestMessageHeader.cpp                               \
    TestPeerConnections.cpp                             \
    TestSecureSession.cpp                               \
    TestSecureSessionMgr.cpp                            \
    TestUDP.cpp                                         \
    $(NULL)

libTransportLayerTests_adir                           = $(includedir)/transport

dist_libTransportLayerTests_a_HEADERS                 = \
    NetworkTestHelpers.h                                \
    TestTransportLayer.h                                \
    $(NULL)

# C/C++ preprocessor option flags that will apply to all compiled
# objects in this makefile.

AM_CPPFLAGS                                           = \
    -I$(top_srcdir)/src                                 \
    -I$(top_srcdir)/src/lib                             \
    -I$(top_srcdir)/src/inet/tests                      \
    $(NLASSERT_CPPFLAGS)                                \
    $(NLFAULTINJECTION_CPPFLAGS)                        \
    $(NLIO_CPPFLAGS)                                    \
    $(NLUNIT_TEST_CPPFLAGS)                             \
    $(LWIP_CPPFLAGS)                                    \
    $(SOCKETS_CPPFLAGS)                                 \
    $(PTHREAD_CFLAGS)                                   \
    $(NULL)

CHIP_LDADD                                            = \
    $(top_builddir)/src/lib/libCHIP.a                   \
    $(NULL)

COMMON_LDADD                                          = \
    libTransportLayerTests.a                            \
    $(top_builddir)/src/inet/tests/libTestInetCommon.a  \
    $(COMMON_LDFLAGS)                                   \
    $(CHIP_LDADD)                                       \
    $(NLFAULTINJECTION_LDFLAGS)                         \
    $(NLFAULTINJECTION_LIBS)                            \
    $(NLUNIT_TEST_LDFLAGS) $(NLUNIT_TEST_LIBS)          \
    $(LWIP_LDFLAGS) $(LWIP_LIBS)                        \
    $(SOCKETS_LDFLAGS) $(SOCKETS_LIBS)                  \
    $(PTHREAD_CFLAGS) $(PTHREAD_LIBS)                   \
    $(NULL)

# The additional environment variables and their values that will be
# made available to all programs and scripts in TESTS.

TESTS_ENVIRONMENT                                     = \
    $(NULL)

# Test applications that should be run when the 'check' target is run.

check_PROGRAMS                                        = \
    $(NULL)

check_SCRIPTS                                         = \
    $(NULL)

if CHIP_DEVICE_LAYER_TARGET_ESP32

check_SCRIPTS                                        += \
    qemu_transport_tests.sh                             \
    $(NULL)

TESTS_ENVIRONMENT                                    += \
    abs_top_srcdir='$(abs_top_srcdir)'                  \
    abs_top_builddir='$(abs_top_builddir)'              \
    QEMU_TEST_TARGET=libTransportLayerTests.a           \
    $(NULL)

else # CHIP_DEVICE_LAYER_TARGET_ESP32

check_PROGRAMS                                       += \
    TestMessageHeader                                   \
    TestPeerConnections                                 \
    TestSecureSessionMgr                                \
    TestSecureSession                                   \
    TestUDP                                             \
    $(NULL)

endif # CHIP_DEVICE_LAYER_TARGET_ESP32

# Test applications and scripts that should be built and run when the
# 'check' target is run.

TESTS                                                 = \
    $(check_PROGRAMS)                                   \
    $(check_SCRIPTS)                                    \
    $(NULL)

# Source, compiler, and linker options for test programs.

TestMessageHeader_SOURCES     = TestMessageHeaderDriver.cpp
TestMessageHeader_LDADD       = $(COMMON_LDADD)

TestSecureSessionMgr_SOURCES  = TestSecureSessionMgrDriver.cpp
TestSecureSessionMgr_LDADD    = $(COMMON_LDADD)

TestSecureSession_SOURCES     = TestSecureSessionDriver.cpp
TestSecureSession_LDADD       = $(COMMON_LDADD)

TestUDP_SOURCES               = TestUDPDriver.cpp
TestUDP_LDADD                 = $(COMMON_LDADD)

TestPeerConnections_SOURCES   = TestPeerConnectionsDriver.cpp
TestPeerConnections_LDADD     = $(COMMON_LDADD)

#
# Foreign make dependencies
#

NLFOREIGN_FILE_DEPENDENCIES                           = \
   $(CHIP_LDADD)                                        \
   $(NULL)

NLFOREIGN_SUBDIR_DEPENDENCIES                         = \
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
