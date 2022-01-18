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

#include <app/OperationalDeviceProxy.h>
#include <lib/support/Pool.h>
#include <transport/SessionHandle.h>

namespace chip {

class OperationalDeviceProxyPoolDelegate
{
public:
    virtual OperationalDeviceProxy * Allocate(DeviceProxyInitParams & params, PeerId peerId) = 0;

    virtual OperationalDeviceProxy * Allocate(DeviceProxyInitParams & params, PeerId peerId,
                                              const Dnssd::ResolvedNodeData & nodeResolutionData) = 0;

    virtual OperationalDeviceProxy * Retain(PeerId peerId) = 0;

    virtual void Release(PeerId peerId) = 0;

    virtual void ReleaseAll() = 0;

    virtual OperationalDeviceProxy * FindDevice(const SessionHandle & session) = 0;

    virtual OperationalDeviceProxy * FindDevice(PeerId peerId) = 0;

    virtual ~OperationalDeviceProxyPoolDelegate() {}
};

template <size_t N>
class OperationalDeviceProxyPool : public OperationalDeviceProxyPoolDelegate
{
    class OperationalDeviceProxyRefCounted;
    using PoolType = BitMapObjectPool<OperationalDeviceProxyRefCounted, N>;

    class OperationalDeviceProxyDeleter
    {
    public:
        static void Release(OperationalDeviceProxyRefCounted * obj);
    };

    class OperationalDeviceProxyRefCounted
        : public ReferenceCounted<OperationalDeviceProxyRefCounted, OperationalDeviceProxyDeleter>
    {
        friend class OperationalDeviceProxyDeleter;

    public:
        OperationalDeviceProxyRefCounted(DeviceProxyInitParams & params, PeerId peerId, PoolType * pool) :
            mDevice(params, peerId), mDevicePool(pool)
        {}

        OperationalDeviceProxyRefCounted(DeviceProxyInitParams & params, PeerId peerId,
                                         const Dnssd::ResolvedNodeData & nodeResolutionData, PoolType * pool) :
            mDevice(params, peerId, nodeResolutionData),
            mDevicePool(pool)
        {}

        OperationalDeviceProxy * GetDevice() { return &mDevice; }

    private:
        OperationalDeviceProxy mDevice;
        PoolType * mDevicePool;
    };

public:
    OperationalDeviceProxy * Allocate(DeviceProxyInitParams & params, PeerId peerId) override
    {
        OperationalDeviceProxyRefCounted * device = mDevicePool.CreateObject(params, peerId, &mDevicePool);
        if (device)
        {
            return device->GetDevice();
        }
        return nullptr;
    }

    OperationalDeviceProxy * Allocate(DeviceProxyInitParams & params, PeerId peerId,
                                      const Dnssd::ResolvedNodeData & nodeResolutionData) override
    {
        OperationalDeviceProxyRefCounted * device = mDevicePool.CreateObject(params, peerId, nodeResolutionData, &mDevicePool);
        if (device)
        {
            return device->GetDevice();
        }
        return nullptr;
    }

    OperationalDeviceProxy * Retain(PeerId peerId) override
    {
        OperationalDeviceProxyRefCounted * device = FindDeviceRefCounted(peerId);
        return device ? device->Retain()->GetDevice() : nullptr;
    }

    void Release(PeerId peerId) override
    {
        OperationalDeviceProxyRefCounted * device = FindDeviceRefCounted(peerId);
        if (device)
        {
            device->Release();
        }
    }

    void ReleaseAll() override
    {
        mDevicePool.ForEachActiveObject([&](auto * activeDevice) {
            activeDevice->GetDevice()->Disconnect();
            activeDevice->GetDevice()->Clear();
            return Loop::Continue;
        });
        mDevicePool.ReleaseAll();
    }

    OperationalDeviceProxy * FindDevice(const SessionHandle & session) override
    {
        OperationalDeviceProxyRefCounted * device = FindDeviceRefCounted(session);
        return device ? device->GetDevice() : nullptr;
    }

    OperationalDeviceProxy * FindDevice(PeerId peerId) override
    {
        OperationalDeviceProxyRefCounted * device = FindDeviceRefCounted(peerId);
        return device ? device->GetDevice() : nullptr;
    }

    ~OperationalDeviceProxyPool() { ReleaseAll(); }

private:
    OperationalDeviceProxyRefCounted * FindDeviceRefCounted(const SessionHandle & session)
    {
        OperationalDeviceProxyRefCounted * foundDevice = nullptr;
        mDevicePool.ForEachActiveObject([&](auto * activeDevice) {
            if (activeDevice->GetDevice()->MatchesSession(session))
            {
                foundDevice = activeDevice;
                return Loop::Break;
            }
            return Loop::Continue;
        });

        return foundDevice;
    }

    OperationalDeviceProxyRefCounted * FindDeviceRefCounted(PeerId peerId)
    {
        OperationalDeviceProxyRefCounted * foundDevice = nullptr;
        mDevicePool.ForEachActiveObject([&](auto * activeDevice) {
            if (activeDevice->GetDevice()->GetPeerId() == peerId)
            {
                foundDevice = activeDevice;
                return Loop::Break;
            }
            return Loop::Continue;
        });

        return foundDevice;
    }

    PoolType mDevicePool;
};

template <size_t N>
void OperationalDeviceProxyPool<N>::OperationalDeviceProxyDeleter::Release(OperationalDeviceProxyRefCounted * obj)
{
    obj->GetDevice()->Disconnect();
    obj->GetDevice()->Clear();
    obj->mDevicePool->ReleaseObject(obj);
}

}; // namespace chip
