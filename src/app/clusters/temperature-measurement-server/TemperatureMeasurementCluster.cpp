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

#include <app/clusters/temperature-measurement-server/TemperatureMeasurementCluster.h>
#include <app/server-cluster/AttributeListBuilder.h>
#include <clusters/TemperatureMeasurement/Metadata.h>

namespace chip::app::Clusters {

using namespace TemperatureMeasurement::Attributes;

// According to the spec, absolute minimum value is -27315
constexpr int16_t kMinimumMinMeasuredValue = -27315;
// According to the spec, maximum minimum value is 32766
constexpr int16_t kMaximumMinMeasuredValue = 32766;

TemperatureMeasurementCluster::TemperatureMeasurementCluster(EndpointId endpointId,
                                                             const OptionalAttributeSet & optionalAttributeSet,
                                                             const StartupConfiguration & config) :
    DefaultServerCluster({ endpointId, TemperatureMeasurement::Id }), mOptionalAttributeSet(optionalAttributeSet)
{
    VerifyOrDie(config.minMeasuredValue.ValueOr(kMinimumMinMeasuredValue) >= kMinimumMinMeasuredValue &&
                config.minMeasuredValue.ValueOr(kMaximumMinMeasuredValue) <= kMaximumMinMeasuredValue);

    VerifyOrDie(config.maxMeasuredValue.ValueOr(config.minMeasuredValue.ValueOr(kMinimumMinMeasuredValue) + 1) >=
                config.minMeasuredValue.ValueOr(kMinimumMinMeasuredValue) + 1);

    VerifyOrDie(!mOptionalAttributeSet.IsSet(Tolerance::Id) || config.tolerance <= 2048);

    mMinMeasuredValue = config.minMeasuredValue;
    mMaxMeasuredValue = config.maxMeasuredValue;
    mTolerance        = config.tolerance;
}

DataModel::ActionReturnStatus TemperatureMeasurementCluster::ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                                           AttributeValueEncoder & encoder)
{
    switch (request.path.mAttributeId)
    {
    case ClusterRevision::Id:
        return encoder.Encode(TemperatureMeasurement::kRevision);
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

CHIP_ERROR TemperatureMeasurementCluster::Attributes(const ConcreteClusterPath & path,
                                                     ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder)
{
    AttributeListBuilder listBuilder(builder);

    const DataModel::AttributeEntry optionalAttributes[] = {
        Tolerance::kMetadataEntry, //
    };

    return listBuilder.Append(Span(kMandatoryMetadata), Span(optionalAttributes), mOptionalAttributeSet);
}

CHIP_ERROR TemperatureMeasurementCluster::SetMeasuredValue(DataModel::Nullable<int16_t> measuredValue)
{
    if (!measuredValue.IsNull())
    {
        VerifyOrReturnError(measuredValue.Value() >= mMinMeasuredValue.ValueOr(kMinimumMinMeasuredValue),
                            CHIP_ERROR_INVALID_ARGUMENT);

        VerifyOrReturnError(measuredValue.Value() <=
                                mMaxMeasuredValue.ValueOr(mMinMeasuredValue.ValueOr(kMinimumMinMeasuredValue + 1)),
                            CHIP_ERROR_INVALID_ARGUMENT);
    }

    SetAttributeValue(mMeasuredValue, measuredValue, MeasuredValue::Id);
    return CHIP_NO_ERROR;
}

CHIP_ERROR TemperatureMeasurementCluster::SetMinMeasuredValue(DataModel::Nullable<int16_t> minMeasuredValue)
{
    if (!minMeasuredValue.IsNull())
    {
        VerifyOrReturnError(minMeasuredValue.ValueOr(kMinimumMinMeasuredValue) >= kMinimumMinMeasuredValue &&
                                minMeasuredValue.ValueOr(kMaximumMinMeasuredValue) <= kMaximumMinMeasuredValue,
                            CHIP_ERROR_INVALID_ARGUMENT);
    }

    SetAttributeValue(mMinMeasuredValue, minMeasuredValue, MinMeasuredValue::Id);
    return CHIP_NO_ERROR;
}

CHIP_ERROR TemperatureMeasurementCluster::SetMaxMeasuredValue(DataModel::Nullable<int16_t> maxMeasuredValue)
{
    if (!maxMeasuredValue.IsNull())
    {
        VerifyOrReturnError(maxMeasuredValue.ValueOr(mMinMeasuredValue.ValueOr(kMinimumMinMeasuredValue) + 1) >=
                                mMinMeasuredValue.ValueOr(kMinimumMinMeasuredValue) + 1,
                            CHIP_ERROR_INVALID_ARGUMENT);
    }

    SetAttributeValue(mMaxMeasuredValue, maxMeasuredValue, MaxMeasuredValue::Id);
    return CHIP_NO_ERROR;
}

} // namespace chip::app::Clusters
