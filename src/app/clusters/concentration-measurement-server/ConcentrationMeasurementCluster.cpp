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
#include <clusters/CarbonDioxideConcentrationMeasurement/Metadata.h>
#include <clusters/CarbonMonoxideConcentrationMeasurement/Metadata.h>
#include <clusters/FormaldehydeConcentrationMeasurement/Metadata.h>
#include <clusters/NitrogenDioxideConcentrationMeasurement/Metadata.h>
#include <clusters/OzoneConcentrationMeasurement/Metadata.h>
#include <clusters/Pm10ConcentrationMeasurement/Metadata.h>
#include <clusters/Pm1ConcentrationMeasurement/Metadata.h>
#include <clusters/Pm25ConcentrationMeasurement/Metadata.h>
#include <clusters/RadonConcentrationMeasurement/Metadata.h>
#include <clusters/TotalVolatileOrganicCompoundsConcentrationMeasurement/Metadata.h>

static_assert(chip::app::Clusters::CarbonDioxideConcentrationMeasurement::kRevision ==
              chip::app::Clusters::CarbonMonoxideConcentrationMeasurement::kRevision);
static_assert(chip::app::Clusters::CarbonDioxideConcentrationMeasurement::kRevision ==
              chip::app::Clusters::FormaldehydeConcentrationMeasurement::kRevision);
static_assert(chip::app::Clusters::CarbonDioxideConcentrationMeasurement::kRevision ==
              chip::app::Clusters::NitrogenDioxideConcentrationMeasurement::kRevision);
static_assert(chip::app::Clusters::CarbonDioxideConcentrationMeasurement::kRevision ==
              chip::app::Clusters::OzoneConcentrationMeasurement::kRevision);
static_assert(chip::app::Clusters::CarbonDioxideConcentrationMeasurement::kRevision ==
              chip::app::Clusters::Pm10ConcentrationMeasurement::kRevision);
static_assert(chip::app::Clusters::CarbonDioxideConcentrationMeasurement::kRevision ==
              chip::app::Clusters::Pm1ConcentrationMeasurement::kRevision);
static_assert(chip::app::Clusters::CarbonDioxideConcentrationMeasurement::kRevision ==
              chip::app::Clusters::Pm25ConcentrationMeasurement::kRevision);
static_assert(chip::app::Clusters::CarbonDioxideConcentrationMeasurement::kRevision ==
              chip::app::Clusters::RadonConcentrationMeasurement::kRevision);
static_assert(chip::app::Clusters::CarbonDioxideConcentrationMeasurement::kRevision ==
              chip::app::Clusters::TotalVolatileOrganicCompoundsConcentrationMeasurement::kRevision);

using namespace chip::app::Clusters::ConcentrationMeasurement::Attributes;
using chip::Protocols::InteractionModel::Status;

namespace {

bool IsInRange(const chip::app::DataModel::Nullable<float> & value, const chip::app::DataModel::Nullable<float> & minV,
               const chip::app::DataModel::Nullable<float> & maxV)
{
    VerifyOrReturnValue(!value.IsNull(), true);
    VerifyOrReturnValue(!minV.IsNull() && value.Value() > minV.Value(), false);
    VerifyOrReturnValue(!maxV.IsNull() && value.Value() < maxV.Value(), false);
    return true;
}

} // namespace

namespace chip {
namespace app {
namespace Clusters {
namespace ConcentrationMeasurement {

ConcentrationMeasurementCluster::ConcentrationMeasurementCluster(EndpointId endpointId, const Config & config) :
    DefaultServerCluster({ endpointId, config.clusterId }), mFeatures([&] {
        BitFlags<Feature> f = config.features;
        if (f.HasAny(Feature::kMediumLevel, Feature::kCriticalLevel))
        {
            f.Set(Feature::kLevelIndication);
        }
        if (f.HasAny(Feature::kPeakMeasurement, Feature::kAverageMeasurement))
        {
            f.Set(Feature::kNumericMeasurement);
        }
        return f;
    }()),
    mOptionalAttributeSet([&] {
        OptionalAttributeSet optionalAttributes;
        optionalAttributes.Set<Attributes::MeasuredValue::Id>(mFeatures.Has(Feature::kNumericMeasurement));
        optionalAttributes.Set<Attributes::MinMeasuredValue::Id>(mFeatures.Has(Feature::kNumericMeasurement));
        optionalAttributes.Set<Attributes::MaxMeasuredValue::Id>(mFeatures.Has(Feature::kNumericMeasurement));
        optionalAttributes.Set<Attributes::MeasurementUnit::Id>(mFeatures.Has(Feature::kNumericMeasurement));
        optionalAttributes.Set<Attributes::Uncertainty::Id>(mFeatures.Has(Feature::kNumericMeasurement) &&
                                                            config.uncertainty.has_value());
        optionalAttributes.Set<Attributes::PeakMeasuredValue::Id>(mFeatures.Has(Feature::kPeakMeasurement));
        optionalAttributes.Set<Attributes::PeakMeasuredValueWindow::Id>(mFeatures.Has(Feature::kPeakMeasurement));
        optionalAttributes.Set<Attributes::AverageMeasuredValue::Id>(mFeatures.Has(Feature::kAverageMeasurement));
        optionalAttributes.Set<Attributes::AverageMeasuredValueWindow::Id>(mFeatures.Has(Feature::kAverageMeasurement));
        optionalAttributes.Set<Attributes::LevelValue::Id>(mFeatures.Has(Feature::kLevelIndication));
        return optionalAttributes;
    }()),
    mMedium(config.medium), mUnit(config.unit), mMinMeasuredValue(config.minMeasured), mMaxMeasuredValue(config.maxMeasured),
    mUncertainty(config.uncertainty)
{
    VerifyOrDie(std::find(AliasedClusters.begin(), AliasedClusters.end(), config.clusterId) !=
                AliasedClusters.end()); // NOLINT(bugprone-signed-bitwise)
}

DataModel::ActionReturnStatus ConcentrationMeasurementCluster::ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                                             AttributeValueEncoder & encoder)
{
    switch (request.path.mAttributeId)
    {
    case MeasuredValue::Id:
        return encoder.Encode(mMeasuredValue);
    case MinMeasuredValue::Id:
        return encoder.Encode(mMinMeasuredValue);
    case MaxMeasuredValue::Id:
        return encoder.Encode(mMaxMeasuredValue);
    case PeakMeasuredValue::Id:
        return encoder.Encode(mPeakMeasuredValue);
    case AverageMeasuredValue::Id:
        return encoder.Encode(mAverageMeasuredValue);
    case MeasurementMedium::Id:
        return encoder.Encode(mMedium);
    case Attributes::FeatureMap::Id:
        return encoder.Encode(mFeatures);
    case Attributes::ClusterRevision::Id:
        return encoder.Encode(chip::app::Clusters::CarbonDioxideConcentrationMeasurement::kRevision);
    case Uncertainty::Id:
        return encoder.Encode(mUncertainty.value_or(0.0f));
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

CHIP_ERROR ConcentrationMeasurementCluster::SetNullableFloatValue(DataModel::Nullable<float> value, Feature requiredFeature,
                                                                  DataModel::Nullable<float> & storage, AttributeId attr)
{
    VerifyOrReturnError(mFeatures.Has(requiredFeature), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
    VerifyOrReturnError(IsInRange(value, mMinMeasuredValue, mMaxMeasuredValue), CHIP_IM_GLOBAL_STATUS(ConstraintError));
    SetAttributeValue(storage, value, attr);
    return CHIP_NO_ERROR;
}

CHIP_ERROR ConcentrationMeasurementCluster::SetMeasuredValue(DataModel::Nullable<float> value)
{
    return SetNullableFloatValue(value, Feature::kNumericMeasurement, mMeasuredValue, MeasuredValue::Id);
}

CHIP_ERROR ConcentrationMeasurementCluster::SetPeakMeasuredValue(DataModel::Nullable<float> value)
{
    return SetNullableFloatValue(value, Feature::kPeakMeasurement, mPeakMeasuredValue, PeakMeasuredValue::Id);
}

CHIP_ERROR ConcentrationMeasurementCluster::SetAverageMeasuredValue(DataModel::Nullable<float> value)
{
    return SetNullableFloatValue(value, Feature::kAverageMeasurement, mAverageMeasuredValue, AverageMeasuredValue::Id);
}

CHIP_ERROR ConcentrationMeasurementCluster::SetWindowValue(uint32_t value, Feature requiredFeature, uint32_t & storage,
                                                           AttributeId attr)
{
    VerifyOrReturnError(mFeatures.Has(requiredFeature), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
    VerifyOrReturnError(value <= kWindowMaxSeconds, CHIP_IM_GLOBAL_STATUS(ConstraintError));
    SetAttributeValue(storage, value, attr);
    return CHIP_NO_ERROR;
}

CHIP_ERROR ConcentrationMeasurementCluster::SetPeakMeasuredValueWindow(uint32_t value)
{
    return SetWindowValue(value, Feature::kPeakMeasurement, mPeakMeasuredValueWindow, PeakMeasuredValueWindow::Id);
}

CHIP_ERROR ConcentrationMeasurementCluster::SetAverageMeasuredValueWindow(uint32_t value)
{
    return SetWindowValue(value, Feature::kAverageMeasurement, mAverageMeasuredValueWindow, AverageMeasuredValueWindow::Id);
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
