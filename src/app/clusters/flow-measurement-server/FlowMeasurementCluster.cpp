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

#include <app/clusters/flow-measurement-server/FlowMeasurementCluster.h>
#include <app/server-cluster/AttributeListBuilder.h>
#include <clusters/FlowMeasurement/Metadata.h>

namespace chip::app::Clusters {

using namespace FlowMeasurement::Attributes;

// upper bounds as per specification
constexpr uint16_t kMinMeasuredValueMax = 65533;
constexpr uint16_t kMaxMeasuredValueMax = 65534;
constexpr uint16_t kMaxTolerance        = 2048;

namespace {

// Helper to check if a value is within the allowed flow measurement bounds
bool IsMeasuredValueInRange(uint16_t value, const DataModel::Nullable<uint16_t> & min, const DataModel::Nullable<uint16_t> & max)
{
    VerifyOrReturnValue(min.IsNull() || value >= min.Value(), false);
    VerifyOrReturnValue(max.IsNull() || value <= max.Value(), false);
    return true;
}

} // namespace

FlowMeasurementCluster::FlowMeasurementCluster(EndpointId endpointId) : FlowMeasurementCluster(endpointId, Config{}) {}

FlowMeasurementCluster::FlowMeasurementCluster(EndpointId endpointId, const Config & config) :
    DefaultServerCluster({ endpointId, FlowMeasurement::Id }), mOptionalAttributeSet(config.mOptionalAttributeSet)
{
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
        VerifyOrDie(config.maxMeasuredValue.Value() <= kMaxMeasuredValueMax);
    }

    VerifyOrDie(!mOptionalAttributeSet.IsSet(Tolerance::Id) || config.mTolerance <= kMaxTolerance);

    mMinMeasuredValue = config.minMeasuredValue;
    mMaxMeasuredValue = config.maxMeasuredValue;
    mTolerance        = config.mTolerance;
}

DataModel::ActionReturnStatus FlowMeasurementCluster::ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                                    AttributeValueEncoder & encoder)
{
    switch (request.path.mAttributeId)
    {
    case ClusterRevision::Id:
        return encoder.Encode(FlowMeasurement::kRevision);
    case FeatureMap::Id:
        return encoder.Encode<uint32_t>(0);
    case MeasuredValue::Id:
        return encoder.Encode(mMeasuredValue);
    case MinMeasuredValue::Id:
        return encoder.Encode(mMinMeasuredValue);
    case MaxMeasuredValue::Id:
        return encoder.Encode(mMaxMeasuredValue);
    case Tolerance::Id:
        return encoder.Encode(mTolerance);
    default:
        return Protocols::InteractionModel::Status::UnsupportedAttribute;
    }
}

CHIP_ERROR FlowMeasurementCluster::Attributes(const ConcreteClusterPath & path,
                                              ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder)
{
    AttributeListBuilder listBuilder(builder);

    const DataModel::AttributeEntry optionalAttributes[] = {
        Tolerance::kMetadataEntry,
    };

    return listBuilder.Append(Span(kMandatoryMetadata), Span(optionalAttributes), mOptionalAttributeSet);
}

CHIP_ERROR FlowMeasurementCluster::SetMeasuredValue(DataModel::Nullable<uint16_t> measuredValue)
{
    if (!measuredValue.IsNull())
    {
        VerifyOrReturnError(IsMeasuredValueInRange(measuredValue.Value(), mMinMeasuredValue, mMaxMeasuredValue),
                            CHIP_IM_GLOBAL_STATUS(ConstraintError));
    }

    SetAttributeValue(mMeasuredValue, measuredValue, MeasuredValue::Id);
    return CHIP_NO_ERROR;
}

CHIP_ERROR FlowMeasurementCluster::SetMeasuredValueRange(DataModel::Nullable<uint16_t> minMeasuredValue,
                                                         DataModel::Nullable<uint16_t> maxMeasuredValue)
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
        VerifyOrReturnError(maxMeasuredValue.Value() <= kMaxMeasuredValueMax, CHIP_IM_GLOBAL_STATUS(ConstraintError));
    }

    SetAttributeValue(mMinMeasuredValue, minMeasuredValue, MinMeasuredValue::Id);
    SetAttributeValue(mMaxMeasuredValue, maxMeasuredValue, MaxMeasuredValue::Id);
    return CHIP_NO_ERROR;
}

} // namespace chip::app::Clusters
