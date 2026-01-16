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

namespace chip::app::Clusters {

class SoilMeasurementCluster : public DefaultServerCluster
{
public:
    SoilMeasurementCluster(
        EndpointId endpointId,
        const SoilMeasurement::Attributes::SoilMoistureMeasurementLimits::TypeInfo::Type & soilMoistureMeasurementLimits);

    // Server cluster implementation
    DataModel::ActionReturnStatus ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                AttributeValueEncoder & encoder) override;
    CHIP_ERROR Attributes(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder) override;

    CHIP_ERROR
    SetSoilMoistureMeasuredValue(
        const SoilMeasurement::Attributes::SoilMoistureMeasuredValue::TypeInfo::Type & soilMoistureMeasuredValue);

protected:
    const SoilMeasurement::Attributes::SoilMoistureMeasurementLimits::TypeInfo::Type mSoilMoistureMeasurementLimits;
    SoilMeasurement::Attributes::SoilMoistureMeasuredValue::TypeInfo::Type mSoilMoistureMeasuredValue;
};

} // namespace chip::app::Clusters
