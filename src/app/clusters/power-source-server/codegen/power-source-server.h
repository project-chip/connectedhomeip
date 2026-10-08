/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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

#include <app-common/zap-generated/ids/Clusters.h>
#include <app/AttributeAccessInterface.h>
#include <app/util/af-types.h>
#include <app/util/basic-types.h>
#include <lib/support/Span.h>
#include <platform/CHIPDeviceConfig.h>

namespace chip {
namespace app {
namespace Clusters {

class PowerSourceServer
{
public:
    static PowerSourceServer & Instance();

    // Caller does not need to retain the span past the call point as these are copied into an internal storage
    CHIP_ERROR SetEndpointList(EndpointId powerSourceClusterEndpoint, Span<EndpointId> endpointList);
    CHIP_ERROR ClearEndpointList(EndpointId powerSourceClusterEndpoint)
    {
        return SetEndpointList(powerSourceClusterEndpoint, Span<EndpointId>());
    }
    // returns nullptr if there's not endpoint list set for this power source cluster endpoint id.
    const Span<EndpointId> * GetEndpointList(EndpointId powerSourceClusterEndpoint) const;
    void Shutdown();
    size_t GetNumSupportedEndpointLists() const;
};

class PowerSourceAttrAccess : public AttributeAccessInterface
{
public:
    // Register on all endpoints.
    PowerSourceAttrAccess() : AttributeAccessInterface(Optional<EndpointId>::Missing(), PowerSource::Id) {}

    CHIP_ERROR Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder) override;
};

PowerSourceAttrAccess & TestOnlyGetPowerSourceAttrAccess();

} // namespace Clusters
} // namespace app
} // namespace chip
