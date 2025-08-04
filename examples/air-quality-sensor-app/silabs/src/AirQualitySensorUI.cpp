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

#include "AirQualitySensorUI.h"
#include "AppTask.h"
#include "SensorManager.h"
#include "demo-ui-bitmaps.h"
#include "dmd.h"
#include <air-quality-sensor-manager.h>
#if DISPLAY_ENABLED
#include "glib.h"
#include "lcd.h"
#endif

using namespace chip::app::Clusters;
using namespace chip::app::Clusters::AirQuality;

namespace {
// Bitmap
const uint8_t silabsLogo[]       = { SILABS_LOGO_SMALL };
const uint8_t matterLogoBitmap[] = { MATTER_LOGO_BITMAP };

const uint8_t wifiLogo[]   = { WIFI_BITMAP };
const uint8_t threadLogo[] = { THREAD_BITMAP };
const uint8_t bleLogo[]    = { BLUETOOTH_ICON_SMALL };

const unsigned char monaco_48pt[] = { MONACO_48PT };

#ifdef SL_WIFI
constexpr bool UI_WIFI = true;
#else
constexpr bool UI_WIFI = false;
#endif
} // namespace

void AirQualitySensorUI::DrawUI(GLIB_Context_t * glibContext)
{
    if (glibContext == nullptr)
    {
        ChipLogDetail(AppServer, "Context is null");
        return;
    }

    GLIB_clear(glibContext);
    DrawHeader(glibContext);
    DrawCurrentAirQuality(glibContext);
    DrawFooter(glibContext);

#if SL_LCDCTRL_MUX
    sl_wfx_host_pre_lcd_spi_transfer();
#endif // SL_LCDCTRL_MUX
    DMD_updateDisplay();
#if SL_LCDCTRL_MUX
    sl_wfx_host_post_lcd_spi_transfer();
#endif // SL_LCDCTRL_MUX
}

void AirQualitySensorUI::DrawHeader(GLIB_Context_t * glibContext)
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

void AirQualitySensorUI::DrawFooter(GLIB_Context_t * glibContext)
{
    switch (AirQualitySensorManager::GetInstance()->GetAirQuality())
    {
    case AirQualityEnum::kGood:
        GLIB_drawStringOnLine(glibContext, "AQ : Good", 11, GLIB_ALIGN_LEFT, 0, 0, true);
        break;
    case AirQualityEnum::kFair:
        GLIB_drawStringOnLine(glibContext, "AQ : Fair", 11, GLIB_ALIGN_LEFT, 0, 0, true);
        break;
    case AirQualityEnum::kModerate:
        GLIB_drawStringOnLine(glibContext, "AQ : Moderate", 11, GLIB_ALIGN_LEFT, 0, 0, true);
        break;
    case AirQualityEnum::kPoor:
        GLIB_drawStringOnLine(glibContext, "AQ : Poor", 11, GLIB_ALIGN_LEFT, 0, 0, true);
        break;
    case AirQualityEnum::kVeryPoor:
        GLIB_drawStringOnLine(glibContext, "AQ : VPoor", 11, GLIB_ALIGN_LEFT, 0, 0, true);
        break;
    case AirQualityEnum::kExtremelyPoor:
        GLIB_drawStringOnLine(glibContext, "AQ : EPoor", 11, GLIB_ALIGN_LEFT, 0, 0, true);
        break;
    case AirQualityEnum::kUnknown:
        GLIB_drawStringOnLine(glibContext, "AQ : Unknown", 11, GLIB_ALIGN_LEFT, 0, 0, true);
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
 * @brief Draw a 2 digit Air Quality of screen. Because of this Celsius is used by default
 * @param GLIB_Context_t * pointer to the context for the GLIB library
 * @param int8_t current Air Quality
 */
void AirQualitySensorUI::DrawCurrentAirQuality(GLIB_Context_t * glibContext)
{
    // LCD line define
    constexpr uint8_t kAirQualityLcdInitialX = 30;
    uint8_t position_x                       = 10;
    uint8_t * data;
    uint8_t print_val;

    // Print Current air quality
    print_val = 0;
    data      = (uint8_t *) &monaco_48pt[print_val * MONACO_FONT_NB_LENGTH];
    DrawFont(glibContext, position_x, kAirQualityLcdInitialX, MONACO_FONT_WIDTH, data, MONACO_FONT_NB_LENGTH);
    position_x += MONACO_FONT_WIDTH;

    print_val = static_cast<uint8_t>(AirQualitySensorManager::GetInstance()->GetAirQuality());
    data      = (uint8_t *) &monaco_48pt[print_val * MONACO_FONT_NB_LENGTH];
    DrawFont(glibContext, position_x, kAirQualityLcdInitialX, MONACO_FONT_WIDTH, data, MONACO_FONT_NB_LENGTH);
}

void AirQualitySensorUI::DrawFont(GLIB_Context_t * glibContext, uint8_t initial_x, uint8_t initial_y, uint8_t width, uint8_t * data,
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
