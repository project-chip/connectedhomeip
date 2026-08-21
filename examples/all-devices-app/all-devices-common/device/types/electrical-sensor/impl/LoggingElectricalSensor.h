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
#include <lib/support/TimerDelegate.h>

namespace chip::app {

/**
 * Simulated electrical sensor that minimally impements and logs on delegate calls
 */
class LoggingElectricalSensor : public ElectricalSensor,
                                public Clusters::ElectricalEnergyMeasurement::Delegate,
                                public Clusters::ElectricalPowerMeasurement::Delegate,
                                public Clusters::PowerTopology::Delegate
{
public:
    explicit LoggingElectricalSensor(TimerDelegate & timerDelegate);
    ~LoggingElectricalSensor() override = default;

    CHIP_ERROR Register(chip::EndpointId endpoint, CodeDrivenDataModelProvider & provider,
                        EndpointComposition composition = {}) override;
    void Unregister(CodeDrivenDataModelProvider & provider) override;

    // Clusters::ElectricalEnergyMeasurement::Delegate implementation

    DataModel::Nullable<int64_t> GetCumulativeEnergyImported() override;
    DataModel::Nullable<int64_t> GetCumulativeEnergyExported() override;
    DataModel::Nullable<int64_t> GetPeriodicEnergyImported() override;
    DataModel::Nullable<int64_t> GetPeriodicEnergyExported() override;

    // Clusters::ElectricalPowerMeasurement::Delegate implementation

    Clusters::ElectricalPowerMeasurement::PowerModeEnum GetPowerMode() override;
    uint8_t GetNumberOfMeasurementTypes() override;

    CHIP_ERROR StartAccuracyRead() override;
    CHIP_ERROR GetAccuracyByIndex(uint8_t index,
                                  Clusters::ElectricalPowerMeasurement::Structs::MeasurementAccuracyStruct::Type & val) override;
    CHIP_ERROR EndAccuracyRead() override;

    CHIP_ERROR StartRangesRead() override;
    CHIP_ERROR GetRangeByIndex(uint8_t index,
                               Clusters::ElectricalPowerMeasurement::Structs::MeasurementRangeStruct::Type &) override;
    CHIP_ERROR EndRangesRead() override;

    CHIP_ERROR StartHarmonicCurrentsRead() override;
    CHIP_ERROR
    GetHarmonicCurrentsByIndex(uint8_t index,
                               Clusters::ElectricalPowerMeasurement::Structs::HarmonicMeasurementStruct::Type &) override;
    CHIP_ERROR EndHarmonicCurrentsRead() override;

    CHIP_ERROR StartHarmonicPhasesRead() override;
    CHIP_ERROR GetHarmonicPhasesByIndex(uint8_t index,
                                        Clusters::ElectricalPowerMeasurement::Structs::HarmonicMeasurementStruct::Type &) override;
    CHIP_ERROR EndHarmonicPhasesRead() override;

    DataModel::Nullable<int64_t> GetVoltage() override;
    DataModel::Nullable<int64_t> GetActiveCurrent() override;
    DataModel::Nullable<int64_t> GetReactiveCurrent() override;
    DataModel::Nullable<int64_t> GetApparentCurrent() override;
    DataModel::Nullable<int64_t> GetActivePower() override;
    DataModel::Nullable<int64_t> GetReactivePower() override;
    DataModel::Nullable<int64_t> GetApparentPower() override;
    DataModel::Nullable<int64_t> GetRMSVoltage() override;
    DataModel::Nullable<int64_t> GetRMSCurrent() override;
    DataModel::Nullable<int64_t> GetRMSPower() override;
    DataModel::Nullable<int64_t> GetFrequency() override;
    DataModel::Nullable<int64_t> GetPowerFactor() override;
    DataModel::Nullable<int64_t> GetNeutralCurrent() override;

    // Clusters::PowerTopology::Delegate implementation

    CHIP_ERROR GetAvailableEndpointAtIndex(size_t index, EndpointId & endpointId) override;
    CHIP_ERROR GetActiveEndpointAtIndex(size_t index, EndpointId & endpointId) override;

private:
    FakeReadings mFakeReadings;
};

} // namespace chip::app
