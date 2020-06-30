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
#
#    Description:
#      Configure toolchain for standalone (host) platforms.
#

#
# Configure defines and include paths for platform
#

add_library(chip-config INTERFACE)

# Use pkgconfig for openssl
find_package(PkgConfig REQUIRED)
pkg_check_modules(OPENSSL REQUIRED openssl)

#target_compile_options(chip-config PUBLIC ${OPENSSL_CFLAGS_OTHER})

target_compile_definitions(chip-config INTERFACE
    HAVE_MALLOC=1
    HAVE_FREE=1
    HAVE_CLOCK_GETTIME=1
    HAVE_NETINET_ICMP6_H=1
    HAVE_ICMP6_FILTER=1
    CHIP_SYSTEM_CONFIG_USE_SOCKETS=1
    CHIP_SYSTEM_CONFIG_USE_LWIP=0
    CHIP_LOGGING_STYLE_STDIO=1
    INET_CONFIG_ENABLE_UDP_ENDPOINT=1
    INET_CONFIG_ENABLE_TCP_ENDPOINT=1
    INET_CONFIG_ENABLE_RAW_ENDPOINT=1
    INET_CONFIG_ENABLE_DNS_RESOLVER=1
    INET_CONFIG_ENABLE_ASYNC_DNS_SOCKETS=1
    INET_CONFIG_ENABLE_IPV4=1
    INET_WANT_ENDPOINT_DNS=1
    INET_CONFIG_TEST=1
    CHIP_CONFIG_TEST=1
    CHIP_WITH_NLFAULTINJECTION=1
    CHIP_SYSTEM_CONFIG_TEST=1
    CHIP_SYSTEM_CONFIG_PROVIDE_STATISTICS=1   # TestSystemObject expects this
    CHIP_CONFIG_ENABLE_ARG_PARSER=1           # TestInet expects this
    CHIP_CRYPTO_OPENSSL=1
    CONFIG_NETWORK_LAYER_BLE=1
)

target_include_directories(chip-config INTERFACE
    ${PROJECT_SOURCE_DIR}/config/standalone
    ${PROJECT_SOURCE_DIR}/third_party/nlassert/repo/include
    ${PROJECT_SOURCE_DIR}/third_party/nlio/repo/include
    ${PROJECT_SOURCE_DIR}/src/lwip/standalone
    ${OPENSSL_INCLUDE_DIRS}
)

set(CHIP_TARGET_STYLE_UNIX 1)

set(CHIP_CRYPTO_MBEDTLS 0)
set(CHIP_CRYPTO_OPENSSL 1)

set(CONFIG_DEVICE_LAYER 0)
set(CHIP_WITH_LWIP_INTERNAL 0)
set(CHIP_LWIP_TARGET_STANDALONE 1)

set(CHIP_PLATFORM_LIBS
    ${OPENSSL_LDFLAGS}
    pthread
    dl
    chip-config
)

# Empty BSP add_library
add_library(PlatformBsp INTERFACE)
add_library(PlatformOs INTERFACE)
add_library(lwipTarget INTERFACE)
