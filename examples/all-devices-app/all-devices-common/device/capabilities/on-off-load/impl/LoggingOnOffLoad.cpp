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

#include <device/capabilities/on-off-load/impl/LoggingOnOffLoad.h>
#include <devices/Types.h>
#include <lib/support/StringBuilder.h>
#include <lib/support/logging/CHIPLogging.h>

using namespace chip::app::Clusters;
using chip::Protocols::InteractionModel::Status;

namespace chip {
namespace app {

LoggingOnOffLoad::LoggingOnOffLoad(Span<const DataModel::DeviceTypeEntry> deviceTypes, const Context & context,
                                   Clusters::OnOffDelegate * customOnOff, Clusters::OnOffEffectDelegate * customEffect,
                                   Clusters::IdentifyDelegate * customIdentify) :
    OnOffLoad(deviceTypes, customOnOff ? *customOnOff : static_cast<Clusters::OnOffDelegate &>(*this),
              customEffect ? *customEffect : static_cast<Clusters::OnOffEffectDelegate &>(*this), customIdentify ? *customIdentify : context.identifyDelegate, context)
{}

// OnOffDelegate

void LoggingOnOffLoad::OnOffStartup(bool on)
{
    ChipLogProgress(DeviceLayer, "LoggingOnOffLoad::OnOffStartup() -> %s", on ? "ON" : "OFF");
}

void LoggingOnOffLoad::OnOnOffChanged(bool on)
{
    ChipLogProgress(DeviceLayer, "LoggingOnOffLoad::OnOnOffChanged() -> %s", on ? "ON" : "OFF");
}

// OnOffEffectDelegate

DataModel::ActionReturnStatus LoggingOnOffLoad::TriggerDelayedAllOff(OnOff::DelayedAllOffEffectVariantEnum e)
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
        ChipLogProgress(DeviceLayer, "DelayedAllOff: UNKNOWN/INVALID");
        break;
    }
    return Status::Success;
}

DataModel::ActionReturnStatus LoggingOnOffLoad::TriggerDyingLight(OnOff::DyingLightEffectVariantEnum e)
{
    switch (e)
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

} // namespace app
} // namespace chip
