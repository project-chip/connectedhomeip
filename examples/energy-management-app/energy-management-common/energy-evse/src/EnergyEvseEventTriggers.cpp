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

#include <EVSEManufacturerImpl.h>

#include <app/clusters/energy-evse-server/EnergyEvseTestEventTriggerHandler.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::EnergyEvse;

struct EVSETestEventSaveData
{
    int64_t mOldMaxHardwareChargeCurrentLimit;
    int64_t mOldMaxHardwareDischargeCurrentLimit;
    int64_t mOldCircuitCapacity;
    int64_t mOldUserMaximumChargeCurrent;
    int64_t mOldCableAssemblyLimit;
    StateEnum mOldHwStateBasic;           /* For storing hwState before Basic Func event */
    StateEnum mOldHwStatePluggedIn;       /* For storing hwState before PluggedIn event */
    StateEnum mOldHwStatePluggedInDemand; /* For storing hwState before PluggedInDemand event */
};

static EVSETestEventSaveData sEVSETestEventSaveData;

EnergyEvseDelegate * GetEvseDelegate()
{
    EVSEManufacturer * mn = GetEvseManufacturer();
    VerifyOrDieWithMsg(mn != nullptr, AppServer, "EVSEManufacturer is null");
    EnergyEvseDelegate * dg = mn->GetEvseDelegate();
    VerifyOrDieWithMsg(dg != nullptr, AppServer, "EVSE Delegate is null");

    return dg;
}

void SetTestEventTrigger_BasicFunctionality()
{
    EnergyEvseDelegate * dg = GetEvseDelegate();

    sEVSETestEventSaveData.mOldMaxHardwareChargeCurrentLimit    = dg->HwGetMaxHardwareChargeCurrentLimit();
    sEVSETestEventSaveData.mOldMaxHardwareDischargeCurrentLimit = dg->HwGetMaxHardwareDischargeCurrentLimit();
    sEVSETestEventSaveData.mOldCircuitCapacity                  = dg->GetCircuitCapacity();
    sEVSETestEventSaveData.mOldUserMaximumChargeCurrent         = dg->GetUserMaximumChargeCurrent();
    sEVSETestEventSaveData.mOldHwStateBasic                     = dg->HwGetState();

    dg->HwSetMaxHardwareChargeCurrentLimit(32000);
    dg->HwSetMaxHardwareDischargeCurrentLimit(32000);
    dg->HwSetCircuitCapacity(32000);
    dg->SetUserMaximumChargeCurrent(32000);
    dg->HwSetState(StateEnum::kNotPluggedIn);
}
void SetTestEventTrigger_BasicFunctionalityClear()
{
    EnergyEvseDelegate * dg = GetEvseDelegate();

    dg->HwSetMaxHardwareChargeCurrentLimit(sEVSETestEventSaveData.mOldMaxHardwareChargeCurrentLimit);
    dg->HwSetMaxHardwareDischargeCurrentLimit(sEVSETestEventSaveData.mOldMaxHardwareDischargeCurrentLimit);
    dg->HwSetCircuitCapacity(sEVSETestEventSaveData.mOldCircuitCapacity);
    dg->SetUserMaximumChargeCurrent(sEVSETestEventSaveData.mOldUserMaximumChargeCurrent);
    dg->HwSetState(sEVSETestEventSaveData.mOldHwStateBasic);
}
void SetTestEventTrigger_EVPluggedIn()
{
    EnergyEvseDelegate * dg = GetEvseDelegate();

    sEVSETestEventSaveData.mOldCableAssemblyLimit = dg->HwGetCableAssemblyLimit();
    sEVSETestEventSaveData.mOldHwStatePluggedIn   = dg->HwGetState();

    dg->HwSetCableAssemblyLimit(63000);
    dg->HwSetState(StateEnum::kPluggedInNoDemand);
}
void SetTestEventTrigger_EVPluggedInClear()
{
    EnergyEvseDelegate * dg = GetEvseDelegate();
    dg->HwSetCableAssemblyLimit(sEVSETestEventSaveData.mOldCableAssemblyLimit);
    dg->HwSetState(sEVSETestEventSaveData.mOldHwStatePluggedIn);
}

void SetTestEventTrigger_EVChargeDemand()
{
    EnergyEvseDelegate * dg = GetEvseDelegate();

    sEVSETestEventSaveData.mOldHwStatePluggedInDemand = dg->HwGetState();
    dg->HwSetState(StateEnum::kPluggedInDemand);
}
void SetTestEventTrigger_EVChargeDemandClear()
{
    EnergyEvseDelegate * dg = GetEvseDelegate();

    dg->HwSetState(sEVSETestEventSaveData.mOldHwStatePluggedInDemand);
}
void SetTestEventTrigger_EVTimeOfUseMode()
{
    // TODO - See #34249
}
void SetTestEventTrigger_EVTimeOfUseModeClear()
{
    // TODO - See #34249
}
void SetTestEventTrigger_EVSEGroundFault()
{
    EnergyEvseDelegate * dg = GetEvseDelegate();

    dg->HwSetFault(FaultStateEnum::kGroundFault);
}

void SetTestEventTrigger_EVSEOverTemperatureFault()
{
    EnergyEvseDelegate * dg = GetEvseDelegate();

    dg->HwSetFault(FaultStateEnum::kOverTemperature);
}

void SetTestEventTrigger_EVSEFaultClear()
{
    EnergyEvseDelegate * dg = GetEvseDelegate();

    dg->HwSetFault(FaultStateEnum::kNoError);
}

void SetTestEventTrigger_EVSEDiagnosticsComplete()
{
    EnergyEvseDelegate * dg = GetEvseDelegate();

    dg->HwDiagnosticsComplete();
}

void SetTestEventTrigger_EVSESetSoCLow()
{
    // Set SoC 20%, 70kWh BatterySize
    EnergyEvseDelegate * dg = GetEvseDelegate();
    dg->SetStateOfCharge(20);
    dg->SetBatteryCapacity(70000000);
}
void SetTestEventTrigger_EVSESetSoCHigh()
{
    // Set SoC 95%, 70kWh BatterySize
    EnergyEvseDelegate * dg = GetEvseDelegate();
    dg->SetStateOfCharge(95);
    dg->SetBatteryCapacity(70000000);
}
void SetTestEventTrigger_EVSESetSoCClear()
{
    // Set SoC null, BatterySize nu;;
    EnergyEvseDelegate * dg = GetEvseDelegate();
    DataModel::Nullable<uint8_t> noSoC;
    DataModel::Nullable<int64_t> noBatteryCapacity;
    dg->SetStateOfCharge(noSoC);
    dg->SetBatteryCapacity(noBatteryCapacity);
}
void SetTestEventTrigger_EVSESetVehicleID()
{
    CharSpan vehicleIdSpan = "Test-Vehicle-ID-012345789-ABCDEF"_span;

    EnergyEvseDelegate * dg = GetEvseDelegate();
    dg->HwSetVehicleID(vehicleIdSpan);
}
void SetTestEventTrigger_EVSETriggerRFID()
{
    EnergyEvseDelegate * dg = GetEvseDelegate();

    uint8_t rfidData[10] = { 0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99 };
    ByteSpan rfidSpan(rfidData);
    dg->HwSetRFID(rfidSpan);
}

bool HandleEnergyEvseTestEventTrigger(uint64_t eventTrigger)
{
    EnergyEvseTrigger trigger = static_cast<EnergyEvseTrigger>(eventTrigger);

    switch (trigger)
    {
    case EnergyEvseTrigger::kBasicFunctionality:
        ChipLogProgress(Support, "[EnergyEVSE-Test-Event] => Basic Functionality install");
        SetTestEventTrigger_BasicFunctionality();
        break;
    case EnergyEvseTrigger::kBasicFunctionalityClear:
        ChipLogProgress(Support, "[EnergyEVSE-Test-Event] => Basic Functionality clear");
        SetTestEventTrigger_BasicFunctionalityClear();
        break;
    case EnergyEvseTrigger::kEVPluggedIn:
        ChipLogProgress(Support, "[EnergyEVSE-Test-Event] => EV plugged in");
        SetTestEventTrigger_EVPluggedIn();
        break;
    case EnergyEvseTrigger::kEVPluggedInClear:
        ChipLogProgress(Support, "[EnergyEVSE-Test-Event] => EV unplugged");
        SetTestEventTrigger_EVPluggedInClear();
        break;
    case EnergyEvseTrigger::kEVChargeDemand:
        ChipLogProgress(Support, "[EnergyEVSE-Test-Event] => EV Charge Demand");
        SetTestEventTrigger_EVChargeDemand();
        break;
    case EnergyEvseTrigger::kEVChargeDemandClear:
        ChipLogProgress(Support, "[EnergyEVSE-Test-Event] => EV Charge NoDemand");
        SetTestEventTrigger_EVChargeDemandClear();
        break;
    case EnergyEvseTrigger::kEVTimeOfUseMode:
        ChipLogProgress(Support, "[EnergyEVSE-Test-Event] => EV TimeOfUse Mode");
        SetTestEventTrigger_EVTimeOfUseMode();
        break;
    case EnergyEvseTrigger::kEVSEGroundFault:
        ChipLogProgress(Support, "[EnergyEVSE-Test-Event] => EVSE has a GroundFault fault");
        SetTestEventTrigger_EVSEGroundFault();
        break;
    case EnergyEvseTrigger::kEVSEOverTemperatureFault:
        ChipLogProgress(Support, "[EnergyEVSE-Test-Event] => EVSE has a OverTemperature fault");
        SetTestEventTrigger_EVSEOverTemperatureFault();
        break;
    case EnergyEvseTrigger::kEVSEFaultClear:
        ChipLogProgress(Support, "[EnergyEVSE-Test-Event] => EVSE faults have cleared");
        SetTestEventTrigger_EVSEFaultClear();
        break;
    case EnergyEvseTrigger::kEVSEDiagnosticsComplete:
        ChipLogProgress(Support, "[EnergyEVSE-Test-Event] => EVSE Diagnostics Completed");
        SetTestEventTrigger_EVSEDiagnosticsComplete();
        break;
    case EnergyEvseTrigger::kEVTimeOfUseModeClear:
        ChipLogProgress(Support, "[EnergyEVSE-Test-Event] => EV TimeOfUse Mode clear");
        SetTestEventTrigger_EVTimeOfUseModeClear();
        break;
    case EnergyEvseTrigger::kEVSESetSoCLow:
        ChipLogProgress(Support, "[EnergyEVSE-Test-Event] => EVSE Set SoC Low");
        SetTestEventTrigger_EVSESetSoCLow();
        break;
    case EnergyEvseTrigger::kEVSESetSoCHigh:
        ChipLogProgress(Support, "[EnergyEVSE-Test-Event] => EVSE Set SoC High");
        SetTestEventTrigger_EVSESetSoCHigh();
        break;
    case EnergyEvseTrigger::kEVSESetSoCClear:
        ChipLogProgress(Support, "[EnergyEVSE-Test-Event] => EVSE Set SoC Clear");
        SetTestEventTrigger_EVSESetSoCClear();
        break;
    case EnergyEvseTrigger::kEVSESetVehicleID:
        ChipLogProgress(Support, "[EnergyEVSE-Test-Event] => EVSE Set VehicleID");
        SetTestEventTrigger_EVSESetVehicleID();
        break;
    case EnergyEvseTrigger::kEVSETriggerRFID:
        ChipLogProgress(Support, "[EnergyEVSE-Test-Event] => EVSE Trigger RFID");
        SetTestEventTrigger_EVSETriggerRFID();
        break;
    default:
        return false;
    }

    return true;
}
