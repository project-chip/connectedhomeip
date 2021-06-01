# Install script for directory: /Users/ritikananda/AndroidStudioProjects/connectedhomeipproject/third_party/ot-commissioner/repo/third_party/mbedtls/repo/include

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/usr/local")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Release")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Install shared libraries without execute permission?
if(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)
  set(CMAKE_INSTALL_SO_NO_EXE "0")
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "TRUE")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/mbedtls" TYPE FILE PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ FILES
    "/Users/ritikananda/AndroidStudioProjects/connectedhomeipproject/third_party/ot-commissioner/repo/third_party/mbedtls/repo/include/mbedtls/aes.h"
    "/Users/ritikananda/AndroidStudioProjects/connectedhomeipproject/third_party/ot-commissioner/repo/third_party/mbedtls/repo/include/mbedtls/aesni.h"
    "/Users/ritikananda/AndroidStudioProjects/connectedhomeipproject/third_party/ot-commissioner/repo/third_party/mbedtls/repo/include/mbedtls/arc4.h"
    "/Users/ritikananda/AndroidStudioProjects/connectedhomeipproject/third_party/ot-commissioner/repo/third_party/mbedtls/repo/include/mbedtls/aria.h"
    "/Users/ritikananda/AndroidStudioProjects/connectedhomeipproject/third_party/ot-commissioner/repo/third_party/mbedtls/repo/include/mbedtls/asn1.h"
    "/Users/ritikananda/AndroidStudioProjects/connectedhomeipproject/third_party/ot-commissioner/repo/third_party/mbedtls/repo/include/mbedtls/asn1write.h"
    "/Users/ritikananda/AndroidStudioProjects/connectedhomeipproject/third_party/ot-commissioner/repo/third_party/mbedtls/repo/include/mbedtls/base64.h"
    "/Users/ritikananda/AndroidStudioProjects/connectedhomeipproject/third_party/ot-commissioner/repo/third_party/mbedtls/repo/include/mbedtls/bignum.h"
    "/Users/ritikananda/AndroidStudioProjects/connectedhomeipproject/third_party/ot-commissioner/repo/third_party/mbedtls/repo/include/mbedtls/blowfish.h"
    "/Users/ritikananda/AndroidStudioProjects/connectedhomeipproject/third_party/ot-commissioner/repo/third_party/mbedtls/repo/include/mbedtls/bn_mul.h"
    "/Users/ritikananda/AndroidStudioProjects/connectedhomeipproject/third_party/ot-commissioner/repo/third_party/mbedtls/repo/include/mbedtls/camellia.h"
    "/Users/ritikananda/AndroidStudioProjects/connectedhomeipproject/third_party/ot-commissioner/repo/third_party/mbedtls/repo/include/mbedtls/ccm.h"
    "/Users/ritikananda/AndroidStudioProjects/connectedhomeipproject/third_party/ot-commissioner/repo/third_party/mbedtls/repo/include/mbedtls/certs.h"
    "/Users/ritikananda/AndroidStudioProjects/connectedhomeipproject/third_party/ot-commissioner/repo/third_party/mbedtls/repo/include/mbedtls/chacha20.h"
    "/Users/ritikananda/AndroidStudioProjects/connectedhomeipproject/third_party/ot-commissioner/repo/third_party/mbedtls/repo/include/mbedtls/chachapoly.h"
    "/Users/ritikananda/AndroidStudioProjects/connectedhomeipproject/third_party/ot-commissioner/repo/third_party/mbedtls/repo/include/mbedtls/check_config.h"
    "/Users/ritikananda/AndroidStudioProjects/connectedhomeipproject/third_party/ot-commissioner/repo/third_party/mbedtls/repo/include/mbedtls/cipher.h"
    "/Users/ritikananda/AndroidStudioProjects/connectedhomeipproject/third_party/ot-commissioner/repo/third_party/mbedtls/repo/include/mbedtls/cipher_internal.h"
    "/Users/ritikananda/AndroidStudioProjects/connectedhomeipproject/third_party/ot-commissioner/repo/third_party/mbedtls/repo/include/mbedtls/cmac.h"
    "/Users/ritikananda/AndroidStudioProjects/connectedhomeipproject/third_party/ot-commissioner/repo/third_party/mbedtls/repo/include/mbedtls/compat-1.3.h"
    "/Users/ritikananda/AndroidStudioProjects/connectedhomeipproject/third_party/ot-commissioner/repo/third_party/mbedtls/repo/include/mbedtls/config.h"
    "/Users/ritikananda/AndroidStudioProjects/connectedhomeipproject/third_party/ot-commissioner/repo/third_party/mbedtls/repo/include/mbedtls/ctr_drbg.h"
    "/Users/ritikananda/AndroidStudioProjects/connectedhomeipproject/third_party/ot-commissioner/repo/third_party/mbedtls/repo/include/mbedtls/debug.h"
    "/Users/ritikananda/AndroidStudioProjects/connectedhomeipproject/third_party/ot-commissioner/repo/third_party/mbedtls/repo/include/mbedtls/des.h"
    "/Users/ritikananda/AndroidStudioProjects/connectedhomeipproject/third_party/ot-commissioner/repo/third_party/mbedtls/repo/include/mbedtls/dhm.h"
    "/Users/ritikananda/AndroidStudioProjects/connectedhomeipproject/third_party/ot-commissioner/repo/third_party/mbedtls/repo/include/mbedtls/ecdh.h"
    "/Users/ritikananda/AndroidStudioProjects/connectedhomeipproject/third_party/ot-commissioner/repo/third_party/mbedtls/repo/include/mbedtls/ecdsa.h"
    "/Users/ritikananda/AndroidStudioProjects/connectedhomeipproject/third_party/ot-commissioner/repo/third_party/mbedtls/repo/include/mbedtls/ecjpake.h"
    "/Users/ritikananda/AndroidStudioProjects/connectedhomeipproject/third_party/ot-commissioner/repo/third_party/mbedtls/repo/include/mbedtls/ecp.h"
    "/Users/ritikananda/AndroidStudioProjects/connectedhomeipproject/third_party/ot-commissioner/repo/third_party/mbedtls/repo/include/mbedtls/ecp_internal.h"
    "/Users/ritikananda/AndroidStudioProjects/connectedhomeipproject/third_party/ot-commissioner/repo/third_party/mbedtls/repo/include/mbedtls/entropy.h"
    "/Users/ritikananda/AndroidStudioProjects/connectedhomeipproject/third_party/ot-commissioner/repo/third_party/mbedtls/repo/include/mbedtls/entropy_poll.h"
    "/Users/ritikananda/AndroidStudioProjects/connectedhomeipproject/third_party/ot-commissioner/repo/third_party/mbedtls/repo/include/mbedtls/error.h"
    "/Users/ritikananda/AndroidStudioProjects/connectedhomeipproject/third_party/ot-commissioner/repo/third_party/mbedtls/repo/include/mbedtls/gcm.h"
    "/Users/ritikananda/AndroidStudioProjects/connectedhomeipproject/third_party/ot-commissioner/repo/third_party/mbedtls/repo/include/mbedtls/havege.h"
    "/Users/ritikananda/AndroidStudioProjects/connectedhomeipproject/third_party/ot-commissioner/repo/third_party/mbedtls/repo/include/mbedtls/hkdf.h"
    "/Users/ritikananda/AndroidStudioProjects/connectedhomeipproject/third_party/ot-commissioner/repo/third_party/mbedtls/repo/include/mbedtls/hmac_drbg.h"
    "/Users/ritikananda/AndroidStudioProjects/connectedhomeipproject/third_party/ot-commissioner/repo/third_party/mbedtls/repo/include/mbedtls/md.h"
    "/Users/ritikananda/AndroidStudioProjects/connectedhomeipproject/third_party/ot-commissioner/repo/third_party/mbedtls/repo/include/mbedtls/md2.h"
    "/Users/ritikananda/AndroidStudioProjects/connectedhomeipproject/third_party/ot-commissioner/repo/third_party/mbedtls/repo/include/mbedtls/md4.h"
    "/Users/ritikananda/AndroidStudioProjects/connectedhomeipproject/third_party/ot-commissioner/repo/third_party/mbedtls/repo/include/mbedtls/md5.h"
    "/Users/ritikananda/AndroidStudioProjects/connectedhomeipproject/third_party/ot-commissioner/repo/third_party/mbedtls/repo/include/mbedtls/md_internal.h"
    "/Users/ritikananda/AndroidStudioProjects/connectedhomeipproject/third_party/ot-commissioner/repo/third_party/mbedtls/repo/include/mbedtls/memory_buffer_alloc.h"
    "/Users/ritikananda/AndroidStudioProjects/connectedhomeipproject/third_party/ot-commissioner/repo/third_party/mbedtls/repo/include/mbedtls/net.h"
    "/Users/ritikananda/AndroidStudioProjects/connectedhomeipproject/third_party/ot-commissioner/repo/third_party/mbedtls/repo/include/mbedtls/net_sockets.h"
    "/Users/ritikananda/AndroidStudioProjects/connectedhomeipproject/third_party/ot-commissioner/repo/third_party/mbedtls/repo/include/mbedtls/nist_kw.h"
    "/Users/ritikananda/AndroidStudioProjects/connectedhomeipproject/third_party/ot-commissioner/repo/third_party/mbedtls/repo/include/mbedtls/oid.h"
    "/Users/ritikananda/AndroidStudioProjects/connectedhomeipproject/third_party/ot-commissioner/repo/third_party/mbedtls/repo/include/mbedtls/padlock.h"
    "/Users/ritikananda/AndroidStudioProjects/connectedhomeipproject/third_party/ot-commissioner/repo/third_party/mbedtls/repo/include/mbedtls/pem.h"
    "/Users/ritikananda/AndroidStudioProjects/connectedhomeipproject/third_party/ot-commissioner/repo/third_party/mbedtls/repo/include/mbedtls/pk.h"
    "/Users/ritikananda/AndroidStudioProjects/connectedhomeipproject/third_party/ot-commissioner/repo/third_party/mbedtls/repo/include/mbedtls/pk_internal.h"
    "/Users/ritikananda/AndroidStudioProjects/connectedhomeipproject/third_party/ot-commissioner/repo/third_party/mbedtls/repo/include/mbedtls/pkcs11.h"
    "/Users/ritikananda/AndroidStudioProjects/connectedhomeipproject/third_party/ot-commissioner/repo/third_party/mbedtls/repo/include/mbedtls/pkcs12.h"
    "/Users/ritikananda/AndroidStudioProjects/connectedhomeipproject/third_party/ot-commissioner/repo/third_party/mbedtls/repo/include/mbedtls/pkcs5.h"
    "/Users/ritikananda/AndroidStudioProjects/connectedhomeipproject/third_party/ot-commissioner/repo/third_party/mbedtls/repo/include/mbedtls/platform.h"
    "/Users/ritikananda/AndroidStudioProjects/connectedhomeipproject/third_party/ot-commissioner/repo/third_party/mbedtls/repo/include/mbedtls/platform_time.h"
    "/Users/ritikananda/AndroidStudioProjects/connectedhomeipproject/third_party/ot-commissioner/repo/third_party/mbedtls/repo/include/mbedtls/platform_util.h"
    "/Users/ritikananda/AndroidStudioProjects/connectedhomeipproject/third_party/ot-commissioner/repo/third_party/mbedtls/repo/include/mbedtls/poly1305.h"
    "/Users/ritikananda/AndroidStudioProjects/connectedhomeipproject/third_party/ot-commissioner/repo/third_party/mbedtls/repo/include/mbedtls/ripemd160.h"
    "/Users/ritikananda/AndroidStudioProjects/connectedhomeipproject/third_party/ot-commissioner/repo/third_party/mbedtls/repo/include/mbedtls/rsa.h"
    "/Users/ritikananda/AndroidStudioProjects/connectedhomeipproject/third_party/ot-commissioner/repo/third_party/mbedtls/repo/include/mbedtls/rsa_internal.h"
    "/Users/ritikananda/AndroidStudioProjects/connectedhomeipproject/third_party/ot-commissioner/repo/third_party/mbedtls/repo/include/mbedtls/sha1.h"
    "/Users/ritikananda/AndroidStudioProjects/connectedhomeipproject/third_party/ot-commissioner/repo/third_party/mbedtls/repo/include/mbedtls/sha256.h"
    "/Users/ritikananda/AndroidStudioProjects/connectedhomeipproject/third_party/ot-commissioner/repo/third_party/mbedtls/repo/include/mbedtls/sha512.h"
    "/Users/ritikananda/AndroidStudioProjects/connectedhomeipproject/third_party/ot-commissioner/repo/third_party/mbedtls/repo/include/mbedtls/ssl.h"
    "/Users/ritikananda/AndroidStudioProjects/connectedhomeipproject/third_party/ot-commissioner/repo/third_party/mbedtls/repo/include/mbedtls/ssl_cache.h"
    "/Users/ritikananda/AndroidStudioProjects/connectedhomeipproject/third_party/ot-commissioner/repo/third_party/mbedtls/repo/include/mbedtls/ssl_ciphersuites.h"
    "/Users/ritikananda/AndroidStudioProjects/connectedhomeipproject/third_party/ot-commissioner/repo/third_party/mbedtls/repo/include/mbedtls/ssl_cookie.h"
    "/Users/ritikananda/AndroidStudioProjects/connectedhomeipproject/third_party/ot-commissioner/repo/third_party/mbedtls/repo/include/mbedtls/ssl_internal.h"
    "/Users/ritikananda/AndroidStudioProjects/connectedhomeipproject/third_party/ot-commissioner/repo/third_party/mbedtls/repo/include/mbedtls/ssl_ticket.h"
    "/Users/ritikananda/AndroidStudioProjects/connectedhomeipproject/third_party/ot-commissioner/repo/third_party/mbedtls/repo/include/mbedtls/threading.h"
    "/Users/ritikananda/AndroidStudioProjects/connectedhomeipproject/third_party/ot-commissioner/repo/third_party/mbedtls/repo/include/mbedtls/timing.h"
    "/Users/ritikananda/AndroidStudioProjects/connectedhomeipproject/third_party/ot-commissioner/repo/third_party/mbedtls/repo/include/mbedtls/version.h"
    "/Users/ritikananda/AndroidStudioProjects/connectedhomeipproject/third_party/ot-commissioner/repo/third_party/mbedtls/repo/include/mbedtls/x509.h"
    "/Users/ritikananda/AndroidStudioProjects/connectedhomeipproject/third_party/ot-commissioner/repo/third_party/mbedtls/repo/include/mbedtls/x509_crl.h"
    "/Users/ritikananda/AndroidStudioProjects/connectedhomeipproject/third_party/ot-commissioner/repo/third_party/mbedtls/repo/include/mbedtls/x509_crt.h"
    "/Users/ritikananda/AndroidStudioProjects/connectedhomeipproject/third_party/ot-commissioner/repo/third_party/mbedtls/repo/include/mbedtls/x509_csr.h"
    "/Users/ritikananda/AndroidStudioProjects/connectedhomeipproject/third_party/ot-commissioner/repo/third_party/mbedtls/repo/include/mbedtls/xtea.h"
    )
endif()

