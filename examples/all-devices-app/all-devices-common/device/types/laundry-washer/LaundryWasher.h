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

#include <app/clusters/laundry-washer-controls-server/LaundryWasherControlsCluster.h>
#include <app/clusters/mode-base-server/ModeBaseCluster.h>
#include <app/clusters/operational-state-server/OperationalStateCluster.h>
#include <clusters/LaundryWasherMode/Enums.h>
#include <device/api/SingleEndpoint.h>
#include <device/capabilities/operational-state/impl/EmulatedOperationalStateDelegate.h>
#include <platform/DiagnosticDataProvider.h>

namespace chip::app {

class EmulatedLaundryWasherControlsDelegate : public Clusters::LaundryWasherControls::Delegate
{
public:
    CHIP_ERROR GetSpinSpeedAtIndex(size_t index, MutableCharSpan & spinSpeed) override
    {
        VerifyOrReturnError(index < MATTER_ARRAY_SIZE(kSpinSpeeds), CHIP_ERROR_PROVIDER_LIST_EXHAUSTED);
        return CopyCharSpanToMutableCharSpan(kSpinSpeeds[index], spinSpeed);
    }

    CHIP_ERROR GetSupportedRinseAtIndex(size_t index, Clusters::LaundryWasherControls::NumberOfRinsesEnum & supportedRinse) override
    {
        VerifyOrReturnError(index < MATTER_ARRAY_SIZE(kRinses), CHIP_ERROR_PROVIDER_LIST_EXHAUSTED);
        supportedRinse = kRinses[index];
        return CHIP_NO_ERROR;
    }

private:
    static constexpr CharSpan kSpinSpeeds[] = { "Off"_span, "Low"_span, "Medium"_span, "High"_span };
    static constexpr Clusters::LaundryWasherControls::NumberOfRinsesEnum kRinses[] = {
        Clusters::LaundryWasherControls::NumberOfRinsesEnum::kNone,
        Clusters::LaundryWasherControls::NumberOfRinsesEnum::kNormal,
        Clusters::LaundryWasherControls::NumberOfRinsesEnum::kExtra
    };
};

class LaundryWasherModeDelegate : public Clusters::ModeBase::AppDelegate
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

    CHIP_ERROR GetModeTagsByIndex(uint8_t modeIndex, DataModel::List<Clusters::detail::Structs::ModeTagStruct::Type> & modeTags) override
    {
        VerifyOrReturnError(modeIndex < MATTER_ARRAY_SIZE(kTagValues), CHIP_ERROR_PROVIDER_LIST_EXHAUSTED);
        VerifyOrReturnError(modeTags.size() >= 1, CHIP_ERROR_INVALID_ARGUMENT);

        modeTags[0].mfgCode = std::nullopt;
        modeTags[0].value   = kTagValues[modeIndex];
        modeTags.reduce_size(1);
        return CHIP_NO_ERROR;
    }

    void HandleChangeToMode(uint8_t newMode, Clusters::ModeBase::Commands::ChangeToModeResponse::Type & response) override
    {
        response.status = to_underlying(Clusters::ModeBase::StatusCode::kSuccess);
    }

private:
    static constexpr CharSpan kLabels[] = { "Normal"_span, "Delicate"_span, "Heavy"_span };
    static constexpr uint16_t kTagValues[] = {
        to_underlying(Clusters::LaundryWasherMode::ModeTag::kNormal),
        to_underlying(Clusters::LaundryWasherMode::ModeTag::kDelicate),
        to_underlying(Clusters::LaundryWasherMode::ModeTag::kHeavy)
    };
};

class LaundryWasher : public SingleEndpoint
{
public:
    explicit LaundryWasher(DeviceLayer::DiagnosticDataProvider & diagnosticDataProvider);
    ~LaundryWasher() override = default;

    CHIP_ERROR Register(EndpointId endpoint, CodeDrivenDataModelProvider & provider, EndpointComposition composition = {}) override;
    void Unregister(CodeDrivenDataModelProvider & provider) override;

    Clusters::OperationalState::OperationalStateCluster & OperationalState() { return mOperationalStateCluster.Cluster(); }
    Clusters::LaundryWasherControlsCluster & LaundryWasherControls() { return mLaundryWasherControlsCluster.Cluster(); }
    Clusters::ModeBaseCluster & LaundryWasherMode() { return mLaundryWasherModeCluster.Cluster(); }

private:
    DeviceLayer::DiagnosticDataProvider & mDiagnosticDataProvider;
    Clusters::OperationalState::EmulatedOperationalStateDelegate mDelegate;
    LazyRegisteredServerCluster<Clusters::OperationalState::OperationalStateCluster> mOperationalStateCluster;

    EmulatedLaundryWasherControlsDelegate mLaundryWasherControlsDelegate;
    LazyRegisteredServerCluster<Clusters::LaundryWasherControlsCluster> mLaundryWasherControlsCluster;

    LaundryWasherModeDelegate mLaundryWasherModeDelegate;
    LazyRegisteredServerCluster<Clusters::ModeBaseCluster> mLaundryWasherModeCluster;
};

} // namespace chip::app
