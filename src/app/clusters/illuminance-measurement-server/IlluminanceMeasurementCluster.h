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
#include <clusters/IlluminanceMeasurement/Attributes.h>
#include <clusters/IlluminanceMeasurement/Metadata.h>

namespace chip::app::Clusters {

class IlluminanceMeasurementCluster : public DefaultServerCluster
{
public:
    using OptionalAttributeSet = app::OptionalAttributeSet<IlluminanceMeasurement::Attributes::Tolerance::Id,
                                                           IlluminanceMeasurement::Attributes::LightSensorType::Id>;

    struct StartupConfiguration
    {
        DataModel::Nullable<uint16_t> minMeasuredValue{};
        DataModel::Nullable<uint16_t> maxMeasuredValue{};
        uint16_t tolerance{};
        DataModel::Nullable<IlluminanceMeasurement::LightSensorTypeEnum> lightSensorType{};
    };

    IlluminanceMeasurementCluster(EndpointId endpointId, const OptionalAttributeSet & optionalAttributeSet,
                                  const StartupConfiguration & config);

    // Server cluster implementation
    DataModel::ActionReturnStatus ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                AttributeValueEncoder & encoder) override;
    CHIP_ERROR Attributes(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder) override;

    CHIP_ERROR SetMeasuredValue(DataModel::Nullable<uint16_t> measuredValue);
    DataModel::Nullable<uint16_t> GetMeasuredValue() const { return mMeasuredValue; }

protected:
    DataModel::Nullable<uint16_t> mMeasuredValue{};
    const OptionalAttributeSet mOptionalAttributeSet;
    const StartupConfiguration mConfig;
};

} // namespace chip::app::Clusters
