/*
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
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
    int commissionerCount = 0;
    for (int i = 0; i < CHIP_DEVICE_CONFIG_MAX_DISCOVERED_NODES; i++)
    {
        const Dnssd::DiscoveredNodeData * commissioner = mCommissionableNodeController.GetDiscoveredCommissioner(i);
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
