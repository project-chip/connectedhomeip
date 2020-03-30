#
#
#    <COPYRIGHT>
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

lib_LIBRARIES              = libQrCode.a

libQrCode_adir             = $(includedir)/setup_payload

libQrCode_a_CPPFLAGS                                      = \
    -I$(top_srcdir)/src/lib                                 \
    -I$(top_srcdir)/src/system                              \
    $(NULL)

libQrCode_a_SOURCES                                       = \
    QRCodeSetupPayloadGenerator.cpp                         \
    SetupPayload.cpp                                        \
    Base45.cpp                                              \
   $(NULL)

dist_libQrCode_a_HEADERS                                 = \
    QRCodeSetupPayloadGenerator.h                          \
    SetupPayload.h                                         \
    Base45.h                                               \
    $(NULL)

include $(abs_top_nlbuild_autotools_dir)/automake/post.am
