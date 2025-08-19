/*
 *   Copyright (c) 2024 Project CHIP Authors
 *   All rights reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */
#pragma once

#include "DeviceSubscription.h"

#include <app/ReadClient.h>
#include <controller/CHIPDeviceController.h>
#include <lib/core/DataModelTypes.h>

#include <memory>

namespace admin {

class DeviceSubscriptionManager
{
public:
    static DeviceSubscriptionManager & Instance()
    {
        static DeviceSubscriptionManager instance;
        return instance;
    }

    /// Usually called after we have added a synchronized device to fabric-bridge to monitor
    /// for any changes that need to be propagated to fabric-bridge.
    CHIP_ERROR StartSubscription(chip::Controller::DeviceController & controller, chip::ScopedNodeId scopedNodeId);

    CHIP_ERROR RemoveSubscription(chip::ScopedNodeId scopedNodeId);

private:
    struct ScopedNodeIdHasher
    {
        std::size_t operator()(const chip::ScopedNodeId & scopedNodeId) const
        {
            std::size_t h1 = std::hash<uint64_t>{}(scopedNodeId.GetFabricIndex());
            std::size_t h2 = std::hash<uint64_t>{}(scopedNodeId.GetNodeId());
            // Bitshifting h2 reduces collisions when fabricIndex == nodeId.
            return h1 ^ (h2 << 1);
        }
    };

    void DeviceSubscriptionTerminated(chip::ScopedNodeId scopedNodeId);

    std::unordered_map<chip::ScopedNodeId, std::unique_ptr<DeviceSubscription>, ScopedNodeIdHasher> mDeviceSubscriptionMap;
};

} // namespace admin
