/* src/include/BuildConfig.h.  Generated from BuildConfig.h.in by configure.  */
/* src/include/BuildConfig.h.in.  Generated from configure.ac by autoheader.  */

#ifndef CHIP_BUILD_CONFIG_H
#define CHIP_BUILD_CONFIG_H

/* Path to BLE platform config header file */
/* #undef BLE_PLATFORM_CONFIG_INCLUDE */

/* Define to 1 if you want to enable CHIPoBle over bluez. */
#define CONFIG_BLE_PLATFORM_BLUEZ 0

/* Define to 1 if you want to use the CHIP Device Layer. */
#define CONFIG_DEVICE_LAYER 0

/* Define to 1 if you want to use CHIP with a system that supports
   callback-based vcbprintf */
#define CONFIG_HAVE_VCBPRINTF 0

/* Define to 1 if you want to use CHIP with a system that supports
   callback-based vcbprintf */
#define CONFIG_HAVE_VSNPRINTF_EX 0

/* Define to 1 if you want to use CHIP with the Bluetooth Low Energy network
   stack. */
#define CONFIG_NETWORK_LAYER_BLE 1

/* Define to 1 if you want to use CHIP with the Internet Protocol network
   stack. */
#define CONFIG_NETWORK_LAYER_INET 1

/* CHIP target network stack(s) */
#define CONFIG_TARGET_NETWORKS "sockets"

/* Define to 1 if your C++ compiler doesn't accept -c and -o together. */
/* #undef CXX_NO_MINUS_C_MINUS_O */

/* Define to 1 if the system has the type `clockid_t'. */
#define HAVE_CLOCKID_T 1

/* Define to 1 if you have the `clock_gettime' function. */
/* #undef HAVE_CLOCK_GETTIME */

/* Define to 1 if you have the `clock_settime' function. */
#define HAVE_CLOCK_SETTIME 1

/* Define to 1 if you have the <ctype.h> header file. */
#define HAVE_CTYPE_H 1

/* Define to 1 if you have the <curl/curl.h> header file. */
/* #undef HAVE_CURL_CURL_H */

/* Define to 1 if you have the <curl/easy.h> header file. */
/* #undef HAVE_CURL_EASY_H */

/* define if the compiler supports basic C++11 syntax */
#define HAVE_CXX11 1

/* Define to 1 if you have the declaration of `CLOCK_BOOTTIME', and to 0 if
   you don't. */
#define HAVE_DECL_CLOCK_BOOTTIME 0

/* Define to 1 if you have the declaration of `CLOCK_MONOTONIC', and to 0 if
   you don't. */
#define HAVE_DECL_CLOCK_MONOTONIC 1

/* Define to 1 if you have the <dlfcn.h> header file. */
#define HAVE_DLFCN_H 1

/* Define to 1 if you have the <Foundation/Foundation.h> header file. */
/* #undef HAVE_FOUNDATION_FOUNDATION_H */

/* Define to 1 if you have the `free' function. */
#define HAVE_FREE 1

/* Define to 1 if you have the `freeifaddrs' function. */
#define HAVE_FREEIFADDRS 1

/* Define to 1 if you have the `getifaddrs' function. */
#define HAVE_GETIFADDRS 1

/* Define to 1 if you have the `gettimeofday' function. */
#define HAVE_GETTIMEOFDAY 1

/* Define to 1 if your <netinet/icmp6.h> header file defines the ICMP6_FILTER
   socket option. */
#define HAVE_ICMP6_FILTER 1

/* Define to 1 if you have the <ifaddrs.h> header file. */
#define HAVE_IFADDRS_H 1

/* Define to 1 if you have the <inttypes.h> header file. */
#define HAVE_INTTYPES_H 1

/* Define to 1 if you have the <linux/if_tun.h> header file. */
/* #undef HAVE_LINUX_IF_TUN_H */

/* Define to 1 if you have the <linux/ipv6_route.h> header file. */
/* #undef HAVE_LINUX_IPV6_ROUTE_H */

/* Define to 1 if you have the <linux/route.h> header file. */
/* #undef HAVE_LINUX_ROUTE_H */

/* Define to 1 if you have the `localtime' function. */
#define HAVE_LOCALTIME 1

/* Define to 1 if you have the <lwip/debug.h> header file. */
/* #undef HAVE_LWIP_DEBUG_H */

/* Define to 1 if you have the <lwip/dns.h> header file. */
/* #undef HAVE_LWIP_DNS_H */

/* Define to 1 if you have the <lwip/err.h> header file. */
/* #undef HAVE_LWIP_ERR_H */

/* Define to 1 if you have the <lwip/ethip6.h> header file. */
/* #undef HAVE_LWIP_ETHIP6_H */

/* Define to 1 if you have the <lwip/init.h> header file. */
/* #undef HAVE_LWIP_INIT_H */

/* Define to 1 if you have the <lwip/ip_addr.h> header file. */
/* #undef HAVE_LWIP_IP_ADDR_H */

/* Define to 1 if you have the <lwip/ip.h> header file. */
/* #undef HAVE_LWIP_IP_H */

/* Define to 1 if you have the <lwip/mem.h> header file. */
/* #undef HAVE_LWIP_MEM_H */

/* Define to 1 if you have the <lwip/netif.h> header file. */
/* #undef HAVE_LWIP_NETIF_H */

/* Define to 1 if you have the <lwip/opt.h> header file. */
/* #undef HAVE_LWIP_OPT_H */

/* Define to 1 if you have the <lwip/pbuf.h> header file. */
/* #undef HAVE_LWIP_PBUF_H */

/* Define to 1 if LwIP has the raw_bind_netif() interface */
/* #undef HAVE_LWIP_RAW_BIND_NETIF */

/* Define to 1 if you have the <lwip/raw.h> header file. */
/* #undef HAVE_LWIP_RAW_H */

/* Define to 1 if you have the <lwip/snmp.h> header file. */
/* #undef HAVE_LWIP_SNMP_H */

/* Define to 1 if you have the <lwip/stats.h> header file. */
/* #undef HAVE_LWIP_STATS_H */

/* Define to 1 if you have the <lwip/sys.h> header file. */
/* #undef HAVE_LWIP_SYS_H */

/* Define to 1 if you have the <lwip/tcpip.h> header file. */
/* #undef HAVE_LWIP_TCPIP_H */

/* Define to 1 if you have the <lwip/tcp.h> header file. */
/* #undef HAVE_LWIP_TCP_H */

/* Define to 1 if LwIP has the udp_bind_netif() interface */
/* #undef HAVE_LWIP_UDP_BIND_NETIF */

/* Define to 1 if you have the <lwip/udp.h> header file. */
/* #undef HAVE_LWIP_UDP_H */

/* Define to 1 if you have the `malloc' function. */
#define HAVE_MALLOC 1

/* Define to 1 if you have the `memchr' function. */
#define HAVE_MEMCHR 1

/* Define to 1 if you have the `memcmp' function. */
#define HAVE_MEMCMP 1

/* Define to 1 if you have the `memmove' function. */
#define HAVE_MEMMOVE 1

/* Define to 1 if you have the <memory.h> header file. */
#define HAVE_MEMORY_H 1

/* Define to 1 if you have the `memset' function. */
#define HAVE_MEMSET 1

/* Define to 1 if you have the <netinet/icmp6.h> header file. */
#define HAVE_NETINET_ICMP6_H 1

/* Define to 1 if you have the <netinet/in.h> header file. */
#define HAVE_NETINET_IN_H 1

/* Define to 1 if you have the <new> header file. */
#define HAVE_NEW 1

/* Define to 1 if you have the <nlassert.h> header file. */
/* #undef HAVE_NLASSERT_H */

/* Define to 1 if you have the <nlbyteorder.h> header file. */
/* #undef HAVE_NLBYTEORDER_H */

/* Define to 1 if you have the <nlfaultinjection.hpp> header file. */
/* #undef HAVE_NLFAULTINJECTION_HPP */

/* Define to 1 if you have the <nlunit-test.h> header file. */
/* #undef HAVE_NLUNIT_TEST_H */

/* Define to 1 if you have the <openssl/aes.h> header file. */
/* #undef HAVE_OPENSSL_AES_H */

/* Define to 1 if you have the <openssl/bn.h> header file. */
/* #undef HAVE_OPENSSL_BN_H */

/* Define to 1 if you have the <openssl/ec.h> header file. */
/* #undef HAVE_OPENSSL_EC_H */

/* Define to 1 if you have the <openssl/err.h> header file. */
/* #undef HAVE_OPENSSL_ERR_H */

/* Define to 1 if you have the <openssl/evp.h> header file. */
/* #undef HAVE_OPENSSL_EVP_H */

/* Define to 1 if you have the <openssl/sha.h> header file. */
/* #undef HAVE_OPENSSL_SHA_H */

/* Define if you have POSIX threads libraries and header files. */
#define HAVE_PTHREAD 1

/* Have PTHREAD_PRIO_INHERIT. */
#define HAVE_PTHREAD_PRIO_INHERIT 1

/* Define to 1 if you have the `realloc' function. */
#define HAVE_REALLOC 1

/* Define to 1 if you have the `snprintf' function. */
#define HAVE_SNPRINTF 1

/* Define to 1 if your <sys/socket.h> header file defines the SO_BINDTODEVICE
   socket option. */
/* #undef HAVE_SO_BINDTODEVICE */

/* Define to 1 if stdbool.h conforms to C99. */
#define HAVE_STDBOOL_H 1

/* Define to 1 if you have the <stdint.h> header file. */
#define HAVE_STDINT_H 1

/* Define to 1 if you have the <stdio.h> header file. */
#define HAVE_STDIO_H 1

/* Define to 1 if you have the <stdlib.h> header file. */
#define HAVE_STDLIB_H 1

/* Define to 1 if you have the `strchr' function. */
#define HAVE_STRCHR 1

/* Define to 1 if you have the `strcmp' function. */
#define HAVE_STRCMP 1

/* Define to 1 if you have the `strdup' function. */
#define HAVE_STRDUP 1

/* Define to 1 if you have the <strings.h> header file. */
#define HAVE_STRINGS_H 1

/* Define to 1 if you have the <string.h> header file. */
#define HAVE_STRING_H 1

/* Define to 1 if you have the `strlen' function. */
#define HAVE_STRLEN 1

/* Define to 1 if the system has the type `struct in6_rtmsg'. */
/* #undef HAVE_STRUCT_IN6_RTMSG */

/* Define to 1 if you have the <sys/socket.h> header file. */
#define HAVE_SYS_SOCKET_H 1

/* Define to 1 if you have the <sys/stat.h> header file. */
#define HAVE_SYS_STAT_H 1

/* Define to 1 if you have the <sys/time.h> header file. */
#define HAVE_SYS_TIME_H 1

/* Define to 1 if you have the <sys/types.h> header file. */
#define HAVE_SYS_TYPES_H 1

/* Define to 1 if you have the <time.h> header file. */
#define HAVE_TIME_H 1

/* Define to 1 if <linux/if_tun.h> header file defines the TUNGETIFF ioctl
   command. */
/* #undef HAVE_TUNGETIFF */

/* Define to 1 if you have the <unistd.h> header file. */
#define HAVE_UNISTD_H 1

/* Define to 1 if your libc exports vcbprintf. */
#define HAVE_VCBPRINTF 0

/* Define to 1 if your libc exports vnsprintf_ex. */
#define HAVE_VSNPRINTF_EX 0

/* Define to 1 if the system has the type `_Bool'. */
#define HAVE__BOOL 1

/* Define to 0 for disabling ASYNC DNS */
#define INET_CONFIG_ENABLE_ASYNC_DNS_SOCKETS 1

/* Define to 1 for enabling DNS Resolver */
#define INET_CONFIG_ENABLE_DNS_RESOLVER 1

/* Define to 1 for enabling IPv4 */
#define INET_CONFIG_ENABLE_IPV4 1

/* Define to 1 for enabling Raw Endpoint */
#define INET_CONFIG_ENABLE_RAW_ENDPOINT 1

/* Define to 1 for enabling TCP Endpoint */
#define INET_CONFIG_ENABLE_TCP_ENDPOINT 1

/* Define to 1 for enabling TUN Endpoint (only for LWIP and Linux w/ support)
   */
/* #undef INET_CONFIG_ENABLE_TUN_ENDPOINT */

/* Define to 1 for enabling UDP Endpoint */
#define INET_CONFIG_ENABLE_UDP_ENDPOINT 1

/* CHIP target inet network endpoint(s) */
#define INET_ENDPOINTS "all"

/* Path to Inet Layer platform config header file */
/* #undef INET_PLATFORM_CONFIG_INCLUDE */

/* Path to CHIP Inet Layer platform config header file */
/* #undef INET_PROJECT_CONFIG_INCLUDE */

/* Define to 1 if you want to use CHIP with the inet dns resolver. */
#define INET_WANT_ENDPOINT_DNS 1

/* Define to 1 if you want to use CHIP with the inet raw endpoint. */
#define INET_WANT_ENDPOINT_RAW 1

/* Define to 1 if you want to use CHIP with the inet tcp endpoint. */
#define INET_WANT_ENDPOINT_TCP 1

/* Define to 1 if you want to use CHIP with the inet tun endpoint. */
#define INET_WANT_ENDPOINT_TUN 0

/* Define to 1 if you want to use CHIP with the inet udp endpoint. */
#define INET_WANT_ENDPOINT_UDP 1

/* Define to 1 if you want to use CHIP with IPv4. */
#define INET_WANT_IPV4 1

/* Define to the sub-directory where libtool stores uninstalled libraries. */
#define LT_OBJDIR ".libs/"

/* Define to 1 if your C compiler doesn't accept -c and -o together. */
/* #undef NO_MINUS_C_MINUS_O */

/* Define to the address where bug reports for this package should be sent. */
#define PACKAGE_BUGREPORT "support@chipsupport.tbd"

/* Define to the full name of this package. */
#define PACKAGE_NAME "CHIP"

/* Define to the full name and version of this package. */
#define PACKAGE_STRING "CHIP gea435d2c"

/* Define to the one symbol short name of this package. */
#define PACKAGE_TARNAME "chip"

/* Define to the home page for this package. */
#define PACKAGE_URL "https://tbd.developer.chip.com/"

/* Define to the version of this package. */
#define PACKAGE_VERSION "gea435d2c"

/* Define to necessary symbol if this constant uses a non-standard name on
   your system. */
/* #undef PTHREAD_CREATE_JOINABLE */

/* Approximation of PTHREAD_NULL since pthread.h does not define one */
#define PTHREAD_NULL 0

/* Define to 1 if you have the ANSI C header files. */
#define STDC_HEADERS 1

/* Path to System Layer platform config header file */
/* #undef SYSTEM_PLATFORM_CONFIG_INCLUDE */

/* Path to CHIP System Layer platform config header file */
#define SYSTEM_PROJECT_CONFIG_INCLUDE <SystemProjectConfig.h>

/* Path to WARM platform config header file */
/* #undef WARM_PLATFORM_CONFIG_INCLUDE */

/* Path to WARM platform config header file */
/* #undef WARM_PROJECT_CONFIG_INCLUDE */

/* Define custom schema directory */
/* #undef CHIP_CONFIG_CUSTOM_BUILTIN_SCHEMA_INCLUDE */

/* Define to 1 to use the legacy WDM profile. */
#define CHIP_CONFIG_LEGACY_WDM 1

/* Define to 1 for enabling CHIP Tunnel failover */
/* #undef CHIP_CONFIG_TUNNEL_FAILOVER_SUPPORTED */

/* Target platform name for CHIP Device Layer. */
#define CHIP_DEVICE_LAYER_TARGET NONE

/* Define to 1 if you want to build the CHIP Device Layer for Silicon
   Labs EFR32 platforms. */
#define CHIP_DEVICE_LAYER_TARGET_EFR32 0

/* Define to 1 if you want to build the CHIP Device Layer for the
   Espressif ESP32. */
#define CHIP_DEVICE_LAYER_TARGET_ESP32 0

/* Define to 1 if you want to build the CHIP Device Layer for Nordic
   nRF5* platforms. */
#define CHIP_DEVICE_LAYER_TARGET_NRF5 0

/* Path to CHIP Device Layer platform config header file */
/* #undef CHIP_DEVICE_PLATFORM_CONFIG_INCLUDE */

/* Path to CHIP Device Layer platform config header file */
/* #undef CHIP_DEVICE_PROJECT_CONFIG_INCLUDE */

/* Define to 1 if you want to enable CHIPoBle Control Path and Throughput Test.
   */
#define CHIP_ENABLE_CHIPOBLE_TEST 0

/* Define to 1 if support for fuzzing enabled */
/* #undef CHIP_FUZZING_ENABLED */

/* CHIP logging style */
#define CHIP_LOGGING_STYLE stdio

/* Define to 1 if you want to use CHIP with the Android logging system */
#define CHIP_LOGGING_STYLE_ANDROID 0

/* Define to 1 if you want to use CHIP with an external (i.e. platform- and
   integrator-defined) logging system */
#define CHIP_LOGGING_STYLE_EXTERNAL 0

/* Define to 1 if you want to use CHIP with a C Standard I/O Library-based
   logging system */
#define CHIP_LOGGING_STYLE_STDIO 1

/* Define to 1 if you want to use CHIP with a C Standard I/O Library-based
   logging system,but override, at link-time an external (i.e. platform- and
   integrator-defined) logging system */
#define CHIP_LOGGING_STYLE_STDIO_WEAK 0

/* Define to 1 if you want to use CHIP with a C Standard I/O Library-based
   logging system with timestamped log entries */
#define CHIP_LOGGING_STYLE_STDIO_WITH_TIMESTAMPS 0

/* Path to CHIP platform config header file */
/* #undef CHIP_PLATFORM_CONFIG_INCLUDE */

/* Path to CHIP Core platform config header file */
#define CHIP_PROJECT_CONFIG_INCLUDE <CHIPProjectConfig.h>

/* Define to 1 if you want to use LwIP with CHIP System Layer. */
#define CHIP_SYSTEM_CONFIG_USE_LWIP 0

/* Define to 1 if you want to use BSD sockets with CHIP System Layer. */
#define CHIP_SYSTEM_CONFIG_USE_SOCKETS 1

/* CHIP target style */
#define CHIP_TARGET_STYLE unix

/* Define to 1 if you want to use CHIP with an embedded (i.e. non-Unix-based)
   system */
#define CHIP_TARGET_STYLE_EMBEDDED 0

/* Define to 1 if you want to use CHIP with a Unix-based system */
#define CHIP_TARGET_STYLE_UNIX 1

/* Define to 1 to build CHIP with nlFaultInjection features */
#define CHIP_WITH_NLFAULTINJECTION 1

/* Define to 1 to build CHIP with OpenSSL features */
#define CHIP_WITH_OPENSSL 1

/* Define for Solaris 2.5.1 so the uint32_t typedef from <sys/synch.h>,
   <pthread.h>, or <semaphore.h> is not used. If the typedef were allowed, the
   #define below would cause a syntax error. */
/* #undef _UINT32_T */

/* Define for Solaris 2.5.1 so the uint64_t typedef from <sys/synch.h>,
   <pthread.h>, or <semaphore.h> is not used. If the typedef were allowed, the
   #define below would cause a syntax error. */
/* #undef _UINT64_T */

/* Define for Solaris 2.5.1 so the uint8_t typedef from <sys/synch.h>,
   <pthread.h>, or <semaphore.h> is not used. If the typedef were allowed, the
   #define below would cause a syntax error. */
/* #undef _UINT8_T */

/* Define to the type of a signed integer type of width exactly 16 bits if
   such a type exists and the standard includes do not define it. */
/* #undef int16_t */

/* Define to the type of a signed integer type of width exactly 32 bits if
   such a type exists and the standard includes do not define it. */
/* #undef int32_t */

/* Define to the type of a signed integer type of width exactly 64 bits if
   such a type exists and the standard includes do not define it. */
/* #undef int64_t */

/* Define to the type of a signed integer type of width exactly 8 bits if such
   a type exists and the standard includes do not define it. */
/* #undef int8_t */

/* Define to the type of an unsigned integer type of width exactly 16 bits if
   such a type exists and the standard includes do not define it. */
/* #undef uint16_t */

/* Define to the type of an unsigned integer type of width exactly 32 bits if
   such a type exists and the standard includes do not define it. */
/* #undef uint32_t */

/* Define to the type of an unsigned integer type of width exactly 64 bits if
   such a type exists and the standard includes do not define it. */
/* #undef uint64_t */

/* Define to the type of an unsigned integer type of width exactly 8 bits if
   such a type exists and the standard includes do not define it. */
/* #undef uint8_t */

#endif // CHIP_BUILD_CONFIG_H
