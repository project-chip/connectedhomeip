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

inline constexpr uint16_t kClusterRevision              = 1;
inline constexpr uint8_t kMinSupportedLocalizationUnits = 2;
inline constexpr uint8_t kMaxSupportedLocalizationUnits = 3;

class SoilMeasurementServer : public AttributeAccessInterface
{
public:
    // Register for the SoilMeasurement cluster on all endpoints.
    static SoilMeasurementServer & Instance();

    CHIP_ERROR Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder) override;

private:
    SoilMeasurementServer() : AttributeAccessInterface(Optional<EndpointId>::Missing(), SoilMeasurement::Id) {}

    CHIP_ERROR ReadClusterRevision(AttributeValueEncoder & aEncoder);

    static SoilMeasurementServer mInstance;
};

} // namespace SoilMeasurement
} // namespace Clusters
} // namespace app
} // namespace chip
