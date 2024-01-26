/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2019 Google LLC.
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


#include "AppEvent.h"
#include "BaseApplication.h"
#include "FreeRTOS.h"
#include "timers.h" // provides FreeRTOS timer support

#include <lib/core/CHIPError.h>

#if DISPLAY_ENABLED
#include "glib.h"
#endif

/*
 * Common methods
 */

// Determines current mode of sensor app. BTN0 to swap between modes
enum CurrentSensorEnum
{
    OccupancySensor = 0,
    TemperatureSensor = 1,
    ContactSensor = 2,
    StatusScreen = 3,
#ifdef QR_CODE_ENABLED
    QrCode = 4,
#endif
    InvalidSensor = 5
};


class SilabsSensors
{
public:
    static void UpdateSensorDisplay(void);
    static void ActionTriggered(AppEvent * aEvent);
#ifdef DISPLAY_ENABLED
    static void TemperatureUI(GLIB_Context_t * glibContext);
#endif

    inline uint16_t getCurrentSensorMode() { return currentSensorMode;};
    CHIP_ERROR Init();

    /**
     * @brief Timer Event processing function
     *        Trigger factory if Press and Hold duration is respected
     *
     * @param aEvent post event being processed
     */
    static void CycleSensor(AppEvent * aEvent);

private :
    static uint16_t currentSensorMode;
    static bool mIsSensorTriggered;
    static void UpdateBinarySensor(bool state);
    friend SilabsSensors & SensorMgr();
    static std::string getSensorModeString(uint16_t);

    static SilabsSensors sSensorManager;
};

inline SilabsSensors & SensorMgr()
{
    return SilabsSensors::sSensorManager;
}
