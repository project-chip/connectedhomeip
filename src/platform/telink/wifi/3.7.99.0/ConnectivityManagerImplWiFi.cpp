/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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

#include <platform/CommissionableDataProvider.h>
#include <platform/ConnectivityManager.h>

#include "../ConnectivityManagerImplWiFi.h"
#include "WiFiManager.h"

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI

using namespace ::chip;
using namespace ::chip::Inet;
using namespace ::chip::System;

namespace chip {
namespace DeviceLayer {

CHIP_ERROR ConnectivityManagerImplWiFi::InitWiFi()
{
    return WiFiManager::Instance().Init();
}

ConnectivityManager::WiFiStationMode ConnectivityManagerImplWiFi::_GetWiFiStationMode(void)
{
    if (mStationMode != ConnectivityManager::WiFiStationMode::kWiFiStationMode_ApplicationControlled)
    {
        mStationMode = (WiFiManager::StationStatus::DISABLED == WiFiManager::Instance().GetStationStatus())
            ? ConnectivityManager::WiFiStationMode::kWiFiStationMode_Disabled
            : ConnectivityManager::WiFiStationMode::kWiFiStationMode_Enabled;
    }
    return mStationMode;
}

CHIP_ERROR ConnectivityManagerImplWiFi::_SetWiFiStationMode(ConnectivityManager::WiFiStationMode aMode)
{
    VerifyOrReturnError(ConnectivityManager::WiFiStationMode::kWiFiStationMode_NotSupported != aMode, CHIP_ERROR_INVALID_ARGUMENT);

    mStationMode = aMode;

    return CHIP_NO_ERROR;
}

bool ConnectivityManagerImplWiFi::_IsWiFiStationEnabled(void)
{
    return (WiFiManager::StationStatus::DISABLED <= WiFiManager::Instance().GetStationStatus());
}

bool ConnectivityManagerImplWiFi::_IsWiFiStationApplicationControlled(void)
{
    return (ConnectivityManager::WiFiStationMode::kWiFiStationMode_ApplicationControlled == mStationMode);
}

bool ConnectivityManagerImplWiFi::_IsWiFiStationConnected(void)
{
    return (WiFiManager::StationStatus::CONNECTED == WiFiManager::Instance().GetStationStatus());
}

System::Clock::Timeout ConnectivityManagerImplWiFi::_GetWiFiStationReconnectInterval(void)
{
    return mWiFiStationReconnectInterval;
}

CHIP_ERROR ConnectivityManagerImplWiFi::_SetWiFiStationReconnectInterval(System::Clock::Timeout val)
{
    mWiFiStationReconnectInterval = val;
    return CHIP_NO_ERROR;
}

bool ConnectivityManagerImplWiFi::_IsWiFiStationProvisioned(void)
{
    // from Matter perspective `provisioned` means that the supplicant has been provided
    // with SSID and password (doesn't matter if valid or not)
    return (WiFiManager::StationStatus::CONNECTING <= WiFiManager::Instance().GetStationStatus());
}

void ConnectivityManagerImplWiFi::_ClearWiFiStationProvision(void)
{
    if (_IsWiFiStationProvisioned())
    {
        if (CHIP_NO_ERROR != WiFiManager::Instance().ClearStationProvisioningData())
        {
            ChipLogError(DeviceLayer, "Cannot clear WiFi station provisioning data");
        }
    }
}

bool ConnectivityManagerImplWiFi::_CanStartWiFiScan()
{
    return (WiFiManager::StationStatus::DISABLED != WiFiManager::Instance().GetStationStatus() &&
            WiFiManager::StationStatus::SCANNING != WiFiManager::Instance().GetStationStatus() &&
            WiFiManager::StationStatus::CONNECTING != WiFiManager::Instance().GetStationStatus());
}

void ConnectivityManagerImplWiFi::_OnWiFiStationProvisionChange()
{
    // do nothing
}

void ConnectivityManagerImplWiFi::_OnWiFiScanDone() {}

CHIP_ERROR ConnectivityManagerImplWiFi::_GetAndLogWiFiStatsCounters(void)
{
    // TODO: when network statistics are enabled
    return CHIP_NO_ERROR;
}

ConnectivityManager::WiFiAPMode ConnectivityManagerImplWiFi::_GetWiFiAPMode(void)
{
    /* AP mode is unsupported */
    return ConnectivityManager::WiFiAPMode::kWiFiAPMode_NotSupported;
}

CHIP_ERROR ConnectivityManagerImplWiFi::_SetWiFiAPMode(ConnectivityManager::WiFiAPMode mode)
{
    /* AP mode is unsupported */
    VerifyOrReturnError(ConnectivityManager::WiFiAPMode::kWiFiAPMode_NotSupported == mode ||
                            ConnectivityManager::WiFiAPMode::kWiFiAPMode_Disabled == mode,
                        CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
    return CHIP_NO_ERROR;
}

bool ConnectivityManagerImplWiFi::_IsWiFiAPActive(void)
{
    /* AP mode is unsupported */
    return false;
}

bool ConnectivityManagerImplWiFi::_IsWiFiAPApplicationControlled(void)
{
    /* AP mode is unsupported */
    return false;
}

void ConnectivityManagerImplWiFi::_DemandStartWiFiAP(void)
{ /* AP mode is unsupported */
}

void ConnectivityManagerImplWiFi::_StopOnDemandWiFiAP(void)
{ /* AP mode is unsupported */
}

void ConnectivityManagerImplWiFi::_MaintainOnDemandWiFiAP(void)
{ /* AP mode is unsupported */
}

System::Clock::Timeout ConnectivityManagerImplWiFi::_GetWiFiAPIdleTimeout(void)
{
    /* AP mode is unsupported */
    return System::Clock::kZero;
}

void ConnectivityManagerImplWiFi::_SetWiFiAPIdleTimeout(System::Clock::Timeout val)
{ /* AP mode is unsupported */
}

} // namespace DeviceLayer
} // namespace chip

#endif // CHIP_DEVICE_CONFIG_ENABLE_WIFI
