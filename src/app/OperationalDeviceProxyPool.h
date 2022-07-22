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

    virtual void Release(OperationalDeviceProxy * device) = 0;

    virtual OperationalDeviceProxy * FindDevice(PeerId peerId) = 0;

    virtual void ReleaseDevicesForFabric(FabricIndex fabricIndex) = 0;

    virtual void ReleaseAllDevices() = 0;

    virtual ~OperationalDeviceProxyPoolDelegate() {}
};

template <size_t N>
class OperationalDeviceProxyPool : public OperationalDeviceProxyPoolDelegate
{
public:
    ~OperationalDeviceProxyPool() override { mDevicePool.ReleaseAll(); }

    OperationalDeviceProxy * Allocate(DeviceProxyInitParams & params, PeerId peerId) override
    {
        return mDevicePool.CreateObject(params, peerId);
    }

    void Release(OperationalDeviceProxy * device) override { mDevicePool.ReleaseObject(device); }

    OperationalDeviceProxy * FindDevice(PeerId peerId) override
    {
        OperationalDeviceProxy * foundDevice = nullptr;
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
    ObjectPool<OperationalDeviceProxy, N> mDevicePool;
};

}; // namespace chip
