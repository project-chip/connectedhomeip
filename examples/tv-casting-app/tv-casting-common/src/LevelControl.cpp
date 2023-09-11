/*
 *
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "LevelControl.h"

using namespace chip;
using namespace chip::app::Clusters;

CHIP_ERROR StepCommand::Invoke(LevelControl::StepMode stepMode, uint8_t stepSize, app::DataModel::Nullable<uint16_t> transitionTime,
                               BitMask<LevelControl::LevelControlOptions> optionMask,
                               BitMask<LevelControl::LevelControlOptions> optionOverride,
                               std::function<void(CHIP_ERROR)> responseCallback)
{
    LevelControl::Commands::Step::Type request;
    request.stepMode        = stepMode;
    request.stepSize        = stepSize;
    request.transitionTime  = transitionTime;
    request.optionsMask     = optionMask;
    request.optionsOverride = optionOverride;
    return MediaCommandBase::Invoke(request, responseCallback);
}

CHIP_ERROR MoveToLevelCommand::Invoke(uint8_t level, app::DataModel::Nullable<uint16_t> transitionTime,
                                      BitMask<LevelControl::LevelControlOptions> optionMask,
                                      BitMask<LevelControl::LevelControlOptions> optionOverride,
                                      std::function<void(CHIP_ERROR)> responseCallback)
{
    LevelControl::Commands::MoveToLevel::Type request;
    request.level           = level;
    request.transitionTime  = transitionTime;
    request.optionsMask     = optionMask;
    request.optionsOverride = optionOverride;
    return MediaCommandBase::Invoke(request, responseCallback);
}
