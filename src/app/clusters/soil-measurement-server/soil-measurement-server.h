/*
 *
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

#include <app/AttributeAccessInterface.h>
#include <clusters/SoilMeasurement/Attributes.h>
#include <lib/core/CHIPError.h>

namespace chip {
namespace app {
namespace Clusters {
namespace SoilMeasurement {

class Instance : public AttributeAccessInterface
{
public:
    /**
     * Creates a soil measurement cluster instance. The Init() function needs to be called for this instance to be registered and
     * called by the interaction model at the appropriate times.
     * @param aEndpointId The endpoint on which this cluster exists. This must match the zap configuration.
     */
    Instance(EndpointId aEndpointId);

    ~Instance() override;

    /**
     * Initialises the soil measurement cluster instance
     * @return Returns an error if an soil measurement cluster has not been enabled in zap for the given endpoint ID or
     * if the AttributeHandler registration fails.
     */
    CHIP_ERROR Init(const Globals::Structs::MeasurementAccuracyStruct::Type & measurementLimits);

    void Shutdown();

    CHIP_ERROR Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder) override;

    CHIP_ERROR SetSoilMeasuredValue(const Attributes::SoilMoistureMeasuredValue::TypeInfo::Type & soilMoistureMeasuredValue);

private:
    struct MeasurementData
    {
        Globals::Structs::MeasurementAccuracyStruct::Type soilMoistureMeasurementLimits;
        Attributes::SoilMoistureMeasuredValue::TypeInfo::Type soilMoistureMeasuredValue;
    };

    EndpointId mEndpointId = 1;
    MeasurementData mSoilMeasurementData;
};

} // namespace SoilMeasurement
} // namespace Clusters
} // namespace app
} // namespace chip
