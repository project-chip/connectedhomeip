/**
 *
 *    Copyright (c) 2020 Silicon Labs
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */
/***************************************************************************//**
 * @file
 * @brief This file provides CLI commands to manipulate the network interface.
 *******************************************************************************
   ******************************************************************************/

#ifndef DOXYGEN_SHOULD_SKIP_THIS
#include PLATFORM_HEADER
#include "app/framework/include/af.h"
#include "rtos/rtos.h"
#include "lwip/netif.h"
#include "lwip/sockets.h"
#include "lwip/tcpip.h"
#include "lwip/err.h"
#include "lwip/def.h"
#include "lwip/sys.h"
#include "lwip/ip_addr.h"
#include "lwip/netif.h"
#include "lwip/stats.h"
#include "lwip/debug.h"
#include "app/framework/plugin/lwip/lwip-plugin.h"
#include <stdio.h>
//=============================================================================

#define MAX_STRING_LENGTH 50

#define LWIP_STATS_MACRO(X) X()
#define LWIP_STATS_MACRO_ARG1(X, Y) X(Y)
#define LWIP_OPTION_ENABLED(X) emberAfCorePrintln("LWIP Option: %p (Enabled)", X)

#define COMMAND_DISPLAY_LWIP_STATS_STUB(X) void emberAfPluginLwip##X##StatsCommand(void) { emberAfCorePrintln("Option not available."); }

#define COMMAND_DISPLAY_LWIP_STATS(X) void emberAfPluginLwip##X##StatsCommand(void) \
  {                                                                                 \
    LWIP_STATS_MACRO(X##_STATS_DISPLAY);                                            \
  }

#if defined(LWIP_STATS_DISPLAY) && defined(LWIP_STATS)
#if LINK_STATS
COMMAND_DISPLAY_LWIP_STATS(LINK)
#else
COMMAND_DISPLAY_LWIP_STATS_STUB(LINK)
#endif

#if ETHARP_STATS
COMMAND_DISPLAY_LWIP_STATS(ETHARP)
#else
COMMAND_DISPLAY_LWIP_STATS_STUB(ETHARP)
#endif

#if SYS_STATS
COMMAND_DISPLAY_LWIP_STATS(SYS)
#else
COMMAND_DISPLAY_LWIP_STATS_STUB(SYS)
#endif

#if IP_STATS
COMMAND_DISPLAY_LWIP_STATS(IP)
#else
COMMAND_DISPLAY_LWIP_STATS_STUB(IP)
#endif

#if IPFRAG_STATS
COMMAND_DISPLAY_LWIP_STATS(IPFRAG)
#else
COMMAND_DISPLAY_LWIP_STATS_STUB(IPFRAG)
#endif

#if TCP_STATS
COMMAND_DISPLAY_LWIP_STATS(TCP)
#else
COMMAND_DISPLAY_LWIP_STATS_STUB(TCP)
#endif

#if UDP_STATS
COMMAND_DISPLAY_LWIP_STATS(UDP)
#else
COMMAND_DISPLAY_LWIP_STATS_STUB(UDP)
#endif

#if ICMP_STATS
COMMAND_DISPLAY_LWIP_STATS(ICMP)
#else
COMMAND_DISPLAY_LWIP_STATS_STUB(ICMP)
#endif

#if MEM_STATS
COMMAND_DISPLAY_LWIP_STATS(MEM)
#else
COMMAND_DISPLAY_LWIP_STATS_STUB(MEM)
#endif

/* NOTE: lwip pbuf stats macros require a pcb pool name table lookup. The indexes below are used to map
 * a plugin command to a particular pbuf pool name.
 */
static void memp_display_stats(char * poolname)
{
  for (uint8_t i = 0; i < MEMP_MAX; i++) {
    if (strcmp(lwip_stats.memp[i].name, poolname) == 0) {
      LWIP_STATS_MACRO_ARG1(MEMP_STATS_DISPLAY, i);
      break;
    }
  }
}
#define COMMAND_DISPLAY_LWIP_STATS_MEMP(X, NAME) void emberAfPluginLwipMEMP##X##StatsCommand(void) \
  {                                                                                                \
    memp_display_stats(NAME);                                                                      \
  }

#if MEMP_STATS
COMMAND_DISPLAY_LWIP_STATS_MEMP(1, "ARP_QUEUE");
COMMAND_DISPLAY_LWIP_STATS_MEMP(2, "NETCONN");
COMMAND_DISPLAY_LWIP_STATS_MEMP(3, "NETBUF");
COMMAND_DISPLAY_LWIP_STATS_MEMP(4, "RAW_PCB");
COMMAND_DISPLAY_LWIP_STATS_MEMP(5, "SYS_PCB");
COMMAND_DISPLAY_LWIP_STATS_MEMP(6, "TCP_PCB");
COMMAND_DISPLAY_LWIP_STATS_MEMP(7, "TCP_PCB_LISTEN");
COMMAND_DISPLAY_LWIP_STATS_MEMP(8, "TCP_SEG");
COMMAND_DISPLAY_LWIP_STATS_MEMP(9, "UDP_PCB");
#else
COMMAND_DISPLAY_LWIP_STATS_STUB(MEMP)
#endif

#else
COMMAND_DISPLAY_LWIP_STATS_STUB(MEM)
COMMAND_DISPLAY_LWIP_STATS_STUB(MEMP)
COMMAND_DISPLAY_LWIP_STATS_STUB(ICMP)
COMMAND_DISPLAY_LWIP_STATS_STUB(UDP)
COMMAND_DISPLAY_LWIP_STATS_STUB(TCP)
COMMAND_DISPLAY_LWIP_STATS_STUB(IP)
COMMAND_DISPLAY_LWIP_STATS_STUB(IPFRAG)
COMMAND_DISPLAY_LWIP_STATS_STUB(SYS)
COMMAND_DISPLAY_LWIP_STATS_STUB(LINK)
#endif

#endif //DOXYGEN_SHOULD_SKIP_THIS
