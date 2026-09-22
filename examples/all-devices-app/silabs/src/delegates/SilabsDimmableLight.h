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
#include <device/types/dimmable-light/DimmableLight.h>

#if SL_MATTER_DISPLAY_ENABLED
#include "lcd.h"
#endif

namespace chip {
namespace app {

/**
 * Silabs implementation of the dimmable-light device type.
 *
 * Extends DimmableLight (which owns the cluster machinery) with
 * Silabs-specific hardware/UI wiring. On kits with an LCD, an LCD page is
 * registered so the user can cycle to a lightbulb view via button 0; the
 * current OnOff / current-level state is mirrored to that page.
 *
 * Hardware LED (PWM) wiring can be added by extending the OnLevelChanged /
 * OnOnOffChanged implementations below.
 */
class SilabsDimmableLight : public DimmableLight,
                            public Clusters::OnOffDelegate,
                            public Clusters::OnOffEffectDelegate,
                            public Clusters::LevelControlDelegate
{
public:
#if SL_MATTER_DISPLAY_ENABLED
    SilabsDimmableLight(const Context & context, SilabsLCD & lcd, const Config & config = {});
#else
    explicit SilabsDimmableLight(const Context & context, const Config & config = {});
#endif
    ~SilabsDimmableLight() override = default;

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

protected:
    using DimmableLoad::Register;

private:
#if SL_MATTER_DISPLAY_ENABLED
    static void DrawDevicePage(GLIB_Context_t * context, EndpointId endpointId, void * userContext);
    // Action-button callback: toggles OnOff for this endpoint via the OnOff cluster.
    static void OnDevicePageButtonPressed(EndpointId endpointId, void * userContext);

    SilabsLCD * mLCD = nullptr;
#endif
    bool mOn      = false;
    uint8_t mLevel = 0;
};

} // namespace app
} // namespace chip
