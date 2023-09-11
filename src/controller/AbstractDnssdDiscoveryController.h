/*
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <controller/DeviceDiscoveryDelegate.h>
#include <lib/dnssd/ResolverProxy.h>
#include <lib/support/Span.h>
#include <platform/CHIPDeviceConfig.h>

namespace chip {

namespace Controller {

/**
 * @brief
 *   Convenient superclass for controller implementations that need to discover
 *   Commissioners or CommissionableNodes using mDNS. This Abstract class
 *   provides base implementations for logic to setup mDNS discovery requests,
 *   handling of received DiscoveredNodeData, etc. while expecting child classes
 *   to maintain a list of DiscoveredNodes and providing the implementation
 *   of the template GetDiscoveredNodes() function.
 */
class DLL_EXPORT AbstractDnssdDiscoveryController : public Dnssd::CommissioningResolveDelegate
{
public:
    AbstractDnssdDiscoveryController() {}
    ~AbstractDnssdDiscoveryController() override { mDNSResolver.Shutdown(); }

    void OnNodeDiscovered(const chip::Dnssd::DiscoveredNodeData & nodeData) override;

protected:
    using DiscoveredNodeList = FixedSpan<Dnssd::DiscoveredNodeData, CHIP_DEVICE_CONFIG_MAX_DISCOVERED_NODES>;
    CHIP_ERROR SetUpNodeDiscovery();
    const Dnssd::DiscoveredNodeData * GetDiscoveredNode(int idx);
    virtual DiscoveredNodeList GetDiscoveredNodes()    = 0;
    DeviceDiscoveryDelegate * mDeviceDiscoveryDelegate = nullptr;
    Dnssd::ResolverProxy mDNSResolver;
};

} // namespace Controller
} // namespace chip
