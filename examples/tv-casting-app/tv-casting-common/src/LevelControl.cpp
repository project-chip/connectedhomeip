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

#include "LevelControl.h"

using namespace chip;
using namespace chip::app::Clusters;

CHIP_ERROR StepCommand::Invoke(LevelControl::StepMode stepMode, uint8_t stepSize, app::DataModel::Nullable<uint16_t> transitionTime,
                               uint8_t optionMask, uint8_t optionOverride, std::function<void(CHIP_ERROR)> responseCallback)
{
    LevelControl::Commands::Step::Type request;
    request.stepMode        = stepMode;
    request.stepSize        = stepSize;
    request.transitionTime  = transitionTime;
    request.optionsMask     = optionMask;
    request.optionsOverride = optionOverride;
    return MediaCommandBase::Invoke(request, responseCallback);
}

CHIP_ERROR MoveToLevelCommand::Invoke(uint8_t level, app::DataModel::Nullable<uint16_t> transitionTime, uint8_t optionMask,
                                      uint8_t optionOverride, std::function<void(CHIP_ERROR)> responseCallback)
{
    LevelControl::Commands::MoveToLevel::Type request;
    request.level           = level;
    request.transitionTime  = transitionTime;
    request.optionsMask     = optionMask;
    request.optionsOverride = optionOverride;
    return MediaCommandBase::Invoke(request, responseCallback);
}
