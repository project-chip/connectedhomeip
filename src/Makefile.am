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

PLATFORM_SUBDIRS                  = \
    platform                        \
    $(NULL)

MAYBE_PLATFORM_SUBDIRS            = \
    $(NULL)

if CONFIG_DEVICE_LAYER
MAYBE_PLATFORM_SUBDIRS           += \
    $(PLATFORM_SUBDIRS)             \
    $(NULL)
endif # CONFIG_DEVICE_LAYER

# Always package (e.g. for 'make dist') these subdirectories.

DIST_SUBDIRS                      = \
    $(BLE_SUBDIRS)                  \
    inet                            \
    lib                             \
    lwip                            \
    setup_payload                   \
    system                          \
    $(PLATFORM_SUBDIRS)             \
    $(NULL)

# Always build (e.g. for 'make all') these subdirectories.

SUBDIRS                           = \
    lib/support                     \
    $(MAYBE_BLE_SUBDIRS)            \
    inet                            \
    lib                             \
    lwip                            \
    setup_payload                   \
    system                          \
    $(MAYBE_PLATFORM_SUBDIRS)       \
    $(NULL)

include $(abs_top_nlbuild_autotools_dir)/automake/post.am
