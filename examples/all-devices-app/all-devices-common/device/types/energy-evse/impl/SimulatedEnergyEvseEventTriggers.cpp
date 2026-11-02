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

namespace chip::app {

Clusters::EnergyEvse::AllDevicesEnergyEvseDelegate * gSimulatedEnergyEvseDelegateForTriggers = nullptr;

void SetSimulatedEnergyEvseDelegateForTriggers(Clusters::EnergyEvse::AllDevicesEnergyEvseDelegate * delegate)
{
    gSimulatedEnergyEvseDelegateForTriggers = delegate;
}

} // namespace chip::app

namespace {

struct EVSETestEventSaveData
{
    int64_t mOldMaxHardwareChargeCurrentLimit;
    int64_t mOldMaxHardwareDischargeCurrentLimit;
    int64_t mOldCircuitCapacity;
    int64_t mOldUserMaximumChargeCurrent;
    int64_t mOldCableAssemblyLimit;
    StateEnum mOldHwStateBasic;
    StateEnum mOldHwStatePluggedIn;
    StateEnum mOldHwStatePluggedInDemand;
};

static EVSETestEventSaveData sEVSETestEventSaveData;

AllDevicesEnergyEvseDelegate * GetEvseDelegate()
{
    AllDevicesEnergyEvseDelegate * dg = chip::app::gSimulatedEnergyEvseDelegateForTriggers;
    VerifyOrDieWithMsg(dg != nullptr, AppServer, "Simulated EVSE delegate is null");
    return dg;
}

void SetTestEventTrigger_BasicFunctionality()
{
    AllDevicesEnergyEvseDelegate * dg = GetEvseDelegate();
    EnergyEvseCluster * cluster       = dg->GetCluster();
    VerifyOrDieWithMsg(cluster != nullptr, AppServer, "EVSE cluster is null");

    sEVSETestEventSaveData.mOldMaxHardwareChargeCurrentLimit    = dg->HwGetMaxHardwareChargeCurrentLimit();
    sEVSETestEventSaveData.mOldMaxHardwareDischargeCurrentLimit = dg->HwGetMaxHardwareDischargeCurrentLimit();
    sEVSETestEventSaveData.mOldCircuitCapacity                  = cluster->GetCircuitCapacity();
    sEVSETestEventSaveData.mOldUserMaximumChargeCurrent         = cluster->GetUserMaximumChargeCurrent();
    sEVSETestEventSaveData.mOldHwStateBasic                     = dg->HwGetState();

    dg->HwSetMaxHardwareChargeCurrentLimit(32000);
    dg->HwSetMaxHardwareDischargeCurrentLimit(32000);
    dg->HwSetCircuitCapacity(32000);
    TEMPORARY_RETURN_IGNORED cluster->SetUserMaximumChargeCurrent(32000);
    dg->HwSetState(StateEnum::kNotPluggedIn);
}

void SetTestEventTrigger_BasicFunctionalityClear()
{
    AllDevicesEnergyEvseDelegate * dg = GetEvseDelegate();
    EnergyEvseCluster * cluster       = dg->GetCluster();
    VerifyOrDieWithMsg(cluster != nullptr, AppServer, "EVSE cluster is null");

    dg->HwSetMaxHardwareChargeCurrentLimit(sEVSETestEventSaveData.mOldMaxHardwareChargeCurrentLimit);
    dg->HwSetMaxHardwareDischargeCurrentLimit(sEVSETestEventSaveData.mOldMaxHardwareDischargeCurrentLimit);
    dg->HwSetCircuitCapacity(sEVSETestEventSaveData.mOldCircuitCapacity);
    TEMPORARY_RETURN_IGNORED cluster->SetUserMaximumChargeCurrent(sEVSETestEventSaveData.mOldUserMaximumChargeCurrent);
    dg->HwSetState(sEVSETestEventSaveData.mOldHwStateBasic);
}

void SetTestEventTrigger_EVPluggedIn()
{
    AllDevicesEnergyEvseDelegate * dg = GetEvseDelegate();

    sEVSETestEventSaveData.mOldCableAssemblyLimit = dg->HwGetCableAssemblyLimit();
    sEVSETestEventSaveData.mOldHwStatePluggedIn   = dg->HwGetState();

    dg->HwSetCableAssemblyLimit(63000);
    dg->HwSetState(StateEnum::kPluggedInNoDemand);
}

void SetTestEventTrigger_EVPluggedInClear()
{
    AllDevicesEnergyEvseDelegate * dg = GetEvseDelegate();
    dg->HwSetCableAssemblyLimit(sEVSETestEventSaveData.mOldCableAssemblyLimit);
    dg->HwSetState(sEVSETestEventSaveData.mOldHwStatePluggedIn);
}

void SetTestEventTrigger_EVChargeDemand()
{
    AllDevicesEnergyEvseDelegate * dg = GetEvseDelegate();

    sEVSETestEventSaveData.mOldHwStatePluggedInDemand = dg->HwGetState();
    dg->HwSetState(StateEnum::kPluggedInDemand);
}

void SetTestEventTrigger_EVChargeDemandClear()
{
    AllDevicesEnergyEvseDelegate * dg = GetEvseDelegate();

    dg->HwSetState(sEVSETestEventSaveData.mOldHwStatePluggedInDemand);
}

void SetTestEventTrigger_EVTimeOfUseMode() {}
void SetTestEventTrigger_EVTimeOfUseModeClear() {}

void SetTestEventTrigger_EVSEGroundFault()
{
    GetEvseDelegate()->HwSetFault(FaultStateEnum::kGroundFault);
}

void SetTestEventTrigger_EVSEOverTemperatureFault()
{
    GetEvseDelegate()->HwSetFault(FaultStateEnum::kOverTemperature);
}

void SetTestEventTrigger_EVSEFaultClear()
{
    GetEvseDelegate()->HwSetFault(FaultStateEnum::kNoError);
}

void SetTestEventTrigger_EVSEDiagnosticsComplete()
{
    GetEvseDelegate()->HwDiagnosticsComplete();
}

void SetTestEventTrigger_EVSESetSoCLow()
{
    AllDevicesEnergyEvseDelegate * dg = GetEvseDelegate();
    EnergyEvseCluster * cluster       = dg->GetCluster();
    VerifyOrDieWithMsg(cluster != nullptr, AppServer, "EVSE cluster is null");

    TEMPORARY_RETURN_IGNORED cluster->SetStateOfCharge(DataModel::MakeNullable(static_cast<Percent>(20)));
    TEMPORARY_RETURN_IGNORED cluster->SetBatteryCapacity(DataModel::MakeNullable(static_cast<int64_t>(70000000)));
}

void SetTestEventTrigger_EVSESetSoCHigh()
{
    EnergyEvseCluster * cluster = GetEvseDelegate()->GetCluster();
    VerifyOrDieWithMsg(cluster != nullptr, AppServer, "EVSE cluster is null");

    TEMPORARY_RETURN_IGNORED cluster->SetStateOfCharge(DataModel::MakeNullable(static_cast<Percent>(95)));
    TEMPORARY_RETURN_IGNORED cluster->SetBatteryCapacity(DataModel::MakeNullable(static_cast<int64_t>(70000000)));
}

void SetTestEventTrigger_EVSESetSoCClear()
{
    EnergyEvseCluster * cluster = GetEvseDelegate()->GetCluster();
    VerifyOrDieWithMsg(cluster != nullptr, AppServer, "EVSE cluster is null");

    TEMPORARY_RETURN_IGNORED cluster->SetStateOfCharge(DataModel::NullNullable);
    TEMPORARY_RETURN_IGNORED cluster->SetBatteryCapacity(DataModel::NullNullable);
}

void SetTestEventTrigger_EVSESetVehicleID()
{
    CharSpan vehicleIdSpan = "Test-Vehicle-ID-012345789-ABCDEF"_span;
    GetEvseDelegate()->HwSetVehicleID(vehicleIdSpan);
}

void SetTestEventTrigger_EVSETriggerRFID()
{
    uint8_t rfidData[10] = { 0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99 };
    ByteSpan rfidSpan(rfidData);
    GetEvseDelegate()->HwSetRFID(rfidSpan);
}

} // namespace

bool HandleEnergyEvseTestEventTrigger(uint64_t eventTrigger)
{
    EnergyEvseTrigger trigger = static_cast<EnergyEvseTrigger>(eventTrigger);

    switch (trigger)
    {
    case EnergyEvseTrigger::kBasicFunctionality:
        SetTestEventTrigger_BasicFunctionality();
        break;
    case EnergyEvseTrigger::kBasicFunctionalityClear:
        SetTestEventTrigger_BasicFunctionalityClear();
        break;
    case EnergyEvseTrigger::kEVPluggedIn:
        SetTestEventTrigger_EVPluggedIn();
        break;
    case EnergyEvseTrigger::kEVPluggedInClear:
        SetTestEventTrigger_EVPluggedInClear();
        break;
    case EnergyEvseTrigger::kEVChargeDemand:
        SetTestEventTrigger_EVChargeDemand();
        break;
    case EnergyEvseTrigger::kEVChargeDemandClear:
        SetTestEventTrigger_EVChargeDemandClear();
        break;
    case EnergyEvseTrigger::kEVTimeOfUseMode:
        SetTestEventTrigger_EVTimeOfUseMode();
        break;
    case EnergyEvseTrigger::kEVSEGroundFault:
        SetTestEventTrigger_EVSEGroundFault();
        break;
    case EnergyEvseTrigger::kEVSEOverTemperatureFault:
        SetTestEventTrigger_EVSEOverTemperatureFault();
        break;
    case EnergyEvseTrigger::kEVSEFaultClear:
        SetTestEventTrigger_EVSEFaultClear();
        break;
    case EnergyEvseTrigger::kEVSEDiagnosticsComplete:
        SetTestEventTrigger_EVSEDiagnosticsComplete();
        break;
    case EnergyEvseTrigger::kEVTimeOfUseModeClear:
        SetTestEventTrigger_EVTimeOfUseModeClear();
        break;
    case EnergyEvseTrigger::kEVSESetSoCLow:
        SetTestEventTrigger_EVSESetSoCLow();
        break;
    case EnergyEvseTrigger::kEVSESetSoCHigh:
        SetTestEventTrigger_EVSESetSoCHigh();
        break;
    case EnergyEvseTrigger::kEVSESetSoCClear:
        SetTestEventTrigger_EVSESetSoCClear();
        break;
    case EnergyEvseTrigger::kEVSESetVehicleID:
        SetTestEventTrigger_EVSESetVehicleID();
        break;
    case EnergyEvseTrigger::kEVSETriggerRFID:
        SetTestEventTrigger_EVSETriggerRFID();
        break;
    default:
        return false;
    }

    return true;
}
