/*
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

// module header, comes first
#include <controller/AbstractDnssdDiscoveryController.h>

#include <lib/core/CHIPEncoding.h>
#include <lib/support/logging/CHIPLogging.h>

namespace chip {
namespace Controller {

void AbstractDnssdDiscoveryController::OnNodeDiscovered(const chip::Dnssd::DiscoveredNodeData & nodeData)
{
    auto discoveredNodes = GetDiscoveredNodes();
    for (auto & discoveredNode : discoveredNodes)
    {
        if (!discoveredNode.resolutionData.IsValid())
        {
            continue;
        }
        if (strcmp(discoveredNode.resolutionData.hostName, nodeData.resolutionData.hostName) == 0 &&
            discoveredNode.resolutionData.port == nodeData.resolutionData.port)
        {
            discoveredNode = nodeData;
            if (mDeviceDiscoveryDelegate != nullptr)
            {
                mDeviceDiscoveryDelegate->OnDiscoveredDevice(nodeData);
            }
            return;
        }
    }
    // Node not yet in the list
    for (auto & discoveredNode : discoveredNodes)
    {
        if (!discoveredNode.resolutionData.IsValid())
        {
            discoveredNode = nodeData;
            if (mDeviceDiscoveryDelegate != nullptr)
            {
                mDeviceDiscoveryDelegate->OnDiscoveredDevice(nodeData);
            }
            return;
        }
    }
    ChipLogError(Discovery, "Failed to add discovered node with hostname %s- Insufficient space", nodeData.resolutionData.hostName);
}

CHIP_ERROR AbstractDnssdDiscoveryController::SetUpNodeDiscovery()
{
    auto discoveredNodes = GetDiscoveredNodes();
    for (auto & discoveredNode : discoveredNodes)
    {
        discoveredNode.Reset();
    }
    return CHIP_NO_ERROR;
}

const Dnssd::DiscoveredNodeData * AbstractDnssdDiscoveryController::GetDiscoveredNode(int idx)
{
    // TODO(cecille): Add assertion about main loop.
    auto discoveredNodes = GetDiscoveredNodes();
    if (0 <= idx && idx < CHIP_DEVICE_CONFIG_MAX_DISCOVERED_NODES && discoveredNodes.data()[idx].resolutionData.IsValid())
    {
        return discoveredNodes.data() + idx;
    }
    return nullptr;
}

} // namespace Controller
} // namespace chip
