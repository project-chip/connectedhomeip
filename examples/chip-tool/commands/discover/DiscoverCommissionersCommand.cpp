/*
 *   Copyright (c) 2020 Project CHIP Authors
 *   All rights reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */

#include "DiscoverCommissionersCommand.h"
#include <arpa/inet.h>
#include <controller/CHIPDeviceController.h>

using namespace ::chip;

CHIP_ERROR DiscoverCommissionersCommand::RunCommand()
{
    return mCommissionableNodeController.DiscoverCommissioners();
}

void DiscoverCommissionersCommand::Shutdown()
{
    [[maybe_unused]] int commissionerCount = 0;
    for (int i = 0; i < CHIP_DEVICE_CONFIG_MAX_DISCOVERED_NODES; i++)
    {
        const Dnssd::CommissionNodeData * commissioner = mCommissionableNodeController.GetDiscoveredCommissioner(i);
        if (commissioner != nullptr)
        {
            ChipLogProgress(chipTool, "Discovered Commissioner #%d", commissionerCount);
            commissioner->LogDetail();
            commissionerCount++;
        }
    }

    ChipLogProgress(chipTool, "Total of %d commissioner(s) discovered in %u sec", commissionerCount,
                    std::chrono::duration_cast<System::Clock::Seconds16>(GetWaitDuration()).count());

    CHIPCommand::Shutdown();
}
