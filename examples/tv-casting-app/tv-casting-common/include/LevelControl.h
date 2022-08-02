/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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

#include "MediaCommandBase.h"

#include <functional>
#include <zap-generated/CHIPClusters.h>

class StepCommand
    : public MediaCommandBase<chip::app::Clusters::LevelControl::Commands::Step::Type, chip::app::DataModel::NullObjectType>
{
public:
    StepCommand() : MediaCommandBase(chip::app::Clusters::LevelControl::Id) {}

    CHIP_ERROR Invoke(chip::app::Clusters::LevelControl::StepMode stepMode, uint8_t stepSize,
                      chip::app::DataModel::Nullable<uint16_t> transitionTime, uint8_t optionMask, uint8_t optionOverride,
                      std::function<void(CHIP_ERROR)> responseCallback);
};

class MoveToLevelCommand
    : public MediaCommandBase<chip::app::Clusters::LevelControl::Commands::MoveToLevel::Type, chip::app::DataModel::NullObjectType>
{
public:
    MoveToLevelCommand() : MediaCommandBase(chip::app::Clusters::LevelControl::Id) {}

    CHIP_ERROR Invoke(uint8_t level, chip::app::DataModel::Nullable<uint16_t> transitionTime, uint8_t optionMask,
                      uint8_t optionOverride, std::function<void(CHIP_ERROR)> responseCallback);
};
