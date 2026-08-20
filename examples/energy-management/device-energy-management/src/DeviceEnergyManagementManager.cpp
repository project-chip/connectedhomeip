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

#include <DeviceEnergyManagementManager.h>

using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::DeviceEnergyManagement;

namespace chip {
namespace app {
namespace Clusters {

DeviceEnergyManagementManager::DeviceEnergyManagementManager(EndpointId aEndpointId, DeviceEnergyManagementDelegate & aDelegate,
                                                             Feature aFeature) :
    DeviceEnergyManagement::Instance(aEndpointId, aDelegate, aFeature),
    mDelegate(&aDelegate)
{}

CHIP_ERROR DeviceEnergyManagementManager::Init()
{
    return Instance::Init();
}

void DeviceEnergyManagementManager::Shutdown()
{
    Instance::Shutdown();
}

/*
 *  @brief  Creates a Delegate and Instance for DEM
 *
 * The Instance is a container around the Delegate, so
 * create the Delegate first, then wrap it in the Instance
 * Then call the Instance->Init() to register the attribute and command handlers
 */
CHIP_ERROR DeviceEnergyManagementInit(chip::EndpointId endpointId,
                                      std::unique_ptr<DeviceEnergyManagement::DeviceEnergyManagementDelegate> & aDelegate,
                                      std::unique_ptr<DeviceEnergyManagementManager> & aInstance,
                                      chip::BitMask<DeviceEnergyManagement::Feature> aFeatureMap)
{
    if (aDelegate || aInstance)
    {
        ChipLogError(AppServer, "DEM Instance or Delegate already exist.");
        return CHIP_ERROR_INCORRECT_STATE;
    }

    aDelegate = std::make_unique<DeviceEnergyManagement::DeviceEnergyManagementDelegate>();
    if (!aDelegate)
    {
        ChipLogError(AppServer, "Failed to allocate memory for DeviceEnergyManagementDelegate");
        return CHIP_ERROR_NO_MEMORY;
    }

    /* Manufacturer may optionally not support all features, commands & attributes */
    aInstance = std::make_unique<DeviceEnergyManagementManager>(endpointId, *aDelegate, aFeatureMap);

    if (!aInstance)
    {
        ChipLogError(AppServer, "Failed to allocate memory for DeviceEnergyManagement Instance");
        aDelegate.reset();
        return CHIP_ERROR_NO_MEMORY;
    }

    aDelegate->SetDeviceEnergyManagementInstance(*aInstance);

    CHIP_ERROR err = aInstance->Init(); /* Register Attribute & Command handlers */
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "Init failed on DeviceEnergyManagement Instance");
        aInstance.reset();
        aDelegate.reset();
        return err;
    }

    return CHIP_NO_ERROR;
}

void DeviceEnergyManagementShutdown(std::unique_ptr<DeviceEnergyManagementManager> & aInstance,
                                    std::unique_ptr<DeviceEnergyManagement::DeviceEnergyManagementDelegate> & aDelegate)
{
    /* Do this in the order Instance first, then delegate
     * Ensure we call the Instance->Shutdown to free attribute & command handlers first
     */
    if (aInstance)
    {
        /* deregister attribute & command handlers */
        aInstance->Shutdown();
        aInstance.reset();
    }
    if (aDelegate)
    {
        aDelegate.reset();
    }
}

} // namespace Clusters
} // namespace app
} // namespace chip
