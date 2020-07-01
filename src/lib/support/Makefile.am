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
#      This file is the GNU automake template for the CHIP support
#      library.
#

include $(abs_top_nlbuild_autotools_dir)/automake/pre.am

SUBDIRS                             = \
    tests                             \
    $(NULL)

include SupportLayer.am

lib_LIBRARIES                       = libSupportLayer.a

libSupportLayer_a_CPPFLAGS                    = \
    -I$(top_srcdir)/src                         \
    -I$(top_srcdir)/src/lib                     \
    -I$(top_srcdir)/src/include                 \
    $(LWIP_CPPFLAGS)                            \
    $(NLASSERT_CPPFLAGS)                        \
    $(NLFAULTINJECTION_CPPFLAGS)                \
    $(NULL)

libSupportLayer_a_SOURCES           = $(CHIP_BUILD_SUPPORT_LAYER_SOURCE_FILES)

libSupportLayer_adir                = $(includedir)/support

dist_libSupportLayer_a_HEADERS      = $(CHIP_BUILD_SUPPORT_LAYER_HEADER_FILES)

Loggingdir                          = $(includedir)/support/logging

dist_Logging_HEADERS                = $(CHIP_BUILD_SUPPORT_LAYER_LOGGING_HEADER_FILES)

EXTRA_DIST                          = $(CHIP_BUILD_SUPPORT_LAYER_VERHOEFF_HEADER_FILES)

include $(abs_top_nlbuild_autotools_dir)/automake/post.am
