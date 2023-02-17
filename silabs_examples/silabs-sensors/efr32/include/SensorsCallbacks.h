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

/*
 * Callbacks for All of the Sensor Applications
 */

#if SL_SENSOR_TYPE == 1
#define SENSOR_NAME "Occupancy-App"
#elif SL_SENSOR_TYPE == 2
#if DISPLAY_ENABLED
#include "glib.h"
#endif
#define SENSOR_NAME "Temperature-App"
#elif SL_SENSOR_TYPE == 3
#define SENSOR_NAME "Contact-App"
#endif

/*
 * Common methods
 */

class SilabsSensors
{
public:
    static void InitSensor(void);

    static void ActionTriggered(AppEvent * aEvent);
#if SL_SENSOR_TYPE == 2
#ifdef DISPLAY_ENABLED
    static void TemperatureUI(GLIB_Context_t * glibContext);
#endif
#endif

private :
#if (SL_SENSOR_TYPE == 1) || (SL_SENSOR_TYPE == 3)
    static bool mIsSensorTriggered;
    static void UpdateBinarySensor(bool state);
#endif

};
