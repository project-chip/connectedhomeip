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

#pragma once

#include <app/clusters/energy-evse-server/EnergyEvseTestEventTriggerHandler.h>
#include <device/types/energy-evse/impl/simulation/AllDevicesEnergyEvseDelegate.h>

namespace chip::app {

struct SimulatedEnergyEvseTestEventSaveData
{
    int64_t mOldMaxHardwareChargeCurrentLimit                  = 0;
    int64_t mOldMaxHardwareDischargeCurrentLimit               = 0;
    int64_t mOldCircuitCapacity                                = 0;
    int64_t mOldUserMaximumChargeCurrent                       = 0;
    int64_t mOldCableAssemblyLimit                             = 0;
    Clusters::EnergyEvse::StateEnum mOldHwStateBasic           = Clusters::EnergyEvse::StateEnum::kNotPluggedIn;
    Clusters::EnergyEvse::StateEnum mOldHwStatePluggedIn       = Clusters::EnergyEvse::StateEnum::kNotPluggedIn;
    Clusters::EnergyEvse::StateEnum mOldHwStatePluggedInDemand = Clusters::EnergyEvse::StateEnum::kNotPluggedIn;
};

bool HandleSimulatedEnergyEvseTestEventTrigger(Clusters::EnergyEvse::AllDevicesEnergyEvseDelegate * delegate,
                                               SimulatedEnergyEvseTestEventSaveData & saveData, uint64_t eventTrigger);

} // namespace chip::app
