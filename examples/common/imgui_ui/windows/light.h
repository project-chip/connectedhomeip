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

#include <app/data-model/Nullable.h>
#include <lib/core/DataModelTypes.h>
#include <lib/core/Optional.h>

#include <app-common/zap-generated/cluster-enums.h>

namespace example {
namespace Ui {
namespace Windows {

/**
 * Assumes light on a given endpoint with FULL support (on/off, level and color)
 */
class Light : public Window
{
public:
    Light(chip::EndpointId endpointId) : mEndpointId(endpointId) {}

    void UpdateState() override;
    void Render() override;

private:
    const chip::EndpointId mEndpointId;

    static constexpr uint8_t kColorModeCurrentHueAndCurrentSaturation         = 0;
    static constexpr uint8_t kColorModeCurrentXAndCurrentY                    = 1;
    static constexpr uint8_t kColorModeColorTemperature                       = 2;
    static constexpr uint8_t kColorModeEnhancedCurrentHueAndCurrentSaturation = 3;

    // OnOff
    bool mLightIsOn = false;
    chip::Optional<bool> mTargetLightIsOn; // allow UI control of this

    // Level
    uint8_t mMinLevel = 0;
    uint8_t mMaxLevel = 0;
    chip::app::DataModel::Nullable<uint8_t> mCurrentLevel;
    chip::Optional<uint8_t> mTargetLevel; // allow UI control of this

    uint16_t mLevelRemainingTime10sOfSec = 0;

    // Color control
    uint8_t mColorMode               = kColorModeCurrentHueAndCurrentSaturation;
    uint8_t mColorHue                = 0;
    uint8_t mColorSaturation         = 0;
    uint16_t mColorX                 = 0;
    uint16_t mColorY                 = 0;
    uint16_t mColorTemperatureMireds = 0;
};

} // namespace Windows
} // namespace Ui
} // namespace example
