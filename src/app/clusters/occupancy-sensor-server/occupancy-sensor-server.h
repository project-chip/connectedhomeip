/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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
#include <app-common/zap-generated/cluster-objects.h>
#include <app/AttributeAccessInterface.h>
#include <app/util/af-types.h>
#include <app/util/basic-types.h>
#include <app/util/config.h>
#include <lib/core/DataModelTypes.h>


namespace chip {
namespace app {
namespace Clusters {
namespace OccupancySensing {

/**
 * Interface to help manage the Hold Time Limits of the Occupancy Sensing Cluster.
 */
class HoldTimeLimitsManager : public AttributeAccessInterface
{
public:
	static constexpr size_t kOccupancySensingServerMaxEndpointCount =
        MATTER_DM_OCCUPANCY_SENSING_CLUSTER_SERVER_ENDPOINT_COUNT + CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT;
    static_assert(kOccupancySensingServerMaxEndpointCount <= kInvalidEndpointId, "Occupancy Sensing endpoint count error");

	//HoldTimeLimits
    class HoldTimeLimits
    {
    public:
		Structs::HoldTimeLimitsStruct::Type * GetHoldTimeLimitsStruct(EndpointId endpoint);
        CHIP_ERROR SetHoldTimeLimitsStruct(EndpointId endpoint, Structs::HoldTimeLimitsStruct::Type & holdTimeLimitsStruct);

    private:
		/// @brief Returns the index of the HoldTimeLimits associated to an endpoint
        /// @param[in] endpoint target endpoint
        /// @param[out] endpointIndex index of the corresponding HoldTimeLimits for an endpoint
        /// @return CHIP_NO_ERROR or CHIP_ERROR_NOT_FOUND, CHIP_ERROR_INVALID_ARGUMENT if invalid endpoint
        CHIP_ERROR FindHoldTimeLimitsIndex(EndpointId endpoint, size_t & endpointIndex);

        Structs::HoldTimeLimitsStruct::Type mHoldTimeLimitsStructs[kOccupancySensingServerMaxEndpointCount];
    };

	static HoldTimeLimitsManager & Instance();

	CHIP_ERROR Init();

	// AttributeAccessInterface
    CHIP_ERROR Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder) override;

	// HoldTimeLimitsStruct Accessors
    Structs::HoldTimeLimitsStruct::Type * GetHoldTimeLimitsStruct(EndpointId endpoint);
    CHIP_ERROR SetHoldTimeLimitsStruct(EndpointId endpoint, Structs::HoldTimeLimitsStruct::Type & holdTimeLimitsStruct);

private:
    HoldTimeLimitsManager() : AttributeAccessInterface(Optional<EndpointId>(), Id) {}
    ~HoldTimeLimitsManager() { }

    bool mIsInitialized = false;

	// HoldTimeLimits
    HoldTimeLimits mHoldTimeLimits;

	// Instance
    static HoldTimeLimitsManager mInstance;
};

inline bool HasFeature(EndpointId ep, Feature feature)
{
    uint32_t map;
    bool success = (Attributes::FeatureMap::Get(ep, &map) == Protocols::InteractionModel::Status::Success);
    return success ? (map & to_underlying(feature)) : false;
}

/** @brief Occupancy Cluster Server Post Init
 *
 * Following resolution of the Occupancy state at startup for this endpoint,
 * perform any additional initialization needed; e.g., synchronize hardware
 * state.
 *
 * @param endpoint Endpoint that is being initialized  Ver.: always
 */
void emberAfPluginOccupancyClusterServerPostInitCallback(chip::EndpointId endpoint);

} // namespace OccupancySensing
} // namespace Clusters
} // namespace app
} // namespace chip