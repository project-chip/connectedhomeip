/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2018 Nest Labs, Inc.
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

#include <platform/ConnectivityManager.h>
#include <platform/internal/CHIPDeviceLayerInternal.h>
#include <platform/mbed/ConnectivityManagerImpl.h>
#include <platform/mbed/NetworkCommissioningDriver.h>

#include <platform/internal/GenericConnectivityManagerImpl_WiFi.ipp>

#include <app/clusters/network-commissioning/network-commissioning.h>

#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>

using namespace ::chip;
using namespace ::chip::Inet;
using namespace ::chip::System;
using namespace ::chip::DeviceLayer;
using namespace ::chip::DeviceLayer::Internal;

namespace chip {
namespace DeviceLayer {

#if CHIP_DEVICE_ENABLE_DATA_MODEL
namespace {
app::Clusters::NetworkCommissioning::Instance
    sWiFiNetworkCommissioningInstance(0 /* Endpoint Id */, &(NetworkCommissioning::WiFiDriverImpl::GetInstance()));
} // namespace
#endif // CHIP_DEVICE_ENABLE_DATA_MODEL

CHIP_ERROR ConnectivityManagerImpl::InitWiFi()
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    NetworkCommissioning::NetworkIterator * networks;

    mWiFiStationState = kWiFiStationState_NotConnected;

    mWiFiStationReconnectInterval = System::Clock::Milliseconds32(CHIP_DEVICE_CONFIG_WIFI_STATION_RECONNECT_INTERVAL);
#if CHIP_DEVICE_ENABLE_DATA_MODEL
    err = sWiFiNetworkCommissioningInstance.Init();
#else
    err = NetworkCommissioning::WiFiDriverImpl::GetInstance().Init(nullptr);
#endif
    VerifyOrExit(err == CHIP_NO_ERROR, ChipLogError(DeviceLayer, "WiFi driver init failed: %s", chip::ErrorStr(err)));

    mWiFiStationMode = kWiFiStationMode_Enabled;
    networks         = NetworkCommissioning::WiFiDriverImpl::GetInstance().GetNetworks();
    if (networks != nullptr && networks->Count() > 0)
    {
        mIsProvisioned = true;
    }
    else
    {
        mIsProvisioned = false;
    }
    if (networks != nullptr)
    {
        networks->Release();
    }
exit:
    return err;
}

void ConnectivityManagerImpl::OnWiFiPlatformEvent(const ChipDeviceEvent * event)
{
    if (event->Type == DeviceEventType::kWiFiConnectivityChange)
    {
        WiFiStationState wifiStaState;
        switch (event->WiFiConnectivityChange.Result)
        {
        case kConnectivity_NoChange:
            wifiStaState = kWiFiStationState_Connecting;
            break;
        case kConnectivity_Established:
            ChipLogProgress(DeviceLayer, "WiFi station connected");
            wifiStaState = kWiFiStationState_Connected;
            break;
        case kConnectivity_Lost:
            ChipLogProgress(DeviceLayer, "WiFi station disconnected");
            wifiStaState = kWiFiStationState_NotConnected;
            NetworkCommissioning::WiFiDriverImpl::GetInstance().SetLastDisconnectReason(event);
            break;
        default:
            ChipLogError(DeviceLayer, "Unknown WiFi connectivity state");
            return;
        }
        ChangeWiFiStationState(wifiStaState);
    }
}

void ConnectivityManagerImpl::ChangeWiFiStationState(WiFiStationState newState)
{
    if (mWiFiStationState != newState)
    {
        ChipLogProgress(DeviceLayer, "WiFi station state change: %s -> %s", WiFiStationStateToStr(mWiFiStationState),
                        WiFiStationStateToStr(newState));
        mWiFiStationState = newState;
        SystemLayer().ScheduleLambda([]() { NetworkCommissioning::WiFiDriverImpl::GetInstance().OnNetworkStatusChange(); });
    }
}

CHIP_ERROR ConnectivityManagerImpl::_SetWiFiStationMode(WiFiStationMode val)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    VerifyOrExit(val != kWiFiStationMode_NotSupported, err = CHIP_ERROR_INVALID_ARGUMENT);

    if (mWiFiStationMode != val)
    {
        ChipLogProgress(DeviceLayer, "WiFi station mode change: %s -> %s", WiFiStationModeToStr(mWiFiStationMode),
                        WiFiStationModeToStr(val));

        mWiFiStationMode = val;

        if (mWiFiStationMode != kWiFiStationMode_ApplicationControlled)
        {
            if (mWiFiStationMode == kWiFiStationMode_Enabled)
            {
                NetworkCommissioning::WiFiDriverImpl::GetInstance().Init(nullptr);
            }
            else if (mWiFiStationMode == kWiFiStationMode_Disabled)
            {
                NetworkCommissioning::WiFiDriverImpl::GetInstance().Shutdown();
            }
        }
    }

exit:
    return err;
}

CHIP_ERROR ConnectivityManagerImpl::_SetWiFiStationReconnectInterval(System::Clock::Timeout val)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    if (mWiFiStationReconnectInterval != val)
    {
        ChipLogProgress(DeviceLayer, "WiFi station reconnect interval change: %lu ms -> %lu ms",
                        System::Clock::Milliseconds32(mWiFiStationReconnectInterval).count(),
                        System::Clock::Milliseconds32(val).count());
    }

    mWiFiStationReconnectInterval = val;

    return err;
}

void ConnectivityManagerImpl::_ClearWiFiStationProvision(void)
{
    if (mWiFiStationMode != kWiFiStationMode_ApplicationControlled)
    {
        auto networks = NetworkCommissioning::WiFiDriverImpl::GetInstance().GetNetworks();
        NetworkCommissioning::Network network;
        for (; networks != nullptr && networks->Next(network);)
        {
            MutableCharSpan emptyBufferForDebugText;
            uint8_t outNetworkIndex;
            if (network.connected)
            {
                NetworkCommissioning::WiFiDriverImpl::GetInstance().DisconnectNetwork(
                    ByteSpan(network.networkID, network.networkIDLen));
            }
            NetworkCommissioning::WiFiDriverImpl::GetInstance().RemoveNetwork(ByteSpan(network.networkID, network.networkIDLen),
                                                                              emptyBufferForDebugText, outNetworkIndex);
        }
        if (networks != nullptr)
        {
            networks->Release();
        }
    }
}

} // namespace DeviceLayer
} // namespace chip
