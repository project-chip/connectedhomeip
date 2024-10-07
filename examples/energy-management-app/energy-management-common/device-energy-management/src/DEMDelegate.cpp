/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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
#include <device-energy-management-modes.h>

#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/ConcreteAttributePath.h>
#include <app/server/Server.h>
#include <lib/support/logging/CHIPLogging.h>

static constexpr int DEM_ENDPOINT = 1;

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::DeviceEnergyManagement;

namespace chip {
namespace app {
namespace Clusters {
namespace DeviceEnergyManagement {

// Keep track of the parsed featureMap option
#if defined(CONFIG_DEM_SUPPORT_POWER_FORECAST_REPORTING) && defined(CONFIG_DEM_SUPPORT_STATE_FORECAST_REPORTING)
#error Cannot define CONFIG_DEM_SUPPORT_POWER_FORECAST_REPORTING and CONFIG_DEM_SUPPORT_STATE_FORECAST_REPORTING
#endif

#if defined(CONFIG_DEM_SUPPORT_POWER_FORECAST_REPORTING)
static chip::BitMask<Feature> sFeatureMap(Feature::kPowerAdjustment, Feature::kPowerForecastReporting,
                                          Feature::kStartTimeAdjustment, Feature::kPausable, Feature::kForecastAdjustment,
                                          Feature::kConstraintBasedAdjustment);
#elif defined(CONFIG_DEM_SUPPORT_STATE_FORECAST_REPORTING)
static chip::BitMask<Feature> sFeatureMap(Feature::kPowerAdjustment, Feature::kStateForecastReporting,
                                          Feature::kStartTimeAdjustment, Feature::kPausable, Feature::kForecastAdjustment,
                                          Feature::kConstraintBasedAdjustment);
#else
static chip::BitMask<Feature> sFeatureMap(Feature::kPowerAdjustment);
#endif

} // namespace DeviceEnergyManagement
} // namespace Clusters
} // namespace app
} // namespace chip


chip::BitMask<chip::app::Clusters::DeviceEnergyManagement::Feature> GetDEMFeatureMap()
{
    return sFeatureMap;
}

void SetDEMFeatureMap(uint32_t featureMap)
{
    sFeatureMap = BitMask<chip::app::Clusters::DeviceEnergyManagement::Feature>(featureMap); 
}

std::unique_ptr<DeviceEnergyManagementDelegate> gDEMDelegate;
std::unique_ptr<DeviceEnergyManagementManager> gDEMInstance;

DeviceEnergyManagement::DeviceEnergyManagementDelegate * GetDEMDelegate()
{
    VerifyOrDieWithMsg(gDEMDelegate.get() != nullptr, AppServer, "DEM Delegate is null");

    return gDEMDelegate.get();
}

/*
 *  @brief  Creates a Delegate and Instance for DEM
 *
 * The Instance is a container around the Delegate, so
 * create the Delegate first, then wrap it in the Instance
 * Then call the Instance->Init() to register the attribute and command handlers
 */
CHIP_ERROR DeviceEnergyManagementInit()
{
    if (gDEMDelegate || gDEMInstance)
    {
        ChipLogError(AppServer, "DEM Instance or Delegate already exist.");
        return CHIP_ERROR_INCORRECT_STATE;
    }

    gDEMDelegate = std::make_unique<DeviceEnergyManagementDelegate>();
    if (!gDEMDelegate)
    {
        ChipLogError(AppServer, "Failed to allocate memory for DeviceEnergyManagementDelegate");
        return CHIP_ERROR_NO_MEMORY;
    }

    BitMask<DeviceEnergyManagement::Feature> featureMap = GetDEMFeatureMap();

    /* Manufacturer may optionally not support all features, commands & attributes */
    gDEMInstance = std::make_unique<DeviceEnergyManagementManager>(EndpointId(DEM_ENDPOINT), *gDEMDelegate, featureMap);

    if (!gDEMInstance)
    {
        ChipLogError(AppServer, "Failed to allocate memory for DeviceEnergyManagementManager");
        gDEMDelegate.reset();
        return CHIP_ERROR_NO_MEMORY;
    }

    gDEMDelegate->SetDeviceEnergyManagementInstance(*gDEMInstance);

    CHIP_ERROR err = gDEMInstance->Init(); /* Register Attribute & Command handlers */
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "Init failed on gDEMInstance");
        gDEMInstance.reset();
        gDEMDelegate.reset();
        return err;
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR DeviceEnergyManagementShutdown()
{
    /* Do this in the order Instance first, then delegate
     * Ensure we call the Instance->Shutdown to free attribute & command handlers first
     */
    if (gDEMInstance)
    {
        /* deregister attribute & command handlers */
        gDEMInstance->Shutdown();
        gDEMInstance.reset();
    }
    if (gDEMDelegate)
    {
        gDEMDelegate.reset();
    }
    return CHIP_NO_ERROR;
}
