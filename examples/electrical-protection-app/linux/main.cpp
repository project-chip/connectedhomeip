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
#include <electrical-alarm-stub.h>
#include <electrical-distribution-stub.h>
#include <electrical-protection-alarm-stub.h>
#include <power-topology-stub.h>

#include <app-common/zap-generated/ids/Clusters.h>
#include <app/util/attribute-storage.h>
#include <lib/support/CodeUtils.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;

namespace {

constexpr EndpointId kEnclosureEndpointId = 1; // Electrical Distribution Enclosure (0x0517)
constexpr EndpointId kBreakerEndpointId   = 2; // Electrical Circuit Breaker (0x0516)

// The enclosure device type requires each breaker child endpoint to carry a semantic tag from the
// Common Number namespace, so this one is breaker number one.
constexpr uint8_t kCommonNumberNamespaceId = 0x07;
constexpr uint8_t kNumberOneTag            = 0x01;

const Descriptor::Structs::SemanticTagStruct::Type kBreakerTagList[] = {
    { .namespaceID = kCommonNumberNamespaceId, .tag = kNumberOneTag },
};

} // namespace

void ApplicationInit()
{
    // Electrical Distribution describes the enclosure's physical characteristics. Its generated
    // Init callback is a no-op, so the app owns the instance and registers it imperatively.
    VerifyOrDie(ElectricalDistribution::ElectricalDistributionInit(kEnclosureEndpointId) == CHIP_NO_ERROR);

    // Electrical Protection Alarm reports the enclosure's safety faults. Same imperative
    // registration: its generated Init callback is a no-op too.
    VerifyOrDie(ElectricalProtectionAlarm::ElectricalProtectionAlarmInit(kEnclosureEndpointId) == CHIP_NO_ERROR);

    // Power Topology is mandatory on both device types, but with different features. The
    // enclosure needs only a topology choice; the breaker device type additionally makes
    // ElectricalCircuit mandatory, which brings the ElectricalCircuitNodes attribute.
    VerifyOrDie(PowerTopology::PowerTopologyInit(
                    kEnclosureEndpointId, BitMask<PowerTopology::Feature>(PowerTopology::Feature::kTreeTopology)) == CHIP_NO_ERROR);
    VerifyOrDie(PowerTopology::PowerTopologyInit(kBreakerEndpointId,
                                                 BitMask<PowerTopology::Feature>(PowerTopology::Feature::kTreeTopology,
                                                                                 PowerTopology::Feature::kElectricalCircuit)) ==
                CHIP_NO_ERROR);

    VerifyOrDie(SetTagList(kBreakerEndpointId, Span<const Descriptor::Structs::SemanticTagStruct::Type>(kBreakerTagList)) ==
                CHIP_NO_ERROR);

    // Electrical Alarm reports measurement-threshold alarms on the breaker. It is one of the
    // choice-group members the breaker device type offers, and it makes OverCurrent mandatory.
    // AdjustableThresholds is offered too, so the thresholds behind that alarm can be set at
    // runtime; without it SetElectricalAlarmThresholds is absent and TC-ESALM-2.2 has no DUT.
    VerifyOrDie(ElectricalAlarm::ElectricalAlarmInit(
                    kBreakerEndpointId,
                    BitMask<ElectricalAlarm::Feature>(ElectricalAlarm::Feature::kOverCurrent,
                                                      ElectricalAlarm::Feature::kAdjustableThresholds)) == CHIP_NO_ERROR);
}

void ApplicationShutdown()
{
    ElectricalAlarm::ElectricalAlarmShutdown();
    ElectricalProtectionAlarm::ElectricalProtectionAlarmShutdown();
    PowerTopology::PowerTopologyShutdown();
    ElectricalDistribution::ElectricalDistributionShutdown();
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
