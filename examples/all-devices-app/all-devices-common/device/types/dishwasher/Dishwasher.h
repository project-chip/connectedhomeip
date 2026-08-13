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
#include <clusters/DishwasherMode/Enums.h>
#include <device/api/SingleEndpoint.h>
#include <device/capabilities/operational-state/impl/EmulatedOperationalStateDelegate.h>
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

    CHIP_ERROR GetModeTagsByIndex(uint8_t modeIndex, DataModel::List<Clusters::detail::Structs::ModeTagStruct::Type> & modeTags) override
    {
        VerifyOrReturnError(modeIndex < MATTER_ARRAY_SIZE(kTagValues), CHIP_ERROR_PROVIDER_LIST_EXHAUSTED);
        VerifyOrReturnError(modeTags.size() >= 1, CHIP_ERROR_INVALID_ARGUMENT);

        modeTags[0] = { .mfgCode = std::nullopt, .value = kTagValues[modeIndex] };
        modeTags.reduce_size(1);
        return CHIP_NO_ERROR;
    }

    void HandleChangeToMode(uint8_t newMode, Clusters::ModeBase::Commands::ChangeToModeResponse::Type & response) override
    {
        response.status = to_underlying(Clusters::ModeBase::StatusCode::kSuccess);
    }

private:
    static constexpr CharSpan kLabels[] = { "Normal"_span, "Heavy"_span, "Light"_span };
    static constexpr uint16_t kTagValues[] = {
        to_underlying(Clusters::DishwasherMode::ModeTag::kNormal),
        to_underlying(Clusters::DishwasherMode::ModeTag::kHeavy),
        to_underlying(Clusters::DishwasherMode::ModeTag::kLight)
    };
};

class Dishwasher : public SingleEndpoint
{
public:
    explicit Dishwasher(DeviceLayer::DiagnosticDataProvider & diagnosticDataProvider);
    ~Dishwasher() override = default;

    CHIP_ERROR Register(EndpointId endpoint, CodeDrivenDataModelProvider & provider, EndpointComposition composition = {}) override;
    void Unregister(CodeDrivenDataModelProvider & provider) override;

    Clusters::OperationalState::OperationalStateCluster & OperationalState() { return mOperationalStateCluster.Cluster(); }
    Clusters::ModeBaseCluster & DishwasherMode() { return mDishwasherModeCluster.Cluster(); }

private:
    DeviceLayer::DiagnosticDataProvider & mDiagnosticDataProvider;
    Clusters::OperationalState::EmulatedOperationalStateDelegate mDelegate;
    LazyRegisteredServerCluster<Clusters::OperationalState::OperationalStateCluster> mOperationalStateCluster;

    DishwasherModeDelegate mDishwasherModeDelegate;
    LazyRegisteredServerCluster<Clusters::ModeBaseCluster> mDishwasherModeCluster;
};

} // namespace chip::app
