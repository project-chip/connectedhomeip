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

#include <device/types/electrical-sensor/ElectricalSensor.h>
#include <platform/DefaultTimerDelegate.h>

namespace chip::app {

/**
 * Simulated electrical sensor that minimally impements and logs on delegate calls
 */
class LoggingElectricalSensor : public ElectricalSensor,
                                public TimerContext,
                                public Clusters::ElectricalEnergyMeasurement::Delegate,
                                public Clusters::ElectricalPowerMeasurement::Delegate,
                                public Clusters::PowerTopology::Delegate
{
public:
    explicit LoggingElectricalSensor(TimerDelegate & timerDelegate) : ElectricalSensor(timerDelegate, *this, *this, *this){};
    ~LoggingElectricalSensor() override = default;

    // Clusters::ElectricalEnergyMeasurement::Delegate implementation

    constexpr static auto msg                      = "%s device: %s cluster: %s called";
    constexpr static auto msgWithIndex             = "%s device: %s cluster: %s called with index %u";
    constexpr static auto device                   = "ElectricalSensor";
    constexpr static auto energyMeasurementCluster = "ElectricalEnergyMeasurement";
    constexpr static auto powerMeasurementCluster  = "ElectricalPowerMeasurement";
    constexpr static auto powerTopologyCluster     = "PowerTopology";

    DataModel::Nullable<int64_t> GetCumulativeEnergyImported() override
    {
        ChipLogProgress(DeviceLayer, msg, device, energyMeasurementCluster, "GetCumulativeEnergyImported");
        return DataModel::NullNullable;
    }
    DataModel::Nullable<int64_t> GetCumulativeEnergyExported() override
    {
        ChipLogProgress(DeviceLayer, msg, device, energyMeasurementCluster, "GetCumulativeEnergyExported");
        return DataModel::NullNullable;
    }
    DataModel::Nullable<int64_t> GetPeriodicEnergyImported() override
    {
        ChipLogProgress(DeviceLayer, msg, device, energyMeasurementCluster, "GetPeriodicEnergyImported");
        return DataModel::NullNullable;
    }
    DataModel::Nullable<int64_t> GetPeriodicEnergyExported() override
    {
        ChipLogProgress(DeviceLayer, msg, device, energyMeasurementCluster, "GetPeriodicEnergyExported");
        return DataModel::NullNullable;
    }

    // Clusters::ElectricalPowerMeasurement::Delegate implementation

    Clusters::ElectricalPowerMeasurement::PowerModeEnum GetPowerMode() override
    {
        ChipLogProgress(DeviceLayer, msg, device, powerMeasurementCluster, "GetPowerMode");
        return Clusters::ElectricalPowerMeasurement::PowerModeEnum::kUnknown;
    }
    uint8_t GetNumberOfMeasurementTypes() override
    {
        ChipLogProgress(DeviceLayer, msg, device, powerMeasurementCluster, "GetNumberOfMeasurementTypes");
        return 0;
    }

    CHIP_ERROR StartAccuracyRead() override
    {
        ChipLogProgress(DeviceLayer, msg, device, powerMeasurementCluster, "StartAccuracyRead");
        return CHIP_NO_ERROR;
    }
    CHIP_ERROR GetAccuracyByIndex(uint8_t index,
                                  Clusters::ElectricalPowerMeasurement::Structs::MeasurementAccuracyStruct::Type & val) override
    {
        ChipLogProgress(DeviceLayer, msgWithIndex, device, powerMeasurementCluster, "GetAccuracyByIndex", index);
        val = {};
        return index == 0 ? CHIP_NO_ERROR : CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
    }
    CHIP_ERROR EndAccuracyRead() override
    {
        ChipLogProgress(DeviceLayer, msg, device, powerMeasurementCluster, "EndAccuracyRead");
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR StartRangesRead() override
    {
        ChipLogProgress(DeviceLayer, msg, device, powerMeasurementCluster, "StartRangesRead");
        return CHIP_NO_ERROR;
    }
    CHIP_ERROR GetRangeByIndex(uint8_t index,
                               Clusters::ElectricalPowerMeasurement::Structs::MeasurementRangeStruct::Type &) override
    {
        ChipLogProgress(DeviceLayer, msgWithIndex, device, powerMeasurementCluster, "GetRangeByIndex", index);
        return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
    }
    CHIP_ERROR EndRangesRead() override
    {
        ChipLogProgress(DeviceLayer, msg, device, powerMeasurementCluster, "EndRangesRead");
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR StartHarmonicCurrentsRead() override
    {
        ChipLogProgress(DeviceLayer, msg, device, powerMeasurementCluster, "StartHarmonicCurrentsRead");
        return CHIP_NO_ERROR;
    }
    CHIP_ERROR GetHarmonicCurrentsByIndex(uint8_t index,
                                          Clusters::ElectricalPowerMeasurement::Structs::HarmonicMeasurementStruct::Type &) override
    {
        ChipLogProgress(DeviceLayer, msgWithIndex, device, powerMeasurementCluster, "GetHarmonicCurrentsByIndex", index);
        return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
    }
    CHIP_ERROR EndHarmonicCurrentsRead() override
    {
        ChipLogProgress(DeviceLayer, msg, device, powerMeasurementCluster, "EndHarmonicCurrentsRead");
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR StartHarmonicPhasesRead() override
    {
        ChipLogProgress(DeviceLayer, msg, device, powerMeasurementCluster, "StartHarmonicPhasesRead");
        return CHIP_NO_ERROR;
    }
    CHIP_ERROR GetHarmonicPhasesByIndex(uint8_t index,
                                        Clusters::ElectricalPowerMeasurement::Structs::HarmonicMeasurementStruct::Type &) override
    {
        ChipLogProgress(DeviceLayer, msgWithIndex, device, powerMeasurementCluster, "GetHarmonicPhasesByIndex", index);
        return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
    }
    CHIP_ERROR EndHarmonicPhasesRead() override
    {
        ChipLogProgress(DeviceLayer, msg, device, powerMeasurementCluster, "EndHarmonicPhasesRead");
        return CHIP_NO_ERROR;
    }

    DataModel::Nullable<int64_t> GetVoltage() override
    {
        ChipLogProgress(DeviceLayer, msg, device, powerMeasurementCluster, "GetVoltage");
        return DataModel::NullNullable;
    }
    DataModel::Nullable<int64_t> GetActiveCurrent() override
    {
        ChipLogProgress(DeviceLayer, msg, device, powerMeasurementCluster, "GetActiveCurrent");
        return DataModel::NullNullable;
    }
    DataModel::Nullable<int64_t> GetReactiveCurrent() override
    {
        ChipLogProgress(DeviceLayer, msg, device, powerMeasurementCluster, "GetReactiveCurrent");
        return DataModel::NullNullable;
    }
    DataModel::Nullable<int64_t> GetApparentCurrent() override
    {
        ChipLogProgress(DeviceLayer, msg, device, powerMeasurementCluster, "GetApparentCurrent");
        return DataModel::NullNullable;
    }
    DataModel::Nullable<int64_t> GetActivePower() override
    {
        ChipLogProgress(DeviceLayer, msg, device, powerMeasurementCluster, "GetActivePower");
        return DataModel::NullNullable;
    }
    DataModel::Nullable<int64_t> GetReactivePower() override
    {
        ChipLogProgress(DeviceLayer, msg, device, powerMeasurementCluster, "GetReactivePower");
        return DataModel::NullNullable;
    }
    DataModel::Nullable<int64_t> GetApparentPower() override
    {
        ChipLogProgress(DeviceLayer, msg, device, powerMeasurementCluster, "GetApparentPower");
        return DataModel::NullNullable;
    }
    DataModel::Nullable<int64_t> GetRMSVoltage() override
    {
        ChipLogProgress(DeviceLayer, msg, device, powerMeasurementCluster, "GetRMSVoltage");
        return DataModel::NullNullable;
    }
    DataModel::Nullable<int64_t> GetRMSCurrent() override
    {
        ChipLogProgress(DeviceLayer, msg, device, powerMeasurementCluster, "GetRMSCurrent");
        return DataModel::NullNullable;
    }
    DataModel::Nullable<int64_t> GetRMSPower() override
    {
        ChipLogProgress(DeviceLayer, msg, device, powerMeasurementCluster, "GetRMSPower");
        return DataModel::NullNullable;
    }
    DataModel::Nullable<int64_t> GetFrequency() override
    {
        ChipLogProgress(DeviceLayer, msg, device, powerMeasurementCluster, "GetFrequency");
        return DataModel::NullNullable;
    }
    DataModel::Nullable<int64_t> GetPowerFactor() override
    {
        ChipLogProgress(DeviceLayer, msg, device, powerMeasurementCluster, "GetPowerFactor");
        return DataModel::NullNullable;
    }
    DataModel::Nullable<int64_t> GetNeutralCurrent() override
    {
        ChipLogProgress(DeviceLayer, msg, device, powerMeasurementCluster, "GetNeutralCurrent");
        return DataModel::NullNullable;
    }

    // Clusters::PowerTopology::Delegate implementation

    CHIP_ERROR GetAvailableEndpointAtIndex(size_t index, EndpointId & endpointId) override
    {
        ChipLogProgress(DeviceLayer, msgWithIndex, device, powerTopologyCluster, "GetAvailableEndpointAtIndex", index);
        return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
    }
    CHIP_ERROR GetActiveEndpointAtIndex(size_t index, EndpointId & endpointId) override
    {
        ChipLogProgress(DeviceLayer, msgWithIndex, device, powerTopologyCluster, "GetActiveEndpointAtIndex", index);
        return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
    }
};

} // namespace chip::app
