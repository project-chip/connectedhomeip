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

#include "AppConfig.h"
#include "AppTask.h"

#include "DataModelHelper.h"
#include "DeviceEnergyManagementDelegateImpl.h"
#include "DeviceEnergyManager.h"
#include "DishwasherManager.h"
#include "EnergyTimeUtils.h"

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::DeviceEnergyManagement;

DeviceEnergyManager DeviceEnergyManager::sDeviceEnergy;

namespace {
std::unique_ptr<DeviceEnergyManagementDelegate> gDEMDelegate;
std::unique_ptr<DeviceEnergyManagementManager> gDEMInstance;
std::unique_ptr<DishwasherManager> gDishwasherManager;
} // namespace

DeviceEnergyManagement::DeviceEnergyManagementDelegate * GetDEMDelegate()
{
    VerifyOrDieWithMsg(gDEMDelegate.get() != nullptr, AppServer, "DEM Delegate is null");
    return gDEMDelegate.get();
}

DishwasherManager * DeviceEnergyManagement::GetDishwasherManager()
{
    VerifyOrDieWithMsg(gDEMDelegate.get() != nullptr, AppServer, "Dishwasher Manager is null");
    return gDishwasherManager.get();
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
    EndpointId DEMEndpointId = DataModelHelper::GetEndpointIdFromCluster(DeviceEnergyManagement::Id);
    CHIP_ERROR err;

    VerifyOrReturnError((DEMEndpointId != kInvalidEndpointId), CHIP_ERROR_INCORRECT_STATE,
                        ChipLogError(AppServer, "DEM Cluster not configured"));

    // Initialize DEM (Device Energy Management) and DEMManufacturer (Device Energy Management Manufacturer)
    VerifyOrReturnError(!gDEMDelegate && !gDEMInstance, CHIP_ERROR_INCORRECT_STATE,
                        ChipLogError(AppServer, "DEM Delegate or Instance already exist"));

    gDEMDelegate = std::make_unique<DeviceEnergyManagementDelegate>();
    VerifyOrReturnError(gDEMDelegate, CHIP_ERROR_NO_MEMORY, ChipLogError(AppServer, "Failed to allocate memory for DEM Delegate"));

    // Manufacturer may optionally not support all features, commands & attributes
    // DeviceEnergyManagement::Feature::kStateForecastReporting is removed from the specification [!PA].a,(STA|PAU|FA|CON),O
    static uint32_t featureMap = static_cast<uint32_t>(DeviceEnergyManagement::Feature::kPowerForecastReporting) +
        static_cast<uint32_t>(DeviceEnergyManagement::Feature::kStartTimeAdjustment) +
        static_cast<uint32_t>(DeviceEnergyManagement::Feature::kPausable);
    BitMask<DeviceEnergyManagement::Feature, uint32_t> fmap(featureMap);

    gDEMInstance = std::make_unique<DeviceEnergyManagementManager>(DEMEndpointId, *gDEMDelegate, fmap);

    VerifyOrReturnError(gDEMInstance, CHIP_ERROR_NO_MEMORY, ChipLogError(AppServer, "Failed to allocate memory for DEM Instance");
                        gDEMDelegate.reset());

    gDEMDelegate->SetDeviceEnergyManagementInstance(*gDEMInstance);

    // Register Attribute & Command handlers
    err = gDEMInstance->Init();
    VerifyOrReturnError(CHIP_NO_ERROR == err, err, ChipLogError(AppServer, "Init failed on gDEMInstance"); gDEMInstance.reset();
                        gDEMDelegate.reset());

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

CHIP_ERROR DeviceEnergyManager::Init()
{
    CHIP_ERROR err;

    err = DeviceEnergyManagementInit();
    VerifyOrReturnError(CHIP_NO_ERROR == err, err, ChipLogError(AppServer, "DEM Init failed"); DeviceEnergyManagementShutdown(););

    // DEM Manufacturer
    VerifyOrReturnError(!gDishwasherManager, CHIP_ERROR_INCORRECT_STATE,
                        ChipLogError(AppServer, "Dishwasher manager already initialized"););

    gDishwasherManager = std::make_unique<DishwasherManager>(gDEMInstance.get());
    VerifyOrReturnError(gDishwasherManager, CHIP_ERROR_NO_MEMORY,
                        ChipLogError(AppServer, "Failed to allocate memory for DEM ManufacturerDelegate"));

    err = gDishwasherManager->Init();
    VerifyOrReturnError(CHIP_NO_ERROR == err, err, ChipLogError(AppServer, "Init failed on gDishwasherManager"));

    gDEMDelegate->SetDEMManufacturerDelegate(*gDishwasherManager.get());

    return CHIP_NO_ERROR;
}

void DeviceEnergyManager::Shutdown()
{
    /* Do this in the order Instance first, then delegate
     * Ensure we call the Instance->Shutdown to free attribute & command handlers first
     */

    // Shutdown DEM
    if (gDEMInstance)
    {
        // Deregister attribute & command handlers
        gDEMInstance->Shutdown();
        gDEMInstance.reset();
    }

    if (gDEMDelegate)
    {
        gDEMDelegate.reset();
    }

    // Shutdown DEMManufacturer
    if (gDishwasherManager)
    {
        gDishwasherManager.reset();
    }
}

DeviceEnergyManagementDelegate * DeviceEnergyManager::GetDEMDelegate()
{
    return gDEMDelegate.get();
};

/**
 * @brief Returns the EndpointID of the DeviceEnergyManagement cluster.
 *        The energy management cluster assumes this functions is global to be able to leverage it.
 */
EndpointId GetEnergyDeviceEndpointId()
{
    return DataModelHelper::GetEndpointIdFromCluster(DeviceEnergyManagement::Id);
}
