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
#include <chrono>
#include <string>
#include <thread>

using namespace ::chip;

CHIP_ERROR DiscoverCommissionersCommand::Run(PersistentStorage & storage, NodeId localId, NodeId remoteId)
{
    mCommissionableNode.Init();
    mCommissionableNode.DiscoverAllCommissioners();

    uint32_t waitTime     = 500;
    System::Timer * timer = nullptr;
    if (DeviceLayer::SystemLayer.NewTimer(timer) == CHIP_NO_ERROR)
    {
        timer->Start(
            waitTime, [](System::Layer *, void *, System::Error err) { DeviceLayer::PlatformMgr().Shutdown(); }, nullptr);
    }
    else
    {
        ChipLogError(chipTool, "Failed to create the shutdown timer. Kill with ^C.\n");
    }

    DeviceLayer::PlatformMgr().RunEventLoop();

    int commissionerCount = 0;
    for (int i = 0; i < mCommissionableNode.GetMaxCommissionerNodesSupported(); i++)
    {
        const Mdns::CommissionableNodeData * commissioner = mCommissionableNode.GetDiscoveredDevice(i);
        if (commissioner != nullptr)
        {
            printf("\nDiscovered Commisioner #%d\n", ++commissionerCount);
            printf(strcmp(commissioner->deviceName, "") != 0 ? "Device Name: %s. " : "", commissioner->deviceName);
            printf((commissioner->vendorId > 0) ? "Vendor ID: %d. " : "", commissioner->vendorId);
            printf((commissioner->productId > 0) ? "Product ID: %d. " : "", commissioner->productId);
            printf((commissioner->deviceType > 0) ? "Device Type: %d. " : "", commissioner->deviceType);
            printf((commissioner->longDiscriminator > 0) ? "Long Discriminator: %d. " : "", commissioner->longDiscriminator);
            printf((!commissioner->IsHost("")) ? "Hostname: %s. " : "", commissioner->hostName);
            printf((commissioner->numIPs > 0) ? "Number of IP addresses: %d. " : "", commissioner->numIPs);
            printf("\n");
        }
    }

    printf("Total of %d commissioner(s) discovered in %d ms\n", commissionerCount, waitTime);
    return CHIP_NO_ERROR;
}
