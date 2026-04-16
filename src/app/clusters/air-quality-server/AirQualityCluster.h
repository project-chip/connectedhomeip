/*
 *
 *    Copyright (c) 2023-2026 Project CHIP Authors
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

namespace chip {
namespace app {
namespace Clusters {

class AirQualityCluster : public DefaultServerCluster
{
public:
    AirQualityCluster(EndpointId endpointId, BitFlags<AirQuality::Feature> features);
    ~AirQualityCluster() = default;

    // Server cluster implementation
    DataModel::ActionReturnStatus ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                AttributeValueEncoder & encoder) override;
    CHIP_ERROR Attributes(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder) override;

    bool HasFeature(AirQuality::Feature aFeature) const;
    Protocols::InteractionModel::Status SetAirQuality(AirQuality::AirQualityEnum aNewAirQuality);
    AirQuality::AirQualityEnum GetAirQuality() const;

private:
    AirQuality::AirQualityEnum mAirQuality = AirQuality::AirQualityEnum::kUnknown;
    const BitFlags<AirQuality::Feature> mFeature;
};

} // namespace Clusters
} // namespace app
} // namespace chip
