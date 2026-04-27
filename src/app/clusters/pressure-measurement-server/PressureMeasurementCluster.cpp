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

// Spec bounds for pressure measurement (int16s)
constexpr int16_t kMinMeasuredValueMax = 32766;
constexpr int16_t kMaxMeasuredValueMin = -32766;
constexpr uint16_t kMaxTolerance       = 2048;

namespace {

bool IsMeasuredValueInRange(int16_t value, const DataModel::Nullable<int16_t> & min, const DataModel::Nullable<int16_t> & max)
{
    VerifyOrReturnValue(min.IsNull() || value >= min.Value(), false);
    VerifyOrReturnValue(max.IsNull() || value <= max.Value(), false);
    return true;
}

} // namespace

PressureMeasurementCluster::PressureMeasurementCluster(EndpointId endpointId) :
    PressureMeasurementCluster(endpointId, Config{})
{}

PressureMeasurementCluster::PressureMeasurementCluster(EndpointId endpointId, const Config & config) :
    DefaultServerCluster({ endpointId, PressureMeasurement::Id }), mOptionalAttributeSet(config.mOptionalAttributeSet),
    mFeatureMap(config.mFeatureMap)
{
    // Validate measured value range
    if (!config.minMeasuredValue.IsNull())
    {
        VerifyOrDie(config.minMeasuredValue.Value() <= kMinMeasuredValueMax);

        if (!config.maxMeasuredValue.IsNull())
        {
            VerifyOrDie(config.maxMeasuredValue.Value() >= config.minMeasuredValue.Value() + 1);
        }
    }

    if (!config.maxMeasuredValue.IsNull())
    {
        VerifyOrDie(config.maxMeasuredValue.Value() >= kMaxMeasuredValueMin);
    }

    VerifyOrDie(!mOptionalAttributeSet.IsSet(Tolerance::Id) || config.mTolerance <= kMaxTolerance);

    // Validate EXT feature: if any EXT attribute is present, the Extended feature must be enabled
    if (mOptionalAttributeSet.IsSet(ScaledValue::Id) || mOptionalAttributeSet.IsSet(MinScaledValue::Id) ||
        mOptionalAttributeSet.IsSet(MaxScaledValue::Id) || mOptionalAttributeSet.IsSet(Scale::Id) ||
        mOptionalAttributeSet.IsSet(ScaledTolerance::Id))
    {
        VerifyOrDieWithMsg(mFeatureMap.Has(PressureMeasurement::Feature::kExtended), AppServer,
                           "Scaled attributes require the Extended feature.");
    }

    VerifyOrDie(!mOptionalAttributeSet.IsSet(ScaledTolerance::Id) || config.mScaledTolerance <= kMaxTolerance);

    if (mFeatureMap.Has(PressureMeasurement::Feature::kExtended))
    {
        VerifyOrDie(config.mScale >= -127);
    }

    mMinMeasuredValue = config.minMeasuredValue;
    mMaxMeasuredValue = config.maxMeasuredValue;
    mTolerance        = config.mTolerance;
    mMinScaledValue   = config.mMinScaledValue;
    mMaxScaledValue   = config.mMaxScaledValue;
    mScaledTolerance  = config.mScaledTolerance;
    mScale            = config.mScale;
}

DataModel::ActionReturnStatus PressureMeasurementCluster::ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                                        AttributeValueEncoder & encoder)
{
    switch (request.path.mAttributeId)
    {
    case ClusterRevision::Id:
        return encoder.Encode(PressureMeasurement::kRevision);
    case FeatureMap::Id:
        return encoder.Encode(mFeatureMap.Raw());
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

    const DataModel::AttributeEntry optionalAttributes[] = {
        Tolerance::kMetadataEntry,      ScaledValue::kMetadataEntry,    MinScaledValue::kMetadataEntry,
        MaxScaledValue::kMetadataEntry, ScaledTolerance::kMetadataEntry, Scale::kMetadataEntry,
    };

    return listBuilder.Append(Span(kMandatoryMetadata), Span(optionalAttributes), mOptionalAttributeSet);
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

CHIP_ERROR PressureMeasurementCluster::SetMeasuredValueRange(DataModel::Nullable<int16_t> minMeasuredValue,
                                                              DataModel::Nullable<int16_t> maxMeasuredValue)
{
    if (!minMeasuredValue.IsNull())
    {
        VerifyOrReturnError(minMeasuredValue.Value() <= kMinMeasuredValueMax, CHIP_IM_GLOBAL_STATUS(ConstraintError));

        if (!maxMeasuredValue.IsNull())
        {
            VerifyOrReturnError(maxMeasuredValue.Value() >= minMeasuredValue.Value() + 1, CHIP_IM_GLOBAL_STATUS(ConstraintError));
        }
    }

    if (!maxMeasuredValue.IsNull())
    {
        VerifyOrReturnError(maxMeasuredValue.Value() >= kMaxMeasuredValueMin, CHIP_IM_GLOBAL_STATUS(ConstraintError));
    }

    SetAttributeValue(mMinMeasuredValue, minMeasuredValue, MinMeasuredValue::Id);
    SetAttributeValue(mMaxMeasuredValue, maxMeasuredValue, MaxMeasuredValue::Id);
    return CHIP_NO_ERROR;
}

} // namespace chip::app::Clusters
