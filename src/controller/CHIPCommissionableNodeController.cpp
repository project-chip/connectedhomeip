/*
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

// module header, comes first
#include <controller/CHIPCommissionableNodeController.h>

#if CONFIG_DEVICE_LAYER
#include <platform/CHIPDeviceLayer.h>
#endif

#include <lib/support/CodeUtils.h>

namespace chip {
namespace Controller {

CHIP_ERROR CommissionableNodeController::DiscoverCommissioners(Dnssd::DiscoveryFilter discoveryFilter)
{
    ReturnErrorOnFailure(SetUpNodeDiscovery());

    if (mResolver == nullptr)
    {
#if CONFIG_DEVICE_LAYER
        mDNSResolver.Shutdown(); // reset if already inited
        ReturnErrorOnFailure(mDNSResolver.Init(DeviceLayer::UDPEndPointManager()));
#endif
        mDNSResolver.SetCommissioningDelegate(this);
        return mDNSResolver.DiscoverCommissioners(discoveryFilter);
    }

#if CONFIG_DEVICE_LAYER
    ReturnErrorOnFailure(mResolver->Init(DeviceLayer::UDPEndPointManager()));
#endif
    return mResolver->DiscoverCommissioners(discoveryFilter);
}

CommissionableNodeController::~CommissionableNodeController()
{
    mDNSResolver.SetCommissioningDelegate(nullptr);
}

const Dnssd::DiscoveredNodeData * CommissionableNodeController::GetDiscoveredCommissioner(int idx)
{
    return GetDiscoveredNode(idx);
}

} // namespace Controller
} // namespace chip
