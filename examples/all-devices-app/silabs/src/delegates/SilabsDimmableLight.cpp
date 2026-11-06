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

#include "SilabsDimmableLight.h"

#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/PlatformManager.h>
#include <protocols/interaction_model/StatusCode.h>

#if SL_MATTER_DISPLAY_ENABLED
#include <cstdio>

#include "LightBitMap.h"
#include "glib.h"
#endif

namespace chip {
namespace app {

using Status = Protocols::InteractionModel::Status;

#if SL_MATTER_DISPLAY_ENABLED

namespace {
constexpr const char * kDeviceTypeName = "dimmable-light";
constexpr int kLcdSize                 = 128;
constexpr uint8_t kHeaderLine          = 0;
constexpr uint8_t kEndpointLine        = 1;
constexpr uint8_t kStateLine           = 10;

} // namespace

SilabsDimmableLight::SilabsDimmableLight(const Context & context, SilabsLCD & lcd, const Config & config) :
    DimmableLight(context,
                  Delegates{
                      .onOff        = *this,
                      .levelControl = *this,
                      .effect       = *this,
                      .identify     = context.identifyDelegate,
                  },
                  config),
    mLCD(&lcd)
{}

#else // !SL_MATTER_DISPLAY_ENABLED

SilabsDimmableLight::SilabsDimmableLight(const Context & context, const Config & config) :
    DimmableLight(context,
                  Delegates{
                      .onOff        = *this,
                      .levelControl = *this,
                      .effect       = *this,
                      .identify     = context.identifyDelegate,
                  },
                  config)
{}

#endif // SL_MATTER_DISPLAY_ENABLED

CHIP_ERROR SilabsDimmableLight::Register(EndpointIdAllocator & allocator, CodeDrivenDataModelProvider & provider,
                                         EndpointComposition composition)
{
    ReturnErrorOnFailure(SingleEndpoint::Register(allocator, provider, composition));

#if SL_MATTER_DISPLAY_ENABLED
    if (mLCD != nullptr)
    {
        // Non-fatal: if the LCD is out of page slots the device still works,
        // it just won't appear in the cycle-through UI.
        CHIP_ERROR pageErr =
            mLCD->RegisterDevicePage(GetEndpointId(), kDeviceTypeName, &DrawDevicePage, this, &OnDevicePageButtonPressed);
        if (pageErr != CHIP_NO_ERROR)
        {
            ChipLogError(AppServer, "SilabsDimmableLight: failed to register LCD page: %" CHIP_ERROR_FORMAT, pageErr.Format());
        }
    }
#endif

    return CHIP_NO_ERROR;
}

void SilabsDimmableLight::OnOffStartup(bool on)
{
    ChipLogProgress(DeviceLayer, "SilabsDimmableLight: OnOffStartup -> %s", on ? "ON" : "OFF");
    mOn = on;
}

void SilabsDimmableLight::OnOnOffChanged(bool on)
{
    ChipLogProgress(DeviceLayer, "SilabsDimmableLight: OnOnOffChanged -> %s", on ? "ON" : "OFF");
    mOn = on;
}

DataModel::ActionReturnStatus SilabsDimmableLight::TriggerDelayedAllOff(Clusters::OnOff::DelayedAllOffEffectVariantEnum)
{
    ChipLogProgress(DeviceLayer, "SilabsDimmableLight: TriggerDelayedAllOff");
    return Status::Success;
}

DataModel::ActionReturnStatus SilabsDimmableLight::TriggerDyingLight(Clusters::OnOff::DyingLightEffectVariantEnum)
{
    ChipLogProgress(DeviceLayer, "SilabsDimmableLight: TriggerDyingLight");
    return Status::Success;
}

void SilabsDimmableLight::OnLevelChanged(uint8_t level)
{
    ChipLogProgress(DeviceLayer, "SilabsDimmableLight: OnLevelChanged -> %u", level);
    mLevel = level;
}

void SilabsDimmableLight::OnOptionsChanged(BitMask<Clusters::LevelControl::OptionsBitmap> options)
{
    ChipLogProgress(DeviceLayer, "SilabsDimmableLight: OnOptionsChanged -> 0x%02X", options.Raw());
}

void SilabsDimmableLight::OnOnLevelChanged(DataModel::Nullable<uint8_t> onLevel)
{
    ChipLogProgress(DeviceLayer, "SilabsDimmableLight: OnOnLevelChanged -> %s", onLevel.IsNull() ? "NULL" : "set");
}

void SilabsDimmableLight::OnDefaultMoveRateChanged(DataModel::Nullable<uint8_t> defaultMoveRate)
{
    ChipLogProgress(DeviceLayer, "SilabsDimmableLight: OnDefaultMoveRateChanged -> %s", defaultMoveRate.IsNull() ? "NULL" : "set");
}

#if SL_MATTER_DISPLAY_ENABLED
void SilabsDimmableLight::DrawDevicePage(GLIB_Context_t * context, EndpointId endpointId, void * userContext)
{
    auto * self = static_cast<SilabsDimmableLight *>(userContext);

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
        GLIB_drawBitmap(context, x, y, kBitmapWidth, kBitmapHeight, self->mOn ? kLightbulbBitmapOn : kLightbulbBitmapOff);
    }
}

void SilabsDimmableLight::OnDevicePageButtonPressed(EndpointId /* endpointId */, void * userContext)
{
    auto * self = static_cast<SilabsDimmableLight *>(userContext);
    if (self == nullptr)
    {
        return;
    }
    chip::DeviceLayer::PlatformMgr().LockChipStack();
    auto & cluster = self->OnOffCluster();
    // SetOnOff invokes OnOnOffChanged on our delegate, which updates mOn.
    LogErrorOnFailure(cluster.SetOnOff(!cluster.GetOnOff()));

    if (self->mLCD != nullptr && self->mLCD->IsCurrentDevicePageFor(self->GetEndpointId()))
    {
        self->mLCD->SetScreen(SilabsLCD::DevicePageScreen);
    }
    chip::DeviceLayer::PlatformMgr().UnlockChipStack();
}
#endif // SL_MATTER_DISPLAY_ENABLED

} // namespace app
} // namespace chip
