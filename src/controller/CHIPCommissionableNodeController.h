/*
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
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
