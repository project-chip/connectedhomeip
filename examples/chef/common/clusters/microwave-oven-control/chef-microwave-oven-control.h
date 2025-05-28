/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
 *    All rights reserved.
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

#include <app-common/zap-generated/cluster-objects.h>
#include <app/clusters/microwave-oven-control-server/microwave-oven-control-server.h>
#include <app/clusters/mode-base-server/mode-base-server.h>
#include <app/clusters/operational-state-server/operational-state-server.h>

#include "../microwave-oven-mode/chef-microwave-oven-mode.h"
#include "../../chef-operational-state-delegate-impl.h"

#include <app/util/config.h>
#include <cstring>
#include <protocols/interaction_model/StatusCode.h>
#include <utility>

#ifdef MATTER_DM_PLUGIN_MICROWAVE_OVEN_CONTROL_SERVER

namespace chip {
namespace app {
namespace Clusters {

using ModeBase::Commands::ChangeToModeResponse::Type;
using OperationalState::GenericOperationalError;
using OperationalState::GenericOperationalState;
using MicrowaveOvenMode::ModeTagStructType;

class ChefMicrowaveOvenDevice : public MicrowaveOvenControl::Delegate
{

public:
    explicit ChefMicrowaveOvenDevice(EndpointId aClustersEndpoint) :
        mMicrowaveOvenControlInstance(this, aClustersEndpoint, MicrowaveOvenControl::Id,
                                      BitMask<MicrowaveOvenControl::Feature>(MicrowaveOvenControl::Feature::kPowerAsNumber,
                                                                             MicrowaveOvenControl::Feature::kPowerNumberLimits),
                                      *OperationalState::GetOperationalStateInstance(),
                                      *MicrowaveOvenMode::GetInstance())
    {}

    void MicrowaveOvenInit();

    Protocols::InteractionModel::Status HandleSetCookingParametersCallback(uint8_t cookMode, uint32_t cookTimeSec,
                                                                           bool startAfterSetting,
                                                                           Optional<uint8_t> powerSettingNum,
                                                                           Optional<uint8_t> wattSettingIndex) override;

    Protocols::InteractionModel::Status HandleModifyCookTimeSecondsCallback(uint32_t finalcookTimeSec) override;

    CHIP_ERROR GetWattSettingByIndex(uint8_t index, uint16_t & wattSetting) override;

    uint8_t GetPowerSettingNum() const override { return mPowerSettingNum; }

    uint8_t GetMinPowerNum() const override { return kMinPowerNum; }

    uint8_t GetMaxPowerNum() const override { return kMaxPowerNum; }

    uint8_t GetPowerStepNum() const override { return kPowerStepNum; }

    uint32_t GetMaxCookTimeSec() const override { return 100; }

    uint8_t GetCurrentWattIndex() const override { return mSelectedWattIndex; };

    uint16_t GetWattRating() const override { return mWattRating; };

    app::DataModel::Nullable<uint32_t> GetCountdownTime();
    CHIP_ERROR GetOperationalStateAtIndex(size_t index, GenericOperationalState & operationalState);
    CHIP_ERROR GetOperationalPhaseAtIndex(size_t index, MutableCharSpan & operationalPhase);
    void HandlePauseStateCallback(GenericOperationalError & err);
    void HandleResumeStateCallback(GenericOperationalError & err);
    void HandleStartStateCallback(GenericOperationalError & err);
    void HandleStopStateCallback(GenericOperationalError & err);

    CHIP_ERROR Init();
    void HandleChangeToMode(uint8_t mode, Type & response);
    CHIP_ERROR GetModeLabelByIndex(uint8_t modeIndex, MutableCharSpan & label);
    CHIP_ERROR GetModeValueByIndex(uint8_t modeIndex, uint8_t & value);
    CHIP_ERROR GetModeTagsByIndex(uint8_t modeIndex, DataModel::List<ModeTagStructType> & tags);

private:
    MicrowaveOvenControl::Instance mMicrowaveOvenControlInstance;

    static constexpr uint8_t kMinPowerNum            = 20u;
    static constexpr uint8_t kMaxPowerNum            = 90u;
    static constexpr uint8_t kPowerStepNum           = 10u;
    static constexpr uint32_t kMaxCookTimeSec        = 86400u;
    static constexpr uint8_t kDefaultPowerSettingNum = kMaxPowerNum;

    uint8_t mPowerSettingNum   = kDefaultPowerSettingNum;
    uint8_t mSelectedWattIndex = 0;
    uint16_t mWattRating       = 0;
};

} // namespace Clusters
} // namespace app
} // namespace chip

void MatterMicrowaveOvenServerInit();

void MatterMicrowaveOvenServerShutdown();

#endif // MATTER_DM_PLUGIN_MICROWAVE_OVEN_CONTROL_SERVER
