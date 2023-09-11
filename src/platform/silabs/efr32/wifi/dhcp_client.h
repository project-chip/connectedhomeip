/*
 *
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#if LWIP_IPV4 && LWIP_DHCP

#pragma once
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
