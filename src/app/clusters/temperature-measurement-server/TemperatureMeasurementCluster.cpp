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
constexpr int16_t kMinMeasuredValueRange = -27315;
// According to the spec, maximum minimum value is 32766
constexpr int16_t kMaxMeasuredValueRange = 32766;
// According to the spec, maximum tolerance value is 2048
constexpr uint16_t kMaxTolerance = 2048;

TemperatureMeasurementCluster::TemperatureMeasurementCluster(EndpointId endpointId,
                                                             const OptionalAttributeSet & optionalAttributeSet,
                                                             const StartupConfiguration & config) :
    DefaultServerCluster({ endpointId, TemperatureMeasurement::Id }),
    mOptionalAttributeSet(optionalAttributeSet)
{
    if (!config.minMeasuredValue.IsNull())
    {
        VerifyOrDie(config.minMeasuredValue.Value() >= kMinMeasuredValueRange &&
                    config.minMeasuredValue.Value() <= kMaxMeasuredValueRange);

        if (!config.maxMeasuredValue.IsNull())
        {
            VerifyOrDie(config.maxMeasuredValue.Value() >= config.minMeasuredValue.Value() + 1);
        }
    }

    VerifyOrDie(!mOptionalAttributeSet.IsSet(Tolerance::Id) || config.tolerance <= kMaxTolerance);

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
        if (!mMinMeasuredValue.IsNull())
        {
            VerifyOrReturnError(measuredValue.Value() >= mMinMeasuredValue.Value(), CHIP_IM_GLOBAL_STATUS(ConstraintError));
        }

        if (!mMaxMeasuredValue.IsNull())
        {
            VerifyOrReturnError(measuredValue.Value() <= mMaxMeasuredValue.Value(), CHIP_IM_GLOBAL_STATUS(ConstraintError));
        }
    }

    SetAttributeValue(mMeasuredValue, measuredValue, MeasuredValue::Id);
    return CHIP_NO_ERROR;
}

CHIP_ERROR TemperatureMeasurementCluster::SetMeasuredValueRange(DataModel::Nullable<int16_t> minMeasuredValue,
                                                                DataModel::Nullable<int16_t> maxMeasuredValue)
{
    if (!minMeasuredValue.IsNull())
    {
        VerifyOrReturnError(minMeasuredValue.Value() >= kMinMeasuredValueRange &&
                                minMeasuredValue.Value() <= kMaxMeasuredValueRange,
                            CHIP_IM_GLOBAL_STATUS(ConstraintError));

        if (!maxMeasuredValue.IsNull())
        {
            VerifyOrReturnError(maxMeasuredValue.Value() >= minMeasuredValue.Value() + 1, CHIP_IM_GLOBAL_STATUS(ConstraintError));
        }
    }

    SetAttributeValue(mMinMeasuredValue, minMeasuredValue, MinMeasuredValue::Id);
    SetAttributeValue(mMaxMeasuredValue, maxMeasuredValue, MaxMeasuredValue::Id);
    return CHIP_NO_ERROR;
}

} // namespace chip::app::Clusters
