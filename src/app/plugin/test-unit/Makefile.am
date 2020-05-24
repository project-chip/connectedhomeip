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
#      This file governs what comes out of the app directory, chiefly
#        the data model and application layer for the CHIP interaction
#        model.
#

include $(abs_top_nlbuild_autotools_dir)/automake/pre.am

if CHIP_BUILD_TESTS

# C/C++ preprocessor option flags that will apply to all compiled
# objects in this makefile.

AM_CPPFLAGS                                    = \
    -I$(top_srcdir)/src/app/chip-zcl             \
    -I$(top_srcdir)/src/app/gen                  \
    $(NULL)

lib_LIBRARIES                                  = \
    libChipZclUnitTests.a                        \
    $(NULL)

# TODO: pick a place to "install": this
EXTRA_DIST = ChipZclUnitTests.h

# plugins we're unit testing
libChipZclUnitTests_a_SOURCES                            = \
    ../../gen/gen-callback-stubs.c                         \
    ../../gen/gen-command-handler.c                        \
    ../binding-mock/mock.c                                 \
    ../core-api/core-api.c                                 \
    ../cluster-server-basic/basic-server.c                 \
    ../cluster-server-level-control/level-control-server.c \
    ../cluster-server-identify/identify-server.c           \
    ../cluster-server-on-off/on-off-server.c               \
    ../codec-simple/codec-simple.c                         \
    ../core-message-dispatch/dispatch.c                    \
    ../core-message-dispatch/general-command-handler.c     \
    ../core-data-model/zcl-data-model.c                    \
    $(NULL)

# individual tests
libChipZclUnitTests_a_SOURCES                           += \
    cluster-server-basic-test.c                            \
    cluster-server-identify-test.c                         \
    cluster-server-level-control-test.c                    \
    cluster-server-on-off-test.c                           \
    codec-simple-test.c                                    \
    core-data-model-test.c                                 \
    core-message-dispatch-test.c                           \
    $(NULL)

check_PROGRAMS                                 = \
    ChipZclUnitTestsDriver                       \
    $(NULL)

ChipZclUnitTestsDriver_LDADD                   = libChipZclUnitTests.a

TESTS = $(check_PROGRAMS)

######################
# coverage boilerplate, TODO: move to an include
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
