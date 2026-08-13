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

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app/server-cluster/DefaultServerCluster.h>
#include <app/server-cluster/OptionalAttributeSet.h>
#include <clusters/TemperatureMeasurement/Attributes.h>
#include <clusters/TemperatureMeasurement/Metadata.h>

namespace chip::app::Clusters {

class TemperatureMeasurementCluster : public DefaultServerCluster
{
public:
    using OptionalAttributeSet = app::OptionalAttributeSet<TemperatureMeasurement::Attributes::Tolerance::Id>;

    struct StartupConfiguration
    {
        DataModel::Nullable<int16_t> minMeasuredValue{};
        DataModel::Nullable<int16_t> maxMeasuredValue{};
        uint16_t tolerance{};
    };

    TemperatureMeasurementCluster(EndpointId endpointId, const OptionalAttributeSet & optionalAttributeSet,
                                  const StartupConfiguration & config);

    // Server cluster implementation
    DataModel::ActionReturnStatus ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                AttributeValueEncoder & encoder) override;
    CHIP_ERROR Attributes(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder) override;

    CHIP_ERROR SetMeasuredValue(DataModel::Nullable<int16_t> measuredValue);
    DataModel::Nullable<int16_t> GetMeasuredValue() const { return mMeasuredValue; }

    CHIP_ERROR SetMeasuredValueRange(DataModel::Nullable<int16_t> minMeasuredValue, DataModel::Nullable<int16_t> maxMeasuredValue);
    DataModel::Nullable<int16_t> GetMinMeasuredValue() const { return mMinMeasuredValue; }
    DataModel::Nullable<int16_t> GetMaxMeasuredValue() const { return mMaxMeasuredValue; }

protected:
    const OptionalAttributeSet mOptionalAttributeSet;
    DataModel::Nullable<int16_t> mMeasuredValue{};
    DataModel::Nullable<int16_t> mMinMeasuredValue{};
    DataModel::Nullable<int16_t> mMaxMeasuredValue{};
    uint16_t mTolerance{};
};

} // namespace chip::app::Clusters
