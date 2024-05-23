/*
 *    Copyright (c) 2022 Project CHIP Authors
 *    All rights reserved.
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

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <pkg_protocol.h>
#include <virt_net.h>

struct netif * deviceInterface_getNetif(void);
void wifiInterface_getMacAddress(uint8_t * pmac);
void wifiInterface_connect(char * ssid, char * passwd);
void wifiInterface_disconnect(void);
struct bflbwifi_ap_record * wifiInterface_getApInfo(void);
void wifiInterface_startScan(void);

void wifiInterface_eventConnected(struct netif * interface);
void wifiInterface_eventDisconnected(struct netif * interface);
void wifiInterface_eventApDisconnected(struct netif * interface);
void wifiInterface_eventLinkStatusDone(struct netif * interface, netbus_fs_link_status_ind_cmd_msg_t * pkg_data);
void wifiInterface_eventGotIP(struct netif * interface);
void wifiInterface_eventScanDone(struct netif * interface, netbus_fs_scan_ind_cmd_msg_t * pmsg);

void wifi_event_handler(uint32_t code);
bool wifi_start_firmware_task(void);

void network_netif_ext_callback(struct netif * netif, netif_nsc_reason_t reason, const netif_ext_callback_args_t * args);

#ifdef __cplusplus
}
#endif
