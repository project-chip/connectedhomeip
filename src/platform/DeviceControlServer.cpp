/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
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
    ChipDeviceEvent event;

    event.Type                              = DeviceEventType::kCommissioningComplete;
    event.CommissioningComplete.nodeId      = peerNodeId;
    event.CommissioningComplete.fabricIndex = accessingFabricIndex;

    return PlatformMgr().PostEvent(&event);
}

CHIP_ERROR DeviceControlServer::SetRegulatoryConfig(uint8_t location, const CharSpan & countryCode)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    err = ConfigurationMgr().StoreRegulatoryLocation(location);
    SuccessOrExit(err);

    err = ConfigurationMgr().StoreCountryCode(countryCode.data(), countryCode.size());
    SuccessOrExit(err);

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "SetRegulatoryConfig failed with error: %s", ErrorStr(err));
    }

    return err;
}

CHIP_ERROR DeviceControlServer::PostConnectedToOperationalNetworkEvent(ByteSpan networkID)
{
    ChipDeviceEvent event;
    event.Type = DeviceEventType::kOperationalNetworkEnabled;
    // TODO(cecille): This should be some way to specify thread or wifi.
    event.OperationalNetwork.network = 0;
    return PlatformMgr().PostEvent(&event);
}

} // namespace DeviceLayer
} // namespace chip
