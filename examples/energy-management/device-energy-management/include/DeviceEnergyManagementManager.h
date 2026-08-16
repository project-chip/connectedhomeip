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

/**
 * @brief   Helper function to create and initialize the DeviceEnergyManagement cluster
 *
 * Creates the delegate and instance, then calls Init() to register attribute and command handlers
 *
 * @param endpointId The endpoint ID to create the cluster on
 * @param aDelegate  Reference to store the created delegate
 * @param aInstance  Reference to store the created instance
 * @param aFeatureMap The feature map for the cluster
 * @return CHIP_NO_ERROR if the DeviceEnergyManagement cluster is initialized successfully, otherwise an error code
 */
CHIP_ERROR DeviceEnergyManagementInit(chip::EndpointId endpointId,
                                      std::unique_ptr<DeviceEnergyManagement::DeviceEnergyManagementDelegate> & aDelegate,
                                      std::unique_ptr<DeviceEnergyManagementManager> & aInstance,
                                      chip::BitMask<DeviceEnergyManagement::Feature> aFeatureMap);

void DeviceEnergyManagementShutdown(std::unique_ptr<DeviceEnergyManagementManager> & aInstance,
                                    std::unique_ptr<DeviceEnergyManagement::DeviceEnergyManagementDelegate> & aDelegate);

} // namespace Clusters
} // namespace app
} // namespace chip
