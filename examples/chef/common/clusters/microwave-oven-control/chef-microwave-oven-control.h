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
#include <app/util/config.h>
#include <cstring>
#include <memory>
#include <protocols/interaction_model/StatusCode.h>
#include <utility>

#include "../../chef-operational-state-delegate-impl.h"
#include "../microwave-oven-mode/chef-microwave-oven-mode.h"

#ifdef MATTER_DM_PLUGIN_MICROWAVE_OVEN_CONTROL_SERVER

namespace chip {
namespace app {
namespace Clusters {

class ChefMicrowaveOvenDevice : public MicrowaveOvenControl::Delegate
{
public:
    explicit ChefMicrowaveOvenDevice(EndpointId aClustersEndpoint);

    void MicrowaveOvenInit();

    /**
     * handle command for microwave oven control: set cooking parameters
     */
    Protocols::InteractionModel::Status HandleSetCookingParametersCallback(uint8_t cookMode, uint32_t cookTimeSec,
                                                                           bool startAfterSetting,
                                                                           Optional<uint8_t> powerSettingNum,
                                                                           Optional<uint8_t> wattSettingIndex) override;

    /**
     * handle command for microwave oven control: add more time
     */
    Protocols::InteractionModel::Status HandleModifyCookTimeSecondsCallback(uint32_t finalcookTimeSec) override;

    /**
     * Get the watt setting from the supported watts array.
     * @param index The index of the watt setting to be returned.
     * It is assumed that watt setting are indexable from 0 and with no
     * gaps.
     * @param wattSetting A reference to receive the watt setting on success.
     * @return Returns a CHIP_NO_ERROR if there was no error and the label was returned successfully.
     * CHIP_ERROR_NOT_FOUND if the index in beyond the list of available labels.
     */
    CHIP_ERROR GetWattSettingByIndex(uint8_t index, uint16_t & wattSetting) override;

    uint8_t GetPowerSettingNum() const override { return mPowerSettingNum; }

    uint8_t GetMinPowerNum() const override { return kMinPowerNum; }

    uint8_t GetMaxPowerNum() const override { return kMaxPowerNum; }

    uint8_t GetPowerStepNum() const override { return kPowerStepNum; }

    uint8_t GetCurrentWattIndex() const override { return mSelectedWattIndex; };

    uint32_t GetMaxCookTimeSec() const override { return 100; }

    uint16_t GetWattRating() const override { return mWattRating; };

private:
    std::unique_ptr<chip::app::Clusters::OperationalState::OperationalStateDelegate> mOperationalStateDelegatePtr;
    std::unique_ptr<chip::app::Clusters::OperationalState::Instance> mOperationalStateInstancePtr;

    chip::app::Clusters::ModeBase::Instance * mMicrowaveOvenModeInstancePtr;

    MicrowaveOvenControl::Instance mMicrowaveOvenControlInstance;

    static constexpr uint8_t kMinPowerNum            = 20u;
    static constexpr uint8_t kMaxPowerNum            = 90u;
    static constexpr uint8_t kPowerStepNum           = 10u;
    static constexpr uint32_t kMaxCookTimeSec        = 86400u;
    static constexpr uint8_t kDefaultPowerSettingNum = kMaxPowerNum;

    uint8_t mPowerSettingNum   = kDefaultPowerSettingNum;
    uint8_t mSelectedWattIndex = 0;
    uint16_t mWattRating       = 0;

    const uint16_t mWattSettingList[5] = { 100u, 300u, 500u, 800u, 1000u };
};

} // namespace Clusters
} // namespace app
} // namespace chip

void InitChefMicrowaveOvenControlCluster();

#endif // MATTER_DM_PLUGIN_MICROWAVE_OVEN_CONTROL_SERVER
