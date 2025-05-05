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

#include <app-common/zap-generated/cluster-enums.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/AttributeAccessInterface.h>
#include <app/data-model/List.h>
#include <cstdint>
#include <lib/core/CHIPError.h>

namespace chip {
namespace app {
namespace Clusters {
namespace SoilMeasurement {

inline constexpr uint16_t kClusterRevision = 1;

struct MeasurementData
{
    Globals::Structs::MeasurementAccuracyStruct::Type soilMoistureMeasurementLimits;
    Attributes::SoilMoistureMeasuredValue::TypeInfo::Type soilMoistureMeasuredValue;
};

class SoilMeasurementAttrAccess : public AttributeAccessInterface
{
public:
    // Register for the SoilMeasurement on all endpoints.
    SoilMeasurementAttrAccess() : AttributeAccessInterface(Optional<EndpointId>::Missing(), Clusters::SoilMeasurement::Id) {}
    ~SoilMeasurementAttrAccess() { Shutdown(); }

    CHIP_ERROR Init();
    void Shutdown();

    CHIP_ERROR Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder) override;
};

CHIP_ERROR SetSoilMeasurementAccuracy(EndpointId endpointId, const Globals::Structs::MeasurementAccuracyStruct::Type & accuracy,
                                      bool reportChange);
CHIP_ERROR SetSoilMeasuredValue(EndpointId endpointId,
                                const Attributes::SoilMoistureMeasuredValue::TypeInfo::Type & soilMoistureMeasuredValue);

MeasurementData * SoilMeasurementDataForEndpoint(EndpointId endpointId);

} // namespace SoilMeasurement
} // namespace Clusters
} // namespace app
} // namespace chip
