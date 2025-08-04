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

#include <ElectricalPowerMeasurementDelegate.h>
#include <app/reporting/reporting.h>

#include <app/clusters/electrical-power-measurement-server/electrical-power-measurement-server.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::DataModel;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::ElectricalPowerMeasurement;
using namespace chip::app::Clusters::ElectricalPowerMeasurement::Attributes;
using namespace chip::app::Clusters::ElectricalPowerMeasurement::Structs;

CHIP_ERROR ElectricalPowerMeasurementInstance::Init()
{
    return Instance::Init();
}

void ElectricalPowerMeasurementInstance::Shutdown()
{
    Instance::Shutdown();
}

// --------------- Internal Attribute Set APIs
CHIP_ERROR ElectricalPowerMeasurementDelegate::SetPowerMode(PowerModeEnum newValue)
{
    PowerModeEnum oldValue = mPowerMode;

    if (EnsureKnownEnumValue(newValue) == PowerModeEnum::kUnknownEnumValue)
    {
        return CHIP_IM_GLOBAL_STATUS(ConstraintError);
    }

    mPowerMode = newValue;
    if (oldValue != newValue)
    {
        ChipLogDetail(AppServer, "mPowerMode updated to %d", static_cast<int>(mPowerMode));
        MatterReportingAttributeChangeCallback(mEndpointId, ElectricalPowerMeasurement::Id, PowerMode::Id);
    }

    return CHIP_NO_ERROR;
}

const MeasurementAccuracyRangeStruct::Type activePowerAccuracyRanges[] = {
    // 2 - 5%, 3% Typ
    {
        .rangeMin       = -50'000'000, // -50kW
        .rangeMax       = -10'000'000, // -10kW
        .percentMax     = MakeOptional(static_cast<chip::Percent100ths>(5000)),
        .percentMin     = MakeOptional(static_cast<chip::Percent100ths>(2000)),
        .percentTypical = MakeOptional(static_cast<chip::Percent100ths>(3000)),
    },
    // 0.1 - 1%, 0.5% Typ
    {
        .rangeMin       = -9'999'999, // -9.999kW
        .rangeMax       = 9'999'999,  //  9.999kW
        .percentMax     = MakeOptional(static_cast<chip::Percent100ths>(1000)),
        .percentMin     = MakeOptional(static_cast<chip::Percent100ths>(100)),
        .percentTypical = MakeOptional(static_cast<chip::Percent100ths>(500)),
    },
    // 2 - 5%, 3% Typ
    {
        .rangeMin       = 10'000'000, // 10 kW
        .rangeMax       = 50'000'000, // 50 kW
        .percentMax     = MakeOptional(static_cast<chip::Percent100ths>(5000)),
        .percentMin     = MakeOptional(static_cast<chip::Percent100ths>(2000)),
        .percentTypical = MakeOptional(static_cast<chip::Percent100ths>(3000)),
    },
};

const MeasurementAccuracyRangeStruct::Type activeCurrentAccuracyRanges[] = {
    // 2 - 5%, 3% Typ
    {
        .rangeMin       = -100'000, // -100A
        .rangeMax       = -5'000,   // -5A
        .percentMax     = MakeOptional(static_cast<chip::Percent100ths>(5000)),
        .percentMin     = MakeOptional(static_cast<chip::Percent100ths>(2000)),
        .percentTypical = MakeOptional(static_cast<chip::Percent100ths>(3000)),
    },
    // 0.1 - 1%, 0.5% Typ
    {
        .rangeMin       = -4'999, // -4.999A
        .rangeMax       = 4'999,  //  4.999A
        .percentMax     = MakeOptional(static_cast<chip::Percent100ths>(1000)),
        .percentMin     = MakeOptional(static_cast<chip::Percent100ths>(100)),
        .percentTypical = MakeOptional(static_cast<chip::Percent100ths>(500)),
    },
    // 2 - 5%, 3% Typ
    {
        .rangeMin       = 5'000,   // 5A
        .rangeMax       = 100'000, // 100 A
        .percentMax     = MakeOptional(static_cast<chip::Percent100ths>(5000)),
        .percentMin     = MakeOptional(static_cast<chip::Percent100ths>(2000)),
        .percentTypical = MakeOptional(static_cast<chip::Percent100ths>(3000)),
    },
};

const MeasurementAccuracyRangeStruct::Type voltageAccuracyRanges[] = {
    // 2 - 5%, 3% Typ
    {
        .rangeMin       = -500'000, // -500V
        .rangeMax       = -100'000, // -100V
        .percentMax     = MakeOptional(static_cast<chip::Percent100ths>(5000)),
        .percentMin     = MakeOptional(static_cast<chip::Percent100ths>(2000)),
        .percentTypical = MakeOptional(static_cast<chip::Percent100ths>(3000)),
    },
    // 0.1 - 1%, 0.5% Typ
    {
        .rangeMin       = -99'999, // -99.999V
        .rangeMax       = 99'999,  //  99.999V
        .percentMax     = MakeOptional(static_cast<chip::Percent100ths>(1000)),
        .percentMin     = MakeOptional(static_cast<chip::Percent100ths>(100)),
        .percentTypical = MakeOptional(static_cast<chip::Percent100ths>(500)),
    },
    // 2 - 5%, 3% Typ
    {
        .rangeMin       = 100'000, // 100 V
        .rangeMax       = 500'000, // 500 V
        .percentMax     = MakeOptional(static_cast<chip::Percent100ths>(5000)),
        .percentMin     = MakeOptional(static_cast<chip::Percent100ths>(2000)),
        .percentTypical = MakeOptional(static_cast<chip::Percent100ths>(3000)),
    }
};

static const Structs::MeasurementAccuracyStruct::Type kMeasurementAccuracies[] = {
    {
        .measurementType  = MeasurementTypeEnum::kActivePower,
        .measured         = true,
        .minMeasuredValue = -50'000'000, // -50 kW
        .maxMeasuredValue = 50'000'000,  //  50 kW
        .accuracyRanges   = DataModel::List<const MeasurementAccuracyRangeStruct::Type>(activePowerAccuracyRanges),
    },
    {
        .measurementType  = MeasurementTypeEnum::kActiveCurrent,
        .measured         = true,
        .minMeasuredValue = -100'000, // -100A
        .maxMeasuredValue = 100'000,  //  100A
        .accuracyRanges   = DataModel::List<const MeasurementAccuracyRangeStruct::Type>(activeCurrentAccuracyRanges),
    },
    {
        .measurementType  = MeasurementTypeEnum::kVoltage,
        .measured         = true,
        .minMeasuredValue = -500'000, // -500V
        .maxMeasuredValue = 500'000,  //  500V
        .accuracyRanges   = DataModel::List<const MeasurementAccuracyRangeStruct::Type>(voltageAccuracyRanges),
    },
};

uint8_t ElectricalPowerMeasurementDelegate::GetNumberOfMeasurementTypes()
{
    return MATTER_ARRAY_SIZE(kMeasurementAccuracies);
};

/* @brief This function is called by the cluster server at the start of read cycle
 *        This could take a semaphore to stop a background update of the data
 */
CHIP_ERROR ElectricalPowerMeasurementDelegate::StartAccuracyRead()
{
    /* Since we have a static array we don't need to do anything here */
    return CHIP_NO_ERROR;
}

CHIP_ERROR ElectricalPowerMeasurementDelegate::GetAccuracyByIndex(uint8_t accuracyIndex,
                                                                  Structs::MeasurementAccuracyStruct::Type & accuracy)
{
    if (accuracyIndex >= MATTER_ARRAY_SIZE(kMeasurementAccuracies))
    {
        return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
    }

    accuracy = kMeasurementAccuracies[accuracyIndex];

    return CHIP_NO_ERROR;
}

/* @brief This function is called by the cluster server at the end of read cycle
 *        This could release a semaphore to allow a background update of the data
 */
CHIP_ERROR ElectricalPowerMeasurementDelegate::EndAccuracyRead()
{
    /* Since we have a static array we don't need to do anything here */
    return CHIP_NO_ERROR;
}

/* @brief This function is called by the cluster server at the start of read cycle
 *        This could take a semaphore to stop a background update of the data
 */
CHIP_ERROR ElectricalPowerMeasurementDelegate::StartRangesRead()
{
    /* Since we don't an implementation here we don't need to do anything here */
    return CHIP_NO_ERROR;
}

CHIP_ERROR ElectricalPowerMeasurementDelegate::GetRangeByIndex(uint8_t rangeIndex, Structs::MeasurementRangeStruct::Type & range)
{
    /** TODO - Manufacturers wanting to support this should
     * implement an array of
     * Structs::MeasurementRangeStruct::Type mMeasurementRanges[];
     *
     * their application code should update the relevant measurement 'Range' information including
     *   - .measurementType
     *   - .min
     *   - .max
     *   - .startTimestamp
     *   - .endTimestamp
     *   - .minTimestamp (the time at which the minimum value was recorded)
     *   - .maxTimestamp (the time at which the maximum value was recorded)
     *   (and optionally use sys time equivalents)
     *
     *   if (rangeIndex >= MATTER_ARRAY_SIZE(mMeasurementRanges))
     *   {
     *       return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
     *   }
     *
     *   range = mMeasurementRanges[rangeIndex];
     *
     *   return CHIP_NO_ERROR;
     */

    /* Return an empty list for now */
    return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
}

/* @brief This function is called by the cluster server at the end of read cycle
 *        This could release a semaphore to allow a background update of the data
 */
CHIP_ERROR ElectricalPowerMeasurementDelegate::EndRangesRead()
{
    /* Since we don't an implementation here we don't need to do anything here */
    return CHIP_NO_ERROR;
}

static const Structs::HarmonicMeasurementStruct::Type kHarmonicCurrentMeasurements[] = {
    { .order = 1, .measurement = MakeNullable(static_cast<int64_t>(100000)) }
};

/* @brief This function is called by the cluster server at the start of read cycle
 *        This could take a semaphore to stop a background update of the data
 */
CHIP_ERROR
ElectricalPowerMeasurementDelegate::StartHarmonicCurrentsRead()
{
    /* Since we have a static array we don't need to do anything here */
    return CHIP_NO_ERROR;
}
CHIP_ERROR
ElectricalPowerMeasurementDelegate::GetHarmonicCurrentsByIndex(uint8_t harmonicCurrentsIndex,
                                                               Structs::HarmonicMeasurementStruct::Type & harmonicCurrent)
{
    /** TODO - Manufacturers wanting to support this could implement an array of
     * Structs::HarmonicMeasurementStruct::Type mHarmonicCurrentMeasurements[];
     *
     * The application code should update the relevant harmonic 'order' information including
     *   - .order
     *   - .measurement
     *
     * The application should also ensure it notifies remote clients that the value has changed
     *  MatterReportingAttributeChangeCallback(mEndpointId, ElectricalPowerMeasurement::Id, HarmonicCurrents::Id);
     */

    /* Added to support testing using a static array for now */
    if (harmonicCurrentsIndex >= MATTER_ARRAY_SIZE(kHarmonicCurrentMeasurements))
    {
        return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
    }

    harmonicCurrent = kHarmonicCurrentMeasurements[harmonicCurrentsIndex];
    return CHIP_NO_ERROR;
}
/* @brief This function is called by the cluster server at the end of read cycle
 *        This could release a semaphore to allow a background update of the data
 */
CHIP_ERROR ElectricalPowerMeasurementDelegate::EndHarmonicCurrentsRead()
{
    /* Since we have a static array we don't need to do anything here */
    return CHIP_NO_ERROR;
}

static const Structs::HarmonicMeasurementStruct::Type kHarmonicPhaseMeasurements[] = {
    { .order = 1, .measurement = MakeNullable(static_cast<int64_t>(100000)) }
};

/* @brief This function is called by the cluster server at the start of read cycle
 *        This could take a semaphore to stop a background update of the data
 */
CHIP_ERROR ElectricalPowerMeasurementDelegate::StartHarmonicPhasesRead()
{
    /* Since we have a static array we don't need to do anything here */
    return CHIP_NO_ERROR;
}

CHIP_ERROR ElectricalPowerMeasurementDelegate::GetHarmonicPhasesByIndex(uint8_t harmonicPhaseIndex,
                                                                        Structs::HarmonicMeasurementStruct::Type & harmonicPhase)
{
    /** TODO - Manufacturers wanting to support this could implement an array of
     * Structs::HarmonicMeasurementStruct::Type mHarmonicPhaseMeasurements[];
     *
     * The application code should update the relevant harmonic 'order' information including
     *   - .order
     *   - .measurement
     *
     * The application should also ensure it notifies remote clients that the value has changed
     *  MatterReportingAttributeChangeCallback(mEndpointId, ElectricalPowerMeasurement::Id, HarmonicPhases::Id);
     */

    /* Added to support testing using a static array for now */
    if (harmonicPhaseIndex >= MATTER_ARRAY_SIZE(kHarmonicPhaseMeasurements))
    {
        return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
    }

    harmonicPhase = kHarmonicPhaseMeasurements[harmonicPhaseIndex];
    return CHIP_NO_ERROR;
}
/* @brief This function is called by the cluster server at the end of read cycle
 *        This could release a semaphore to allow a background update of the data
 */
CHIP_ERROR ElectricalPowerMeasurementDelegate::EndHarmonicPhasesRead()
{
    /* Since we have a static array we don't need to do anything here */
    return CHIP_NO_ERROR;
}

CHIP_ERROR ElectricalPowerMeasurementDelegate::SetVoltage(DataModel::Nullable<int64_t> newValue)
{
    DataModel::Nullable<int64_t> oldValue = mVoltage;

    mVoltage = newValue;
    if (oldValue != newValue)
    {
        // We won't log raw values since these could change frequently
        MatterReportingAttributeChangeCallback(mEndpointId, ElectricalPowerMeasurement::Id, Voltage::Id);
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR ElectricalPowerMeasurementDelegate::SetActiveCurrent(DataModel::Nullable<int64_t> newValue)
{
    DataModel::Nullable<int64_t> oldValue = mActiveCurrent;

    mActiveCurrent = newValue;
    if (oldValue != newValue)
    {
        // We won't log raw values since these could change frequently
        MatterReportingAttributeChangeCallback(mEndpointId, ElectricalPowerMeasurement::Id, ActiveCurrent::Id);
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR ElectricalPowerMeasurementDelegate::SetReactiveCurrent(DataModel::Nullable<int64_t> newValue)
{
    DataModel::Nullable<int64_t> oldValue = mReactiveCurrent;

    mReactiveCurrent = newValue;
    if (oldValue != newValue)
    {
        // We won't log raw values since these could change frequently
        MatterReportingAttributeChangeCallback(mEndpointId, ElectricalPowerMeasurement::Id, ReactiveCurrent::Id);
    }

    return CHIP_NO_ERROR;
}
CHIP_ERROR ElectricalPowerMeasurementDelegate::SetApparentCurrent(DataModel::Nullable<int64_t> newValue)
{
    DataModel::Nullable<int64_t> oldValue = mApparentCurrent;

    mApparentCurrent = newValue;
    if (oldValue != newValue)
    {
        // We won't log raw values since these could change frequently
        MatterReportingAttributeChangeCallback(mEndpointId, ElectricalPowerMeasurement::Id, ApparentCurrent::Id);
    }

    return CHIP_NO_ERROR;
}
CHIP_ERROR ElectricalPowerMeasurementDelegate::SetActivePower(DataModel::Nullable<int64_t> newValue)
{
    DataModel::Nullable<int64_t> oldValue = mActivePower;

    mActivePower = newValue;
    if (oldValue != newValue)
    {
        // We won't log raw values since these could change frequently
        MatterReportingAttributeChangeCallback(mEndpointId, ElectricalPowerMeasurement::Id, ActivePower::Id);
    }

    return CHIP_NO_ERROR;
}
CHIP_ERROR ElectricalPowerMeasurementDelegate::SetReactivePower(DataModel::Nullable<int64_t> newValue)
{
    DataModel::Nullable<int64_t> oldValue = mReactivePower;

    mReactivePower = newValue;
    if (oldValue != newValue)
    {
        // We won't log raw values since these could change frequently
        MatterReportingAttributeChangeCallback(mEndpointId, ElectricalPowerMeasurement::Id, ReactivePower::Id);
    }

    return CHIP_NO_ERROR;
}
CHIP_ERROR ElectricalPowerMeasurementDelegate::SetApparentPower(DataModel::Nullable<int64_t> newValue)
{
    DataModel::Nullable<int64_t> oldValue = mApparentPower;

    mApparentPower = newValue;
    if (oldValue != newValue)
    {
        // We won't log raw values since these could change frequently
        MatterReportingAttributeChangeCallback(mEndpointId, ElectricalPowerMeasurement::Id, ApparentPower::Id);
    }

    return CHIP_NO_ERROR;
}
CHIP_ERROR ElectricalPowerMeasurementDelegate::SetRMSVoltage(DataModel::Nullable<int64_t> newValue)
{
    DataModel::Nullable<int64_t> oldValue = mRMSVoltage;

    mRMSVoltage = newValue;
    if (oldValue != newValue)
    {
        // We won't log raw values since these could change frequently
        MatterReportingAttributeChangeCallback(mEndpointId, ElectricalPowerMeasurement::Id, RMSVoltage::Id);
    }

    return CHIP_NO_ERROR;
}
CHIP_ERROR ElectricalPowerMeasurementDelegate::SetRMSCurrent(DataModel::Nullable<int64_t> newValue)
{
    DataModel::Nullable<int64_t> oldValue = mRMSCurrent;

    mRMSCurrent = newValue;
    if (oldValue != newValue)
    {
        // We won't log raw values since these could change frequently
        MatterReportingAttributeChangeCallback(mEndpointId, ElectricalPowerMeasurement::Id, RMSCurrent::Id);
    }

    return CHIP_NO_ERROR;
}
CHIP_ERROR ElectricalPowerMeasurementDelegate::SetRMSPower(DataModel::Nullable<int64_t> newValue)
{
    DataModel::Nullable<int64_t> oldValue = mRMSPower;

    mRMSPower = newValue;
    if (oldValue != newValue)
    {
        // We won't log raw values since these could change frequently
        MatterReportingAttributeChangeCallback(mEndpointId, ElectricalPowerMeasurement::Id, RMSPower::Id);
    }

    return CHIP_NO_ERROR;
}
CHIP_ERROR ElectricalPowerMeasurementDelegate::SetFrequency(DataModel::Nullable<int64_t> newValue)
{
    DataModel::Nullable<int64_t> oldValue = mFrequency;

    mFrequency = newValue;
    if (oldValue != newValue)
    {
        // We won't log raw values since these could change frequently
        MatterReportingAttributeChangeCallback(mEndpointId, ElectricalPowerMeasurement::Id, Frequency::Id);
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR ElectricalPowerMeasurementDelegate::SetPowerFactor(DataModel::Nullable<int64_t> newValue)
{
    DataModel::Nullable<int64_t> oldValue = mPowerFactor;

    mPowerFactor = newValue;
    if (oldValue != newValue)
    {
        // We won't log raw values since these could change frequently
        MatterReportingAttributeChangeCallback(mEndpointId, ElectricalPowerMeasurement::Id, PowerFactor::Id);
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR ElectricalPowerMeasurementDelegate::SetNeutralCurrent(DataModel::Nullable<int64_t> newValue)
{
    DataModel::Nullable<int64_t> oldValue = mNeutralCurrent;

    mNeutralCurrent = newValue;
    if (oldValue != newValue)
    {
        // We won't log raw values since these could change frequently
        MatterReportingAttributeChangeCallback(mEndpointId, ElectricalPowerMeasurement::Id, NeutralCurrent::Id);
    }

    return CHIP_NO_ERROR;
}
