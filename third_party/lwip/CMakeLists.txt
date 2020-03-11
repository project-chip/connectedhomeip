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
#      This file is the CMake template for the Lightweight Internet Protocol (LwIP) library.
#

add_library(lwip)

set(COMMON_INCLUDES
    ${PROJECT_SOURCE_DIR}/src
    ${PROJECT_SOURCE_DIR}/src/include
    ${PROJECT_SOURCE_DIR}/src/lib
    ${PROJECT_SOURCE_DIR}/src/system
    ${PROJECT_SOURCE_DIR}/third_party/lwip/repo/lwip/src/include
)

set(LWIP_API_SOURCES
    repo/lwip/src/api/api_lib.c
    repo/lwip/src/api/api_msg.c
    repo/lwip/src/api/err.c
    repo/lwip/src/api/netbuf.c
    repo/lwip/src/api/netdb.c
    repo/lwip/src/api/netifapi.c
    repo/lwip/src/api/sockets.c
    repo/lwip/src/api/tcpip.c
)

set(LWIP_CORE_SOURCES
    repo/lwip/src/core/def.c
    repo/lwip/src/core/dns.c
    repo/lwip/src/core/inet_chksum.c
    repo/lwip/src/core/init.c
    repo/lwip/src/core/ipv4/autoip.c
    repo/lwip/src/core/ipv4/icmp.c
    repo/lwip/src/core/ipv4/igmp.c
    repo/lwip/src/core/ipv4/ip4.c
    repo/lwip/src/core/ipv4/ip4_addr.c
    repo/lwip/src/core/ipv4/etharp.c
    repo/lwip/src/core/ipv4/ip4_frag.c
    repo/lwip/src/core/ipv4/dhcp.c
    repo/lwip/src/core/ipv6/ip6_route_table.c
    repo/lwip/src/core/ipv6/dhcp6.c
    repo/lwip/src/core/ipv6/ethip6.c
    repo/lwip/src/core/ipv6/icmp6.c
    repo/lwip/src/core/ipv6/inet6.c
    repo/lwip/src/core/ipv6/ip6.c
    repo/lwip/src/core/ipv6/ip6_addr.c
    repo/lwip/src/core/ipv6/ip6_frag.c
    repo/lwip/src/core/ipv6/mld6.c
    repo/lwip/src/core/ipv6/nd6.c
    repo/lwip/src/core/mem.c
    repo/lwip/src/core/memp.c
    repo/lwip/src/core/netif.c
    repo/lwip/src/core/pbuf.c
    repo/lwip/src/core/raw.c
    repo/lwip/src/core/stats.c
    repo/lwip/src/core/sys.c
    repo/lwip/src/core/tcp.c
    repo/lwip/src/core/tcp_in.c
    repo/lwip/src/core/tcp_out.c
    repo/lwip/src/core/udp.c
    repo/lwip/src/core/ip.c
    repo/lwip/src/core/timeouts.c
)

set(LWIP_NETIF_SOURCES
    repo/lwip/src/netif/ethernet.c
)

set(COMMON_SOURCES
    ${LWIP_API_SOURCES}
    ${LWIP_CORE_SOURCES}
    ${LWIP_NETIF_SOURCES}
)


target_compile_definitions(lwip PRIVATE)
target_compile_options(lwip PRIVATE)
target_include_directories(lwip PRIVATE ${COMMON_INCLUDES})
target_sources(lwip PRIVATE ${COMMON_SOURCES})

target_link_libraries(lwip PRIVATE
    lwipTarget
    chip-config
)
