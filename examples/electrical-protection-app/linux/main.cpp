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
#include <electrical-distribution-stub.h>
#include <electrical-protection-alarm-stub.h>
#include <power-topology-stub.h>

#include <app-common/zap-generated/ids/Clusters.h>
#include <lib/support/CodeUtils.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;

namespace {

constexpr EndpointId kEnclosureEndpointId = 1; // Electrical Distribution Enclosure (0x0517)

} // namespace

void ApplicationInit()
{
    // Electrical Distribution describes the enclosure's physical characteristics. Its generated
    // Init callback is a no-op, so the app owns the instance and registers it imperatively.
    VerifyOrDie(ElectricalDistribution::ElectricalDistributionInit(kEnclosureEndpointId) == CHIP_NO_ERROR);

    // Electrical Protection Alarm reports the enclosure's safety faults. Same imperative
    // registration: its generated Init callback is a no-op too.
    VerifyOrDie(ElectricalProtectionAlarm::ElectricalProtectionAlarmInit(kEnclosureEndpointId) == CHIP_NO_ERROR);

    // Power Topology is mandatory on the enclosure device type.
    VerifyOrDie(PowerTopology::PowerTopologyInit(kEnclosureEndpointId) == CHIP_NO_ERROR);
}

void ApplicationShutdown()
{
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
