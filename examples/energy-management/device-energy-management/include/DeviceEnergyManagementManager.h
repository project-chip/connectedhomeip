/*
 *
 *    Copyright (c) 2023-2024 Project CHIP Authors
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

#pragma once

#include <DeviceEnergyManagementDelegateImpl.h>
#include <app/util/af-types.h>
#include <lib/core/CHIPError.h>

namespace chip {
namespace app {
namespace Clusters {

using namespace chip::app::Clusters::DeviceEnergyManagement;

class DeviceEnergyManagementManager : public Instance
{
public:
    DeviceEnergyManagementManager(EndpointId aEndpointId, DeviceEnergyManagementDelegate & aDelegate, Feature aFeature);

    // Delete copy constructor and assignment operator.
    DeviceEnergyManagementManager(const DeviceEnergyManagementManager &)             = delete;
    DeviceEnergyManagementManager(const DeviceEnergyManagementManager &&)            = delete;
    DeviceEnergyManagementManager & operator=(const DeviceEnergyManagementManager &) = delete;

    CHIP_ERROR Init();
    void Shutdown();

    DeviceEnergyManagementDelegate * GetDelegate() { return mDelegate; };

private:
    DeviceEnergyManagementDelegate * mDelegate;
};

} // namespace Clusters
} // namespace app
} // namespace chip
