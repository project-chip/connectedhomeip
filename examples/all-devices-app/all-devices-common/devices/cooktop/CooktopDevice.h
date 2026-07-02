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
#include <app/clusters/on-off-server/OnOffDelegate.h>
#include <devices/Types.h>
#include <devices/interface/DeviceInterface.h>
#include <devices/interface/SingleEndpointDevice.h>
#include <lib/support/TimerDelegate.h>

namespace chip::app {

class CookSurfacePart : public SingleEndpointDevice
{
public:
    using SingleEndpointDevice::Register;

    CookSurfacePart(TimerDelegate & timerDelegate, Clusters::OnOffDelegate & onOffDelegate,
                    Clusters::IdentifyDelegate & identifyDelegate);
    ~CookSurfacePart() override = default;

    CHIP_ERROR Register(EndpointId endpoint, CodeDrivenDataModelProvider & provider, EndpointComposition composition) override;
    void Unregister(CodeDrivenDataModelProvider & provider) override;

    // Public cluster getters for programmatic control
    Clusters::OnOffCluster & OnOffCluster() { return mOnOffCluster.Cluster(); }
    Clusters::IdentifyCluster & IdentifyCluster() { return mIdentifyCluster.Cluster(); }

private:
    TimerDelegate & mTimerDelegate;
    Clusters::OnOffDelegate & mOnOffDelegate;
    Clusters::IdentifyDelegate & mIdentifyDelegate;

    LazyRegisteredServerCluster<Clusters::IdentifyCluster> mIdentifyCluster;
    LazyRegisteredServerCluster<Clusters::OnOffCluster> mOnOffCluster;
};

class CooktopDevice : public DeviceInterface
{
public:
    CooktopDevice(TimerDelegate & timerDelegate, Clusters::OnOffDelegate & surface1OnOff, Clusters::OnOffDelegate & surface2OnOff,
                  Clusters::IdentifyDelegate & surface1Identify, Clusters::IdentifyDelegate & surface2Identify);
    ~CooktopDevice() override = default;

    CHIP_ERROR Register(EndpointIdAllocator & allocator, CodeDrivenDataModelProvider & provider,
                        EndpointComposition composition = {}) override;
    void Unregister(CodeDrivenDataModelProvider & provider) override;

    // Composition getters to expose child endpoints
    CookSurfacePart & Surface1() { return mSurface1; }
    CookSurfacePart & Surface2() { return mSurface2; }

private:
    EndpointId mEndpointId = kInvalidEndpointId;
    CookSurfacePart mSurface1;
    CookSurfacePart mSurface2;
};

} // namespace chip::app
