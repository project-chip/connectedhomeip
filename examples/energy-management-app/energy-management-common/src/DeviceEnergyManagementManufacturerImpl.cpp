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

#include <DeviceEnergyManagementManufacturerImpl.h>
// #include <EnergyEvseManager.h>
#include <app/clusters/device-energy-management-server/DeviceEnergyManagementTestEventTriggerHandler.h>
// #include <app/clusters/electrical-energy-measurement-server/EnergyReportingTestEventTriggerHandler.h>
// #include <app/clusters/electrical-energy-measurement-server/electrical-energy-measurement-server.h>
// #include <app/clusters/energy-evse-server/EnergyEvseTestEventTriggerHandler.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::DeviceEnergyManagement;

CHIP_ERROR DeviceEnergyManagementManufacturer::Init()
{
    /* Manufacturers should modify this to do any custom initialisation */

    /* Register callbacks */
    DeviceEnergyManagementDelegate * dg = GetDeviceEnergyManagementManufacturer()->GetDelegate();
    if (dg == nullptr)
    {
        ChipLogError(AppServer, "EVSE Delegate is not initialized");
        return CHIP_ERROR_UNINITIALIZED;
    }

// TODO:    dg->HwRegisterEvseCallbackHandler(ApplicationCallbackHandler, reinterpret_cast<intptr_t>(this));

    /*
     * This is an example implementation for manufacturers to consider
     *
     * For Manufacturer to specify the hardware capability in mA:
     *  dg->HwSetMaxHardwareCurrentLimit(32000);    // 32A
     *
     * For Manufacturer to specify the CircuitCapacity in mA (e.g. from DIP switches)
     *  dg->HwSetCircuitCapacity(20000);            // 20A
     *
     */

    /* Once the system is initialised then check to see if the state was restored
     * (e.g. after a power outage), and if the Enable timer check needs to be started
     */
//    dg->ScheduleCheckOnEnabledTimeout();

    return CHIP_NO_ERROR;
}

/*
 * When the EV is plugged in, and asking for demand change the state
 * and set the CableAssembly current limit
 *
 *   EnergyEvseDelegate * dg = GetEvseManufacturer()->GetDelegate();
 *   if (dg == nullptr)
 *   {
 *       ChipLogError(AppServer, "Delegate is not initialized");
 *       return CHIP_ERROR_UNINITIALIZED;
 *   }
 *
 *   dg->HwSetState(StateEnum::kPluggedInDemand);
 *   dg->HwSetCableAssemblyLimit(63000);    // 63A = 63000mA
 *
 *
 * If the vehicle ID can be retrieved (e.g. over Powerline)
 *   dg->HwSetVehicleID(CharSpan::fromCharString("TEST_VEHICLE_123456789"));
 *
 *
 * If the EVSE has an RFID sensor, the RFID value read can cause an event to be sent
 * (e.g. can be used to indicate if a user as tried to activate the charging)
 *
 *   uint8_t uid[10] = { 0x01, 0x23, 0x45, 0x67, 0x89, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE };
 *   dg->HwSetRFID(ByteSpan(uid));
 */

CHIP_ERROR DeviceEnergyManagementManufacturer::Shutdown()
{
    return CHIP_NO_ERROR;
}

/**
 * @brief   Allows a client application to send in power readings into the system
 *
 * @param[in]  aEndpointId     - Endpoint to send to EPM Cluster
 */
// CHIP_ERROR DeviceEnergyManagementManufacturer::SendPowerReading(EndpointId aEndpointId, int64_t aActivePower_mW, int64_t aVoltage_mV,
//                                               int64_t aCurrent_mA)
// {
//     // TODO add Power Readings when EPM cluster is merged

//     return CHIP_NO_ERROR;
// }


// struct EVSETestEventSaveData
// {
//     int64_t mOldMaxHardwareCurrentLimit;
//     int64_t mOldCircuitCapacity;
//     int64_t mOldUserMaximumChargeCurrent;
//     int64_t mOldCableAssemblyLimit;
//     StateEnum mOldHwStateBasic;           /* For storing hwState before Basic Func event */
//     StateEnum mOldHwStatePluggedIn;       /* For storing hwState before PluggedIn event */
//     StateEnum mOldHwStatePluggedInDemand; /* For storing hwState before PluggedInDemand event */
// };

// static EVSETestEventSaveData sEVSETestEventSaveData;

// EnergyEvseDelegate * GetEvseDelegate()
// {
//     DeviceEnergyManagementManufacturer * mn = GetDeviceEnergyManagementManufacturer();
//     VerifyOrDieWithMsg(mn != nullptr, AppServer, "DeviceEnergyManagementManufacturer is null");
//     DeviceEnergyManagementDelegate * dg = mn->GetDelegate();
//     VerifyOrDieWithMsg(dg != nullptr, AppServer, "DeviceEnergyManagement Delegate is null");

//     return dg;
// }

void SetTestEventTrigger_zzzzzzzzzz()
{
//     EnergyEvseDelegate * dg = GetEvseDelegate();
//     sEVSETestEventSaveData.mOldMaxHardwareCurrentLimit  = dg->HwGetMaxHardwareCurrentLimit();
//     sEVSETestEventSaveData.mOldCircuitCapacity          = dg->GetCircuitCapacity();
//     sEVSETestEventSaveData.mOldUserMaximumChargeCurrent = dg->GetUserMaximumChargeCurrent();
//     sEVSETestEventSaveData.mOldHwStateBasic             = dg->HwGetState();
//     dg->HwSetMaxHardwareCurrentLimit(32000);
//     dg->HwSetCircuitCapacity(32000);
//     dg->SetUserMaximumChargeCurrent(32000);
//     dg->HwSetState(StateEnum::kNotPluggedIn);
}

bool HandleDeviceEnergyManagementTestEventTrigger(uint64_t eventTrigger)
{
    DeviceEnergyManagementTrigger trigger = static_cast<DeviceEnergyManagementTrigger>(eventTrigger);

 ChipLogProgress(Support, "[PowerAdjustment-Test-Event] => zzzzzzzzz-%d", __LINE__);
 
    switch (trigger)
    {
    case DeviceEnergyManagementTrigger::kPowerAdjustment:
        ChipLogProgress(Support, "[PowerAdjustment-Test-Event] => Create PowerAdjustment struct");
        SetTestEventTrigger_zzzzzzzzzz();
        break;
    case DeviceEnergyManagementTrigger::kPowerAdjustmentClear:
        ChipLogProgress(Support, "[PowerAdjustmentClear-Test-Event] => Clear PowerAdjustment struct");
        // TODO call implementation
        break;
    case DeviceEnergyManagementTrigger::kUserOptOutLocalOptimization:
        ChipLogProgress(Support, "[UserOptOutLocalOptimization-Test-Event] => Set User opt-out Local Optimization");
        // TODO call implementation
        break;
    case DeviceEnergyManagementTrigger::kUserOptOutGridOptimization:
        ChipLogProgress(Support, "[UserOptOutGrisOptimization-Test-Event] => Set User opt-out Grid Optimization");
        // TODO call implementation
        break;
    case DeviceEnergyManagementTrigger::kUserOptOutClearAll:
        ChipLogProgress(Support, "[UserOptOutClearAll-Test-Event] => Clear all User opt-outs");
        // TODO call implementation
        break;
    case DeviceEnergyManagementTrigger::kStartTimeAdjustment:
        ChipLogProgress(Support, "[StartTimeAdjustment-Test-Event] => Create simulated forecast to allow StartTimeAdjustment");
        // TODO call implementation
        break;
    case DeviceEnergyManagementTrigger::kStartTimeAdjustmentClear:
        ChipLogProgress(Support, "[StartTimeAdjustmentClear-Test-Event] => Clear StartTimeAdjustment forecast");
        // TODO call implementation
        break;
    case DeviceEnergyManagementTrigger::kPausable:
        ChipLogProgress(Support, "[Pausable-Test-Event] => Create Pausable forecast");
        // TODO call implementation
        break;
    case DeviceEnergyManagementTrigger::kPausableNextSlot:
        ChipLogProgress(Support, "[PausableNextSlot-Test-Event] => Move to next Pausable slot in forecast");
        // TODO call implementation
        break;
    case DeviceEnergyManagementTrigger::kPausableClear:
        ChipLogProgress(Support, "[PausableClear-Test-Event] => Clear Pausable forecast");
        // TODO call implementation
        break;

    default:
        return false;
    }

    return true;
}