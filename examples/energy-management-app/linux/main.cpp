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

static EnergyEvseDelegate * gDelegate       = nullptr;
static EnergyEvseManager * gInstance        = nullptr;
static EVSEManufacturer * gEvseManufacturer = nullptr;

void ApplicationInit()
{
    CHIP_ERROR err;

    if ((gDelegate != nullptr) || (gInstance != nullptr) || (gEvseManufacturer != nullptr))
    {
        ChipLogError(AppServer, "EVSE Instance or Delegate, EvseManufacturer already exist.");
        return;
    }

    gDelegate = new EnergyEvseDelegate();
    if (gDelegate == nullptr)
    {
        ChipLogError(AppServer, "Failed to allocate memory for EnergyEvseDelegate");
        return;
    }

    /* Manufacturer may optionally not support all features, commands & attributes */
    gInstance =
        new EnergyEvseManager(EndpointId(ENERGY_EVSE_ENDPOINT), *gDelegate,
                              BitMask<EnergyEvse::Feature, uint32_t>(
                                  EnergyEvse::Feature::kChargingPreferences, EnergyEvse::Feature::kPlugAndCharge,
                                  EnergyEvse::Feature::kRfid, EnergyEvse::Feature::kSoCReporting, EnergyEvse::Feature::kV2x),
                              BitMask<OptionalAttributes, uint32_t>(OptionalAttributes::kSupportsUserMaximumChargingCurrent,
                                                                    OptionalAttributes::kSupportsRandomizationWindow,
                                                                    OptionalAttributes::kSupportsApproximateEvEfficiency),
                              BitMask<OptionalCommands, uint32_t>(OptionalCommands::kSupportsStartDiagnostics));

    if (gInstance == nullptr)
    {
        ChipLogError(AppServer, "Failed to allocate memory for EnergyEvseManager");
        delete gDelegate;
        gDelegate = nullptr;
        return;
    }

    err = gInstance->Init(); /* Register Attribute & Command handlers */
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "Init failed on gInstance");
        delete gInstance;
        delete gDelegate;
        gInstance = nullptr;
        gDelegate = nullptr;
        return;
    }

    /* Now create EVSEManufacturer*/
    gEvseManufacturer = new EVSEManufacturer();
    if (gEvseManufacturer == nullptr)
    {
        ChipLogError(AppServer, "Failed to allocate memory for EvseManufacturer");
        delete gInstance;
        delete gDelegate;
        gInstance = nullptr;
        gDelegate = nullptr;
        return;
    }

    /* Call Manufacturer specific init */
    err = gEvseManufacturer->Init(gInstance);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "Init failed on gEvseManufacturer");
        delete gEvseManufacturer;
        delete gInstance;
        delete gDelegate;
        gEvseManufacturer = nullptr;
        gInstance         = nullptr;
        gDelegate         = nullptr;
        return;
    }
}

void ApplicationShutdown()
{
    ChipLogDetail(AppServer, "Energy Management App: ApplicationShutdown()");

    /* Shutdown the EVSEManufacturer*/
    gEvseManufacturer->Shutdown(gInstance);

    /* Shutdown the Instance - deregister attribute & command handler */
    gInstance->Shutdown();

    delete gEvseManufacturer;
    delete gInstance;
    delete gDelegate;
    gEvseManufacturer = nullptr;
    gInstance         = nullptr;
    gDelegate         = nullptr;
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
