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

#include <controller/AbstractDnssdDiscoveryController.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/CHIPDeviceConfig.h>

namespace chip {

namespace Controller {

/**
 *    @brief
 *      CHIPCommissionableNodeController is a Controller class that
 *      centralizes (acts as facade) discovery of commissioners, sending User
 *      Directed Commissioning requests, Commissionable Node advertisement and
 *      Commissioning of the node
 */
class DLL_EXPORT CommissionableNodeController : public AbstractDnssdDiscoveryController
{
public:
    CommissionableNodeController(chip::Dnssd::Resolver * resolver = nullptr) : mResolver(resolver) {}
    ~CommissionableNodeController() override;

    void RegisterDeviceDiscoveryDelegate(DeviceDiscoveryDelegate * delegate) { mDeviceDiscoveryDelegate = delegate; }
    CHIP_ERROR DiscoverCommissioners(Dnssd::DiscoveryFilter discoveryFilter = Dnssd::DiscoveryFilter());

    /**
     * @return
     *   Pointer to DiscoveredNodeData at index idx in the list of commissioners discovered
     *   by the CHIPCommissionableNodeController, if the node is a valid node.
     *   Otherwise, returns nullptr
     *   See Resolver.h IsValid()
     */
    const Dnssd::DiscoveredNodeData * GetDiscoveredCommissioner(int idx);

protected:
    DiscoveredNodeList GetDiscoveredNodes() override { return DiscoveredNodeList(mDiscoveredCommissioners); }

private:
    Dnssd::Resolver * mResolver = nullptr;
    Dnssd::DiscoveredNodeData mDiscoveredCommissioners[CHIP_DEVICE_CONFIG_MAX_DISCOVERED_NODES];
};

} // namespace Controller
} // namespace chip
