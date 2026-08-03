#pragma once

#include <app/clusters/identify-server/IdentifyCluster.h>
#include <app/clusters/window-covering-server/WindowCoveringCluster.h>
#include <data-model-providers/codedriven/CodeDrivenDataModelProvider.h>
#include <devices/interface/SingleEndpointDevice.h>
#include <lib/support/TimerDelegate.h>

namespace chip::app {

class WindowCovering : public SingleEndpointDevice
{
public:
    WindowCovering(WindowCoveringDelegate & delegate, TimerDelegate & timerDelegate, Clusters::IdentifyDelegate & identifyDelegate);
    ~WindowCovering() override = default;

    // DeviceInterface pure virtual lifecycle hooks
    CHIP_ERROR Register(chip::EndpointId endpoint, CodeDrivenDataModelProvider & provider,
                        EndpointComposition composition = {}) override;
    void Unregister(CodeDrivenDataModelProvider & provider) override;

    // Public cluster getters for programmatic control
    Clusters::IdentifyCluster & IdentifyCluster() { return mIdentifyCluster.Cluster(); }
    Clusters::WindowCoveringCluster & WindowCoveringCluster() { return mWindowCoveringCluster.Cluster(); }
    Clusters::GroupsCluster & GroupsCluster() { return mGroupsCluster.Cluster(); }

private:
    Clusters::WindowCovering::WindowCoveringDelegate & mWindowCoveringDelagate;
    TimerDelegate & mTimerDelegate;
    Clusters::IdentifyDelegate & mIdentifyDelegate;

    LazyRegisteredServerCluster<Clusters::IdentifyCluster> mIdentifyCluster;
    LazyRegisteredServerCluster<Clusters::WindowCoveringCluster> mWindowCoveringCluster;
    // LazyRegisteredServerCluster<Clusters::Groups> mGroupsCluster;
};

} // namespace chip::app
