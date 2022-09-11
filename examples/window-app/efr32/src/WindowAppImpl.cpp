/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include <AppConfig.h>
#include <WindowAppImpl.h>
#include <app-common/zap-generated/attributes/Accessors.h>
#include <app/clusters/window-covering-server/window-covering-server.h>
#include <app/server/OnboardingCodesUtil.h>
#include <lib/core/CHIPError.h>
#include <lib/dnssd/Advertiser.h>
#include <lib/support/CodeUtils.h>
#include <platform/CHIPDeviceLayer.h>
#ifdef QR_CODE_ENABLED
#include <qrcodegen.h>
#endif // QR_CODE_ENABLED
#include <sl_simple_button_instances.h>
#include <sl_simple_led_instances.h>
#include <sl_system_kernel.h>

#ifdef SL_WIFI
#include "wfx_host_events.h"
#include <app/clusters/network-commissioning/network-commissioning.h>
#include <platform/EFR32/NetworkCommissioningWiFiDriver.h>
#endif

#ifdef DISPLAY_ENABLED
#include <LcdPainter.h>
SilabsLCD slLCD;
#endif

#define APP_TASK_STACK_SIZE (4096)
#define APP_TASK_PRIORITY 2
#define APP_EVENT_QUEUE_SIZE 10
#define EXAMPLE_VENDOR_ID 0xcafe

#define LCD_ICON_TIMEOUT 1000

using namespace chip::app::Clusters::WindowCovering;
#define APP_STATE_LED &sl_led_led0
#define APP_ACTION_LED &sl_led_led1

#ifdef SL_WIFI
chip::app::Clusters::NetworkCommissioning::Instance
    sWiFiNetworkCommissioningInstance(0 /* Endpoint Id */, &(chip::DeviceLayer::NetworkCommissioning::SlWiFiDriver::GetInstance()));
#endif
//------------------------------------------------------------------------------
// Timers
//------------------------------------------------------------------------------

WindowAppImpl::Timer::Timer(const char * name, uint32_t timeoutInMs, Callback callback, void * context) :
    WindowApp::Timer(name, timeoutInMs, callback, context)
{
    mHandler = xTimerCreate(name,          // Just a text name, not used by the RTOS kernel
                            timeoutInMs,   // == default timer period (mS)
                            false,         // no timer reload (==one-shot)
                            (void *) this, // init timer id = app task obj context
                            TimerCallback  // timer callback handler
    );
    if (mHandler == NULL)
    {
        EFR32_LOG("Timer create failed");
        appError(CHIP_ERROR_INTERNAL);
    }
}

void WindowAppImpl::Timer::Start()
{
    if (xTimerIsTimerActive(mHandler))
    {
        Stop();
    }

    // Timer is not active
    if (xTimerStart(mHandler, 100) != pdPASS)
    {
        EFR32_LOG("Timer start() failed");
        appError(CHIP_ERROR_INTERNAL);
    }

    mIsActive = true;
}

void WindowAppImpl::Timer::IsrStart()
{
    portBASE_TYPE taskWoken = pdFALSE; // For FreeRTOS timer (below).
    // Start/restart the button debounce timer (Note ISR version of FreeRTOS
    // api call here).
    xTimerStartFromISR(mHandler, &taskWoken);
    if (taskWoken != pdFALSE)
    {
        taskYIELD();
    }
    mIsActive = true;
}

void WindowAppImpl::Timer::Stop()
{
    mIsActive = false;
    if (xTimerStop(mHandler, 0) == pdFAIL)
    {
        EFR32_LOG("Timer stop() failed");
        appError(CHIP_ERROR_INTERNAL);
    }
}

void WindowAppImpl::Timer::TimerCallback(TimerHandle_t xTimer)
{
    Timer * timer = (Timer *) pvTimerGetTimerID(xTimer);
    if (timer)
    {
        timer->Timeout();
    }
}

//------------------------------------------------------------------------------
// Main Task
//------------------------------------------------------------------------------

StackType_t sAppStack[APP_TASK_STACK_SIZE / sizeof(StackType_t)];
StaticTask_t sAppTaskStruct;

uint8_t sAppEventQueueBuffer[APP_EVENT_QUEUE_SIZE * sizeof(WindowApp::Event)];
StaticQueue_t sAppEventQueueStruct;

WindowAppImpl WindowAppImpl::sInstance;

WindowApp & WindowApp::Instance()
{
    return WindowAppImpl::sInstance;
}

#ifdef DISPLAY_ENABLED
WindowAppImpl::WindowAppImpl() : mIconTimer("Timer:icon", LCD_ICON_TIMEOUT, OnIconTimeout, this) {}
#else
WindowAppImpl::WindowAppImpl() {}
#endif

void WindowAppImpl::OnTaskCallback(void * parameter)
{
#ifdef SL_WIFI
    /*
     * Wait for the WiFi to be initialized
     */
    EFR32_LOG("APP: Wait WiFi Init");
    while (!wfx_hw_ready())
    {
        vTaskDelay(10);
    }
    EFR32_LOG("APP: Done WiFi Init");
    /* We will init server when we get IP */
    sWiFiNetworkCommissioningInstance.Init();
    /* added for commisioning with wifi */
#endif

    sInstance.Run();
}

void WindowAppImpl::OnIconTimeout(WindowApp::Timer & timer)
{
#ifdef DISPLAY_ENABLED
    sInstance.mIcon = LcdIcon::None;
    sInstance.UpdateLCD();
#endif
}

CHIP_ERROR WindowAppImpl::Init()
{
    WindowApp::Init();

    // Initialize App Task
    mHandle = xTaskCreateStatic(OnTaskCallback, APP_TASK_NAME, ArraySize(sAppStack), NULL, 1, sAppStack, &sAppTaskStruct);
    if (NULL == mHandle)
    {
        EFR32_LOG("Failed to allocate app task");
        return CHIP_ERROR_NO_MEMORY;
    }

    // Initialize App Queue
    mQueue = xQueueCreateStatic(APP_EVENT_QUEUE_SIZE, sizeof(WindowApp::Event), sAppEventQueueBuffer, &sAppEventQueueStruct);
    if (NULL == mQueue)
    {
        EFR32_LOG("Failed to allocate app event queue");
        return CHIP_ERROR_NO_MEMORY;
    }

    // Initialize LEDs
    LEDWidget::InitGpio();
    mStatusLED.Init(APP_STATE_LED);
    mActionLED.Init(APP_ACTION_LED);

#ifdef DISPLAY_ENABLED
    slLCD.Init();
#endif

    return CHIP_NO_ERROR;
}

CHIP_ERROR WindowAppImpl::Start()
{
    EFR32_LOG("Starting FreeRTOS scheduler");
    sl_system_kernel_start();

    return CHIP_NO_ERROR;
}

void WindowAppImpl::Finish()
{
    WindowApp::Finish();
    chip::Platform::MemoryShutdown();
    // Should never get here.
    EFR32_LOG("vTaskStartScheduler() failed");
    appError(CHIP_ERROR_INTERNAL);
}

void WindowAppImpl::PostEvent(const WindowApp::Event & event)
{
    if (mQueue)
    {
        BaseType_t status;
        if (xPortIsInsideInterrupt())
        {
            BaseType_t higherPrioTaskWoken = pdFALSE;
            status                         = xQueueSendFromISR(mQueue, &event, &higherPrioTaskWoken);

#ifdef portYIELD_FROM_ISR
            portYIELD_FROM_ISR(higherPrioTaskWoken);
#elif portEND_SWITCHING_ISR // portYIELD_FROM_ISR or portEND_SWITCHING_ISR
            portEND_SWITCHING_ISR(higherPrioTaskWoken);
#else                       // portYIELD_FROM_ISR or portEND_SWITCHING_ISR
#error "Must have portYIELD_FROM_ISR or portEND_SWITCHING_ISR"
#endif // portYIELD_FROM_ISR or portEND_SWITCHING_ISR
        }
        else
        {
            status = xQueueSend(mQueue, &event, 1);
        }

        if (!status)
        {
            EFR32_LOG("Failed to post event to app task event queue");
        }
    }
}

void WindowAppImpl::PostAttributeChange(chip::EndpointId endpoint, chip::AttributeId attributeId)
{
    Instance().PostEvent(WindowApp::Event(WindowApp::EventId::AttributeChange, endpoint, attributeId));
}

void WindowAppImpl::ProcessEvents()
{
    WindowApp::Event event = EventId::None;

    BaseType_t received = xQueueReceive(mQueue, &event, pdMS_TO_TICKS(10));
    while (pdTRUE == received)
    {
        DispatchEvent(event);
        received = xQueueReceive(mQueue, &event, 0);
    }
}

WindowApp::Timer * WindowAppImpl::CreateTimer(const char * name, uint32_t timeoutInMs, WindowApp::Timer::Callback callback,
                                              void * context)
{
    return new Timer(name, timeoutInMs, callback, context);
}

WindowApp::Button * WindowAppImpl::CreateButton(WindowApp::Button::Id id, const char * name)
{
    return new Button(id, name);
}

void WindowAppImpl::DispatchEventAttributeChange(chip::EndpointId endpoint, chip::AttributeId attribute)
{
    switch (attribute)
    {
    /* RO OperationalStatus */
    case Attributes::OperationalStatus::Id:
        UpdateLEDs();
        break;
    /* RO Type: not supposed to dynamically change -> Cycling Window Covering Demo */
    case Attributes::Type::Id:
    /* ============= Positions for Position Aware ============= */
    case Attributes::CurrentPositionLiftPercent100ths::Id:
    case Attributes::CurrentPositionTiltPercent100ths::Id:
        UpdateLCD();
        break;
    /* ### ATTRIBUTEs CHANGEs IGNORED ### */
    /* RO EndProductType: not supposed to dynamically change */
    case Attributes::EndProductType::Id:
    /* RO ConfigStatus: set by WC server */
    case Attributes::ConfigStatus::Id:
    /* RO SafetyStatus: set by WC server */
    case Attributes::SafetyStatus::Id:
    /* RW Mode: User can change */
    case Attributes::Mode::Id:
    default:
        break;
    }
}

void WindowAppImpl::DispatchEvent(const WindowApp::Event & event)
{
    WindowApp::DispatchEvent(event);
    switch (event.mId)
    {
    case EventId::AttributeChange:
        DispatchEventAttributeChange(event.mEndpoint, event.mAttributeId);
        break;
    case EventId::ResetWarning:
        EFR32_LOG("Factory Reset Triggered. Release button within %ums to cancel.", LONG_PRESS_TIMEOUT);
        // Turn off all LEDs before starting blink to make sure blink is
        // co-ordinated.
        UpdateLEDs();
        break;
    case EventId::ResetCanceled:
        EFR32_LOG("Factory Reset has been Canceled");
        UpdateLEDs();
        break;
    case EventId::ProvisionedStateChanged:
        UpdateLEDs();
        UpdateLCD();
        break;

    case EventId::WinkOn:
    case EventId::WinkOff:
        mState.isWinking = (EventId::WinkOn == event.mId);
        UpdateLEDs();
        break;
    case EventId::ConnectivityStateChanged:
    case EventId::BLEConnectionsChanged:
        UpdateLEDs();
        break;
#ifdef DISPLAY_ENABLED
    case EventId::CoverTypeChange:
        UpdateLCD();
        break;
    case EventId::CoverChange:
        mIconTimer.Start();
        mIcon = (GetCover().mEndpoint == 1) ? LcdIcon::One : LcdIcon::Two;
        UpdateLCD();
        break;
    case EventId::TiltModeChange:
        mIconTimer.Start();
        mIcon = mTiltMode ? LcdIcon::Tilt : LcdIcon::Lift;
        UpdateLCD();
        break;
#endif
    default:
        break;
    }
}

void WindowAppImpl::UpdateLEDs()
{
    Cover & cover = GetCover();
    if (mResetWarning)
    {
        mStatusLED.Set(false);
        mStatusLED.Blink(500);

        mActionLED.Set(false);
        mActionLED.Blink(500);
    }
    else
    {
        if (mState.isWinking)
        {
            mStatusLED.Blink(200, 200);
        }
        else
#if CHIP_ENABLE_OPENTHREAD
            if (mState.isThreadProvisioned && mState.isThreadEnabled)
#else
            if (mState.isWiFiProvisioned && mState.isWiFiEnabled)
#endif

        {
            mStatusLED.Blink(950, 50);
        }
        else if (mState.haveBLEConnections) { mStatusLED.Blink(100, 100); }
        else { mStatusLED.Blink(50, 950); }

        // Action LED
        NPercent100ths current;
        LimitStatus liftLimit = LimitStatus::Intermediate;

        chip::DeviceLayer::PlatformMgr().LockChipStack();
        Attributes::CurrentPositionLiftPercent100ths::Get(cover.mEndpoint, current);
        chip::DeviceLayer::PlatformMgr().UnlockChipStack();

        if (!current.IsNull())
        {
            AbsoluteLimits limits = { .open = WC_PERCENT100THS_MIN_OPEN, .closed = WC_PERCENT100THS_MAX_CLOSED };
            liftLimit             = CheckLimitState(current.Value(), limits);
        }

        if (OperationalState::Stall != cover.mLiftOpState)
        {
            mActionLED.Blink(100);
        }
        else if (LimitStatus::IsUpOrOpen == liftLimit)
        {
            mActionLED.Set(true);
        }
        else if (LimitStatus::IsDownOrClose == liftLimit)
        {
            mActionLED.Set(false);
        }
        else
        {
            mActionLED.Blink(1000);
        }
    }
}

void WindowAppImpl::UpdateLCD()
{
    // Update LCD
#ifdef DISPLAY_ENABLED
#if CHIP_ENABLE_OPENTHREAD
    if (mState.isThreadProvisioned)
#else
    if (mState.isWiFiProvisioned)
#endif // CHIP_ENABLE_OPENTHREAD
    {
        Cover & cover = GetCover();
        chip::app::DataModel::Nullable<uint16_t> lift;
        chip::app::DataModel::Nullable<uint16_t> tilt;

        chip::DeviceLayer::PlatformMgr().LockChipStack();
        Type type = TypeGet(cover.mEndpoint);

        Attributes::CurrentPositionLift::Get(cover.mEndpoint, lift);
        Attributes::CurrentPositionTilt::Get(cover.mEndpoint, tilt);
        chip::DeviceLayer::PlatformMgr().UnlockChipStack();

#ifdef DISPLAY_ENABLED
        if (!tilt.IsNull() && !lift.IsNull())
        {
            LcdPainter::Paint(slLCD, type, lift.Value(), tilt.Value(), mIcon);
        }
#endif
    }
#ifdef QR_CODE_ENABLED
    else
    {
        chip::MutableCharSpan qrCode(mQRCodeBuffer);
        if (GetQRCode(qrCode, chip::RendezvousInformationFlags(chip::RendezvousInformationFlag::kBLE)) == CHIP_NO_ERROR)
        {
            slLCD.SetQRCode((uint8_t *) qrCode.data(), qrCode.size());
            slLCD.ShowQRCode(true, true);
        }
    }
#endif // QR_CODE_ENABLED
#endif // DISPLAY_ENABLED
}

void WindowAppImpl::OnMainLoop()
{
    mStatusLED.Animate();
    mActionLED.Animate();
}

//------------------------------------------------------------------------------
// Buttons
//------------------------------------------------------------------------------
WindowAppImpl::Button::Button(WindowApp::Button::Id id, const char * name) : WindowApp::Button(id, name) {}

void WindowAppImpl::OnButtonChange(const sl_button_t * handle)
{
    WindowApp::Button * btn = static_cast<Button *>((handle == &sl_button_btn0) ? sInstance.mButtonUp : sInstance.mButtonDown);

    if (sl_button_get_state(handle) == SL_SIMPLE_BUTTON_PRESSED)
    {
        btn->Press();
    }
    else
    {
        btn->Release();
    }
}

// Silabs button callback from button event ISR
void sl_button_on_change(const sl_button_t * handle)
{
    WindowAppImpl * app = static_cast<WindowAppImpl *>(&WindowAppImpl::sInstance);
    app->OnButtonChange(handle);
}
