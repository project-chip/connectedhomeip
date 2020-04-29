#
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
#      This file is the GNU automake template for the CHIP QR code
#      library.
#

include $(abs_top_nlbuild_autotools_dir)/automake/pre.am

SUBDIRS                    = tests

lib_LIBRARIES              = libSetupPayload.a

libSetupPayload_adir             = $(includedir)/setup_payload

libSetupPayload_a_CPPFLAGS                                      = \
    -I$(top_srcdir)/src                                     \
    -I$(top_srcdir)/src/lib                                 \
    -I$(top_srcdir)/src/system                              \
    -I$(top_srcdir)/src/include/                            \
    $(NULL)

libSetupPayload_a_SOURCES                                       = \
    ManualSetupPayloadGenerator.cpp                         \
    ManualSetupPayloadParser.cpp                            \
    QRCodeSetupPayloadGenerator.cpp                         \
    SetupPayload.cpp                                        \
    Base41.cpp                                              \
    QRCodeSetupPayloadParser.cpp                            \
   $(NULL)

dist_libSetupPayload_a_HEADERS                                  = \
    ManualSetupPayloadGenerator.h                           \
    ManualSetupPayloadParser.h                              \
    QRCodeSetupPayloadGenerator.h                           \
    SetupPayload.h                                          \
    Base41.h                                                \
    QRCodeSetupPayloadParser.h                              \
    $(NULL)

include $(abs_top_nlbuild_autotools_dir)/automake/post.am
