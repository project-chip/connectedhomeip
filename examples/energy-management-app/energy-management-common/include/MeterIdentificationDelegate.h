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
#include <lib/core/CHIPError.h>

namespace chip {
namespace app {
namespace Clusters {
namespace MeterIdentification {

class MeterIdentificationDelegate : public MeterIdentification::Delegate
{
public:
    ~MeterIdentificationDelegate() = default;

    // Attribute Accessors

    DataModel::Nullable<MeterTypeEnum> GetMeterType() override { return mMeterType; };
    CharSpan GetCustomerName() override { return mCustomerName; };
    CharSpan GetUtilityName() override { return mUtilityName; };
    CharSpan GetPointOfDelivery() override { return mPointOfDelivery; };
    DataModel::Nullable<uint64_t> GetPowerThreshold() override { return mPowerThreshold; };
    DataModel::Nullable<PowerThresholdSourceEnum> GetPowerThresholdSource() override { return mPowerThresholdSource; };

    CHIP_ERROR SetCustomerName(CharSpan & value) override;

    // Internal Application API to set attribute values
    CHIP_ERROR SetMeterType(DataModel::Nullable<MeterTypeEnum>);
    CHIP_ERROR SetUtilityName(CharSpan & value);
    CHIP_ERROR SetPointOfDelivery(CharSpan & value);
    CHIP_ERROR SetPowerThreshold(DataModel::Nullable<uint64_t>);
    CHIP_ERROR SetPowerThresholdSource(DataModel::Nullable<PowerThresholdSourceEnum>);

private:
    // Attribute storage
    DataModel::Nullable<MeterTypeEnum> mMeterType;
    CharSpan mCustomerName;
    CharSpan mUtilityName;
    CharSpan mPointOfDelivery;
    DataModel::Nullable<uint64_t> mPowerThreshold;
    DataModel::Nullable<PowerThresholdSourceEnum> mPowerThresholdSource;
};

class MeterIdentificationInstance : public Instance
{
public:
    MeterIdentificationInstance(EndpointId aEndpointId, MeterIdentificationDelegate & aDelegate, Feature aFeature) :
        MeterIdentification::Instance(aEndpointId, aDelegate, aFeature)
    {
        mDelegate = &aDelegate;
    }

    // Delete copy constructor and assignment operator.
    MeterIdentificationInstance(const MeterIdentificationInstance &)             = delete;
    MeterIdentificationInstance(const MeterIdentificationInstance &&)            = delete;
    MeterIdentificationInstance & operator=(const MeterIdentificationInstance &) = delete;

    CHIP_ERROR Init();
    void Shutdown();

    MeterIdentificationDelegate * GetDelegate() { return mDelegate; };

private:
    MeterIdentificationDelegate * mDelegate;
};

} // namespace MeterIdentification
} // namespace Clusters
} // namespace app
} // namespace chip
