/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2013-2017 Nest Labs, Inc.
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

/**
 *    @file
 *      Declaration of CHIP CommissionableNode
 *
 */

#pragma once

#include <mdns/Resolver.h>
#include <support/logging/CHIPLogging.h>

namespace chip {

namespace Controller {

constexpr uint16_t kMdnsPort = 5388;

// struct ResolvedNodeData;
struct CommissionableNodeData;

class DLL_EXPORT CommissionableNode : public Mdns::ResolverDelegate
{
public:
    CommissionableNode(){};
    virtual ~CommissionableNode() {}

    CHIP_ERROR Init();

    /**
     * @brief
     *   Discover devices advertising as commissionable that match the long discriminator.
     * @return CHIP_ERROR   The return status
     */
    CHIP_ERROR DiscoverAllCommissionersLongDiscriminator(uint16_t long_discriminator);

    /**
     * @brief
     *   Discover all devices advertising as commissionable.
     *   Should be called on main loop thread.
     * @return CHIP_ERROR   The return status
     */
    CHIP_ERROR DiscoverAllCommissioners();

    /**
     * @brief
     *   Returns information about discovered devices.
     *   Should be called on main loop thread.
     * @return const CommissionableNodeData* info about the selected device. May be nullptr if no information has been returned yet.
     */
    const Mdns::CommissionableNodeData * GetDiscoveredDevice(int idx);

    /**
     * @brief
     *   Returns the max number of commissionable nodes this commissioner can track mdns information for.
     * @return int  The max number of commissionable nodes supported
     */
    int GetMaxCommissionersSupported() { return kMaxCommissioners; }

    void OnCommissionerFound(const chip::Mdns::CommissionableNodeData & nodeData) override;

    void OnNodeIdResolved(const chip::Mdns::ResolvedNodeData & nodeData) override
    {
        ChipLogError(chipTool, "Unsupported operation CommissionableNode::OnNodeIdResolved");
    };

    void OnNodeIdResolutionFailed(const chip::PeerId & peerId, CHIP_ERROR error) override
    {
        ChipLogError(chipTool, "Unsupported operation CommissionableNode::OnNodeIdResolutionFailed");
    };

    void OnCommissionableNodeFound(const chip::Mdns::CommissionableNodeData & nodeData) override
    {
        ChipLogError(chipTool, "Unsupported operation CommissionableNode::OnCommissionableNodeFound");
    };

private:
    static constexpr int kMaxCommissioners = 10;
    Mdns::CommissionableNodeData mCommissioners[kMaxCommissioners];
};

} // namespace Controller
} // namespace chip
