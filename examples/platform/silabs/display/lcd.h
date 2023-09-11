/*
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "AppConfig.h"
#include "glib.h"
#ifdef QR_CODE_ENABLED
#include "qrcodegen.h"
#include <setup_payload/QRCodeSetupPayloadGenerator.h>
#endif // QR_CODE_ENABLED

#include "demo-ui.h"

#define MAX_STR_LEN 48

class SilabsLCD
{

public:
    typedef void (*customUICB)(GLIB_Context_t * context);
    CHIP_ERROR Init(uint8_t * name = nullptr, bool initialState = false);
    void * Context();
    int Clear(void);
    int DrawPixel(void * pContext, int32_t x, int32_t y);
    int Update(void);
    void WriteDemoUI(bool state);
    void SetCustomUI(customUICB cb);

#ifdef QR_CODE_ENABLED
    void SetQRCode(uint8_t * str, uint32_t size);
    void ShowQRCode(bool show, bool forceRefresh = false);
    void ToggleQRCode(void);
#endif

private:
    typedef struct demoState
    {
        bool mainState = false;
        bool protocol1 = false; /* data */
    } DemoState_t;

    void WriteQRCode();
    void WriteDemoUI();
#ifdef QR_CODE_ENABLED
    void LCDFillRect(uint8_t x, uint8_t y, uint8_t w, uint8_t h);
    char mQRCodeBuffer[chip::QRCodeBasicSetupPayloadGenerator::kMaxQRCodeBase38RepresentationLength + 1];
    bool mShowQRCode = true;
#endif
    GLIB_Context_t glibContext;

#ifdef SL_DEMO_NAME
    uint8_t mName[] = SL_DEMO_NAME
#else
    uint8_t mName[APP_NAME_MAX_LENGTH + 1];
#endif
        customUICB customUI = nullptr;
    DemoState_t dState;
};
