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

#include <app/clusters/electrical-power-measurement-server/electrical-power-measurement-server.h>

using namespace chip;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::ElectricalPowerMeasurement;
using namespace chip::app::Clusters::ElectricalPowerMeasurement::Structs;

namespace chip {
namespace app {
namespace Clusters {
namespace ElectricalPowerMeasurement {

static MeasurementAccuracyRangeStruct::Type activeCurrentAccuracyRanges[] = { { .rangeMin = 500, .rangeMax = 1000 } };

class StubAccuracyIterator : public Delegate::AccuracyIterator
{
public:
    size_t Count() override;
    bool Next(MeasurementAccuracyStruct::Type & output) override;
    void Release() override;

private:
    uint8_t mIndex;
};

size_t StubAccuracyIterator::Count()
{
    return 1;
}

bool StubAccuracyIterator::Next(MeasurementAccuracyStruct::Type & output)
{
    if (mIndex >= 1)
    {
        return false;
    }
    output.measurementType  = MeasurementTypeEnum::kActiveCurrent;
    output.measured         = true;
    output.minMeasuredValue = -10000000;
    output.maxMeasuredValue = 10000000;
    output.accuracyRanges   = DataModel::List<const MeasurementAccuracyRangeStruct::Type>(activeCurrentAccuracyRanges);
    mIndex++;
    return true;
}

void StubAccuracyIterator::Release()
{
    mIndex = 0;
}

class StubRangeIterator : public Delegate::RangeIterator
{
public:
    size_t Count() override;
    bool Next(MeasurementRangeStruct::Type & output) override;
    void Release() override;
};

size_t StubRangeIterator::Count()
{
    return 0;
}

bool StubRangeIterator::Next(MeasurementRangeStruct::Type & output)
{
    return false;
}

void StubRangeIterator::Release() {}

class StubHarmonicMeasurementIterator : public Delegate::HarmonicMeasurementIterator
{
public:
    size_t Count() override;
    bool Next(HarmonicMeasurementStruct::Type & output) override;
    void Release() override;
};

size_t StubHarmonicMeasurementIterator::Count()
{
    return 0;
}

bool StubHarmonicMeasurementIterator::Next(HarmonicMeasurementStruct::Type & output)
{
    return false;
}

void StubHarmonicMeasurementIterator::Release() {}

static StubAccuracyIterator accuracyIterator;
static StubRangeIterator rangeIterator;
static StubHarmonicMeasurementIterator harmonicMeasurementIterator;

class ElectricalPowerMeasurementDelegate : public Delegate
{
public:
    PowerModeEnum GetPowerMode() override;
    uint8_t GetNumberOfMeasurementTypes() override;
    AccuracyIterator * IterateAccuracy() override;
    RangeIterator * IterateRanges() override;
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
    HarmonicMeasurementIterator * IterateHarmonicCurrents() override;
    HarmonicMeasurementIterator * IterateHarmonicPhases() override;
    DataModel::Nullable<int64_t> GetPowerFactor() override;
    DataModel::Nullable<int64_t> GetNeutralCurrent() override;

    ~ElectricalPowerMeasurementDelegate() = default;
};

PowerModeEnum ElectricalPowerMeasurementDelegate::GetPowerMode()
{
    return PowerModeEnum::kAc;
}

uint8_t ElectricalPowerMeasurementDelegate::GetNumberOfMeasurementTypes()
{
    return 1;
}

Delegate::AccuracyIterator * ElectricalPowerMeasurementDelegate::IterateAccuracy()
{
    return &accuracyIterator;
}

Delegate::RangeIterator * ElectricalPowerMeasurementDelegate::IterateRanges()
{
    return &rangeIterator;
}

DataModel::Nullable<int64_t> ElectricalPowerMeasurementDelegate::GetVoltage()
{
    return {};
}

DataModel::Nullable<int64_t> ElectricalPowerMeasurementDelegate::GetActiveCurrent()
{
    return {};
}

DataModel::Nullable<int64_t> ElectricalPowerMeasurementDelegate::GetReactiveCurrent()
{
    return {};
}

DataModel::Nullable<int64_t> ElectricalPowerMeasurementDelegate::GetApparentCurrent()
{
    return {};
}

DataModel::Nullable<int64_t> ElectricalPowerMeasurementDelegate::GetActivePower()
{
    return DataModel::Nullable<int64_t>(10000);
}

DataModel::Nullable<int64_t> ElectricalPowerMeasurementDelegate::GetReactivePower()
{
    return {};
}

DataModel::Nullable<int64_t> ElectricalPowerMeasurementDelegate::GetApparentPower()
{
    return {};
}

DataModel::Nullable<int64_t> ElectricalPowerMeasurementDelegate::GetRMSVoltage()
{
    return {};
}

DataModel::Nullable<int64_t> ElectricalPowerMeasurementDelegate::GetRMSCurrent()
{
    return {};
}

DataModel::Nullable<int64_t> ElectricalPowerMeasurementDelegate::GetRMSPower()
{
    return {};
}

DataModel::Nullable<int64_t> ElectricalPowerMeasurementDelegate::GetFrequency()
{
    return {};
}

Delegate::HarmonicMeasurementIterator * ElectricalPowerMeasurementDelegate::IterateHarmonicCurrents()
{
    return &harmonicMeasurementIterator;
}

Delegate::HarmonicMeasurementIterator * ElectricalPowerMeasurementDelegate::IterateHarmonicPhases()
{
    return &harmonicMeasurementIterator;
}

DataModel::Nullable<int64_t> ElectricalPowerMeasurementDelegate::GetPowerFactor()
{
    return {};
}

DataModel::Nullable<int64_t> ElectricalPowerMeasurementDelegate::GetNeutralCurrent()
{
    return {};
}

} // namespace ElectricalPowerMeasurement
} // namespace Clusters
} // namespace app
} // namespace chip

static std::unique_ptr<ElectricalPowerMeasurement::Delegate> gDelegate;
static std::unique_ptr<ElectricalPowerMeasurement::Instance> gInstance;

void emberAfElectricalPowerMeasurementClusterInitCallback(chip::EndpointId endpointId)
{
    VerifyOrDie(endpointId == 1); // this cluster is only enabled for endpoint 1.
    VerifyOrDie(!gInstance);

    gDelegate = std::make_unique<ElectricalPowerMeasurementDelegate>();
    if (gDelegate)
    {
        gInstance = std::make_unique<Instance>(
            endpointId, *gDelegate,
            BitMask<Feature, uint32_t>(Feature::kDirectCurrent, Feature::kAlternatingCurrent, Feature::kPolyphasePower,
                                       Feature::kHarmonics, Feature::kPowerQuality),
            BitMask<OptionalAttributes, uint32_t>(
                OptionalAttributes::kOptionalAttributeRanges, OptionalAttributes::kOptionalAttributeVoltage,
                OptionalAttributes::kOptionalAttributeActiveCurrent, OptionalAttributes::kOptionalAttributeReactiveCurrent,
                OptionalAttributes::kOptionalAttributeApparentCurrent, OptionalAttributes::kOptionalAttributeReactivePower,
                OptionalAttributes::kOptionalAttributeApparentPower, OptionalAttributes::kOptionalAttributeRMSVoltage,
                OptionalAttributes::kOptionalAttributeRMSCurrent, OptionalAttributes::kOptionalAttributeRMSPower,
                OptionalAttributes::kOptionalAttributeFrequency, OptionalAttributes::kOptionalAttributePowerFactor,
                OptionalAttributes::kOptionalAttributeNeutralCurrent));

        gInstance->Init();
    }
}
