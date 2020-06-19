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
 * @brief This is an IP packet based lwIP network interface to connect
 * with the Ember znet stack.
 *******************************************************************************
   ******************************************************************************/

#ifndef DOXYGEN_SHOULD_SKIP_THIS
#include "app/framework/include/af.h"
#include "lwip/common-lwipopts.h"
#include "lwip/opt.h"
#include "lwip/def.h"
#include "lwip/pbuf.h"
#include "lwip/sys.h"
#include "lwip/stats.h"
#include "lwip/snmp.h"
#include "lwip/netif.h"
#include "lwip/sockets.h"
#include "lwip/tcpip.h"
#include "lwip/def.h"
#include "lwip/ip4_addr.h"
#include "lwip/ip_addr.h"
#include "netif/etharp.h"
#include "lwip/err.h"

#include "app/framework/plugin/lwip/lwip-main.h"
#include "app/framework/plugin/lwip/micrel/micrel-ethernet.h"
#include "lwip/src/include/lwip/netifapi.h"

#include "app/framework/plugin/lwip/lwip-plugin.h"

//============================================================================
// Globals

static struct netif  main_netif; // master netif
static EmberLwipStateCallbackFunc registeredLwipStateCallbacks[EMBER_MAX_LWIP_STATE_CALLBACK_REGISTRATIONS];
static bool isDnsEnabled = false;

struct sockaddr_in servaddr, cliaddr;
socklen_t clilen;
struct tcp_pcb *pcb;

//=============================================================================
// Forward Declarations
static err_t initializeEthernetDriver(struct netif *netif);
static void dhcpCompleteCallback(struct netif *netif);
//=============================================================================
// Local Functions

static err_t initializeEthernetDriver(struct netif *netif)
{
  err_t ret;
  if (main_netif.hwaddr[0] == 0xFF) {
    emberAfCorePrintln("initializeEthernetDriver:  Invalid MAC configured! Configure using 'plugin lwip set-mac-address'");
    ret = ERR_IF;
  } else {
    micrelSetEthernetDebugPrint(0); // Start quietly. Set to 2 for verbose
    micrelEthernetInit(netif);      // Turning the ignition key...
    if (ETHERNET_CHECK_LINK_IS_UP) {// Check is done natively. Not using Lwip Netif.
      emberAfCorePrintln("initializeEthernetDriver: INIT COMPLETE");
      ret = ERR_OK;
    } else {
      emberAfCorePrintln("initializeEthernetDriver: Ethernet Cable unplugged!");
      ret = ERR_CONN;
    }
  }
  return ret;
}

void emberAfPluginLwipPrintBigEndianEthernetAddress(const EmberEthernetAddress* ethernetAddress)
{
  emberAfCorePrint("(>)%X%X%X%X%X%X",
                   ethernetAddress->contents[0],
                   ethernetAddress->contents[1],
                   ethernetAddress->contents[2],
                   ethernetAddress->contents[3],
                   ethernetAddress->contents[4],
                   ethernetAddress->contents[5]);
}

void emberAfPluginLwipPrintHostOrderIpv4Address(uint32_t hostOrderIpv4Address)
{
  emberAfCorePrint("%d.%d.%d.%d",
                   (hostOrderIpv4Address >> 24) & 0xFF,
                   (hostOrderIpv4Address >> 16) & 0xFF,
                   (hostOrderIpv4Address >> 8)  & 0xFF,
                   (hostOrderIpv4Address)       & 0xFF);
}

void emberAfPluginLwipPrintIpv4Parameters(uint32_t hostOrderIpv4Address,
                                          uint32_t hostOrderNetmask,
                                          uint32_t hostOrderGateway)
{
  emberAfCorePrint("IPv4 Address: ");
  emberAfPluginLwipPrintHostOrderIpv4Address(hostOrderIpv4Address);
  emberAfCorePrintln("");
  emberAfCorePrint("Netmask:      ");
  emberAfPluginLwipPrintHostOrderIpv4Address(hostOrderIpv4Netmask);
  emberAfCorePrintln("");
  emberAfCorePrint("Gateway:      ");
  emberAfPluginLwipPrintHostOrderIpv4Address(hostOrderIpv4Gateway);
  emberAfCorePrintln("");
}

void emberAfPluginLwipPrintIpv4Token(const EmberAfPluginLwipIpv4TokenStruct* ipv4Token)
{
  bool useDhcp = ((ipv4Token->flags & EMBER_AF_PLUGIN_LWIP_TOKEN_DHCP_MASK)
                  == EMBER_AF_PLUGIN_LWIP_TOKEN_DHCP_VALUE);

  emberAfCorePrintln("DHCP: %p", (useDhcp ? "yes" : "no"));
  emberAfPluginLwipPrintIpv4Parameters(ipv4Token->hostOrderIpv4Address,
                                       ipv4Token->hostOrderIpv4Netmask,
                                       ipv4Token->hostOrderIpv4Gateway);
}

void emberAfPluginLwipSetIpv4AddressFromToken(void)
{
  EmberAfPluginLwipIpv4TokenStruct ipv4TokenStruct;
  halCommonGetToken(&ipv4TokenStruct, TOKEN_AFV2_PLUGIN_LWIP_IPV4);

  emberAfCorePrintln("Setting IPv4 parameters from tokens.");
  emberAfPluginLwipPrintIpv4Token(&ipv4TokenStruct);

  netif_set_down(&main_netif);

  ip_addr_t ipAddress;
  ip_addr_t gateway;
  ip_addr_t netmask;

  ip4_addr_set_u32(&ipAddress, htonl(ipv4TokenStruct.hostOrderIpv4Address));
  ip4_addr_set_u32(&netmask, htonl(ipv4TokenStruct.hostOrderIpv4Netmask));
  ip4_addr_set_u32(&gateway, htonl(ipv4TokenStruct.hostOrderIpv4Gateway));

  netif_set_addr(&main_netif,
                 &ipAddress,
                 &netmask,
                 &gateway);

  netif_set_up(&main_netif);
  /* Do Not Enable Interface Here. Controller may not be configured at this point in time*/
}

/*
 * Send a pbuf over the Ethernet interface.
 *
 * @param netif the lwip network interface structure for this znetif
 * @param p the pbuf chaing packet to send
 * @param ipaddr the ip address to send the packet to
 * @return Always returns ERR_OK
 */
err_t emberAfPluginLwipOutput(struct netif *netif, struct pbuf *p, ip_addr_t *ipaddr)
{
  LWIP_ASSERT("netif != NULL", (netif != NULL));
  LWIP_ASSERT("netif->state != NULL", (netif->state != NULL));
  LWIP_ASSERT("p != NULL", (p != NULL));

  etharp_output(netif, p, ipaddr);

  return ERR_OK;
}

#if LWIP_NETIF_STATUS_CALLBACK
static void setStateCallbackRegistration(EmberLwipStateCallbackFunc *cb, netif_status_callback_fn handler, const uint16_t handlerID)
{
  cb->handler = handler;
  cb->handlerID = handlerID;
}

void clearStateCallbackRegistrations(void)
{
  emberAfCorePrintln("Clearing Lwip netif state callback registrations.");
  EmberLwipStateCallbackFunc *cb = &registeredLwipStateCallbacks[0];
  EmberLwipStateCallbackFunc *cbend = cb  + EMBER_MAX_LWIP_STATE_CALLBACK_REGISTRATIONS * sizeof(EmberLwipStateCallbackFunc);

  do {
    setStateCallbackRegistration(cb, NULL, 0x0001);
    cb++;
  } while (cb < cbend);
}

bool emberAfPluginLwipStateRegisterCallback(netif_status_callback_fn handler, const uint16_t handlerID)
{
  bool registered = false;
  EmberLwipStateCallbackFunc *cb = &registeredLwipStateCallbacks[0];
  EmberLwipStateCallbackFunc *cbend = &registeredLwipStateCallbacks[EMBER_MAX_LWIP_STATE_CALLBACK_REGISTRATIONS];

  if (handlerID > 0x0001 && handlerID !=  0xFFFF && handler != NULL) {
    do {
      if (cb->handler == NULL && cb->handlerID == 0x0001) {
        /* Find an empty slot and register callback, if provided a valid handlerID */
        setStateCallbackRegistration(cb, handler, handlerID);
        emberAfCorePrintln("emberAfPluginLwipStateRegisterCallback: Registered callback handler. id[0x%2X]", handlerID);
        registered = true;
        break;
      }
      cb++;
    } while (cb < cbend);
  }

  return registered;
}

static EmberStatus startOrRenewDhcp(void)
{
  EmberStatus status = EMBER_ERR_FATAL;
  err_t result;
  if (main_netif.ip_addr.addr == 0) {
    emberAfCorePrintln("No IP Address detected. Starting DHCP");
    result = netifapi_dhcp_start(&main_netif);
  } else {
    emberAfCorePrintln("Attempting to renew lease on: ");
    emberAfPluginLwipPrintHostOrderIpv4Address(ntohl(main_netif.ip_addr.addr));
    result = netifapi_dhcp_renew(&main_netif);
  }
  if (result == 0) {
    status = EMBER_SUCCESS;
  }
  return status;
}
void emberAfPluginLwipLinkStatusCallback(void)
{
  EmberAfPluginLwipIpv4TokenStruct ipv4TokenStruct;
  halCommonGetToken(&ipv4TokenStruct, TOKEN_AFV2_PLUGIN_LWIP_IPV4);
  emberAfCorePrintln("emberAfPluginLwipLinkStatusCallback called");
  /* Use netif_is_link_up(netif) to determine link state
   * In the majority of cases, applications requiring status should use
   * emberAfPluginLwipStateRegisterCallback()  and  register a callback.
   * Use netif_is_up(netif) to determine status */

  //While the device is still up, if we lose the link.
  if (emberAfPluginLwipIsDhcpEnabled()) {
    if (netif_is_link_up(&main_netif)) {
      EmberStatus result = startOrRenewDhcp();
      emberAfCorePrintln("Starting dhcp 0x%x", result);
    }
  } else if (netif_is_link_up(&main_netif)) {
    //Not using DHCP. Just set the token ip address and call the netif up.
    emberAfPluginLwipSetIpv4AddressFromToken();
  }
}

void emberAfPluginLwipNetifStatusCallback(void)
{
  EmberLwipStateCallbackFunc *cb = &registeredLwipStateCallbacks[0];
  EmberLwipStateCallbackFunc *cbend = &registeredLwipStateCallbacks[EMBER_MAX_LWIP_STATE_CALLBACK_REGISTRATIONS];
  emberAfCorePrint("emberAfPluginLwipNetifStatusCallback called: ");
  if (netif_is_up(&main_netif)) {
    emberAfCorePrintln("NETIF UP");
  } else {
    emberAfCorePrintln("NETIF DOWN");
  }

  do {
    if (cb->handler != NULL && cb->handlerID > 0x0001 && cb->handlerID !=  0xFFFF) {
      (cb->handler)(&main_netif);
    }
    cb++;
  } while (cb < cbend);
}

#endif

/* ***************************************************************************** *
* Simplified Startup Sequence Description:                                      *
*       1. FreeRTOS startup calls emberAfPluginLwipInitCallback()                *
*           1a. Load MAC from token into netif                                   *
*     2. netif_add calls  initializeEthernetDriver()                            *
*         2a. initializeEthernetDriver calls micrel ethernet driver             *
*    3. Load IP address or start DHCP                                           *
*    4. Enable Lwip Interface to make it available to upper layers.             *
*                                                                               *
*   Best Practices:                                                              *
*   >>Design goal is to manage netif contents as much as possible outside       *
*        the ethernet driver.                                                   *
*  >> Use lwip API functions as much as possible.                               *
*  >> If you need to change the initialization steps please do so carefully     *
* ***************************************************************************** */
void emberAfPluginLwipInitCallback(void)
{
  emberAfCorePrintln("Adding Ethernet...");

  halCommonGetMfgToken(main_netif.hwaddr, TOKEN_MFG_ETHERNET_ADDRESS); // load MAC into netif.
  tcpip_init(NULL, NULL);                           // Required by LWIP. Must be done here.
  clearStateCallbackRegistrations();

  if (netif_add(&main_netif, NULL, NULL, NULL, NULL, initializeEthernetDriver, ethernet_input) == NULL) {
    emberAfCorePrintln("netif_add Failed!!!");
  } else {
    main_netif.output = emberAfPluginLwipOutput;         //Add tx handler.

    netif_set_link_callback(&main_netif, (netif_status_callback_fn)emberAfPluginLwipLinkStatusCallback);     // Called by ethernet driver.

    netif_set_status_callback(&main_netif, (netif_status_callback_fn)emberAfPluginLwipNetifStatusCallback);   // Called by lwip

    if (!emberAfPluginLwipIsDhcpEnabled()) {
      emberAfPluginLwipSetIpv4AddressFromToken();      // load ip into netif
      netif_set_up(&main_netif);                       // enable lwip interface
    } else {
      emberAfPluginLwipStateRegisterCallback((netif_status_callback_fn)dhcpCompleteCallback, 0xD4C9);
      EmberStatus result = startOrRenewDhcp();
      emberAfCorePrintln("Using DHCP to Acquire IPv4 Address... 0x%x", result);
    }

    netif_set_default(&main_netif);
  }
}

void emberAfPluginLwipTickCallback(void)
{
  // Check if any ethernet rx packets pending
  micrelEthernetTick(&main_netif);
}

/* Returns reference to current netif. */
struct netif* emberAfPluginLwipGetNetIf(void)
{
  return &main_netif;
}

static void dhcpCompleteCallback(struct netif *netif)
{
  if (netif->flags & NETIF_FLAG_UP == NETIF_FLAG_UP) {
    emberAfCorePrint("Netif up: ");
    emberAfPluginLwipPrintHostOrderIpv4Address(ntohl(netif->ip_addr.addr));
    emberAfCorePrintln("");
  } else {
    //If the netif was brought down, we try a new address.
    emberAfCorePrintln("Netif down");
  }
}

bool emberAfPluginLwipIsDhcpEnabled(void)
{
  EmberAfPluginLwipIpv4TokenStruct ipv4TokenStruct;
  halCommonGetToken(&ipv4TokenStruct, TOKEN_AFV2_PLUGIN_LWIP_IPV4);
  bool useDhcp = ((ipv4TokenStruct.flags & EMBER_AF_PLUGIN_LWIP_TOKEN_DHCP_MASK)
                  == EMBER_AF_PLUGIN_LWIP_TOKEN_DHCP_VALUE);
  return useDhcp;
}

bool emberAfPluginLwipIsDnsEnabled(void)
{
  return isDnsEnabled;
}

#endif // DOXYGEN_SHOULD_SKIP_THIS
