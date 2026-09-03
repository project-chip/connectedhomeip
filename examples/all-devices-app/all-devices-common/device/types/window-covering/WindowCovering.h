/*
 *
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
#include <app/clusters/window-covering-server/WindowCoveringCluster.h>
#include <device/api/SingleEndpoint.h>
#include <lib/support/BitFlags.h>
#include <lib/support/TimerDelegate.h>

namespace chip {
namespace Credentials {
class GroupDataProvider;
} // namespace Credentials

namespace app {

class WindowCovering : public SingleEndpoint
{
public:
    struct Context
    {
        Credentials::GroupDataProvider & groupDataProvider;
        TimerDelegate & timerDelegate;
    };

    WindowCovering(Clusters::WindowCovering::WindowCoveringDelegate & delegate, Clusters::IdentifyDelegate & identifyDelegate,
                   const Context & context, BitFlags<Clusters::WindowCovering::Feature> features,
                   Clusters::WindowCovering::OptionalAttributeSet optionalAttributes = {});
    ~WindowCovering() override = default;

    // DeviceInterface pure virtual lifecycle hooks
    CHIP_ERROR Register(chip::EndpointId endpoint, CodeDrivenDataModelProvider & provider,
                        EndpointComposition composition = {}) override;
    void Unregister(CodeDrivenDataModelProvider & provider) override;

    // Public cluster getters for programmatic control
    Clusters::IdentifyCluster & IdentifyCluster() { return mIdentifyCluster.Cluster(); }
    Clusters::WindowCovering::WindowCoveringCluster & WindowCoveringCluster() { return mWindowCoveringCluster.Cluster(); }

protected:
    virtual CHIP_ERROR RegisterOptionalClusters(chip::EndpointId endpoint, CodeDrivenDataModelProvider & provider)
    {
        return CHIP_NO_ERROR;
    }

    virtual void UnregisterOptionalClusters(CodeDrivenDataModelProvider & provider) {}

    TimerDelegate & mTimerDelegate;
    const Clusters::WindowCovering::OptionalAttributeSet mOptionalAttributes;
    const Context mContext;
    const BitFlags<Clusters::WindowCovering::Feature> mFeatures;

private:
    Clusters::WindowCovering::WindowCoveringDelegate & mWindowCoveringDelegate;
    Clusters::IdentifyDelegate & mIdentifyDelegate;

    LazyRegisteredServerCluster<Clusters::IdentifyCluster> mIdentifyCluster;
    LazyRegisteredServerCluster<Clusters::WindowCovering::WindowCoveringCluster> mWindowCoveringCluster;
};

} // namespace app
} // namespace chip
