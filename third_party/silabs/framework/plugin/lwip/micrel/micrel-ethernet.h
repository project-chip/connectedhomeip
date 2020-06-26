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
 * @brief Module for interacting with Ethernet
 *******************************************************************************
   ******************************************************************************/

#ifndef SILABS_MICREL_ETHERNET_H
#define SILABS_MICREL_ETHERNET_H

#include PLATFORM_HEADER //compiler/micro specifics, types
#include "stack/include/ember.h"
#include "stack/include/error.h"
#include "hal/hal.h"
#include "plugin/serial/serial.h"

#ifdef ETHERNET_STANDALONE_DRIVER
  #include "app/slabgw/micrel-ethernet.h"
  #include "app/slabgw/slabgw-arp.h"
  #include "app/slabgw/slabgw-ip.h"
#endif //ETHERNET_STANDALONE_DRIVER

#define ETHERTYPE_ARP  0x0806
#define ETHERTYPE_IPv4 0x0800
#define ETHERTYPE_IPv6 0x86DD
// not standard
#define ETHERTYPE_TEST 0x8000

#define ETHERNET_PRINT_LEVEL_NONE  micrelSetEthernetDebugPrint(0)
#define ETHERNET_PRINT_LEVEL_DEBUG  micrelSetEthernetDebugPrint(1)
#define ETHERNET_PRINT_LEVEL_ALL micrelSetEthernetDebugPrint(2)

#define ETHERNET_CHECK_LINK_IS_UP  micrelLinkStatus() == NETIF_FLAG_LINK_UP

#define MAX_ETHERNET_FRAME_SIZE 1500

#define NETIF_RXCR1_BROADCAST_SET    0xA1
#define NETIF_RXCR1_BROADCAST_CLEAR  0x41
#define NETIF_HWADDR_LEN 6

// ****************************************************************************
// Ethernet Driver - Standard interface
// ****************************************************************************

// the netif->state (experimental)
struct ethStats {
  uint32_t txPackets;
  uint32_t txBytes;
  uint32_t rxPackets;
  uint32_t rxBytes;
  uint32_t rxErrPacketCountZero;
  uint32_t rxErrErrorFlagSet;
  uint32_t rxErrBadLength;
  uint32_t rxErrNoPbufAvail;
};

// module init - must call this at startup
void micrelEthernetInit(struct netif* netif);

// module tick - needs to be called regularly - this services received pkts
void micrelEthernetTick(struct netif* netif);

// check and receive a packet
void micrelEthReceive(struct netif *netif);

void micrelGetCurrentState(void);

uint8_t micrelLinkStatus(void);

void micrelPrintEthernetRxStats(void);

void micrelPrintVersion(void);

void micrelPrintMacAddress(void);

void micrelSetEthernetDebugPrint(uint8_t value);

// ****************************************************************************
// functions for adjusting behavior - mostly for debugging
// ****************************************************************************

void micrelEthTransmitEthPayload(uint8_t* destMacAddr, uint16_t etherType, uint16_t payloadLength, uint8_t* pTxData, struct netif* netif);

// send a raw Ethernet message
int8_t micrelEthTransmitRaw(struct netif* netif, struct pbuf *p);

void micrelDebugReadRegisters(void);

void micrelSpiInit(void);

// turn on/off debug printing
void micrelSetEthernetDebugPrint(uint8_t value);

#endif //__MICREL_ETHERNET_H__
