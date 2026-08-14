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

#include <device/capabilities/color-light/LoggingLightDriver.h>
#include <lib/support/logging/CHIPLogging.h>

using namespace chip::app::Clusters;
using chip::Protocols::InteractionModel::Status;

namespace chip {
namespace app {

// OnOffDelegate

void LoggingLightDriver::OnOffStartup(bool on)
{
    ChipLogProgress(DeviceLayer, "LoggingLightDriver::OnOffStartup() -> %s", on ? "ON" : "OFF");
}

void LoggingLightDriver::OnOnOffChanged(bool on)
{
    ChipLogProgress(DeviceLayer, "LoggingLightDriver::OnOnOffChanged() -> %s", on ? "ON" : "OFF");
}

// LevelControlDelegate

void LoggingLightDriver::OnLevelChanged(uint8_t level)
{
    ChipLogProgress(DeviceLayer, "LoggingLightDriver::OnLevelChanged() -> %u", level);
}

void LoggingLightDriver::OnOptionsChanged(BitMask<LevelControl::OptionsBitmap> options)
{
    ChipLogProgress(DeviceLayer, "LoggingLightDriver::OnOptionsChanged() -> 0x%02X", options.Raw());
}

void LoggingLightDriver::OnOnLevelChanged(DataModel::Nullable<uint8_t> onLevel)
{
    if (onLevel.IsNull())
    {
        ChipLogProgress(DeviceLayer, "LoggingLightDriver::OnOnLevelChanged() -> NULL");
        return;
    }
    ChipLogProgress(DeviceLayer, "LoggingLightDriver::OnOnLevelChanged() -> %u", onLevel.Value());
}

void LoggingLightDriver::OnDefaultMoveRateChanged(DataModel::Nullable<uint8_t> defaultMoveRate)
{
    if (defaultMoveRate.IsNull())
    {
        ChipLogProgress(DeviceLayer, "LoggingLightDriver::OnDefaultMoveRateChanged() -> NULL");
        return;
    }
    ChipLogProgress(DeviceLayer, "LoggingLightDriver::OnDefaultMoveRateChanged() -> %u", defaultMoveRate.Value());
}

// OnOffEffectDelegate

DataModel::ActionReturnStatus LoggingLightDriver::TriggerDelayedAllOff(OnOff::DelayedAllOffEffectVariantEnum effect)
{
    switch (effect)
    {
    case OnOff::DelayedAllOffEffectVariantEnum::kDelayedOffFastFade:
        ChipLogProgress(DeviceLayer, "DelayedAllOff: FastFade");
        break;
    case OnOff::DelayedAllOffEffectVariantEnum::kNoFade:
        ChipLogProgress(DeviceLayer, "DelayedAllOff: NoFade");
        break;
    case OnOff::DelayedAllOffEffectVariantEnum::kDelayedOffSlowFade:
        ChipLogProgress(DeviceLayer, "DelayedAllOff: SlowFade");
        break;
    default:
        ChipLogProgress(DeviceLayer, "DelayedAllOff: UNKNOWN/INVALID");
        break;
    }
    return Status::Success;
}

DataModel::ActionReturnStatus LoggingLightDriver::TriggerDyingLight(OnOff::DyingLightEffectVariantEnum effect)
{
    switch (effect)
    {
    case OnOff::DyingLightEffectVariantEnum::kDyingLightFadeOff:
        ChipLogProgress(DeviceLayer, "DyingLight: FadeOff");
        break;
    default:
        ChipLogProgress(DeviceLayer, "DyingLight: UNKNOWN/INVALID");
        break;
    }
    return Status::Success;
}

// ColorControlDelegate

void LoggingLightDriver::OnColorCTChanged(uint16_t mireds)
{
    ChipLogProgress(DeviceLayer, "LoggingLightDriver::OnColorCTChanged() -> %u mireds", mireds);
}

} // namespace app
} // namespace chip
