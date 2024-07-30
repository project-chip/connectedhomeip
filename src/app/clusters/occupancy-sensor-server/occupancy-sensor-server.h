/*
 *
 *    Copyright (c) 2020-2024 Project CHIP Authors
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

class OccupancySensingAttrAccess : public AttributeAccessInterface
{
public:
    OccupancySensingAttrAccess(BitMask<Feature> aFeature) :
        app::AttributeAccessInterface(Optional<EndpointId>::Missing(), app::Clusters::OccupancySensing::Id), mFeature(aFeature)
    {}

    ~OccupancySensingAttrAccess() { Shutdown(); }

    CHIP_ERROR Init();
    void Shutdown();

    CHIP_ERROR Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder) override;

    bool HasFeature(Feature aFeature) const;

private:
    BitMask<Feature> mFeature;
};

CHIP_ERROR SetHoldTimeLimits(EndpointId endpointId, const Structs::HoldTimeLimitsStruct::Type & holdTimeLimits);

CHIP_ERROR SetHoldTime(EndpointId endpointId, const uint16_t & holdTime);

Structs::HoldTimeLimitsStruct::Type * GetHoldTimeLimitsForEndpoint(EndpointId endpoint);

uint16_t * GetHoldTimeForEndpoint(EndpointId endpoint);

} // namespace OccupancySensing
} // namespace Clusters
} // namespace app
} // namespace chip
