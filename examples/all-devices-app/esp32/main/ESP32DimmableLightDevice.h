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

#pragma once

#include <LEDWidget.h>
#include <app/clusters/identify-server/IdentifyCluster.h>
#include <app/clusters/level-control/LevelControlDelegate.h>
#include <app/clusters/on-off-server/OnOffDelegate.h>
#include <app/clusters/on-off-server/OnOffEffectDelegate.h>
#include <devices/dimmable-light/DimmableLightDevice.h>
#include <freertos/FreeRTOS.h>
#include <freertos/timers.h>

namespace chip {
namespace app {

/**
 * @brief ESP32 implementation of a Dimmable Light Device.
 *
 * Drives a real LED (GPIO or RMT/WS2812) via LEDWidget instead of just logging.
 */
class ESP32DimmableLightDevice : public DimmableLightDevice,
                                 public Clusters::OnOffDelegate,
                                 public Clusters::LevelControlDelegate,
                                 public Clusters::OnOffEffectDelegate,
                                 public Clusters::IdentifyDelegate
{
public:
    ESP32DimmableLightDevice(const Context & context);
    ~ESP32DimmableLightDevice() override = default;

    // OnOffDelegate
    void OnOffStartup(bool on) override;
    void OnOnOffChanged(bool on) override;

    // LevelControlDelegate
    void OnLevelChanged(uint8_t level) override;
    void OnOptionsChanged(BitMask<Clusters::LevelControl::OptionsBitmap> options) override;
    void OnOnLevelChanged(DataModel::Nullable<uint8_t> onLevel) override;
    void OnDefaultMoveRateChanged(DataModel::Nullable<uint8_t> defaultMoveRate) override;

    // OnOffEffectDelegate
    DataModel::ActionReturnStatus TriggerDelayedAllOff(Clusters::OnOff::DelayedAllOffEffectVariantEnum e) override;
    DataModel::ActionReturnStatus TriggerDyingLight(Clusters::OnOff::DyingLightEffectVariantEnum e) override;

    // IdentifyDelegate
    void OnIdentifyStart(Clusters::IdentifyCluster & cluster) override;
    void OnIdentifyStop(Clusters::IdentifyCluster & cluster) override;
    void OnTriggerEffect(Clusters::IdentifyCluster & cluster) override;
    bool IsTriggerEffectEnabled() const override;

private:
    void StartBlinkTimer();
    void StopBlinkTimer();
    static void BlinkTimerCallback(TimerHandle_t handle);

    LEDWidget mLED;
    TimerHandle_t mBlinkTimer = nullptr;
};

} // namespace app
} // namespace chip
