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
    VerifyOrDie(std::find(AliasedClusters.begin(), AliasedClusters.end(), config.clusterId) !=
                AliasedClusters.end()); // NOLINT(bugprone-signed-bitwise)
}

namespace {

CHIP_ERROR EncodeNullableFloat(AttributeValueEncoder & encoder, const char * member)
{
    return encoder.Encode(*reinterpret_cast<const DataModel::Nullable<float> *>(member));
}

CHIP_ERROR EncodeFloat(AttributeValueEncoder & encoder, const char * member)
{
    return encoder.Encode(*reinterpret_cast<const float *>(member));
}

CHIP_ERROR EncodeMediumEnum(AttributeValueEncoder & encoder, const char * member)
{
    return encoder.Encode(*reinterpret_cast<const MeasurementMediumEnum *>(member));
}

CHIP_ERROR EncodeUnitEnum(AttributeValueEncoder & encoder, const char * member)
{
    return encoder.Encode(*reinterpret_cast<const MeasurementUnitEnum *>(member));
}

CHIP_ERROR EncodeLevelEnum(AttributeValueEncoder & encoder, const char * member)
{
    return encoder.Encode(*reinterpret_cast<const LevelValueEnum *>(member));
}

} // namespace

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Winvalid-offsetof"
const ConcentrationMeasurementCluster::AttrDispatch ConcentrationMeasurementCluster::kDispatchTable[] = {
    { MeasuredValue::Id, offsetof(ConcentrationMeasurementCluster, mMeasuredValue), EncodeNullableFloat },
    { MinMeasuredValue::Id, offsetof(ConcentrationMeasurementCluster, mMinMeasuredValue), EncodeNullableFloat },
    { MaxMeasuredValue::Id, offsetof(ConcentrationMeasurementCluster, mMaxMeasuredValue), EncodeNullableFloat },
    { PeakMeasuredValue::Id, offsetof(ConcentrationMeasurementCluster, mPeakMeasuredValue), EncodeNullableFloat },
    { AverageMeasuredValue::Id, offsetof(ConcentrationMeasurementCluster, mAverageMeasuredValue), EncodeNullableFloat },

    { MeasurementMedium::Id, offsetof(ConcentrationMeasurementCluster, mMedium), EncodeMediumEnum },
    { MeasurementUnit::Id, offsetof(ConcentrationMeasurementCluster, mUnit), EncodeUnitEnum },
    { Uncertainty::Id, offsetof(ConcentrationMeasurementCluster, mUncertainty), EncodeFloat },
    { LevelValue::Id, offsetof(ConcentrationMeasurementCluster, mLevelValue), EncodeLevelEnum },
};
#pragma GCC diagnostic pop

DataModel::ActionReturnStatus ConcentrationMeasurementCluster::ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                                             AttributeValueEncoder & encoder)
{
    const AttributeId id = request.path.mAttributeId;

    for (const auto & entry : kDispatchTable)
    {
        if (entry.id == id)
            return entry.encode(encoder, reinterpret_cast<const char *>(this) + entry.offset);
    }

    return Status::UnsupportedAttribute;
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

CHIP_ERROR ConcentrationMeasurementCluster::SetNullableFloat(Feature feature, DataModel::Nullable<float> & field, AttributeId id,
                                                             DataModel::Nullable<float> value)
{
    VerifyOrReturnError(mFeatures.Has(feature), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
    VerifyOrReturnError(IsInRange(value, mMinMeasuredValue, mMaxMeasuredValue), CHIP_IM_GLOBAL_STATUS(ConstraintError));
    SetAttributeValue(field, value, id);
    return CHIP_NO_ERROR;
}

CHIP_ERROR ConcentrationMeasurementCluster::SetWindow(Feature feature, uint32_t & field, AttributeId id, uint32_t value)
{
    VerifyOrReturnError(mFeatures.Has(feature), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
    VerifyOrReturnError(value <= kWindowMaxSeconds, CHIP_IM_GLOBAL_STATUS(ConstraintError));
    SetAttributeValue(field, value, id);
    return CHIP_NO_ERROR;
}

CHIP_ERROR ConcentrationMeasurementCluster::SetMeasuredValue(DataModel::Nullable<float> value)
{
    return SetNullableFloat(Feature::kNumericMeasurement, mMeasuredValue, MeasuredValue::Id, value);
}

CHIP_ERROR ConcentrationMeasurementCluster::SetPeakMeasuredValue(DataModel::Nullable<float> value)
{
    return SetNullableFloat(Feature::kPeakMeasurement, mPeakMeasuredValue, PeakMeasuredValue::Id, value);
}

CHIP_ERROR ConcentrationMeasurementCluster::SetPeakMeasuredValueWindow(uint32_t value)
{
    return SetWindow(Feature::kPeakMeasurement, mPeakMeasuredValueWindow, PeakMeasuredValueWindow::Id, value);
}

CHIP_ERROR ConcentrationMeasurementCluster::SetAverageMeasuredValue(DataModel::Nullable<float> value)
{
    return SetNullableFloat(Feature::kAverageMeasurement, mAverageMeasuredValue, AverageMeasuredValue::Id, value);
}

CHIP_ERROR ConcentrationMeasurementCluster::SetAverageMeasuredValueWindow(uint32_t value)
{
    return SetWindow(Feature::kAverageMeasurement, mAverageMeasuredValueWindow, AverageMeasuredValueWindow::Id, value);
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
