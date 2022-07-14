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
    virtual OperationalSessionSetup * Allocate(DeviceProxyInitParams & params, ScopedNodeId peerId,
                                               OperationalSessionReleaseDelegate * releaseDelegate) = 0;

    virtual void Release(OperationalSessionSetup * device) = 0;

    virtual OperationalSessionSetup * FindDevice(ScopedNodeId peerId) = 0;

    virtual void ReleaseDevicesForFabric(FabricIndex fabricIndex) = 0;

    virtual void ReleaseAllDevices() = 0;

    virtual ~OperationalSessionSetupPoolDelegate() {}
};

template <size_t N>
class OperationalSessionSetupPool : public OperationalSessionSetupPoolDelegate
{
public:
    ~OperationalSessionSetupPool() override { mDevicePool.ReleaseAll(); }

    OperationalSessionSetup * Allocate(DeviceProxyInitParams & params, ScopedNodeId peerId,
                                       OperationalSessionReleaseDelegate * releaseDelegate) override
    {
        return mDevicePool.CreateObject(params, peerId, releaseDelegate);
    }

    void Release(OperationalSessionSetup * device) override { mDevicePool.ReleaseObject(device); }

    OperationalSessionSetup * FindDevice(ScopedNodeId peerId) override
    {
        OperationalSessionSetup * foundDevice = nullptr;
        mDevicePool.ForEachActiveObject([&](auto * activeDevice) {
            if (activeDevice->GetPeerId() == peerId)
            {
                foundDevice = activeDevice;
                return Loop::Break;
            }
            return Loop::Continue;
        });

        return foundDevice;
    }

    void ReleaseDevicesForFabric(FabricIndex fabricIndex) override
    {
        mDevicePool.ForEachActiveObject([&](auto * activeDevice) {
            if (activeDevice->GetFabricIndex() == fabricIndex)
            {
                Release(activeDevice);
            }
            return Loop::Continue;
        });
    }

    void ReleaseAllDevices() override
    {
        mDevicePool.ForEachActiveObject([&](auto * activeDevice) {
            Release(activeDevice);
            return Loop::Continue;
        });
    }

private:
    ObjectPool<OperationalSessionSetup, N> mDevicePool;
};

}; // namespace chip
