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

#include <device/types/microwave-oven/MicrowaveOven.h>
#include <lib/support/TimerDelegate.h>

namespace chip::app {

class EmulatedMicrowaveOven : public MicrowaveOven,
                              public Clusters::OperationalState::Delegate,
                              public Clusters::MicrowaveOvenControl::IntegrationDelegate,
                              public Clusters::MicrowaveOvenControl::AppDelegate,
                              public Clusters::ModeBase::AppDelegate,
                              public TimerContext
{
public:
    struct Context
    {
        TimerDelegate & timerDelegate;
        DeviceLayer::DiagnosticDataProvider & diagnosticDataProvider;
    };

    explicit EmulatedMicrowaveOven(const Context & context);
    ~EmulatedMicrowaveOven() override;

    void Unregister(CodeDrivenDataModelProvider & provider) override;

    // -- TimerContext Interface --
    void TimerFired() override;

    // -- OperationalState::Delegate Interface --
    DataModel::Nullable<uint32_t> GetCountdownTime() override { return mCountdownTime; }
    CHIP_ERROR GetOperationalStateAtIndex(size_t index,
                                          Clusters::OperationalState::GenericOperationalState & operationalState) override;
    CHIP_ERROR GetOperationalPhaseAtIndex(size_t index, MutableCharSpan & operationalPhase) override;
    void HandlePauseStateCallback(Clusters::OperationalState::GenericOperationalError & err) override;
    void HandleResumeStateCallback(Clusters::OperationalState::GenericOperationalError & err) override;
    void HandleStartStateCallback(Clusters::OperationalState::GenericOperationalError & err) override;
    void HandleStopStateCallback(Clusters::OperationalState::GenericOperationalError & err) override;

    // -- MicrowaveOvenControl::IntegrationDelegate Interface --
    uint8_t GetCurrentOperationalState() const override { return to_underlying(mOperationalState); }
    CHIP_ERROR GetNormalOperatingMode(uint8_t & mode) const override;
    bool IsSupportedMode(uint8_t mode) const override;
    bool IsSupportedOperationalStateCommand(EndpointId endpointId, CommandId commandId) const override;

    // -- MicrowaveOvenControl::AppDelegate Interface --
    Protocols::InteractionModel::Status HandleSetCookingParametersCallback(uint8_t cookMode, uint32_t cookTimeSec,
                                                                           bool startAfterSetting,
                                                                           Optional<uint8_t> powerSettingNum,
                                                                           Optional<uint8_t> wattSettingIndex) override;
    Protocols::InteractionModel::Status HandleModifyCookTimeSecondsCallback(uint32_t finalCookTimeSec) override
    {
        return Protocols::InteractionModel::Status::Success;
    }
    CHIP_ERROR GetWattSettingByIndex(uint8_t index, uint16_t & wattSetting) override;
    uint32_t GetMaxCookTimeSec() const override { return kMaxCookTimeSec; }
    uint8_t GetPowerSettingNum() const override { return kPowerSettingNum; }
    uint8_t GetMinPowerNum() const override { return kMinPowerNum; }
    uint8_t GetMaxPowerNum() const override { return kMaxPowerNum; }
    uint8_t GetPowerStepNum() const override { return kPowerStepNum; }
    uint8_t GetCurrentWattIndex() const override { return 0; }
    uint16_t GetWattRating() const override { return kWattRating; }

    // -- ModeBase::AppDelegate Interface --
    CHIP_ERROR Init() override { return CHIP_NO_ERROR; }
    CHIP_ERROR GetModeLabelByIndex(uint8_t modeIndex, MutableCharSpan & label) override;
    CHIP_ERROR GetModeValueByIndex(uint8_t modeIndex, uint8_t & value) override;
    CHIP_ERROR GetModeTagsByIndex(uint8_t modeIndex,
                                  DataModel::List<Clusters::detail::Structs::ModeTagStruct::Type> & modeTags) override;
    void HandleChangeToMode(uint8_t newMode, Clusters::ModeBase::Commands::ChangeToModeResponse::Type & response) override;

private:
    static constexpr uint16_t kWattRating                   = 1000;
    static constexpr uint32_t kMaxCookTimeSec               = 3600;
    static constexpr uint8_t kPowerSettingNum               = 100;
    static constexpr uint8_t kMinPowerNum                   = 10;
    static constexpr uint8_t kMaxPowerNum                   = 100;
    static constexpr uint8_t kPowerStepNum                  = 10;
    static constexpr uint32_t kEmulatedOperationDurationSec = 30;

    static constexpr CharSpan kModeLabels[]    = { "Normal"_span, "Defrost"_span };
    static constexpr uint16_t kModeTagValues[] = { to_underlying(Clusters::MicrowaveOvenMode::ModeTag::kNormal),
                                                   to_underlying(Clusters::MicrowaveOvenMode::ModeTag::kDefrost) };

    void CancelTimer();
    void StartEmulatedOperationTimer();

    TimerDelegate & mTimerDelegate;
    Clusters::OperationalState::OperationalStateEnum mOperationalState = Clusters::OperationalState::OperationalStateEnum::kStopped;
    DataModel::Nullable<uint32_t> mCountdownTime;
};

} // namespace chip::app
