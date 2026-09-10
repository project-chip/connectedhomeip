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

#include <device/capabilities/color-light/ColorLight.h>
#include <device/capabilities/color-light/impl/ColorConverter.h>

namespace chip {
namespace app {

/**
 * A ColorLight whose output side only logs, so the device can be exercised without anything behind
 * it. It supplies itself as every delegate ColorLight needs.
 *
 * A real product does not use this: it implements the delegate interfaces against its PWM/LED
 * channels and hands them to ColorLight, which is why ColorLight itself knows nothing about this
 * class.
 */
class LoggingLightDriver : public ColorLight,
                           public Clusters::OnOffDelegate,
                           public Clusters::LevelControlDelegate,
                           public Clusters::OnOffEffectDelegate,
                           public ColorConverter,
                           public Clusters::IdentifyDelegate
{
public:
    LoggingLightDriver(Span<const DataModel::DeviceTypeEntry> deviceTypes, const Context & context,
                       const Conformance & conformance);
    ~LoggingLightDriver() override = default;

protected:
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

    // ColorControlDelegate. Both output channels the device types built on this driver advertise are
    // logged; HueSaturation and EnhancedHue are left at their no-op defaults because neither device
    // type advertises those features, so the cluster never feeds them.
    //
    // The XY <-> mireds conversions the Extended Color Light does need come from
    // ColorConverter, so a mode switch or a read of the inactive mode's attribute reports a
    // value derived from the active color rather than a constant.
    void OnColorXYChanged(uint16_t x, uint16_t y, bool transitionActive) override;
    void OnColorCTChanged(uint16_t mireds, bool transitionActive) override;
    void OnColorHSChanged(uint8_t hue, uint8_t sat, bool transitionActive) override;
    void OnEnhancedHueChanged(uint16_t enhancedHue, bool transitionActive) override;
    void OnColorLoopStarted(uint16_t startEnhancedHue, uint16_t loopTimeSec, bool directionUp) override;

    // IdentifyDelegate. TriggerEffect is mandatory for both device types built on this driver
    // (Color Temperature Light 0x010C and Extended Color Light 0x010D), so it is always enabled.
    void OnIdentifyStart(Clusters::IdentifyCluster & cluster) override;
    void OnIdentifyStop(Clusters::IdentifyCluster & cluster) override;
    void OnTriggerEffect(Clusters::IdentifyCluster & cluster) override;
    bool IsTriggerEffectEnabled() const override;
};

} // namespace app
} // namespace chip
