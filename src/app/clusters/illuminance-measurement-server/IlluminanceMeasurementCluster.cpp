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

#include <app/clusters/illuminance-measurement-server/IlluminanceMeasurementCluster.h>
#include <app/server-cluster/AttributeListBuilder.h>
#include <clusters/IlluminanceMeasurement/Metadata.h>

namespace chip::app::Clusters {

using namespace IlluminanceMeasurement::Attributes;

// According to the spec, absolute minimum value is 1
constexpr uint16_t kMinMeasuredValueRange = 1;
// According to the spec, maximum minimum value is 65533
constexpr uint16_t kMaxMeasuredValueRange = 65533;
// According to the spec, maximum tolerance value is 2048
constexpr uint16_t kMaxTolerance = 2048;

IlluminanceMeasurementCluster::IlluminanceMeasurementCluster(EndpointId endpointId,
                                                             const OptionalAttributeSet & optionalAttributeSet,
                                                             const StartupConfiguration & config) :
    DefaultServerCluster({ endpointId, IlluminanceMeasurement::Id }),
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
    mLightSensorType  = config.lightSensorType;
}

DataModel::ActionReturnStatus IlluminanceMeasurementCluster::ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                                           AttributeValueEncoder & encoder)
{
    switch (request.path.mAttributeId)
    {
    case ClusterRevision::Id:
        return encoder.Encode(IlluminanceMeasurement::kRevision);
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
    case LightSensorType::Id:
        return encoder.Encode(mLightSensorType);
    default:
        return Protocols::InteractionModel::Status::UnsupportedAttribute;
    }
}

CHIP_ERROR IlluminanceMeasurementCluster::Attributes(const ConcreteClusterPath & path,
                                                     ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder)
{
    AttributeListBuilder listBuilder(builder);

    const DataModel::AttributeEntry optionalAttributes[] = {
        Tolerance::kMetadataEntry,       //
        LightSensorType::kMetadataEntry, //
    };

    return listBuilder.Append(Span(kMandatoryMetadata), Span(optionalAttributes), mOptionalAttributeSet);
}

CHIP_ERROR IlluminanceMeasurementCluster::SetMeasuredValue(DataModel::Nullable<uint16_t> measuredValue)
{
    if (measuredValue.ValueOr(0) != 0)
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

CHIP_ERROR IlluminanceMeasurementCluster::SetMeasuredValueRange(DataModel::Nullable<uint16_t> minMeasuredValue,
                                                                DataModel::Nullable<uint16_t> maxMeasuredValue)
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
