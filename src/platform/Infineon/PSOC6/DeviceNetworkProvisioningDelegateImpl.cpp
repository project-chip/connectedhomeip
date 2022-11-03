/*
 * SPDX-FileCopyrightText: (c) 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <lib/support/ErrorStr.h>
#include <lib/support/logging/CHIPLogging.h>

#include "DeviceNetworkProvisioningDelegateImpl.h"
#include <platform/Infineon/PSOC6/P6Utils.h>

namespace chip {
namespace DeviceLayer {

CHIP_ERROR DeviceNetworkProvisioningDelegateImpl::_ProvisionWiFiNetwork(const char * ssid, const char * passwd)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    ChipLogProgress(NetworkProvisioning, "P6NetworkProvisioningDelegate: SSID: %s", ssid);
    err = ConnectivityMgr().SetWiFiStationMode(ConnectivityManager::kWiFiStationMode_Disabled);
    SuccessOrExit(err);

    // Set the wifi configuration
    wifi_config_t wifi_config;
    Internal::P6Utils::populate_wifi_config_t(&wifi_config, WIFI_IF_STA, (const cy_wcm_ssid_t *) ssid,
                                              (const cy_wcm_passphrase_t *) passwd,
                                              (strlen(passwd)) ? CHIP_DEVICE_CONFIG_DEFAULT_STA_SECURITY : CY_WCM_SECURITY_OPEN);

    err = Internal::P6Utils::p6_wifi_set_config(WIFI_IF_STA, &wifi_config);
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
