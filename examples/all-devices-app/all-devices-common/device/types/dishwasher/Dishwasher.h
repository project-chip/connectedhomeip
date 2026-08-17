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
#include <app/clusters/operational-state-server/OperationalStateCluster.h>
#include <app/clusters/operational-state-server/OperationalStateDelegate.h>
#include <clusters/DishwasherMode/Enums.h>
#include <device/api/SingleEndpoint.h>
#include <platform/DiagnosticDataProvider.h>

namespace chip::app {

class DishwasherModeDelegate : public Clusters::ModeBase::AppDelegate
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

    // Mode indexes correspond to 0-based positions in the kLabels / SupportedModes list.
    static constexpr uint8_t kModeIndexNormal = 0;
    static constexpr uint8_t kModeIndexHeavy  = 1;
    static constexpr uint8_t kModeIndexLight  = 2;

    // Mode values are transmitted in command payloads (newMode). They are defined separately to
    // maintain semantic domain separation from list indexes, but are intentionally set equal to
    // the mode indexes here so GetModeValueByIndex returns value = modeIndex.
    static constexpr uint8_t kModeValueNormal = kModeIndexNormal;
    static constexpr uint8_t kModeValueHeavy  = kModeIndexHeavy;
    static constexpr uint8_t kModeValueLight  = kModeIndexLight;

    CHIP_ERROR GetModeTagsByIndex(uint8_t modeIndex,
                                  DataModel::List<Clusters::detail::Structs::ModeTagStruct::Type> & modeTags) override
    {
        VerifyOrReturnError(modeIndex < MATTER_ARRAY_SIZE(kLabels), CHIP_ERROR_PROVIDER_LIST_EXHAUSTED);

        switch (modeIndex)
        {
        case kModeIndexNormal:
            VerifyOrReturnError(modeTags.size() >= 1, CHIP_ERROR_INVALID_ARGUMENT);
            modeTags[0].mfgCode.ClearValue();
            modeTags[0].value = to_underlying(Clusters::DishwasherMode::ModeTag::kNormal);
            modeTags.reduce_size(1);
            break;
        case kModeIndexHeavy:
            VerifyOrReturnError(modeTags.size() >= 2, CHIP_ERROR_INVALID_ARGUMENT);
            modeTags[0].mfgCode.ClearValue();
            modeTags[0].value = to_underlying(Clusters::DishwasherMode::ModeTag::kHeavy);
            modeTags[1].mfgCode.ClearValue();
            modeTags[1].value = to_underlying(Clusters::DishwasherMode::ModeTag::kMax);
            modeTags.reduce_size(2);
            break;
        case kModeIndexLight:
            VerifyOrReturnError(modeTags.size() >= 1, CHIP_ERROR_INVALID_ARGUMENT);
            modeTags[0].mfgCode.ClearValue();
            modeTags[0].value = to_underlying(Clusters::DishwasherMode::ModeTag::kLight);
            modeTags.reduce_size(1);
            break;
        default:
            return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
        }
        return CHIP_NO_ERROR;
    }

    void HandleChangeToMode(uint8_t newMode, Clusters::ModeBase::Commands::ChangeToModeResponse::Type & response) override
    {
        if (newMode == kModeValueLight)
        {
            response.status = to_underlying(Clusters::ModeBase::StatusCode::kInvalidInMode);
            response.statusText.SetValue("Invalid in current state"_span);
            return;
        }
        response.status = to_underlying(Clusters::ModeBase::StatusCode::kSuccess);
    }

private:
    static constexpr CharSpan kLabels[] = { "Normal"_span, "Heavy"_span, "Light"_span };
};

class Dishwasher : public SingleEndpoint
{
public:
    struct Config
    {
        Clusters::OperationalState::OperationalStateCluster::Delegate & operationalStateDelegate;
        Clusters::ModeBase::AppDelegate & modeDelegate;
        DeviceLayer::DiagnosticDataProvider & diagnosticDataProvider;
    };

    explicit Dishwasher(const Config & config);
    ~Dishwasher() override = default;

    CHIP_ERROR Register(EndpointId endpoint, CodeDrivenDataModelProvider & provider, EndpointComposition composition = {}) override;
    void Unregister(CodeDrivenDataModelProvider & provider) override;

    Clusters::OperationalState::OperationalStateCluster & OperationalState() { return mOperationalStateCluster.Cluster(); }
    Clusters::ModeBaseCluster & DishwasherMode() { return mDishwasherModeCluster.Cluster(); }

private:
    DeviceLayer::DiagnosticDataProvider & mDiagnosticDataProvider;
    Clusters::OperationalState::OperationalStateCluster::Delegate & mOperationalStateDelegate;
    LazyRegisteredServerCluster<Clusters::OperationalState::OperationalStateCluster> mOperationalStateCluster;

    Clusters::ModeBase::AppDelegate & mDishwasherModeDelegate;
    LazyRegisteredServerCluster<Clusters::ModeBaseCluster> mDishwasherModeCluster;
};

} // namespace chip::app
