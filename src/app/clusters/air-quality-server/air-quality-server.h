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

#include <app/AttributeAccessInterface.h>

namespace chip {
namespace app {
namespace Clusters {
namespace AirQuality {

class Instance : public AttributeAccessInterface
{
public:
    /**
     * Creates an air quality cluster instance. The Init() function needs to be called for this instance to be registered and
     * called by the interaction model at the appropriate times.
     * @param aEndpointId The endpoint on which this cluster exists. This must match the zap configuration.
     * @param aFeature The bitmask value that identifies which features are supported by this instance.
     */
    Instance(EndpointId aEndpointId, BitMask<Feature> aFeature);

    ~Instance() override;

    /**
     * Initialises the air quality cluster instance
     * @return Returns an error if an air quality cluster has not been enabled in zap for the given endpoint ID or
     * if the AttributeHandler registration fails.
     */
    CHIP_ERROR Init();

    /**
     * Returns true if the feature is supported.
     * @param feature the feature to check.
     */
    bool HasFeature(Feature aFeature) const;

    /**
     * Sets the AirQuality attribute.
     * @param aNewAirQuality The value to which the AirQuality attribute is to be set.
     * @return Returns a ConstraintError if the aNewAirQuality value is not valid. Returns Success otherwise.
     */
    Protocols::InteractionModel::Status UpdateAirQuality(AirQualityEnum aNewAirQuality);

    /**
     * @return The current AirQuality enum.
     */
    AirQualityEnum GetAirQuality();

private:
    EndpointId mEndpointId;
    AirQualityEnum mAirQuality = AirQualityEnum::kUnknown;
    BitMask<Feature> mFeature;

    // AttributeAccessInterface
    CHIP_ERROR Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder) override;
};

} // namespace AirQuality
} // namespace Clusters
} // namespace app
} // namespace chip
