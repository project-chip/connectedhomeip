#
#    Copyright (c) 2020 Project CHIP Authors
#    Copyright (c) 2014-2017 Nest Labs, Inc.
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
#      This file is the GNU automake template for the CHIP in-package,
#      LwIP library.
#
#      LwIP lacks any build system of its own, so this actually builds
#      the library.
#

include $(abs_top_nlbuild_autotools_dir)/automake/pre.am

SUBDIRS                                                                        = tests

if CHIP_WITH_LWIP_INTERNAL

lib_LIBRARIES                                                                  = liblwip.a

liblwip_API_SOURCES                                                            = \
    @top_builddir@/third_party/lwip/repo/lwip/src/api/api_lib.c                  \
    @top_builddir@/third_party/lwip/repo/lwip/src/api/api_msg.c                  \
    @top_builddir@/third_party/lwip/repo/lwip/src/api/err.c                      \
    @top_builddir@/third_party/lwip/repo/lwip/src/api/netbuf.c                   \
    @top_builddir@/third_party/lwip/repo/lwip/src/api/netdb.c                    \
    @top_builddir@/third_party/lwip/repo/lwip/src/api/netifapi.c                 \
    @top_builddir@/third_party/lwip/repo/lwip/src/api/sockets.c                  \
    @top_builddir@/third_party/lwip/repo/lwip/src/api/tcpip.c                    \
    $(NULL)

liblwip_CORE_SOURCES                                                           = \
    @top_builddir@/third_party/lwip/repo/lwip/src/core/def.c                     \
    @top_builddir@/third_party/lwip/repo/lwip/src/core/dns.c                     \
    @top_builddir@/third_party/lwip/repo/lwip/src/core/inet_chksum.c             \
    @top_builddir@/third_party/lwip/repo/lwip/src/core/init.c                    \
    @top_builddir@/third_party/lwip/repo/lwip/src/core/ipv4/autoip.c             \
    @top_builddir@/third_party/lwip/repo/lwip/src/core/ipv4/icmp.c               \
    @top_builddir@/third_party/lwip/repo/lwip/src/core/ipv4/igmp.c               \
    @top_builddir@/third_party/lwip/repo/lwip/src/core/ipv4/ip4.c                \
    @top_builddir@/third_party/lwip/repo/lwip/src/core/ipv4/ip4_addr.c           \
    @top_builddir@/third_party/lwip/repo/lwip/src/core/ipv4/etharp.c             \
    @top_builddir@/third_party/lwip/repo/lwip/src/core/ipv4/ip4_frag.c           \
    @top_builddir@/third_party/lwip/repo/lwip/src/core/ipv4/dhcp.c               \
    @top_builddir@/third_party/lwip/repo/lwip/src/core/ipv6/ip6_route_table.c    \
    @top_builddir@/third_party/lwip/repo/lwip/src/core/ipv6/dhcp6.c              \
    @top_builddir@/third_party/lwip/repo/lwip/src/core/ipv6/ethip6.c             \
    @top_builddir@/third_party/lwip/repo/lwip/src/core/ipv6/icmp6.c              \
    @top_builddir@/third_party/lwip/repo/lwip/src/core/ipv6/inet6.c              \
    @top_builddir@/third_party/lwip/repo/lwip/src/core/ipv6/ip6.c                \
    @top_builddir@/third_party/lwip/repo/lwip/src/core/ipv6/ip6_addr.c           \
    @top_builddir@/third_party/lwip/repo/lwip/src/core/ipv6/ip6_frag.c           \
    @top_builddir@/third_party/lwip/repo/lwip/src/core/ipv6/mld6.c               \
    @top_builddir@/third_party/lwip/repo/lwip/src/core/ipv6/nd6.c                \
    @top_builddir@/third_party/lwip/repo/lwip/src/core/mem.c                     \
    @top_builddir@/third_party/lwip/repo/lwip/src/core/memp.c                    \
    @top_builddir@/third_party/lwip/repo/lwip/src/core/netif.c                   \
    @top_builddir@/third_party/lwip/repo/lwip/src/core/pbuf.c                    \
    @top_builddir@/third_party/lwip/repo/lwip/src/core/raw.c                     \
    @top_builddir@/third_party/lwip/repo/lwip/src/core/stats.c                   \
    @top_builddir@/third_party/lwip/repo/lwip/src/core/sys.c                     \
    @top_builddir@/third_party/lwip/repo/lwip/src/core/tcp.c                     \
    @top_builddir@/third_party/lwip/repo/lwip/src/core/tcp_in.c                  \
    @top_builddir@/third_party/lwip/repo/lwip/src/core/tcp_out.c                 \
    @top_builddir@/third_party/lwip/repo/lwip/src/core/udp.c                     \
    @top_builddir@/third_party/lwip/repo/lwip/src/core/ip.c                      \
    @top_builddir@/third_party/lwip/repo/lwip/src/core/timeouts.c                \
    $(NULL)

liblwip_NETIF_SOURCES                                                          = \
    @top_builddir@/third_party/lwip/repo/lwip/src/netif/ethernet.c               \
    $(NULL)

if CHIP_LWIP_TARGET_STANDALONE

liblwip_TARGET_SOURCES                                                         = \
    standalone/sys_arch.c                                                        \
    standalone/TapInterface.c                                                    \
    $(NULL)

EXTRA_DIST                                                                     = \
    standalone/arch                                                              \
    standalone/lwipopts.h                                                        \
    standalone/TapInterface.h                                                    \
    $(NULL)

endif # CHIP_LWIP_TARGET_STANDALONE

if CHIP_LWIP_TARGET_NRF5

liblwip_TARGET_SOURCES                                                         = \
    freertos/sys_arch.c                                                          \
    $(NULL)

EXTRA_DIST                                                                     = \
    freertos/arch                                                                \
    nrf5/arch                                                                    \
    nrf5/lwipopts.h                                                              \
    nrf5/lwippools.h                                                             \
    $(NULL)

endif # CHIP_LWIP_TARGET_NRF5

if CHIP_LWIP_TARGET_EFR32

liblwip_TARGET_SOURCES                                                         = \
    freertos/sys_arch.c                                                          \
    $(NULL)

EXTRA_DIST                                                                     = \
    freertos/arch                                                                \
    efr32/arch                                                                    \
    efr32/lwipopts.h                                                              \
    efr32/lwippools.h                                                             \
    $(NULL)

endif # CHIP_LWIP_TARGET_EFR32

if CHIP_LWIP_TARGET_K32W

liblwip_TARGET_SOURCES                                                         = \
    freertos/sys_arch.c                                                          \
    $(NULL)

EXTRA_DIST                                                                     = \
    freertos/arch                                                                \
    k32w/arch                                                                    \
    k32w/lwipopts.h                                                              \
    k32w/lwippools.h                                                             \
    $(NULL)

endif # CHIP_LWIP_TARGET_K32W

liblwip_a_SOURCES                                                              = \
    $(liblwip_API_SOURCES)                                                       \
    $(liblwip_CORE_SOURCES)                                                      \
    $(liblwip_NETIF_SOURCES)                                                     \
    $(liblwip_TARGET_SOURCES)                                                    \
    $(NULL)

liblwip_a_CPPFLAGS                                                             = \
    $(LWIP_CPPFLAGS)                                                             \
    $(NULL)

endif # CHIP_WITH_LWIP_INTERNAL

PRETTY_FILES := $(NULL)

include $(abs_top_nlbuild_autotools_dir)/automake/post.am
