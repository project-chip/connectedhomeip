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

#include <optional>

#include <device/capabilities/operational-state/impl/EmulatedOperationalStateDelegate.h>
#include <device/types/microwave-oven/MicrowaveOven.h>

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
    void SetOperationalStateDelegate(Clusters::OperationalState::EmulatedOperationalStateDelegate * opStateDelegate)
    {
        mOpStateDelegate = opStateDelegate;
    }

    Protocols::InteractionModel::Status HandleSetCookingParametersCallback(uint8_t cookMode, uint32_t cookTimeSec,
                                                                           bool startAfterSetting,
                                                                           Optional<uint8_t> powerSettingNum,
                                                                           Optional<uint8_t> wattSettingIndex) override
    {
        if (startAfterSetting && mOpStateDelegate != nullptr)
        {
            mOpStateDelegate->StartEmulatedOperationTimer();
        }
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

private:
    Clusters::OperationalState::EmulatedOperationalStateDelegate * mOpStateDelegate = nullptr;
};

class EmulatedMicrowaveOven : public MicrowaveOven
{
public:
    struct Context
    {
        DeviceLayer::DiagnosticDataProvider & diagnosticDataProvider;
    };

    explicit EmulatedMicrowaveOven(const Context & context) :
        MicrowaveOven(MicrowaveOven::Config{
            .operationalStateDelegate    = mOpStateDelegate,
            .controlIntegrationDelegate = mControlIntegrationDelegate,
            .controlAppDelegate         = mControlAppDelegate,
            .modeDelegate               = mModeDelegate,
            .diagnosticDataProvider     = context.diagnosticDataProvider,
        }),
        mControlIntegrationDelegate(OperationalState())
    {}

    ~EmulatedMicrowaveOven() override = default;

    CHIP_ERROR Register(EndpointId endpoint, CodeDrivenDataModelProvider & provider, EndpointComposition composition = {}) override
    {
        ReturnErrorOnFailure(MicrowaveOven::Register(endpoint, provider, composition));
        mOpStateDelegate.SetCluster(&OperationalState());
        mControlAppDelegate.SetOperationalStateDelegate(&mOpStateDelegate);
        return CHIP_NO_ERROR;
    }

    void Unregister(CodeDrivenDataModelProvider & provider) override
    {
        mOpStateDelegate.SetCluster(nullptr);
        MicrowaveOven::Unregister(provider);
    }

private:
    Clusters::OperationalState::EmulatedOperationalStateDelegate mOpStateDelegate;
    EmulatedMicrowaveOvenControlIntegrationDelegate mControlIntegrationDelegate;
    EmulatedMicrowaveOvenControlDelegate mControlAppDelegate;
    MicrowaveOvenModeDelegate mModeDelegate;
};

} // namespace chip::app
