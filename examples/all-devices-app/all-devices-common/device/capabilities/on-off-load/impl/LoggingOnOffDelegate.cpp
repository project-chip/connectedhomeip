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

#include <device/capabilities/on-off-load/impl/LoggingOnOffDelegate.h>
#include <lib/support/logging/CHIPLogging.h>

using namespace chip::app::Clusters;
using chip::Protocols::InteractionModel::Status;

namespace chip {
namespace app {

// OnOffDelegate

void LoggingOnOffDelegate::OnOffStartup(bool on)
{
    ChipLogProgress(DeviceLayer, "LoggingOnOffDelegate::OnOffStartup() -> %s", on ? "ON" : "OFF");
}

void LoggingOnOffDelegate::OnOnOffChanged(bool on)
{
    ChipLogProgress(DeviceLayer, "LoggingOnOffDelegate::OnOnOffChanged() -> %s", on ? "ON" : "OFF");
}

// OnOffEffectDelegate

DataModel::ActionReturnStatus LoggingOnOffDelegate::TriggerDelayedAllOff(OnOff::DelayedAllOffEffectVariantEnum e)
{
    switch (e)
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
        ChipLogError(DeviceLayer, "DelayedAllOff: UNKNOWN/INVALID");
        return Status::ConstraintError;
    }
    return Status::Success;
}

DataModel::ActionReturnStatus LoggingOnOffDelegate::TriggerDyingLight(OnOff::DyingLightEffectVariantEnum e)
{
    switch (e)
    {
    case OnOff::DyingLightEffectVariantEnum::kDyingLightFadeOff:
        ChipLogProgress(DeviceLayer, "DyingLight: FadeOff");
        break;
    default:
        ChipLogError(DeviceLayer, "DyingLight: UNKNOWN/INVALID");
        return Status::ConstraintError;
    }
    return Status::Success;
}

} // namespace app
} // namespace chip
