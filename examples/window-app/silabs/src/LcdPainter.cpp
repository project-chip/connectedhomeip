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

#include "demo-ui-bitmaps.h"
#include <LcdPainter.h>
#include <AppConfig.h>
#include <cstdint>
#include <cstdio>

using namespace chip::app::Clusters::WindowCovering;

SilabsLCD * LcdPainter::sLcd       = nullptr;
bool LcdPainter::mLcdCleared      = false;

namespace {
static const uint8_t sSilabsLogoSmall[] = { SILABS_LOGO_SMALL };
static const uint8_t sMatterLogo[]      = { MATTER_LOGO_BITMAP };

static unsigned RawToWholePercent(uint16_t raw, int openLimit, int closedLimit)
{
    const int denom = closedLimit - openLimit;
    if (denom == 0)
    {
        return 0;
    }
    return static_cast<unsigned>((static_cast<int>(raw) - openLimit) * 100 / denom);
}
} // namespace

LcdPainter::LcdPainter(SilabsLCD & lcd)
{
    sLcd = &lcd;
}

void LcdPainter::ClearScreen()
{
    if (mLcdCleared || sLcd == nullptr)
    {
        return;
    }
    sLcd->Clear();
    sLcd->Update();
    mLcdCleared = true;
}

void LcdPainter::Paint(Type type, uint16_t lift, uint16_t tilt, LcdIcon icon)
{
    (void) type;
    (void) icon;

    if (sLcd == nullptr)
    {
        return;
    }

    const bool drawStaticChrome = !mLcdCleared;
    ClearScreen();

    const unsigned liftPct = RawToWholePercent(lift, LIFT_OPEN_LIMIT, LIFT_CLOSED_LIMIT);
    const unsigned tiltPct = RawToWholePercent(tilt, TILT_OPEN_LIMIT, TILT_CLOSED_LIMIT);

    GLIB_Context_t * glibContext = static_cast<GLIB_Context_t *>(sLcd->Context());
    const int32_t xSize          = glibContext->pDisplayGeometry->xSize;

    char buf[32];
    if (drawStaticChrome)
    {
        const int32_t silabsX = (xSize - SILABS_LOGO_WIDTH) / 2;
        GLIB_drawBitmap(glibContext, silabsX, 0, SILABS_LOGO_WIDTH, SILABS_LOGO_HEIGHT, sSilabsLogoSmall);
        GLIB_drawBitmap(glibContext, MATTER_ICON_POSITION_X, 4, MATTER_LOGO_WIDTH, MATTER_LOGO_HEIGHT, sMatterLogo);

        GLIB_drawStringOnLine(glibContext, "Window App", 4, GLIB_ALIGN_CENTER, 0, 0, true);
        GLIB_drawStringOnLine(glibContext, "Lift Tilt", 6, GLIB_ALIGN_CENTER, 0, 0, true);
    }
    snprintf(buf, sizeof(buf), "%3u%% %3u%%", liftPct, tiltPct);
    GLIB_drawStringOnLine(glibContext, buf, 8, GLIB_ALIGN_CENTER, 0, 0, true);
    sLcd->Update();
}
