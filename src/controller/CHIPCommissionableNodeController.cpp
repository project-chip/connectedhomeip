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

#include <app/server/Mdns.h>
#if CONFIG_DEVICE_LAYER
#include <platform/CHIPDeviceLayer.h>
#endif
#include <mdns/Advertiser.h>
#include <support/CodeUtils.h>

namespace chip {
namespace Controller {

CHIP_ERROR CommissionableNodeController::DiscoverCommissioners(Mdns::DiscoveryFilter discoveryFilter)
{
    ReturnErrorOnFailure(SetUpNodeDiscovery());
    return chip::Mdns::Resolver::Instance().FindCommissioners(discoveryFilter);
}

const Mdns::DiscoveredNodeData * CommissionableNodeController::GetDiscoveredCommissioner(int idx)
{
    return GetDiscoveredNode(idx);
}

CHIP_ERROR CommissionableNodeController::AdvertiseCommissionableNode()
{
#if CONFIG_DEVICE_LAYER
    ReturnErrorOnFailure(chip::Mdns::ServiceAdvertiser::Instance().Start(&chip::DeviceLayer::InetLayer, chip::Mdns::kMdnsPort));
    return app::Mdns::AdvertiseCommissionableNode();
#else
    return CHIP_ERROR_NOT_IMPLEMENTED;
#endif
}

CHIP_ERROR CommissionableNodeController::SendUserDirectedCommissioningRequest(chip::Inet::IPAddress commissioner, uint16_t port)
{
    // TODO: integrate with Server:SendUserDirectedCommissioningRequest()
    ChipLogError(Controller, "Unsupported operation CommissionableNodeController::SendUserDirectedCommissioningRequest");
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

} // namespace Controller
} // namespace chip
