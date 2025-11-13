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

#include <stdio.h>
#include <string.h>

#include "ThermostatUI.h"
#include "demo-ui-bitmaps.h"
#include "dmd.h"
#include "glib.h"
#include "lcd.h"

// Only needed for wifi NCP devices
#if SL_WIFI && !defined(SLI_SI91X_MCU_INTERFACE)
#include <platform/silabs/wifi/ncp/spi_multiplex.h>
#endif // SL_WIFI && !defined(SLI_SI91X_MCU_INTERFACE)

// LCD line define
constexpr uint8_t kTempLcdInitialX = 30;

// Bitmap
static const uint8_t silabsLogo[]       = { SILABS_LOGO_SMALL };
static const uint8_t matterLogoBitmap[] = { MATTER_LOGO_BITMAP };

static const uint8_t wifiLogo[]   = { WIFI_BITMAP };
static const uint8_t threadLogo[] = { THREAD_BITMAP };
static const uint8_t bleLogo[]    = { BLUETOOTH_ICON_SMALL };

static const unsigned char monaco_48pt[]          = { MONACO_48PT };
static const unsigned char heating_bits[]         = { HEATING_BITMAP };
static const unsigned char cooling_bits[]         = { COOLING_BITMAP };
static const unsigned char heating_cooling_bits[] = { HEATING_COOLING_BITMAP };

static int8_t mHeatingCelsiusSetPoint;
static int8_t mCoolingCelsiusSetPoint;
static int8_t mCurrentTempCelsius;
static uint8_t mMode;

#ifdef SL_WIFI
#define UI_WIFI 1
#else
#define UI_WIFI 0
#endif

void ThermostatUI::DrawUI(GLIB_Context_t * glibContext)
{
    if (glibContext == nullptr)
    {
        SILABS_LOG("Context is null");
        return;
    }

    GLIB_clear(glibContext);
    DrawHeader(glibContext);
    DrawCurrentTemp(glibContext, mCurrentTempCelsius);
    DrawFooter(glibContext, false);

#if SL_LCDCTRL_MUX
    sl_wfx_host_pre_lcd_spi_transfer();
#endif // SL_LCDCTRL_MUX
    DMD_updateDisplay();
#if SL_LCDCTRL_MUX
    sl_wfx_host_post_lcd_spi_transfer();
#endif // SL_LCDCTRL_MUX
}

void ThermostatUI::SetHeatingSetPoint(int8_t temp)
{
    mHeatingCelsiusSetPoint = temp;
}

void ThermostatUI::SetCoolingSetPoint(int8_t temp)
{
    mCoolingCelsiusSetPoint = temp;
}

void ThermostatUI::SetCurrentTemp(int8_t temp)
{
    mCurrentTempCelsius = temp;
}
void ThermostatUI::SetMode(uint8_t mode)
{
    mMode = mode;
}

void ThermostatUI::DrawHeader(GLIB_Context_t * glibContext)
{
    // Draw Silabs Corner icon
    GLIB_drawBitmap(glibContext, SILABS_ICON_POSITION_X, STATUS_ICON_LINE, SILABS_LOGO_WIDTH, SILABS_LOGO_HEIGHT, silabsLogo);
    // Draw BLE Icon
    GLIB_drawBitmap(glibContext, BLE_ICON_POSITION_X, STATUS_ICON_LINE, BLUETOOTH_ICON_SIZE, BLUETOOTH_ICON_SIZE, bleLogo);
    // Draw WiFi/OpenThread Icon
    GLIB_drawBitmap(glibContext, NETWORK_ICON_POSITION_X, STATUS_ICON_LINE, (UI_WIFI) ? WIFI_BITMAP_HEIGHT : THREAD_BITMAP_WIDTH,
                    WIFI_BITMAP_HEIGHT, (UI_WIFI) ? wifiLogo : threadLogo);
    // Draw Matter Icon
    GLIB_drawBitmap(glibContext, MATTER_ICON_POSITION_X, STATUS_ICON_LINE, MATTER_LOGO_WIDTH, MATTER_LOGO_HEIGHT, matterLogoBitmap);
#if SL_LCDCTRL_MUX
    sl_wfx_host_pre_lcd_spi_transfer();
#endif // SL_LCDCTRL_MUX
    DMD_updateDisplay();
#if SL_LCDCTRL_MUX
    sl_wfx_host_post_lcd_spi_transfer();
#endif // SL_LCDCTRL_MUX
}

void ThermostatUI::DrawFooter(GLIB_Context_t * glibContext, bool autoMode)
{
    switch (static_cast<ThermostatUI::HVACMode>(mMode))
    {
    case HVACMode::HEATING:
        GLIB_drawStringOnLine(glibContext, "Mode : Heating", 11, GLIB_ALIGN_LEFT, 0, 0, true);
        GLIB_drawBitmap(glibContext, HEATING_COOLING_X, HEATING_COOLING_Y, COOLING_WIDTH, COOLING_HEIGHT, heating_bits);
        DrawSetPoint(glibContext, mHeatingCelsiusSetPoint, false);
        break;
    case HVACMode::COOLING:
        GLIB_drawStringOnLine(glibContext, "Mode : Cooling", 11, GLIB_ALIGN_LEFT, 0, 0, true);
        GLIB_drawBitmap(glibContext, HEATING_COOLING_X, HEATING_COOLING_Y, COOLING_WIDTH, COOLING_HEIGHT, cooling_bits);
        DrawSetPoint(glibContext, mCoolingCelsiusSetPoint, false);
        break;
    case HVACMode::HEATING_COOLING:
        GLIB_drawStringOnLine(glibContext, "Mode : Auto", 11, GLIB_ALIGN_LEFT, 0, 0, true);
        GLIB_drawBitmap(glibContext, HEATING_COOLING_X, HEATING_COOLING_Y, COOLING_WIDTH, COOLING_HEIGHT, heating_cooling_bits);
        DrawSetPoint(glibContext, mHeatingCelsiusSetPoint, false);
        DrawSetPoint(glibContext, mCoolingCelsiusSetPoint, true);
        break;
    case HVACMode::MODE_OFF:
        DrawSetPoint(glibContext, 0, false);
        GLIB_drawStringOnLine(glibContext, "Mode : OFF", 11, GLIB_ALIGN_LEFT, 0, 0, true);
        break;
    default:
        break;
    }

#if SL_LCDCTRL_MUX
    sl_wfx_host_pre_lcd_spi_transfer();
#endif // SL_LCDCTRL_MUX
    DMD_updateDisplay();
#if SL_LCDCTRL_MUX
    sl_wfx_host_post_lcd_spi_transfer();
#endif // SL_LCDCTRL_MUX
}

/**
 * @brief Draw a 2 digit Temp of screen. Because of this Celsius is used by default
 * @param GLIB_Context_t * pointer to the context for the GLIB library
 * @param int8_t current Temperature in Celsius
 * @param int8_t setPoint in Celsius
 * @param bool isCelsius By default set to True. For future development
 */
void ThermostatUI::DrawCurrentTemp(GLIB_Context_t * glibContext, int8_t temp, bool isCelsius)
{
    uint8_t tempArray[2];
    uint8_t position_x = 10;
    uint8_t * data;

    if (temp > 99 || temp < -99)
    {
        // Invalid temp for a house thermostat
        return;
    }

    if (temp < 0)
    {
        for (uint8_t y = 60; y < 64; y++)
        {
            for (uint8_t x = 1; x < 10; x++)
            {
                GLIB_drawPixel(glibContext, x, y);
            }
        }

        temp = temp * -1;
    }

    // Draw Unit
    if (isCelsius)
    {
        data = (uint8_t *) &monaco_48pt[DEGREE_INDEX];
        DrawFont(glibContext, 65, kTempLcdInitialX, 15, data, MONACO_FONT_CH_LENGTH);
        data = (uint8_t *) &monaco_48pt[CELSIUS_INDEX];
        DrawFont(glibContext, 70, kTempLcdInitialX, 15, data, MONACO_FONT_CH_LENGTH);
    }
    else
    {
        data = (uint8_t *) &monaco_48pt[DEGREE_INDEX];
        DrawFont(glibContext, 65, 25, 15, data, MONACO_FONT_CH_LENGTH); // 25 to fit with f of fahrenheint
        data = (uint8_t *) &monaco_48pt[FAHRENHEIT_INDEX];
        DrawFont(glibContext, 70, kTempLcdInitialX, 15, data, MONACO_FONT_CH_LENGTH);
    }

    // Print Current temp
    tempArray[0] = (uint8_t) ((temp / 10));
    tempArray[1] = (uint8_t) ((temp % 10));

    for (uint8_t ch = 0; ch < 2; ch++)
    {
        data = (uint8_t *) &monaco_48pt[tempArray[ch] * MONACO_FONT_NB_LENGTH];
        DrawFont(glibContext, position_x, kTempLcdInitialX, MONACO_FONT_WIDTH, data, MONACO_FONT_NB_LENGTH);
        position_x += MONACO_FONT_WIDTH;
    }
}

void ThermostatUI::DrawFont(GLIB_Context_t * glibContext, uint8_t initial_x, uint8_t initial_y, uint8_t width, uint8_t * data,
                            uint32_t size)
{
    uint8_t x = initial_x, y = initial_y;
    for (uint16_t i = 0; i < size; i++)
    {
        for (uint8_t mask = 0; mask < 8; mask++)
        {
            if (!(data[i] & (0x01 << mask)))
            {
                GLIB_drawPixel(glibContext, x, y);
            }
            // Check line changes
            if (((x - initial_x) % width) == 0 && x != initial_x)
            {
                x = initial_x;
                y++;
                // Font is 8 bit align with paddings bits;
                mask = 8;
            }
            else
            {
                x++;
            }
        }
    }
}

void ThermostatUI::DrawSetPoint(GLIB_Context_t * glibContext, int8_t setPoint, bool secondLine)
{
    char setPointLine[] = { '-', 'X', 'X', '\0' };

    if (setPoint > 99 || setPoint < -99)
    {
        return;
    }

    // Update SetPoint string
    if (static_cast<ThermostatUI::HVACMode>(mMode) == ThermostatUI::HVACMode::MODE_OFF)
    {
        setPointLine[0] = '-';
        setPointLine[1] = '-';
        setPointLine[2] = '\0';
    }
    else if (setPoint < 0)
    {
        setPointLine[0] = (setPoint < 0) ? '-' : ' ';
        setPoint *= -1;
        setPointLine[1] = ((setPoint / 10) + 0x30);
        setPointLine[2] = ((setPoint % 10) + 0x30);
    }
    else
    {
        setPointLine[0] = ((setPoint / 10) + 0x30);
        setPointLine[1] = ((setPoint % 10) + 0x30);
        setPointLine[2] = '\0';
    }

    // Print SetPoint
    GLIB_drawStringOnLine(glibContext, setPointLine, (secondLine) ? 8 : 7, GLIB_ALIGN_LEFT, 67, 0, true);
}
