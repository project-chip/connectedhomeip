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

#include "EnergyManagementAppCmdLineOptions.h"

#include <DeviceEnergyManagementManager.h>
#include <EVSEManufacturerImpl.h>
#include <ElectricalPowerMeasurementDelegate.h>
#include <ElectricalSensorInit.h>
#include <EnergyEvseManager.h>
#include <PowerTopologyDelegate.h>
#include <device-energy-management-modes.h>
#include <energy-evse-modes.h>

#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/ConcreteAttributePath.h>
#include <app/clusters/electrical-energy-measurement-server/electrical-energy-measurement-server.h>
#include <app/clusters/network-commissioning/network-commissioning.h>
#include <app/server/Server.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/Linux/NetworkCommissioningDriver.h>

using namespace chip;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::DeviceEnergyManagement;

static std::unique_ptr<DeviceEnergyManagementDelegate> gDEMDelegate;
static std::unique_ptr<DeviceEnergyManagementManager> gDEMInstance;

DeviceEnergyManagement::DeviceEnergyManagementDelegate * GetDEMDelegate()
{
    VerifyOrDieWithMsg(gDEMDelegate.get() != nullptr, AppServer, "DEM Delegate is null");

    return gDEMDelegate.get();
}

DeviceEnergyManagementManager * GetDEMInstance()
{
    VerifyOrDieWithMsg(gDEMInstance.get() != nullptr, AppServer, "DEM Instance is null");

    return gDEMInstance.get();
}

/*
 *  @brief  Creates a Delegate and Instance for DEM
 *
 * The Instance is a container around the Delegate, so
 * create the Delegate first, then wrap it in the Instance
 * Then call the Instance->Init() to register the attribute and command handlers
 */
CHIP_ERROR DeviceEnergyManagementInit(chip::EndpointId endpointId)
{
    // TODO: support both EVSE and WaterHeater endpoints
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

    chip::BitMask<DeviceEnergyManagement::Feature> featureMap = GetFeatureMapFromCmdLine();

    /* Manufacturer may optionally not support all features, commands & attributes */
    gDEMInstance = std::make_unique<DeviceEnergyManagementManager>(EndpointId(endpointId), *gDEMDelegate, featureMap);

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
    // TODO: support both EVSE and WaterHeater endpoints
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
