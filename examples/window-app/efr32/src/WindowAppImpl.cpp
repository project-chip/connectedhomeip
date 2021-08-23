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
#include <LcdPainter.h>
#include <WindowAppImpl.h>
#include <app/clusters/window-covering-server/window-covering-server.h>
#include <app/server/OnboardingCodesUtil.h>
#include <core/CHIPError.h>
#include <lcd.h>
#include <lib/mdns/Advertiser.h>
#include <lib/support/CodeUtils.h>
#include <platform/CHIPDeviceLayer.h>
#include <qrcodegen.h>

#define APP_TASK_STACK_SIZE (4096)
#define APP_TASK_PRIORITY 2
#define APP_EVENT_QUEUE_SIZE 10
#define EXAMPLE_VENDOR_ID 0xcafe

#define FACTORY_RESET_TRIGGER_TIMEOUT 3000
#define WINDOW_COVER_TYPE_CYCLE_TIMEOUT 3000
#define LCD_ICON_TIMEOUT 1000

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
        EFR32_LOG("Timer already started!");
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
// Buttons
//------------------------------------------------------------------------------

const WindowAppImpl::Button::Config WindowAppImpl::Button::sEfr32Configs[BSP_BUTTON_COUNT] = BSP_BUTTON_INIT;

WindowAppImpl::Button::Button(WindowApp::Button::Id id, const char * name) :
    WindowApp::Button(id, name), mTimer(name, APP_BUTTON_DEBOUNCE_PERIOD_MS, OnButtonTimeout, this)
{
    const Button::Config & config = Button::GetConfig(id);
    mPort                         = config.port;
    mPin                          = config.pin;
    GPIO_PinModeSet(mPort, mPin, gpioModeInputPull, 1);
    GPIO_IntConfig(mPort, mPin, true, true, true);
    GPIOINT_CallbackRegister(mPin, OnButtonInterrupt);
}

const WindowAppImpl::Button::Config & WindowAppImpl::Button::GetConfig(Button::Id id)
{
    return Button::Id::Up == id ? sEfr32Configs[0] : sEfr32Configs[1];
}

void WindowAppImpl::Button::OnButtonInterrupt(uint8_t pin)
{
    const Button::Config & up = Button::GetConfig(Button::Id::Up);
    Button * btn              = static_cast<Button *>(up.pin == pin ? sInstance.mButtonUp : sInstance.mButtonDown);
    btn->mIsPressed           = !GPIO_PinInGet(btn->mPort, btn->mPin);
    btn->mTimer.IsrStart();
}

void WindowAppImpl::Button::OnButtonTimeout(WindowApp::Timer & timer)
{
    Button * btn = static_cast<Button *>(timer.mContext);
    if (btn->mIsPressed)
    {
        btn->Press();
    }
    else
    {
        btn->Release();
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

WindowAppImpl::WindowAppImpl() : mIconTimer("Timer:icon", LCD_ICON_TIMEOUT, OnIconTimeout, this) {}

void WindowAppImpl::OnTaskCallback(void * parameter)
{
    sInstance.Run();
}

void WindowAppImpl::OnIconTimeout(WindowApp::Timer & timer)
{
    sInstance.UpdateLCD();
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

    // Initialize Buttons
    GPIOINT_Init();
    NVIC_SetPriority(GPIO_EVEN_IRQn, 5);
    NVIC_SetPriority(GPIO_ODD_IRQn, 5);

    // Initialize LEDs
    LEDWidget::InitGpio();
    mStatusLED.Init(APP_STATE_LED);
    mActionLED.Init(APP_ACTION_LED);

    // Print setup info on LCD if available
    UpdateLCD();

    return CHIP_NO_ERROR;
}

CHIP_ERROR WindowAppImpl::Start()
{
    EFR32_LOG("Starting FreeRTOS scheduler");
    vTaskStartScheduler();
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
        if (!xQueueSend(mQueue, &event, 1))
        {
            EFR32_LOG("Failed to post event to app task event queue");
        }
    }
}

void WindowAppImpl::ProcessEvents()
{
    WindowApp::Event event = WindowApp::Event::None;

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

void WindowAppImpl::DispatchEvent(const WindowApp::Event & event)
{
    WindowApp::DispatchEvent(event);
    switch (event)
    {
    case WindowApp::Event::ResetWarning:
        EFR32_LOG("Factory Reset Triggered. Release button within %ums to cancel.", FACTORY_RESET_WINDOW_TIMEOUT);
        // Turn off all LEDs before starting blink to make sure blink is
        // co-ordinated.
        UpdateLEDs();
        break;
    case WindowApp::Event::ResetCanceled:
        EFR32_LOG("Factory Reset has been Canceled");
        UpdateLEDs();
        break;
    case WindowApp::Event::ProvisionedStateChanged:
        UpdateLEDs();
        UpdateLCD();
        break;
    case WindowApp::Event::ConnectivityStateChanged:
    case WindowApp::Event::BLEConnectionsChanged:
        UpdateLEDs();
        break;
    case WindowApp::Event::CoverTypeChange:
    case WindowApp::Event::LiftChanged:
    case WindowApp::Event::TiltChanged:
        UpdateLCD();
        break;
    case WindowApp::Event::TiltModeChange:
        mIconTimer.Start();
        UpdateLCD();
        break;
    default:
        break;
    }
}

void WindowAppImpl::UpdateLEDs()
{
    if (mState.resetWarning)
    {
        mStatusLED.Set(false);
        mStatusLED.Blink(500);

        mActionLED.Set(false);
        mActionLED.Blink(500);
    }
    else
    {
        // Consider the system to be "fully connected" if it has service
        // connectivity
        if (mState.haveServiceConnectivity)
        {
            mStatusLED.Set(true);
        }
        else if (mState.isThreadProvisioned && mState.isThreadEnabled)
        {
            mStatusLED.Blink(950, 50);
        }
        else if (mState.haveBLEConnections)
        {
            mStatusLED.Blink(100, 100);
        }
        else
        {
            mStatusLED.Blink(50, 950);
        }

        // Action LED
        WindowCover & cover = WindowCover::Instance();

        if (WindowApp::Event::None != mLiftAction || WindowApp::Event::None != mTiltAction)
        {
            mActionLED.Blink(100);
        }
        else if (cover.IsOpen())
        {
            mActionLED.Set(true);
        }
        else if (cover.IsClosed())
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
    if (mState.isThreadProvisioned)
    {
        WindowCover & cover = WindowCover::Instance();
        LcdIcon icon        = LcdIcon::None;
        if (mIconTimer.mIsActive)
        {
            icon = mState.tiltMode ? LcdIcon::Tilt : LcdIcon::Lift;
        }
        uint16_t lift = cover.Lift().PositionValueGet();
        uint16_t tilt = cover.Tilt().PositionValueGet();
        LcdPainter::Paint(cover.TypeGet(), (uint8_t) lift, (uint8_t) tilt, icon);
    }
    else
    {
        LCDWriteQRCode((uint8_t *) mQRCode.c_str());
    }
#endif
}

void WindowAppImpl::OnMainLoop()
{
    mStatusLED.Animate();
    mActionLED.Animate();
}
