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

#include <device/types/temperature-sensor/impl/IncreasingTemperatureSensor.h>

#if SL_MATTER_DISPLAY_ENABLED
#include "lcd.h"
#endif

namespace chip {
namespace app {

/**
 * Silabs implementation of the temperature-sensor device type.
 *
 * Extends IncreasingTemperatureSensor (which owns the cluster machinery and
 * drives a simulated temperature value) with Silabs-specific UI wiring. On
 * kits with an LCD, an LCD page is registered so the user can cycle to a
 * temperature view via button 0; the current MeasuredValue attribute is
 * mirrored to that page.
 */
class SilabsTemperatureSensor : public IncreasingTemperatureSensor
{
public:
#if SL_MATTER_DISPLAY_ENABLED
    explicit SilabsTemperatureSensor(SilabsLCD & lcd);
#else
    SilabsTemperatureSensor() = default;
#endif
    ~SilabsTemperatureSensor() override = default;

    // DeviceInterface: allocate endpoint, register clusters (via base), then register LCD page.
    CHIP_ERROR Register(EndpointIdAllocator & allocator, CodeDrivenDataModelProvider & provider,
                        EndpointComposition composition = {}) override;

#if defined(SL_MATTER_USE_SI70XX_SENSOR) && SL_MATTER_USE_SI70XX_SENSOR
    // TimerContext: replaces the base simulation with a live Si70xx read.
    void TimerFired() override;
#endif

protected:
    using IncreasingTemperatureSensor::Register;

private:
#if SL_MATTER_DISPLAY_ENABLED
    static void DrawDevicePage(GLIB_Context_t * context, EndpointId endpointId, void * userContext);

    SilabsLCD * mLCD = nullptr;
#endif
};

} // namespace app
} // namespace chip
