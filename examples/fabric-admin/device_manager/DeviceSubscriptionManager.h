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

class DeviceSubscriptionManager
{
public:
    static DeviceSubscriptionManager & Instance();

    /// Usually called after we have added a synchronized device to fabric-bridge to monitor
    /// for any changes that need to be propagated to fabric-bridge.
    CHIP_ERROR StartSubscription(chip::Controller::DeviceController & controller, chip::NodeId nodeId);

    CHIP_ERROR RemoveSubscription(chip::NodeId nodeId);

private:
    void DeviceSubscriptionTerminated(chip::NodeId nodeId);

    std::unordered_map<chip::NodeId, std::unique_ptr<DeviceSubscription>> mDeviceSubscriptionMap;
};
