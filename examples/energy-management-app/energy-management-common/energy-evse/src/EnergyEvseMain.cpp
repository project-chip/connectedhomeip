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
#include <EnergyManagementAppCommonMain.h>
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
using namespace chip::app;
using namespace chip::app::DataModel;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::DeviceEnergyManagement;
using namespace chip::app::Clusters::ElectricalPowerMeasurement;
using namespace chip::app::Clusters::ElectricalEnergyMeasurement;
using namespace chip::app::Clusters::EnergyEvse;
using namespace chip::app::Clusters::PowerTopology;

static std::unique_ptr<EnergyEvseDelegate> gEvseDelegate;
static std::unique_ptr<EvseTargetsDelegate> gEvseTargetsDelegate;
static std::unique_ptr<EnergyEvseManager> gEvseInstance;
static std::unique_ptr<EVSEManufacturer> gEvseManufacturer;

EVSEManufacturer * EnergyEvse::GetEvseManufacturer()
{
    return gEvseManufacturer.get();
}

/*
 *  @brief  Creates a Delegate and Instance for EVSE cluster
 *
 * The Instance is a container around the Delegate, so
 * create the Delegate first, then wrap it in the Instance
 * Then call the Instance->Init() to register the attribute and command handlers
 */
CHIP_ERROR EnergyEvseInit(chip::EndpointId endpointId)
{
    CHIP_ERROR err;

    if (gEvseDelegate || gEvseInstance || gEvseTargetsDelegate)
    {
        ChipLogError(AppServer, "EVSE Instance, Delegate or TargetsDelegate already exist.");
        return CHIP_ERROR_INCORRECT_STATE;
    }

    gEvseTargetsDelegate = std::make_unique<EvseTargetsDelegate>();
    if (!gEvseTargetsDelegate)
    {
        ChipLogError(AppServer, "Failed to allocate memory for EvseTargetsDelegate");
        return CHIP_ERROR_NO_MEMORY;
    }

    gEvseDelegate = std::make_unique<EnergyEvseDelegate>(*gEvseTargetsDelegate);
    if (!gEvseDelegate)
    {
        ChipLogError(AppServer, "Failed to allocate memory for EnergyEvseDelegate");
        gEvseTargetsDelegate.reset();
        return CHIP_ERROR_NO_MEMORY;
    }

    /* Manufacturer may optionally not support all features, commands & attributes */
    gEvseInstance = std::make_unique<EnergyEvseManager>(
        EndpointId(endpointId), *gEvseDelegate,
        BitMask<EnergyEvse::Feature, uint32_t>(EnergyEvse::Feature::kChargingPreferences, EnergyEvse::Feature::kRfid,
                                               EnergyEvse::Feature::kSoCReporting, EnergyEvse::Feature::kPlugAndCharge,
                                               EnergyEvse::Feature::kV2x),
        BitMask<EnergyEvse::OptionalAttributes, uint32_t>(EnergyEvse::OptionalAttributes::kSupportsUserMaximumChargingCurrent,
                                                          EnergyEvse::OptionalAttributes::kSupportsRandomizationWindow,
                                                          EnergyEvse::OptionalAttributes::kSupportsApproximateEvEfficiency),
        BitMask<EnergyEvse::OptionalCommands, uint32_t>(EnergyEvse::OptionalCommands::kSupportsStartDiagnostics));

    if (!gEvseInstance)
    {
        ChipLogError(AppServer, "Failed to allocate memory for EnergyEvseManager");
        gEvseTargetsDelegate.reset();
        gEvseDelegate.reset();
        return CHIP_ERROR_NO_MEMORY;
    }

    err = gEvseInstance->Init(); /* Register Attribute & Command handlers */
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "Init failed on gEvseInstance");
        gEvseTargetsDelegate.reset();
        gEvseInstance.reset();
        gEvseDelegate.reset();
        return err;
    }

    err = gEvseTargetsDelegate->LoadTargets();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "Failed to LoadTargets");
        gEvseTargetsDelegate.reset();
        gEvseInstance.reset();
        gEvseDelegate.reset();
        return err;
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR EnergyEvseShutdown()
{
    /* Do this in the order Instance first, then delegate
     * Ensure we call the Instance->Shutdown to free attribute & command handlers first
     */
    if (gEvseInstance)
    {
        /* deregister attribute & command handlers */
        gEvseInstance->Shutdown();
        gEvseInstance.reset();
    }

    if (gEvseDelegate)
    {
        gEvseDelegate.reset();
    }

    if (gEvseTargetsDelegate)
    {
        gEvseTargetsDelegate.reset();
    }

    return CHIP_NO_ERROR;
}

/*
 *  @brief  Creates a EVSEManufacturer class to hold the EVSE & DEM clusters
 *
 * The Instance is a container around the Delegate, so
 * create the Delegate first, then wrap it in the Instance
 * Then call the Instance->Init() to register the attribute and command handlers
 */
CHIP_ERROR EVSEManufacturerInit(chip::EndpointId powerSourceEndpointId, ElectricalPowerMeasurementInstance & epmInstance,
                                PowerTopologyInstance & ptInstance, DeviceEnergyManagementManager & demInstance,
                                DeviceEnergyManagementDelegate & demDelegate)
{
    CHIP_ERROR err;

    if (gEvseManufacturer)
    {
        ChipLogError(AppServer, "EvseManufacturer already exist.");
        return CHIP_ERROR_INCORRECT_STATE;
    }

    /* Now create EVSEManufacturer */
    gEvseManufacturer = std::make_unique<EVSEManufacturer>(gEvseInstance.get(), &epmInstance, &ptInstance, &demInstance);
    if (!gEvseManufacturer)
    {
        ChipLogError(AppServer, "Failed to allocate memory for EvseManufacturer");
        return CHIP_ERROR_NO_MEMORY;
    }

    demDelegate.SetDEMManufacturerDelegate(*gEvseManufacturer.get());

    /* Call Manufacturer specific init */
    err = gEvseManufacturer->Init(powerSourceEndpointId);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "Init failed on gEvseManufacturer");
        gEvseManufacturer.reset();
        return err;
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR EVSEManufacturerShutdown()
{
    if (gEvseManufacturer)
    {
        /* Shutdown the EVSEManufacturer */
        gEvseManufacturer->Shutdown();
        gEvseManufacturer.reset();
    }

    return CHIP_NO_ERROR;
}
