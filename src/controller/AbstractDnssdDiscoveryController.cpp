/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

// module header, comes first
#include <controller/AbstractDnssdDiscoveryController.h>

#include <lib/core/CHIPEncoding.h>
#include <lib/support/logging/CHIPLogging.h>

#include <bitset>

namespace chip {
namespace Controller {

static bool SameExceptOrder(const chip::Span<const Inet::IPAddress> & v1, const chip::Span<const Inet::IPAddress> & v2)
{
    std::bitset<chip::Dnssd::CommonResolutionData::kMaxIPAddresses> addressUsed;

    VerifyOrDie(v1.size() <= Dnssd::CommissionNodeData::kMaxIPAddresses && v2.size() <= Dnssd::CommissionNodeData::kMaxIPAddresses);
    if (v1.size() != v2.size())
    {
        return false;
    }

    for (size_t s = 0; s < v1.size(); s++)
    {
        for (size_t d = 0; d < v2.size(); d++)
        {
            if (!addressUsed[d] && v1[s] == v2[d])
            {
                // Change the used flag so that the compared target is no longer used
                addressUsed.set(d, true);
                break;
            }
        }
    }
    return addressUsed.count() == v2.size();
}

void AbstractDnssdDiscoveryController::OnNodeDiscovered(const chip::Dnssd::DiscoveredNodeData & discNodeData)
{
    VerifyOrReturn(discNodeData.Is<chip::Dnssd::CommissionNodeData>());

    auto discoveredNodes = GetDiscoveredNodes();
    auto & nodeData      = discNodeData.Get<chip::Dnssd::CommissionNodeData>();
    for (auto & discoveredNode : discoveredNodes)
    {

        if (!discoveredNode.IsValid())
        {
            continue;
        }
        chip::Span<const Inet::IPAddress> discoveredNodeIPAddressSpan(&discoveredNode.ipAddress[0], discoveredNode.numIPs);
        chip::Span<const Inet::IPAddress> nodeDataIPAddressSpan(&nodeData.ipAddress[0], nodeData.numIPs);
        if (strcmp(discoveredNode.hostName, nodeData.hostName) == 0 && discoveredNode.port == nodeData.port &&
            SameExceptOrder(discoveredNodeIPAddressSpan, nodeDataIPAddressSpan))
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
        if (!discoveredNode.IsValid())
        {
            discoveredNode = nodeData;
            if (mDeviceDiscoveryDelegate != nullptr)
            {
                mDeviceDiscoveryDelegate->OnDiscoveredDevice(nodeData);
            }
            return;
        }
    }
    ChipLogError(Discovery, "Failed to add discovered node with hostname %s- Insufficient space", nodeData.hostName);
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

const Dnssd::CommissionNodeData * AbstractDnssdDiscoveryController::GetDiscoveredNode(int idx)
{
    // TODO(cecille): Add assertion about main loop.
    auto discoveredNodes = GetDiscoveredNodes();
    if (0 <= idx && idx < CHIP_DEVICE_CONFIG_MAX_DISCOVERED_NODES && discoveredNodes.data()[idx].IsValid())
    {
        return discoveredNodes.data() + idx;
    }
    return nullptr;
}

} // namespace Controller
} // namespace chip
