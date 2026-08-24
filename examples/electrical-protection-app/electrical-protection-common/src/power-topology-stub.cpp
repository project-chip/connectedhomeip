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

#include <lib/support/CodeUtils.h>

#include <memory>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;

namespace {

// TreeTopology gates none of this cluster's attributes: AvailableEndpoints is SET, ActiveEndpoints
// is DYPF and ElectricalCircuitNodes is CIRC. Neither accessor is reachable, so both report an
// empty list rather than inventing endpoints this app does not model.
class EnclosureTopologyDelegate : public PowerTopology::Delegate
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

EnclosureTopologyDelegate gDelegate;
std::unique_ptr<PowerTopology::Instance> gInstance;

} // namespace

namespace chip::app::Clusters::PowerTopology {

CHIP_ERROR PowerTopologyInit(EndpointId endpointId)
{
    VerifyOrReturnError(gInstance == nullptr, CHIP_ERROR_INCORRECT_STATE);

    gInstance = std::make_unique<Instance>(endpointId, gDelegate, BitMask<Feature>(Feature::kTreeTopology));
    VerifyOrReturnError(gInstance != nullptr, CHIP_ERROR_NO_MEMORY);

    CHIP_ERROR err = gInstance->Init();
    if (err != CHIP_NO_ERROR)
    {
        gInstance.reset();
    }
    return err;
}

void PowerTopologyShutdown()
{
    // ~Instance() unregisters.
    gInstance.reset();
}

} // namespace chip::app::Clusters::PowerTopology
