/*
 *    Copyright (c) 2026 Project CHIP Authors
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

#include <app/clusters/color-control-server/ColorControlDelegate.h>
#include <app/clusters/level-control/LevelControlDelegate.h>
#include <app/clusters/on-off-server/OnOffDelegate.h>
#include <app/clusters/on-off-server/OnOffEffectDelegate.h>

namespace chip {
namespace app {

/**
 * The output side of a color temperature light: every state change the On/Off, Level Control and
 * Color Control clusters push towards the hardware arrives here.
 *
 * This implementation only logs, so the device can be exercised without anything behind it. A real
 * product swaps it for one that drives the PWM/LED channels.
 */
class LoggingLightDriver : public Clusters::OnOffDelegate,
                           public Clusters::LevelControlDelegate,
                           public Clusters::OnOffEffectDelegate,
                           public Clusters::ColorControlDelegate
{
public:
    // OnOffDelegate
    void OnOffStartup(bool on) override;
    void OnOnOffChanged(bool on) override;

    // LevelControlDelegate
    void OnLevelChanged(uint8_t level) override;
    void OnOptionsChanged(BitMask<Clusters::LevelControl::OptionsBitmap> options) override;
    void OnOnLevelChanged(DataModel::Nullable<uint8_t> onLevel) override;
    void OnDefaultMoveRateChanged(DataModel::Nullable<uint8_t> defaultMoveRate) override;

    // OnOffEffectDelegate
    DataModel::ActionReturnStatus TriggerDelayedAllOff(Clusters::OnOff::DelayedAllOffEffectVariantEnum effect) override;
    DataModel::ActionReturnStatus TriggerDyingLight(Clusters::OnOff::DyingLightEffectVariantEnum effect) override;

    // ColorControlDelegate. Only the color temperature output is implemented: this device type
    // advertises the ColorTemperature feature alone, so the cluster never asks for a conversion
    // between the color representations.
    void OnColorCTChanged(uint16_t mireds) override;
};

} // namespace app
} // namespace chip
