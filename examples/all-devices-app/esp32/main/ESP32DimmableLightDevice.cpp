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

#include <ESP32DimmableLight.h>
#include <lib/support/logging/CHIPLogging.h>

using namespace chip::app::Clusters;
using chip::Protocols::InteractionModel::Status;

namespace chip {
namespace app {

ESP32DimmableLight::ESP32DimmableLight(const Context & context) :
    DimmableLight(context, { .onOff = *this, .levelControl = *this, .effect = *this, .identify = *this })
{
    mLED.Init();
}

// OnOffDelegate

void ESP32DimmableLight::OnOffStartup(bool on)
{
    ChipLogProgress(DeviceLayer, "ESP32DimmableLight: OnOffStartup -> %s", on ? "ON" : "OFF");
    mLED.Set(on);
}

void ESP32DimmableLight::OnOnOffChanged(bool on)
{
    ChipLogProgress(DeviceLayer, "ESP32DimmableLight: OnOnOffChanged -> %s", on ? "ON" : "OFF");
    mLED.Set(on);
}

// LevelControlDelegate

void ESP32DimmableLight::OnLevelChanged(uint8_t level)
{
    ChipLogProgress(DeviceLayer, "ESP32DimmableLight: OnLevelChanged -> %u", level);
    mLED.SetBrightness(level);
}

void ESP32DimmableLight::OnOptionsChanged(BitMask<LevelControl::OptionsBitmap> options)
{
    ChipLogProgress(DeviceLayer, "ESP32DimmableLight: OnOptionsChanged -> 0x%02X", options.Raw());
}

void ESP32DimmableLight::OnOnLevelChanged(DataModel::Nullable<uint8_t> onLevel)
{
    ChipLogProgress(DeviceLayer, "ESP32DimmableLight: OnOnLevelChanged -> %s", onLevel.IsNull() ? "NULL" : "set");
}

void ESP32DimmableLight::OnDefaultMoveRateChanged(DataModel::Nullable<uint8_t> defaultMoveRate)
{
    ChipLogProgress(DeviceLayer, "ESP32DimmableLight: OnDefaultMoveRateChanged -> %s", defaultMoveRate.IsNull() ? "NULL" : "set");
}

// OnOffEffectDelegate

DataModel::ActionReturnStatus ESP32DimmableLight::TriggerDelayedAllOff(OnOff::DelayedAllOffEffectVariantEnum e)
{
    ChipLogProgress(DeviceLayer, "ESP32DimmableLight: TriggerDelayedAllOff");
    mLED.Set(false);
    return Status::Success;
}

DataModel::ActionReturnStatus ESP32DimmableLight::TriggerDyingLight(OnOff::DyingLightEffectVariantEnum e)
{
    ChipLogProgress(DeviceLayer, "ESP32DimmableLight: TriggerDyingLight");
    mLED.SetBrightness(1);
    return Status::Success;
}

// IdentifyDelegate

void ESP32DimmableLight::OnIdentifyStart(Clusters::IdentifyCluster & cluster)
{
    ChipLogProgress(DeviceLayer, "ESP32DimmableLight: Identify START");
    mLED.Blink(500);
    StartBlinkTimer();
}

void ESP32DimmableLight::OnIdentifyStop(Clusters::IdentifyCluster & cluster)
{
    ChipLogProgress(DeviceLayer, "ESP32DimmableLight: Identify STOP");
    StopBlinkTimer();
    mLED.Set(mLED.IsTurnedOn());
}

void ESP32DimmableLight::OnTriggerEffect(Clusters::IdentifyCluster & cluster)
{
    ChipLogProgress(DeviceLayer, "ESP32DimmableLight: TriggerEffect");
    mLED.Blink(250);
    StartBlinkTimer();
}

void ESP32DimmableLight::BlinkTimerCallback(TimerHandle_t handle)
{
    auto * self = static_cast<ESP32DimmableLight *>(pvTimerGetTimerID(handle));
    self->mLED.Animate();
}

void ESP32DimmableLight::StartBlinkTimer()
{
    if (mBlinkTimer == nullptr)
    {
        mBlinkTimer = xTimerCreate("LedBlink", pdMS_TO_TICKS(50), pdTRUE, this, BlinkTimerCallback);
        if (mBlinkTimer == nullptr)
        {
            ChipLogError(DeviceLayer, "ESP32DimmableLight: Failed to create blink timer");
            return;
        }
    }
    xTimerStart(mBlinkTimer, 0);
}

void ESP32DimmableLight::StopBlinkTimer()
{
    if (mBlinkTimer != nullptr)
    {
        xTimerStop(mBlinkTimer, 0);
    }
}

bool ESP32DimmableLight::IsTriggerEffectEnabled() const
{
    return true;
}

} // namespace app
} // namespace chip
