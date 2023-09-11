/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <app/CASESessionManager.h>
#include <app/OperationalSessionSetup.h>
#include <lib/support/Pool.h>
#include <transport/Session.h>

namespace chip {

class OperationalSessionSetupPoolDelegate
{
public:
    virtual OperationalSessionSetup * Allocate(const CASEClientInitParams & params, CASEClientPoolDelegate * clientPool,
                                               ScopedNodeId peerId, OperationalSessionReleaseDelegate * releaseDelegate) = 0;

    virtual void Release(OperationalSessionSetup * device) = 0;

    virtual OperationalSessionSetup * FindSessionSetup(ScopedNodeId peerId, bool forAddressUpdate) = 0;

    virtual void ReleaseAllSessionSetupsForFabric(FabricIndex fabricIndex) = 0;

    virtual void ReleaseAllSessionSetup() = 0;

    virtual ~OperationalSessionSetupPoolDelegate() {}
};

template <size_t N>
class OperationalSessionSetupPool : public OperationalSessionSetupPoolDelegate
{
public:
    ~OperationalSessionSetupPool() override { mSessionSetupPool.ReleaseAll(); }

    OperationalSessionSetup * Allocate(const CASEClientInitParams & params, CASEClientPoolDelegate * clientPool,
                                       ScopedNodeId peerId, OperationalSessionReleaseDelegate * releaseDelegate) override
    {
        return mSessionSetupPool.CreateObject(params, clientPool, peerId, releaseDelegate);
    }

    void Release(OperationalSessionSetup * device) override { mSessionSetupPool.ReleaseObject(device); }

    OperationalSessionSetup * FindSessionSetup(ScopedNodeId peerId, bool forAddressUpdate) override
    {
        OperationalSessionSetup * foundDevice = nullptr;
        mSessionSetupPool.ForEachActiveObject([&](auto * activeSetup) {
            if (activeSetup->GetPeerId() == peerId && activeSetup->IsForAddressUpdate() == forAddressUpdate)
            {
                foundDevice = activeSetup;
                return Loop::Break;
            }
            return Loop::Continue;
        });

        return foundDevice;
    }

    void ReleaseAllSessionSetupsForFabric(FabricIndex fabricIndex) override
    {
        mSessionSetupPool.ForEachActiveObject([&](auto * activeSetup) {
            if (activeSetup->GetFabricIndex() == fabricIndex)
            {
                Release(activeSetup);
            }
            return Loop::Continue;
        });
    }

    void ReleaseAllSessionSetup() override
    {
        mSessionSetupPool.ForEachActiveObject([&](auto * activeSetup) {
            Release(activeSetup);
            return Loop::Continue;
        });
    }

private:
    ObjectPool<OperationalSessionSetup, N> mSessionSetupPool;
};

}; // namespace chip
