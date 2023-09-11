/*
 *
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef DEMO_CONFIG_H
#define DEMO_CONFIG_H

#define USE_DHCP_CLIENT_DEFAULT (1) ///< If defined, DHCP is enabled, otherwise static address below is used

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

/************************** Access Point Static Default ****************************/
// #define AP_IP_ADDR0_DEFAULT (uint8_t) 10 ///< Static IP: IP address value 0
// #define AP_IP_ADDR1_DEFAULT (uint8_t) 10 ///< Static IP: IP address value 1
// #define AP_IP_ADDR2_DEFAULT (uint8_t) 0  ///< Static IP: IP address value 2
// #define AP_IP_ADDR3_DEFAULT (uint8_t) 1  ///< Static IP: IP address value 3

/*NETMASK*/
// #define AP_NETMASK_ADDR0_DEFAULT (uint8_t) 255 ///< Static IP: Netmask value 0
// #define AP_NETMASK_ADDR1_DEFAULT (uint8_t) 255 ///< Static IP: Netmask value 1
// #define AP_NETMASK_ADDR2_DEFAULT (uint8_t) 255 ///< Static IP: Netmask value 2
// #define AP_NETMASK_ADDR3_DEFAULT (uint8_t) 0   ///< Static IP: Netmask value 3

/*Gateway Address*/
// #define AP_GW_ADDR0_DEFAULT (uint8_t) 0 ///< Static IP: Gateway value 0
// #define AP_GW_ADDR1_DEFAULT (uint8_t) 0 ///< Static IP: Gateway value 1
// #define AP_GW_ADDR2_DEFAULT (uint8_t) 0 ///< Static IP: Gateway value 2
// #define AP_GW_ADDR3_DEFAULT (uint8_t) 0 ///< Static IP: Gateway value 3

#define WLAN_SSID_DEFAULT "AP_name"                      ///< wifi ssid for client mode
#define WLAN_PASSKEY_DEFAULT "passkey"                   ///< wifi password for client mode
#define WLAN_SECURITY_DEFAULT WFM_SECURITY_MODE_WPA2_PSK ///< wifi security mode for client mode:
///< WFM_SECURITY_MODE_OPEN/WFM_SECURITY_MODE_WEP/WFM_SECURITY_MODE_WPA2_WPA1_PSK

#define SOFTAP_SSID_DEFAULT "silabs_softap"                ///< wifi ssid for soft ap mode
#define SOFTAP_PASSKEY_DEFAULT "changeme"                  ///< wifi password for soft ap mode
#define SOFTAP_SECURITY_DEFAULT WFM_SECURITY_MODE_WPA2_PSK ///< wifi security for soft ap mode:
///< WFM_SECURITY_MODE_OPEN/WFM_SECURITY_MODE_WEP/WFM_SECURITY_MODE_WPA2_WPA1_PSK

#define SOFTAP_CHANNEL_DEFAULT (6) ///< wifi channel for soft ap

#endif // DEMO_CONFIG_H
