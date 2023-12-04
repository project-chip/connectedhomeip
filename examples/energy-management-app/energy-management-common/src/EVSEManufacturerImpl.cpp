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

#include <EVSEManufacturerImpl.h>
#include <EnergyEvseManager.h>

using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::EnergyEvse;

CHIP_ERROR EVSEManufacturer::Init(EnergyEvseManager * aInstance)
{
    /* Manufacturers should modify this to do any custom initialisation */

    /* Register callbacks */
    // TODO EnergyEvseManager::GetInstance()->GetDelegate()->RegisterCallbacks();

    /* Set the EVSE Hardware Maximum current limit */
    // For Manufacturer to specify the hardware capability in mA
    aInstance->GetDelegate()->hwSetMaxHardwareCurrentLimit(32000);

    // For Manufacturer to specify the CircuitCapacity (e.g. from DIP switches)
    aInstance->GetDelegate()->hwSetCircuitCapacity(20000);

    /* For now let's pretend the EV is plugged in, and asking for demand */
    aInstance->GetDelegate()->hwSetState(StateEnum::kPluggedInDemand);
    aInstance->GetDelegate()->hwSetCableAssemblyLimit(63000);

    return CHIP_NO_ERROR;
}

CHIP_ERROR EVSEManufacturer::Shutdown(EnergyEvseManager * aInstance)
{

    return CHIP_NO_ERROR;
}