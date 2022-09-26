/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2019 Nest Labs, Inc.
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

#include <NetworkCommissioningDriver.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/ConnectivityManager.h>
#include <platform/internal/BLEManager.h>

#include <hal_wifi.h>
#include <wifi_mgmr_ext.h>

#include <platform/internal/GenericConnectivityManagerImpl_UDP.ipp>

#if INET_CONFIG_ENABLE_TCP_ENDPOINT
#include <platform/internal/GenericConnectivityManagerImpl_TCP.ipp>
#endif

#include <platform/internal/GenericConnectivityManagerImpl_WiFi.ipp>

#if CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE
#include <platform/internal/GenericConnectivityManagerImpl_BLE.ipp>
#endif

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
#include <platform/internal/GenericConnectivityManagerImpl_Thread.ipp>
#endif

#include <wifi_mgmr_ext.h>

#include <FreeRTOS.h>
#include <task.h>

#include <lwip/netifapi.h>
#include <wifi_mgmr_ext.h>

#include <FreeRTOS.h>
#include <stdio.h>
#include <string.h>
#include <task.h>

#define zero(S) memset(&S, 0, sizeof(S))

using namespace ::chip;
using namespace ::chip::Dnssd;
using namespace ::chip::Inet;
using namespace ::chip::System;
using namespace ::chip::TLV;
using namespace ::chip::DeviceLayer::Internal;

namespace chip {
namespace DeviceLayer {

ConnectivityManagerImpl ConnectivityManagerImpl::sInstance;
static ConnectivityManager::WiFiStationState ConnectivityManagerImpl::mWiFiStationState =
    ConnectivityManager::kWiFiStationState_NotConnected;

void ConnectivityManagerImpl::WifiStationStateChange(void)
{
    ChipDeviceEvent event;

    if (ConnectivityManagerImpl::mWiFiStationState == ConnectivityManager::kWiFiStationState_Connected)
    {
        event.Type                          = DeviceEventType::kWiFiConnectivityChange;
        event.WiFiConnectivityChange.Result = kConnectivity_Established;
        PlatformMgr().PostEventOrDie(&event);
    }
}

void ConnectivityManagerImpl::DriveStationState()
{

exit:
    return;
}

void ConnectivityManagerImpl::DriveStationState(::chip::System::Layer * aLayer, void * aAppState)
{
    sInstance.DriveStationState();
}

CHIP_ERROR ConnectivityManagerImpl::_SetWiFiStationMode(WiFiStationMode val)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    DeviceLayer::SystemLayer().ScheduleWork(DriveStationState, NULL);
exit:
    return err;
}

CHIP_ERROR ConnectivityManagerImpl::_Init()
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    // Initialize the generic base classes that require it.
#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
    GenericConnectivityManagerImpl_Thread<ConnectivityManagerImpl>::_Init();
#endif

    SuccessOrExit(err);

    err = SetWiFiStationMode(kWiFiStationMode_Enabled);
    NetworkCommissioning::BLWiFiDriver::GetInstance().ReConnectWiFiNetwork();

    SuccessOrExit(err);

exit:
    return err;
}

void ConnectivityManagerImpl::_OnPlatformEvent(const ChipDeviceEvent * event)
{
    // Forward the event to the generic base classes as needed.
#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
    GenericConnectivityManagerImpl_Thread<ConnectivityManagerImpl>::_OnPlatformEvent(event);
#endif
}

bool ConnectivityManagerImpl::_IsWiFiStationEnabled(void)
{
    return GetWiFiStationMode() == kWiFiStationMode_Enabled;
}

void ConnectivityManagerImpl::OnStationConnected()
{
    NetworkCommissioning::BLWiFiDriver::GetInstance().OnConnectWiFiNetwork();
    // TODO Invoke WARM to perform actions that occur when the WiFi station interface comes up.

#if 0
    // Alert other components of the new state.
    ChipDeviceEvent event;
    event.Type                          = DeviceEventType::kWiFiConnectivityChange;
    event.WiFiConnectivityChange.Result = kConnectivity_Established;
    PlatformMgr().PostEventOrDie(&event);

    UpdateInternetConnectivityState();
#endif
}

void ConnectivityManagerImpl::ChangeWiFiStationState(WiFiStationState newState)
{
    if (mWiFiStationState != newState)
    {
        ChipLogProgress(DeviceLayer, "WiFi station state change: %s -> %s", WiFiStationStateToStr(mWiFiStationState),
                        WiFiStationStateToStr(newState));
        mWiFiStationState = newState;
        SystemLayer().ScheduleLambda([]() { NetworkCommissioning::BLWiFiDriver::GetInstance().OnNetworkStatusChange(); });
    }
}

void ConnectivityManagerImpl::OnIPv6AddressAvailable()
{
    ChipLogProgress(DeviceLayer, "IPv6 addr available.");

    ChipDeviceEvent event;
    event.Type                           = DeviceEventType::kInterfaceIpAddressChanged;
    event.InterfaceIpAddressChanged.Type = InterfaceIpChangeType::kIpV6_Assigned;
    PlatformMgr().PostEventOrDie(&event);
}

} // namespace DeviceLayer
} // namespace chip
