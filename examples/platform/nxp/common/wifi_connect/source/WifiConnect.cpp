/*
 *  Copyright 2023-2024 NXP
 *  All rights reserved.
 *
 *  SPDX-License-Identifier: BSD-3-Clause
 */

#include "WifiConnect.h"
#include <lib/support/Span.h>
#include <platform/ConnectivityManager.h>
#include <platform/NetworkCommissioning.h>
#include <platform/PlatformManager.h>
#include <platform/nxp/common/NetworkCommissioningDriver.h>

using namespace ::chip::DeviceLayer;

static void WifiConnectPostConnectivityLostEvent(void)
{
    CHIP_ERROR err;
    ChipDeviceEvent event;

    event.Type                          = DeviceEventType::kWiFiConnectivityChange;
    event.WiFiConnectivityChange.Result = kConnectivity_Lost;
    (void) PlatformMgr().PostEvent(&event);
}

/* Creates a dedicated Task responsible for connecting to a WiFi network */
void chip::NXP::App::WifiConnectAtboot(void)
{
    NetworkCommissioning::NetworkIterator * networks;
    NetworkCommissioning::Network currentNetwork;
    CHIP_ERROR chip_err;
    NetworkCommissioning::Status status;
    chip::MutableCharSpan debugText;
    uint8_t networkIndex;
    int len_ssid, len_pass;
    chip::ByteSpan ssid;
    chip::ByteSpan password;

    networks = NetworkCommissioning::NXPWiFiDriver::GetInstance().GetNetworks();
    if (networks == nullptr || networks->Count() == 0)
    {
#ifdef CONFIG_CHIP_APP_WIFI_SSID
        /* If SSID and PASSWORD are entered at build time, they can be used */
        ssid = chip::ByteSpan(reinterpret_cast<const uint8_t *>(CONFIG_CHIP_APP_WIFI_SSID), strlen(CONFIG_CHIP_APP_WIFI_SSID));
        ChipLogProgress(DeviceLayer, "Info: Using the default SSID %s", ssid.data());
#endif

#ifdef CONFIG_CHIP_APP_WIFI_PASSWORD
        /* If SSID and PASSWORD are entered at build time, they can be used */
        password =
            chip::ByteSpan(reinterpret_cast<const uint8_t *>(CONFIG_CHIP_APP_WIFI_PASSWORD), strlen(CONFIG_CHIP_APP_WIFI_PASSWORD));
        ChipLogProgress(DeviceLayer, "Info: Using the default password %s", password.data());
#endif

        /* If SSID and Password are filled, WiFi can be automatically started */
        if (IsSpanUsable(ssid) && IsSpanUsable(password))
        {
            ChipLogProgress(DeviceLayer, "Connecting to Wi-Fi network: SSID = %s and PASSWORD = %s", ssid.data(), password.data());

            status = NetworkCommissioning::NXPWiFiDriver::GetInstance().AddOrUpdateNetwork(ssid, password, debugText, networkIndex);

            if (status != NetworkCommissioning::Status::kSuccess)
            {
                WifiConnectPostConnectivityLostEvent();
                ChipLogError(DeviceLayer, "Error: AddOrUpdateNetwork: %u", (uint8_t) status);
            }

            /* Connection event will be returned in OnWiFiConnectivityChange from DeviceCallbacks.cpp */
            NetworkCommissioning::NXPWiFiDriver::GetInstance().ConnectNetwork(ssid, nullptr);
        }
        else
        {
            ChipLogError(DeviceLayer, "Wrong SSID and password");
            WifiConnectPostConnectivityLostEvent();
        }
    }
    else
    {
        networks->Release();
    }
}
