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

#include <device/types/temperature-controlled-cabinet/TemperatureControlledCabinetPart.h>
#include <lib/support/TimerDelegate.h>
#include <lib/support/logging/CHIPLogging.h>

namespace chip::app {

class LoggingTemperatureControlledCabinetPart : public TemperatureControlledCabinetPart,
                                                public Clusters::OperationalState::Delegate,
                                                public Clusters::IdentifyDelegate,
                                                public TimerContext
{
public:
    LoggingTemperatureControlledCabinetPart(TimerDelegate & timerDelegate, PlatformIdentifyIntegration & platformIdentify,
                                            const char * name);
    LoggingTemperatureControlledCabinetPart(TimerDelegate & timerDelegate, PlatformIdentifyIntegration & platformIdentify,
                                            Config config, const char * name);
    ~LoggingTemperatureControlledCabinetPart() override;

    void Unregister(CodeDrivenDataModelProvider & provider) override;

    // TimerContext Interface
    void TimerFired() override;

    // OperationalState::Delegate Interface
    DataModel::Nullable<uint32_t> GetCountdownTime() override { return mCountdownTime; }
    CHIP_ERROR GetOperationalStateAtIndex(size_t index,
                                          Clusters::OperationalState::GenericOperationalState & operationalState) override;
    CHIP_ERROR GetOperationalPhaseAtIndex(size_t index, MutableCharSpan & operationalPhase) override;
    void HandlePauseStateCallback(Clusters::OperationalState::GenericOperationalError & err) override;
    void HandleResumeStateCallback(Clusters::OperationalState::GenericOperationalError & err) override;
    void HandleStartStateCallback(Clusters::OperationalState::GenericOperationalError & err) override;
    void HandleStopStateCallback(Clusters::OperationalState::GenericOperationalError & err) override;

    // IdentifyDelegate Interface
    void OnIdentifyStart(Clusters::IdentifyCluster & cluster) override;
    void OnIdentifyStop(Clusters::IdentifyCluster & cluster) override;
    void OnTriggerEffect(Clusters::IdentifyCluster & cluster) override;
    bool IsTriggerEffectEnabled() const override { return true; }

private:
    static constexpr uint32_t kEmulatedOperationDurationSec = 30;

    void CancelTimer();
    void StartEmulatedOperationTimer();

    const char * mName;
    TimerDelegate & mTimerDelegate;
    PlatformIdentifyIntegration & mPlatformIdentify;
    Clusters::OperationalState::OperationalStateEnum mOperationalState = Clusters::OperationalState::OperationalStateEnum::kStopped;
    DataModel::Nullable<uint32_t> mCountdownTime;
};

} // namespace chip::app
