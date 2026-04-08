/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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
 *    @file
 *          Provides the implementation of the DeviceControlServer object.
 */

#include <platform/DeviceControlServer.h>

#include <platform/ConfigurationManager.h>

namespace chip {
namespace DeviceLayer {

DeviceControlServer DeviceControlServer::sInstance;

DeviceControlServer & DeviceControlServer::DeviceControlSvr()
{
    return sInstance;
}

CHIP_ERROR DeviceControlServer::PostCommissioningCompleteEvent(NodeId peerNodeId, FabricIndex accessingFabricIndex)
{
    ChipDeviceEvent event{

        .Type                  = DeviceEventType::kCommissioningComplete,
        .CommissioningComplete = { .nodeId = peerNodeId, .fabricIndex = accessingFabricIndex }
    };

    return PlatformMgr().PostEvent(&event);
}

CHIP_ERROR DeviceControlServer::SetRegulatoryConfig(uint8_t location, const CharSpan & countryCode)
{
    CHIP_ERROR err;

    err = ConfigurationMgr().StoreRegulatoryLocation(location);
    SuccessOrExit(err);

    err = ConfigurationMgr().StoreCountryCode(countryCode.data(), countryCode.size());
    SuccessOrExit(err);

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "SetRegulatoryConfig failed with error: %" CHIP_ERROR_FORMAT, err.Format());
    }

    return err;
}

CHIP_ERROR DeviceControlServer::PostConnectedToOperationalNetworkEvent(ByteSpan networkID)
{
#if CHIP_DEVICE_CONFIG_ENABLE_WIFIPAF
    // Under WiFiPAF the device must deliver ConnectNetworkResponse via NAN
    // (PAF) before the mDNS burst that kOperationalNetworkEnabled triggers.
    // Firing mDNS immediately causes a large Avahi burst that occupies the
    // shared radio and prevents the NAN Follow-up from being received by the
    // proxy.  Defer by 5 s to give the PAF exchange time to complete first.
    return DeviceLayer::SystemLayer().StartTimer(
        System::Clock::Seconds32(5),
        [](System::Layer *, void *) {
            ChipDeviceEvent ev{ .Type               = DeviceEventType::kOperationalNetworkEnabled,
                                .OperationalNetwork = { .network = 0 } };
            CHIP_ERROR err = PlatformMgr().PostEvent(&ev);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(DeviceLayer, "Deferred kOperationalNetworkEnabled: %" CHIP_ERROR_FORMAT, err.Format());
            }
        },
        nullptr);
#else
    ChipDeviceEvent event{ .Type = DeviceEventType::kOperationalNetworkEnabled,
                           // TODO(cecille): This should be some way to specify thread or wifi.
                           .OperationalNetwork = { .network = 0 } };
    return PlatformMgr().PostEvent(&event);
#endif
}

CHIP_ERROR DeviceControlServer::PostCloseAllBLEConnectionsToOperationalNetworkEvent()
{
    ChipDeviceEvent event{ .Type = DeviceEventType::kCloseAllBleConnections };
    return PlatformMgr().PostEvent(&event);
}

CHIP_ERROR DeviceControlServer::PostWiFiDeviceAvailableNetworkEvent()
{
    ChipDeviceEvent event{ .Type = DeviceEventType::kWiFiDeviceAvailable };
    return PlatformMgr().PostEvent(&event);
}

CHIP_ERROR DeviceControlServer::PostOperationalNetworkStartedEvent()
{
    ChipDeviceEvent event{ .Type = DeviceEventType::kOperationalNetworkStarted };
    return PlatformMgr().PostEvent(&event);
}

} // namespace DeviceLayer
} // namespace chip
