/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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
#include <support/DLLUtil.h>
#include <transport/raw/MessageHeader.h>

namespace chip {
namespace Controller {

class DeviceController;

/// Callbacks for CHIP device address resolution
class DLL_EXPORT DeviceAddressUpdateDelegate
{
public:
    virtual ~DeviceAddressUpdateDelegate() {}
    virtual void OnAddressUpdateComplete(NodeId nodeId, CHIP_ERROR error) = 0;
};

/// Class for updating CHIP devices' addresses based on responses from mDNS Resolver
class DLL_EXPORT DeviceAddressUpdater : public Mdns::ResolverDelegate
{
public:
    CHIP_ERROR Init(DeviceController * controller, DeviceAddressUpdateDelegate * delegate = nullptr);

private:
    // Mdns::ResolverDelegate Implementation
    void OnNodeIdResolved(NodeId nodeId, const Mdns::ResolvedNodeData & nodeData) override;
    void OnNodeIdResolutionFailed(NodeId nodeId, CHIP_ERROR error) override;

    DeviceController * mController          = nullptr;
    DeviceAddressUpdateDelegate * mDelegate = nullptr;
};

} // namespace Controller
} // namespace chip
