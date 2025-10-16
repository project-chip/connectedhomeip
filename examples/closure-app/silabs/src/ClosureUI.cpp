/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
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

#include "AppConfig.h"
#include "ClosureUI.h"
#include "demo-ui-bitmaps.h"
#include "dmd.h"
#include "glib.h"
#include "lcd.h"

// Only needed for wifi NCP devices
#if SL_WIFI && !defined(SLI_SI91X_MCU_INTERFACE)
#include <platform/silabs/wifi/ncp/spi_multiplex.h>
#endif // SL_WIFI && !defined(SLI_SI91X_MCU_INTERFACE)

// Layout positions
#define STATUS_ICON_LINE 0
#define SILABS_ICON_POSITION_X 0
#define BLE_ICON_POSITION_X 72
#define NETWORK_ICON_POSITION_X 90
#define MATTER_ICON_POSITION_X 108

// Icon sizes
#define SILABS_LOGO_WIDTH 47
#define SILABS_LOGO_HEIGHT 18  
#define BLUETOOTH_ICON_SIZE 18

// State display positions
#define STATE_DISPLAY_LINE 3
#define POSITION_DISPLAY_LINE 5
#define LATCH_DISPLAY_LINE 6
#define SECURE_DISPLAY_LINE 7
#define SPEED_DISPLAY_LINE 8
#define FOOTER_DISPLAY_LINE 11

// Bitmap definitions
static const uint8_t silabsLogo[]       = { SILABS_LOGO_SMALL };
static const uint8_t matterLogoBitmap[] = { MATTER_LOGO_BITMAP };
static const uint8_t wifiLogo[]         = { WIFI_BITMAP };
static const uint8_t threadLogo[]       = { THREAD_BITMAP };
static const uint8_t bleLogo[]          = { BLUETOOTH_ICON_SMALL };

// Static variables to hold the current closure state
ClosureUI::MainState ClosureUI::sMainState = ClosureUI::STATE_UNKNOWN;
char ClosureUI::sPositionText[24] = "Position: Unknown";
char ClosureUI::sLatchText[20] = "Latch: Unknown";
char ClosureUI::sSecureText[20] = "Secure: Unknown";
char ClosureUI::sSpeedText[18] = "Speed: Unknown";

#ifdef SL_WIFI
#define UI_WIFI 1
#else
#define UI_WIFI 0
#endif

void ClosureUI::DrawUI(GLIB_Context_t * glibContext)
{
    if (glibContext == nullptr)
    {
        SILABS_LOG("ClosureUI: Graphics context is null");
        return;
    }

    GLIB_clear(glibContext);
    
    DrawHeader(glibContext);
    DrawMainState(glibContext);
    DrawOverallCurrentState(glibContext);
    DrawFooter(glibContext);

#if SL_LCDCTRL_MUX
    sl_wfx_host_pre_lcd_spi_transfer();
#endif // SL_LCDCTRL_MUX
    DMD_updateDisplay();
#if SL_LCDCTRL_MUX
    sl_wfx_host_post_lcd_spi_transfer();
#endif // SL_LCDCTRL_MUX
}

void ClosureUI::SetMainState(uint8_t state)
{
    // Validate state value is within valid range
    if (state > STATE_UNKNOWN)
    {
        SILABS_LOG("ClosureUI: Invalid state value %d, defaulting to STATE_UNKNOWN", state);
        sMainState = STATE_UNKNOWN;
    }
    else
    {
        sMainState = static_cast<MainState>(state);
    }
}

void ClosureUI::SetOverallCurrentState(const char * positionText, const char * latchText, const char * secureText, const char * speedText)
{
    // Validate input parameters
    if (positionText == nullptr || latchText == nullptr || secureText == nullptr || speedText == nullptr)
    {
        SILABS_LOG("ClosureUI: Null parameter in SetOverallCurrentState");
        return;
    }
    
    strncpy(sPositionText, positionText, sizeof(sPositionText) - 1);
    sPositionText[sizeof(sPositionText) - 1] = '\0';
    
    strncpy(sLatchText, latchText, sizeof(sLatchText) - 1);
    sLatchText[sizeof(sLatchText) - 1] = '\0';
    
    strncpy(sSecureText, secureText, sizeof(sSecureText) - 1);
    sSecureText[sizeof(sSecureText) - 1] = '\0';
    
    strncpy(sSpeedText, speedText, sizeof(sSpeedText) - 1);
    sSpeedText[sizeof(sSpeedText) - 1] = '\0';
}

void ClosureUI::DrawHeader(GLIB_Context_t * glibContext)
{
    // Draw Silabs Corner icon
    GLIB_drawBitmap(glibContext, SILABS_ICON_POSITION_X, STATUS_ICON_LINE, SILABS_LOGO_WIDTH, SILABS_LOGO_HEIGHT, silabsLogo);
    // Draw BLE Icon
    GLIB_drawBitmap(glibContext, BLE_ICON_POSITION_X, STATUS_ICON_LINE, BLUETOOTH_ICON_SIZE, BLUETOOTH_ICON_SIZE, bleLogo);
    // Draw WiFi/OpenThread Icon
    GLIB_drawBitmap(glibContext, NETWORK_ICON_POSITION_X, STATUS_ICON_LINE, 
                    (UI_WIFI) ? WIFI_BITMAP_WIDTH : THREAD_BITMAP_WIDTH,
                    (UI_WIFI) ? WIFI_BITMAP_HEIGHT : THREAD_BITMAP_HEIGHT, 
                    (UI_WIFI) ? wifiLogo : threadLogo);
    // Draw Matter Icon
    GLIB_drawBitmap(glibContext, MATTER_ICON_POSITION_X, STATUS_ICON_LINE, MATTER_LOGO_WIDTH, MATTER_LOGO_HEIGHT, matterLogoBitmap);
}

void ClosureUI::DrawMainState(GLIB_Context_t * glibContext)
{
    const char * stateText = "";
    
    switch (sMainState)
    {
    case STATE_STOPPED:
        stateText = "State: Stopped";
        break;
    case STATE_MOVING:
        stateText = "State: Moving";
        break;
    case STATE_WAITING_FOR_MOTION:
        stateText = "State: Waiting";
        break;
    case STATE_ERROR:
        stateText = "State: Error";
        break;
    case STATE_CALIBRATING:
        stateText = "State: Calibrating";
        break;
    case STATE_PROTECTED:
        stateText = "State: Protected";
        break;
    case STATE_DISENGAGED:
        stateText = "State: Disengaged";
        break;
    case STATE_SETUP_REQUIRED:
        stateText = "State: Setup Required";
        break;
    case STATE_UNKNOWN:
    default:
        stateText = "State: Unknown";
        break;
    }
    
    GLIB_drawStringOnLine(glibContext, stateText, STATE_DISPLAY_LINE, GLIB_ALIGN_CENTER, 0, 0, true);
}

void ClosureUI::DrawOverallCurrentState(GLIB_Context_t * glibContext)
{
    // Draw position information
    GLIB_drawStringOnLine(glibContext, sPositionText, POSITION_DISPLAY_LINE, GLIB_ALIGN_LEFT, 0, 0, true);
    
    // Draw latch information
    GLIB_drawStringOnLine(glibContext, sLatchText, LATCH_DISPLAY_LINE, GLIB_ALIGN_LEFT, 0, 0, true);
    
    // Draw secure state information
    GLIB_drawStringOnLine(glibContext, sSecureText, SECURE_DISPLAY_LINE, GLIB_ALIGN_LEFT, 0, 0, true);
    
    // Draw speed information
    GLIB_drawStringOnLine(glibContext, sSpeedText, SPEED_DISPLAY_LINE, GLIB_ALIGN_LEFT, 0, 0, true);
}



void ClosureUI::DrawFooter(GLIB_Context_t * glibContext)
{
    // Draw a simple footer indicating this is the closure app
    GLIB_drawStringOnLine(glibContext, "Closure Control", FOOTER_DISPLAY_LINE, GLIB_ALIGN_CENTER, 0, 0, true);
}

void ClosureUI::DrawStateIcon(GLIB_Context_t * glibContext, MainState state)
{
    // For future enhancement: draw visual icons based on closure state
    // This could include door/window icons, arrows for movement, etc.
}

void ClosureUI::DrawFont(GLIB_Context_t * glibContext, uint8_t initial_x, uint8_t initial_y, uint8_t width, uint8_t * data,
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