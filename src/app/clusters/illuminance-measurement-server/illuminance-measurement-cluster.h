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

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app/server-cluster/DefaultServerCluster.h>
#include <app/server-cluster/OptionalAttributeSet.h>
#include <clusters/IlluminanceMeasurement/Metadata.h>
#include <platform/DeviceInfoProvider.h>

namespace chip::app::Clusters {

using namespace chip::app::Clusters::IlluminanceMeasurement;
using namespace chip::app::Clusters::IlluminanceMeasurement::Attributes;

class IlluminanceMeasurementCluster : public DefaultServerCluster
{
public:
    // NOTE: this set is smaller than the full optional attributes supported by illuminance measurement
    //       as other attributes are controlled by feature flags
    using OptionalAttributeSet = app::OptionalAttributeSet<LightSensorType::Id, Tolerance::Id>;

    struct StartupConfiguration
    {
        DataModel::Nullable<LightSensorTypeEnum> lightSensorType{ LightSensorTypeEnum::kPhotodiode };
        Tolerance::TypeInfo::Type tolerance{};
    };

    IlluminanceMeasurementCluster(EndpointId endpointId, const OptionalAttributeSet & optionalAttributeSet,
                                  const StartupConfiguration & config);

    // Server cluster implementation
    DataModel::ActionReturnStatus ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                AttributeValueEncoder & encoder) override;
    CHIP_ERROR Attributes(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder) override;

    DataModel::Nullable<LightSensorTypeEnum> GetLightSensorType() const { return mLightSensorType; }

    MaxMeasuredValue::TypeInfo::Type GetMaxMeasuredValue() const { return mMaxMeasuredValue; }

    MeasuredValue::TypeInfo::Type GetMeasuredValue() const { return mMeasuredValue; }

    MinMeasuredValue::TypeInfo::Type GetMinMeasuredValue() const { return mMinMeasuredValue; }

    Tolerance::TypeInfo::Type GetTolerance() const { return mTolerance; }

protected:
    DataModel::Nullable<LightSensorTypeEnum> mLightSensorType{};
    MaxMeasuredValue::TypeInfo::Type mMaxMeasuredValue{};
    MeasuredValue::TypeInfo::Type mMeasuredValue{};
    MinMeasuredValue::TypeInfo::Type mMinMeasuredValue{};
    Tolerance::TypeInfo::Type mTolerance{};
    const OptionalAttributeSet mOptionalAttributeSet;
};

} // namespace chip::app::Clusters
