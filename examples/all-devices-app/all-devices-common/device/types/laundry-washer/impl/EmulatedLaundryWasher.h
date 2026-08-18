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

#include <device/types/laundry-washer/LaundryWasher.h>
#include <lib/support/TimerDelegate.h>

namespace chip::app {

class EmulatedLaundryWasher : public LaundryWasher,
                              public Clusters::OperationalState::Delegate,
                              public Clusters::LaundryWasherControls::Delegate,
                              public Clusters::ModeBase::AppDelegate,
                              public TimerContext
{
public:
    struct Context
    {
        TimerDelegate & timerDelegate;
        DeviceLayer::DiagnosticDataProvider & diagnosticDataProvider;
    };

    explicit EmulatedLaundryWasher(const Context & context);
    ~EmulatedLaundryWasher() override;

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

    // -- LaundryWasherControls::Delegate Interface --
    CHIP_ERROR GetSpinSpeedAtIndex(size_t index, MutableCharSpan & spinSpeed) override;
    CHIP_ERROR GetSupportedRinseAtIndex(size_t index, Clusters::LaundryWasherControls::NumberOfRinsesEnum & supportedRinse) override;

    // -- ModeBase::AppDelegate Interface --
    CHIP_ERROR Init() override { return CHIP_NO_ERROR; }
    CHIP_ERROR GetModeLabelByIndex(uint8_t modeIndex, MutableCharSpan & label) override;
    CHIP_ERROR GetModeValueByIndex(uint8_t modeIndex, uint8_t & value) override;
    CHIP_ERROR GetModeTagsByIndex(uint8_t modeIndex,
                                  DataModel::List<Clusters::detail::Structs::ModeTagStruct::Type> & modeTags) override;
    void HandleChangeToMode(uint8_t newMode, Clusters::ModeBase::Commands::ChangeToModeResponse::Type & response) override;

private:
    static constexpr uint32_t kEmulatedOperationDurationSec = 30;

    static constexpr CharSpan kSpinSpeeds[] = { "Off"_span, "Low"_span, "Medium"_span, "High"_span };
    static constexpr Clusters::LaundryWasherControls::NumberOfRinsesEnum kRinses[] = {
        Clusters::LaundryWasherControls::NumberOfRinsesEnum::kNone,
        Clusters::LaundryWasherControls::NumberOfRinsesEnum::kNormal,
        Clusters::LaundryWasherControls::NumberOfRinsesEnum::kExtra
    };

    static constexpr CharSpan kModeLabels[]    = { "Normal"_span, "Delicate"_span, "Heavy"_span };
    static constexpr uint16_t kModeTagValues[] = { to_underlying(Clusters::LaundryWasherMode::ModeTag::kNormal),
                                                   to_underlying(Clusters::LaundryWasherMode::ModeTag::kDelicate),
                                                   to_underlying(Clusters::LaundryWasherMode::ModeTag::kHeavy) };

    void CancelTimer();
    void StartEmulatedOperationTimer();

    TimerDelegate & mTimerDelegate;
    Clusters::OperationalState::OperationalStateEnum mOperationalState = Clusters::OperationalState::OperationalStateEnum::kStopped;
    DataModel::Nullable<uint32_t> mCountdownTime;
};

} // namespace chip::app
