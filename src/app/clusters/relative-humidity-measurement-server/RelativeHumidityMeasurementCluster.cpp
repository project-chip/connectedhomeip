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

#include "RelativeHumidityMeasurementCluster.h"
#include <app/server-cluster/AttributeListBuilder.h>
#include <clusters/RelativeHumidityMeasurement/Metadata.h>

namespace chip::app::Clusters {

using namespace RelativeHumidityMeasurement::Attributes;

// Spec-defined upper bound for MinMeasuredValue
constexpr uint16_t kMinMeasuredValueMax = 9999;
// Spec-defined upper bound for MeasuredValue and MaxMeasuredValue
constexpr uint16_t kMeasuredValueMax = 10000;
// Spec-defined upper bound for Tolerance
constexpr uint16_t kMaxTolerance = 2048;

namespace {

// Helper function to check if a value is within the allowed humidity bounds
bool IsValueInHumidityRange(uint16_t value, const DataModel::Nullable<uint16_t> & min, const DataModel::Nullable<uint16_t> & max)
{
    if (value > kMeasuredValueMax)
        return false;
    if (!min.IsNull() && value < min.Value())
        return false;
    if (!max.IsNull() && value > max.Value())
        return false;
    return true;
}

} // namespace

RelativeHumidityMeasurementCluster::RelativeHumidityMeasurementCluster(EndpointId endpointId) :
    RelativeHumidityMeasurementCluster(endpointId, Config{})
{}

RelativeHumidityMeasurementCluster::RelativeHumidityMeasurementCluster(EndpointId endpointId, const Config & config) :
    DefaultServerCluster({ endpointId, RelativeHumidityMeasurement::Id }), mOptionalAttributeSet(config.mOptionalAttributeSet)
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
        VerifyOrDie(config.maxMeasuredValue.Value() <= kMeasuredValueMax);
    }

    VerifyOrDie(!mOptionalAttributeSet.IsSet(Tolerance::Id) || config.mTolerance <= kMaxTolerance);

    mMinMeasuredValue = config.minMeasuredValue;
    mMaxMeasuredValue = config.maxMeasuredValue;
    mTolerance        = config.mTolerance;
}

DataModel::ActionReturnStatus RelativeHumidityMeasurementCluster::ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                                                AttributeValueEncoder & encoder)
{
    switch (request.path.mAttributeId)
    {
    case ClusterRevision::Id:
        return encoder.Encode(RelativeHumidityMeasurement::kRevision);
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

CHIP_ERROR RelativeHumidityMeasurementCluster::Attributes(const ConcreteClusterPath & path,
                                                          ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder)
{
    AttributeListBuilder listBuilder(builder);

    const DataModel::AttributeEntry optionalAttributes[] = {
        Tolerance::kMetadataEntry,
    };

    return listBuilder.Append(Span(kMandatoryMetadata), Span(optionalAttributes), mOptionalAttributeSet);
}

CHIP_ERROR RelativeHumidityMeasurementCluster::SetMeasuredValue(DataModel::Nullable<uint16_t> measuredValue)
{
    if (!measuredValue.IsNull())
    {
        VerifyOrReturnError(IsValueInHumidityRange(measuredValue.Value(), mMinMeasuredValue, mMaxMeasuredValue),
                            CHIP_IM_GLOBAL_STATUS(ConstraintError));
    }

    SetAttributeValue(mMeasuredValue, measuredValue, MeasuredValue::Id);
    return CHIP_NO_ERROR;
}

} // namespace chip::app::Clusters
