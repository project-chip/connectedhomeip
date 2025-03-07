/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
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

#include "EnergyGatewayAppCommonMain.h"

#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/data-model/Nullable.h>
#include <lib/support/CodeUtils.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::DataModel;
using namespace chip::app::Clusters;


/*
 *  @brief  Creates a Delegate and Instance for CommodityPrice clusters
 *
 * The Instance is a container around the Delegate, so
 * create the Delegate first, then wrap it in the Instance
 * Then call the Instance->Init() to register the attribute and command handlers
 */
void ElectricalPriceApplicationInit()
{
//     auto endpointId = GetEnergyDeviceEndpointId();
//     VerifyOrDie(EnergyGatewayCommonClustersInit(endpointId) == CHIP_NO_ERROR);
//     VerifyOrDie(EnergyEvseInit(endpointId) == CHIP_NO_ERROR);
//     VerifyOrDie(EVSEManufacturerInit(endpointId, *gEPMInstance.get(), *gPTInstance.get(), *gDEMInstance.get(),
//                                      *gDEMDelegate.get()) == CHIP_NO_ERROR);
}

void ElectricalPriceApplicationShutdown()
{
    ChipLogDetail(AppServer, "Energy Gateway App : ElectricalPriceApplicationShutdown()");

    // /* Shutdown in reverse order that they were created */
    // EVSEManufacturerShutdown();           /* Free the EVSEManufacturer */
    // PowerTopologyShutdown();              /* Free the PowerTopology */
    // ElectricalPowerMeasurementShutdown(); /* Free the Electrical Power Measurement */
    // EnergyEvseShutdown();                 /* Free the EnergyEvse */
    // DeviceEnergyManagementShutdown();     /* Free the DEM */

    // Clusters::DeviceEnergyManagementMode::Shutdown();
    // Clusters::EnergyEvseMode::Shutdown();
}
