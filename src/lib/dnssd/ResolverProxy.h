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

class ResolverDelegateProxy : public ReferenceCounted<ResolverDelegateProxy>,
                              public OperationalResolveDelegate,
                              public CommissioningResolveDelegate

{
public:
    void SetOperationalDelegate(OperationalResolveDelegate * delegate) { mOperationalDelegate = delegate; }
    void SetCommissioningDelegate(CommissioningResolveDelegate * delegate) { mCommissioningDelegate = delegate; }

    // OperationalResolveDelegate
    void OnOperationalNodeResolved(const ResolvedNodeData & nodeData) override
    {
        if (mOperationalDelegate != nullptr)
        {
            mOperationalDelegate->OnOperationalNodeResolved(nodeData);
        }
        else
        {
            ChipLogError(Discovery, "Missing operational delegate. Data discarded.");
        }
    }

    void OnOperationalNodeResolutionFailed(const PeerId & peerId, CHIP_ERROR error) override
    {
        if (mOperationalDelegate != nullptr)
        {
            mOperationalDelegate->OnOperationalNodeResolutionFailed(peerId, error);
        }
        else
        {
            ChipLogError(Discovery, "Missing operational delegate. Failure info discarded.");
        }
    }

    // CommissioningResolveDelegate
    void OnNodeDiscovered(const DiscoveredNodeData & nodeData) override
    {
        if (mCommissioningDelegate != nullptr)
        {
            mCommissioningDelegate->OnNodeDiscovered(nodeData);
        }
        else
        {
            ChipLogError(Discovery, "Missing commissioning delegate. Data discarded.");
        }
    }

private:
    OperationalResolveDelegate * mOperationalDelegate     = nullptr;
    CommissioningResolveDelegate * mCommissioningDelegate = nullptr;
};

class ResolverProxy : public Resolver
{
public:
    ResolverProxy() {}
    ~ResolverProxy() override;

    // Resolver interface.
    CHIP_ERROR Init(Inet::EndPointManager<Inet::UDPEndPoint> * udpEndPoint = nullptr) override
    {
        ReturnErrorOnFailure(chip::Dnssd::Resolver::Instance().Init(udpEndPoint));
        VerifyOrReturnError(mDelegate == nullptr, CHIP_ERROR_INCORRECT_STATE);
        mDelegate = chip::Platform::New<ResolverDelegateProxy>();

        if (mDelegate != nullptr)
        {
            if (mPreInitOperationalDelegate != nullptr)
            {
                ChipLogProgress(Discovery, "Setting operational delegate post init");
                mDelegate->SetOperationalDelegate(mPreInitOperationalDelegate);
                mPreInitOperationalDelegate = nullptr;
            }

            if (mPreInitCommissioningDelegate != nullptr)
            {
                ChipLogProgress(Discovery, "Setting commissioning delegate post init");
                mDelegate->SetCommissioningDelegate(mPreInitCommissioningDelegate);
                mPreInitCommissioningDelegate = nullptr;
            }
        }

        return mDelegate != nullptr ? CHIP_NO_ERROR : CHIP_ERROR_NO_MEMORY;
    }

    bool IsInitialized() override { return Resolver::Instance().IsInitialized(); }

    void SetOperationalDelegate(OperationalResolveDelegate * delegate) override
    {
        if (mDelegate != nullptr)
        {
            mDelegate->SetOperationalDelegate(delegate);
        }
        else
        {
            if (delegate != nullptr)
            {
                ChipLogProgress(Discovery, "Delaying proxy of operational discovery: missing delegate");
            }
            mPreInitOperationalDelegate = delegate;
        }
    }

    void SetCommissioningDelegate(CommissioningResolveDelegate * delegate) override
    {
        if (mDelegate != nullptr)
        {
            mDelegate->SetCommissioningDelegate(delegate);
        }
        else
        {
            if (delegate != nullptr)
            {
                ChipLogError(Discovery, "Delaying proxy of commissioning discovery: missing delegate");
            }
            mPreInitCommissioningDelegate = delegate;
        }
    }

    void Shutdown() override
    {
        VerifyOrReturn(mDelegate != nullptr);
        mDelegate->SetOperationalDelegate(nullptr);
        mDelegate->SetCommissioningDelegate(nullptr);
        mDelegate->Release();
        mDelegate = nullptr;
    }

    CHIP_ERROR ResolveNodeId(const PeerId & peerId) override;
    void NodeIdResolutionNoLongerNeeded(const PeerId & peerId) override;
    CHIP_ERROR DiscoverCommissionableNodes(DiscoveryFilter filter = DiscoveryFilter()) override;
    CHIP_ERROR DiscoverCommissioners(DiscoveryFilter filter = DiscoveryFilter()) override;
    CHIP_ERROR StopDiscovery() override;
    CHIP_ERROR ReconfirmRecord(const char * hostname, Inet::IPAddress address, Inet::InterfaceId interfaceId) override;

private:
    ResolverDelegateProxy * mDelegate                            = nullptr;
    OperationalResolveDelegate * mPreInitOperationalDelegate     = nullptr;
    CommissioningResolveDelegate * mPreInitCommissioningDelegate = nullptr;

    // While discovery (commissionable or commissioner) is ongoing,
    // mDiscoveryContext may have a value to allow StopDiscovery to work.
    Optional<intptr_t> mDiscoveryContext;
};

} // namespace Dnssd
} // namespace chip
