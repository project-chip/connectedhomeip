/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

#if LWIP_IPV4 && LWIP_DHCP

#pragma once
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif

// DHCP client states
#define DHCP_OFF ((uint8_t) 0)
#define DHCP_START ((uint8_t) 1)
#define DHCP_WAIT_ADDRESS ((uint8_t) 2)
#define DHCP_ADDRESS_ASSIGNED ((uint8_t) 3)
#define DHCP_TIMEOUT ((uint8_t) 4)
#define DHCP_LINK_DOWN ((uint8_t) 5)

/************************** Station Static Default ****************************/
#define STA_IP_ADDR0_DEFAULT ((uint8_t) 192) ///< Static IP: IP address value 0
#define STA_IP_ADDR1_DEFAULT ((uint8_t) 168) ///< Static IP: IP address value 1
#define STA_IP_ADDR2_DEFAULT ((uint8_t) 0)   ///< Static IP: IP address value 2
#define STA_IP_ADDR3_DEFAULT ((uint8_t) 1)   ///< Static IP: IP address value 3

/*NETMASK*/
#define STA_NETMASK_ADDR0_DEFAULT ((uint8_t) 255) ///< Static IP: Netmask value 0
#define STA_NETMASK_ADDR1_DEFAULT ((uint8_t) 255) ///< Static IP: Netmask value 1
#define STA_NETMASK_ADDR2_DEFAULT ((uint8_t) 255) ///< Static IP: Netmask value 2
#define STA_NETMASK_ADDR3_DEFAULT ((uint8_t) 0)   ///< Static IP: Netmask value 3

/*Gateway Address*/
#define STA_GW_ADDR0_DEFAULT ((uint8_t) 0) ///< Static IP: Gateway value 0
#define STA_GW_ADDR1_DEFAULT ((uint8_t) 0) ///< Static IP: Gateway value 1
#define STA_GW_ADDR2_DEFAULT ((uint8_t) 0) ///< Static IP: Gateway value 2
#define STA_GW_ADDR3_DEFAULT ((uint8_t) 0) ///< Static IP: Gateway value 3

/*****************************************************************************
 * @fn  void dhcpclient_set_link_state(int link_up)
 * @brief
 * Notify DHCP client task about the wifi status
 * @param link_up link status
 ******************************************************************************/
void dhcpclient_set_link_state(int link_up);
uint8_t dhcpclient_poll(void * arg);
#ifdef __cplusplus
}
#endif

#endif /* LWIP_IPV4 && LWIP_DHCP */
