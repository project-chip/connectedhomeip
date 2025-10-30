/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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

#include "window.h"

#include <stdint.h>
#include <string>

#include <app/data-model/Nullable.h>
#include <lib/core/DataModelTypes.h>
#include <lib/core/Optional.h>

#include <app-common/zap-generated/cluster-enums.h>

namespace example {
namespace Ui {
namespace Windows {

class FanControl : public Window
{
public:
    FanControl(chip::EndpointId endpointId) : mEndpointId(endpointId), mTitle("Fan control") {}

    void UpdateState() override;
    void Render() override;

private:
    const chip::EndpointId mEndpointId;
    const std::string mTitle;

    chip::app::Clusters::FanControl::AirflowDirectionEnum mAirflowDirection;
    chip::app::Clusters::FanControl::AirflowDirectionEnum mTargetAirflowDirection;

    chip::app::Clusters::FanControl::FanModeSequenceEnum mFanModeSequence;
    chip::app::Clusters::FanControl::FanModeSequenceEnum mTargetFanModeSequence;

    chip::app::Clusters::FanControl::FanModeEnum mFanMode;
    chip::app::Clusters::FanControl::FanModeEnum mTargetFanMode;

    chip::Percent mPercent;
    chip::Percent mTargetPercent;

    uint8_t mSpeedMax;
    uint8_t mTargetSpeedMax;

    uint8_t mSpeedCurrent;
    uint8_t mTargetSpeedCurrent;

    uint32_t mFeatureMap;
    uint32_t mTargetFeatureMap;

    uint8_t mSpeedSetting;
    chip::app::DataModel::Nullable<uint8_t> mTargetSpeedSetting;

    chip::Percent mPercentSetting;
    chip::app::DataModel::Nullable<chip::Percent> mTargetPercentSetting;

    chip::BitMask<chip::app::Clusters::FanControl::RockBitmap> mRockSupport;
    chip::BitMask<chip::app::Clusters::FanControl::RockBitmap> mTargetRockSupport;

    chip::BitMask<chip::app::Clusters::FanControl::RockBitmap> mRockSetting;
    chip::BitMask<chip::app::Clusters::FanControl::RockBitmap> mTargetRockSetting;

    chip::BitMask<chip::app::Clusters::FanControl::WindBitmap> mWindSupport;
    chip::BitMask<chip::app::Clusters::FanControl::WindBitmap> mTargetWindSupport;

    chip::BitMask<chip::app::Clusters::FanControl::WindBitmap> mWindSetting;
    chip::BitMask<chip::app::Clusters::FanControl::WindBitmap> mTargetWindSetting;
};

} // namespace Windows
} // namespace Ui
} // namespace example
