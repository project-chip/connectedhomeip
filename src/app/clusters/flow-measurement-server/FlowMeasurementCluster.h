/*
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
#pragma once

#include <app/server-cluster/DefaultServerCluster.h>
#include <app/server-cluster/OptionalAttributeSet.h>
#include <clusters/FlowMeasurement/Attributes.h>
#include <clusters/FlowMeasurement/Metadata.h>

namespace chip::app::Clusters {

class FlowMeasurementCluster : public DefaultServerCluster
{
public:
    using OptionalAttributeSet = app::OptionalAttributeSet<FlowMeasurement::Attributes::Tolerance::Id>;

    struct Config
    {
        Config() : minMeasuredValue(), maxMeasuredValue(), mOptionalAttributeSet(), mTolerance(0) {}

        Config & WithTolerance(uint16_t value)
        {
            mTolerance = value;
            mOptionalAttributeSet.template Set<FlowMeasurement::Attributes::Tolerance::Id>();
            return *this;
        }

        DataModel::Nullable<uint16_t> minMeasuredValue;
        DataModel::Nullable<uint16_t> maxMeasuredValue;
        OptionalAttributeSet mOptionalAttributeSet;
        uint16_t mTolerance;
    };

    explicit FlowMeasurementCluster(EndpointId endpointId);
    FlowMeasurementCluster(EndpointId endpointId, const Config & config);

    DataModel::ActionReturnStatus ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                AttributeValueEncoder & encoder) override;
    CHIP_ERROR Attributes(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder) override;

    CHIP_ERROR SetMeasuredValue(DataModel::Nullable<uint16_t> measuredValue);
    DataModel::Nullable<uint16_t> GetMeasuredValue() const { return mMeasuredValue; }

    DataModel::Nullable<uint16_t> GetMinMeasuredValue() const { return mMinMeasuredValue; }
    DataModel::Nullable<uint16_t> GetMaxMeasuredValue() const { return mMaxMeasuredValue; }

protected:
    CHIP_ERROR SetMeasuredValueRange(DataModel::Nullable<uint16_t> minMeasuredValue,
                                     DataModel::Nullable<uint16_t> maxMeasuredValue);
    const OptionalAttributeSet mOptionalAttributeSet;
    DataModel::Nullable<uint16_t> mMeasuredValue{};
    DataModel::Nullable<uint16_t> mMinMeasuredValue{};
    DataModel::Nullable<uint16_t> mMaxMeasuredValue{};
    uint16_t mTolerance{};
};

} // namespace chip::app::Clusters
