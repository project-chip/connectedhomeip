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

#pragma once

#include "lwip/err.h"
#include "lwip/netif.h"

#include "sl_wfx_api.h"
#include "sl_wfx_constants.h"

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

void sl_wfx_host_received_frame_callback(sl_wfx_received_ind_t * rx_buffer);

#ifdef __cplusplus
}
#endif
