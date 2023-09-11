/*
 *
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
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
    uint8_t mColorMode               = EMBER_ZCL_COLOR_MODE_CURRENT_HUE_AND_CURRENT_SATURATION;
    uint8_t mColorHue                = 0;
    uint8_t mColorSaturation         = 0;
    uint16_t mColorX                 = 0;
    uint16_t mColorY                 = 0;
    uint16_t mColorTemperatureMireds = 0;
};

} // namespace Windows
} // namespace Ui
} // namespace example
