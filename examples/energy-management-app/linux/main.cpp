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
    if ((gDelegate == nullptr) && (gInstance == nullptr))
    {
        gDelegate = new EnergyEvseDelegate();
        if (gDelegate != nullptr)
        {
            /* Manufacturer may optionally not support all features, commands & attributes */
            gInstance = new EnergyEvseManager(
                EndpointId(ENERGY_EVSE_ENDPOINT), *gDelegate,
                BitMask<EnergyEvse::Feature, uint32_t>(EnergyEvse::Feature::kChargingPreferences,
                                                       EnergyEvse::Feature::kPlugAndCharge, EnergyEvse::Feature::kRfid,
                                                       EnergyEvse::Feature::kSoCReporting, EnergyEvse::Feature::kV2x),
                BitMask<OptionalAttributes, uint32_t>(OptionalAttributes::kSupportsUserMaximumChargingCurrent,
                                                      OptionalAttributes::kSupportsRandomizationWindow,
                                                      OptionalAttributes::kSupportsApproximateEvEfficiency),
                BitMask<OptionalCommands, uint32_t>(OptionalCommands::kSupportsStartDiagnostics));
            gInstance->Init(); /* Register Attribute & Command handlers */
        }
    }
    else
    {
        ChipLogError(AppServer, "EVSE Instance or Delegate already exist.")
    }

    if (gEvseManufacturer == nullptr)
    {
        gEvseManufacturer = new EVSEManufacturer();
        gEvseManufacturer->Init(gInstance);
    }
    else
    {
        ChipLogError(AppServer, "EVSEManufacturer already exists.")
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
