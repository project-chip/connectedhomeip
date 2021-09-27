/*
 *
 *    Copyright (c) 2020-2021 Project CHIP Authors
 *    Copyright (c) 2019 Nest Labs, Inc.
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

#include <platform/internal/CHIPDeviceLayerInternal.h>

#include <platform/ConnectivityManager.h>
#include <platform/internal/BLEManager.h>

#include <cstdlib>

#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>

#if CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE
#include <platform/internal/GenericConnectivityManagerImpl_BLE.cpp>
#endif

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
#include <platform/internal/GenericConnectivityManagerImpl_Thread.cpp>
#endif

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI
#include <platform/internal/GenericConnectivityManagerImpl_WiFi.cpp>
#endif

using namespace ::chip;
using namespace ::chip::TLV;
using namespace ::chip::DeviceLayer::Internal;

namespace chip {
namespace DeviceLayer {

ConnectivityManagerImpl ConnectivityManagerImpl::sInstance;

CHIP_ERROR ConnectivityManagerImpl::_Init(void)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    return err;
}

void ConnectivityManagerImpl::_OnPlatformEvent(const ChipDeviceEvent * event) {}

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI

ConnectivityManager::WiFiStationMode ConnectivityManagerImpl::_GetWiFiStationMode(void)
{
    return ConnectivityManager::kWiFiStationMode_NotSupported;
}

CHIP_ERROR ConnectivityManagerImpl::_SetWiFiStationMode(ConnectivityManager::WiFiStationMode val)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

uint32_t ConnectivityManagerImpl::_GetWiFiStationReconnectIntervalMS(void)
{
    return 0;
}

CHIP_ERROR ConnectivityManagerImpl::_SetWiFiStationReconnectIntervalMS(uint32_t val)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

bool ConnectivityManagerImpl::_IsWiFiStationEnabled(void)
{
    return false;
}

bool ConnectivityManagerImpl::_IsWiFiStationConnected(void)
{
    return false;
}

bool ConnectivityManagerImpl::_IsWiFiStationApplicationControlled(void)
{
    return false;
}

bool ConnectivityManagerImpl::_IsWiFiStationProvisioned(void)
{
    return false;
}

void ConnectivityManagerImpl::_ClearWiFiStationProvision(void) {}

bool ConnectivityManagerImpl::_CanStartWiFiScan(void)
{
    return false;
}

CHIP_ERROR ConnectivityManagerImpl::_SetWiFiAPMode(WiFiAPMode val)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

void ConnectivityManagerImpl::_DemandStartWiFiAP(void) {}

void ConnectivityManagerImpl::_StopOnDemandWiFiAP(void) {}

void ConnectivityManagerImpl::_MaintainOnDemandWiFiAP(void) {}

void ConnectivityManagerImpl::_SetWiFiAPIdleTimeoutMS(uint32_t val) {}
#endif // CHIP_DEVICE_CONFIG_ENABLE_WIFI

CHIP_ERROR ConnectivityManagerImpl::ProvisionWiFiNetwork(const char * ssid, const char * key)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

} // namespace DeviceLayer
} // namespace chip
