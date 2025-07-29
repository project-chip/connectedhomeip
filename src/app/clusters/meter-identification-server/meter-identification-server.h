/**
 *
 *    Copyright (c) 2024 Project CHIP Authors
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

#include <stddef.h>

#include <lib/core/Optional.h>

#include <app-common/zap-generated/cluster-enums.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <app/AttributeAccessInterface.h>
#include <app/util/basic-types.h>
#include <lib/core/CHIPError.h>

namespace chip {
namespace app {
namespace Clusters {
namespace MeterIdentification {

class Instance : public AttributeAccessInterface
{
public:
    Instance(const EndpointId & aEndpointId, const BitMask<Feature> & aFeature) :
        AttributeAccessInterface(MakeOptional(aEndpointId), Id), mEndpointId(aEndpointId), mFeature(aFeature)
    {}
    ~Instance() override;

    CHIP_ERROR Init();
    void Shutdown();

    bool HasFeature(const Feature & aFeature) const;

    // Attribute Accessors
    const DataModel::Nullable<MeterTypeEnum> & GetMeterType() const { return mMeterType; }
    const DataModel::Nullable<CharSpan> & GetPointOfDelivery() const { return mPointOfDelivery; }
    const DataModel::Nullable<CharSpan> & GetMeterSerialNumber() const { return mMeterSerialNumber; }
    const DataModel::Nullable<CharSpan> & GetProtocolVersion() const { return mProtocolVersion; }
    const DataModel::Nullable<Globals::Structs::PowerThresholdStruct::Type> & GetPowerThreshold() const { return mPowerThreshold; }

    // Internal Application API to set attribute values
    CHIP_ERROR SetMeterType(const DataModel::Nullable<MeterTypeEnum> & value);
    CHIP_ERROR SetPointOfDelivery(const DataModel::Nullable<CharSpan> & value);
    CHIP_ERROR SetMeterSerialNumber(const DataModel::Nullable<CharSpan> & value);
    CHIP_ERROR SetProtocolVersion(const DataModel::Nullable<CharSpan> & value);
    CHIP_ERROR SetPowerThreshold(const DataModel::Nullable<Globals::Structs::PowerThresholdStruct::Type> & value);

private:
    // The maximum string size of 64 bytes is specified in:
    // Matter Application Cluster Specification - Meter Identification Cluster
    static constexpr size_t kMaximumStringSize = 64;

    // Attribute storage
    char mPointOfDeliveryBuf[kMaximumStringSize]   = {};
    char mMeterSerialNumberBuf[kMaximumStringSize] = {};
    char mProtocolVersionBuf[kMaximumStringSize]   = {};
    DataModel::Nullable<MeterTypeEnum> mMeterType;
    DataModel::Nullable<CharSpan> mPointOfDelivery;
    DataModel::Nullable<CharSpan> mMeterSerialNumber;
    DataModel::Nullable<CharSpan> mProtocolVersion;
    DataModel::Nullable<Globals::Structs::PowerThresholdStruct::Type> mPowerThreshold;
    EndpointId mEndpointId = 0;
    BitMask<Feature> mFeature;

    // AttributeAccessInterface
    CHIP_ERROR Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder) override;
};

} // namespace MeterIdentification
} // namespace Clusters
} // namespace app
} // namespace chip
