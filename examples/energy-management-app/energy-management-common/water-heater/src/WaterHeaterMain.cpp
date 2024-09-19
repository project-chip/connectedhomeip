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

#include <DeviceEnergyManagementDelegateImpl.h>
#include <DeviceEnergyManagementManager.h>
#include <ElectricalPowerMeasurementDelegate.h>
#include <PowerTopologyDelegate.h>
#include <WhmManufacturer.h>
#include <device-energy-management-modes.h>
#include <water-heater-mode.h>

#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/ConcreteAttributePath.h>
#include <app/clusters/electrical-energy-measurement-server/electrical-energy-measurement-server.h>
#include <app/clusters/network-commissioning/network-commissioning.h>
#include <app/clusters/power-topology-server/power-topology-server.h>
#include <app/server/Server.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/Linux/NetworkCommissioningDriver.h>

#include <DEMDelegate.h>
#include <EnergyEvseMain.h>
#include <WhmMain.h>

namespace chip {
namespace app {
namespace Clusters {
namespace WaterHeaterManagement {

void FullWhmApplicationInit()
{
    ReturnOnFailure(WhmApplicationInit());

    if (DeviceEnergyManagementInit() != CHIP_NO_ERROR)
    {
        WhmApplicationShutdown();
        return;
    }

    if (EnergyMeterInit() != CHIP_NO_ERROR)
    {
        DeviceEnergyManagementShutdown();
        WhmApplicationShutdown();
        return;
    }

    if (PowerTopologyInit() != CHIP_NO_ERROR)
    {
        EnergyMeterShutdown();
        DeviceEnergyManagementShutdown();
        WhmApplicationShutdown();
        return;
    }

    /* For Device Energy Management we need the ESA to be Online and ready to accept commands */

    GetDEMDelegate()->SetESAState(ESAStateEnum::kOnline);
    GetDEMDelegate()->SetESAType(ESATypeEnum::kWaterHeating);
    GetDEMDelegate()->SetDEMManufacturerDelegate(*GetWhmManufacturer());

    // Set the abs min and max power
    GetDEMDelegate()->SetAbsMinPower(1200000); // 1.2KW
    GetDEMDelegate()->SetAbsMaxPower(7600000); // 7.6KW
}

void FullWhmApplicationShutdown()
{
    ChipLogDetail(AppServer, "Energy Management App (WaterHeater): ApplicationShutdown()");

    /* Shutdown in reverse order that they were created */
    PowerTopologyShutdown();          /* Free the PowerTopology */
    EnergyMeterShutdown();            /* Free the Energy Meter */
    DeviceEnergyManagementShutdown(); /* Free the DEM */
    WhmApplicationShutdown();

    Clusters::DeviceEnergyManagementMode::Shutdown();
    Clusters::WaterHeaterMode::Shutdown();
}

} // namespace WaterHeaterManagement
} // namespace Clusters
} // namespace app
} // namespace chip
