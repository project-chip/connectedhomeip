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

#ifndef __STDC_LIMIT_MACROS
#define __STDC_LIMIT_MACROS
#endif
#ifndef __STDC_FORMAT_MACROS
#define __STDC_FORMAT_MACROS
#endif

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
    // Didn't find the host name already in our list, return an invalid
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

CHIP_ERROR AbstractMdnsDiscoveryController::SetUpNodeDiscoveryLongDiscriminator(uint16_t long_discriminator)
{
    filter = Mdns::DiscoveryFilter(Mdns::DiscoveryFilterType::kLong, long_discriminator);
    return SetUpNodeDiscovery();
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
