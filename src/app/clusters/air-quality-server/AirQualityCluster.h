/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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

/**
 * Code-driven implementation of the Matter Air Quality cluster.
 * Manages a single AirQuality attribute with feature-gated validation.
 * Instances are created by the framework via CodegenIntegration.
 */
class AirQualityCluster : public DefaultServerCluster
{
public:
    /**
     * @param endpointId The endpoint on which this cluster exists.
     */
    AirQualityCluster(EndpointId endpointId);

    // Server cluster implementation
    DataModel::ActionReturnStatus ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                AttributeValueEncoder & encoder) override;
    CHIP_ERROR Attributes(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder) override;

    /**
     * Returns true if the feature is supported.
     * @param feature the feature to check.
     */
    bool HasFeature(AirQuality::Feature aFeature) const;

    /**
     * Sets the AirQuality attribute.
     * @param aNewAirQuality The value to which the AirQuality attribute is to be set.
     * @return Returns a ConstraintError if the aNewAirQuality value is not valid. Returns Success otherwise.
     */
    Protocols::InteractionModel::Status SetAirQuality(AirQuality::AirQualityEnum aNewAirQuality);

    // Backward compatibility alias
    inline Protocols::InteractionModel::Status UpdateAirQuality(AirQuality::AirQualityEnum aNewAirQuality)
    {
        return SetAirQuality(aNewAirQuality);
    }

    /**
     * @return The current AirQuality enum.
     */
    AirQuality::AirQualityEnum GetAirQuality() const;

    /**
     * Sets the feature map for this cluster instance.
     * Called by CodegenIntegration during creation.
     */
    void SetFeatureMap(BitFlags<AirQuality::Feature> features);

protected:
    AirQuality::AirQualityEnum mAirQuality = AirQuality::AirQualityEnum::kUnknown;
    BitFlags<AirQuality::Feature> mFeature;
};

namespace AirQuality {

// Backward compatibility alias
using Instance = AirQualityCluster;

} // namespace AirQuality
} // namespace Clusters
} // namespace app
} // namespace chip
