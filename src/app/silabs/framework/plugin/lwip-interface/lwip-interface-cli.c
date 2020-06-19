/**
 *
 *    Copyright (c) 2020 Project CHIP Authors
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
#include "app/framework/plugin/lwip/micrel/micrel-ethernet.h"
#include "hal/micro/cortexm3/token-manufacturing.h"
#include "app/framework/plugin/lwip/lwip-main.h"
#include "app/framework/plugin/lwip/lwip-plugin.h"

//=============================================================================
// Globals

extern uint8_t emberStringToIpv4Address(uint8_t argNum, uint8_t *ipAddress);

//=============================================================================

#define MAX_STRING_LENGTH 50

#define LWIP_OPTION_ENABLED(X) emberAfCorePrintln("%p (Enabled)", X)

static void printLwipEthernetAddress(const uint8_t* lwipHardwareAddress)
{
  EmberEthernetAddress emberEthernetAddress;
  MEMMOVE(emberEthernetAddress.contents, lwipHardwareAddress, EMBER_ETHERNET_ADDRESS_SIZE);
  emberAfPluginLwipPrintBigEndianEthernetAddress(&emberEthernetAddress);
}

void emberAfPluginLwipStatusCommand(void)
{
  EmberAfPluginLwipIpv4TokenStruct ipv4Token;
  EmberEthernetAddress ethernetAddress;

  halCommonGetToken(&ipv4Token, TOKEN_AFV2_PLUGIN_LWIP_IPV4);
  emberAfCorePrintln("Token Data");
  emberAfPluginLwipPrintIpv4Token(&ipv4Token);
  halCommonGetMfgToken(ethernetAddress.contents, TOKEN_MFG_ETHERNET_ADDRESS);
  emberAfCorePrint("Token Ethernet Address: ");
  emberAfPluginLwipPrintBigEndianEthernetAddress(&ethernetAddress);
  emberAfCorePrintln("");

  emberAfCorePrintln("LWIP NETIF Values");
  struct netif* interface = emberAfPluginLwipGetNetIf();
  emberAfCorePrintln("Name: %c%c", interface->name[0], interface->name[1]);
  emberAfPluginLwipPrintIpv4Parameters(ntohl(interface->ip_addr.addr),
                                       ntohl(interface->netmask.addr),
                                       ntohl(interface->gw.addr));
  emberAfCorePrintln("MTU: %d", interface->mtu);

  emberAfCorePrint("LWIP Hardware Address: ");
  printLwipEthernetAddress(interface->hwaddr);
  emberAfCorePrintln("\r\n");

  #if (LWIP_STATS == 1)
  LWIP_OPTION_ENABLED("LWIP_STATS");
  #endif

  #if (LWIP_STATS_DISPLAY == 1)
  LWIP_OPTION_ENABLED("LWIP_STATS_DISPLAY");
  #endif

  #if (LWIP_DEBUG == 1)
  LWIP_OPTION_ENABLED("LWIP_DEBUG");
  #endif

  #if  (LWIP_DHCP == 1)
  LWIP_OPTION_ENABLED("LWIP_DHCP");
  #endif

  #if (LWIP_DNS == 1)
  LWIP_OPTION_ENABLED("LWIP_DNS");
  #endif

  #if (LWIP_ICMP == 1)
  LWIP_OPTION_ENABLED("LWIP_ICMP");
  #endif

  #if (LWIP_UDP == 1)
  LWIP_OPTION_ENABLED("LWIP_UDP");
  #endif

  #if (LWIP_TCP == 1)
  LWIP_OPTION_ENABLED("LWIP_TCP");
  #endif

  #if (LWIP_ARP == 1)
  LWIP_OPTION_ENABLED("LWIP_ARP");
  #endif

  #if (LWIP_NETIF_STATUS_CALLBACK == 1)
  LWIP_OPTION_ENABLED("LWIP_NETIF_STATUS_CALLBACK");
  #endif

  #if (LWIP_NETIF_LINK_CALLBACK == 1)
  LWIP_OPTION_ENABLED("LWIP_NETIF_LINK_CALLBACK");
  #endif

  #if (LWIP_USE_BUILTIN_CHECKSUM == 1)
  LWIP_OPTION_ENABLED("LWIP_CHECKSUM_GEN_X and LWIP_CHECKSUM_CHECK_X");
  #endif
}

//=============================================================================

void emberAfPluginLwipSetStaticIpv4Address(uint32_t hostOrderIpv4Address,
                                           uint32_t hostOrderNetmask,
                                           uint32_t hostOrderGateway)
{
  EmberAfPluginLwipIpv4TokenStruct ipv4TokenStruct;
  ipv4TokenStruct.hostOrderIpv4Address = hostOrderIpv4Address;
  ipv4TokenStruct.hostOrderIpv4Gateway = hostOrderGateway;
  ipv4TokenStruct.hostOrderIpv4Netmask = hostOrderNetmask;

  halCommonSetToken(TOKEN_AFV2_PLUGIN_LWIP_IPV4, &ipv4TokenStruct);

  emberAfPluginLwipSetIpv4AddressFromToken();
}

static bool validateIpv4HostAddress(const uint8_t* name, uint32_t address)
{
  if ((address >> 24) == 0) {
    emberAfCorePrintln("Error: %p, 1st quad of IP address cannot be 0.", name);
    return false;
  }
  if ((address & 0xFF) == 0) {
    emberAfCorePrintln("Error: %p, last quad of IP address cannot be 0.");
    return false;
  }
  return true;
}

/*
 * Set the IP address for the Ember ZigbeeIP interface. This will allow
 * lwIP to properly route packets and must be called by someone when the
 * stack changes its IP address.
 * @param address The new IPv6 address for this device as a 16 byte array.
 */
void emberAfPluginLwipSetIpv4AddressCommand(void)
{
  uint32_t hostOrderIpAddress;
  uint32_t hostOrderNetmask;
  uint32_t hostOrderGateway;
  uint8_t addressString[MAX_STRING_LENGTH];
  uint32_t* int32uPointers[] = { &hostOrderIpAddress, &hostOrderNetmask, &hostOrderGateway };

  const uint8_t* argumentNames[] = {
    "IPv4 Address",
    "Netmask",
    "Gateway",
  };

  uint8_t i;
  for (i = 0; i < 3; i++ ) {
    if (!emberStringArgumentToHostOrderIpv4Address(i, int32uPointers[i])) {
      emberCopyStringArgument(i,
                              addressString,
                              MAX_STRING_LENGTH,
                              false);  // left pad?
      emberAfCorePrintln("Error: %p '%p' is invalid.",
                         argumentNames[i],
                         addressString);
      return;
    }

    if (i != 1) {
      // Netmask is allowed to break the rules of host addresses.
      if (!validateIpv4HostAddress(argumentNames[i], *(int32uPointers[i]))) {
        return;
      }
    }
  }

  emberAfPluginLwipSetStaticIpv4Address(hostOrderIpAddress, hostOrderNetmask, hostOrderGateway);
}

void emberAfPluginLwipSetMacAddressCommand(void)
{
  uint8_t length;
  uint8_t *enet = emberStringCommandArgument(0, &length);
  halCommonSetMfgToken(TOKEN_MFG_ETHERNET_ADDRESS, enet);
  emberAfCorePrintln("Done");
}

void emberAfPluginLwipIfconfigCommand(void)
{
  struct netif *n;

  ETHERNET_PRINT_LEVEL_DEBUG;
  n = emberAfPluginLwipGetNetIf();
  emberAfCorePrint("(>) LWIP NETIF Status: ");
  if (netif_is_up(n)) {
    emberAfCorePrintln("UP");
  } else {
    emberAfCorePrintln("DOWN");
  }

  emberAfCorePrint("Ethernet Link Status: ");
  if (ETHERNET_CHECK_LINK_IS_UP) {
    emberAfCorePrintln("UP");
  } else {
    emberAfCorePrintln("DOWN");
  }
  emberAfCorePrint("MAC Address: ");
  micrelPrintMacAddress();
  emberAfCorePrint("\r\n");

  micrelPrintEthernetRxStats();
  micrelPrintVersion();
  ETHERNET_PRINT_LEVEL_NONE;
}

void emberAfPluginLwipIfupCommand(void)
{
  netif_set_up(emberAfPluginLwipGetNetIf());
}

void emberAfPluginLwipIfdownCommand(void)
{
  netif_set_down(emberAfPluginLwipGetNetIf());
}

void emberAfPluginLwipPrintEnetCommand(void)
{
  uint8_t level = (uint8_t)emberUnsignedCommandArgument(0);
  if (level < 1) {
    ETHERNET_PRINT_LEVEL_NONE;
  } else if (level == 1) {
    ETHERNET_PRINT_LEVEL_DEBUG;
  } else {
    ETHERNET_PRINT_LEVEL_ALL;
  }
}

void emberAfPluginLwipEnableDhcpCommand(void)
{
  uint8_t enable = (uint8_t)emberUnsignedCommandArgument(0);
  EmberAfPluginLwipIpv4TokenStruct ipv4TokenStruct;
  halCommonGetToken(&ipv4TokenStruct, TOKEN_AFV2_PLUGIN_LWIP_IPV4);
  if (enable) {
    ipv4TokenStruct.flags |=  EMBER_AF_PLUGIN_LWIP_TOKEN_DHCP_VALUE;
  } else {
    uint32_t flags = EMBER_AF_PLUGIN_LWIP_TOKEN_DHCP_VALUE;
    ipv4TokenStruct.flags &= ~flags;
  }
  halCommonSetToken(TOKEN_AFV2_PLUGIN_LWIP_IPV4, &ipv4TokenStruct);
}
#endif //DOXYGEN_SHOULD_SKIP_THIS
