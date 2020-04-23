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
#      This file is the GNU automake template for the CHIP crypto
#      library.
#

include $(abs_top_nlbuild_autotools_dir)/automake/pre.am

SUBDIRS                    = tests

lib_LIBRARIES              = libChipCrypto.a

libChipCrypto_adir             = $(includedir)/crypto

libChipCrypto_a_CPPFLAGS            = \
    $(NLASSERT_CPPFLAGS)              \
    -I$(top_srcdir)/src               \
    -I$(top_srcdir)/src/lib           \
    -I$(top_srcdir)/src/system        \
    -I$(top_srcdir)/src/include/      \
    $(NULL)

libChipCrypto_a_SOURCES                                       = \
    $(NULL)

dist_libChipCrypto_a_HEADERS                                 = \
    CHIPCryptoPAL.h                                            \
    $(NULL)

if CHIP_CRYPTO_OPENSSL
libChipCrypto_a_SOURCES                                      += \
    CHIPBase+Crypto.c                                           \
    CHIPOpenSSL.c                                               \
    CHIPCryptoPALOpenSSL.cpp                                    \
    $(NULL)

dist_libChipCrypto_a_HEADERS                                += \
    CHIPCrypto.h                                               \
    CHIPBase+CompilerAbstraction.h                             \
    $(NULL)
endif

if CHIP_CRYPTO_MBEDTLS
libChipCrypto_a_SOURCES                                      += \
    CHIPCryptoPALmbedTLS.cpp                                    \
    $(NULL)
endif

include $(abs_top_nlbuild_autotools_dir)/automake/post.am
