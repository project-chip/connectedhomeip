/*
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

#include <app/clusters/pressure-measurement-server/PressureMeasurementCluster.h>
#include <app/server-cluster/AttributeListBuilder.h>
#include <clusters/PressureMeasurement/Metadata.h>

namespace chip::app::Clusters {

using namespace PressureMeasurement::Attributes;

// Spec bounds for constructor validation
constexpr int16_t kMinMeasuredValueMax = 32766;
constexpr int16_t kMaxMeasuredValueMax = 32767;
constexpr uint16_t kToleranceMax       = 2048;
constexpr int16_t kMinScaledValueMax   = 32766;
constexpr int16_t kMaxScaledValueMax   = 32767;
constexpr uint16_t kScaledToleranceMax = 2048;
constexpr int8_t kScaleMin             = -127;

namespace {

bool IsMeasuredValueInRange(int16_t value, const DataModel::Nullable<int16_t> & min, const DataModel::Nullable<int16_t> & max)
{
    VerifyOrReturnValue(min.IsNull() || value >= min.Value(), false);
    VerifyOrReturnValue(max.IsNull() || value <= max.Value(), false);
    return true;
}

} // namespace

PressureMeasurementCluster::PressureMeasurementCluster(EndpointId endpointId) : PressureMeasurementCluster(endpointId, Config{}) {}

PressureMeasurementCluster::PressureMeasurementCluster(EndpointId endpointId, const Config & config) :
    DefaultServerCluster({ endpointId, PressureMeasurement::Id }), mOptionalAttributeSet(config.mOptionalAttributeSet),
    mFeatureMap(config.mFeatureMap), mMinMeasuredValue(config.minMeasuredValue), mMaxMeasuredValue(config.maxMeasuredValue),
    mTolerance(config.mTolerance), mMinScaledValue(config.mMinScaledValue), mMaxScaledValue(config.mMaxScaledValue),
    mScaledTolerance(config.mScaledTolerance), mScale(config.mScale)
{
    // Validate measured value range per spec constraints
    if (!mMinMeasuredValue.IsNull())
    {
        VerifyOrDie(mMinMeasuredValue.Value() <= kMinMeasuredValueMax);
    }

    if (!mMaxMeasuredValue.IsNull())
    {
        VerifyOrDie(mMaxMeasuredValue.Value() <= kMaxMeasuredValueMax);

        if (!mMinMeasuredValue.IsNull())
        {
            VerifyOrDie(mMaxMeasuredValue.Value() >= mMinMeasuredValue.Value() + 1);
        }
    }

    VerifyOrDie(!mOptionalAttributeSet.IsSet(Tolerance::Id) || mTolerance <= kToleranceMax);

    // Validate EXT feature
    VerifyOrDieWithMsg(!mOptionalAttributeSet.IsSet(ScaledTolerance::Id) ||
                           mFeatureMap.Has(PressureMeasurement::Feature::kExtended),
                       AppServer, "ScaledTolerance requires the Extended feature. Use WithExtendedFeature() in Config.");

    if (mFeatureMap.Has(PressureMeasurement::Feature::kExtended))
    {
        VerifyOrDie(mScale >= kScaleMin);
        VerifyOrDie(!mOptionalAttributeSet.IsSet(ScaledTolerance::Id) || mScaledTolerance <= kScaledToleranceMax);

        if (!mMinScaledValue.IsNull())
        {
            VerifyOrDie(mMinScaledValue.Value() <= kMinScaledValueMax);
        }

        if (!mMaxScaledValue.IsNull())
        {
            VerifyOrDie(mMaxScaledValue.Value() <= kMaxScaledValueMax);

            if (!mMinScaledValue.IsNull())
            {
                VerifyOrDie(mMaxScaledValue.Value() >= mMinScaledValue.Value() + 1);
            }
        }
    }
}

DataModel::ActionReturnStatus PressureMeasurementCluster::ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                                        AttributeValueEncoder & encoder)
{
    switch (request.path.mAttributeId)
    {
    case ClusterRevision::Id:
        return encoder.Encode(PressureMeasurement::kRevision);
    case FeatureMap::Id:
        return encoder.Encode(mFeatureMap);
    case MeasuredValue::Id:
        return encoder.Encode(mMeasuredValue);
    case MinMeasuredValue::Id:
        return encoder.Encode(mMinMeasuredValue);
    case MaxMeasuredValue::Id:
        return encoder.Encode(mMaxMeasuredValue);
    case Tolerance::Id:
        return encoder.Encode(mTolerance);
    case ScaledValue::Id:
        return encoder.Encode(mScaledValue);
    case MinScaledValue::Id:
        return encoder.Encode(mMinScaledValue);
    case MaxScaledValue::Id:
        return encoder.Encode(mMaxScaledValue);
    case ScaledTolerance::Id:
        return encoder.Encode(mScaledTolerance);
    case Scale::Id:
        return encoder.Encode(mScale);
    default:
        return Protocols::InteractionModel::Status::UnsupportedAttribute;
    }
}

CHIP_ERROR PressureMeasurementCluster::Attributes(const ConcreteClusterPath & path,
                                                  ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder)
{
    AttributeListBuilder listBuilder(builder);

    const bool hasExt = mFeatureMap.Has(PressureMeasurement::Feature::kExtended);

    const AttributeListBuilder::OptionalAttributeEntry optionalAttributes[] = {
        { mOptionalAttributeSet.IsSet(Tolerance::Id), Tolerance::kMetadataEntry },
        { hasExt, ScaledValue::kMetadataEntry },
        { hasExt, MinScaledValue::kMetadataEntry },
        { hasExt, MaxScaledValue::kMetadataEntry },
        { hasExt, Scale::kMetadataEntry },
        { hasExt && mOptionalAttributeSet.IsSet(ScaledTolerance::Id), ScaledTolerance::kMetadataEntry },
    };

    return listBuilder.Append(Span(kMandatoryMetadata), Span(optionalAttributes));
}

CHIP_ERROR PressureMeasurementCluster::SetMeasuredValue(DataModel::Nullable<int16_t> measuredValue)
{
    if (!measuredValue.IsNull())
    {
        VerifyOrReturnError(IsMeasuredValueInRange(measuredValue.Value(), mMinMeasuredValue, mMaxMeasuredValue),
                            CHIP_IM_GLOBAL_STATUS(ConstraintError));
    }

    SetAttributeValue(mMeasuredValue, measuredValue, MeasuredValue::Id);
    return CHIP_NO_ERROR;
}

CHIP_ERROR PressureMeasurementCluster::SetScaledValue(DataModel::Nullable<int16_t> scaledValue)
{
    VerifyOrReturnError(mFeatureMap.Has(PressureMeasurement::Feature::kExtended), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);

    if (!scaledValue.IsNull())
    {
        VerifyOrReturnError(IsMeasuredValueInRange(scaledValue.Value(), mMinScaledValue, mMaxScaledValue),
                            CHIP_IM_GLOBAL_STATUS(ConstraintError));
    }

    SetAttributeValue(mScaledValue, scaledValue, ScaledValue::Id);
    return CHIP_NO_ERROR;
}

} // namespace chip::app::Clusters
