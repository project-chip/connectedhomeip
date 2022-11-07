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

#include <platform/internal/GenericConnectivityManagerImpl_UDP.ipp>

#if INET_CONFIG_ENABLE_TCP_ENDPOINT
#include <platform/internal/GenericConnectivityManagerImpl_TCP.ipp>
#endif

#if CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE
#include <platform/internal/GenericConnectivityManagerImpl_BLE.ipp>
#endif

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
#include <platform/internal/GenericConnectivityManagerImpl_Thread.ipp>
#endif

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI
#include <platform/internal/GenericConnectivityManagerImpl_WiFi.ipp>
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

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI
    mWiFiStationMode              = kWiFiStationMode_Disabled;
    mWiFiAPMode                   = kWiFiAPMode_Disabled;
    mWiFiAPState                  = kWiFiAPState_NotActive;
    mLastAPDemandTime             = System::Clock::kZero;
    mWiFiStationReconnectInterval = System::Clock::Milliseconds32(CHIP_DEVICE_CONFIG_WIFI_STATION_RECONNECT_INTERVAL);
    mWiFiAPIdleTimeout            = System::Clock::Milliseconds32(CHIP_DEVICE_CONFIG_WIFI_AP_IDLE_TIMEOUT);

    WiFiMgr().Init();
#endif

    return err;
}

void ConnectivityManagerImpl::_OnPlatformEvent(const ChipDeviceEvent * event) {}

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI

ConnectivityManager::WiFiStationMode ConnectivityManagerImpl::_GetWiFiStationMode(void)
{
    CHIP_ERROR err                          = CHIP_NO_ERROR;
    wifi_manager_device_state_e deviceState = WIFI_MANAGER_DEVICE_STATE_DEACTIVATED;

    ReturnErrorCodeIf(mWiFiStationMode == kWiFiStationMode_ApplicationControlled, mWiFiStationMode);

    err = WiFiMgr().GetDeviceState(&deviceState);
    VerifyOrReturnError(err == CHIP_NO_ERROR, mWiFiStationMode);

    mWiFiStationMode = (deviceState == WIFI_MANAGER_DEVICE_STATE_ACTIVATED) ? kWiFiStationMode_Enabled : kWiFiStationMode_Disabled;

    return mWiFiStationMode;
}

CHIP_ERROR ConnectivityManagerImpl::_SetWiFiStationMode(ConnectivityManager::WiFiStationMode val)
{
    CHIP_ERROR err                          = CHIP_NO_ERROR;
    wifi_manager_device_state_e deviceState = WIFI_MANAGER_DEVICE_STATE_DEACTIVATED;

    ReturnErrorCodeIf(val == kWiFiStationMode_NotSupported, CHIP_ERROR_INVALID_ARGUMENT);

    if (val != kWiFiStationMode_ApplicationControlled)
    {
        deviceState =
            (val == kWiFiStationMode_Disabled) ? WIFI_MANAGER_DEVICE_STATE_DEACTIVATED : WIFI_MANAGER_DEVICE_STATE_ACTIVATED;
        err = WiFiMgr().SetDeviceState(deviceState);
        VerifyOrReturnError(err == CHIP_NO_ERROR, err);
    }

    if (mWiFiStationMode != val)
    {
        ChipLogProgress(DeviceLayer, "WiFi station mode change: %s -> %s", WiFiStationModeToStr(mWiFiStationMode),
                        WiFiStationModeToStr(val));

        mWiFiStationMode = val;
    }

    return err;
}

System::Clock::Timeout ConnectivityManagerImpl::_GetWiFiStationReconnectInterval(void)
{
    return mWiFiStationReconnectInterval;
}

CHIP_ERROR ConnectivityManagerImpl::_SetWiFiStationReconnectInterval(System::Clock::Timeout val)
{
    mWiFiStationReconnectInterval = val;

    return CHIP_NO_ERROR;
}

bool ConnectivityManagerImpl::_IsWiFiStationEnabled(void)
{
    bool isWiFiStationEnabled = false;

    WiFiMgr().IsActivated(&isWiFiStationEnabled);

    return isWiFiStationEnabled;
}

bool ConnectivityManagerImpl::_IsWiFiStationConnected(void)
{
    CHIP_ERROR err                                  = CHIP_NO_ERROR;
    wifi_manager_connection_state_e connectionState = WIFI_MANAGER_CONNECTION_STATE_DISCONNECTED;
    bool isWiFiStationConnected                     = false;

    err = WiFiMgr().GetConnectionState(&connectionState);
    VerifyOrReturnError(err == CHIP_NO_ERROR, isWiFiStationConnected);

    if (connectionState == WIFI_MANAGER_CONNECTION_STATE_CONNECTED)
        isWiFiStationConnected = true;

    return isWiFiStationConnected;
}

bool ConnectivityManagerImpl::_IsWiFiStationProvisioned(void)
{
    CHIP_ERROR err                                  = CHIP_NO_ERROR;
    wifi_manager_connection_state_e connectionState = WIFI_MANAGER_CONNECTION_STATE_DISCONNECTED;
    bool isWiFiStationProvisioned                   = false;

    err = WiFiMgr().GetConnectionState(&connectionState);
    VerifyOrReturnError(err == CHIP_NO_ERROR, isWiFiStationProvisioned);

    if (connectionState >= WIFI_MANAGER_CONNECTION_STATE_ASSOCIATION)
        isWiFiStationProvisioned = true;

    return isWiFiStationProvisioned;
}

void ConnectivityManagerImpl::_ClearWiFiStationProvision(void)
{
    WiFiMgr().RemoveAllConfigs();
}

bool ConnectivityManagerImpl::_CanStartWiFiScan(void)
{
    return false;
}

ConnectivityManager::WiFiAPMode ConnectivityManagerImpl::_GetWiFiAPMode()
{
    return mWiFiAPMode;
}

CHIP_ERROR ConnectivityManagerImpl::_SetWiFiAPMode(WiFiAPMode val)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

bool ConnectivityManagerImpl::_IsWiFiAPActive()
{
    return mWiFiAPState == kWiFiAPState_Active;
}

void ConnectivityManagerImpl::_DemandStartWiFiAP(void) {}

void ConnectivityManagerImpl::_StopOnDemandWiFiAP(void) {}

void ConnectivityManagerImpl::_MaintainOnDemandWiFiAP(void) {}

void ConnectivityManagerImpl::_SetWiFiAPIdleTimeout(System::Clock::Timeout val) {}

void ConnectivityManagerImpl::StartWiFiManagement(void)
{
    SystemLayer().ScheduleWork(ActivateWiFiManager, nullptr);
}

void ConnectivityManagerImpl::StopWiFiManagement(void)
{
    SystemLayer().ScheduleWork(DeactivateWiFiManager, nullptr);
}

void ConnectivityManagerImpl::ActivateWiFiManager(::chip::System::Layer * aLayer, void * aAppState)
{
    WiFiMgr().Activate();
}

void ConnectivityManagerImpl::DeactivateWiFiManager(::chip::System::Layer * aLayer, void * aAppState)
{
    WiFiMgr().Deactivate();
}
#endif // CHIP_DEVICE_CONFIG_ENABLE_WIFI

} // namespace DeviceLayer
} // namespace chip
