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
 * @brief This is an lwIP network interface that will connect to
 * the Ember ZigbeePro stack.
 *******************************************************************************
   ******************************************************************************/

#include "lwip/netif.h"

#define EMBER_ETHERNET_ADDRESS_SIZE 6
#define EMBER_MAX_LWIP_STATE_CALLBACK_REGISTRATIONS 4

typedef struct {
  uint8_t contents[EMBER_ETHERNET_ADDRESS_SIZE];
} EmberEthernetAddress;

typedef struct {
  netif_status_callback_fn handler;
  uint16_t handlerID;
}EmberLwipStateCallbackFunc;

/*
 * Sends a pbuf to the Ember ZigbeePro stack.
 *
 * @param netif The LWIP network interface structure for this zipif.
 * @param p The pbuf chaining packet to send.
 * @param ipaddr The IP address to send the packet to.
 * @return Always returns ERR_OK.
 */
err_t emberAfPluginLwipOutput(struct netif *netif, struct pbuf *p, ip_addr_t *ipaddr);

void emberAfPluginLwipSetIpv4AddressFromToken(void);

void emberAfPluginLwipPrintIpv4Token(const EmberAfPluginLwipIpv4TokenStruct* ipv4Token);

struct netif  *emberAfPluginLwipGetNetIf(void);

void emberAfPluginLwipPrintBigEndianEthernetAddress(const EmberEthernetAddress* ethernetAddress);

void emberAfPluginLwipPrintHostOrderIpv4Address(uint32_t hostOrderIpv4Address);

void emberAfPluginLwipPrintIpv4Parameters(uint32_t hostOrderIpv4Address,
                                          uint32_t hostOrderNetmask,
                                          uint32_t hostOrderGateway);

void emberAfPluginLwipTickCallback(void);

bool emberAfPluginLwipStateRegisterCallback(netif_status_callback_fn handler, const uint16_t handlerID);
