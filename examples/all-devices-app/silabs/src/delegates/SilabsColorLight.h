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

#pragma once

#include "AppConfig.h"

#include <app/clusters/level-control/LevelControlDelegate.h>
#include <app/clusters/on-off-server/OnOffDelegate.h>
#include <app/clusters/on-off-server/OnOffEffectDelegate.h>
#include <device/capabilities/color-light/impl/ColorConverter.h>
#include <device/types/extended-color-light/ExtendedColorLight.h>

#if SL_MATTER_DISPLAY_ENABLED
#include "lcd.h"
#endif

#if (defined(SL_MATTER_RGB_LED_ENABLED) && SL_MATTER_RGB_LED_ENABLED == 1)
#include "RGBLEDWidget.h"
#endif

namespace chip {
namespace app {

/**
 * Silabs implementation of the extended-color-light device type.
 *
 * Tracks OnOff / CurrentLevel / color (XY, HS, EnhancedHue or CT) and derives an sRGB value from them.
 * That value drives the kit's RGB LED when the board has one (SL_MATTER_RGB_LED_ENABLED) and, on
 * kits with an LCD, is shown on a device page alongside the lightbulb icon and level.
 */
class SilabsColorLight : public ExtendedColorLight,
                         public Clusters::OnOffDelegate,
                         public Clusters::OnOffEffectDelegate,
                         public Clusters::LevelControlDelegate,
                         public ColorConverter
{
public:
#if SL_MATTER_DISPLAY_ENABLED
    SilabsColorLight(const Context & context, Clusters::IdentifyDelegate & identify, SilabsLCD & lcd);
#else
    SilabsColorLight(const Context & context, Clusters::IdentifyDelegate & identify);
#endif
    ~SilabsColorLight() override = default;

    // DeviceInterface: allocate endpoint, register clusters, then register LCD page.
    CHIP_ERROR Register(EndpointIdAllocator & allocator, CodeDrivenDataModelProvider & provider,
                        EndpointComposition composition = {}) override;

    // OnOffDelegate
    void OnOffStartup(bool on) override;
    void OnOnOffChanged(bool on) override;

    // OnOffEffectDelegate
    DataModel::ActionReturnStatus TriggerDelayedAllOff(Clusters::OnOff::DelayedAllOffEffectVariantEnum e) override;
    DataModel::ActionReturnStatus TriggerDyingLight(Clusters::OnOff::DyingLightEffectVariantEnum e) override;

    // LevelControlDelegate
    void OnLevelChanged(uint8_t level) override;
    void OnOptionsChanged(BitMask<Clusters::LevelControl::OptionsBitmap> options) override;
    void OnOnLevelChanged(DataModel::Nullable<uint8_t> onLevel) override;
    void OnDefaultMoveRateChanged(DataModel::Nullable<uint8_t> defaultMoveRate) override;

    // ColorControlDelegate (conversions come from ColorConverter)
    void OnColorXYChanged(uint16_t x, uint16_t y, bool transitionActive) override;
    void OnColorHSChanged(uint8_t hue, uint8_t sat, bool transitionActive) override;
    void OnEnhancedHueChanged(uint16_t enhancedHue, bool transitionActive) override;
    void OnColorCTChanged(uint16_t mireds, bool transitionActive) override;

protected:
    using ColorLight::Register;

private:
    struct Rgb
    {
        uint8_t r, g, b;
    };

    // Recomputes mRgb from the tracked state and pushes it to the LED and LCD.
    void UpdateOutput();

#if SL_MATTER_DISPLAY_ENABLED
    static void DrawDevicePage(GLIB_Context_t * context, EndpointId endpointId, void * userContext);
    // Action-button callback: toggles OnOff for this endpoint via the OnOff cluster.
    static void OnDevicePageButtonPressed(EndpointId endpointId, void * userContext);

    SilabsLCD * mLCD = nullptr;
#endif
#if (defined(SL_MATTER_RGB_LED_ENABLED) && SL_MATTER_RGB_LED_ENABLED == 1)
    RGBLEDWidget mLed;
#endif
    bool mOn       = false;
    uint8_t mLevel = 0;
    uint8_t mHue   = 0;
    uint8_t mSat   = 0;
    // Chromaticity of the current color; every color mode is folded into this. Defaults to the
    // cluster's CurrentX/CurrentY defaults.
    uint16_t mX = 0x616B;
    uint16_t mY = 0x607D;
    Rgb mRgb    = { 0, 0, 0 };
};

} // namespace app
} // namespace chip
