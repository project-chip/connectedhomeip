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
#include <platform/internal/CHIPDeviceLayerInternal.h>

#ifdef __cplusplus
extern "C" {
#endif
#include "lega_wlan_api.h"
#ifdef __cplusplus
}
#endif

typedef struct
{
    char wifi_mode;     /* refer to hal_wifi_type_t*/
    char security;      /* security mode, refer to lega_wlan_security_e */
    char wifi_ssid[32]; /* in station mode, indicate SSID of the wlan needs to be connected.
                           in softap mode, indicate softap SSID*/
    char wifi_key[64];  /* in station mode, indicate Security key of the wlan needs to be connected,
                           in softap mode, indicate softap password.(ignored in an open system.) */
    uint32_t ssid_len;
    uint32_t key_len;
} lega_wlan_wifi_conf;

namespace chip {
namespace DeviceLayer {
namespace Internal {

class ASRUtils
{
public:
#if CHIP_DEVICE_CONFIG_ENABLE_WIFI_AP
    static CHIP_ERROR IsAPEnabled(bool & apEnabled);
#endif
    static CHIP_ERROR IsStationEnabled(bool & staEnabled);
    static bool IsStationProvisioned(void);
    static CHIP_ERROR IsStationConnected(bool & connected);
    static CHIP_ERROR SetStationConnected(bool connected);
    static CHIP_ERROR EnableStationMode(void);
    static struct netif * GetNetif(const char * ifKey);
    static struct netif * GetStationNetif(void);
    static bool IsInterfaceUp(const char * ifKey);

    static CHIP_ERROR GetWiFiStationProvision(Internal::DeviceNetworkInfo & netInfo, bool includeCredentials);
    static CHIP_ERROR ClearWiFiStationProvision(void);
    static CHIP_ERROR asr_wifi_get_config(lega_wlan_wifi_conf * conf);
    static CHIP_ERROR asr_wifi_set_config(lega_wlan_wifi_conf * conf);
    static CHIP_ERROR asr_wifi_disconnect(void);
    static CHIP_ERROR asr_wifi_connect(void);
    static CHIP_ERROR OnIPAddressAvailable(void);
    static CHIP_ERROR ping_init(void);
    static CHIP_ERROR GetWiFiSSID(char * buf, size_t bufSize);
    static CHIP_ERROR StoreWiFiSSID(char * buf, size_t size);
    static CHIP_ERROR GetWiFiPassword(char * buf, size_t bufSize);
    static CHIP_ERROR StoreWiFiPassword(char * buf, size_t size);
    static CHIP_ERROR GetWiFiSecurityCode(uint32_t & security);
    static CHIP_ERROR StoreWiFiSecurityCode(uint32_t security);
    static CHIP_ERROR wifi_get_mode(uint32_t & security);
    static CHIP_ERROR wifi_set_mode(uint32_t security);
};

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
