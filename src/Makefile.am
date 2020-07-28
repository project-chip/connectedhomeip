#
#    Copyright (c) 2020 Project CHIP Authors
#    Copyright (c) 2014-2018 Nest Labs, Inc.
#    Copyright (c) 2018 Google LLC
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
#      This file is the GNU automake template for the CHIP SDK
#      source code.
#

include $(abs_top_nlbuild_autotools_dir)/automake/pre.am


BLE_SUBDIRS                       = \
    ble                             \
    $(NULL)

MAYBE_BLE_SUBDIRS                 = \
    $(NULL)

if CONFIG_NETWORK_LAYER_BLE
MAYBE_BLE_SUBDIRS                += \
    $(BLE_SUBDIRS)                  \
    $(NULL)
endif # CONFIG_NETWORK_LAYER_BLE

MAYBE_PLATFORM_SUBDIRS            = \
    $(NULL)

if CONFIG_DEVICE_LAYER
MAYBE_PLATFORM_SUBDIRS           += \
    platform                        \
    $(NULL)
endif # CONFIG_DEVICE_LAYER

MAYBE_QRCODE_TOOL                 = \
    $(NULL)

if !CONFIG_DEVICE_LAYER
MAYBE_QRCODETOOL_SUBDIR           = \
    qrcodetool                      \
    $(NULL)
endif

SETUP_PAYLOAD_SUBDIRS             = \
    setup_payload/java              \
    $(NULL)

MAYBE_SETUP_PAYLOAD_SUBDIRS       = \
    $(NULL)

if CHIP_WITH_ANDROID
MAYBE_SETUP_PAYLOAD_SUBDIRS      += \
    $(SETUP_PAYLOAD_SUBDIRS)        \
    $(NULL)
endif # CHIP_WITH_ANDROID

CONTROLLER_SUBDIRS                = \
    controller/java                 \
    $(NULL)

MAYBE_CONTROLLER_SUBDIRS          = \
    $(NULL)

if CHIP_WITH_ANDROID
MAYBE_CONTROLLER_SUBDIRS         += \
    $(CONTROLLER_SUBDIRS)           \
    $(NULL)
endif # CHIP_WITH_ANDROID

# Always package (e.g. for 'make dist') these subdirectories.

DIST_SUBDIRS                      = \
    app                             \
    include                         \
    lwip                            \
    system                          \
    $(BLE_SUBDIRS)                  \
    inet                            \
    lib                             \
    setup_payload                   \
		$(CONTROLLER_SUBDIRS)           \
    $(SETUP_PAYLOAD_SUBDIRS)        \
    crypto                          \
    platform                        \
    qrcodetool                      \
    transport                       \
    $(NULL)

# Always build (e.g. for 'make all') these subdirectories.

SUBDIRS                           = \
    app                             \
    include                         \
    lwip                            \
    system                          \
    inet                            \
    lib                             \
    $(MAYBE_CONTROLLER_SUBDIRS)     \
    setup_payload                   \
    $(MAYBE_SETUP_PAYLOAD_SUBDIRS)  \
    crypto                          \
    transport                       \
    $(MAYBE_BLE_SUBDIRS)            \
    $(MAYBE_PLATFORM_SUBDIRS)       \
    $(MAYBE_QRCODETOOL_SUBDIR)      \
    $(NULL)

if CHIP_BUILD_TESTS
if CHIP_BUILD_COVERAGE
if CHIP_BUILD_COVERAGE_REPORTS
# The bundle should positively be qualified with the absolute build
# path. Otherwise, VPATH will get auto-prefixed to it if there is
# already such a directory in the non-colocated source tree.

CHIP_COVERAGE_BUNDLE              = ${abs_builddir}/${PACKAGE}${NL_COVERAGE_BUNDLE_SUFFIX}
CHIP_COVERAGE_INFO                = ${CHIP_COVERAGE_BUNDLE}/${PACKAGE}${NL_COVERAGE_INFO_SUFFIX}

$(CHIP_COVERAGE_BUNDLE):
	$(call create-directory)

$(CHIP_COVERAGE_INFO): check | $(CHIP_COVERAGE_BUNDLE)
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
