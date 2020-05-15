/* src/include/BuildConfig.h.in.  Generated from configure.ac by autoheader.  */

#ifndef CHIP_BUILD_CONFIG_H
#define CHIP_BUILD_CONFIG_H

/* Path to BLE platform config header file */
#undef BLE_PLATFORM_CONFIG_INCLUDE

/* Path to CHIP BLE Layer platform config header file */
#undef BLE_PROJECT_CONFIG_INCLUDE

/* Target platform name for CHIP Device Layer. */
#undef CHIP_DEVICE_LAYER_TARGET

/* Define to 1 if you want to build the CHIP Device Layer for Silicon Labs
   EFR32 platforms. */
#undef CHIP_DEVICE_LAYER_TARGET_EFR32

/* Define to 1 if you want to build the CHIP Device Layer for the Espressif
   ESP32. */
#undef CHIP_DEVICE_LAYER_TARGET_ESP32

/* Define to 1 if you want to build the CHIP Device Layer for Nordic nRF5*
   platforms. */
#undef CHIP_DEVICE_LAYER_TARGET_NRF5

/* Path to CHIP Device Layer platform config header file */
#undef CHIP_DEVICE_PLATFORM_CONFIG_INCLUDE

/* Path to CHIP Device Layer platform config header file */
#undef CHIP_DEVICE_PROJECT_CONFIG_INCLUDE

/* Define to 1 if you want to enable CHIPoBle Control Path and Throughput
   Test. */
#undef CHIP_ENABLE_BTP_CODEC_TEST

/* CHIP logging style */
#undef CHIP_LOGGING_STYLE

/* Define to 1 if you want to use CHIP with the Android logging system */
#undef CHIP_LOGGING_STYLE_ANDROID

/* Define to 1 if you want to use CHIP with an external (i.e. platform- and
   integrator-defined) logging system */
#undef CHIP_LOGGING_STYLE_EXTERNAL

/* Define to 1 if you want to use CHIP with a C Standard I/O Library-based
   logging system */
#undef CHIP_LOGGING_STYLE_STDIO

/* Define to 1 if you want to use CHIP with a C Standard I/O Library-based
   logging system,but override, at link-time an external (i.e. platform- and
   integrator-defined) logging system */
#undef CHIP_LOGGING_STYLE_STDIO_WEAK

/* Define to 1 if you want to use CHIP with a C Standard I/O Library-based
   logging system with timestamped log entries */
#undef CHIP_LOGGING_STYLE_STDIO_WITH_TIMESTAMPS

/* Path to CHIP platform config header file */
#undef CHIP_PLATFORM_CONFIG_INCLUDE

/* Path to CHIP Core platform config header file */
#undef CHIP_PROJECT_CONFIG_INCLUDE

/* Define to 1 if you want to use LwIP with CHIP System Layer. */
#undef CHIP_SYSTEM_CONFIG_USE_LWIP

/* Define to 1 if you want to use BSD sockets with CHIP System Layer. */
#undef CHIP_SYSTEM_CONFIG_USE_SOCKETS

/* CHIP target style */
#undef CHIP_TARGET_STYLE

/* Define to 1 if you want to use CHIP with an embedded (i.e. non-Unix-based)
   system */
#undef CHIP_TARGET_STYLE_EMBEDDED

/* Define to 1 if you want to use CHIP with a Unix-based system */
#undef CHIP_TARGET_STYLE_UNIX

/* Define to 1 to build CHIP with nlFaultInjection features */
#undef CHIP_WITH_NLFAULTINJECTION

/* Define to 1 to build CHIP with OpenSSL features */
#undef CHIP_WITH_OPENSSL

/* Define to 1 if you want to use the CHIP Device Layer. */
#undef CONFIG_DEVICE_LAYER

/* Define to 1 if you want to use CHIP with the Bluetooth Low Energy network
   stack. */
#undef CONFIG_NETWORK_LAYER_BLE

/* Define to 1 if you want to use CHIP with the Internet Protocol network
   stack. */
#undef CONFIG_NETWORK_LAYER_INET

/* CHIP target network stack(s) */
#undef CONFIG_TARGET_NETWORKS

/* Define to 1 if your C++ compiler doesn't accept -c and -o together. */
#undef CXX_NO_MINUS_C_MINUS_O

/* Define to 1 if the system has the type `clockid_t'. */
#undef HAVE_CLOCKID_T

/* Define to 1 if you have the `clock_gettime' function. */
#undef HAVE_CLOCK_GETTIME

/* Define to 1 if you have the `clock_settime' function. */
#undef HAVE_CLOCK_SETTIME

/* Define to 1 if you have the <ctype.h> header file. */
#undef HAVE_CTYPE_H

/* define if the compiler supports basic C++11 syntax */
#undef HAVE_CXX11

/* Define to 1 if you have the declaration of `CLOCK_BOOTTIME', and to 0 if
   you don't. */
#undef HAVE_DECL_CLOCK_BOOTTIME

/* Define to 1 if you have the declaration of `CLOCK_MONOTONIC', and to 0 if
   you don't. */
#undef HAVE_DECL_CLOCK_MONOTONIC

/* Define to 1 if you have the <dlfcn.h> header file. */
#undef HAVE_DLFCN_H

/* Define to 1 if you have the <Foundation/Foundation.h> header file. */
#undef HAVE_FOUNDATION_FOUNDATION_H

/* Define to 1 if you have the `free' function. */
#undef HAVE_FREE

/* Define to 1 if you have the `freeifaddrs' function. */
#undef HAVE_FREEIFADDRS

/* Define to 1 if you have the `getifaddrs' function. */
#undef HAVE_GETIFADDRS

/* Define to 1 if you have the `gettimeofday' function. */
#undef HAVE_GETTIMEOFDAY

/* Define to 1 if your <netinet/icmp6.h> header file defines the ICMP6_FILTER
   socket option. */
#undef HAVE_ICMP6_FILTER

/* Define to 1 if you have the <ifaddrs.h> header file. */
#undef HAVE_IFADDRS_H

/* Define to 1 if you have the <inttypes.h> header file. */
#undef HAVE_INTTYPES_H

/* Define to 1 if you have the <linux/if_tun.h> header file. */
#undef HAVE_LINUX_IF_TUN_H

/* Define to 1 if you have the <linux/ipv6_route.h> header file. */
#undef HAVE_LINUX_IPV6_ROUTE_H

/* Define to 1 if you have the <linux/route.h> header file. */
#undef HAVE_LINUX_ROUTE_H

/* Define to 1 if you have the `localtime' function. */
#undef HAVE_LOCALTIME

/* Define to 1 if you have the <lwip/debug.h> header file. */
#undef HAVE_LWIP_DEBUG_H

/* Define to 1 if you have the <lwip/dns.h> header file. */
#undef HAVE_LWIP_DNS_H

/* Define to 1 if you have the <lwip/err.h> header file. */
#undef HAVE_LWIP_ERR_H

/* Define to 1 if you have the <lwip/ethip6.h> header file. */
#undef HAVE_LWIP_ETHIP6_H

/* Define to 1 if you have the <lwip/init.h> header file. */
#undef HAVE_LWIP_INIT_H

/* Define to 1 if you have the <lwip/ip_addr.h> header file. */
#undef HAVE_LWIP_IP_ADDR_H

/* Define to 1 if you have the <lwip/ip.h> header file. */
#undef HAVE_LWIP_IP_H

/* Define to 1 if you have the <lwip/mem.h> header file. */
#undef HAVE_LWIP_MEM_H

/* Define to 1 if you have the <lwip/netif.h> header file. */
#undef HAVE_LWIP_NETIF_H

/* Define to 1 if you have the <lwip/opt.h> header file. */
#undef HAVE_LWIP_OPT_H

/* Define to 1 if you have the <lwip/pbuf.h> header file. */
#undef HAVE_LWIP_PBUF_H

/* Define to 1 if LwIP has the raw_bind_netif() interface */
#undef HAVE_LWIP_RAW_BIND_NETIF

/* Define to 1 if you have the <lwip/raw.h> header file. */
#undef HAVE_LWIP_RAW_H

/* Define to 1 if you have the <lwip/snmp.h> header file. */
#undef HAVE_LWIP_SNMP_H

/* Define to 1 if you have the <lwip/stats.h> header file. */
#undef HAVE_LWIP_STATS_H

/* Define to 1 if you have the <lwip/sys.h> header file. */
#undef HAVE_LWIP_SYS_H

/* Define to 1 if you have the <lwip/tcpip.h> header file. */
#undef HAVE_LWIP_TCPIP_H

/* Define to 1 if you have the <lwip/tcp.h> header file. */
#undef HAVE_LWIP_TCP_H

/* Define to 1 if LwIP has the udp_bind_netif() interface */
#undef HAVE_LWIP_UDP_BIND_NETIF

/* Define to 1 if you have the <lwip/udp.h> header file. */
#undef HAVE_LWIP_UDP_H

/* Define to 1 if you have the `malloc' function. */
#undef HAVE_MALLOC

/* Define to 1 if you have the `memchr' function. */
#undef HAVE_MEMCHR

/* Define to 1 if you have the `memcmp' function. */
#undef HAVE_MEMCMP

/* Define to 1 if you have the `memmove' function. */
#undef HAVE_MEMMOVE

/* Define to 1 if you have the <memory.h> header file. */
#undef HAVE_MEMORY_H

/* Define to 1 if you have the `memset' function. */
#undef HAVE_MEMSET

/* Define to 1 if you have the <netinet/icmp6.h> header file. */
#undef HAVE_NETINET_ICMP6_H

/* Define to 1 if you have the <netinet/in.h> header file. */
#undef HAVE_NETINET_IN_H

/* Define to 1 if you have the <new> header file. */
#undef HAVE_NEW

/* Define to 1 if you have the <nlassert.h> header file. */
#undef HAVE_NLASSERT_H

/* Define to 1 if you have the <nlbyteorder.h> header file. */
#undef HAVE_NLBYTEORDER_H

/* Define to 1 if you have the <nlfaultinjection.hpp> header file. */
#undef HAVE_NLFAULTINJECTION_HPP

/* Define to 1 if you have the <nlunit-test.h> header file. */
#undef HAVE_NLUNIT_TEST_H

/* Define to 1 if you have the <openssl/aes.h> header file. */
#undef HAVE_OPENSSL_AES_H

/* Define to 1 if you have the <openssl/bn.h> header file. */
#undef HAVE_OPENSSL_BN_H

/* Define to 1 if you have the <openssl/crypto.h> header file. */
#undef HAVE_OPENSSL_CRYPTO_H

/* Define to 1 if you have the <openssl/ec.h> header file. */
#undef HAVE_OPENSSL_EC_H

/* Define to 1 if you have the <openssl/err.h> header file. */
#undef HAVE_OPENSSL_ERR_H

/* Define to 1 if you have the <openssl/evp.h> header file. */
#undef HAVE_OPENSSL_EVP_H

/* Define to 1 if you have the <openssl/hmac.h> header file. */
#undef HAVE_OPENSSL_HMAC_H

/* Define to 1 if you have the <openssl/kdf.h> header file. */
#undef HAVE_OPENSSL_KDF_H

/* Define to 1 if you have the <openssl/rand.h> header file. */
#undef HAVE_OPENSSL_RAND_H

/* Define to 1 if you have the <openssl/sha.h> header file. */
#undef HAVE_OPENSSL_SHA_H

/* Define to 1 if you have the <openssl/srp.h> header file. */
#undef HAVE_OPENSSL_SRP_H

/* Define if you have POSIX threads libraries and header files. */
#undef HAVE_PTHREAD

/* Have PTHREAD_PRIO_INHERIT. */
#undef HAVE_PTHREAD_PRIO_INHERIT

/* Define to 1 if you have the `realloc' function. */
#undef HAVE_REALLOC

/* Define to 1 if you have the `snprintf' function. */
#undef HAVE_SNPRINTF

/* Define to 1 if your <sys/socket.h> header file defines the SO_BINDTODEVICE
   socket option. */
#undef HAVE_SO_BINDTODEVICE

/* Define to 1 if stdbool.h conforms to C99. */
#undef HAVE_STDBOOL_H

/* Define to 1 if you have the <stdint.h> header file. */
#undef HAVE_STDINT_H

/* Define to 1 if you have the <stdlib.h> header file. */
#undef HAVE_STDLIB_H

/* Define to 1 if you have the `strchr' function. */
#undef HAVE_STRCHR

/* Define to 1 if you have the `strcmp' function. */
#undef HAVE_STRCMP

/* Define to 1 if you have the `strdup' function. */
#undef HAVE_STRDUP

/* Define to 1 if you have the <strings.h> header file. */
#undef HAVE_STRINGS_H

/* Define to 1 if you have the <string.h> header file. */
#undef HAVE_STRING_H

/* Define to 1 if you have the `strlen' function. */
#undef HAVE_STRLEN

/* Define to 1 if the system has the type `struct in6_rtmsg'. */
#undef HAVE_STRUCT_IN6_RTMSG

/* Define to 1 if you have the <sys/socket.h> header file. */
#undef HAVE_SYS_SOCKET_H

/* Define to 1 if you have the <sys/sockio.h> header file. */
#undef HAVE_SYS_SOCKIO_H

/* Define to 1 if you have the <sys/stat.h> header file. */
#undef HAVE_SYS_STAT_H

/* Define to 1 if you have the <sys/time.h> header file. */
#undef HAVE_SYS_TIME_H

/* Define to 1 if you have the <sys/types.h> header file. */
#undef HAVE_SYS_TYPES_H

/* Define to 1 if you have the <time.h> header file. */
#undef HAVE_TIME_H

/* Define to 1 if <linux/if_tun.h> header file defines the TUNGETIFF ioctl
   command. */
#undef HAVE_TUNGETIFF

/* Define to 1 if you have the <unistd.h> header file. */
#undef HAVE_UNISTD_H

/* Define to 1 if the system has the type `_Bool'. */
#undef HAVE__BOOL

/* Define to 0 for disabling ASYNC DNS */
#undef INET_CONFIG_ENABLE_ASYNC_DNS_SOCKETS

/* Define to 1 for enabling DNS Resolver */
#undef INET_CONFIG_ENABLE_DNS_RESOLVER

/* Define to 1 for enabling IPv4 */
#undef INET_CONFIG_ENABLE_IPV4

/* Define to 1 for enabling Raw Endpoint */
#undef INET_CONFIG_ENABLE_RAW_ENDPOINT

/* Define to 1 for enabling TCP Endpoint */
#undef INET_CONFIG_ENABLE_TCP_ENDPOINT

/* Define to 1 for enabling TUN Endpoint (only for LWIP and Linux w/ support)
   */
#undef INET_CONFIG_ENABLE_TUN_ENDPOINT

/* Define to 1 for enabling UDP Endpoint */
#undef INET_CONFIG_ENABLE_UDP_ENDPOINT

/* CHIP target inet network endpoint(s) */
#undef INET_ENDPOINTS

/* Path to Inet Layer platform config header file */
#undef INET_PLATFORM_CONFIG_INCLUDE

/* Path to CHIP Inet Layer platform config header file */
#undef INET_PROJECT_CONFIG_INCLUDE

/* Define to 1 if you want to use CHIP with the inet dns resolver. */
#undef INET_WANT_ENDPOINT_DNS

/* Define to 1 if you want to use CHIP with the inet raw endpoint. */
#undef INET_WANT_ENDPOINT_RAW

/* Define to 1 if you want to use CHIP with the inet tcp endpoint. */
#undef INET_WANT_ENDPOINT_TCP

/* Define to 1 if you want to use CHIP with the inet tun endpoint. */
#undef INET_WANT_ENDPOINT_TUN

/* Define to 1 if you want to use CHIP with the inet udp endpoint. */
#undef INET_WANT_ENDPOINT_UDP

/* Define to 1 if you want to use CHIP with IPv4. */
#undef INET_WANT_IPV4

/* Define to the sub-directory where libtool stores uninstalled libraries. */
#undef LT_OBJDIR

/* Define to 1 if your C compiler doesn't accept -c and -o together. */
#undef NO_MINUS_C_MINUS_O

/* Define to the address where bug reports for this package should be sent. */
#undef PACKAGE_BUGREPORT

/* Define to the full name of this package. */
#undef PACKAGE_NAME

/* Define to the full name and version of this package. */
#undef PACKAGE_STRING

/* Define to the one symbol short name of this package. */
#undef PACKAGE_TARNAME

/* Define to the home page for this package. */
#undef PACKAGE_URL

/* Define to the version of this package. */
#undef PACKAGE_VERSION

/* Define to necessary symbol if this constant uses a non-standard name on
   your system. */
#undef PTHREAD_CREATE_JOINABLE

/* Approximation of PTHREAD_NULL since pthread.h does not define one */
#undef PTHREAD_NULL

/* Define to 1 if you have the ANSI C header files. */
#undef STDC_HEADERS

/* Path to System Layer platform config header file */
#undef SYSTEM_PLATFORM_CONFIG_INCLUDE

/* Path to CHIP System Layer platform config header file */
#undef SYSTEM_PROJECT_CONFIG_INCLUDE

/* Path to WARM platform config header file */
#undef WARM_PLATFORM_CONFIG_INCLUDE

/* Define for Solaris 2.5.1 so the uint32_t typedef from <sys/synch.h>,
   <pthread.h>, or <semaphore.h> is not used. If the typedef were allowed, the
   #define below would cause a syntax error. */
#undef _UINT32_T

/* Define for Solaris 2.5.1 so the uint64_t typedef from <sys/synch.h>,
   <pthread.h>, or <semaphore.h> is not used. If the typedef were allowed, the
   #define below would cause a syntax error. */
#undef _UINT64_T

/* Define for Solaris 2.5.1 so the uint8_t typedef from <sys/synch.h>,
   <pthread.h>, or <semaphore.h> is not used. If the typedef were allowed, the
   #define below would cause a syntax error. */
#undef _UINT8_T

/* Define to the type of a signed integer type of width exactly 16 bits if
   such a type exists and the standard includes do not define it. */
#undef int16_t

/* Define to the type of a signed integer type of width exactly 32 bits if
   such a type exists and the standard includes do not define it. */
#undef int32_t

/* Define to the type of a signed integer type of width exactly 64 bits if
   such a type exists and the standard includes do not define it. */
#undef int64_t

/* Define to the type of a signed integer type of width exactly 8 bits if such
   a type exists and the standard includes do not define it. */
#undef int8_t

/* Define to the type of an unsigned integer type of width exactly 16 bits if
   such a type exists and the standard includes do not define it. */
#undef uint16_t

/* Define to the type of an unsigned integer type of width exactly 32 bits if
   such a type exists and the standard includes do not define it. */
#undef uint32_t

/* Define to the type of an unsigned integer type of width exactly 64 bits if
   such a type exists and the standard includes do not define it. */
#undef uint64_t

/* Define to the type of an unsigned integer type of width exactly 8 bits if
   such a type exists and the standard includes do not define it. */
#undef uint8_t

#endif // CHIP_BUILD_CONFIG_H
