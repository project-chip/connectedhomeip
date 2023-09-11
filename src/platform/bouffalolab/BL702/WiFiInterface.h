/*
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <pkg_protocol.h>

bool wifiInterface_init();

struct netif * deviceInterface_getNetif(void);
void wifiInterface_getMacAddress(uint8_t * pmac);
void wifiInterface_connect(char * ssid, char * passwd);
void wifiInterface_disconnect(void);
bool wifiInterface_getApInfo(struct bflbwifi_ap_record * ap_info);
void wifiInterface_startScan(void);

void wifiInterface_eventConnected(struct netif * interface);
void wifiInterface_eventDisconnected(struct netif * interface);
void wifiInterface_eventApDisconnected(struct netif * interface);
void wifiInterface_eventLinkStatusDone(struct netif * interface, netbus_fs_link_status_ind_cmd_msg_t * pkg_data);
void wifiInterface_eventGotIP(struct netif * interface);
void wifiInterface_eventScanDone(struct netif * interface, netbus_fs_scan_ind_cmd_msg_t * pmsg);

#ifdef __cplusplus
}
#endif
