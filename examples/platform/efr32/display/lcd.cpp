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

#include "demo-ui.h"
#include "lcd.h"

#include "dmd.h"
#include "glib.h"

#ifdef QR_CODE_ENABLED
#include "qrcodegen.h"
#endif // QR_CODE_ENABLED

#include "sl_board_control.h"

#define LCD_SIZE 128
#define QR_CODE_VERSION 4
#define QR_CODE_MODULE_SIZE 3
#define QR_CODE_BORDER_SIZE 0

#ifdef QR_CODE_ENABLED
static uint8_t qrCode[qrcodegen_BUFFER_LEN_FOR_VERSION(QR_CODE_VERSION)];
static uint8_t workBuffer[qrcodegen_BUFFER_LEN_FOR_VERSION(QR_CODE_VERSION)];
#endif // QR_CODE_ENABLED

CHIP_ERROR SilabsLCD::Init(uint8_t * name, bool initialState)
{
    EMSTATUS status;
    CHIP_ERROR err = CHIP_NO_ERROR;

    // Check if Name is to long
    if (name != nullptr)
    {
        if (APP_NAME_MAX_LENGTH < strlen((char *) name))
        {
            EFR32_LOG("App Name too long");
            return CHIP_ERROR_INVALID_ARGUMENT;
        }
        else
        {
            strcpy((char *) mName, (char *) name);
        }
    }

    /* Enable the memory lcd */
    status = sl_board_enable_display();
    if (status != SL_STATUS_OK)
    {
        EFR32_LOG("Board Display enable fail %d", status);
        err = CHIP_ERROR_INTERNAL;
    }

    /* Initialize the DMD module for the DISPLAY device driver. */
    status = DMD_init(0);
    if (DMD_OK != status)
    {
        EFR32_LOG("DMD init failed %d", status);
        err = CHIP_ERROR_INTERNAL;
    }

    /* Initialize the glib context */
    status = GLIB_contextInit(&glibContext);
    if (GLIB_OK != status)
    {
        EFR32_LOG("Glib context init failed %d", status);
        err = CHIP_ERROR_INTERNAL;
    }

    glibContext.backgroundColor = White;
    glibContext.foregroundColor = Black;
    status                      = GLIB_clear(&glibContext);
    if (GLIB_OK != status)
    {
        EFR32_LOG("Glib clear failed %d", status);
        err = CHIP_ERROR_INTERNAL;
    }
    demoUIInit(&glibContext);

    dState.mainState = initialState;

    return err;
}

/* This function is necessary because currently glib.h cannot be used within a C++ context. */
void * SilabsLCD::Context()
{
    return (void *) &glibContext;
}

int SilabsLCD::Clear()
{
    return GLIB_clear(&glibContext);
}

int SilabsLCD::DrawPixel(void * pContext, int32_t x, int32_t y)
{
    return GLIB_drawPixel((GLIB_Context_t *) pContext, x, y);
}

int SilabsLCD::Update(void)
{
    return DMD_updateDisplay();
}

void SilabsLCD::WriteDemoUI(bool state)
{
#ifdef QR_CODE_ENABLED
    if (mShowQRCode)
    {
        mShowQRCode = false;
    }
#endif
    dState.mainState = state;
    WriteDemoUI();
}

void SilabsLCD::WriteDemoUI()
{
    Clear();
    demoUIClearMainScreen(mName);
    demoUIDisplayApp(dState.mainState);
}

#ifdef QR_CODE_ENABLED
void SilabsLCD::WriteQRCode()
{
    if (!qrcodegen_encodeText((const char *) mQRCodeBuffer, workBuffer, qrCode, qrcodegen_Ecc_LOW, QR_CODE_VERSION, QR_CODE_VERSION,
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

void SilabsLCD::SetQRCode(uint8_t * str, uint32_t size)
{
    if (size < chip::QRCodeBasicSetupPayloadGenerator::kMaxQRCodeBase38RepresentationLength + 1)
    {
        memcpy(mQRCodeBuffer, str, size);
    }
}

void SilabsLCD::ShowQRCode(bool show, bool forceRefresh)
{
    if (show != mShowQRCode || forceRefresh)
    {
        (show) ? WriteQRCode() : WriteDemoUI();
        mShowQRCode = show;
    }
}

void SilabsLCD::ToggleQRCode(void)
{
    (mShowQRCode) ? WriteDemoUI() : WriteQRCode();
    mShowQRCode = !mShowQRCode;
}

void SilabsLCD::LCDFillRect(uint8_t x, uint8_t y, uint8_t w, uint8_t h)
{
    for (int i = 0; i < h; i++)
    {
        for (int j = 0; j < w; j++)
        {
            GLIB_drawPixel(&glibContext, x + j, y + i);
        }
    }
}
#endif // QR_CODE_ENABLED
