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

#include "SilabsHumiditySensor.h"

#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>

#if SL_MATTER_DISPLAY_ENABLED
#include <cstdio>

#include "glib.h"
#endif

#if defined(SL_MATTER_USE_SI70XX_SENSOR) && SL_MATTER_USE_SI70XX_SENSOR
#include "sensors/Si70xxSensor.h"
#endif

namespace chip {
namespace app {

namespace {
constexpr const char * kDeviceTypeName = "humidity-sensor";
} // namespace

#if SL_MATTER_DISPLAY_ENABLED

namespace {
constexpr uint8_t kHeaderLine   = 0;
constexpr uint8_t kEndpointLine = 1;
constexpr uint8_t kValueLine    = 6;
} // namespace

SilabsHumiditySensor::SilabsHumiditySensor(TimerDelegate & timerDelegate, SilabsLCD & lcd) :
    IncreasingHumiditySensor(timerDelegate), mLCD(&lcd)
{}

#else // !SL_MATTER_DISPLAY_ENABLED

SilabsHumiditySensor::SilabsHumiditySensor(TimerDelegate & timerDelegate) : IncreasingHumiditySensor(timerDelegate) {}

#endif // SL_MATTER_DISPLAY_ENABLED

CHIP_ERROR SilabsHumiditySensor::Register(EndpointIdAllocator & allocator, CodeDrivenDataModelProvider & provider,
                                          EndpointComposition composition)
{
    ReturnErrorOnFailure(SingleEndpoint::Register(allocator, provider, composition));

#if SL_MATTER_DISPLAY_ENABLED
    if (mLCD != nullptr)
    {
        // Non-fatal: if the LCD is out of page slots the device still works,
        // it just won't appear in the cycle-through UI.
        CHIP_ERROR pageErr = mLCD->RegisterDevicePage(GetEndpointId(), kDeviceTypeName, &DrawDevicePage, this);
        if (pageErr != CHIP_NO_ERROR)
        {
            ChipLogError(AppServer, "SilabsHumiditySensor: failed to register LCD page: %" CHIP_ERROR_FORMAT, pageErr.Format());
        }
    }
#endif

    return CHIP_NO_ERROR;
}

#if SL_MATTER_DISPLAY_ENABLED
void SilabsHumiditySensor::DrawDevicePage(GLIB_Context_t * context, EndpointId endpointId, void * userContext)
{
    auto * self = static_cast<SilabsHumiditySensor *>(userContext);

    GLIB_drawStringOnLine(context, kDeviceTypeName, kHeaderLine, GLIB_ALIGN_CENTER, 0, 0, true);

    char epLine[24];
    std::snprintf(epLine, sizeof(epLine), "Endpoint %u", static_cast<unsigned>(endpointId));
    GLIB_drawStringOnLine(context, epLine, kEndpointLine, GLIB_ALIGN_CENTER, 0, 0, true);

    if (self != nullptr)
    {
        // MeasuredValue is reported in 0.01 %RH units per the spec.
        DataModel::Nullable<uint16_t> measured = self->RelativeHumidityMeasurementCluster().GetMeasuredValue();
        char valueLine[24];
        if (measured.IsNull())
        {
            std::snprintf(valueLine, sizeof(valueLine), "RH: --.-- %%");
        }
        else
        {
            unsigned raw   = measured.Value();
            unsigned whole = raw / 100u;
            unsigned frac  = raw % 100u;
            std::snprintf(valueLine, sizeof(valueLine), "RH: %u.%02u %%", whole, frac);
        }
        GLIB_drawStringOnLine(context, valueLine, kValueLine, GLIB_ALIGN_CENTER, 0, 0, true);
    }
}
#endif // SL_MATTER_DISPLAY_ENABLED

#if defined(SL_MATTER_USE_SI70XX_SENSOR) && SL_MATTER_USE_SI70XX_SENSOR
namespace {
constexpr System::Clock::Seconds16 kSi70xxSampleIntervalSec = System::Clock::Seconds16(5);
} // namespace

void SilabsHumiditySensor::TimerFired()
{
    uint16_t humidity   = 0;
    int16_t temperature = 0;
    sl_status_t status  = Si70xxSensor::GetSensorData(humidity, temperature);
    if (status == SL_STATUS_OK)
    {
        LogErrorOnFailure(mRelativeHumidityMeasurementCluster.Cluster().SetMeasuredValue(DataModel::MakeNullable(humidity)));
#if SL_MATTER_DISPLAY_ENABLED
        // Refresh the LCD only if our own device page is currently displayed.
        if (mLCD != nullptr && mLCD->IsCurrentDevicePageFor(GetEndpointId()))
        {
            mLCD->SetScreen(SilabsLCD::DevicePageScreen);
        }
#endif
    }
    else
    {
        ChipLogError(AppServer, "SilabsHumiditySensor: Si70xx read failed: %x", status);
    }

    LogErrorOnFailure(HumiditySensor::mTimerDelegate.StartTimer(this, kSi70xxSampleIntervalSec));
}
#endif // defined(SL_MATTER_USE_SI70XX_SENSOR) && SL_MATTER_USE_SI70XX_SENSOR

} // namespace app
} // namespace chip
