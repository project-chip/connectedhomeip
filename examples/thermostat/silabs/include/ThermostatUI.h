/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

#include "ThermostatIcons.h"
#include "glib.h"
#include "lcd.h"

class ThermostatUI
{

public:
    // AppCluster Spec Table 85.
    enum HVACMode
    {
        MODE_OFF = 0,
        HEATING_COOLING,
        NOT_USED,
        COOLING,
        HEATING,
    };

    static void DrawUI(GLIB_Context_t * glibContext);
    static void SetHeatingSetPoint(int8_t temp);
    static void SetCoolingSetPoint(int8_t temp);
    static void SetCurrentTemp(int8_t temp);
    static void SetMode(uint8_t mode);

private:
    static void DrawHeader(GLIB_Context_t * glibContext);
    static void DrawFooter(GLIB_Context_t * glibContext, bool autoMode = true);
    static void DrawCurrentTemp(GLIB_Context_t * glibContext, int8_t temp, bool isCelsius = true);
    static void DrawFont(GLIB_Context_t * glibContext, uint8_t initial_x, uint8_t initial_y, uint8_t width, uint8_t * data,
                         uint32_t size);
    static void DrawSetPoint(GLIB_Context_t * glibContext, int8_t setPoint, bool secondLine);
};
