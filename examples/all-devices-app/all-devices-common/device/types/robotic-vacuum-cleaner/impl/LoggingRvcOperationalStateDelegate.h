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

#include <app/clusters/mode-base-server/ModeBaseCluster.h>
#include <app/clusters/service-area-server/ServiceAreaCluster.h>
#include <clusters/RvcOperationalState/Enums.h>
#include <device/capabilities/operational-state/impl/LoggingOperationalStateDelegate.h>
#include <string>

namespace chip::app::Clusters::ServiceArea {
class LoggingServiceAreaDelegate;
} // namespace chip::app::Clusters::ServiceArea

namespace chip::app::Clusters::OperationalState {

class LoggingRvcOperationalStateDelegate : public LoggingOperationalStateDelegate
{
public:
    LoggingRvcOperationalStateDelegate() = default;

    CHIP_ERROR GetOperationalStateAtIndex(size_t index, GenericOperationalState & operationalState) override;
    void HandlePauseStateCallback(GenericOperationalError & err) override;
    void HandleResumeStateCallback(GenericOperationalError & err) override;
    void HandleGoHomeCommandCallback(GenericOperationalError & err) override;

    // Bound after construction so Resume can tell whether the RVC is mid-task (Cleaning/Mapping)
    // before allowing it to leave Charging/Docked, mirroring examples/rvc-app's RvcDevice.
    void SetRunModeCluster(ModeBaseCluster * runModeCluster) { mRunModeCluster = runModeCluster; }
    void SetServiceAreaCluster(ServiceArea::ServiceAreaCluster * serviceAreaCluster) { mServiceAreaCluster = serviceAreaCluster; }
    void SetServiceAreaDelegate(ServiceArea::LoggingServiceAreaDelegate * serviceAreaDelegate)
    {
        mServiceAreaDelegate = serviceAreaDelegate;
    }

    // Named-pipe simulation entry points ported from examples/rvc-app/rvc-common/src/rvc-device.cpp.
    void HandleCharged();
    void HandleCharging();
    void HandleDocked();
    void HandleChargerFound();
    void HandleLowCharge();
    void HandleActivityComplete();
    void HandleAreaComplete();
    void HandleClearError();
    void HandleErrorEvent(const std::string & error);

    // Clears dock/charge simulation flags when leaving dock/charge for Running,
    // mirroring examples/rvc-app/rvc-common/src/rvc-device.cpp HandleRvcRunChangeToMode().
    void ClearDockChargingTracking();

private:
    void SetDeviceToIdleState();
    void UpdateServiceAreaProgressOnExit();

    ModeBaseCluster * mRunModeCluster                              = nullptr;
    ServiceArea::ServiceAreaCluster * mServiceAreaCluster          = nullptr;
    ServiceArea::LoggingServiceAreaDelegate * mServiceAreaDelegate = nullptr;
    uint8_t mStateBeforePause                                      = 0;
    bool mDocked                                                   = false;
    bool mCharging                                                 = false;
};

} // namespace chip::app::Clusters::OperationalState
