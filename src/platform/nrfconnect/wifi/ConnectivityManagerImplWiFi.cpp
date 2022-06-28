/*
 *
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
/* this file behaves like a config.h, comes first */
#include <platform/internal/CHIPDeviceLayerInternal.h>

#include <platform/CommissionableDataProvider.h>
#include <platform/ConnectivityManager.h>

#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/DiagnosticDataProvider.h>
#include <platform/internal/BLEManager.h>

#include "ConnectivityManagerImplWiFi.h"
#include <platform/Zephyr/WiFiManager.h>

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI

using namespace ::chip;
using namespace ::chip::Inet;
using namespace ::chip::System;
using namespace ::chip::TLV;

#define WIFI_BAND_2_4GHZ 2400
#define WIFI_BAND_5_0GHZ 5000

static uint16_t Map2400MHz(const uint8_t inChannel)
{
    uint16_t frequency = 0;

    if (inChannel >= 1 && inChannel <= 13)
    {
        // Cast is OK because we definitely fit in 16 bits.
        frequency = static_cast<uint16_t>(2412 + ((inChannel - 1) * 5));
    }
    else if (inChannel == 14)
    {
        frequency = 2484;
    }

    return frequency;
}

static uint16_t Map5000MHz(const uint8_t inChannel)
{
    uint16_t frequency = 0;

    return frequency;
}

static uint16_t MapFrequency(const uint16_t inBand, const uint8_t inChannel)
{
    uint16_t frequency = 0;

    if (inBand == WIFI_BAND_2_4GHZ)
    {
        frequency = Map2400MHz(inChannel);
    }
    else if (inBand == WIFI_BAND_5_0GHZ)
    {
        frequency = Map5000MHz(inChannel);
    }

    return frequency;
}

namespace chip {
namespace DeviceLayer {

ConnectivityManager::WiFiStationMode ConnectivityManagerImplWiFi::_GetWiFiStationMode(void)
{
    return ConnectivityManager::WiFiStationMode::kWiFiStationMode_Disabled;
}

CHIP_ERROR ConnectivityManagerImplWiFi::_SetWiFiStationMode(ConnectivityManager::WiFiStationMode val)
{
    return CHIP_NO_ERROR;
}

bool ConnectivityManagerImplWiFi::_IsWiFiStationEnabled(void)
{
    return false;
}

bool ConnectivityManagerImplWiFi::_IsWiFiStationApplicationControlled(void)
{
    return false;
}

bool ConnectivityManagerImplWiFi::_IsWiFiStationConnected(void)
{
    return false;
}

System::Clock::Timeout ConnectivityManagerImplWiFi::_GetWiFiStationReconnectInterval(void)
{
    return System::Clock::kZero;
}

CHIP_ERROR ConnectivityManagerImplWiFi::_SetWiFiStationReconnectInterval(System::Clock::Timeout val)
{
    return CHIP_NO_ERROR;
}

bool ConnectivityManagerImplWiFi::_IsWiFiStationProvisioned(void)
{
    return false;
}

void ConnectivityManagerImplWiFi::_ClearWiFiStationProvision(void) {}

ConnectivityManager::WiFiAPMode ConnectivityManagerImplWiFi::_GetWiFiAPMode(void)
{
    return ConnectivityManager::WiFiAPMode::kWiFiAPMode_Disabled;
}

CHIP_ERROR ConnectivityManagerImplWiFi::_SetWiFiAPMode(ConnectivityManager::WiFiAPMode val)
{
    return CHIP_NO_ERROR;
}

bool ConnectivityManagerImplWiFi::_IsWiFiAPActive(void)
{
    return false;
}

bool ConnectivityManagerImplWiFi::_IsWiFiAPApplicationControlled(void)
{
    return false;
}

void ConnectivityManagerImplWiFi::_DemandStartWiFiAP(void) {}

void ConnectivityManagerImplWiFi::_StopOnDemandWiFiAP(void) {}

void ConnectivityManagerImplWiFi::_MaintainOnDemandWiFiAP(void) {}

System::Clock::Timeout ConnectivityManagerImplWiFi::_GetWiFiAPIdleTimeout(void)
{
    return System::Clock::kZero;
}

void ConnectivityManagerImplWiFi::_SetWiFiAPIdleTimeout(System::Clock::Timeout val) {}

CHIP_ERROR ConnectivityManagerImplWiFi::_GetAndLogWiFiStatsCounters(void)
{
    return CHIP_NO_ERROR;
}

bool ConnectivityManagerImplWiFi::_CanStartWiFiScan()
{
    return false;
}

void ConnectivityManagerImplWiFi::_OnWiFiScanDone() {}

void ConnectivityManagerImplWiFi::_OnWiFiStationProvisionChange() {}

CHIP_ERROR ConnectivityManagerImplWiFi::InitWiFi()
{
    return WiFiManager::Instance().Init();
}

void ConnectivityManagerImplWiFi::OnWiFiPlatformEvent(const ChipDeviceEvent * event)
{
    return;
}

// helpers
const char * ConnectivityManagerImplWiFi::_WiFiStationModeToStr(ConnectivityManager::WiFiStationMode mode)
{
    return nullptr;
}

const char * ConnectivityManagerImplWiFi::_WiFiAPModeToStr(ConnectivityManager::WiFiAPMode mode)
{
    return nullptr;
}

const char * ConnectivityManagerImplWiFi::_WiFiStationStateToStr(ConnectivityManager::WiFiStationState state)
{
    return nullptr;
}

const char * ConnectivityManagerImplWiFi::_WiFiAPStateToStr(ConnectivityManager::WiFiAPState state)
{
    return nullptr;
}

} // namespace DeviceLayer
} // namespace chip

#endif // CHIP_DEVICE_CONFIG_ENABLE_WIFI
