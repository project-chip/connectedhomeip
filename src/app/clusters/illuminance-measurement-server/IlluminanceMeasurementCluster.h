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
#include <clusters/IlluminanceMeasurement/Metadata.h>
#include <platform/DeviceInfoProvider.h>

namespace chip::app::Clusters {

class IlluminanceMeasurementCluster : public DefaultServerCluster
{
public:
    using OptionalAttributeSet = app::OptionalAttributeSet<IlluminanceMeasurement::Attributes::LightSensorType::Id,
                                                           IlluminanceMeasurement::Attributes::Tolerance::Id>;

    using LightSensorTypeType  = IlluminanceMeasurement::Attributes::LightSensorType::TypeInfo::Type;
    using ToleranceType        = IlluminanceMeasurement::Attributes::Tolerance::TypeInfo::Type;
    using MeasuredValueType    = IlluminanceMeasurement::Attributes::MeasuredValue::TypeInfo::Type;
    using MinMeasuredValueType = IlluminanceMeasurement::Attributes::MinMeasuredValue::TypeInfo::Type;
    using MaxMeasuredValueType = IlluminanceMeasurement::Attributes::MaxMeasuredValue::TypeInfo::Type;

    struct StartupConfiguration
    {
        MinMeasuredValueType minMeasuredValue{};
        MaxMeasuredValueType maxMeasuredValue{};
        ToleranceType tolerance{};
        LightSensorTypeType lightSensorType{};
    };

    IlluminanceMeasurementCluster(EndpointId endpointId, const OptionalAttributeSet & optionalAttributeSet,
                                  const StartupConfiguration & config);

    // Server cluster implementation
    DataModel::ActionReturnStatus ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                AttributeValueEncoder & encoder) override;
    CHIP_ERROR Attributes(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder) override;

    CHIP_ERROR SetMeasuredValue(MeasuredValueType measuredValue);
    MeasuredValueType GetMeasuredValue() const { return mMeasuredValue; }

protected:
    MeasuredValueType mMeasuredValue{};
    const OptionalAttributeSet mOptionalAttributeSet;
    const StartupConfiguration mConfig;
};

} // namespace chip::app::Clusters
