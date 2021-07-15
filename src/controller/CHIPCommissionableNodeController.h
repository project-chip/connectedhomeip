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

#pragma once

#include <controller/AbstractMdnsDiscoveryController.h>
#include <mdns/Resolver.h>
#include <platform/CHIPDeviceConfig.h>
#include <support/logging/CHIPLogging.h>

namespace chip {

namespace Controller {

/**
 *    @brief
 *      CHIPCommissionableNodeController is a Controller class that
 *      centralizes (acts as facade) discovery of commissioners, sending User
 *      Directed Commissioning requests, Commissionable Node advertisement and
 *      Commissioning of the node
 */
class DLL_EXPORT CommissionableNodeController : public AbstractMdnsDiscoveryController
{
public:
    CommissionableNodeController(){};
    virtual ~CommissionableNodeController() {}

    CHIP_ERROR DiscoverCommissioners(Mdns::DiscoveryFilter discoveryFilter = Mdns::DiscoveryFilter());

    const Mdns::DiscoveredNodeData * GetDiscoveredCommissioner(int idx);

    void OnNodeIdResolved(const chip::Mdns::ResolvedNodeData & nodeData) override
    {
        ChipLogError(Controller, "Unsupported operation CommissionableNodeController::OnNodeIdResolved");
    }

    void OnNodeIdResolutionFailed(const chip::PeerId & peerId, CHIP_ERROR error) override
    {
        ChipLogError(Controller, "Unsupported operation CommissionableNodeController::OnNodeIdResolutionFailed");
    }

protected:
    DiscoveredNodeList GetDiscoveredNodes() override { return DiscoveredNodeList(mDiscoveredCommissioners); }

private:
    Mdns::DiscoveredNodeData mDiscoveredCommissioners[CHIP_DEVICE_CONFIG_MAX_DISCOVERED_NODES];
};

} // namespace Controller
} // namespace chip
