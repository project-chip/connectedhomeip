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

#if CONFIG_DEVICE_LAYER
    mDNSResolver.Shutdown(); // reset if already inited
    ReturnErrorOnFailure(mDNSResolver.Init(DeviceLayer::UDPEndPointManager()));
#endif
    mDNSResolver.SetCommissioningDelegate(this);
    return mDNSResolver.DiscoverCommissioners(discoveryFilter);
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
