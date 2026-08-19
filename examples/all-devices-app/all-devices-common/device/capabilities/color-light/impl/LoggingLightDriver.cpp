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

#include <device/capabilities/color-light/impl/LoggingLightDriver.h>
#include <lib/support/StringBuilder.h>
#include <lib/support/logging/CHIPLogging.h>

using namespace chip::app::Clusters;
using chip::Protocols::InteractionModel::Status;

namespace chip {
namespace app {

// Passing *this as every delegate is safe: ColorLight only stores the references, and nothing calls
// through them before Register() runs.
LoggingLightDriver::LoggingLightDriver(Span<const DataModel::DeviceTypeEntry> deviceTypes, const Context & context,
                                       const Conformance & conformance) :
    ColorLight(deviceTypes, context,
               Delegates{ .onOff = *this, .levelControl = *this, .effect = *this, .color = *this, .identify = *this }, conformance)
{}

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
//
// OnOffEffectDelegate::TriggerEffect checks the EffectIdentifier but casts the EffectVariant
// straight from the wire without validating it, so an unrecognized variant reaches these methods
// and only they can reject it. ConstraintError is the status the SDK uses for a command field whose
// value is outside the range its type allows.

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
        ChipLogError(DeviceLayer, "DelayedAllOff: unknown variant %u", to_underlying(effect));
        return Status::ConstraintError;
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
        ChipLogError(DeviceLayer, "DyingLight: unknown variant %u", to_underlying(effect));
        return Status::ConstraintError;
    }
    return Status::Success;
}

// ColorControlDelegate

void LoggingLightDriver::OnColorXYChanged(uint16_t x, uint16_t y)
{
    ChipLogProgress(DeviceLayer, "LoggingLightDriver::OnColorXYChanged() -> x=%u y=%u", x, y);
}

void LoggingLightDriver::OnColorCTChanged(uint16_t mireds)
{
    ChipLogProgress(DeviceLayer, "LoggingLightDriver::OnColorCTChanged() -> %u mireds", mireds);
}

// Neither of these converts. They leave the out-params as the cluster initialized them, which is
// the startup default of the attribute being asked for and not a value derived from the active
// color: a mode switch or an inactive-mode read therefore reports that default. §3.2.7 defines no
// fallback for this, so a product must replace both with its own calibration.

void LoggingLightDriver::ConvertXYToMireds(uint16_t x, uint16_t y, uint16_t & outMireds)
{
    ChipLogProgress(DeviceLayer, "LoggingLightDriver::ConvertXYToMireds(x=%u, y=%u): no conversion supplied, leaving %u mireds", x,
                    y, outMireds);
}

void LoggingLightDriver::ConvertMiredsToXY(uint16_t mireds, uint16_t & outX, uint16_t & outY)
{
    ChipLogProgress(DeviceLayer, "LoggingLightDriver::ConvertMiredsToXY(%u mireds): no conversion supplied, leaving x=%u y=%u",
                    mireds, outX, outY);
}

void LoggingLightDriver::OnIdentifyStart(Clusters::IdentifyCluster & cluster)
{
    ChipLogProgress(DeviceLayer, "LoggingLightDriver: Identify START");
}

void LoggingLightDriver::OnIdentifyStop(Clusters::IdentifyCluster & cluster)
{
    ChipLogProgress(DeviceLayer, "LoggingLightDriver: Identify STOP");
}

void LoggingLightDriver::OnTriggerEffect(Clusters::IdentifyCluster & cluster)
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
    ChipLogProgress(DeviceLayer, "LoggingLightDriver: TriggerEffect: %s", msg.c_str());
}

bool LoggingLightDriver::IsTriggerEffectEnabled() const
{
    return true;
}

} // namespace app
} // namespace chip
