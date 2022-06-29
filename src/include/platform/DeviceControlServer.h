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
