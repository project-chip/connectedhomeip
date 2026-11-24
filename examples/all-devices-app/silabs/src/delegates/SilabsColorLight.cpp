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

#include "SilabsColorLight.h"

#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/PlatformManager.h>
#include <protocols/interaction_model/StatusCode.h>

#include <algorithm>

#if SL_MATTER_DISPLAY_ENABLED
#include <cstdio>

#include "LightBitMap.h"
#include "glib.h"
#endif

namespace chip {
namespace app {

using Status = Protocols::InteractionModel::Status;

namespace {
constexpr float kMaxLevel   = 254.0f;
constexpr float kMaxChannel = 255.0f;

#if (defined(SL_MATTER_RGB_LED_ENABLED) && SL_MATTER_RGB_LED_ENABLED == 1)
// Every LED index resolves to the board's single RGB PWM instance.
constexpr uint8_t kRgbLed = 0;
#endif
} // namespace

#if SL_MATTER_DISPLAY_ENABLED

namespace {
constexpr const char * kDeviceTypeName = "color-light";
constexpr int kLcdSize                 = 128;
constexpr uint8_t kHeaderLine          = 0;
constexpr uint8_t kEndpointLine        = 1;
constexpr uint8_t kStateLine           = 10;
constexpr uint8_t kColorLine           = 11;
} // namespace

SilabsColorLight::SilabsColorLight(const Context & context, Clusters::IdentifyDelegate & identify, SilabsLCD & lcd) :
    ExtendedColorLight(
        Delegates{
            .onOff        = *this,
            .levelControl = *this,
            .effect       = *this,
            .color        = *this,
            .identify     = identify,
        },
        context),
    mLCD(&lcd)
{}

#else // !SL_MATTER_DISPLAY_ENABLED

SilabsColorLight::SilabsColorLight(const Context & context, Clusters::IdentifyDelegate & identify) :
    ExtendedColorLight(
        Delegates{
            .onOff        = *this,
            .levelControl = *this,
            .effect       = *this,
            .color        = *this,
            .identify     = identify,
        },
        context)
{}

#endif // SL_MATTER_DISPLAY_ENABLED

CHIP_ERROR SilabsColorLight::Register(EndpointIdAllocator & allocator, CodeDrivenDataModelProvider & provider,
                                      EndpointComposition composition)
{
#if (defined(SL_MATTER_RGB_LED_ENABLED) && SL_MATTER_RGB_LED_ENABLED == 1)
    // Before clusters register, so the startup callbacks land on an initialized LED.
    mLed.Init(kRgbLed);
#endif

    ReturnErrorOnFailure(SingleEndpoint::Register(allocator, provider, composition));

    // Clusters start with the server, after this returns.
    LogErrorOnFailure(DeviceLayer::PlatformMgr().ScheduleWork(RestoreColor, reinterpret_cast<intptr_t>(this)));

#if SL_MATTER_DISPLAY_ENABLED
    if (mLCD != nullptr)
    {
        // Non-fatal: if the LCD is out of page slots the device still works,
        // it just won't appear in the cycle-through UI.
        CHIP_ERROR pageErr =
            mLCD->RegisterDevicePage(GetEndpointId(), kDeviceTypeName, &DrawDevicePage, this, &OnDevicePageButtonPressed);
        if (pageErr != CHIP_NO_ERROR)
        {
            ChipLogError(AppServer, "SilabsColorLight: failed to register LCD page: %" CHIP_ERROR_FORMAT, pageErr.Format());
        }
    }
#endif

    return CHIP_NO_ERROR;
}

void SilabsColorLight::OnOffStartup(bool on)
{
    ChipLogProgress(DeviceLayer, "SilabsColorLight: OnOffStartup -> %s", on ? "ON" : "OFF");
    mOn = on;
    UpdateOutput();
}

void SilabsColorLight::OnOnOffChanged(bool on)
{
    ChipLogProgress(DeviceLayer, "SilabsColorLight: OnOnOffChanged -> %s", on ? "ON" : "OFF");
    mOn = on;
    UpdateOutput();
}

DataModel::ActionReturnStatus SilabsColorLight::TriggerDelayedAllOff(Clusters::OnOff::DelayedAllOffEffectVariantEnum)
{
    ChipLogProgress(DeviceLayer, "SilabsColorLight: TriggerDelayedAllOff");
    return Status::Success;
}

DataModel::ActionReturnStatus SilabsColorLight::TriggerDyingLight(Clusters::OnOff::DyingLightEffectVariantEnum)
{
    ChipLogProgress(DeviceLayer, "SilabsColorLight: TriggerDyingLight");
    return Status::Success;
}

void SilabsColorLight::OnLevelChanged(uint8_t level)
{
    ChipLogProgress(DeviceLayer, "SilabsColorLight: OnLevelChanged -> %u", level);
    mLevel = level;
    UpdateOutput();
}

void SilabsColorLight::OnOptionsChanged(BitMask<Clusters::LevelControl::OptionsBitmap> options)
{
    ChipLogProgress(DeviceLayer, "SilabsColorLight: OnOptionsChanged -> 0x%02X", options.Raw());
}

void SilabsColorLight::OnOnLevelChanged(DataModel::Nullable<uint8_t> onLevel)
{
    ChipLogProgress(DeviceLayer, "SilabsColorLight: OnOnLevelChanged -> %s", onLevel.IsNull() ? "NULL" : "set");
}

void SilabsColorLight::OnDefaultMoveRateChanged(DataModel::Nullable<uint8_t> defaultMoveRate)
{
    ChipLogProgress(DeviceLayer, "SilabsColorLight: OnDefaultMoveRateChanged -> %s", defaultMoveRate.IsNull() ? "NULL" : "set");
}

void SilabsColorLight::OnColorHSChanged(uint8_t hue, uint8_t sat, bool /* transitionActive */)
{
    ConvertHueSatToXY(hue, sat, mX, mY);
    UpdateOutput();
}

void SilabsColorLight::OnEnhancedHueChanged(uint16_t enhancedHue, bool /* transitionActive */)
{
    ConvertHueSatToXY(Clusters::ColorControl::Hue8FromEnhancedHue(enhancedHue), GetColorControlCluster().Saturation(), mX, mY);
    UpdateOutput();
}
// No hardware fade engine: every frame, intermediate or final, is written straight to the LED.
void SilabsColorLight::OnColorXYChanged(uint16_t x, uint16_t y, bool /* transitionActive */)
{
    mX = x;
    mY = y;
    UpdateOutput();
}

void SilabsColorLight::OnColorCTChanged(uint16_t mireds, bool /* transitionActive */)
{
    ConvertMiredsToXY(mireds, mX, mY);
    UpdateOutput();
}

// ColorControlCluster::Startup() restores the persisted color without notifying the delegate.
void SilabsColorLight::RestoreColor(intptr_t context)
{
    auto * self    = reinterpret_cast<SilabsColorLight *>(context);
    auto & cluster = self->GetColorControlCluster();

    switch (cluster.GetEnhancedColorMode())
    {
    case Clusters::ColorControl::EnhancedColorModeEnum::kCurrentXAndCurrentY:
        self->mX = cluster.CurrentX();
        self->mY = cluster.CurrentY();
        break;
    case Clusters::ColorControl::EnhancedColorModeEnum::kColorTemperatureMireds:
        self->ConvertMiredsToXY(cluster.ColorTempMireds(), self->mX, self->mY);
        break;
    default:
        self->ConvertHueSatToXY(cluster.CurrentHue(), cluster.Saturation(), self->mX, self->mY);
        break;
    }
    self->UpdateOutput();
}

void SilabsColorLight::UpdateOutput()
{
    float r, g, b;
    ConvertXYToRgb(mX, mY, r, g, b);

    const float scale = mOn ? (static_cast<float>(mLevel) / kMaxLevel) * kMaxChannel : 0.0f;
    mRgb              = Rgb{
        static_cast<uint8_t>(std::clamp(r * scale, 0.0f, kMaxChannel)),
        static_cast<uint8_t>(std::clamp(g * scale, 0.0f, kMaxChannel)),
        static_cast<uint8_t>(std::clamp(b * scale, 0.0f, kMaxChannel)),
    };

#if (defined(SL_MATTER_RGB_LED_ENABLED) && SL_MATTER_RGB_LED_ENABLED == 1)
    mLed.SetColor(mRgb.r, mRgb.g, mRgb.b);
    mLed.Set(mOn);
#endif

#if SL_MATTER_DISPLAY_ENABLED
    if (mLCD != nullptr && GetEndpointId() != kInvalidEndpointId && mLCD->IsCurrentDevicePageFor(GetEndpointId()))
    {
        mLCD->SetScreen(SilabsLCD::DevicePageScreen);
    }
#endif
}

#if SL_MATTER_DISPLAY_ENABLED
void SilabsColorLight::DrawDevicePage(GLIB_Context_t * context, EndpointId endpointId, void * userContext)
{
    auto * self = static_cast<SilabsColorLight *>(userContext);

    GLIB_drawStringOnLine(context, kDeviceTypeName, kHeaderLine, GLIB_ALIGN_CENTER, 0, 0, true);

    char epLine[24];
    std::snprintf(epLine, sizeof(epLine), "Endpoint %u", static_cast<unsigned>(endpointId));
    GLIB_drawStringOnLine(context, epLine, kEndpointLine, GLIB_ALIGN_CENTER, 0, 0, true);

    const int x = (kLcdSize - kBitmapWidth) / 2;
    const int y = (kLcdSize - kBitmapHeight) / 2;

    if (self != nullptr)
    {
        char stateLine[24];
        std::snprintf(stateLine, sizeof(stateLine), "%s  Lvl %u", self->mOn ? "ON " : "OFF", static_cast<unsigned>(self->mLevel));
        GLIB_drawStringOnLine(context, stateLine, kStateLine, GLIB_ALIGN_CENTER, 0, 0, true);

        char colorLine[24];
        std::snprintf(colorLine, sizeof(colorLine), "R%u G%u B%u", static_cast<unsigned>(self->mRgb.r),
                      static_cast<unsigned>(self->mRgb.g), static_cast<unsigned>(self->mRgb.b));
        GLIB_drawStringOnLine(context, colorLine, kColorLine, GLIB_ALIGN_CENTER, 0, 0, true);

        GLIB_drawBitmap(context, x, y, kBitmapWidth, kBitmapHeight, self->mOn ? kLightbulbBitmapOn : kLightbulbBitmapOff);
    }
}

void SilabsColorLight::OnDevicePageButtonPressed(EndpointId /* endpointId */, void * userContext)
{
    auto * self = static_cast<SilabsColorLight *>(userContext);
    if (self == nullptr)
    {
        return;
    }
    chip::DeviceLayer::PlatformMgr().LockChipStack();
    auto & cluster = self->GetOnOffCluster();
    // SetOnOff invokes OnOnOffChanged on our delegate, which updates the LED and redraws the page.
    LogErrorOnFailure(cluster.SetOnOff(!cluster.GetOnOff()));
    chip::DeviceLayer::PlatformMgr().UnlockChipStack();
}
#endif // SL_MATTER_DISPLAY_ENABLED

} // namespace app
} // namespace chip
