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

static EnergyEvseDelegate * gEvseDelegate            = nullptr;
static EnergyEvseManager * gEvseInstance             = nullptr;
static DeviceEnergyManagementDelegate * gDEMDelegate = nullptr;
static DeviceEnergyManagementManager * gDEMInstance  = nullptr;
static EVSEManufacturer * gEvseManufacturer          = nullptr;

CHIP_ERROR DeviceEnergyManagementInit()
{
    if ((gDEMDelegate != nullptr) || (gDEMInstance != nullptr))
    {
        ChipLogError(AppServer, "DEM Instance or Delegate already exist.");
        return CHIP_ERROR_INCORRECT_STATE;
    }

    gDEMDelegate = new DeviceEnergyManagementDelegate();
    if (gDEMDelegate == nullptr)
    {
        ChipLogError(AppServer, "Failed to allocate memory for DeviceEnergyManagementDelegate");
        return CHIP_ERROR_NO_MEMORY;
    }

    /* Manufacturer may optionally not support all features, commands & attributes */
    gDEMInstance = new DeviceEnergyManagementManager(
        EndpointId(ENERGY_EVSE_ENDPOINT), *gDEMDelegate,
        BitMask<DeviceEnergyManagement::Feature, uint32_t>(
            DeviceEnergyManagement::Feature::kPowerForecastReporting, DeviceEnergyManagement::Feature::kStateForecastReporting,
            DeviceEnergyManagement::Feature::kPowerAdjustment, DeviceEnergyManagement::Feature::kForecastAdjustment),
        BitMask<DeviceEnergyManagement::OptionalCommands, uint32_t>(
            DeviceEnergyManagement::OptionalCommands::kSupportsModifyForecastRequest,
            DeviceEnergyManagement::OptionalCommands::kSupportsRequestConstraintBasedForecast));

    if (gDEMInstance == nullptr)
    {
        ChipLogError(AppServer, "Failed to allocate memory for DeviceEnergyManagementManager");
        delete gDEMDelegate;
        gDEMDelegate = nullptr;
        return CHIP_ERROR_NO_MEMORY;
    }

    CHIP_ERROR err = gDEMInstance->Init(); /* Register Attribute & Command handlers */
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "Init failed on gDEMInstance");
        delete gDEMInstance;
        delete gDEMDelegate;
        gDEMInstance = nullptr;
        gDEMDelegate = nullptr;
        return err;
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR DeviceEnergyManagementShutdown()
{
    delete gDEMInstance;
    delete gDEMDelegate;
    gDEMInstance = nullptr;
    gDEMDelegate = nullptr;

    return CHIP_NO_ERROR;
}

CHIP_ERROR EnergyEvseInit()
{
    if ((gEvseDelegate != nullptr) || (gEvseInstance != nullptr))
    {
        ChipLogError(AppServer, "EVSE Instance or Delegate already exist.");
        return CHIP_ERROR_INCORRECT_STATE;
    }

    gEvseDelegate = new EnergyEvseDelegate();
    if (gEvseDelegate == nullptr)
    {
        ChipLogError(AppServer, "Failed to allocate memory for EnergyEvseDelegate");
        return CHIP_ERROR_NO_MEMORY;
    }

    /* Manufacturer may optionally not support all features, commands & attributes */
    gEvseInstance = new EnergyEvseManager(
        EndpointId(ENERGY_EVSE_ENDPOINT), *gEvseDelegate,
        BitMask<EnergyEvse::Feature, uint32_t>(EnergyEvse::Feature::kChargingPreferences, EnergyEvse::Feature::kPlugAndCharge,
                                               EnergyEvse::Feature::kRfid, EnergyEvse::Feature::kSoCReporting,
                                               EnergyEvse::Feature::kV2x),
        BitMask<EnergyEvse::OptionalAttributes, uint32_t>(EnergyEvse::OptionalAttributes::kSupportsUserMaximumChargingCurrent,
                                                          EnergyEvse::OptionalAttributes::kSupportsRandomizationWindow,
                                                          EnergyEvse::OptionalAttributes::kSupportsApproximateEvEfficiency),
        BitMask<EnergyEvse::OptionalCommands, uint32_t>(EnergyEvse::OptionalCommands::kSupportsStartDiagnostics));

    if (gEvseInstance == nullptr)
    {
        ChipLogError(AppServer, "Failed to allocate memory for EnergyEvseManager");
        delete gEvseDelegate;
        gEvseDelegate = nullptr;
        return CHIP_ERROR_NO_MEMORY;
    }

    CHIP_ERROR err = gEvseInstance->Init(); /* Register Attribute & Command handlers */
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "Init failed on gEvseInstance");
        delete gEvseInstance;
        delete gEvseDelegate;
        gEvseInstance = nullptr;
        gEvseDelegate = nullptr;
        return err;
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR EnergyEvseShutdown()
{
    delete gEvseInstance;
    delete gEvseDelegate;
    gEvseInstance = nullptr;
    gEvseDelegate = nullptr;

    return CHIP_NO_ERROR;
}

CHIP_ERROR EVSEManufacturerInit()
{
    if (gEvseManufacturer != nullptr)
    {
        ChipLogError(AppServer, "EvseManufacturer already exist.");
        return CHIP_ERROR_INCORRECT_STATE;
    }

    /* Now create EVSEManufacturer*/
    gEvseManufacturer = new EVSEManufacturer();
    if (gEvseManufacturer == nullptr)
    {
        ChipLogError(AppServer, "Failed to allocate memory for EvseManufacturer");
        return CHIP_ERROR_NO_MEMORY;
    }

    /* Call Manufacturer specific init */
    CHIP_ERROR err = gEvseManufacturer->Init(gEvseInstance, gDEMInstance);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "Init failed on gEvseManufacturer");
        delete gEvseManufacturer;
        gEvseManufacturer = nullptr;
        return err;
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR EVSEManufacturerShutdown()
{
    delete gEvseManufacturer;
    gEvseManufacturer = nullptr;

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

    /* Shutdown the EVSEManufacturer*/
    gEvseManufacturer->Shutdown(gEvseInstance, gDEMInstance);

    /* Shutdown the Instances - deregister attribute & command handlers */
    gEvseInstance->Shutdown();
    gDEMInstance->Shutdown();

    EVSEManufacturerShutdown();       /* Free the EVSEManufacturer */
    EnergyEvseShutdown();             /* Free the EnergyEvse */
    DeviceEnergyManagementShutdown(); /* Free the DEM */
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
