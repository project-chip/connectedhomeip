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

/**
 * Note: Use public include for ConnectivityManager which includes our local
 *       platform/<PLATFORM>/ConnectivityManagerImpl.h after defining interface
 *       class. */
#include <platform/ConnectivityManager.h>

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI
#include <wifi-manager.h>
#endif

#include <inet/InetConfig.h>
#include <lib/core/CHIPError.h>
#include <lib/support/CodeUtils.h>
#include <platform/CHIPDeviceConfig.h>
#include <platform/CHIPDeviceEvent.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/Tizen/ConnectivityUtils.h>

#include <system/SystemClock.h>
#include <system/SystemLayer.h>

#include "platform/internal/GenericConnectivityManagerImpl_UDP.ipp"
#if INET_CONFIG_ENABLE_TCP_ENDPOINT
#include "platform/internal/GenericConnectivityManagerImpl_TCP.ipp"
#endif
#if CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE
#include "platform/internal/GenericConnectivityManagerImpl_BLE.ipp"
#endif
#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
#include "platform/internal/GenericConnectivityManagerImpl_Thread.ipp"
#endif
#if CHIP_DEVICE_CONFIG_ENABLE_WIFI
#include "platform/internal/GenericConnectivityManagerImpl_WiFi.ipp"
#endif

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI
#include "WiFiManager.h"
#endif

using namespace ::chip::DeviceLayer::Internal;
using namespace ::chip::app::Clusters::WiFiNetworkDiagnostics;

namespace chip {
namespace DeviceLayer {

ConnectivityManagerImpl ConnectivityManagerImpl::sInstance;

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI
char ConnectivityManagerImpl::sWiFiIfName[];
#endif

CHIP_ERROR ConnectivityManagerImpl::_Init()
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    if (ConnectivityUtils::GetEthInterfaceName(mEthIfName, Inet::InterfaceId::kMaxIfNameLength) == CHIP_NO_ERROR)
    {
        ChipLogProgress(DeviceLayer, "Got Ethernet interface: %s", mEthIfName);
    }
    else
    {
        ChipLogError(DeviceLayer, "Failed to get Ethernet interface");
        mEthIfName[0] = '\0';
    }

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI
    mWiFiStationMode              = kWiFiStationMode_Disabled;
    mWiFiAPMode                   = kWiFiAPMode_Disabled;
    mWiFiAPState                  = kWiFiAPState_NotActive;
    mLastAPDemandTime             = System::Clock::kZero;
    mWiFiStationReconnectInterval = System::Clock::Milliseconds32(CHIP_DEVICE_CONFIG_WIFI_STATION_RECONNECT_INTERVAL);
    mWiFiAPIdleTimeout            = System::Clock::Milliseconds32(CHIP_DEVICE_CONFIG_WIFI_AP_IDLE_TIMEOUT);

    Internal::WiFiMgr().Init();

    if (ConnectivityUtils::GetWiFiInterfaceName(sWiFiIfName, Inet::InterfaceId::kMaxIfNameLength) == CHIP_NO_ERROR)
    {
        ChipLogProgress(DeviceLayer, "Got WiFi interface: %s", sWiFiIfName);
    }
    else
    {
        ChipLogError(DeviceLayer, "Failed to get WiFi interface");
        sWiFiIfName[0] = '\0';
    }
#endif

    return err;
}

void ConnectivityManagerImpl::_OnPlatformEvent(const ChipDeviceEvent * event) {}

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI

ConnectivityManager::WiFiStationMode ConnectivityManagerImpl::_GetWiFiStationMode()
{
    CHIP_ERROR err                          = CHIP_NO_ERROR;
    wifi_manager_device_state_e deviceState = WIFI_MANAGER_DEVICE_STATE_DEACTIVATED;

    VerifyOrReturnError(mWiFiStationMode != kWiFiStationMode_ApplicationControlled, mWiFiStationMode);

    err = Internal::WiFiMgr().GetDeviceState(&deviceState);
    VerifyOrReturnError(err == CHIP_NO_ERROR, mWiFiStationMode);

    mWiFiStationMode = (deviceState == WIFI_MANAGER_DEVICE_STATE_ACTIVATED) ? kWiFiStationMode_Enabled : kWiFiStationMode_Disabled;

    return mWiFiStationMode;
}

CHIP_ERROR ConnectivityManagerImpl::_SetWiFiStationMode(ConnectivityManager::WiFiStationMode val)
{
    CHIP_ERROR err                          = CHIP_NO_ERROR;
    wifi_manager_device_state_e deviceState = WIFI_MANAGER_DEVICE_STATE_DEACTIVATED;

    VerifyOrReturnError(val != kWiFiStationMode_NotSupported, CHIP_ERROR_INVALID_ARGUMENT);

    if (val != kWiFiStationMode_ApplicationControlled)
    {
        deviceState =
            (val == kWiFiStationMode_Disabled) ? WIFI_MANAGER_DEVICE_STATE_DEACTIVATED : WIFI_MANAGER_DEVICE_STATE_ACTIVATED;
        err = Internal::WiFiMgr().SetDeviceState(deviceState);
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

System::Clock::Timeout ConnectivityManagerImpl::_GetWiFiStationReconnectInterval()
{
    return mWiFiStationReconnectInterval;
}

CHIP_ERROR ConnectivityManagerImpl::_SetWiFiStationReconnectInterval(System::Clock::Timeout val)
{
    mWiFiStationReconnectInterval = val;

    return CHIP_NO_ERROR;
}

bool ConnectivityManagerImpl::_IsWiFiStationEnabled()
{
    bool isWiFiStationEnabled = false;

    Internal::WiFiMgr().IsActivated(&isWiFiStationEnabled);

    return isWiFiStationEnabled;
}

bool ConnectivityManagerImpl::_IsWiFiStationConnected()
{
    CHIP_ERROR err                                  = CHIP_NO_ERROR;
    wifi_manager_connection_state_e connectionState = WIFI_MANAGER_CONNECTION_STATE_DISCONNECTED;
    bool isWiFiStationConnected                     = false;

    err = Internal::WiFiMgr().GetConnectionState(&connectionState);
    VerifyOrReturnError(err == CHIP_NO_ERROR, isWiFiStationConnected);

    if (connectionState == WIFI_MANAGER_CONNECTION_STATE_CONNECTED)
        isWiFiStationConnected = true;

    return isWiFiStationConnected;
}

bool ConnectivityManagerImpl::_IsWiFiStationProvisioned()
{
    CHIP_ERROR err                                  = CHIP_NO_ERROR;
    wifi_manager_connection_state_e connectionState = WIFI_MANAGER_CONNECTION_STATE_DISCONNECTED;
    bool isWiFiStationProvisioned                   = false;

    err = Internal::WiFiMgr().GetConnectionState(&connectionState);
    VerifyOrReturnError(err == CHIP_NO_ERROR, isWiFiStationProvisioned);

    if (connectionState >= WIFI_MANAGER_CONNECTION_STATE_ASSOCIATION)
        isWiFiStationProvisioned = true;

    return isWiFiStationProvisioned;
}

void ConnectivityManagerImpl::_ClearWiFiStationProvision()
{
    Internal::WiFiMgr().RemoveAllConfigs();
}

bool ConnectivityManagerImpl::_CanStartWiFiScan()
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

void ConnectivityManagerImpl::_DemandStartWiFiAP() {}

void ConnectivityManagerImpl::_StopOnDemandWiFiAP() {}

void ConnectivityManagerImpl::_MaintainOnDemandWiFiAP() {}

void ConnectivityManagerImpl::_SetWiFiAPIdleTimeout(System::Clock::Timeout val) {}

void ConnectivityManagerImpl::StartWiFiManagement()
{
    Internal::WiFiMgr().Activate();
}

void ConnectivityManagerImpl::StopWiFiManagement()
{
    Internal::WiFiMgr().Deactivate();
}

bool ConnectivityManagerImpl::IsWiFiManagementStarted()
{
    bool isActivated = false;
    Internal::WiFiMgr().IsActivated(&isActivated);
    return isActivated;
}

CHIP_ERROR ConnectivityManagerImpl::GetWiFiBssId(MutableByteSpan & value)
{
    return Internal::WiFiMgr().GetBssId(value);
}

CHIP_ERROR ConnectivityManagerImpl::GetWiFiSecurityType(SecurityTypeEnum & securityType)
{
    wifi_manager_security_type_e secType;
    CHIP_ERROR err = Internal::WiFiMgr().GetSecurityType(&secType);
    ReturnErrorOnFailure(err);

    switch (secType)
    {
    case WIFI_MANAGER_SECURITY_TYPE_NONE:
        securityType = SecurityTypeEnum::kNone;
        break;
    case WIFI_MANAGER_SECURITY_TYPE_WEP:
        securityType = SecurityTypeEnum::kWep;
        break;
    case WIFI_MANAGER_SECURITY_TYPE_WPA_PSK:
        securityType = SecurityTypeEnum::kWpa;
        break;
    case WIFI_MANAGER_SECURITY_TYPE_WPA2_PSK:
        securityType = SecurityTypeEnum::kWpa2;
        break;
    case WIFI_MANAGER_SECURITY_TYPE_EAP:
    case WIFI_MANAGER_SECURITY_TYPE_WPA_FT_PSK:
    case WIFI_MANAGER_SECURITY_TYPE_SAE:
    case WIFI_MANAGER_SECURITY_TYPE_OWE:
    case WIFI_MANAGER_SECURITY_TYPE_DPP:
    default:
        securityType = SecurityTypeEnum::kUnspecified;
        break;
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR ConnectivityManagerImpl::GetWiFiVersion(WiFiVersionEnum & wiFiVersion)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

#endif // CHIP_DEVICE_CONFIG_ENABLE_WIFI

} // namespace DeviceLayer
} // namespace chip
