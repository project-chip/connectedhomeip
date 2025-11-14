/*
 *    Copyright (c) 2025 Project CHIP Authors
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
#include <platform/DeviceInfoProvider.h>

namespace chip::app::Clusters::IlluminanceMeasurement {

class IlluminanceMeasurementCluster : public DefaultServerCluster
{
public:
    IlluminanceMeasurementCluster(EndpointId endpointId);

    // Server cluster implementation
    DataModel::ActionReturnStatus ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                AttributeValueEncoder & encoder) override;
    CHIP_ERROR Attributes(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder) override;

    DataModel::Nullable<chip::app::Clusters::IlluminanceMeasurement::LightSensorTypeEnum> GetLightSensorType() const { return mStateValue; }

    uint16_t GetMaxMeasuredValue() const { return MaxMeasuredValue; }

    uint16_t GetMeasuredValue() const { return MeasuredValue; }

    uint16_t GetMinMeasuredValue() const { return MinMeasuredValue; }

    DataModel::Nullable<uint16_t> GetTolerance() const { return Tolerance; }

protected:
    DataModel::Nullable<chip::app::Clusters::IlluminanceMeasurement::LightSensorTypeEnum> LightSensorType;
    uint16_t MaxMeasuredValue;
    uint16_t MeasuredValue;
    uint16_t MinMeasuredValue;
    DataModel::Nullable<uint16_t> Tolerance;
};

} // namespace chip::app::Clusters::IlluminanceMeasurement