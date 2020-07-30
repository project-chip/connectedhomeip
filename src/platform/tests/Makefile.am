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
#      This is the GNU automake file for the CHIP crypto unit
#      and functional tests.
#

include $(abs_top_nlbuild_autotools_dir)/automake/pre.am

#
# Local headers to build against and distribute but not to install
# since they are not part of the package.
#
noinst_HEADERS                                 = \
    $(NULL)

#
# Other files we do want to distribute with the package.
#
EXTRA_DIST                                     = \
    $(NULL)

if CHIP_BUILD_TESTS
lib_LIBRARIES                                  = \
    libPlatformTests.a                           \
    $(NULL)

libPlatformTests_a_SOURCES                     = \
    TestPlatformMgr.cpp                          \
    TestPlatformTime.cpp                         \
    TestConfigurationMgr.cpp                     \
    $(NULL)

libPlatformTests_adir                          = $(includedir)/platform

dist_libPlatformTests_a_HEADERS                = \
    TestPlatformMgr.h                            \
    TestPlatformTime.h                           \
    TestConfigurationMgr.h                       \
    $(NULL)

# C/C++ preprocessor option flags that will apply to all compiled
# objects in this makefile.

AM_CPPFLAGS                                    = \
    -I$(builddir)/../Linux/gen                   \
    -I$(top_srcdir)/src/                         \
    -I$(top_srcdir)/src/lib                      \
    -I$(top_srcdir)/src/system                   \
    -I$(top_srcdir)/src/include/                 \
    -I$(top_srcdir)/src/platform                 \
    $(NLASSERT_CPPFLAGS)                         \
    $(NLFAULTINJECTION_CPPFLAGS)                 \
    $(NLUNIT_TEST_CPPFLAGS)                      \
    $(NULL)

if CHIP_DEVICE_LAYER_TARGET_LINUX
AM_CPPFLAGS                                   += \
    $(GIO_CFLAGS)                                \
    $(GIO_UNIX_CFLAGS)                           \
    $(NULL)

if CHIP_WITH_OT_BR_POSIX
AM_CPPFLAGS                                   += \
    $(DBUS_CFLAGS)                               \
    $(OT_BR_POSIX_CPPFLAGS)                      \
    $(NULL)

libPlatformTests_a_SOURCES += TestThreadStackMgr.cpp

dist_libPlatformTests_a_HEADERS                += \
    TestThreadStackMgr.h                          \
    $(NULL)

endif # CHIP_WITH_OT_BR_POSIX
endif # CHIP_DEVICE_LAYER_TARGET_LINUX


CHIP_LDADD                                      = \
    $(top_builddir)/src/platform/libDeviceLayer.a \
    $(top_builddir)/src/inet/libInetLayer.a       \
    $(top_builddir)/src/system/libSystemLayer.a   \
    $(top_builddir)/src/crypto/libChipCrypto.a    \
    $(top_builddir)/src/lib/support/libSupportLayer.a   \
    $(NULL)

if CHIP_DEVICE_LAYER_TARGET_LINUX

COMMON_LDADD                                          = \
    libPlatformTests.a                                  \
    $(CHIP_LDADD)                                       \
    $(GIO_CFLAGS) $(GIO_LIBS)                           \
    $(GIO_UNIX_CFLAGS) $(GIO_UNIX_LIBS)                 \
    $(NLFAULTINJECTION_LDFLAGS) $(NLFAULTINJECTION_LIBS)\
    $(NLUNIT_TEST_LDFLAGS) $(NLUNIT_TEST_LIBS)          \
    $(LWIP_LDFLAGS) $(LWIP_LIBS)                        \
    $(SOCKETS_LDFLAGS) $(SOCKETS_LIBS)                  \
    $(PTHREAD_CFLAGS) $(PTHREAD_LIBS)                   \
    $(NULL)

if CHIP_WITH_OT_BR_POSIX
COMMON_LDADD                                   += \
    $(DBUS_LIBS)                                  \
    $(OT_BR_POSIX_LDFLAGS) $(OT_BR_POSIX_LIBS)    \
    $(NULL)
endif # CHIP_WITH_OT_BR_POSIX
endif # CHIP_DEVICE_LAYER_TARGET_LINUX

# Test applications that should be run when the 'check' target is run.

check_PROGRAMS                                 = \
    TestPlatformTime                             \
    TestPlatformMgr                              \
    TestConfigurationMgr                         \
    $(NULL)

if CHIP_DEVICE_LAYER_TARGET_LINUX
if CHIP_WITH_OT_BR_POSIX
check_PROGRAMS += TestThreadStackMgr

TestThreadStackMgr_LDADD                       = $(COMMON_LDADD)
TestThreadStackMgr_SOURCES                     = TestThreadStackMgrDriver.cpp
endif
endif

# TODO Add TestConfigurationMgr to check_PROGRAMS when dependent core profile are merged 

# Test applications and scripts that should be built and run when the
# 'check' target is run.

# TestThreadStackMgr requires additional daemon to be launched
TESTS                                          = \
    TestPlatformTime                             \
    TestPlatformMgr                              \
    TestConfigurationMgr                         \
    $(NULL)

# The additional environment variables and their values that will be
# made available to all programs and scripts in TESTS.

TESTS_ENVIRONMENT                              = \
    $(NULL)

# Source, compiler, and linker options for test programs.

TestPlatformTime_LDADD                         = $(COMMON_LDADD)
TestPlatformTime_SOURCES                       = TestPlatformTimeDriver.cpp

TestPlatformMgr_LDADD                          = $(COMMON_LDADD)
TestPlatformMgr_SOURCES                        = TestPlatformMgrDriver.cpp

TestConfigurationMgr_LDADD                     = $(COMMON_LDADD)
TestConfigurationMgr_SOURCES                   = TestConfigurationMgrDriver.cpp

#
# Foreign make dependencies
#

NLFOREIGN_FILE_DEPENDENCIES                           = \
   $(CHIP_LDADD)                                        \
   $(NULL)

NLFOREIGN_SUBDIR_DEPENDENCIES                         = \
   $(NLUNIT_TEST_FOREIGN_SUBDIR_DEPENDENCY)             \
   $(NLFAULTINJECTION_FOREIGN_SUBDIR_DEPENDENCY)        \
   $(NULL)

if CHIP_BUILD_COVERAGE
CLEANFILES                                     = $(wildcard *.gcda *.gcno)

if CHIP_BUILD_COVERAGE_REPORTS
# The bundle should positively be qualified with the absolute build
# path. Otherwise, VPATH will get auto-prefixed to it if there is
# already such a directory in the non-colocated source tree.

CHIP_COVERAGE_BUNDLE                           = ${abs_builddir}/${PACKAGE}${NL_COVERAGE_BUNDLE_SUFFIX}
CHIP_COVERAGE_INFO                             = ${CHIP_COVERAGE_BUNDLE}/${PACKAGE}${NL_COVERAGE_INFO_SUFFIX}

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
