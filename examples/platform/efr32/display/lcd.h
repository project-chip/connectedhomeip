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
    CHIP_ERROR Init(uint8_t * name = nullptr, bool initialState = false);
    void * Context();
    int Clear(void);
    int DrawPixel(void * pContext, int32_t x, int32_t y);
    int Update(void);
    void WriteDemoUI(bool state);
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

        DemoState_t dState;
};
