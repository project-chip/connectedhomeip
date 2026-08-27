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

#include "FakeReadings.h"

#include <device/types/electrical-sensor/ElectricalSensor.h>
#include <app/TestEventTriggerDelegate.h>
#include <lib/support/TimerDelegate.h>

namespace chip::app {

/**
 * Simulated electrical sensor that supports test event triggers
 */
class SimulatedElectricalSensor : public ElectricalSensor,
                                public Clusters::ElectricalEnergyMeasurement::Delegate,
                                public Clusters::ElectricalPowerMeasurement::Delegate,
                                public Clusters::PowerTopology::Delegate
{
public:

    explicit SimulatedElectricalSensor(TimerDelegate & timerDelegate, TestEventTriggerDelegate & testEventTriggerDelegate);
    ~SimulatedElectricalSensor() override = default;

    CHIP_ERROR Register(chip::EndpointId endpoint, CodeDrivenDataModelProvider & provider,
                        EndpointComposition composition = {}) override;
    void Unregister(CodeDrivenDataModelProvider & provider) override;

    // Clusters::ElectricalEnergyMeasurement::Delegate implementation

    DataModel::Nullable<int64_t> GetCumulativeEnergyImported() override
    {
        return MakeNullable(mFakeReadings.GetCumulativeEnergyImported());
    }
    DataModel::Nullable<int64_t> GetCumulativeEnergyExported() override
    {
        return MakeNullable(mFakeReadings.GetCumulativeEnergyExported());
    }
    DataModel::Nullable<int64_t> GetPeriodicEnergyImported() override
    {
        return MakeNullable(mFakeReadings.GetPeriodicEnergyImported());
    }
    DataModel::Nullable<int64_t> GetPeriodicEnergyExported() override
    {
        return MakeNullable(mFakeReadings.GetPeriodicEnergyExported());
    }

    // Clusters::ElectricalPowerMeasurement::Delegate implementation

    Clusters::ElectricalPowerMeasurement::PowerModeEnum GetPowerMode() override { return ElectricalPowerMeasurement::PowerModeEnum::kAc; }
    uint8_t GetNumberOfMeasurementTypes() override { return 1; }

    CHIP_ERROR StartAccuracyRead() override { return CHIP_NO_ERROR; }
    CHIP_ERROR GetAccuracyByIndex(uint8_t index,
                                  Clusters::ElectricalPowerMeasurement::Structs::MeasurementAccuracyStruct::Type & val) override;
    CHIP_ERROR EndAccuracyRead() override { return CHIP_NO_ERROR; }

    CHIP_ERROR StartRangesRead() override { return CHIP_NO_ERROR; }
    CHIP_ERROR GetRangeByIndex(uint8_t index,
                               Clusters::ElectricalPowerMeasurement::Structs::MeasurementRangeStruct::Type &) override
    {
        return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
    }
    CHIP_ERROR EndRangesRead() override { return CHIP_NO_ERROR; }

    CHIP_ERROR StartHarmonicCurrentsRead() override { return CHIP_NO_ERROR; }
    CHIP_ERROR GetHarmonicCurrentsByIndex(uint8_t index,
                                          Clusters::ElectricalPowerMeasurement::Structs::HarmonicMeasurementStruct::Type &) override
    {
        return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
    }
    CHIP_ERROR EndHarmonicCurrentsRead() override { return CHIP_NO_ERROR; }

    CHIP_ERROR StartHarmonicPhasesRead() override { return CHIP_NO_ERROR; }
    CHIP_ERROR GetHarmonicPhasesByIndex(uint8_t index,
                                        Clusters::ElectricalPowerMeasurement::Structs::HarmonicMeasurementStruct::Type &) override
    {
        return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
    }
    CHIP_ERROR EndHarmonicPhasesRead() override { return CHIP_NO_ERROR; }

    DataModel::Nullable<int64_t> GetVoltage() override { return MakeNullable(mFakeReadings.GetVoltage()); }
    DataModel::Nullable<int64_t> GetActiveCurrent() override { return MakeNullable(mFakeReadings.GetActiveCurrent()); }
    DataModel::Nullable<int64_t> GetReactiveCurrent() override { return NullNullable; }
    DataModel::Nullable<int64_t> GetApparentCurrent() override { return NullNullable; }
    DataModel::Nullable<int64_t> GetActivePower() override { return MakeNullable(mFakeReadings.GetActivePower()); }
    DataModel::Nullable<int64_t> GetReactivePower() override { return NullNullable; }
    DataModel::Nullable<int64_t> GetApparentPower() override { return NullNullable; }
    DataModel::Nullable<int64_t> GetRMSVoltage() override { return NullNullable; }
    DataModel::Nullable<int64_t> GetRMSCurrent() override { return NullNullable; }
    DataModel::Nullable<int64_t> GetRMSPower() override { return NullNullable; }
    DataModel::Nullable<int64_t> GetFrequency() override { return NullNullable; }
    DataModel::Nullable<int64_t> GetPowerFactor() override { return NullNullable; }
    DataModel::Nullable<int64_t> GetNeutralCurrent() override { return NullNullable; }

    // Clusters::PowerTopology::Delegate implementation

    CHIP_ERROR GetAvailableEndpointAtIndex(size_t index, EndpointId & endpointId) override { return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED; }
    CHIP_ERROR GetActiveEndpointAtIndex(size_t index, EndpointId & endpointId) override { return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED; }

private:
    FakeReadings mFakeReadings;
    TestEventTriggerDelegate & mTestEventTriggerDelegate;
};

} // namespace chip::app
