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
#include <chrono>
#include <string>
#include <thread>

using namespace ::chip;

CHIP_ERROR DiscoverCommissionersCommand::Run(PersistentStorage & storage, NodeId localId, NodeId remoteId)
{
    mCommissionableNodeController.Init();
    mCommissionableNodeController.DiscoverCommissioners();

    uint32_t waitTime     = 500;
    System::Timer * timer = nullptr;
    if (DeviceLayer::SystemLayer.NewTimer(timer) == CHIP_NO_ERROR)
    {
        timer->Start(
            waitTime, [](System::Layer *, void *, System::Error err) { DeviceLayer::PlatformMgr().Shutdown(); }, nullptr);
    }
    else
    {
        ChipLogError(chipTool, "Failed to create the shutdown timer. Kill with ^C.");
    }

    DeviceLayer::PlatformMgr().RunEventLoop();

    int commissionerCount = 0;
    for (int i = 0; i < CHIP_DEVICE_CONFIG_MAX_DISCOVERED_NODES; i++)
    {
        const Mdns::DiscoveredNodeData * commissioner = mCommissionableNodeController.GetDiscoveredCommissioner(i);
        if (commissioner != nullptr)
        {
            printf("Discovered Commisioner #%d\n", ++commissionerCount);
            if (strcmp(commissioner->deviceName, "") != 0)
            {
                printf("Device Name: %s. ", commissioner->deviceName);
            }
            if (commissioner->vendorId > 0)
            {
                printf("Vendor ID: %d. ", commissioner->vendorId);
            }
            if (commissioner->productId > 0)
            {
                printf("Product ID: %d. ", commissioner->productId);
            }
            if (commissioner->deviceType > 0)
            {
                printf("Device Type: %d. ", commissioner->deviceType);
            }
            if (commissioner->longDiscriminator > 0)
            {
                printf("Long Discriminator: %d. ", commissioner->longDiscriminator);
            }
            if (!commissioner->IsHost(""))
            {
                printf("Hostname: %s. ", commissioner->hostName);
            }
            if (commissioner->numIPs > 0)
            {
                printf("Number of IP addresses: %d. IP Adddress(es): ", commissioner->numIPs);
                for (int j = 0; j < commissioner->numIPs; j++)
                {
                    char ipAddress[INET6_ADDRSTRLEN];
                    printf("%s, ", commissioner->ipAddress[j].ToString(ipAddress, sizeof(ipAddress)));
                }
            }

            printf("\n");
        }
    }

    printf("Total of %d commissioner(s) discovered in %d ms\n", commissionerCount, waitTime);
    return CHIP_NO_ERROR;
}
