/*
 *
 *    Copyright (c) 2024-2026 Project CHIP Authors
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

#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <app/data-model/Nullable.h>
#include <app/server-cluster/DefaultServerCluster.h>
#include <app/server-cluster/OptionalAttributeSet.h>
#include <clusters/MeterIdentification/Attributes.h>
#include <clusters/MeterIdentification/Metadata.h>
#include <lib/core/CHIPError.h>
#include <lib/support/BitFlags.h>
#include <lib/support/Span.h>

namespace chip::app::Clusters {

class MeterIdentificationCluster : public DefaultServerCluster
{
public:
    using Feature       = MeterIdentification::Feature;
    using MeterTypeEnum = MeterIdentification::MeterTypeEnum;

    // Optional attributes are tracked as a compile-time bitset.
    using OptionalAttributeSet = app::OptionalAttributeSet<MeterIdentification::Attributes::ProtocolVersion::Id,
                                                           MeterIdentification::Attributes::PowerThreshold::Id>;

    static constexpr size_t kMaximumStringSize = 64;

    class Config
    {
    public:
        Config & WithFeatures(BitFlags<Feature> features)
        {
            mFeatureMap = features;
            return *this;
        }

        Config & WithFeature(Feature feature)
        {
            mFeatureMap.Set(feature);
            return *this;
        }

        Config & WithProtocolVersion()
        {
            mOptionalAttributes.Set<MeterIdentification::Attributes::ProtocolVersion::Id>();
            return *this;
        }

        Config & WithProtocolVersion(DataModel::Nullable<CharSpan> value)
        {
            mOptionalAttributes.Set<MeterIdentification::Attributes::ProtocolVersion::Id>();
            mProtocolVersion = value;
            return *this;
        }

        Config & WithOptionalAttributes(OptionalAttributeSet attrs)
        {
            mOptionalAttributes = attrs;
            return *this;
        }

        Config & WithMeterType(DataModel::Nullable<MeterTypeEnum> val)
        {
            mMeterType = val;
            return *this;
        }

        Config & WithPointOfDelivery(DataModel::Nullable<CharSpan> val)
        {
            mPointOfDelivery = val;
            return *this;
        }

        Config & WithMeterSerialNumber(DataModel::Nullable<CharSpan> val)
        {
            mMeterSerialNumber = val;
            return *this;
        }

        Config & WithPowerThreshold(DataModel::Nullable<Globals::Structs::PowerThresholdStruct::Type> val)
        {
            mPowerThreshold = val;
            return *this;
        }

        BitFlags<Feature> mFeatureMap;
        OptionalAttributeSet mOptionalAttributes;
        DataModel::Nullable<MeterTypeEnum> mMeterType;
        DataModel::Nullable<CharSpan> mPointOfDelivery;
        DataModel::Nullable<CharSpan> mMeterSerialNumber;
        DataModel::Nullable<CharSpan> mProtocolVersion;
        DataModel::Nullable<Globals::Structs::PowerThresholdStruct::Type> mPowerThreshold;
    };

    explicit MeterIdentificationCluster(EndpointId endpointId, const Config & config = {});

    // The CharSpan members below point into this object's own buffers. A defaulted copy would
    // shallow-copy those spans, aliasing the source object's buffers, so copying is disallowed.
    MeterIdentificationCluster(const MeterIdentificationCluster &)             = delete;
    MeterIdentificationCluster & operator=(const MeterIdentificationCluster &) = delete;

    // --- ServerClusterInterface overrides ---
    DataModel::ActionReturnStatus ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                AttributeValueEncoder & encoder) override;
    CHIP_ERROR Attributes(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder) override;

    BitFlags<Feature> GetFeatureMap() const { return mFeatureMap; }
    bool HasFeature(Feature feature) const { return mFeatureMap.Has(feature); }

    const DataModel::Nullable<MeterTypeEnum> & GetMeterType() const { return mMeterType; }
    const DataModel::Nullable<CharSpan> & GetPointOfDelivery() const { return mPointOfDelivery; }
    const DataModel::Nullable<CharSpan> & GetMeterSerialNumber() const { return mMeterSerialNumber; }
    const DataModel::Nullable<CharSpan> & GetProtocolVersion() const { return mProtocolVersion; }
    const DataModel::Nullable<Globals::Structs::PowerThresholdStruct::Type> & GetPowerThreshold() const { return mPowerThreshold; }

    CHIP_ERROR SetMeterType(const DataModel::Nullable<MeterTypeEnum> & value);
    CHIP_ERROR SetPointOfDelivery(const DataModel::Nullable<CharSpan> & value);
    CHIP_ERROR SetMeterSerialNumber(const DataModel::Nullable<CharSpan> & value);
    CHIP_ERROR SetProtocolVersion(const DataModel::Nullable<CharSpan> & value);
    CHIP_ERROR SetPowerThreshold(const DataModel::Nullable<Globals::Structs::PowerThresholdStruct::Type> & value);

private:
    const BitFlags<Feature> mFeatureMap;
    const OptionalAttributeSet mOptionalAttributeSet;

    // +1 for the null terminator that Platform::CopyString always writes; kMaximumStringSize is the
    // spec max data length, not the buffer size.
    char mPointOfDeliveryBuf[kMaximumStringSize + 1]   = {};
    char mMeterSerialNumberBuf[kMaximumStringSize + 1] = {};
    char mProtocolVersionBuf[kMaximumStringSize + 1]   = {};

    DataModel::Nullable<MeterTypeEnum> mMeterType;
    DataModel::Nullable<CharSpan> mPointOfDelivery;
    DataModel::Nullable<CharSpan> mMeterSerialNumber;
    DataModel::Nullable<CharSpan> mProtocolVersion;
    DataModel::Nullable<Globals::Structs::PowerThresholdStruct::Type> mPowerThreshold;
};

} // namespace chip::app::Clusters
