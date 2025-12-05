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
#pragma once

#include <app/clusters/power-topology-server/PowerTopologyDelegate.h>

#include <app/util/af-types.h>
#include <lib/core/CHIPError.h>
#include <vector>

namespace chip {
namespace app {
namespace Clusters {
namespace PowerTopology {

class MockPowerTopologyDelegate : public Delegate
{
public:
    ~MockPowerTopologyDelegate() = default;

    CHIP_ERROR GetAvailableEndpointAtIndex(size_t index, EndpointId & endpointId) override;
    CHIP_ERROR GetActiveEndpointAtIndex(size_t index, EndpointId & endpointId) override;

    // Test helper methods
    void SetAvailableEndpoints(const std::vector<EndpointId> & endpoints) { mAvailableEndpoints = endpoints; }
    void SetActiveEndpoints(const std::vector<EndpointId> & endpoints) { mActiveEndpoints = endpoints; }
    void SetAvailableEndpointsError(CHIP_ERROR error) { mAvailableEndpointsError = error; }
    void SetActiveEndpointsError(CHIP_ERROR error) { mActiveEndpointsError = error; }
    void ClearAvailableEndpoints() { mAvailableEndpoints.clear(); }
    void ClearActiveEndpoints() { mActiveEndpoints.clear(); }

private:
    std::vector<EndpointId> mAvailableEndpoints;
    std::vector<EndpointId> mActiveEndpoints;
    CHIP_ERROR mAvailableEndpointsError = CHIP_NO_ERROR;
    CHIP_ERROR mActiveEndpointsError    = CHIP_NO_ERROR;
};

} // namespace PowerTopology
} // namespace Clusters
} // namespace app
} // namespace chip
