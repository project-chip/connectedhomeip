/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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

#include "ConcentrationMeasurementCluster.h"
#include <app/server-cluster/AttributeListBuilder.h>
#include <app/server-cluster/DefaultServerCluster.h>
#include <lib/core/CHIPError.h>
#include <lib/support/CodeUtils.h>

using namespace chip::app::Clusters::ConcentrationMeasurement::Attributes;
using chip::Protocols::InteractionModel::Status;

namespace chip {
namespace app {
namespace Clusters {
namespace ConcentrationMeasurement {

ConcentrationMeasurementCluster::ConcentrationMeasurementCluster(EndpointId endpointId, const Config & config) :
    DefaultServerCluster({ endpointId, config.clusterId }), mFeatures([&] {
        BitFlags<Feature> f = config.features;
        if (f.HasAny(Feature::kMediumLevel, Feature::kCriticalLevel))
            f.Set(Feature::kLevelIndication);
        if (f.HasAny(Feature::kPeakMeasurement, Feature::kAverageMeasurement))
            f.Set(Feature::kNumericMeasurement);
        return f;
    }()),
    mOptionalAttributeSet([&] {
        OptionalAttributeSet s;
        if (mFeatures.Has(Feature::kNumericMeasurement))
        {
            s.Set<Attributes::MeasuredValue::Id>();
            s.Set<Attributes::MinMeasuredValue::Id>();
            s.Set<Attributes::MaxMeasuredValue::Id>();
            s.Set<Attributes::Uncertainty::Id>();
            s.Set<Attributes::MeasurementUnit::Id>();
        }
        if (mFeatures.Has(Feature::kPeakMeasurement))
        {
            s.Set<Attributes::PeakMeasuredValue::Id>();
            s.Set<Attributes::PeakMeasuredValueWindow::Id>();
        }
        if (mFeatures.Has(Feature::kAverageMeasurement))
        {
            s.Set<Attributes::AverageMeasuredValue::Id>();
            s.Set<Attributes::AverageMeasuredValueWindow::Id>();
        }
        if (mFeatures.Has(Feature::kLevelIndication))
            s.Set<Attributes::LevelValue::Id>();
        return s;
    }()),
    mMedium(config.medium), mUnit(config.unit), mMinMeasuredValue(config.minMeasured), mMaxMeasuredValue(config.maxMeasured),
    mUncertainty(config.uncertainty)
{
    (void) mMedium;
    (void) mUnit;
    (void) mUncertainty;
    VerifyOrDie(std::find(AliasedClusters.begin(), AliasedClusters.end(), config.clusterId) !=
                AliasedClusters.end()); // NOLINT(bugprone-signed-bitwise)
}

DataModel::ActionReturnStatus ConcentrationMeasurementCluster::ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                                             AttributeValueEncoder & encoder)
{
    struct NullableFloatAttr
    {
        AttributeId id;
        const DataModel::Nullable<float> & value;
    };
    const NullableFloatAttr kNullableFloatAttrs[] = {
        { MeasuredValue::Id, mMeasuredValue },
        { MinMeasuredValue::Id, mMinMeasuredValue },
        { MaxMeasuredValue::Id, mMaxMeasuredValue },
        { PeakMeasuredValue::Id, mPeakMeasuredValue },
        { AverageMeasuredValue::Id, mAverageMeasuredValue },
    };
    for (const auto & attr : kNullableFloatAttrs)
    {
        if (request.path.mAttributeId == attr.id)
            return encoder.Encode(attr.value);
    }

    switch (request.path.mAttributeId)
    {
    case MeasurementMedium::Id:
        return encoder.Encode(mMedium);

    case Attributes::FeatureMap::Id:
        return encoder.Encode(mFeatures);

    case Attributes::ClusterRevision::Id:
        return encoder.Encode(chip::app::Clusters::CarbonDioxideConcentrationMeasurement::kRevision);

    case Uncertainty::Id:
        return encoder.Encode(mUncertainty);

    case MeasurementUnit::Id:
        return encoder.Encode(mUnit);

    case PeakMeasuredValueWindow::Id:
        return encoder.Encode(mPeakMeasuredValueWindow);

    case AverageMeasuredValueWindow::Id:
        return encoder.Encode(mAverageMeasuredValueWindow);

    case LevelValue::Id:
        return encoder.Encode(mLevelValue);

    default:
        return Status::UnsupportedAttribute;
    }
}

CHIP_ERROR ConcentrationMeasurementCluster::Attributes(const ConcreteClusterPath & path,
                                                       ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder)
{
    static constexpr DataModel::AttributeEntry kMandatoryAttrs[] = {
        Attributes::MeasurementMedium::kMetadataEntry,
    };
    static constexpr DataModel::AttributeEntry kOptionalAttrs[] = {
        Attributes::MeasuredValue::kMetadataEntry,
        Attributes::MinMeasuredValue::kMetadataEntry,
        Attributes::MaxMeasuredValue::kMetadataEntry,
        Attributes::Uncertainty::kMetadataEntry,
        Attributes::MeasurementUnit::kMetadataEntry,
        Attributes::PeakMeasuredValue::kMetadataEntry,
        Attributes::PeakMeasuredValueWindow::kMetadataEntry,
        Attributes::AverageMeasuredValue::kMetadataEntry,
        Attributes::AverageMeasuredValueWindow::kMetadataEntry,
        Attributes::LevelValue::kMetadataEntry,
    };
    return AttributeListBuilder(builder).Append(chip::Span<const DataModel::AttributeEntry>(kMandatoryAttrs),
                                                chip::Span<const DataModel::AttributeEntry>(kOptionalAttrs), mOptionalAttributeSet);
}

CHIP_ERROR ConcentrationMeasurementCluster::SetMeasuredValue(DataModel::Nullable<float> value)
{
    VerifyOrReturnError(mFeatures.Has(Feature::kNumericMeasurement), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
    VerifyOrReturnError(IsInRange(value, mMinMeasuredValue, mMaxMeasuredValue), CHIP_IM_GLOBAL_STATUS(ConstraintError));
    SetAttributeValue(mMeasuredValue, value, MeasuredValue::Id);
    return CHIP_NO_ERROR;
}

CHIP_ERROR ConcentrationMeasurementCluster::SetPeakMeasuredValue(DataModel::Nullable<float> value)
{
    VerifyOrReturnError(mFeatures.Has(Feature::kPeakMeasurement), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
    VerifyOrReturnError(IsInRange(value, mMinMeasuredValue, mMaxMeasuredValue), CHIP_IM_GLOBAL_STATUS(ConstraintError));
    SetAttributeValue(mPeakMeasuredValue, value, PeakMeasuredValue::Id);
    return CHIP_NO_ERROR;
}

CHIP_ERROR ConcentrationMeasurementCluster::SetPeakMeasuredValueWindow(uint32_t value)
{
    VerifyOrReturnError(mFeatures.Has(Feature::kPeakMeasurement), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
    VerifyOrReturnError(value <= kWindowMaxSeconds, CHIP_IM_GLOBAL_STATUS(ConstraintError));
    SetAttributeValue(mPeakMeasuredValueWindow, value, PeakMeasuredValueWindow::Id);
    return CHIP_NO_ERROR;
}

CHIP_ERROR ConcentrationMeasurementCluster::SetAverageMeasuredValue(DataModel::Nullable<float> value)
{
    VerifyOrReturnError(mFeatures.Has(Feature::kAverageMeasurement), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
    VerifyOrReturnError(IsInRange(value, mMinMeasuredValue, mMaxMeasuredValue), CHIP_IM_GLOBAL_STATUS(ConstraintError));
    SetAttributeValue(mAverageMeasuredValue, value, AverageMeasuredValue::Id);
    return CHIP_NO_ERROR;
}

CHIP_ERROR ConcentrationMeasurementCluster::SetAverageMeasuredValueWindow(uint32_t value)
{
    VerifyOrReturnError(mFeatures.Has(Feature::kAverageMeasurement), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
    VerifyOrReturnError(value <= kWindowMaxSeconds, CHIP_IM_GLOBAL_STATUS(ConstraintError));
    SetAttributeValue(mAverageMeasuredValueWindow, value, AverageMeasuredValueWindow::Id);
    return CHIP_NO_ERROR;
}

CHIP_ERROR ConcentrationMeasurementCluster::SetLevelValue(LevelValueEnum value)
{
    VerifyOrReturnError(mFeatures.Has(Feature::kLevelIndication), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
    VerifyOrReturnError(value < LevelValueEnum::kUnknownEnumValue, CHIP_IM_GLOBAL_STATUS(ConstraintError));
    if ((value == LevelValueEnum::kMedium) && !mFeatures.Has(Feature::kMediumLevel))
    {
        return CHIP_IM_GLOBAL_STATUS(ConstraintError);
    }
    if ((value == LevelValueEnum::kCritical) && !mFeatures.Has(Feature::kCriticalLevel))
    {
        return CHIP_IM_GLOBAL_STATUS(ConstraintError);
    }
    SetAttributeValue(mLevelValue, value, LevelValue::Id);

    return CHIP_NO_ERROR;
}

} // namespace ConcentrationMeasurement
} // namespace Clusters
} // namespace app
} // namespace chip
