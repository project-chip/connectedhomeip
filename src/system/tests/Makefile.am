#
#    Copyright (c) 2020 Project CHIP Authors
#    Copyright (c) 2018 Google LLC
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
#      This file is the GNU automake template for the Project CHIP
#      System layer library unit tests.
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
    libSystemLayerTests.a                               \
    $(NULL)

libSystemLayerTests_a_SOURCES                         = \
    TestSystemErrorStr.cpp                              \
    TestSystemObject.cpp                                \
    TestSystemPacketBuffer.cpp                          \
    TestSystemTimer.cpp                                 \
    TestSystemWakeEvent.cpp                             \
    TestTimeSource.cpp                                  \
    $(NULL)

libSystemLayerTests_adir                              = $(includedir)/system

dist_libSystemLayerTests_a_HEADERS                    = \
    TestSystemLayer.h                                   \
    $(NULL)

# C/C++ preprocessor option flags that will apply to all compiled
# objects in this makefile.

AM_CPPFLAGS                                           = \
    -I$(top_srcdir)/src                                 \
    -I$(top_srcdir)/src/lib                             \
    -I$(top_srcdir)/src/lib/core                        \
    -I$(top_srcdir)/src/system                          \
    $(NLASSERT_CPPFLAGS)                                \
    $(NLUNIT_TEST_CPPFLAGS)                             \
    $(LWIP_CPPFLAGS)                                    \
    $(SOCKETS_CPPFLAGS)                                 \
    $(PTHREAD_CFLAGS)                                   \
    $(NULL)

CHIP_LDADD                                            = \
    $(top_builddir)/src/system/libSystemLayer.a         \
    $(top_builddir)/src/lib/support/libSupportLayer.a   \
    $(NULL)

COMMON_LDADD                                           = \
    libSystemLayerTests.a                                \
    $(COMMON_LDFLAGS)                                    \
    $(CHIP_LDADD)                                        \
    $(NLUNIT_TEST_LDFLAGS) $(NLUNIT_TEST_LIBS)           \
    $(NLFAULTINJECTION_LDFLAGS) $(NLFAULTINJECTION_LIBS) \
    $(LWIP_LDFLAGS) $(LWIP_LIBS)                         \
    $(SOCKETS_LDFLAGS) $(SOCKETS_LIBS)                   \
    $(PTHREAD_CFLAGS) $(PTHREAD_LIBS)                    \
    $(NULL)

# Test applications that should be run when the 'check' target is run.

check_PROGRAMS                                        = \
    $(NULL)

check_SCRIPTS                                         = \
    $(NULL)

# The additional environment variables and their values that will be
# made available to all programs and scripts in TESTS.
TESTS_ENVIRONMENT                                      = \
    $(NULL)

if CHIP_DEVICE_LAYER_TARGET_ESP32

check_SCRIPTS                                         += \
    qemu_system_tests.sh                                 \
    $(NULL)

TESTS_ENVIRONMENT                                     += \
    abs_top_srcdir='$(abs_top_srcdir)'                   \
    abs_top_builddir='$(abs_top_builddir)'               \
    QEMU_TEST_TARGET=libSystemLayerTests.a               \
    $(NULL)

else # CHIP_DEVICE_LAYER_TARGET_ESP32

check_PROGRAMS                                       += \
    TestSystemErrorStr                                  \
    TestSystemObject                                    \
    TestSystemPacketBuffer                              \
    TestSystemTimer                                     \
    TestSystemWakeEvent                                 \
    TestTimeSource                                      \
    $(NULL)

endif # CHIP_DEVICE_LAYER_TARGET_ESP32

# Test applications and scripts that should be built and run when the
# 'check' target is run.

TESTS                                                 = \
    $(check_PROGRAMS)                                   \
    $(check_SCRIPTS)                                    \
    $(NULL)

# Source, compiler, and linker options for test programs.

TestSystemErrorStr_SOURCES                            = TestSystemErrorStrDriver.cpp
TestSystemErrorStr_LDADD                              = $(COMMON_LDADD)

TestSystemObject_SOURCES                              = TestSystemObjectDriver.cpp
TestSystemObject_LDADD                                = $(COMMON_LDADD)

TestSystemPacketBuffer_SOURCES                        = TestSystemPacketBufferDriver.cpp
TestSystemPacketBuffer_LDADD                          = $(COMMON_LDADD)

TestSystemTimer_SOURCES                               = TestSystemTimerDriver.cpp
TestSystemTimer_LDADD                                 = $(COMMON_LDADD)

TestSystemWakeEvent_SOURCES                           = TestSystemWakeEventDriver.cpp
TestSystemWakeEvent_LDADD                             = $(COMMON_LDADD)

TestTimeSource_SOURCES                               = TestTimeSourceDriver.cpp
TestTimeSource_LDADD                                 = $(COMMON_LDADD)

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
