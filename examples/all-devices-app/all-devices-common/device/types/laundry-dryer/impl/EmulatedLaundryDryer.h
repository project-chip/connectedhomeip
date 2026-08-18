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

#include <device/types/laundry-dryer/LaundryDryer.h>
#include <lib/support/TimerDelegate.h>

namespace chip::app {

class EmulatedLaundryDryer : public LaundryDryer,
                             public Clusters::OperationalState::Delegate,
                             public TimerContext
{
public:
    explicit EmulatedLaundryDryer(TimerDelegate & timerDelegate);
    ~EmulatedLaundryDryer() override;

    void Unregister(CodeDrivenDataModelProvider & provider) override;

    // -- TimerContext Interface --
    void TimerFired() override;

    // -- OperationalState::Delegate Interface --
    DataModel::Nullable<uint32_t> GetCountdownTime() override { return mCountdownTime; }
    CHIP_ERROR GetOperationalStateAtIndex(size_t index, Clusters::OperationalState::GenericOperationalState & operationalState) override;
    CHIP_ERROR GetOperationalPhaseAtIndex(size_t index, MutableCharSpan & operationalPhase) override;
    void HandlePauseStateCallback(Clusters::OperationalState::GenericOperationalError & err) override;
    void HandleResumeStateCallback(Clusters::OperationalState::GenericOperationalError & err) override;
    void HandleStartStateCallback(Clusters::OperationalState::GenericOperationalError & err) override;
    void HandleStopStateCallback(Clusters::OperationalState::GenericOperationalError & err) override;

private:
    static constexpr uint32_t kEmulatedOperationDurationSec = 30;

    void CancelTimer();
    void StartEmulatedOperationTimer();

    TimerDelegate & mTimerDelegate;
    Clusters::OperationalState::OperationalStateEnum mOperationalState = Clusters::OperationalState::OperationalStateEnum::kStopped;
    DataModel::Nullable<uint32_t> mCountdownTime;
};

} // namespace chip::app
