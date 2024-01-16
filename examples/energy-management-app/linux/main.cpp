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

static std::unique_ptr<EnergyEvseDelegate> gDelegate;
static std::unique_ptr<EnergyEvseManager> gInstance;
static std::unique_ptr<EVSEManufacturer> gEvseManufacturer;

EVSEManufacturer * EnergyEvse::GetEvseManufacturer()
{
    return gEvseManufacturer.get();
}

void ApplicationInit()
{
    CHIP_ERROR err;

    if (gDelegate || gInstance || gEvseManufacturer)
    {
        ChipLogError(AppServer, "EVSE Instance or Delegate, EvseManufacturer already exist.");
        return;
    }

    gDelegate = std::make_unique<EnergyEvseDelegate>();
    if (!gDelegate)
    {
        ChipLogError(AppServer, "Failed to allocate memory for EnergyEvseDelegate");
        return;
    }

    /* Manufacturer may optionally not support all features, commands & attributes */
    gInstance = std::make_unique<EnergyEvseManager>(
        EndpointId(ENERGY_EVSE_ENDPOINT), *gDelegate,
        BitMask<EnergyEvse::Feature, uint32_t>(EnergyEvse::Feature::kChargingPreferences, EnergyEvse::Feature::kPlugAndCharge,
                                               EnergyEvse::Feature::kRfid, EnergyEvse::Feature::kSoCReporting,
                                               EnergyEvse::Feature::kV2x),
        BitMask<OptionalAttributes, uint32_t>(OptionalAttributes::kSupportsUserMaximumChargingCurrent,
                                              OptionalAttributes::kSupportsRandomizationWindow,
                                              OptionalAttributes::kSupportsApproximateEvEfficiency),
        BitMask<OptionalCommands, uint32_t>(OptionalCommands::kSupportsStartDiagnostics));

    if (!gInstance)
    {
        ChipLogError(AppServer, "Failed to allocate memory for EnergyEvseManager");
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

    /* Now create EVSEManufacturer*/
    gEvseManufacturer = std::make_unique<EVSEManufacturer>(gInstance.get());
    if (!gEvseManufacturer)
    {
        ChipLogError(AppServer, "Failed to allocate memory for EvseManufacturer");
        gInstance.reset();
        gDelegate.reset();
        return;
    }

    /* Call Manufacturer specific init */
    err = gEvseManufacturer->Init();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "Init failed on gEvseManufacturer");
        gEvseManufacturer.reset();
        gInstance.reset();
        gDelegate.reset();
        return;
    }
}

void ApplicationShutdown()
{
    ChipLogDetail(AppServer, "Energy Management App: ApplicationShutdown()");

    /* Shutdown the EVSEManufacturer*/
    if (gEvseManufacturer)
        gEvseManufacturer->Shutdown();

    /* Shutdown the Instance - deregister attribute & command handler */
    if (gInstance)
        gInstance->Shutdown();
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
