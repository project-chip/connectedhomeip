/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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
#pragma once

#include <app/clusters/meter-identification-server/meter-identification-server.h>
#include <app/util/af-types.h>
#include <json/value.h>
#include <lib/core/CHIPError.h>

namespace chip {
namespace app {
namespace Clusters {
namespace MeterIdentification {

class MeterIdentificationDelegate : public MeterIdentification::Delegate
{
    // Attribute storage
    DataModel::Nullable<MeterTypeEnum> mMeterType;
    DataModel::Nullable<CharSpan> mPointOfDelivery;
    DataModel::Nullable<CharSpan> mMeterSerialNumber;
    DataModel::Nullable<CharSpan> mProtocolVersion;
    DataModel::Nullable<Structs::PowerThresholdStruct::Type> mPowerThreshold;

private:

    static bool NullableCharSpanCompare(const DataModel::Nullable<CharSpan> & a, const DataModel::Nullable<CharSpan> & b);
    static CHIP_ERROR NullableCharSpanCopy(const DataModel::Nullable<CharSpan> & src, DataModel::Nullable<CharSpan> & dst);
    static CHIP_ERROR JsonToPowerThreshold(const Json::Value & root, Structs::PowerThresholdStruct::Type & value);

public:

    virtual ~MeterIdentificationDelegate();

    void Init();

    CHIP_ERROR LoadJson(const Json::Value & root);

    // Attribute Accessors

    DataModel::Nullable<MeterTypeEnum> GetMeterType() override { return mMeterType; }
    DataModel::Nullable<CharSpan> GetPointOfDelivery() override { return mPointOfDelivery; }
    DataModel::Nullable<CharSpan> GetMeterSerialNumber() override { return mMeterSerialNumber; }
    DataModel::Nullable<CharSpan> GetProtocolVersion() override { return mProtocolVersion; }
    DataModel::Nullable<Structs::PowerThresholdStruct::Type> GetPowerThreshold() override { return mPowerThreshold; }

    // Internal Application API to set attribute values
    CHIP_ERROR SetMeterType(const DataModel::Nullable<MeterTypeEnum> & value);
    CHIP_ERROR SetPointOfDelivery(const DataModel::Nullable<CharSpan> & value);
    CHIP_ERROR SetMeterSerialNumber(const DataModel::Nullable<CharSpan> & value);
    CHIP_ERROR SetProtocolVersion(const DataModel::Nullable<CharSpan> & value);
    CHIP_ERROR SetPowerThreshold(const DataModel::Nullable<Structs::PowerThresholdStruct::Type> & value);
};

class MeterIdentificationInstance : public Instance
{
    MeterIdentificationDelegate * mDelegate;

public:

    MeterIdentificationInstance(EndpointId aEndpointId, MeterIdentificationDelegate & aDelegate, const Feature & aFeature) :
        MeterIdentification::Instance(aEndpointId, aDelegate, aFeature), mDelegate(&aDelegate) {}

    // Delete copy constructor and assignment operator.
    MeterIdentificationInstance(const MeterIdentificationInstance &)             = delete;
    MeterIdentificationInstance(const MeterIdentificationInstance &&)            = delete;
    MeterIdentificationInstance & operator=(const MeterIdentificationInstance &) = delete;

    CHIP_ERROR Init();
    void Shutdown();

    MeterIdentificationDelegate * GetDelegate() { return mDelegate; };
};

MeterIdentificationDelegate * GetDelegate();

} // namespace MeterIdentification
} // namespace Clusters
} // namespace app
} // namespace chip
