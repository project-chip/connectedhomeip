/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2020 Nest Labs, Inc.
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
/* this file behaves like a config.h, comes first */
#include <platform/internal/CHIPDeviceLayerInternal.h>

#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/ConnectivityManager.h>
#include <platform/internal/GenericConnectivityManagerImpl_UDP.ipp>

#if INET_CONFIG_ENABLE_TCP_ENDPOINT
#include <platform/internal/GenericConnectivityManagerImpl_TCP.ipp>
#endif

#include <lwip/dns.h>
#include <lwip/ip_addr.h>
#include <lwip/nd6.h>
#include <lwip/netif.h>

extern "C" {
#include "wlan.h"
void test_wlan_scan(int argc, char ** argv);
void test_wlan_add(int argc, char ** argv);
}

#if CHIP_DEVICE_CONFIG_ENABLE_WPA
#include <platform/internal/GenericConnectivityManagerImpl_WiFi.ipp>
#endif

using namespace ::chip;
using namespace ::chip::Inet;
using namespace ::chip::System;
using namespace ::chip::TLV;
using namespace ::chip::DeviceLayer::Internal;

namespace chip {
namespace DeviceLayer {

ConnectivityManagerImpl ConnectivityManagerImpl::sInstance;

CHIP_ERROR ConnectivityManagerImpl::_Init()
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    mWiFiStationMode                = kWiFiStationMode_Disabled;
    mWiFiStationReconnectIntervalMS = CHIP_DEVICE_CONFIG_WIFI_STATION_RECONNECT_INTERVAL;

    // Initialize the generic base classes that require it.

    SuccessOrExit(err);

exit:
    return err;
}

void ConnectivityManagerImpl::_OnPlatformEvent(const ChipDeviceEvent * event)
{
    // Forward the event to the generic base classes as needed.
}

#if CHIP_DEVICE_CONFIG_ENABLE_WPA

ConnectivityManager::WiFiStationMode ConnectivityManagerImpl::_GetWiFiStationMode()
{
    if (mWiFiStationMode != kWiFiStationMode_ApplicationControlled)
    {
        mWiFiStationMode =
            kWiFiStationMode_Enabled; //(mWpaSupplicant.iface != nullptr) ? kWiFiStationMode_Enabled : kWiFiStationMode_Disabled;
    }

    return mWiFiStationMode;
}

CHIP_ERROR ConnectivityManagerImpl::_SetWiFiStationMode(ConnectivityManager::WiFiStationMode val)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    VerifyOrExit(val != ConnectivityManager::kWiFiStationMode_NotSupported, err = CHIP_ERROR_INVALID_ARGUMENT);

    if (mWiFiStationMode != val)
    {
        ChipLogProgress(DeviceLayer, "WiFi station mode change: %s -> %s", WiFiStationModeToStr(mWiFiStationMode),
                        WiFiStationModeToStr(val));
    }

    mWiFiStationMode = val;
    test_wlan_scan(0, NULL);
exit:
    return err;
}

CHIP_ERROR ConnectivityManagerImpl::_SetWiFiAPMode(WiFiAPMode val)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    VerifyOrExit(val != kWiFiAPMode_NotSupported, err = CHIP_ERROR_INVALID_ARGUMENT);

    if (mWiFiAPMode != val)
    {
        ChipLogProgress(DeviceLayer, "WiFi AP mode change: %s -> %s", WiFiAPModeToStr(mWiFiAPMode), WiFiAPModeToStr(val));
    }

    mWiFiAPMode = val;
exit:
    return err;
}

bool ConnectivityManagerImpl::_IsWiFiStationEnabled()
{
    return GetWiFiStationMode() == kWiFiStationMode_Enabled;
}

bool ConnectivityManagerImpl::_IsWiFiStationConnected()
{
    bool ret = false;

    return ret;
}

bool ConnectivityManagerImpl::_IsWiFiStationApplicationControlled()
{
    return mWiFiStationMode == ConnectivityManager::kWiFiStationMode_ApplicationControlled;
}

void ConnectivityManagerImpl::StartWiFiManagement() {}
#endif // CHIP_DEVICE_CONFIG_ENABLE_WPA

CHIP_ERROR ConnectivityManagerImpl::ProvisionWiFiNetwork(const char * ssid, const char * key)
{
#if CHIP_DEVICE_CONFIG_ENABLE_WPA
    CHIP_ERROR ret = CHIP_NO_ERROR;
    int ret_mcuXpresso;
    char arg0[] = "wlan-add";
    char arg1[32];
    char arg2[] = "ssid";
    char arg3[32];
    char arg4[] = "wpa2";
    char arg5[64];
    char * argv[] = { &arg0[0], &arg1[0], &arg2[0], &arg3[0], &arg4[0], &arg5[0], NULL };
    int argc      = (int) (sizeof(argv) / sizeof(argv[0])) - 1;

    sprintf((char *) arg1, "%s", ssid);
    sprintf((char *) arg3, "%s", ssid);
    sprintf((char *) arg5, "%s", key);
    test_wlan_add(argc, &argv[0]);
    ret_mcuXpresso = wlan_connect(argv[1]);
    if (ret_mcuXpresso == WLAN_ERROR_STATE)
    {
        ChipLogProgress(DeviceLayer, "Error: connect manager not running");
        ret = CHIP_ERROR_CONNECTION_CLOSED_UNEXPECTEDLY;
    }
    if (ret_mcuXpresso == -WM_E_INVAL)
    {
        ChipLogProgress(DeviceLayer, "Error: specify a network to connect");
        ret = CHIP_ERROR_INVALID_ARGUMENT;
    }
    else
    {
        ChipLogProgress(DeviceLayer,
                        "Connecting to network...\r\nUse 'wlan-stat' for "
                        "current connection status.");
    }
    return ret;
#else
    return CHIP_ERROR_NOT_IMPLEMENTED;
#endif
}

} // namespace DeviceLayer
} // namespace chip
