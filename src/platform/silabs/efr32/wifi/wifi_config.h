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

#ifndef WIFI_CONFIG_H
#define WIFI_CONFIG_H

#define USE_DHCP_CLIENT_DEFAULT 1 ///< If defined, DHCP is enabled, otherwise static address below is used

/************************** Station Static Default ****************************/
#define STA_IP_ADDR0_DEFAULT (uint8_t) 192 ///< Static IP: IP address value 0
#define STA_IP_ADDR1_DEFAULT (uint8_t) 168 ///< Static IP: IP address value 1
#define STA_IP_ADDR2_DEFAULT (uint8_t) 0   ///< Static IP: IP address value 2
#define STA_IP_ADDR3_DEFAULT (uint8_t) 1   ///< Static IP: IP address value 3

/*NETMASK*/
#define STA_NETMASK_ADDR0_DEFAULT (uint8_t) 255 ///< Static IP: Netmask value 0
#define STA_NETMASK_ADDR1_DEFAULT (uint8_t) 255 ///< Static IP: Netmask value 1
#define STA_NETMASK_ADDR2_DEFAULT (uint8_t) 255 ///< Static IP: Netmask value 2
#define STA_NETMASK_ADDR3_DEFAULT (uint8_t) 0   ///< Static IP: Netmask value 3

/*Gateway Address*/
#define STA_GW_ADDR0_DEFAULT (uint8_t) 0 ///< Static IP: Gateway value 0
#define STA_GW_ADDR1_DEFAULT (uint8_t) 0 ///< Static IP: Gateway value 1
#define STA_GW_ADDR2_DEFAULT (uint8_t) 0 ///< Static IP: Gateway value 2
#define STA_GW_ADDR3_DEFAULT (uint8_t) 0 ///< Static IP: Gateway value 3

#define WLAN_SSID_DEFAULT "AP_name"                      ///< wifi ssid for client mode
#define WLAN_PASSKEY_DEFAULT "passkey"                   ///< wifi password for client mode
#define WLAN_SECURITY_DEFAULT WFM_SECURITY_MODE_WPA2_PSK ///< wifi security mode for client mode:
///< WFM_SECURITY_MODE_OPEN/WFM_SECURITY_MODE_WEP/WFM_SECURITY_MODE_WPA2_WPA1_PSK

#define SOFTAP_SSID_DEFAULT "silabs_softap"                ///< wifi ssid for soft ap mode
#define SOFTAP_PASSKEY_DEFAULT "changeme"                  ///< wifi password for soft ap mode
#define SOFTAP_SECURITY_DEFAULT WFM_SECURITY_MODE_WPA2_PSK ///< wifi security for soft ap mode:
///< WFM_SECURITY_MODE_OPEN/WFM_SECURITY_MODE_WEP/WFM_SECURITY_MODE_WPA2_WPA1_PSK

#define SOFTAP_CHANNEL_DEFAULT 6 ///< wifi channel for soft ap

#endif // WIFI_CONFIG_H
