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
 *      Implementation of CHIP Device Controller, a common class
 *      that implements discovery, pairing and provisioning of CHIP
 *      devices.
 *
 */

#ifndef __STDC_LIMIT_MACROS
#define __STDC_LIMIT_MACROS
#endif
#ifndef __STDC_FORMAT_MACROS
#define __STDC_FORMAT_MACROS
#endif

// module header, comes first
#include <controller/CHIPCommissionableNode.h>

#if CONFIG_DEVICE_LAYER
#include <platform/CHIPDeviceLayer.h>
#endif

#include <core/CHIPCore.h>
#include <core/CHIPEncoding.h>
#include <core/CHIPSafeCasts.h>
#include <support/CHIPArgParser.hpp>
#include <support/CHIPMem.h>
#include <support/CodeUtils.h>
#include <support/logging/CHIPLogging.h>

#if CONFIG_NETWORK_LAYER_BLE
#include <ble/BleLayer.h>
#include <transport/raw/BLE.h>
#endif

#include <app/util/af-enums.h>

#include <errno.h>
#include <inttypes.h>
#include <memory>
#include <stdint.h>
#include <stdlib.h>

namespace chip {
namespace Controller {

CHIP_ERROR CommissionableNode::Init()
{
    if (Platform::MemoryInit() != CHIP_NO_ERROR)
    {
        ChipLogError(chipTool, "FAILED to initialize memory");
        return 1;
    }

#if CONFIG_DEVICE_LAYER
    if (DeviceLayer::PlatformMgr().InitChipStack() != CHIP_NO_ERROR)
    {
        ChipLogError(chipTool, "FAILED to initialize chip stack");
        return 1;
    }
#endif

    return CHIP_NO_ERROR;
}

CHIP_ERROR CommissionableNode::DiscoverAllCommissioners()
{
    chip::Mdns::Resolver::Instance().SetResolverDelegate(this);
#if CONFIG_DEVICE_LAYER
    ReturnErrorOnFailure(chip::Mdns::Resolver::Instance().StartResolver(&DeviceLayer::InetLayer, kMdnsPort));
#endif

    for (int i = 0; i < kMaxCommissioners; ++i)
    {
        mCommissioners[i].Reset();
    }
    return chip::Mdns::Resolver::Instance().FindCommissioners();
}

CHIP_ERROR CommissionableNode::DiscoverAllCommissionersLongDiscriminator(uint16_t long_discriminator)
{
    chip::Mdns::Resolver::Instance().SetResolverDelegate(this);
#if CONFIG_DEVICE_LAYER
    ReturnErrorOnFailure(chip::Mdns::Resolver::Instance().StartResolver(&DeviceLayer::InetLayer, kMdnsPort));
#endif

    for (int i = 0; i < kMaxCommissioners; ++i)
    {
        mCommissioners[i].Reset();
    }
    Mdns::DiscoveryFilter filter(Mdns::DiscoveryFilterType::kLong, long_discriminator);
    return Mdns::Resolver::Instance().FindCommissionableNodes(filter);
}

const Mdns::CommissionableNodeData * CommissionableNode::GetDiscoveredDevice(int idx)
{
    // TODO(cecille): Add assertion about main loop.
    if (mCommissioners[idx].IsValid())
    {
        return &mCommissioners[idx];
    }
    return nullptr;
}

void CommissionableNode::OnCommissionerFound(const chip::Mdns::CommissionableNodeData & nodeData)
{
    for (int i = 0; i < kMaxCommissioners; ++i)
    {
        if (!mCommissioners[i].IsValid())
        {
            continue;
        }
        if (strcmp(mCommissioners[i].hostName, nodeData.hostName) == 0)
        {
            mCommissioners[i] = nodeData;
            return;
        }
    }
    // Didn't find the host name already in our list, return an invalid
    for (int i = 0; i < kMaxCommissioners; ++i)
    {
        if (!mCommissioners[i].IsValid())
        {
            mCommissioners[i] = nodeData;
            return;
        }
    }
    ChipLogError(Discovery, "Failed to add discovered commisisoners - Insufficient space");
}

} // namespace Controller
} // namespace chip
