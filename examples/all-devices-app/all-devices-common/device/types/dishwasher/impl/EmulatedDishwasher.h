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

#include <device/types/dishwasher/Dishwasher.h>
#include <lib/support/TimerDelegate.h>

namespace chip::app {

class EmulatedDishwasher : public Dishwasher,
                           public Clusters::OperationalState::Delegate,
                           public Clusters::ModeBase::AppDelegate,
                           public TimerContext
{
public:
    struct Context
    {
        TimerDelegate & timerDelegate;
        DeviceLayer::DiagnosticDataProvider & diagnosticDataProvider;
    };

    explicit EmulatedDishwasher(const Context & context);
    ~EmulatedDishwasher() override;

    void Unregister(CodeDrivenDataModelProvider & provider) override;

    // -- TimerContext Interface --
    void TimerFired() override;

    // -- OperationalState::Delegate Interface --
    DataModel::Nullable<uint32_t> GetCountdownTime() override;
    CHIP_ERROR GetOperationalStateAtIndex(size_t index,
                                          Clusters::OperationalState::GenericOperationalState & operationalState) override;
    CHIP_ERROR GetOperationalPhaseAtIndex(size_t index, MutableCharSpan & operationalPhase) override;
    void HandlePauseStateCallback(Clusters::OperationalState::GenericOperationalError & err) override;
    void HandleResumeStateCallback(Clusters::OperationalState::GenericOperationalError & err) override;
    void HandleStartStateCallback(Clusters::OperationalState::GenericOperationalError & err) override;
    void HandleStopStateCallback(Clusters::OperationalState::GenericOperationalError & err) override;

    // -- ModeBase::AppDelegate Interface --
    CHIP_ERROR Init() override { return CHIP_NO_ERROR; }
    CHIP_ERROR GetModeLabelByIndex(uint8_t modeIndex, MutableCharSpan & label) override;
    CHIP_ERROR GetModeValueByIndex(uint8_t modeIndex, uint8_t & value) override;
    CHIP_ERROR GetModeTagsByIndex(uint8_t modeIndex,
                                  DataModel::List<Clusters::detail::Structs::ModeTagStruct::Type> & modeTags) override;
    void HandleChangeToMode(uint8_t newMode, Clusters::ModeBase::Commands::ChangeToModeResponse::Type & response) override;

private:
    static constexpr uint32_t kEmulatedOperationDurationSec = 30;

    static constexpr uint8_t kModeIndexNormal = 0;
    static constexpr uint8_t kModeIndexHeavy  = 1;
    static constexpr uint8_t kModeIndexLight  = 2;

    static constexpr uint8_t kModeValueNormal = kModeIndexNormal;
    static constexpr uint8_t kModeValueHeavy  = kModeIndexHeavy;
    static constexpr uint8_t kModeValueLight  = kModeIndexLight;

    static constexpr CharSpan kLabels[] = { "Normal"_span, "Heavy"_span, "Light"_span };

    void CancelTimer();
    void StartEmulatedOperationTimer(uint32_t durationSec = kEmulatedOperationDurationSec);

    TimerDelegate & mTimerDelegate;
    Clusters::OperationalState::OperationalStateEnum mOperationalState = Clusters::OperationalState::OperationalStateEnum::kStopped;
    DataModel::Nullable<uint32_t> mCountdownTime;
    System::Clock::Timestamp mOperationEndTime{};
};

} // namespace chip::app
