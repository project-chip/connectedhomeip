/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include "Resolver.h"

#include <lib/support/logging/CHIPLogging.h>

namespace chip {
namespace Dnssd {
namespace {

class NoneResolver : public Resolver
{
public:
    CHIP_ERROR Init(chip::Inet::EndPointManager<chip::Inet::UDPEndPoint> *) override { return CHIP_NO_ERROR; }
    bool IsInitialized() override { return true; }
    void Shutdown() override {}
    void SetOperationalDelegate(OperationalResolveDelegate * delegate) override {}

    CHIP_ERROR ResolveNodeId(const PeerId & peerId) override
    {
        ChipLogError(Discovery, "Failed to resolve node ID: dnssd resolving not available");
        return CHIP_ERROR_NOT_IMPLEMENTED;
    }
    void NodeIdResolutionNoLongerNeeded(const PeerId & peerId) override
    {
        ChipLogError(Discovery, "Failed to stop resolving node ID: dnssd resolving not available");
    }
    CHIP_ERROR DiscoverCommissionableNodes(DiscoveryFilter filter, DiscoveryContext & context) override
    {
        return CHIP_ERROR_NOT_IMPLEMENTED;
    }
    CHIP_ERROR DiscoverCommissioners(DiscoveryFilter filter, DiscoveryContext & context) override
    {
        return CHIP_ERROR_NOT_IMPLEMENTED;
    }
    CHIP_ERROR StopDiscovery(DiscoveryContext & context) override { return CHIP_ERROR_NOT_IMPLEMENTED; }
    CHIP_ERROR ReconfirmRecord(const char * hostname, Inet::IPAddress address, Inet::InterfaceId interfaceId) override
    {
        return CHIP_ERROR_NOT_IMPLEMENTED;
    }
};

NoneResolver gResolver;

} // namespace

Resolver & chip::Dnssd::Resolver::Instance()
{
    return gResolver;
}

} // namespace Dnssd
} // namespace chip
