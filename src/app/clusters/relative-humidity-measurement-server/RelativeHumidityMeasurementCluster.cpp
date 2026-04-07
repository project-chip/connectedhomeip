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

// Per the Matter spec (Relative Humidity Measurement cluster):
// MinMeasuredValue is constrained to [0, 9999]
constexpr uint16_t kMinMeasuredValueMax = 9999;
// Global ceiling for any measured humidity value (applies to MeasuredValue and MaxMeasuredValue)
constexpr uint16_t kMeasuredValueMax = 10000;
// Tolerance max is 2048
constexpr uint16_t kMaxTolerance = 2048;

RelativeHumidityMeasurementCluster::RelativeHumidityMeasurementCluster(EndpointId endpointId,
                                                                       const OptionalAttributeSet & optionalAttributeSet,
                                                                       const StartupConfiguration & config) :
    DefaultServerCluster({ endpointId, RelativeHumidityMeasurement::Id }),
    mOptionalAttributeSet(optionalAttributeSet)
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
        VerifyOrDie(config.maxMeasuredValue.Value() >= 1 && config.maxMeasuredValue.Value() <= kMeasuredValueMax);
    }

    VerifyOrDie(!mOptionalAttributeSet.IsSet(Tolerance::Id) || config.tolerance <= kMaxTolerance);

    mMinMeasuredValue = config.minMeasuredValue;
    mMaxMeasuredValue = config.maxMeasuredValue;
    mTolerance        = config.tolerance;
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
        VerifyOrReturnError(measuredValue.Value() <= kMeasuredValueMax, CHIP_IM_GLOBAL_STATUS(ConstraintError));

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

CHIP_ERROR RelativeHumidityMeasurementCluster::SetMeasuredValueRange(DataModel::Nullable<uint16_t> minMeasuredValue,
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
        // MaxMeasuredValue must be >= MinMeasuredValue+1 (min of MinMeasuredValue is 0, so global min is 1)
        VerifyOrReturnError(maxMeasuredValue.Value() >= 1 && maxMeasuredValue.Value() <= kMeasuredValueMax,
                            CHIP_IM_GLOBAL_STATUS(ConstraintError));
    }

    SetAttributeValue(mMinMeasuredValue, minMeasuredValue, MinMeasuredValue::Id);
    SetAttributeValue(mMaxMeasuredValue, maxMeasuredValue, MaxMeasuredValue::Id);
    return CHIP_NO_ERROR;
}

} // namespace chip::app::Clusters
