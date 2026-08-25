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

#include <app/clusters/mode-base-server/AppDelegate.h>
#include <app/clusters/mode-base-server/ModeBaseCluster.h>
#include <clusters/RvcCleanMode/Enums.h>

namespace chip::app::Clusters::RvcCleanMode {

class LoggingRvcCleanModeDelegate : public ModeBase::AppDelegate
{
public:
    // The cleaning mode may only change while the RVC Run Mode cluster is Idle, mirroring
    // examples/rvc-app/rvc-common/src/rvc-device.cpp's RvcDevice.
    explicit LoggingRvcCleanModeDelegate(ModeBaseCluster & runModeCluster) : mRunModeCluster(runModeCluster) {}

    CHIP_ERROR Init() override { return CHIP_NO_ERROR; }

    CHIP_ERROR GetModeLabelByIndex(uint8_t modeIndex, MutableCharSpan & label) override;
    CHIP_ERROR GetModeValueByIndex(uint8_t modeIndex, uint8_t & value) override;
    CHIP_ERROR GetModeTagsByIndex(uint8_t modeIndex, DataModel::List<detail::Structs::ModeTagStruct::Type> & modeTags) override;
    void HandleChangeToMode(uint8_t newMode, ModeBase::Commands::ChangeToModeResponse::Type & response) override;

private:
    ModeBaseCluster & mRunModeCluster;
};

} // namespace chip::app::Clusters::RvcCleanMode
