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
#include <device/api/Interface.h>
#include <device/api/PlatformIdentifyIntegration.h>
#include <device/api/SingleEndpoint.h>
#include <devices/Types.h>
#include <lib/support/TimerDelegate.h>

namespace chip::app {

class CookSurfacePart : public SingleEndpoint
{
public:
    using SingleEndpoint::Register;

    CookSurfacePart(TimerDelegate & timerDelegate, Clusters::OnOffDelegate & onOffDelegate,
                    Clusters::IdentifyDelegate & identifyDelegate, PlatformIdentifyIntegration & platformIdentify);
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
    PlatformIdentifyIntegration & mPlatformIdentify;

    LazyRegisteredServerCluster<Clusters::IdentifyCluster> mIdentifyCluster;
    LazyRegisteredServerCluster<Clusters::OnOffCluster> mOnOffCluster;
};

class Cooktop : public DeviceInterface
{
public:
    Cooktop();
    ~Cooktop() override = default;

    CHIP_ERROR Register(EndpointIdAllocator & allocator, CodeDrivenDataModelProvider & provider,
                        EndpointComposition composition = {}) override;
    void Unregister(CodeDrivenDataModelProvider & provider) override;

    EndpointId GetEndpointId() const { return mEndpointId; }

protected:
    virtual CHIP_ERROR RegisterParts(EndpointIdAllocator & allocator, CodeDrivenDataModelProvider & provider) = 0;
    virtual void UnregisterParts(CodeDrivenDataModelProvider & provider)                                      = 0;

private:
    EndpointId mEndpointId = kInvalidEndpointId;
};

} // namespace chip::app
