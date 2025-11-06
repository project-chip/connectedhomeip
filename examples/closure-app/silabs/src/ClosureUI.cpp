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

#include "AppConfig.h"
#include "ClosureUI.h"
#include "ClosureUIStrings.h"
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
chip::app::Clusters::ClosureControl::MainStateEnum ClosureUI::sMainState =
    chip::app::Clusters::ClosureControl::MainStateEnum::kUnknownEnumValue;

// Helper function to format text using prefix + suffix
namespace {
inline void FormatText(char * dest, size_t destSize, const char * prefix, const char * suffix)
{
    snprintf(dest, destSize, "%s%s", prefix, suffix);
}
} // namespace

// Initialize arrays using buffer sizes
char ClosureUI::sPositionText[ClosureUIStrings::BUFFER_SIZE_POSITION];
char ClosureUI::sLatchText[ClosureUIStrings::BUFFER_SIZE_LATCH];
char ClosureUI::sSecureText[ClosureUIStrings::BUFFER_SIZE_SECURE];
char ClosureUI::sSpeedText[ClosureUIStrings::BUFFER_SIZE_SPEED];
char ClosureUI::sStateText[ClosureUIStrings::BUFFER_SIZE_STATE];

// Static initializer to set default values using prefix + suffix
struct ClosureUITextInitializer
{
    ClosureUITextInitializer()
    {
        FormatText(ClosureUI::sPositionText, sizeof(ClosureUI::sPositionText), ClosureUIStrings::POSITION_PREFIX,
                   ClosureUIStrings::POSITION_SUFFIX_UNKNOWN);
        FormatText(ClosureUI::sLatchText, sizeof(ClosureUI::sLatchText), ClosureUIStrings::LATCH_PREFIX,
                   ClosureUIStrings::LATCH_SUFFIX_UNKNOWN);
        FormatText(ClosureUI::sSecureText, sizeof(ClosureUI::sSecureText), ClosureUIStrings::SECURE_PREFIX,
                   ClosureUIStrings::SECURE_SUFFIX_UNKNOWN);
        FormatText(ClosureUI::sSpeedText, sizeof(ClosureUI::sSpeedText), ClosureUIStrings::SPEED_PREFIX,
                   ClosureUIStrings::SPEED_SUFFIX_UNKNOWN);
        FormatText(ClosureUI::sStateText, sizeof(ClosureUI::sStateText), ClosureUIStrings::STATE_PREFIX,
                   ClosureUIStrings::STATE_SUFFIX_UNKNOWN);
    }
};
static ClosureUITextInitializer sInitializer;

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

void ClosureUI::SetMainState(chip::app::Clusters::ClosureControl::MainStateEnum state)
{
    sMainState = state;
}

void ClosureUI::DrawHeader(GLIB_Context_t * glibContext)
{
    // Draw Silabs Corner icon
    GLIB_drawBitmap(glibContext, SILABS_ICON_POSITION_X, STATUS_ICON_LINE, SILABS_LOGO_WIDTH, SILABS_LOGO_HEIGHT, silabsLogo);
    // Draw BLE Icon
    GLIB_drawBitmap(glibContext, BLE_ICON_POSITION_X, STATUS_ICON_LINE, BLUETOOTH_ICON_SIZE, BLUETOOTH_ICON_SIZE, bleLogo);
    // Draw WiFi/OpenThread Icon
#ifdef SL_WIFI
    GLIB_drawBitmap(glibContext, NETWORK_ICON_POSITION_X, STATUS_ICON_LINE, WIFI_BITMAP_WIDTH, WIFI_BITMAP_HEIGHT, wifiLogo);
#else
    GLIB_drawBitmap(glibContext, NETWORK_ICON_POSITION_X, STATUS_ICON_LINE, THREAD_BITMAP_WIDTH, THREAD_BITMAP_HEIGHT, threadLogo);
#endif // SL_WIFI
    // Draw Matter Icon
    GLIB_drawBitmap(glibContext, MATTER_ICON_POSITION_X, STATUS_ICON_LINE, MATTER_LOGO_WIDTH, MATTER_LOGO_HEIGHT, matterLogoBitmap);
}

void ClosureUI::DrawMainState(GLIB_Context_t * glibContext)
{
    const char * stateSuffix = ClosureUIStrings::STATE_SUFFIX_UNKNOWN;

    switch (sMainState)
    {
    case chip::app::Clusters::ClosureControl::MainStateEnum::kStopped:
        stateSuffix = ClosureUIStrings::STATE_SUFFIX_STOPPED;
        break;
    case chip::app::Clusters::ClosureControl::MainStateEnum::kMoving:
        stateSuffix = ClosureUIStrings::STATE_SUFFIX_MOVING;
        break;
    case chip::app::Clusters::ClosureControl::MainStateEnum::kWaitingForMotion:
        stateSuffix = ClosureUIStrings::STATE_SUFFIX_WAITING;
        break;
    case chip::app::Clusters::ClosureControl::MainStateEnum::kError:
        stateSuffix = ClosureUIStrings::STATE_SUFFIX_ERROR;
        break;
    case chip::app::Clusters::ClosureControl::MainStateEnum::kCalibrating:
        stateSuffix = ClosureUIStrings::STATE_SUFFIX_CALIBRATING;
        break;
    case chip::app::Clusters::ClosureControl::MainStateEnum::kProtected:
        stateSuffix = ClosureUIStrings::STATE_SUFFIX_PROTECTED;
        break;
    case chip::app::Clusters::ClosureControl::MainStateEnum::kDisengaged:
        stateSuffix = ClosureUIStrings::STATE_SUFFIX_DISENGAGED;
        break;
    case chip::app::Clusters::ClosureControl::MainStateEnum::kSetupRequired:
        stateSuffix = ClosureUIStrings::STATE_SUFFIX_SETUP_REQUIRED;
        break;
    case chip::app::Clusters::ClosureControl::MainStateEnum::kUnknownEnumValue:
    default:
        stateSuffix = ClosureUIStrings::STATE_SUFFIX_UNKNOWN;
        break;
    }

    FormatText(sStateText, sizeof(sStateText), ClosureUIStrings::STATE_PREFIX, stateSuffix);
    GLIB_drawStringOnLine(glibContext, sStateText, STATE_DISPLAY_LINE, GLIB_ALIGN_CENTER, 0, 0, true);
}

void ClosureUI::DrawOverallCurrentState(GLIB_Context_t * glibContext)
{
    GLIB_drawStringOnLine(glibContext, sPositionText, POSITION_DISPLAY_LINE, GLIB_ALIGN_LEFT, 0, 0, true);

    GLIB_drawStringOnLine(glibContext, sLatchText, LATCH_DISPLAY_LINE, GLIB_ALIGN_LEFT, 0, 0, true);

    GLIB_drawStringOnLine(glibContext, sSecureText, SECURE_DISPLAY_LINE, GLIB_ALIGN_LEFT, 0, 0, true);

    GLIB_drawStringOnLine(glibContext, sSpeedText, SPEED_DISPLAY_LINE, GLIB_ALIGN_LEFT, 0, 0, true);
}

void ClosureUI::DrawFooter(GLIB_Context_t * glibContext)
{
    GLIB_drawStringOnLine(glibContext, ClosureUIStrings::FOOTER_TEXT, FOOTER_DISPLAY_LINE, GLIB_ALIGN_CENTER, 0, 0, true);
}

void ClosureUI::FormatAndSetPosition(const char * suffix)
{
    const char * safeSuffix = (suffix != nullptr) ? suffix : ClosureUIStrings::POSITION_SUFFIX_UNKNOWN;
    FormatText(sPositionText, sizeof(sPositionText), ClosureUIStrings::POSITION_PREFIX, safeSuffix);
}

void ClosureUI::FormatAndSetLatch(const char * suffix)
{
    const char * safeSuffix = (suffix != nullptr) ? suffix : ClosureUIStrings::LATCH_SUFFIX_UNKNOWN;
    FormatText(sLatchText, sizeof(sLatchText), ClosureUIStrings::LATCH_PREFIX, safeSuffix);
}

void ClosureUI::FormatAndSetSecure(const char * suffix)
{
    const char * safeSuffix = (suffix != nullptr) ? suffix : ClosureUIStrings::SECURE_SUFFIX_UNKNOWN;
    FormatText(sSecureText, sizeof(sSecureText), ClosureUIStrings::SECURE_PREFIX, safeSuffix);
}

void ClosureUI::FormatAndSetSpeed(const char * suffix)
{
    const char * safeSuffix = (suffix != nullptr) ? suffix : ClosureUIStrings::SPEED_SUFFIX_UNKNOWN;
    FormatText(sSpeedText, sizeof(sSpeedText), ClosureUIStrings::SPEED_PREFIX, safeSuffix);
}
