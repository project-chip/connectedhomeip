/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include <lib/support/ErrorStr.h>
#include <lib/support/logging/CHIPLogging.h>

#include "DeviceNetworkProvisioningDelegateImpl.h"
#include "platform/P6/P6Utils.h"

namespace chip {
namespace DeviceLayer {

CHIP_ERROR DeviceNetworkProvisioningDelegateImpl::_ProvisionWiFiNetwork(const char * ssid, const char * passwd)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    cy_rslt_t rslt = CY_RSLT_SUCCESS;

    ChipLogProgress(NetworkProvisioning, "P6NetworkProvisioningDelegate: SSID: %s", ssid);
    err = ConnectivityMgr().SetWiFiStationMode(ConnectivityManager::kWiFiStationMode_Disabled);
    SuccessOrExit(err);

    // Set the wifi configuration
    wifi_config_t wifi_config;
    Internal::P6Utils::populate_wifi_config_t(&wifi_config, WIFI_IF_STA, (const cy_wcm_ssid_t *) ssid,
                                              (const cy_wcm_passphrase_t *) passwd, CHIP_DEVICE_CONFIG_DEFAULT_STA_SECURITY);

    rslt = Internal::P6Utils::p6_wifi_set_config(WIFI_IF_STA, &wifi_config);
    if (rslt != CY_RSLT_SUCCESS)
    {
        err = CHIP_ERROR_INTERNAL;
        ChipLogError(DeviceLayer, "p6_wifi_set_config() failed");
    }
    SuccessOrExit(err);

    err = ConnectivityMgr().SetWiFiStationMode(ConnectivityManager::kWiFiStationMode_Enabled);
    SuccessOrExit(err);

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(NetworkProvisioning, "Failed to connect to WiFi network: %s", chip::ErrorStr(err));
    }

    return err;
}

} // namespace DeviceLayer
} // namespace chip
