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
#if SL_MATTER_QR_CODE_ENABLED
#include "qrcodegen.h"
#include <setup_payload/QRCodeSetupPayloadGenerator.h>
#endif // SL_MATTER_QR_CODE_ENABLED

#include "demo-ui.h"
#include <array>
#include <lib/core/DataModelTypes.h>
#include <platform/internal/DeviceNetworkInfo.h>

class SilabsLCD
{

public:
    // Cycling order (see CycleScreens()):
    //   DemoScreen -> StatusScreen -> [QRCodeScreen] -> DevicePageScreen[0..N-1] -> DemoScreen ...
    // Once any device page is registered the demo screen is suppressed and the cycle becomes:
    //   StatusScreen -> [QRCodeScreen] -> DevicePageScreen[0..N-1] -> StatusScreen ...
    // DevicePageScreen is a single enum value; the specific page shown is
    // tracked internally by mCurrentDevicePage.
    typedef enum screen
    {
        DemoScreen = 0,
        StatusScreen,
#if SL_MATTER_QR_CODE_ENABLED
        QRCodeScreen,
#endif
        DevicePageScreen,
        CycleScreen,
        InvalidScreen,
    } Screen_e;

    // Maximum number of per-device UI pages that can be registered.
    // Bounded at compile time to avoid heap allocation on embedded targets.
    static constexpr size_t kMaxDevicePages = 8;

    // Draw callback for a per-device UI page. The callback is expected to
    // draw into the provided glib context; SilabsLCD clears the screen
    // before invoking it and updates the display afterwards.
    //   context     - glib drawing context
    //   endpointId  - endpoint the page is associated with
    //   userContext - opaque pointer passed at registration
    typedef void (*DevicePageDrawCB)(GLIB_Context_t * context, chip::EndpointId endpointId, void * userContext);

    // Optional action-button callback for a per-device UI page. Invoked when the user
    // presses the action button while this page is the currently displayed screen.
    // Pages without a handler (typical for read-only sensor UIs) simply pass nullptr.
    typedef void (*DevicePageButtonCB)(chip::EndpointId endpointId, void * userContext);

    typedef enum icdMode
    {
        NotICD = 0,
        SIT,
        LIT,
    } ICDMode_e;

    typedef struct dStatus
    {
        uint8_t nbFabric                                                  = 0;
        bool connected                                                    = false;
        char networkName[chip::DeviceLayer::Internal::kMaxWiFiSSIDLength] = { 0 };
        bool advertising                                                  = false;
        ICDMode_e icdMode                                                 = NotICD;
    } DisplayStatus_t;

    typedef void (*customUICB)(GLIB_Context_t * context);
    CHIP_ERROR Init(uint8_t * name = nullptr, bool initialState = false);
    void * Context();
    int Clear(void);
    int DrawPixel(void * pContext, int32_t x, int32_t y);
    int Update(void);
    void WriteDemoUI(bool state);
    void WriteDemoUI();
    void SetCustomUI(customUICB cb);

    void GetScreen(Screen_e & screen);
    void SetScreen(Screen_e screen);
    void CycleScreens(void);
    void SetStatus(DisplayStatus_t & status);
    void WriteStatus();

    // Register a UI page for a device on a specific endpoint. Pages are
    // cycled through (in registration order) after the built-in screens
    // when the user presses the display-cycle button.
    //
    // typeName must point to storage that outlives SilabsLCD (typically a
    // string literal or a persistent std::string). Returns CHIP_ERROR_NO_MEMORY
    // when kMaxDevicePages pages have already been registered.
    //
    // `buttonCb` is optional; when non-null it is invoked by
    // DispatchButtonToCurrentDevicePage() while this page is the currently
    // displayed screen.
    CHIP_ERROR RegisterDevicePage(chip::EndpointId endpointId, const char * typeName, DevicePageDrawCB cb,
                                  void * userContext = nullptr, DevicePageButtonCB buttonCb = nullptr);

    // Forwards an action-button press to the callback registered for the currently
    // displayed device page (if any). Returns true iff a handler was invoked.
    bool DispatchButtonToCurrentDevicePage();

    // Number of currently-registered device pages.
    uint8_t GetDevicePageCount() const { return mDevicePageCount; }

    // True iff the currently displayed screen is the device page registered for `endpointId`.
    // Useful for delegates that want to trigger a repaint only when their own page is active.
    bool IsCurrentDevicePageFor(chip::EndpointId endpointId) const
    {
        return (mCurrentScreen == DevicePageScreen) && (mCurrentDevicePage < mDevicePageCount) &&
            (mDevicePages[mCurrentDevicePage].endpointId == endpointId);
    }

#if SL_MATTER_QR_CODE_ENABLED
    void SetQRCode(uint8_t * str, uint32_t size);
    void ShowQRCode(bool show);
#endif

private:
    typedef struct demoState
    {
        bool mainState = false;
        bool protocol1 = false; /* data */
    } DemoState_t;

#if SL_MATTER_QR_CODE_ENABLED
    void WriteQRCode();
    void LCDFillRect(uint8_t x, uint8_t y, uint8_t w, uint8_t h);
    char mQRCodeBuffer[chip::QRCodeBasicSetupPayloadGenerator::kMaxQRCodeBase38RepresentationLength + 1];
#endif

    void WriteDevicePage(uint8_t index);

    struct DevicePage
    {
        chip::EndpointId endpointId = chip::kInvalidEndpointId;
        const char * typeName       = nullptr;
        DevicePageDrawCB draw       = nullptr;
        void * userContext          = nullptr;
        DevicePageButtonCB button   = nullptr;
    };

    std::array<DevicePage, kMaxDevicePages> mDevicePages{};
    uint8_t mDevicePageCount   = 0;
    uint8_t mCurrentDevicePage = 0;

    GLIB_Context_t glibContext;

#ifdef SL_DEMO_NAME
    uint8_t mName[] = SL_DEMO_NAME
#else
    uint8_t mName[APP_NAME_MAX_LENGTH + 1];
#endif
        customUICB customUI = nullptr;
    DemoState_t dState;

    DisplayStatus_t mStatus;
    uint8_t mCurrentScreen = DemoScreen;
};
