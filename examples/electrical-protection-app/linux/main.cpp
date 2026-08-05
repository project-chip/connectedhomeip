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
#include <AppMain.h>
#include <PowerTopologyDelegateImpl.h>
#include <electrical-distribution-stub.h>
#include <electrical-protection-alarm-stub.h>

#include <app-common/zap-generated/ids/Clusters.h>
#include <app/server/Server.h>
#include <app/util/attribute-storage.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>

#include <memory>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;

namespace {

// The Electrical Circuit Breaker component endpoint carries a Common Number semantic tag.
// Common Number namespace: 0x07. See
// https://github.com/CHIP-Specifications/connectedhomeip-spec/blob/master/src/namespaces
constexpr uint8_t kNamespaceCommonNumber                                       = 0x07;
constexpr uint8_t kTagBreakerNumberZero                                        = 0x00;
const Clusters::Descriptor::Structs::SemanticTagStruct::Type kBreakerTagList[] = { { .namespaceID = kNamespaceCommonNumber,
                                                                                     .tag         = kTagBreakerNumberZero } };

constexpr EndpointId kEnclosureEndpointId = 1; // Electrical Distribution Enclosure (0x0517)
constexpr EndpointId kBreakerEndpointId   = 2; // Electrical Circuit Breaker (0x0516)

// Power Topology is a code-driven cluster registered imperatively (its Init callback is empty),
// so the app owns the delegate + instance lifetime for each endpoint that hosts it.
std::unique_ptr<PowerTopology::PowerTopologyDelegate> gEnclosurePtDelegate;
std::unique_ptr<PowerTopology::PowerTopologyInstance> gEnclosurePtInstance;
std::unique_ptr<PowerTopology::PowerTopologyDelegate> gBreakerPtDelegate;
std::unique_ptr<PowerTopology::PowerTopologyInstance> gBreakerPtInstance;

} // namespace

void ApplicationInit()
{
    // Compose the enclosure (EP1) as a tree parent of the breaker (EP2), and tag the breaker.
    LogErrorOnFailure(SetTreeCompositionForEndpoint(kEnclosureEndpointId));
    LogErrorOnFailure(SetParentEndpointForEndpoint(kBreakerEndpointId, kEnclosureEndpointId));
    LogErrorOnFailure(
        SetTagList(kBreakerEndpointId, Span<const Clusters::Descriptor::Structs::SemanticTagStruct::Type>(kBreakerTagList)));

    // Enclosure: base Power Topology (TREE - the enclosure feeds its child breaker endpoints).
    VerifyOrDie(PowerTopology::PowerTopologyInit(kEnclosureEndpointId, gEnclosurePtDelegate, gEnclosurePtInstance,
                                                 BitMask<PowerTopology::Feature>(PowerTopology::Feature::kTreeTopology)) ==
                CHIP_NO_ERROR);

    // Breaker: Power Topology with the ElectricalCircuit (CIRC) feature -> the ElectricalCircuitNodes
    // attribute. The fabric table lets the cluster purge a removed fabric's fabric-scoped nodes.
    VerifyOrDie(PowerTopology::PowerTopologyInit(kBreakerEndpointId, gBreakerPtDelegate, gBreakerPtInstance,
                                                 BitMask<PowerTopology::Feature>(PowerTopology::Feature::kNodeTopology,
                                                                                 PowerTopology::Feature::kElectricalCircuit),
                                                 &Server::GetInstance().GetFabricTable()) == CHIP_NO_ERROR);

    // Electrical Distribution describes the enclosure's physical characteristics. Like Power
    // Topology above, its generated Init callback is a no-op and the app owns the instance.
    VerifyOrDie(ElectricalDistribution::ElectricalDistributionInit(kEnclosureEndpointId) == CHIP_NO_ERROR);

    // Electrical Protection Alarm reports the breaker's safety faults. Same imperative
    // registration: its generated Init callback is a no-op.
    VerifyOrDie(ElectricalProtectionAlarm::ElectricalProtectionAlarmInit(kBreakerEndpointId) == CHIP_NO_ERROR);
}

void ApplicationShutdown()
{
    ElectricalProtectionAlarm::ElectricalProtectionAlarmShutdown();
    ElectricalDistribution::ElectricalDistributionShutdown();

    LogErrorOnFailure(PowerTopology::PowerTopologyShutdown(gBreakerPtInstance, gBreakerPtDelegate));
    LogErrorOnFailure(PowerTopology::PowerTopologyShutdown(gEnclosurePtInstance, gEnclosurePtDelegate));
}

int main(int argc, char * argv[])
{
    if (ChipLinuxAppInit(argc, argv) != 0)
    {
        return -1;
    }

    ChipLinuxAppMainLoop();
    return 0;
}
