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
#    Description:
#      This file is the GNU automake template for the CHIP in-package,
#      mbedTLS library.
#
#

include $(abs_top_nlbuild_autotools_dir)/automake/pre.am

if CHIP_WITH_MBEDTLS_INTERNAL

lib_LIBRARIES                                                                  = libmbedtls.a

# Never include the mbedTLS sources in a distribution; they always will get pulled from upstream
# via git operations, hence the 'nodist_' qualifier.

nodist_libmbedtls_a_SOURCES                                                    = \
    repo/library/aes.c                      \
    repo/library/pem.c                      \
    repo/library/aesni.c                    \
    repo/library/pk.c                       \
    repo/library/arc4.c                     \
    repo/library/pkcs11.c                   \
    repo/library/aria.c                     \
    repo/library/pkcs12.c                   \
    repo/library/asn1parse.c                \
    repo/library/pkcs5.c                    \
    repo/library/asn1write.c                \
    repo/library/pkparse.c                  \
    repo/library/base64.c                   \
    repo/library/pk_wrap.c                  \
    repo/library/bignum.c                   \
    repo/library/pkwrite.c                  \
    repo/library/blowfish.c                 \
    repo/library/platform.c                 \
    repo/library/camellia.c                 \
    repo/library/platform_util.c            \
    repo/library/ccm.c                      \
    repo/library/poly1305.c                 \
    repo/library/certs.c                    \
    repo/library/chacha20.c                 \
    repo/library/chachapoly.c               \
    repo/library/cipher.c                   \
    repo/library/cipher_wrap.c              \
    repo/library/cmac.c                     \
    repo/library/ripemd160.c                \
    repo/library/ctr_drbg.c                 \
    repo/library/rsa.c                      \
    repo/library/debug.c                    \
    repo/library/rsa_internal.c             \
    repo/library/des.c                      \
    repo/library/sha1.c                     \
    repo/library/dhm.c                      \
    repo/library/sha256.c                   \
    repo/library/ecdh.c                     \
    repo/library/sha512.c                   \
    repo/library/ecdsa.c                    \
    repo/library/ssl_cache.c                \
    repo/library/ecjpake.c                  \
    repo/library/ssl_ciphersuites.c         \
    repo/library/ecp.c                      \
    repo/library/ssl_cli.c                  \
    repo/library/ecp_curves.c               \
    repo/library/ssl_cookie.c               \
    repo/library/entropy.c                  \
    repo/library/ssl_srv.c                  \
    repo/library/error.c                    \
    repo/library/ssl_ticket.c               \
    repo/library/gcm.c                      \
    repo/library/ssl_tls.c                  \
    repo/library/havege.c                   \
    repo/library/threading.c                \
    repo/library/hkdf.c                     \
    repo/library/hmac_drbg.c                \
    repo/library/version.c                  \
    repo/library/md2.c                      \
    repo/library/version_features.c         \
    repo/library/md4.c                      \
    repo/library/x509.c                     \
    repo/library/md5.c                      \
    repo/library/x509_create.c              \
    repo/library/md.c                       \
    repo/library/x509_crl.c                 \
    repo/library/memory_buffer_alloc.c      \
    repo/library/x509_csr.c                 \
    repo/library/nist_kw.c                  \
    repo/library/x509write_crt.c            \
    repo/library/oid.c                      \
    repo/library/x509write_csr.c            \
    repo/library/padlock.c                  \
    repo/library/xtea.c                     \
    repo/library/md_wrap.c                  \
    $(NULL)

mbedtls_includedir = $(includedir)/mbedtls

mbedtls_include_HEADERS =                   \
    repo/include/mbedtls/aes.h              \
    repo/include/mbedtls/aesni.h            \
    repo/include/mbedtls/arc4.h             \
    repo/include/mbedtls/aria.h             \
    repo/include/mbedtls/asn1.h             \
    repo/include/mbedtls/asn1write.h        \
    repo/include/mbedtls/base64.h           \
    repo/include/mbedtls/bignum.h           \
    repo/include/mbedtls/blowfish.h         \
    repo/include/mbedtls/bn_mul.h           \
    repo/include/mbedtls/camellia.h         \
    repo/include/mbedtls/ccm.h              \
    repo/include/mbedtls/certs.h            \
    repo/include/mbedtls/chacha20.h         \
    repo/include/mbedtls/chachapoly.h       \
    repo/include/mbedtls/check_config.h     \
    repo/include/mbedtls/cipher.h           \
    repo/include/mbedtls/cipher_internal.h  \
    repo/include/mbedtls/cmac.h             \
    repo/include/mbedtls/compat-1.3.h       \
    repo/include/mbedtls/config.h           \
    repo/include/mbedtls/ctr_drbg.h         \
    repo/include/mbedtls/debug.h            \
    repo/include/mbedtls/des.h              \
    repo/include/mbedtls/dhm.h              \
    repo/include/mbedtls/ecdh.h             \
    repo/include/mbedtls/ecdsa.h            \
    repo/include/mbedtls/ecjpake.h          \
    repo/include/mbedtls/ecp.h              \
    repo/include/mbedtls/ecp_internal.h     \
    repo/include/mbedtls/entropy.h          \
    repo/include/mbedtls/entropy_poll.h     \
    repo/include/mbedtls/error.h            \
    repo/include/mbedtls/gcm.h              \
    repo/include/mbedtls/havege.h           \
    repo/include/mbedtls/hkdf.h             \
    repo/include/mbedtls/hmac_drbg.h        \
    repo/include/mbedtls/md2.h              \
    repo/include/mbedtls/md4.h              \
    repo/include/mbedtls/md5.h              \
    repo/include/mbedtls/md.h               \
    repo/include/mbedtls/md_internal.h      \
    repo/include/mbedtls/memory_buffer_alloc.h \
    repo/include/mbedtls/net.h              \
    repo/include/mbedtls/net_sockets.h      \
    repo/include/mbedtls/nist_kw.h          \
    repo/include/mbedtls/oid.h              \
    repo/include/mbedtls/padlock.h          \
    repo/include/mbedtls/pem.h              \
    repo/include/mbedtls/pkcs11.h           \
    repo/include/mbedtls/pkcs12.h           \
    repo/include/mbedtls/pkcs5.h            \
    repo/include/mbedtls/pk.h               \
    repo/include/mbedtls/pk_internal.h      \
    repo/include/mbedtls/platform.h         \
    repo/include/mbedtls/platform_time.h    \
    repo/include/mbedtls/platform_util.h    \
    repo/include/mbedtls/poly1305.h         \
    repo/include/mbedtls/psa_util.h         \
    repo/include/mbedtls/ripemd160.h        \
    repo/include/mbedtls/rsa.h              \
    repo/include/mbedtls/rsa_internal.h     \
    repo/include/mbedtls/sha1.h             \
    repo/include/mbedtls/sha256.h           \
    repo/include/mbedtls/sha512.h           \
    repo/include/mbedtls/ssl_cache.h        \
    repo/include/mbedtls/ssl_ciphersuites.h \
    repo/include/mbedtls/ssl_cookie.h       \
    repo/include/mbedtls/ssl.h              \
    repo/include/mbedtls/ssl_internal.h     \
    repo/include/mbedtls/ssl_ticket.h       \
    repo/include/mbedtls/threading.h        \
    repo/include/mbedtls/timing.h           \
    repo/include/mbedtls/version.h          \
    repo/include/mbedtls/x509_crl.h         \
    repo/include/mbedtls/x509_crt.h         \
    repo/include/mbedtls/x509_csr.h         \
    repo/include/mbedtls/x509.h             \
    repo/include/mbedtls/xtea.h             \
    $(NULL)

if !CONFIG_DEVICE_LAYER
nodist_libmbedtls_a_SOURCES                                                   += \
    repo/library/entropy_poll.c             \
    repo/library/timing.c                   \
    $(NULL)
endif

if CHIP_DEVICE_LAYER_TARGET_DARWIN
nodist_libmbedtls_a_SOURCES                                                   += \
    repo/library/entropy_poll.c             \
    repo/library/timing.c                   \
    $(NULL)
endif

if CHIP_DEVICE_LAYER_TARGET_LINUX
nodist_libmbedtls_a_SOURCES                                                   += \
    repo/library/entropy_poll.c             \
    repo/library/timing.c                   \
    $(NULL)
endif

libmbedtls_a_CPPFLAGS                                                          = \
    -I$(top_srcdir)/third_party/mbedtls/repo/include                             \
    $(MBEDTLS_CPPFLAGS)                                                          \
    $(NULL)

endif # CHIP_WITH_MBEDTLS_INTERNAL

include $(abs_top_nlbuild_autotools_dir)/automake/post.am
