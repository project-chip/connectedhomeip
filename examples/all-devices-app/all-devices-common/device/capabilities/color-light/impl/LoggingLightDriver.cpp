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

namespace chip {
namespace app {

ColorLight::Delegates LoggingLightDriver::GetDelegates()
{
    return ColorLight::Delegates{
        .onOff        = *this,
        .levelControl = *this,
        .effect       = *this,
        .color        = *this,
        .identify     = *this,
    };
}

// ColorControlDelegate
//
// This driver only logs, so it has no hardware ramp to abort or resynchronize: both intermediate
// frames and settled endpoints are reported the same way, with the flag recorded for visibility.

void LoggingLightDriver::OnColorXYChanged(uint16_t x, uint16_t y, bool transitionActive)
{
    ChipLogProgress(DeviceLayer, "LoggingLightDriver::OnColorXYChanged() -> x=%u y=%u (transitionActive=%s)", x, y,
                    transitionActive ? "true" : "false");
}

void LoggingLightDriver::OnColorCTChanged(uint16_t mireds, bool transitionActive)
{
    ChipLogProgress(DeviceLayer, "LoggingLightDriver::OnColorCTChanged() -> %u mireds (transitionActive=%s)", mireds,
                    transitionActive ? "true" : "false");
}

void LoggingLightDriver::OnColorHSChanged(uint8_t hue, uint8_t sat, bool transitionActive)
{
    ChipLogProgress(DeviceLayer, "LoggingLightDriver::OnColorHSChanged() -> Hue=%u Saturation=%u (transitionActive=%s)", hue, sat,
                    transitionActive ? "true" : "false");
}

void LoggingLightDriver::OnEnhancedHueChanged(uint16_t enhancedHue, bool transitionActive)
{
    ChipLogProgress(DeviceLayer, "LoggingLightDriver::OnEnhancedHueChanged() -> EnhancedHue=%u (transitionActive=%s)", enhancedHue,
                    transitionActive ? "true" : "false");
}

void LoggingLightDriver::OnColorLoopStarted(uint16_t startEnhancedHue, uint16_t loopTimeSec, bool directionUp)
{
    ChipLogProgress(DeviceLayer, "LoggingLightDriver::OnColorLoopStarted() -> startEnhancedHue=%u loopTimeSec=%u (direction=%s)",
                    startEnhancedHue, loopTimeSec, directionUp ? "Up" : "Down");
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
