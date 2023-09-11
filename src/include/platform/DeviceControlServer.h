/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 *    @file
 *          Defines the Device Layer DeviceControlServer object.
 */

#pragma once

#include <lib/support/Span.h>
#include <platform/internal/CHIPDeviceLayerInternal.h>

namespace chip {
namespace DeviceLayer {

class DeviceControlServer final
{
public:
    // ===== Members for internal use by other Device Layer components.

    CHIP_ERROR PostCommissioningCompleteEvent(NodeId peerNodeId, FabricIndex accessingFabricIndex);
    CHIP_ERROR SetRegulatoryConfig(uint8_t location, const CharSpan & countryCode);
    CHIP_ERROR PostConnectedToOperationalNetworkEvent(ByteSpan networkID);

    static DeviceControlServer & DeviceControlSvr();

private:
    // ===== Members for internal use by the following friends.
    static DeviceControlServer sInstance;

    // ===== Private members reserved for use by this class only.

    DeviceControlServer()  = default;
    ~DeviceControlServer() = default;

    // No copy, move or assignment.
    DeviceControlServer(const DeviceControlServer &)  = delete;
    DeviceControlServer(const DeviceControlServer &&) = delete;
    DeviceControlServer & operator=(const DeviceControlServer &) = delete;
};

} // namespace DeviceLayer
} // namespace chip
