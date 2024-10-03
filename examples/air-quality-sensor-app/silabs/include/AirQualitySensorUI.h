/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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

#include "demo-ui-bitmaps.h"
#include "dmd.h"
#include "glib.h"
#include "lcd.h"

class AirQualitySensorUI
{

public:
    static void DrawUI(GLIB_Context_t * glibContext);

private:
    static void DrawHeader(GLIB_Context_t * glibContext);
    static void DrawFooter(GLIB_Context_t * glibContext);
    static void DrawCurrentAirQuality(GLIB_Context_t * glibContext);
    static void DrawFont(GLIB_Context_t * glibContext, uint8_t initial_x, uint8_t initial_y, uint8_t width, uint8_t * data,
                         uint32_t size);
};
