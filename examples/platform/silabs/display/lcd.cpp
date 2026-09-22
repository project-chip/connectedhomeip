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
#include <lib/support/CodeUtils.h>
#include <platform/PlatformError.h>

#include "dmd.h"
#include "glib.h"

#if defined(SLI_SI91X_MCU_INTERFACE) && (SLI_SI91X_MCU_INTERFACE)
#include "sl_memlcd.h"
#endif

#include "sl_board_control.h"

#define LCD_SIZE 128
#define QR_CODE_VERSION 4
#define QR_CODE_MODULE_SIZE 3
#define QR_CODE_BORDER_SIZE 0
#define SL_BOARD_ENABLE_DISPLAY_PIN 0

#if SL_MATTER_QR_CODE_ENABLED
#include "qrcodegen.h"

static uint8_t qrCode[qrcodegen_BUFFER_LEN_FOR_VERSION(QR_CODE_VERSION)];
static uint8_t workBuffer[qrcodegen_BUFFER_LEN_FOR_VERSION(QR_CODE_VERSION)];
#endif // SL_MATTER_QR_CODE_ENABLED

CHIP_ERROR SilabsLCD::Init(uint8_t * name, bool initialState)
{
    EMSTATUS status;
    CHIP_ERROR err = CHIP_NO_ERROR;

    // Check if Name is to long
    if (name != nullptr)
    {
        if (APP_NAME_MAX_LENGTH < strlen((char *) name))
        {
            SILABS_LOG("App Name too long");
            return CHIP_ERROR_INVALID_ARGUMENT;
        }
        else
        {
            strcpy((char *) mName, (char *) name);
        }
    }

    /* Enable the memory lcd */
#if defined(SLI_SI91X_MCU_INTERFACE) && (SLI_SI91X_MCU_INTERFACE)
    sl_memlcd_display_enable();
#else
    status = sl_board_enable_display();
    if (status != SL_STATUS_OK)
    {
        SILABS_LOG("Board Display enable fail %lx", status);
        err = MATTER_PLATFORM_ERROR(status);
    }
#endif

    /* Initialize the DMD module for the DISPLAY device driver. */
    status = DMD_init(0);
    if (DMD_OK != status)
    {
        SILABS_LOG("DMD init failed %lx", status);
        err = MATTER_PLATFORM_ERROR(status);
    }

    /* Initialize the glib context */
    status = GLIB_contextInit(&glibContext);
    if (GLIB_OK != status)
    {
        SILABS_LOG("Glib context init failed %lx", status);
        err = MATTER_PLATFORM_ERROR(status);
    }

    glibContext.backgroundColor = White;
    glibContext.foregroundColor = Black;
    status                      = GLIB_clear(&glibContext);
    if (GLIB_OK != status)
    {
        SILABS_LOG("Glib clear failed %lx", status);
        err = MATTER_PLATFORM_ERROR(status);
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
    return updateDisplay();
}

void SilabsLCD::WriteDemoUI(bool state)
{
    dState.mainState = state;
    // Device pages suppress the demo screen entirely.
    if (mDevicePageCount > 0)
    {
        SetScreen(StatusScreen);
        return;
    }
    if (mCurrentScreen != DemoScreen)
    {
        mCurrentScreen = DemoScreen;
    }
    WriteDemoUI();
}

void SilabsLCD::WriteDemoUI()
{
    if (mDevicePageCount > 0)
    {
        SetScreen(StatusScreen);
        return;
    }
    Clear();
    if (customUI != nullptr)
    {
        customUI(&glibContext);
    }
    else
    {
        demoUIClearMainScreen(mName);
        demoUIDisplayApp(dState.mainState);
    }
}

void SilabsLCD::WriteStatus()
{
    uint8_t lineNb = 0;
    char str[20];

    GLIB_clear(&glibContext);
    sprintf(str, "# Fabrics : %d", mStatus.nbFabric);
    GLIB_drawStringOnLine(&glibContext, str, lineNb++, GLIB_ALIGN_LEFT, 0, 0, true);

    if (strlen(mStatus.networkName) >= sizeof(str))
    {
        memcpy(str, mStatus.networkName, sizeof(str) - 1);
        str[sizeof(str) - 1] = '\0';
    }
    else
    {
        memcpy(str, mStatus.networkName, sizeof(str));
    }

#if defined(SL_WIFI) && SL_WIFI
    GLIB_drawStringOnLine(&glibContext, "SSID : ", lineNb++, GLIB_ALIGN_LEFT, 0, 0, true);
    GLIB_drawStringOnLine(&glibContext, str, lineNb++, GLIB_ALIGN_LEFT, 0, 0, true);
#else
    GLIB_drawStringOnLine(&glibContext, "PANID : ", lineNb, GLIB_ALIGN_LEFT, 0, 0, true);
    GLIB_drawStringOnLine(&glibContext, str, lineNb++, GLIB_ALIGN_LEFT, 64, 0, true);
#if CHIP_DEVICE_CONFIG_THREAD_FTD
    GLIB_drawStringOnLine(&glibContext, "OT Type : FTD", lineNb++, GLIB_ALIGN_LEFT, 0, 0, true);
#else
    GLIB_drawStringOnLine(&glibContext, "OT Type : MTD", lineNb++, GLIB_ALIGN_LEFT, 0, 0, true);
#endif // FTD
#endif
    GLIB_drawStringOnLine(&glibContext, "", lineNb++, GLIB_ALIGN_LEFT, 0, 0, true);
    sprintf(str, "Connected : %c", mStatus.connected ? 'Y' : 'N');
    GLIB_drawStringOnLine(&glibContext, str, lineNb++, GLIB_ALIGN_LEFT, 0, 0, true);
    sprintf(str, "Advertising : %c", mStatus.advertising ? 'Y' : 'N');
    GLIB_drawStringOnLine(&glibContext, str, lineNb++, GLIB_ALIGN_LEFT, 0, 0, true);
    if (mStatus.icdMode != NotICD)
    {
        sprintf(str, "ICD : %s", mStatus.icdMode == SIT ? "SIT" : "LIT");
        GLIB_drawStringOnLine(&glibContext, str, lineNb++, GLIB_ALIGN_LEFT, 0, 0, true);
    }

    updateDisplay();
}

void SilabsLCD::SetCustomUI(customUICB cb)
{
    customUI = cb;
}

void SilabsLCD::GetScreen(Screen_e & screen)
{
    screen = static_cast<Screen_e>(mCurrentScreen);
}

void SilabsLCD::SetScreen(Screen_e screen)
{
    if (screen >= InvalidScreen)
    {
        return;
    }

    // Suppress the demo screen entirely once device pages are registered.
    if (screen == DemoScreen && mDevicePageCount > 0)
    {
        screen = StatusScreen;
    }

    switch (screen)
    {
    case DemoScreen:
        WriteDemoUI();
        break;
    case StatusScreen:
        WriteStatus();
        break;
#if SL_MATTER_QR_CODE_ENABLED
    case QRCodeScreen:
        WriteQRCode();
        break;
#endif
    case DevicePageScreen:
        if (mDevicePageCount == 0)
        {
            // No device pages registered; stay on the demo screen.
            WriteDemoUI();
            mCurrentScreen = DemoScreen;
            return;
        }
        WriteDevicePage(mCurrentDevicePage);
        break;
    default:
        break;
    }
    mCurrentScreen = screen;
}

void SilabsLCD::CycleScreens(void)
{
    // Advance through: [Demo ->] Status -> [QRCode] -> DevicePages... -> wrap.
    // The demo screen is skipped entirely when at least one device page is registered.
    const bool hasDevicePages = mDevicePageCount > 0;
    const Screen_e kWrapScreen = hasDevicePages ? StatusScreen : DemoScreen;

    if (mCurrentScreen == DevicePageScreen)
    {
        if (mCurrentDevicePage + 1 < mDevicePageCount)
        {
            mCurrentDevicePage++;
            SetScreen(DevicePageScreen);
            return;
        }
        mCurrentDevicePage = 0;
        SetScreen(kWrapScreen);
        return;
    }

    constexpr uint8_t kLastBuiltInScreen =
#if SL_MATTER_QR_CODE_ENABLED
        QRCodeScreen;
#else
        StatusScreen;
#endif

    if (mCurrentScreen < kLastBuiltInScreen)
    {
        Screen_e next = static_cast<Screen_e>(mCurrentScreen + 1);
        if (hasDevicePages && next == DemoScreen)
        {
            next = StatusScreen;
        }
        SetScreen(next);
        return;
    }

    // Past the last built-in screen: enter device pages if any, otherwise wrap.
    if (hasDevicePages)
    {
        mCurrentDevicePage = 0;
        SetScreen(DevicePageScreen);
    }
    else
    {
        SetScreen(DemoScreen);
    }
}

void SilabsLCD::SetStatus(DisplayStatus_t & status)
{
    mStatus = status;
}

CHIP_ERROR SilabsLCD::RegisterDevicePage(chip::EndpointId endpointId, const char * typeName, DevicePageDrawCB cb,
                                        void * userContext, DevicePageButtonCB buttonCb)
{
    VerifyOrReturnError(cb != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(mDevicePageCount < mDevicePages.size(), CHIP_ERROR_NO_MEMORY);

    mDevicePages[mDevicePageCount] = DevicePage{ endpointId, typeName, cb, userContext, buttonCb };
    mDevicePageCount++;
    return CHIP_NO_ERROR;
}

bool SilabsLCD::DispatchButtonToCurrentDevicePage()
{
    if (mCurrentScreen != DevicePageScreen || mCurrentDevicePage >= mDevicePageCount)
    {
        return false;
    }
    const DevicePage & page = mDevicePages[mCurrentDevicePage];
    if (page.button == nullptr)
    {
        return false;
    }
    page.button(page.endpointId, page.userContext);
    return true;
}

void SilabsLCD::WriteDevicePage(uint8_t index)
{
    if (index >= mDevicePageCount)
    {
        return;
    }
    const DevicePage & page = mDevicePages[index];
    if (page.draw == nullptr)
    {
        return;
    }
    GLIB_clear(&glibContext);
    page.draw(&glibContext, page.endpointId, page.userContext);
    updateDisplay();
}

#if SL_MATTER_QR_CODE_ENABLED
void SilabsLCD::WriteQRCode()
{
    if (!qrcodegen_encodeText((const char *) mQRCodeBuffer, workBuffer, qrCode, qrcodegen_Ecc_LOW, QR_CODE_VERSION, QR_CODE_VERSION,
                              qrcodegen_Mask_AUTO, true))
    {
        SILABS_LOG("qrcodegen_encodeText() failed");
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
    SilabsLCD::Update();
}

void SilabsLCD::SetQRCode(uint8_t * str, uint32_t size)
{
    if (size < chip::QRCodeBasicSetupPayloadGenerator::kMaxQRCodeBase38RepresentationLength + 1)
    {
        memcpy(mQRCodeBuffer, str, size);
    }
}

void SilabsLCD::ShowQRCode(bool show)
{
    if (mCurrentScreen != QRCodeScreen)
    {
        mCurrentScreen = QRCodeScreen;
    }

    WriteQRCode();
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
#endif // SL_MATTER_QR_CODE_ENABLED
