/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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

#include <device/types/energy-evse/impl/simulation/AllDevicesEnergyEvseDelegate.h>

#include <app/clusters/energy-evse-server/EnergyEvseTestEventTriggerHandler.h>

#include <device/types/energy-evse/impl/SimulatedEnergyEvseTriggers.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::EnergyEvse;

namespace {

void SetTestEventTrigger_BasicFunctionality(AllDevicesEnergyEvseDelegate & delegate,
                                            SimulatedEnergyEvseTestEventSaveData & saveData)
{
    EnergyEvseCluster * cluster = delegate.GetCluster();
    VerifyOrDieWithMsg(cluster != nullptr, AppServer, "EVSE cluster is null");

    saveData.mOldMaxHardwareChargeCurrentLimit    = delegate.HwGetMaxHardwareChargeCurrentLimit();
    saveData.mOldMaxHardwareDischargeCurrentLimit = delegate.HwGetMaxHardwareDischargeCurrentLimit();
    saveData.mOldCircuitCapacity                  = cluster->GetCircuitCapacity();
    saveData.mOldUserMaximumChargeCurrent         = cluster->GetUserMaximumChargeCurrent();
    saveData.mOldHwStateBasic                     = delegate.HwGetState();

    delegate.HwSetMaxHardwareChargeCurrentLimit(32000);
    delegate.HwSetMaxHardwareDischargeCurrentLimit(32000);
    delegate.HwSetCircuitCapacity(32000);
    TEMPORARY_RETURN_IGNORED cluster->SetUserMaximumChargeCurrent(32000);
    delegate.HwSetState(StateEnum::kNotPluggedIn);
}

void SetTestEventTrigger_BasicFunctionalityClear(AllDevicesEnergyEvseDelegate & delegate,
                                                 SimulatedEnergyEvseTestEventSaveData & saveData)
{
    EnergyEvseCluster * cluster = delegate.GetCluster();
    VerifyOrDieWithMsg(cluster != nullptr, AppServer, "EVSE cluster is null");

    delegate.HwSetMaxHardwareChargeCurrentLimit(saveData.mOldMaxHardwareChargeCurrentLimit);
    delegate.HwSetMaxHardwareDischargeCurrentLimit(saveData.mOldMaxHardwareDischargeCurrentLimit);
    delegate.HwSetCircuitCapacity(saveData.mOldCircuitCapacity);
    TEMPORARY_RETURN_IGNORED cluster->SetUserMaximumChargeCurrent(saveData.mOldUserMaximumChargeCurrent);
    delegate.HwSetState(saveData.mOldHwStateBasic);
}

void SetTestEventTrigger_EVPluggedIn(AllDevicesEnergyEvseDelegate & delegate, SimulatedEnergyEvseTestEventSaveData & saveData)
{
    saveData.mOldCableAssemblyLimit = delegate.HwGetCableAssemblyLimit();
    saveData.mOldHwStatePluggedIn   = delegate.HwGetState();

    delegate.HwSetCableAssemblyLimit(63000);
    delegate.HwSetState(StateEnum::kPluggedInNoDemand);
}

void SetTestEventTrigger_EVPluggedInClear(AllDevicesEnergyEvseDelegate & delegate, SimulatedEnergyEvseTestEventSaveData & saveData)
{
    delegate.HwSetCableAssemblyLimit(saveData.mOldCableAssemblyLimit);
    delegate.HwSetState(saveData.mOldHwStatePluggedIn);
}

void SetTestEventTrigger_EVChargeDemand(AllDevicesEnergyEvseDelegate & delegate, SimulatedEnergyEvseTestEventSaveData & saveData)
{
    saveData.mOldHwStatePluggedInDemand = delegate.HwGetState();
    delegate.HwSetState(StateEnum::kPluggedInDemand);
}

void SetTestEventTrigger_EVChargeDemandClear(AllDevicesEnergyEvseDelegate & delegate,
                                             SimulatedEnergyEvseTestEventSaveData & saveData)
{
    delegate.HwSetState(saveData.mOldHwStatePluggedInDemand);
}

void SetTestEventTrigger_EVTimeOfUseMode() {}
void SetTestEventTrigger_EVTimeOfUseModeClear() {}

void SetTestEventTrigger_EVSEGroundFault(AllDevicesEnergyEvseDelegate & delegate)
{
    delegate.HwSetFault(FaultStateEnum::kGroundFault);
}

void SetTestEventTrigger_EVSEOverTemperatureFault(AllDevicesEnergyEvseDelegate & delegate)
{
    delegate.HwSetFault(FaultStateEnum::kOverTemperature);
}

void SetTestEventTrigger_EVSEFaultClear(AllDevicesEnergyEvseDelegate & delegate)
{
    delegate.HwSetFault(FaultStateEnum::kNoError);
}

void SetTestEventTrigger_EVSEDiagnosticsComplete(AllDevicesEnergyEvseDelegate & delegate)
{
    delegate.HwDiagnosticsComplete();
}

void SetTestEventTrigger_EVSESetSoCLow(AllDevicesEnergyEvseDelegate & delegate)
{
    EnergyEvseCluster * cluster = delegate.GetCluster();
    VerifyOrDieWithMsg(cluster != nullptr, AppServer, "EVSE cluster is null");

    TEMPORARY_RETURN_IGNORED cluster->SetStateOfCharge(DataModel::MakeNullable(static_cast<Percent>(20)));
    TEMPORARY_RETURN_IGNORED cluster->SetBatteryCapacity(DataModel::MakeNullable(static_cast<int64_t>(70000000)));
}

void SetTestEventTrigger_EVSESetSoCHigh(AllDevicesEnergyEvseDelegate & delegate)
{
    EnergyEvseCluster * cluster = delegate.GetCluster();
    VerifyOrDieWithMsg(cluster != nullptr, AppServer, "EVSE cluster is null");

    TEMPORARY_RETURN_IGNORED cluster->SetStateOfCharge(DataModel::MakeNullable(static_cast<Percent>(95)));
    TEMPORARY_RETURN_IGNORED cluster->SetBatteryCapacity(DataModel::MakeNullable(static_cast<int64_t>(70000000)));
}

void SetTestEventTrigger_EVSESetSoCClear(AllDevicesEnergyEvseDelegate & delegate)
{
    EnergyEvseCluster * cluster = delegate.GetCluster();
    VerifyOrDieWithMsg(cluster != nullptr, AppServer, "EVSE cluster is null");

    TEMPORARY_RETURN_IGNORED cluster->SetStateOfCharge(DataModel::NullNullable);
    TEMPORARY_RETURN_IGNORED cluster->SetBatteryCapacity(DataModel::NullNullable);
}

void SetTestEventTrigger_EVSESetVehicleID(AllDevicesEnergyEvseDelegate & delegate)
{
    CharSpan vehicleIdSpan = "Test-Vehicle-ID-012345789-ABCDEF"_span;
    delegate.HwSetVehicleID(vehicleIdSpan);
}

void SetTestEventTrigger_EVSETriggerRFID(AllDevicesEnergyEvseDelegate & delegate)
{
    uint8_t rfidData[10] = { 0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99 };
    ByteSpan rfidSpan(rfidData);
    delegate.HwSetRFID(rfidSpan);
}

} // namespace

namespace chip::app {

bool HandleSimulatedEnergyEvseTestEventTrigger(AllDevicesEnergyEvseDelegate * delegate,
                                               SimulatedEnergyEvseTestEventSaveData & saveData, uint64_t eventTrigger)
{
    VerifyOrReturnError(delegate != nullptr, false);

    EnergyEvseTrigger trigger = static_cast<EnergyEvseTrigger>(eventTrigger);

    switch (trigger)
    {
    case EnergyEvseTrigger::kBasicFunctionality:
        SetTestEventTrigger_BasicFunctionality(*delegate, saveData);
        break;
    case EnergyEvseTrigger::kBasicFunctionalityClear:
        SetTestEventTrigger_BasicFunctionalityClear(*delegate, saveData);
        break;
    case EnergyEvseTrigger::kEVPluggedIn:
        SetTestEventTrigger_EVPluggedIn(*delegate, saveData);
        break;
    case EnergyEvseTrigger::kEVPluggedInClear:
        SetTestEventTrigger_EVPluggedInClear(*delegate, saveData);
        break;
    case EnergyEvseTrigger::kEVChargeDemand:
        SetTestEventTrigger_EVChargeDemand(*delegate, saveData);
        break;
    case EnergyEvseTrigger::kEVChargeDemandClear:
        SetTestEventTrigger_EVChargeDemandClear(*delegate, saveData);
        break;
    case EnergyEvseTrigger::kEVTimeOfUseMode:
        SetTestEventTrigger_EVTimeOfUseMode();
        break;
    case EnergyEvseTrigger::kEVSEGroundFault:
        SetTestEventTrigger_EVSEGroundFault(*delegate);
        break;
    case EnergyEvseTrigger::kEVSEOverTemperatureFault:
        SetTestEventTrigger_EVSEOverTemperatureFault(*delegate);
        break;
    case EnergyEvseTrigger::kEVSEFaultClear:
        SetTestEventTrigger_EVSEFaultClear(*delegate);
        break;
    case EnergyEvseTrigger::kEVSEDiagnosticsComplete:
        SetTestEventTrigger_EVSEDiagnosticsComplete(*delegate);
        break;
    case EnergyEvseTrigger::kEVTimeOfUseModeClear:
        SetTestEventTrigger_EVTimeOfUseModeClear();
        break;
    case EnergyEvseTrigger::kEVSESetSoCLow:
        SetTestEventTrigger_EVSESetSoCLow(*delegate);
        break;
    case EnergyEvseTrigger::kEVSESetSoCHigh:
        SetTestEventTrigger_EVSESetSoCHigh(*delegate);
        break;
    case EnergyEvseTrigger::kEVSESetSoCClear:
        SetTestEventTrigger_EVSESetSoCClear(*delegate);
        break;
    case EnergyEvseTrigger::kEVSESetVehicleID:
        SetTestEventTrigger_EVSESetVehicleID(*delegate);
        break;
    case EnergyEvseTrigger::kEVSETriggerRFID:
        SetTestEventTrigger_EVSETriggerRFID(*delegate);
        break;
    default:
        return false;
    }

    return true;
}

} // namespace chip::app
