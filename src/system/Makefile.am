#
#    Copyright (c) 2020 Project CHIP Authors
#    Copyright (c) 2016-2017 Nest Labs, Inc.
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
#      This file is the GNU automake template for the CHIP system layer
#      library.
#

include $(abs_top_nlbuild_autotools_dir)/automake/pre.am

include SystemLayer.am

SUBDIRS                             = \
    tests                             \
    $(NULL)

EXTRA_DIST                          = \
    SystemLayerPrivate.h              \
    $(NULL)

lib_LIBRARIES                       = libSystemLayer.a

libSystemLayer_a_CPPFLAGS           = \
    -I$(top_srcdir)/src               \
    -I$(top_srcdir)/src/include       \
    -I$(top_srcdir)/src/system        \
    -I$(top_srcdir)/src/lib           \
    -I$(top_srcdir)/src/lib/core      \
    $(NLASSERT_CPPFLAGS)              \
    $(NLFAULTINJECTION_CPPFLAGS)      \
    $(LWIP_CPPFLAGS)                  \
    $(SOCKETS_CPPFLAGS)               \
    $(NULL)

libSystemLayer_a_SOURCES          = $(CHIP_BUILD_SYSTEM_LAYER_SOURCE_FILES)

libSystemLayer_adir               = $(includedir)/system

dist_libSystemLayer_a_HEADERS     = $(CHIP_BUILD_SYSTEM_LAYER_HEADER_FILES)

include $(abs_top_nlbuild_autotools_dir)/automake/post.am
