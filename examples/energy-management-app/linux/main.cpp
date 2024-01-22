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

#include <AppMain.h>
#include <DeviceEnergyManagementManager.h>
#include <EVSEManufacturerImpl.h>
#include <EnergyEvseManager.h>
#include <EnergyManagementManager.h>
#include <device-energy-management-modes.h>
#include <energy-evse-modes.h>

#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/ConcreteAttributePath.h>
#include <app/clusters/network-commissioning/network-commissioning.h>
#include <app/server/Server.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/Linux/NetworkCommissioningDriver.h>

#define ENERGY_EVSE_ENDPOINT 1

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;

static std::unique_ptr<EnergyEvseDelegate> gEvseDelegate;
static std::unique_ptr<EnergyEvseManager> gEvseInstance;
static std::unique_ptr<DeviceEnergyManagementDelegate> gDEMDelegate;
static std::unique_ptr<DeviceEnergyManagementManager> gDEMInstance;
static std::unique_ptr<EVSEManufacturer> gEvseManufacturer;

EVSEManufacturer * EnergyEvse::GetEvseManufacturer()
{
    return gEvseManufacturer.get();
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

    /* Manufacturer may optionally not support all features, commands & attributes */
    gDEMInstance = std::make_unique<DeviceEnergyManagementManager>(
        EndpointId(ENERGY_EVSE_ENDPOINT), *gDEMDelegate,
        BitMask<DeviceEnergyManagement::Feature, uint32_t>(
            DeviceEnergyManagement::Feature::kPowerAdjustment, DeviceEnergyManagement::Feature::kPowerForecastReporting,
            DeviceEnergyManagement::Feature::kStateForecastReporting, DeviceEnergyManagement::Feature::kStartTimeAdjustment,
            DeviceEnergyManagement::Feature::kPausable, DeviceEnergyManagement::Feature::kForecastAdjustment,
            DeviceEnergyManagement::Feature::kConstraintBasedAdjustment));

    if (!gDEMInstance)
    {
        ChipLogError(AppServer, "Failed to allocate memory for DeviceEnergyManagementManager");
        gDEMDelegate.reset();
        return CHIP_ERROR_NO_MEMORY;
    }

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

/*
 *  @brief  Creates a Delegate and Instance for EVSE cluster
 *
 * The Instance is a container around the Delegate, so
 * create the Delegate first, then wrap it in the Instance
 * Then call the Instance->Init() to register the attribute and command handlers
 */
CHIP_ERROR EnergyEvseInit()
{
    CHIP_ERROR err;

    if (gEvseDelegate || gEvseInstance)
    {
        ChipLogError(AppServer, "EVSE Instance or Delegate already exist.");
        return CHIP_ERROR_INCORRECT_STATE;
    }

    gEvseDelegate = std::make_unique<EnergyEvseDelegate>();
    if (!gEvseDelegate)
    {
        ChipLogError(AppServer, "Failed to allocate memory for EnergyEvseDelegate");
        return CHIP_ERROR_NO_MEMORY;
    }

    /* Manufacturer may optionally not support all features, commands & attributes */
    gEvseInstance = std::make_unique<EnergyEvseManager>(
        EndpointId(ENERGY_EVSE_ENDPOINT), *gEvseDelegate,
        BitMask<EnergyEvse::Feature, uint32_t>(EnergyEvse::Feature::kChargingPreferences, EnergyEvse::Feature::kPlugAndCharge,
                                               EnergyEvse::Feature::kRfid, EnergyEvse::Feature::kSoCReporting,
                                               EnergyEvse::Feature::kV2x),
        BitMask<EnergyEvse::OptionalAttributes, uint32_t>(EnergyEvse::OptionalAttributes::kSupportsUserMaximumChargingCurrent,
                                                          EnergyEvse::OptionalAttributes::kSupportsRandomizationWindow,
                                                          EnergyEvse::OptionalAttributes::kSupportsApproximateEvEfficiency),
        BitMask<EnergyEvse::OptionalCommands, uint32_t>(EnergyEvse::OptionalCommands::kSupportsStartDiagnostics));

    if (!gEvseInstance)
    {
        ChipLogError(AppServer, "Failed to allocate memory for EnergyEvseManager");
        gEvseDelegate.reset();
        return CHIP_ERROR_NO_MEMORY;
    }

    err = gEvseInstance->Init(); /* Register Attribute & Command handlers */
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "Init failed on gEvseInstance");
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

    return CHIP_NO_ERROR;
}

/*
 *  @brief  Creates a EVSEManufacturer class to hold the EVSE & DEM clusters
 *
 * The Instance is a container around the Delegate, so
 * create the Delegate first, then wrap it in the Instance
 * Then call the Instance->Init() to register the attribute and command handlers
 */
CHIP_ERROR EVSEManufacturerInit()
{
    CHIP_ERROR err;

    if (gEvseManufacturer)
    {
        ChipLogError(AppServer, "EvseManufacturer already exist.");
        return CHIP_ERROR_INCORRECT_STATE;
    }

    /* Now create EVSEManufacturer */
    // TODO  this takes just the EVSE Instance for now, but will need the DEM adding
    gEvseManufacturer = std::make_unique<EVSEManufacturer>(gEvseInstance.get());
    if (!gEvseManufacturer)
    {
        ChipLogError(AppServer, "Failed to allocate memory for EvseManufacturer");
        return CHIP_ERROR_NO_MEMORY;
    }

    /* Call Manufacturer specific init */
    err = gEvseManufacturer->Init();
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

void ApplicationInit()
{
    if (DeviceEnergyManagementInit() != CHIP_NO_ERROR)
    {
        return;
    }

    if (EnergyEvseInit() != CHIP_NO_ERROR)
    {
        DeviceEnergyManagementShutdown();
        return;
    }

    if (EVSEManufacturerInit() != CHIP_NO_ERROR)
    {
        DeviceEnergyManagementShutdown();
        EnergyEvseShutdown();
        return;
    }
}

void ApplicationShutdown()
{
    ChipLogDetail(AppServer, "Energy Management App: ApplicationShutdown()");

    /* Shutdown in reverse order that they were created */
    EVSEManufacturerShutdown();       /* Free the EVSEManufacturer */
    EnergyEvseShutdown();             /* Free the EnergyEvse */
    DeviceEnergyManagementShutdown(); /* Free the DEM */

    Clusters::DeviceEnergyManagementMode::Shutdown();
    Clusters::EnergyEvseMode::Shutdown();
}

int main(int argc, char * argv[])
{
    if (ChipLinuxAppInit(argc, argv) != 0)
    {
        return -1;
    }

    ChipLinuxAppMainLoop();

    return 0;
}
