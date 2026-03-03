/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
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
 *      Abstract base for side channel implementations (e.g. provisioning CLI).
 *      Extends BleChannel with side-channel-specific API (GATT read/write/CCCD, etc.).
 */

#pragma once

#include "BleChannel.h"
#include "BlePlatformTypes.h"
#include <lib/core/CHIPError.h>

namespace chip {
namespace DeviceLayer {
namespace Internal {

/**
 * Configuration for side-channel advertising (platform-specific payload).
 * Platform may define a concrete type; this struct is a placeholder for the skeleton.
 */
struct BleSideChannelAdvConfig
{
    void * opaque = nullptr;
};

/**
 * Abstract base for side channel. Implements BleChannel and adds
 * HandleReadRequest, HandleWriteRequest, HandleCCCDWriteRequest, UpdateMtu,
 * AddConnection, RemoveConnection, ConfigureAdvertising.
 */
class BleSideChannel : public BleChannel
{
public:
    ~BleSideChannel() override = default;

    // ----- Side-channel advertising -----
    virtual CHIP_ERROR ConfigureAdvertising(const BleSideChannelAdvConfig & config) = 0;

    // ----- GATT server (side channel) -----
    virtual void HandleReadRequest(void * platformEvent, const uint8_t * data, size_t length)   = 0;
    virtual void HandleWriteRequest(void * platformEvent, uint8_t * data, size_t length)       = 0;
    virtual void HandleCCCDWriteRequest(void * platformEvent, bool & isNewSubscription)        = 0;
    virtual void UpdateMtu(void * platformEvent)                                               = 0;

    // ----- Connection state (side channel) -----
    virtual CHIP_ERROR AddConnection(BleConnectionHandle conId, uint16_t mtu)                  = 0;
    virtual CHIP_ERROR RemoveConnection(BleConnectionHandle conId)                             = 0;
};

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
