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

#include <ESP32DimmableLightDevice.h>
#include <lib/support/logging/CHIPLogging.h>

using namespace chip::app::Clusters;
using chip::Protocols::InteractionModel::Status;

namespace chip {
namespace app {

ESP32DimmableLightDevice::ESP32DimmableLightDevice(const Context & context) :
    DimmableLightDevice(*this, *this, *this, *this, context)
{
    mLED.Init();
}

// OnOffDelegate

void ESP32DimmableLightDevice::OnOffStartup(bool on)
{
    ChipLogProgress(DeviceLayer, "ESP32DimmableLightDevice: OnOffStartup -> %s", on ? "ON" : "OFF");
    mLED.Set(on);
}

void ESP32DimmableLightDevice::OnOnOffChanged(bool on)
{
    ChipLogProgress(DeviceLayer, "ESP32DimmableLightDevice: OnOnOffChanged -> %s", on ? "ON" : "OFF");
    mLED.Set(on);
}

// LevelControlDelegate

void ESP32DimmableLightDevice::OnLevelChanged(uint8_t level)
{
    ChipLogProgress(DeviceLayer, "ESP32DimmableLightDevice: OnLevelChanged -> %u", level);
    mLED.SetBrightness(level);
}

void ESP32DimmableLightDevice::OnOptionsChanged(BitMask<LevelControl::OptionsBitmap> options)
{
    ChipLogProgress(DeviceLayer, "ESP32DimmableLightDevice: OnOptionsChanged -> 0x%02X", options.Raw());
}

void ESP32DimmableLightDevice::OnOnLevelChanged(DataModel::Nullable<uint8_t> onLevel)
{
    ChipLogProgress(DeviceLayer, "ESP32DimmableLightDevice: OnOnLevelChanged -> %s", onLevel.IsNull() ? "NULL" : "set");
}

void ESP32DimmableLightDevice::OnDefaultMoveRateChanged(DataModel::Nullable<uint8_t> defaultMoveRate)
{
    ChipLogProgress(DeviceLayer, "ESP32DimmableLightDevice: OnDefaultMoveRateChanged -> %s",
                    defaultMoveRate.IsNull() ? "NULL" : "set");
}

// OnOffEffectDelegate

DataModel::ActionReturnStatus ESP32DimmableLightDevice::TriggerDelayedAllOff(OnOff::DelayedAllOffEffectVariantEnum e)
{
    ChipLogProgress(DeviceLayer, "ESP32DimmableLightDevice: TriggerDelayedAllOff");
    mLED.Set(false);
    return Status::Success;
}

DataModel::ActionReturnStatus ESP32DimmableLightDevice::TriggerDyingLight(OnOff::DyingLightEffectVariantEnum e)
{
    ChipLogProgress(DeviceLayer, "ESP32DimmableLightDevice: TriggerDyingLight");
    mLED.SetBrightness(1);
    return Status::Success;
}

// IdentifyDelegate

void ESP32DimmableLightDevice::OnIdentifyStart(IdentifyCluster & cluster)
{
    ChipLogProgress(DeviceLayer, "ESP32DimmableLightDevice: Identify START");
    mLED.Blink(500);
    StartBlinkTimer();
}

void ESP32DimmableLightDevice::OnIdentifyStop(IdentifyCluster & cluster)
{
    ChipLogProgress(DeviceLayer, "ESP32DimmableLightDevice: Identify STOP");
    StopBlinkTimer();
    mLED.Set(mLED.IsTurnedOn());
}

void ESP32DimmableLightDevice::OnTriggerEffect(IdentifyCluster & cluster)
{
    ChipLogProgress(DeviceLayer, "ESP32DimmableLightDevice: TriggerEffect");
    mLED.Blink(250);
    StartBlinkTimer();
}

void ESP32DimmableLightDevice::BlinkTimerCallback(TimerHandle_t handle)
{
    auto * self = static_cast<ESP32DimmableLightDevice *>(pvTimerGetTimerID(handle));
    self->mLED.Animate();
}

void ESP32DimmableLightDevice::StartBlinkTimer()
{
    if (mBlinkTimer == nullptr)
    {
        mBlinkTimer = xTimerCreate("LedBlink", pdMS_TO_TICKS(50), pdTRUE, this, BlinkTimerCallback);
        if (mBlinkTimer == nullptr)
        {
            ChipLogError(DeviceLayer, "ESP32DimmableLightDevice: Failed to create blink timer");
            return;
        }
    }
    xTimerStart(mBlinkTimer, 0);
}

void ESP32DimmableLightDevice::StopBlinkTimer()
{
    if (mBlinkTimer != nullptr)
    {
        xTimerStop(mBlinkTimer, 0);
    }
}

bool ESP32DimmableLightDevice::IsTriggerEffectEnabled() const
{
    return true;
}

} // namespace app
} // namespace chip
