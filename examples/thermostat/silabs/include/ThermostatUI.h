/*
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
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
