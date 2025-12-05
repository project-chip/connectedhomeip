/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
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
#pragma once

// #include <app-common/zap-generated/cluster-objects.h>
#include <clusters/ElectricalPowerMeasurement/Enums.h>
#include <clusters/ElectricalPowerMeasurement/Structs.h>
#include <lib/core/CHIPError.h>
#include <lib/core/DataModelTypes.h>
#include <lib/core/Optional.h>
namespace chip {
namespace app {
namespace Clusters {
namespace ElectricalPowerMeasurement {

class Delegate
{
public:
    virtual ~Delegate() = default;

    void SetEndpointId(EndpointId aEndpoint) { mEndpointId = aEndpoint; }

    virtual PowerModeEnum GetPowerMode()          = 0;
    virtual uint8_t GetNumberOfMeasurementTypes() = 0;

    /* These functions are called by the ReadAttribute handler to iterate through lists
     * The cluster server will call Start<Type>Read to allow the delegate to create a temporary
     * lock on the data.
     * The delegate is expected to not change these values once Start<Type>Read has been called
     * until the End<Type>Read() has been called (e.g. releasing a lock on the data)
     */
    virtual CHIP_ERROR StartAccuracyRead()                                                     = 0;
    virtual CHIP_ERROR GetAccuracyByIndex(uint8_t, Structs::MeasurementAccuracyStruct::Type &) = 0;
    virtual CHIP_ERROR EndAccuracyRead()                                                       = 0;

    virtual CHIP_ERROR StartRangesRead()                                                 = 0;
    virtual CHIP_ERROR GetRangeByIndex(uint8_t, Structs::MeasurementRangeStruct::Type &) = 0;
    virtual CHIP_ERROR EndRangesRead()                                                   = 0;

    virtual CHIP_ERROR StartHarmonicCurrentsRead()                                                     = 0;
    virtual CHIP_ERROR GetHarmonicCurrentsByIndex(uint8_t, Structs::HarmonicMeasurementStruct::Type &) = 0;
    virtual CHIP_ERROR EndHarmonicCurrentsRead()                                                       = 0;

    virtual CHIP_ERROR StartHarmonicPhasesRead()                                                     = 0;
    virtual CHIP_ERROR GetHarmonicPhasesByIndex(uint8_t, Structs::HarmonicMeasurementStruct::Type &) = 0;
    virtual CHIP_ERROR EndHarmonicPhasesRead()                                                       = 0;

    virtual DataModel::Nullable<int64_t> GetVoltage()         = 0;
    virtual DataModel::Nullable<int64_t> GetActiveCurrent()   = 0;
    virtual DataModel::Nullable<int64_t> GetReactiveCurrent() = 0;
    virtual DataModel::Nullable<int64_t> GetApparentCurrent() = 0;
    virtual DataModel::Nullable<int64_t> GetActivePower()     = 0;
    virtual DataModel::Nullable<int64_t> GetReactivePower()   = 0;
    virtual DataModel::Nullable<int64_t> GetApparentPower()   = 0;
    virtual DataModel::Nullable<int64_t> GetRMSVoltage()      = 0;
    virtual DataModel::Nullable<int64_t> GetRMSCurrent()      = 0;
    virtual DataModel::Nullable<int64_t> GetRMSPower()        = 0;
    virtual DataModel::Nullable<int64_t> GetFrequency()       = 0;
    virtual DataModel::Nullable<int64_t> GetPowerFactor()     = 0;
    virtual DataModel::Nullable<int64_t> GetNeutralCurrent()  = 0;

protected:
    EndpointId mEndpointId = 0;
};

} // namespace ElectricalPowerMeasurement
} // namespace Clusters
} // namespace app
} // namespace chip
