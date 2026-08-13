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

#include <app/clusters/microwave-oven-control-server/MicrowaveOvenControlCluster.h>
#include <app/clusters/mode-base-server/ModeBaseCluster.h>
#include <app/clusters/operational-state-server/OperationalStateCluster.h>
#include <clusters/MicrowaveOvenMode/Enums.h>
#include <device/api/SingleEndpoint.h>
#include <device/capabilities/operational-state/impl/EmulatedOperationalStateDelegate.h>
#include <platform/DiagnosticDataProvider.h>

namespace chip::app {

class EmulatedMicrowaveOvenControlIntegrationDelegate : public Clusters::MicrowaveOvenControl::IntegrationDelegate
{
public:
    explicit EmulatedMicrowaveOvenControlIntegrationDelegate(Clusters::OperationalState::OperationalStateCluster & opStateCluster) :
        mOpStateCluster(opStateCluster)
    {}

    uint8_t GetCurrentOperationalState() const override { return mOpStateCluster.GetCurrentOperationalState(); }
    CHIP_ERROR GetNormalOperatingMode(uint8_t & mode) const override
    {
        mode = 0;
        return CHIP_NO_ERROR;
    }
    bool IsSupportedMode(uint8_t mode) const override { return mode == 0 || mode == 1; }
    bool IsSupportedOperationalStateCommand(EndpointId endpointId, CommandId commandId) const override
    {
        return commandId == Clusters::OperationalState::Commands::Start::Id;
    }

private:
    Clusters::OperationalState::OperationalStateCluster & mOpStateCluster;
};

class EmulatedMicrowaveOvenControlDelegate : public Clusters::MicrowaveOvenControl::AppDelegate
{
public:
    Protocols::InteractionModel::Status HandleSetCookingParametersCallback(uint8_t cookMode, uint32_t cookTimeSec,
                                                                           bool startAfterSetting,
                                                                           Optional<uint8_t> powerSettingNum,
                                                                           Optional<uint8_t> wattSettingIndex) override
    {
        return Protocols::InteractionModel::Status::Success;
    }

    Protocols::InteractionModel::Status HandleModifyCookTimeSecondsCallback(uint32_t finalcookTimeSec) override
    {
        return Protocols::InteractionModel::Status::Success;
    }

    CHIP_ERROR GetWattSettingByIndex(uint8_t index, uint16_t & wattSetting) override
    {
        if (index == 0)
        {
            wattSetting = 1000;
            return CHIP_NO_ERROR;
        }
        return CHIP_ERROR_NOT_FOUND;
    }

    uint32_t GetMaxCookTimeSec() const override { return 3600; }
    uint8_t GetPowerSettingNum() const override { return 100; }
    uint8_t GetMinPowerNum() const override { return 10; }
    uint8_t GetMaxPowerNum() const override { return 100; }
    uint8_t GetPowerStepNum() const override { return 10; }
    uint8_t GetCurrentWattIndex() const override { return 0; }
    uint16_t GetWattRating() const override { return 1000; }
};

class MicrowaveOvenModeDelegate : public Clusters::ModeBase::AppDelegate
{
public:
    CHIP_ERROR Init() override { return CHIP_NO_ERROR; }

    CHIP_ERROR GetModeLabelByIndex(uint8_t modeIndex, MutableCharSpan & label) override
    {
        VerifyOrReturnError(modeIndex < MATTER_ARRAY_SIZE(kLabels), CHIP_ERROR_PROVIDER_LIST_EXHAUSTED);
        return CopyCharSpanToMutableCharSpan(kLabels[modeIndex], label);
    }

    CHIP_ERROR GetModeValueByIndex(uint8_t modeIndex, uint8_t & value) override
    {
        VerifyOrReturnError(modeIndex < MATTER_ARRAY_SIZE(kLabels), CHIP_ERROR_PROVIDER_LIST_EXHAUSTED);
        value = modeIndex;
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR GetModeTagsByIndex(uint8_t modeIndex,
                                  DataModel::List<Clusters::detail::Structs::ModeTagStruct::Type> & modeTags) override
    {
        VerifyOrReturnError(modeIndex < MATTER_ARRAY_SIZE(kTagValues), CHIP_ERROR_PROVIDER_LIST_EXHAUSTED);
        VerifyOrReturnError(modeTags.size() >= 1, CHIP_ERROR_INVALID_ARGUMENT);

        modeTags[0].mfgCode.ClearValue();
        modeTags[0].value = kTagValues[modeIndex];
        modeTags.reduce_size(1);
        return CHIP_NO_ERROR;
    }

    void HandleChangeToMode(uint8_t newMode, Clusters::ModeBase::Commands::ChangeToModeResponse::Type & response) override
    {
        response.status = to_underlying(Clusters::ModeBase::StatusCode::kSuccess);
    }

private:
    static constexpr CharSpan kLabels[]    = { "Normal"_span, "Defrost"_span };
    static constexpr uint16_t kTagValues[] = { to_underlying(Clusters::MicrowaveOvenMode::ModeTag::kNormal),
                                               to_underlying(Clusters::MicrowaveOvenMode::ModeTag::kDefrost) };
};

class MicrowaveOven : public SingleEndpoint
{
public:
    struct Config
    {
        DeviceLayer::DiagnosticDataProvider & diagnosticDataProvider;
    };

    explicit MicrowaveOven(const Config & config);
    ~MicrowaveOven() override = default;

    CHIP_ERROR Register(EndpointId endpoint, CodeDrivenDataModelProvider & provider, EndpointComposition composition = {}) override;
    void Unregister(CodeDrivenDataModelProvider & provider) override;

    Clusters::OperationalState::OperationalStateCluster & OperationalState() { return mOperationalStateCluster.Cluster(); }
    Clusters::MicrowaveOvenControlCluster & MicrowaveOvenControl() { return mMicrowaveOvenControlCluster.Cluster(); }
    Clusters::ModeBaseCluster & MicrowaveOvenMode() { return mMicrowaveOvenModeCluster.Cluster(); }

private:
    DeviceLayer::DiagnosticDataProvider & mDiagnosticDataProvider;
    Clusters::OperationalState::EmulatedOperationalStateDelegate mDelegate;
    LazyRegisteredServerCluster<Clusters::OperationalState::OperationalStateCluster> mOperationalStateCluster;

    EmulatedMicrowaveOvenControlIntegrationDelegate mControlIntegrationDelegate;
    EmulatedMicrowaveOvenControlDelegate mControlAppDelegate;
    LazyRegisteredServerCluster<Clusters::MicrowaveOvenControlCluster> mMicrowaveOvenControlCluster;

    MicrowaveOvenModeDelegate mMicrowaveOvenModeDelegate;
    LazyRegisteredServerCluster<Clusters::ModeBaseCluster> mMicrowaveOvenModeCluster;
};

} // namespace chip::app
