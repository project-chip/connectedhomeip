/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
 *    Copyright (c) 2018 Nest Labs, Inc.
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

#include "platform/internal/DeviceNetworkInfo.h"
#include <cy_wcm.h>
#include <platform/internal/CHIPDeviceLayerInternal.h>

typedef struct
{
    cy_wcm_ssid_t ssid;           /**< SSID of the Wi-Fi network to join; should be a null-terminated string. */
    cy_wcm_passphrase_t password; /**< Password needed to join the AP; should be a null-terminated string. */
    cy_wcm_security_t security;   /**< Wi-Fi Security. @see cy_wcm_security_t. */
    cy_wcm_mac_t BSSID;
    cy_wcm_ip_setting_t * static_ip_settings;
} wifi_config_sta_t;

typedef struct
{
    cy_wcm_ssid_t ssid;           /**< SSID of the Wi-Fi network to join; should be a null-terminated string. */
    cy_wcm_passphrase_t password; /**< Password needed to join the AP; should be a null-terminated string. */
    cy_wcm_security_t security;   /**< Wi-Fi Security. @see cy_wcm_security_t. */
    uint8_t channel;
    cy_wcm_ip_setting_t ip_settings;
} wifi_config_ap_t;

typedef struct
{
    wifi_config_sta_t sta; /**< configuration of STA */
    wifi_config_ap_t ap;   /**< configuration of AP */
} wifi_config_t;

typedef enum
{
    WIFI_MODE_NULL = 0, /**< null mode */
    WIFI_MODE_STA,      /**< WiFi station mode */
    WIFI_MODE_AP,       /**< WiFi soft-AP mode */
    WIFI_MODE_APSTA,    /**< WiFi station + soft-AP mode */
    WIFI_MODE_MAX
} wifi_mode_t;

typedef enum
{
    WIFI_IF_STA    = CY_WCM_INTERFACE_TYPE_STA,
    WIFI_IF_AP     = CY_WCM_INTERFACE_TYPE_AP,
    WIFI_IF_STA_AP = CY_WCM_INTERFACE_TYPE_AP_STA,
} wifi_interface_t;

namespace chip {
namespace DeviceLayer {
namespace Internal {

class P6Utils
{
public:
    static CHIP_ERROR IsAPEnabled(bool & apEnabled);
    static bool IsStationProvisioned(void);
    static CHIP_ERROR IsStationConnected(bool & connected);
    static CHIP_ERROR StartWiFiLayer(void);
    static CHIP_ERROR EnableStationMode(void);
    static CHIP_ERROR SetAPMode(bool enabled);
    static int OrderScanResultsByRSSI(const void * _res1, const void * _res2);
    static const char * WiFiModeToStr(wifi_mode_t wifiMode);
    static struct netif * GetNetif(const char * ifKey);
    static struct netif * GetStationNetif(void);
    static bool IsInterfaceUp(const char * ifKey);
    static bool HasIPv6LinkLocalAddress(const char * ifKey);

    static CHIP_ERROR GetWiFiStationProvision(Internal::DeviceNetworkInfo & netInfo, bool includeCredentials);
    static CHIP_ERROR SetWiFiStationProvision(const Internal::DeviceNetworkInfo & netInfo);
    static CHIP_ERROR ClearWiFiStationProvision(void);
    static cy_rslt_t p6_wifi_get_config(wifi_interface_t interface, wifi_config_t * conf);
    static cy_rslt_t p6_wifi_set_config(wifi_interface_t interface, wifi_config_t * conf);
    static CHIP_ERROR p6_wifi_disconnect(void);
    static CHIP_ERROR p6_wifi_connect(void);
    static CHIP_ERROR p6_start_ap(void);
    static CHIP_ERROR p6_stop_ap(void);
    // Fills out some common fields in a wifi_config_t. Use interface param to
    // select whether sta or ap config is populated.
    static void populate_wifi_config_t(wifi_config_t * wifi_config, wifi_interface_t interface, const cy_wcm_ssid_t * ssid,
                                       const cy_wcm_passphrase_t * password, cy_wcm_security_t security = CY_WCM_SECURITY_OPEN);
    static CHIP_ERROR OnIPAddressAvailable(void);
    static CHIP_ERROR ping_init(void);
};

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
