/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::DeviceEnergyManagement;

static std::unique_ptr<DeviceEnergyManagementDelegate> gDelegate;
static std::unique_ptr<DeviceEnergyManagementManager> gInstance;

void emberAfDeviceEnergyManagementClusterInitCallback(chip::EndpointId endpointId)
{
    VerifyOrDie(endpointId == 1); // this cluster is only enabled for endpoint 1.
    VerifyOrDie(gInstance == nullptr);

    CHIP_ERROR err;

    if (gDelegate || gInstance)
    {
        ChipLogError(AppServer, "DEM Instance or Delegate already exist.");
        return;
    }

    gDelegate = std::make_unique<DeviceEnergyManagementDelegate>();
    if (!gDelegate)
    {
        ChipLogError(AppServer, "Failed to allocate memory for DeviceEnergyManagementDelegate");
        return;
    }

    /* Manufacturer may optionally not support all features, commands & attributes */
    gInstance = std::make_unique<DeviceEnergyManagementManager>(
        endpointId, *gDelegate,
        BitMask<DeviceEnergyManagement::Feature, uint32_t>(
            DeviceEnergyManagement::Feature::kPowerAdjustment, DeviceEnergyManagement::Feature::kPowerForecastReporting,
            DeviceEnergyManagement::Feature::kStateForecastReporting, DeviceEnergyManagement::Feature::kStartTimeAdjustment,
            DeviceEnergyManagement::Feature::kPausable));

    if (!gInstance)
    {
        ChipLogError(AppServer, "Failed to allocate memory for DeviceEnergyManagementManager");
        gDelegate.reset();
        return;
    }

    err = gInstance->Init(); /* Register Attribute & Command handlers */
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "Init failed on gInstance");
        gInstance.reset();
        gDelegate.reset();
        return;
    }
}
