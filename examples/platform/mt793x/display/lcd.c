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

#include "lcd.h"

#include "dmd.h"
#include "glib.h"
#include "qrcodegen.h"
#include "sl_board_control.h"

#define LCD_SIZE 128
#define QR_CODE_VERSION 4
#define QR_CODE_MODULE_SIZE 3
#define QR_CODE_BORDER_SIZE 0

static GLIB_Context_t glibContext;
static uint8_t qrCode[qrcodegen_BUFFER_LEN_FOR_VERSION(QR_CODE_VERSION)];
static uint8_t workBuffer[qrcodegen_BUFFER_LEN_FOR_VERSION(QR_CODE_VERSION)];

static void LCDFillRect(uint8_t x, uint8_t y, uint8_t w, uint8_t h);

void initLCD(void)
{
    EMSTATUS status;

    /* Enable the memory lcd */
    status = sl_board_enable_display();
    if (status == SL_STATUS_OK)
    {
        EFR32_LOG("Board Display enable fail %d", status);
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
}

/* This function is necessary because currently glib.h cannot be used within a C++ context. */
void * LCDContext()
{
    return (void *) &glibContext;
}

int LCD_clear(void * pContext)
{
    return GLIB_clear((GLIB_Context_t *) pContext);
}

int LCD_drawPixel(void * pContext, int32_t x, int32_t y)
{
    return GLIB_drawPixel((GLIB_Context_t *) pContext, x, y);
}

int LCD_update(void)
{
    return DMD_updateDisplay();
}

void LCDWriteQRCode(uint8_t * str)
{
    if (!qrcodegen_encodeText((const char *) str, workBuffer, qrCode, qrcodegen_Ecc_LOW, QR_CODE_VERSION, QR_CODE_VERSION,
                              qrcodegen_Mask_AUTO, true))
    {
        EFR32_LOG("qrcodegen_encodeText() failed");
        return;
    }

    const int size = qrcodegen_getSize(qrCode);

    GLIB_clear(&glibContext);

    const int displaySize = (2 * QR_CODE_BORDER_SIZE + size) * QR_CODE_MODULE_SIZE;
    const int displayX    = (LCD_SIZE - displaySize) / 2;
    const int displayY    = displayX;

    for (int y = 0; y < size; ++y)
    {
        for (int x = 0; x < size; ++x)
        {
            if (qrcodegen_getModule(qrCode, x, y))
            {
                LCDFillRect(displayX + (QR_CODE_BORDER_SIZE + x) * QR_CODE_MODULE_SIZE,
                            displayY + (QR_CODE_BORDER_SIZE + y) * QR_CODE_MODULE_SIZE, QR_CODE_MODULE_SIZE, QR_CODE_MODULE_SIZE);
            }
        }
    }

    DMD_updateDisplay();
}

void LCDFillRect(uint8_t x, uint8_t y, uint8_t w, uint8_t h)
{
    for (int i = 0; i < h; i++)
    {
        for (int j = 0; j < w; j++)
        {
            GLIB_drawPixel(&glibContext, x + j, y + i);
        }
    }
}
