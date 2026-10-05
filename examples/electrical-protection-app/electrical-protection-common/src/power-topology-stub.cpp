/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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

#include <power-topology-stub.h>

#include <app/server/Server.h>
#include <lib/support/CodeUtils.h>

#include <algorithm>
#include <array>
#include <memory>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;

namespace {

// Neither accessor is reachable: AvailableEndpoints is gated on SET and ActiveEndpoints on DYPF,
// and this app enables neither on either endpoint. The delegate holds no state, so both
// instances share one.
class StubTopologyDelegate : public PowerTopology::Delegate
{
public:
    CHIP_ERROR GetAvailableEndpointAtIndex(size_t index, EndpointId & endpointId) override
    {
        return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
    }

    CHIP_ERROR GetActiveEndpointAtIndex(size_t index, EndpointId & endpointId) override
    {
        return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
    }
};

StubTopologyDelegate gDelegate;

// One per endpoint carrying the cluster: the enclosure and the breaker.
constexpr size_t kMaxInstances = 2;
std::array<std::unique_ptr<PowerTopology::Instance>, kMaxInstances> gInstances;

} // namespace

namespace chip::app::Clusters::PowerTopology {

CHIP_ERROR PowerTopologyInit(EndpointId endpointId, BitMask<Feature> features)
{
    auto slot = std::find_if(gInstances.begin(), gInstances.end(), [](const auto & i) { return i == nullptr; });
    VerifyOrReturnError(slot != gInstances.end(), CHIP_ERROR_NO_MEMORY);

    // ElectricalCircuitNodes is fabric-scoped, so the cluster needs the fabric table to serve it.
    // The constructor defaults this to nullptr, and Startup() then fails with INCORRECT_STATE
    // whenever CIRC is set, so pass it unconditionally.
    *slot = std::make_unique<Instance>(endpointId, gDelegate, features, &Server::GetInstance().GetFabricTable());
    VerifyOrReturnError(*slot != nullptr, CHIP_ERROR_NO_MEMORY);

    CHIP_ERROR err = (*slot)->Init();
    if (err != CHIP_NO_ERROR)
    {
        slot->reset();
    }
    return err;
}

void PowerTopologyShutdown()
{
    // ~Instance() unregisters.
    for (auto & instance : gInstances)
    {
        instance.reset();
    }
}

} // namespace chip::app::Clusters::PowerTopology
