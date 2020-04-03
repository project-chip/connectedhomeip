#
#
#    Copyright (c) 2020 Project CHIP Authors
#    Copyright (c) 2014-2017 Nest Labs, Inc.
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
#      This file is the GNU automake template for the CHIP BleLayer
#      library.
#

include $(abs_top_nlbuild_autotools_dir)/automake/pre.am

SUBDIRS                             = \
    tests                             \
    $(NULL)

include BleLayer.am

lib_LIBRARIES                       = libBleLayer.a

libBleLayer_a_CPPFLAGS              = \
    -I$(top_srcdir)/src               \
    -I$(top_srcdir)/src/include       \
    -I$(top_srcdir)/src/lib           \
    -I$(top_srcdir)/src/lib/core      \
    -I$(top_srcdir)/src/system        \
    $(LWIP_CPPFLAGS)                  \
    $(NULL)

libBleLayer_a_SOURCES               = $(CHIP_BUILD_BLE_LAYER_SOURCE_FILES)

libBleLayer_adir                    = $(includedir)/ble

dist_libBleLayer_a_HEADERS          = $(CHIP_BUILD_BLE_LAYER_HEADER_FILES)

$(RECURSIVE_TARGETS): $(lib_LIBRARIES)

include $(abs_top_nlbuild_autotools_dir)/automake/post.am
