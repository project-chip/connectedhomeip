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

#include <device/types/humidity-sensor/impl/IncreasingHumiditySensor.h>
#include <lib/support/TimerDelegate.h>

#if SL_MATTER_DISPLAY_ENABLED
#include "lcd.h"
#endif

namespace chip {
namespace app {

/**
 * Silabs implementation of the humidity-sensor device type.
 *
 * Extends IncreasingHumiditySensor (which owns the cluster machinery and
 * drives a simulated humidity value) with Silabs-specific UI wiring. On
 * kits with an LCD, an LCD page is registered so the user can cycle to a
 * humidity view via button 0; the current MeasuredValue attribute is
 * mirrored to that page.
 */
class SilabsHumiditySensor : public IncreasingHumiditySensor
{
public:
#if SL_MATTER_DISPLAY_ENABLED
    SilabsHumiditySensor(TimerDelegate & timerDelegate, SilabsLCD & lcd);
#else
    explicit SilabsHumiditySensor(TimerDelegate & timerDelegate);
#endif
    ~SilabsHumiditySensor() override = default;

    // DeviceInterface: allocate endpoint, register clusters (via base), then register LCD page.
    CHIP_ERROR Register(EndpointIdAllocator & allocator, CodeDrivenDataModelProvider & provider,
                        EndpointComposition composition = {}) override;

#if defined(SL_MATTER_USE_SI70XX_SENSOR) && SL_MATTER_USE_SI70XX_SENSOR
    // TimerContext: replaces the base simulation with a live Si70xx read.
    void TimerFired() override;
#endif

protected:
    using IncreasingHumiditySensor::Register;

private:
#if SL_MATTER_DISPLAY_ENABLED
    static void DrawDevicePage(GLIB_Context_t * context, EndpointId endpointId, void * userContext);

    SilabsLCD * mLCD = nullptr;
#endif
};

} // namespace app
} // namespace chip
