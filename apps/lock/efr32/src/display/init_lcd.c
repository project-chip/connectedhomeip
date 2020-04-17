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

#include "init_lcd.h"
#include "sample_qr_code.h"
#include "display.h"
#include "dmd.h"
#include "glib.h"

static GLIB_Context_t glibContext;

void initLCD(void)
{

    EMSTATUS status;

    /* Initialize the display module. */
    status = DISPLAY_Init();
    if (DISPLAY_EMSTATUS_OK != status)
    {
        EFR32_LOG("Display init failed %d", status);
    }

    /* Initialize the DMD module for the DISPLAY device driver. */
    status = DMD_init(0);
    if (DMD_OK != status)
    {
        EFR32_LOG("DMD init failed %d", status);
    }

    /* Initialize the glib context */
    status = GLIB_contextInit(&glibContext);
    if (GLIB_OK != status)
    {
        EFR32_LOG("Glib context init failed %d", status);
    }

    glibContext.backgroundColor = White;
    glibContext.foregroundColor = Black;
    status                      = GLIB_clear(&glibContext);
    if (GLIB_OK != status)
    {
        EFR32_LOG("Glib clear failed %d", status);
    }
    // Draw the QR Code
    char str[MAX_STR_LEN];
    GLIB_drawBitmap(&glibContext, 0, 0, SAMPLE_QR_CODE_WIDTH, SAMPLE_QR_CODE_HEIGHT, sample_qr_code_bits);
    DMD_updateDisplay();
}
