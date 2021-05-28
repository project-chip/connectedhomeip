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

#pragma once

#include <mdns/Resolver.h>
#include <platform/CHIPDeviceConfig.h>
#include <support/logging/CHIPLogging.h>

namespace chip {

namespace Controller {

constexpr uint16_t kMdnsPort = 5388;

class DLL_EXPORT CommissionableNodeController : public Mdns::ResolverDelegate
{
public:
    CommissionableNodeController(){};
    virtual ~CommissionableNodeController() {}

    CHIP_ERROR Init();

    CHIP_ERROR DiscoverAllCommissionersLongDiscriminator(uint16_t long_discriminator);

    CHIP_ERROR DiscoverAllCommissioners();

    const Mdns::CommissionableNodeData * GetDiscoveredDevice(int idx);

    void OnCommissionerFound(const chip::Mdns::CommissionableNodeData & nodeData) override;

    void OnNodeIdResolved(const chip::Mdns::ResolvedNodeData & nodeData) override
    {
        ChipLogError(chipTool, "Unsupported operation CommissionableNodeController::OnNodeIdResolved");
    };

    void OnNodeIdResolutionFailed(const chip::PeerId & peerId, CHIP_ERROR error) override
    {
        ChipLogError(chipTool, "Unsupported operation CommissionableNodeController::OnNodeIdResolutionFailed");
    };

    void OnCommissionableNodeFound(const chip::Mdns::CommissionableNodeData & nodeData) override
    {
        ChipLogError(chipTool, "Unsupported operation CommissionableNodeController::OnCommissionableNodeFound");
    };

private:
    Mdns::CommissionableNodeData mDiscoveredCommissioners[CHIP_DEVICE_CONFIG_MAX_DISCOVERED_COMMISSIONERS];
};

} // namespace Controller
} // namespace chip
