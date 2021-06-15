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
#include <controller/CHIPCommissionableNodeController.h>

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

CHIP_ERROR CommissionableNodeController::Init()
{
    CHIP_ERROR err = CHIP_NO_ERROR;
#if CONFIG_DEVICE_LAYER
    if ((err = DeviceLayer::PlatformMgr().InitChipStack()) != CHIP_NO_ERROR)
    {
        ChipLogError(chipTool, "FAILED to initialize chip stack");
    }
#endif
    return err;
}

CHIP_ERROR CommissionableNodeController::DiscoverCommissioners()
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    if ((err = SetUpNodeDiscovery()) == CHIP_NO_ERROR)
    {
        return chip::Mdns::Resolver::Instance().FindCommissioners();
    }
    return err;
}

CHIP_ERROR CommissionableNodeController::DiscoverCommissionersLongDiscriminator(uint16_t long_discriminator)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    if ((err = SetUpNodeDiscoveryLongDiscriminator(long_discriminator)) == CHIP_NO_ERROR)
    {
        return chip::Mdns::Resolver::Instance().FindCommissioners(filter);
    }
    return err;
}

const Mdns::DiscoveredNodeData * CommissionableNodeController::GetDiscoveredCommissioner(int idx)
{
    return GetDiscoveredNode(idx);
}

} // namespace Controller
} // namespace chip
