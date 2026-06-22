/*
 *    Copyright (c) 2026 Project CHIP Authors
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

#include <app/clusters/identify-server/IdentifyCluster.h>
#include <app/clusters/on-off-server/OnOffCluster.h>
#include <devices/Types.h>
#include <devices/interface/DeviceInterface.h>
#include <devices/interface/SingleEndpointDevice.h>
#include <lib/support/TimerDelegate.h>

namespace chip::app {

class CookSurfacePart : public SingleEndpointDevice
{
public:
    using SingleEndpointDevice::Register;

    explicit CookSurfacePart(TimerDelegate & timerDelegate) :
        SingleEndpointDevice(Span<const DataModel::DeviceTypeEntry>(&Device::Type::kCookSurface, 1)), mTimerDelegate(timerDelegate)
    {}
    ~CookSurfacePart() override = default;

    CHIP_ERROR Register(EndpointId endpoint, CodeDrivenDataModelProvider & provider, EndpointComposition composition) override
    {
        ReturnErrorOnFailure(RegisterDescriptor(endpoint, provider, composition));
        mIdentifyCluster.Create(Clusters::IdentifyCluster::Config(endpoint, mTimerDelegate));
        mOnOffCluster.Create(endpoint, Clusters::OnOffCluster::Context{ .timerDelegate = mTimerDelegate });
        ReturnErrorOnFailure(provider.AddCluster(mIdentifyCluster.Registration()));
        ReturnErrorOnFailure(provider.AddCluster(mOnOffCluster.Registration()));
        return provider.AddEndpoint(mEndpointRegistration);
    }

    void Unregister(CodeDrivenDataModelProvider & provider) override
    {
        UnregisterDescriptor(provider);
        if (mIdentifyCluster.IsConstructed())
        {
            LogErrorOnFailure(provider.RemoveCluster(&mIdentifyCluster.Cluster()));
            mIdentifyCluster.Destroy();
        }
        if (mOnOffCluster.IsConstructed())
        {
            LogErrorOnFailure(provider.RemoveCluster(&mOnOffCluster.Cluster()));
            mOnOffCluster.Destroy();
        }
    }

private:
    TimerDelegate & mTimerDelegate;
    LazyRegisteredServerCluster<Clusters::IdentifyCluster> mIdentifyCluster;
    LazyRegisteredServerCluster<Clusters::OnOffCluster> mOnOffCluster;
};

class CooktopDevice : public DeviceInterface
{
public:
    explicit CooktopDevice(TimerDelegate & timerDelegate);
    ~CooktopDevice() override = default;

    CHIP_ERROR Register(EndpointIdAllocator & allocator, CodeDrivenDataModelProvider & provider,
                        EndpointComposition composition = {}) override;
    void Unregister(CodeDrivenDataModelProvider & provider) override;

private:
    EndpointId mEndpointId = kInvalidEndpointId;
    CookSurfacePart mSurface1;
    CookSurfacePart mSurface2;
};

} // namespace chip::app
