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

#include <app/CASESessionManager.h>
#include <app/OperationalSessionSetup.h>
#include <lib/support/Pool.h>
#include <transport/SessionHandle.h>

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
