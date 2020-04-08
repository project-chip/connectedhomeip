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

lib_LIBRARIES              = libChipCrypto.a

libChipCrypto_adir             = $(includedir)/crypto

libChipCrypto_a_CPPFLAGS                                      = \
    -I$(top_srcdir)/src/lib                                     \
    -I$(top_srcdir)/src/system                                  \
    $(NULL)

libChipCrypto_a_SOURCES                                       = \
    CHIPOpenSSL.c                                               \
    CHIPBase+Crypto.c                                           \
   $(NULL)

dist_libChipCrypto_a_HEADERS                                 = \
    CHIPCrypto.h                                               \
    CHIPBase+CompilerAbstraction.h                             \
    CHIPCryptoPAL.h                                            \
    $(NULL)

$(RECURSIVE_TARGETS): $(lib_LIBRARIES)

include $(abs_top_nlbuild_autotools_dir)/automake/post.am
