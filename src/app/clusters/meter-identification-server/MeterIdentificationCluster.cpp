/*
 *
 *    Copyright (c) 2024-2026 Project CHIP Authors
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

#include "MeterIdentificationCluster.h"

#include <algorithm>
#include <cstring>

#include <app/server-cluster/AttributeListBuilder.h>
#include <clusters/MeterIdentification/Metadata.h>
#include <lib/core/CHIPError.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/TypeTraits.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::MeterIdentification::Attributes;
using chip::Protocols::InteractionModel::Status;

namespace {

bool PowerThresholdStructEqual(const Globals::Structs::PowerThresholdStruct::Type & a,
                               const Globals::Structs::PowerThresholdStruct::Type & b)
{
    return a.powerThreshold == b.powerThreshold && a.apparentPowerThreshold == b.apparentPowerThreshold &&
        a.powerThresholdSource == b.powerThresholdSource;
}

bool NullableCharSpanEqual(const DataModel::Nullable<CharSpan> & a, const DataModel::Nullable<CharSpan> & b)
{
    if (a.IsNull() || b.IsNull())
    {
        return a.IsNull() == b.IsNull();
    }

    return a.Value().data_equal(b.Value());
}

// Copies `value` into `buf` (clamped to kMaximumStringSize) and stores the resulting span in `dest`,
// or nulls `dest` if `value` is null. Uses memmove since `buf` may alias `value` (e.g. a caller
// passing back a span obtained from this same cluster's Get*() accessor).
void SetNullableString(char * buf, DataModel::Nullable<CharSpan> & dest, const DataModel::Nullable<CharSpan> & value)
{
    if (value.IsNull())
    {
        dest.SetNull();
        return;
    }

    const size_t len = std::min(value.Value().size(), MeterIdentificationCluster::kMaximumStringSize);
    memmove(buf, value.Value().data(), len);
    dest.SetNonNull(CharSpan(buf, len));
}

} // namespace

namespace chip::app::Clusters {

MeterIdentificationCluster::MeterIdentificationCluster(EndpointId endpointId, const Config & config) :
    DefaultServerCluster(ConcreteClusterPath(endpointId, MeterIdentification::Id)), mFeatureMap(config.mFeatureMap),
    mOptionalAttributeSet([&config]() {
        OptionalAttributeSet attrs;
        if (config.mOptionalAttributes.IsSet(ProtocolVersion::Id))
        {
            attrs.Set<ProtocolVersion::Id>();
        }
        if (config.mFeatureMap.Has(Feature::kPowerThreshold))
        {
            // PowerThreshold is not part of OptionalAttributeSet's template pack: its presence is
            // forced by the feature, not independently settable via Config::mOptionalAttributes.
            attrs.ForceSet<PowerThreshold::Id>();
        }
        return attrs;
    }())
{
    // Route initial values through the real setters so construction gets the same validation
    // (enum checks, length limits) as runtime writes, instead of storing unvalidated Config data
    // directly. An invalid initial value is logged and left null rather than failing construction.
    LogErrorOnFailure(SetMeterType(config.mMeterType));
    LogErrorOnFailure(SetPointOfDelivery(config.mPointOfDelivery));
    LogErrorOnFailure(SetMeterSerialNumber(config.mMeterSerialNumber));

    // SetProtocolVersion/SetPowerThreshold reject any call (even with a null value) when their
    // attribute/feature isn't enabled, so only call them when applicable to avoid logging a
    // spurious error for the common case of an unconfigured optional attribute.
    if (mOptionalAttributeSet.IsSet(ProtocolVersion::Id))
    {
        LogErrorOnFailure(SetProtocolVersion(config.mProtocolVersion));
    }
    if (HasFeature(Feature::kPowerThreshold))
    {
        LogErrorOnFailure(SetPowerThreshold(config.mPowerThreshold));
    }
}

DataModel::ActionReturnStatus MeterIdentificationCluster::ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                                        AttributeValueEncoder & encoder)
{
    switch (request.path.mAttributeId)
    {
    case ClusterRevision::Id:
        return encoder.Encode(MeterIdentification::kRevision);
    case FeatureMap::Id:
        return encoder.Encode(mFeatureMap.Raw());
    case MeterType::Id:
        return encoder.Encode(mMeterType);
    case PointOfDelivery::Id:
        return encoder.Encode(mPointOfDelivery);
    case MeterSerialNumber::Id:
        return encoder.Encode(mMeterSerialNumber);
    case ProtocolVersion::Id:
        return encoder.Encode(mProtocolVersion);
    case PowerThreshold::Id:
        return encoder.Encode(mPowerThreshold);
    default:
        return Status::UnsupportedAttribute;
    }
}

CHIP_ERROR MeterIdentificationCluster::Attributes(const ConcreteClusterPath & path,
                                                  ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder)
{
    AttributeListBuilder listBuilder(builder);

    const DataModel::AttributeEntry optionalAttrs[] = {
        ProtocolVersion::kMetadataEntry,
        PowerThreshold::kMetadataEntry,
    };

    return listBuilder.Append(Span(kMandatoryMetadata), Span(optionalAttrs), mOptionalAttributeSet);
}

CHIP_ERROR MeterIdentificationCluster::SetMeterType(const DataModel::Nullable<MeterTypeEnum> & value)
{
    if (!value.IsNull())
    {
        VerifyOrReturnError(EnsureKnownEnumValue(value.Value()) != MeterTypeEnum::kUnknownEnumValue,
                            CHIP_IM_GLOBAL_STATUS(ConstraintError));
    }

    SetAttributeValue(mMeterType, value, MeterType::Id);
    return CHIP_NO_ERROR;
}

CHIP_ERROR MeterIdentificationCluster::SetPointOfDelivery(const DataModel::Nullable<CharSpan> & value)
{
    if (NullableCharSpanEqual(value, mPointOfDelivery))
    {
        return CHIP_NO_ERROR;
    }

    if (!value.IsNull())
    {
        VerifyOrReturnError(value.Value().size() <= kMaximumStringSize, CHIP_IM_GLOBAL_STATUS(ConstraintError));
    }

    SetNullableString(mPointOfDeliveryBuf, mPointOfDelivery, value);

    NotifyAttributeChanged(PointOfDelivery::Id);
    return CHIP_NO_ERROR;
}

CHIP_ERROR MeterIdentificationCluster::SetMeterSerialNumber(const DataModel::Nullable<CharSpan> & value)
{
    if (NullableCharSpanEqual(value, mMeterSerialNumber))
    {
        return CHIP_NO_ERROR;
    }

    if (!value.IsNull())
    {
        VerifyOrReturnError(value.Value().size() <= kMaximumStringSize, CHIP_IM_GLOBAL_STATUS(ConstraintError));
    }

    SetNullableString(mMeterSerialNumberBuf, mMeterSerialNumber, value);

    NotifyAttributeChanged(MeterSerialNumber::Id);
    return CHIP_NO_ERROR;
}

CHIP_ERROR MeterIdentificationCluster::SetProtocolVersion(const DataModel::Nullable<CharSpan> & value)
{
    VerifyOrReturnError(mOptionalAttributeSet.IsSet(ProtocolVersion::Id), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);

    if (NullableCharSpanEqual(value, mProtocolVersion))
    {
        return CHIP_NO_ERROR;
    }

    if (!value.IsNull())
    {
        VerifyOrReturnError(value.Value().size() <= kMaximumStringSize, CHIP_IM_GLOBAL_STATUS(ConstraintError));
    }

    SetNullableString(mProtocolVersionBuf, mProtocolVersion, value);

    NotifyAttributeChanged(ProtocolVersion::Id);
    return CHIP_NO_ERROR;
}

CHIP_ERROR
MeterIdentificationCluster::SetPowerThreshold(const DataModel::Nullable<Globals::Structs::PowerThresholdStruct::Type> & value)
{
    VerifyOrReturnError(HasFeature(Feature::kPowerThreshold), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);

    if (value.IsNull())
    {
        if (mPowerThreshold.IsNull())
        {
            return CHIP_NO_ERROR;
        }
        mPowerThreshold.SetNull();
    }
    else
    {
        if (!mPowerThreshold.IsNull() && PowerThresholdStructEqual(mPowerThreshold.Value(), value.Value()))
        {
            return CHIP_NO_ERROR;
        }

        if (!value.Value().powerThresholdSource.IsNull())
        {
            VerifyOrReturnError(EnsureKnownEnumValue(value.Value().powerThresholdSource.Value()) !=
                                    Globals::PowerThresholdSourceEnum::kUnknownEnumValue,
                                CHIP_IM_GLOBAL_STATUS(ConstraintError));
        }

        mPowerThreshold.SetNonNull(value.Value());
    }

    NotifyAttributeChanged(PowerThreshold::Id);
    return CHIP_NO_ERROR;
}

} // namespace chip::app::Clusters
