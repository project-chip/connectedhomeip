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
    EnergyEvseDelegate * dg = aInstance->GetDelegate();
    if (dg == nullptr)
    {
        ChipLogError(AppServer, "Delegate is not initialized");
        return CHIP_ERROR_UNINITIALIZED;
    }

    // TODO EnergyEvseManager::GetInstance()->GetDelegate()->RegisterCallbacks();

    /* Set the EVSE Hardware Maximum current limit */
    // For Manufacturer to specify the hardware capability in mA
    dg->HwSetMaxHardwareCurrentLimit(32000);

    // For Manufacturer to specify the CircuitCapacity (e.g. from DIP switches)
    dg->HwSetCircuitCapacity(20000);

    /* For now let's pretend the EV is plugged in, and asking for demand */
    dg->HwSetState(StateEnum::kPluggedInDemand);
    dg->HwSetCableAssemblyLimit(63000);

    /* For now let's pretend the vehicle ID is set */
    dg->HwSetVehicleID(CharSpan::fromCharString("TEST_VEHICLE_123456789"));
    dg->HwSetVehicleID(CharSpan::fromCharString("TEST_VEHICLE_9876543210"));

    /* This next one will fail because it is too long */
    dg->HwSetVehicleID(CharSpan::fromCharString("TEST_VEHICLE_9876543210TOOOOOOOOOOOOOOOOOOO"));

    /* For now let's pretend the RFID sensor was triggered - send an event */
    uint8_t uid[10] = { 0x01, 0x23, 0x45, 0x67, 0x89, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE };
    dg->HwSetRFID(ByteSpan(uid));

    return CHIP_NO_ERROR;
}

CHIP_ERROR EVSEManufacturer::Shutdown(EnergyEvseManager * aInstance)
{

    return CHIP_NO_ERROR;
}
