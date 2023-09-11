/*
 *
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "lwip/err.h"
#include "lwip/netif.h"
#ifdef __cplusplus
extern "C" {
#endif
/***************************************************************************
 * @fn  err_t sta_ethernetif_init(struct netif *netif)
 * @brief
 * Sets up the station network interface.
 *
 * @param netif the lwip network interface structure
 * @returns ERR_OK if successful
 ******************************************************************************/
err_t sta_ethernetif_init(struct netif * netif);

/***************************************************************************
 * @fn err_t ap_ethernetif_init(struct netif *netif
 * @brief
 * Sets up the AP network interface.
 *
 * @param netif the lwip network interface structure
 * @returns ERR_OK if successful
 ******************************************************************************/
err_t ap_ethernetif_init(struct netif * netif);

void wfx_host_received_sta_frame_cb(uint8_t * buf, int len);

#ifdef __cplusplus
}
#endif
