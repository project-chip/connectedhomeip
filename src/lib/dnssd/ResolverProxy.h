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

#pragma once

#include <lib/dnssd/Resolver.h>
#include <lib/dnssd/platform/Dnssd.h>

namespace chip {
namespace Dnssd {

class ResolverProxy : public Resolver, public ResolverDelegate
{
public:
    ResolverProxy() {}
    ResolverProxy(ResolverDelegate * delegate) : mResolverDelegate(delegate) {}

    // Resolver interface.
    CHIP_ERROR Init(Inet::InetLayer * = nullptr) override { return CHIP_NO_ERROR; }
    void Shutdown() override { mResolverDelegate = nullptr; };
    void SetResolverDelegate(ResolverDelegate * delegate) override { mResolverDelegate = delegate; }
    CHIP_ERROR ResolveNodeId(const PeerId & peerId, Inet::IPAddressType type,
                             Resolver::CacheBypass dnssdCacheBypass = CacheBypass::Off) override;
    CHIP_ERROR FindCommissionableNodes(DiscoveryFilter filter = DiscoveryFilter()) override;
    CHIP_ERROR FindCommissioners(DiscoveryFilter filter = DiscoveryFilter()) override;

    /// ResolverDelegate interface
    void OnNodeIdResolved(const ResolvedNodeData & nodeData) override
    {
        if (mResolverDelegate != nullptr)
        {
            mResolverDelegate->OnNodeIdResolved(nodeData);
        }
    }
    void OnNodeIdResolutionFailed(const PeerId & peerId, CHIP_ERROR error) override
    {
        if (mResolverDelegate != nullptr)
        {
            mResolverDelegate->OnNodeIdResolutionFailed(peerId, error);
        }
    }
    void OnNodeDiscoveryComplete(const DiscoveredNodeData & nodeData) override
    {
        if (mResolverDelegate != nullptr)
        {
            mResolverDelegate->OnNodeDiscoveryComplete(nodeData);
        }
    }

private:
    ResolverDelegate * mResolverDelegate = nullptr;
};

} // namespace Dnssd
} // namespace chip
