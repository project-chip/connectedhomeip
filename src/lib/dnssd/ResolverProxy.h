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

#include <lib/core/ReferenceCounted.h>
#include <lib/dnssd/Resolver.h>

namespace chip {
namespace Dnssd {

class ResolverDelegateProxy : public ReferenceCounted<ResolverDelegateProxy>, public ResolverDelegate
{
public:
    void SetDelegate(ResolverDelegate * delegate) { mDelegate = delegate; }

    /// ResolverDelegate interface
    void OnNodeIdResolved(const ResolvedNodeData & nodeData) override
    {
        if (mDelegate != nullptr)
        {
            mDelegate->OnNodeIdResolved(nodeData);
        }
    }

    void OnNodeIdResolutionFailed(const PeerId & peerId, CHIP_ERROR error) override
    {
        if (mDelegate != nullptr)
        {
            mDelegate->OnNodeIdResolutionFailed(peerId, error);
        }
    }

    void OnNodeDiscoveryComplete(const DiscoveredNodeData & nodeData) override
    {
        if (mDelegate != nullptr)
        {
            mDelegate->OnNodeDiscoveryComplete(nodeData);
        }
    }

private:
    ResolverDelegate * mDelegate = nullptr;
};

class ResolverProxy : public Resolver
{
public:
    ResolverProxy() {}

    // Resolver interface.
    CHIP_ERROR Init(Inet::EndPointManager<Inet::UDPEndPoint> * udpEndPoint = nullptr) override
    {
        ReturnErrorOnFailure(chip::Dnssd::Resolver::Instance().Init(udpEndPoint));
        VerifyOrReturnError(mDelegate == nullptr, CHIP_ERROR_INCORRECT_STATE);
        mDelegate = chip::Platform::New<ResolverDelegateProxy>();
        return mDelegate != nullptr ? CHIP_NO_ERROR : CHIP_ERROR_NO_MEMORY;
    }

    void SetResolverDelegate(ResolverDelegate * delegate) override
    {
        VerifyOrReturn(mDelegate != nullptr);
        mDelegate->SetDelegate(delegate);
    }

    void Shutdown() override
    {
        VerifyOrReturn(mDelegate != nullptr);
        mDelegate->SetDelegate(nullptr);
        mDelegate->Release();
        mDelegate = nullptr;
    }

    CHIP_ERROR ResolveNodeId(const PeerId & peerId, Inet::IPAddressType type) override;
    CHIP_ERROR FindCommissionableNodes(DiscoveryFilter filter = DiscoveryFilter()) override;
    CHIP_ERROR FindCommissioners(DiscoveryFilter filter = DiscoveryFilter()) override;
    bool ResolveNodeIdFromInternalCache(const PeerId & peerId, Inet::IPAddressType type) override;

private:
    ResolverDelegateProxy * mDelegate = nullptr;
};

} // namespace Dnssd
} // namespace chip
