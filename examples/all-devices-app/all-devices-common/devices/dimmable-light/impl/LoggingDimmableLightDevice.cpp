/*
 *
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

#include <devices/dimmable-light/impl/LoggingDimmableLightDevice.h>
#include <lib/support/StringBuilder.h>
#include <lib/support/logging/CHIPLogging.h>

using namespace chip::app::Clusters;
using chip::Protocols::InteractionModel::Status;

namespace chip {
namespace app {

LoggingDimmableLightDevice::LoggingDimmableLightDevice(const Context & context) :
    DimmableLightDevice(*this, *this, *this, *this, context)
{}

// OnOffDelegate

void LoggingDimmableLightDevice::OnOffStartup(bool on)
{
    ChipLogProgress(DeviceLayer, "LoggingDimmableLightDevice::OnOffStartup() -> %s", on ? "ON" : "OFF");
}

void LoggingDimmableLightDevice::OnOnOffChanged(bool on)
{
    ChipLogProgress(DeviceLayer, "LoggingDimmableLightDevice::OnOnOffChanged() -> %s", on ? "ON" : "OFF");
}

// LevelControlDelegate

void LoggingDimmableLightDevice::OnLevelChanged(uint8_t level)
{
    ChipLogProgress(DeviceLayer, "LoggingDimmableLightDevice::OnLevelChanged() -> %u", level);
}

void LoggingDimmableLightDevice::OnOptionsChanged(BitMask<LevelControl::OptionsBitmap> options)
{
    ChipLogProgress(DeviceLayer, "LoggingDimmableLightDevice::OnOptionsChanged() -> 0x%02X", options.Raw());
}

void LoggingDimmableLightDevice::OnOnLevelChanged(DataModel::Nullable<uint8_t> onLevel)
{
    if (!onLevel.IsNull())
    {
        ChipLogProgress(DeviceLayer, "LoggingDimmableLightDevice::OnOnLevelChanged() -> %u", onLevel.Value());
    }
    else
    {
        ChipLogProgress(DeviceLayer, "LoggingDimmableLightDevice::OnOnLevelChanged() -> NULL");
    }
}

void LoggingDimmableLightDevice::OnDefaultMoveRateChanged(DataModel::Nullable<uint8_t> defaultMoveRate)
{
    if (!defaultMoveRate.IsNull())
    {
        ChipLogProgress(DeviceLayer, "LoggingDimmableLightDevice::OnDefaultMoveRateChanged() -> %u", defaultMoveRate.Value());
    }
    else
    {
        ChipLogProgress(DeviceLayer, "LoggingDimmableLightDevice::OnDefaultMoveRateChanged() -> NULL");
    }
}

// OnOffEffectDelegate

DataModel::ActionReturnStatus LoggingDimmableLightDevice::TriggerDelayedAllOff(OnOff::DelayedAllOffEffectVariantEnum e)
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

DataModel::ActionReturnStatus LoggingDimmableLightDevice::TriggerDyingLight(OnOff::DyingLightEffectVariantEnum e)
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

// IdentifyDelegate

void LoggingDimmableLightDevice::OnIdentifyStart(IdentifyCluster & cluster)
{
    ChipLogProgress(DeviceLayer, "LoggingDimmableLightDevice: Identify START");
}

void LoggingDimmableLightDevice::OnIdentifyStop(IdentifyCluster & cluster)
{
    ChipLogProgress(DeviceLayer, "LoggingDimmableLightDevice: Identify STOP");
}

void LoggingDimmableLightDevice::OnTriggerEffect(IdentifyCluster & cluster)
{
    StringBuilder<64> msg;

    switch (cluster.GetEffectIdentifier())
    {
    case Identify::EffectIdentifierEnum::kBlink:
        msg.Add("BlinkEffect");
        break;
    case Identify::EffectIdentifierEnum::kBreathe:
        msg.Add("BreatheEffect");
        break;
    case Identify::EffectIdentifierEnum::kOkay:
        msg.Add("OkayEffect");
        break;
    case Identify::EffectIdentifierEnum::kChannelChange:
        msg.Add("ChannelChangeEffect");
        break;
    case Identify::EffectIdentifierEnum::kFinishEffect:
        msg.Add("FinishEffect");
        break;
    case Identify::EffectIdentifierEnum::kStopEffect:
        msg.Add("StopEffect");
        break;
    default:
        msg.AddFormat("UnknownEffect(%d)", static_cast<int>(cluster.GetEffectIdentifier()));
        break;
    }
    msg.Add(" / ");
    switch (cluster.GetEffectVariant())
    {
    case Identify::EffectVariantEnum::kDefault:
        msg.Add("DefaultVariant");
        break;
    default:
        msg.AddFormat("UnknownVariant(%d)", static_cast<int>(cluster.GetEffectVariant()));
        break;
    }
    ChipLogProgress(DeviceLayer, "LoggingDimmableLightDevice: TriggerEffect: %s", msg.c_str());
}

bool LoggingDimmableLightDevice::IsTriggerEffectEnabled() const
{
    return true;
}

} // namespace app
} // namespace chip
