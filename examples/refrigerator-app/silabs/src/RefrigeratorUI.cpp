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

#include <stdio.h>
#include <string.h>

#include "RefrigeratorUI.h"
#include "demo-ui-bitmaps.h"
#include "dmd.h"
#include "glib.h"
#include "lcd.h"
#include <lib/support/logging/CHIPLogging.h>

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

static int8_t mCurrentTempCelsius;
static uint8_t mMode;

#ifdef SL_WIFI
#define UI_WIFI 1
#else
#define UI_WIFI 0
#endif

/**
 * @brief Draw the entire UI on the screen including header, footer, and temperature.
 * @param GLIB_Context_t * pointer to the context for the GLIB library
 */
void RefrigeratorUI::DrawUI(GLIB_Context_t * glibContext)
{
    if (glibContext == nullptr)
    {
        ChipLogError(AppServer, "App Task started");
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

/**
 * @brief Set the current temperature of the refrigerator.
 * @param int8_t temp The current temperature in Celsius
 */
void RefrigeratorUI::SetCurrentTemp(int8_t temp)
{
    mCurrentTempCelsius = temp;
}

/**
 * @brief Set the operating mode of the refrigerator.
 * @param uint8_t mode Mode of the refrigerator (e.g., cooling, defrosting)
 */
void RefrigeratorUI::SetMode(uint8_t mode)
{
    mMode = mode;
}

/**
 * @brief Draw the header of the UI, usually displaying the title or current status.
 * @param GLIB_Context_t * pointer to the context for the GLIB library
 */
void RefrigeratorUI::DrawHeader(GLIB_Context_t * glibContext)
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

/**
 * @brief Draw the footer of the UI, which may display additional information such as mode.
 * @param GLIB_Context_t * pointer to the context for the GLIB library
 * @param bool autoMode Whether the refrigerator is in automatic mode
 */
void RefrigeratorUI::DrawFooter(GLIB_Context_t * glibContext, bool autoMode)
{
    // TODO:: Need to implement this function

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
void RefrigeratorUI::DrawCurrentTemp(GLIB_Context_t * glibContext, int8_t temp, bool isCelsius)
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

/**
 * @brief Draw a font at a specified position on the screen.
 * @param GLIB_Context_t * pointer to the context for the GLIB library
 * @param uint8_t initial_x X-coordinate for the starting position
 * @param uint8_t initial_y Y-coordinate for the starting position
 * @param uint8_t width Width of the font to be drawn
 * @param uint8_t * data Pointer to the font data
 * @param uint32_t size Size of the font data in bytes
 */
void RefrigeratorUI::DrawFont(GLIB_Context_t * glibContext, uint8_t initial_x, uint8_t initial_y, uint8_t width, uint8_t * data,
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

/**
 * @brief Draw the set point temperature on the screen.
 * @param GLIB_Context_t * pointer to the context for the GLIB library
 * @param int8_t setPoint The set point temperature in Celsius
 * @param bool secondLine Whether to display on the second line (true) or first line (false)
 */
void RefrigeratorUI::DrawSetPoint(GLIB_Context_t * glibContext, int8_t setPoint, bool secondLine)
{
    char setPointLine[] = { '-', 'X', 'X', '\0' };

    if (setPoint > 99 || setPoint < -99)
    {
        return;
    }

    // Update SetPoint string
    if (setPoint < 0)
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
