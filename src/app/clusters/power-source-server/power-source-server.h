/*
 *
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
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

private:
    // Both return std::numeric_limits<size_t>::max() for not found
    size_t PowerSourceClusterEndpointIndex(EndpointId endpointId) const;
    size_t NextEmptyIndex() const;
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
