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
#include <controller/AbstractMdnsDiscoveryController.h>

#if CONFIG_DEVICE_LAYER
#include <platform/CHIPDeviceLayer.h>
#endif

#include <core/CHIPEncoding.h>
#include <support/logging/CHIPLogging.h>

namespace chip {
namespace Controller {

void AbstractMdnsDiscoveryController::OnNodeDiscoveryComplete(const chip::Mdns::DiscoveredNodeData & nodeData)
{
    Mdns::DiscoveredNodeData * mDiscoveredNodes = GetDiscoveredNodes();
    for (int i = 0; i < CHIP_DEVICE_CONFIG_MAX_DISCOVERED_NODES; ++i)
    {
        if (!mDiscoveredNodes[i].IsValid())
        {
            continue;
        }
        if (strcmp(mDiscoveredNodes[i].hostName, nodeData.hostName) == 0)
        {
            mDiscoveredNodes[i] = nodeData;
            return;
        }
    }
    // Node not yet in the list
    for (int i = 0; i < CHIP_DEVICE_CONFIG_MAX_DISCOVERED_NODES; ++i)
    {
        if (!mDiscoveredNodes[i].IsValid())
        {
            mDiscoveredNodes[i] = nodeData;
            return;
        }
    }
    ChipLogError(Discovery, "Failed to add discovered node with hostname %s- Insufficient space", nodeData.hostName);
}

CHIP_ERROR AbstractMdnsDiscoveryController::SetUpNodeDiscovery()
{
    chip::Mdns::Resolver::Instance().SetResolverDelegate(this);
#if CONFIG_DEVICE_LAYER
    ReturnErrorOnFailure(chip::Mdns::Resolver::Instance().StartResolver(&DeviceLayer::InetLayer, kMdnsPort));
#endif

    Mdns::DiscoveredNodeData * mDiscoveredNodes = GetDiscoveredNodes();
    for (int i = 0; i < CHIP_DEVICE_CONFIG_MAX_DISCOVERED_NODES; ++i)
    {
        mDiscoveredNodes[i].Reset();
    }
    return CHIP_NO_ERROR;
}

const Mdns::DiscoveredNodeData * AbstractMdnsDiscoveryController::GetDiscoveredNode(int idx)
{
    // TODO(cecille): Add assertion about main loop.
    Mdns::DiscoveredNodeData * mDiscoveredNodes = GetDiscoveredNodes();
    if (mDiscoveredNodes[idx].IsValid())
    {
        return &mDiscoveredNodes[idx];
    }
    return nullptr;
}

} // namespace Controller
} // namespace chip
