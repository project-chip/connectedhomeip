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

#include <AppConfig.h>
#include <LcdPainter.h>
#include <WindowAppImpl.h>
#include <lcd.h>
#include <qrcodegen.h>

#include <app/clusters/window-covering-server/window-covering-server.h>
#include <app/server/OnboardingCodesUtil.h>
#include <core/CHIPError.h>
#include <lib/mdns/Advertiser.h>
#include <lib/support/CodeUtils.h>
#include <platform/CHIPDeviceLayer.h>

#define APP_TASK_STACK_SIZE (1536)
#define APP_TASK_PRIORITY 2
#define APP_EVENT_QUEUE_SIZE 10
#define EXAMPLE_VENDOR_ID 0xcafe

#define FACTORY_RESET_TRIGGER_TIMEOUT 3000
#define WINDOW_COVER_TYPE_CYCLE_TIMEOUT 3000
#define LCD_ICON_TIMEOUT 1000

using namespace ::chip::DeviceLayer;

namespace example {

WindowApp & WindowApp::Instance()
{
    return example::efr32::WindowApp::sInstance;
}

} // namespace example

namespace example::efr32 {

//------------------------------------------------------------------------------
// Timers
//------------------------------------------------------------------------------

Timer::Timer(const char * name, uint32_t timeoutInMs, Callback callback, void * context) :
    example::Timer(name, timeoutInMs, callback, context)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    mHandler = xTimerCreate(name,          // Just a text name, not used by the RTOS kernel
                            timeoutInMs,   // == default timer period (mS)
                            false,         // no timer reload (==one-shot)
                            (void *) this, // init timer id = app task obj context
                            TimerCallback  // timer callback handler
    );
    if (mHandler == NULL)
    {
        EFR32_LOG("Timer create failed");
        appError(err);
    }
}

void Timer::Start()
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
        appError(CHIP_CONFIG_CORE_ERROR_MAX);
    }

    mIsActive = true;
}

void Timer::IsrStart()
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

void Timer::Stop()
{
    mIsActive = false;
    if (xTimerStop(mHandler, 0) == pdFAIL)
    {
        EFR32_LOG("Timer stop() failed");
        appError(CHIP_CONFIG_CORE_ERROR_MAX);
    }
}

void Timer::TimerCallback(TimerHandle_t xTimer)
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

static const WindowApp::Button::ButtonConfig sButtonsConfig[BSP_BUTTON_COUNT] = BSP_BUTTON_INIT;

WindowApp::Button::Button(const char * name, Timer::Callback callback) : mTimer(name, APP_BUTTON_DEBOUNCE_PERIOD_MS, callback, this)
{}

void WindowApp::Button::Init(const Button::ButtonConfig & config)
{
    mPort = config.port;
    mPin  = config.pin;
    GPIO_PinModeSet(mPort, mPin, gpioModeInputPull, 1);
    GPIO_IntConfig(mPort, mPin, true, true, true);
    GPIOINT_CallbackRegister(mPin, OnInterrupt);
}

void WindowApp::Button::OnInterrupt(uint8_t pin)
{
    Button & btn   = (sInstance.mButtonUp.mPin == pin) ? sInstance.mButtonUp : sInstance.mButtonDown;
    btn.mIsPressed = !GPIO_PinInGet(btn.mPort, btn.mPin);
    btn.mTimer.IsrStart();
}

void WindowApp::Button::OnUpTimeout(example::Timer & timer)
{
    sInstance.PostEvent(sInstance.mButtonUp.mIsPressed ? WindowEvent::UpPressed : WindowEvent::UpReleased);
}

void WindowApp::Button::OnDownTimeout(example::Timer & timer)
{
    sInstance.PostEvent(sInstance.mButtonDown.mIsPressed ? WindowEvent::DownPressed : WindowEvent::DownReleased);
}

//------------------------------------------------------------------------------
// Main Task
//------------------------------------------------------------------------------

uint8_t sAppEventQueueBuffer[APP_EVENT_QUEUE_SIZE * sizeof(WindowEvent)];
StaticQueue_t sAppEventQueueStruct;

StackType_t sAppStack[APP_TASK_STACK_SIZE / sizeof(StackType_t)];
StaticTask_t sAppTaskStruct;

WindowApp WindowApp::sInstance;

// WindowApp & WindowApp::Instance()
// {
//     return sInstance;
// }

void WindowApp::TaskCallback(void * parameter)
{
    WindowApp::sInstance.Run();
}

WindowApp::WindowApp() :
    mButtonUp("Button:Up", Button::OnUpTimeout), mButtonDown("Button:Down", Button::OnDownTimeout),
    mIconTimer("Timer:icon", LCD_ICON_TIMEOUT, OnIconTimeout, this)
{}

CHIP_ERROR WindowApp::Init()
{
    example::WindowApp::Init();

    mQueue = xQueueCreateStatic(APP_EVENT_QUEUE_SIZE, sizeof(WindowEvent), sAppEventQueueBuffer, &sAppEventQueueStruct);
    if (NULL == mQueue)
    {
        EFR32_LOG("Failed to allocate app event queue");
        return CHIP_ERROR_NO_MEMORY;
    }

    // Start App task.
    mHandle = xTaskCreateStatic(TaskCallback, APP_TASK_NAME, ArraySize(sAppStack), NULL, 1, sAppStack, &sAppTaskStruct);
    if (NULL == mHandle)
    {
        EFR32_LOG("Failed to allocate app task");
        return CHIP_ERROR_NO_MEMORY;
    }

    // Initialize Buttons
    GPIOINT_Init();
    NVIC_SetPriority(GPIO_EVEN_IRQn, 5);
    NVIC_SetPriority(GPIO_ODD_IRQn, 5);
    mButtonUp.Init(sButtonsConfig[0]);
    mButtonDown.Init(sButtonsConfig[1]);

    // Initialize LEDs
    LEDWidget::InitGpio();
    mStatusLED.Init(APP_STATE_LED);
    mActionLED.Init(APP_ACTION_LED);

    // Initialize Actuators
    WindowCover::Actuator & lift = WindowCover::Instance().Lift();
    WindowCover::Actuator & tilt = WindowCover::Instance().Tilt();
    lift.OpenLimitSet(LIFT_OPEN_LIMIT);
    lift.ClosedLimitSet(LIFT_CLOSED_LIMIT);
    lift.PositionSet(LIFT_CLOSED_LIMIT, WindowCover::PositionUnits::Value);
    tilt.OpenLimitSet(TILT_OPEN_LIMIT);
    tilt.ClosedLimitSet(TILT_CLOSED_LIMIT);
    tilt.PositionSet(TILT_CLOSED_LIMIT, WindowCover::PositionUnits::Value);

    // Print setup info on LCD if available
    UpdateLCD();

    return CHIP_NO_ERROR;
}

void WindowApp::PostEvent(const WindowEvent & event)
{
    if (mQueue)
    {
        if (!xQueueSend(mQueue, &event, 1))
        {
            EFR32_LOG("Failed to post event to app task event queue");
        }
    }
}

void WindowApp::ProcessEvents()
{
    WindowEvent event = WindowEvent::None;

    BaseType_t received = xQueueReceive(mQueue, &event, pdMS_TO_TICKS(10));
    while (pdTRUE == received)
    {
        DispatchEvent(event);
        received = xQueueReceive(mQueue, &event, 0);
    }
}

example::Timer * WindowApp::CreateTimer(const char * name, uint32_t timeoutInMs, example::Timer::Callback callback, void * context)
{
    return new Timer(name, timeoutInMs, callback, context);
}

void WindowApp::DestroyTimer(example::Timer * timer)
{
    if (timer)
    {
        delete timer;
    }
}

void WindowApp::OnMainLoop()
{
    mStatusLED.Animate();
    mActionLED.Animate();
}

void WindowApp::OnCoverTypeChanged()
{
    sInstance.UpdateLCD();
}

void WindowApp::OnLiftChanged()
{
    UpdateLCD();
}

void WindowApp::OnTiltChanged()
{
    UpdateLCD();
}

void WindowApp::OnTiltModeChanged()
{
    mIconTimer.Start();
    sInstance.UpdateLCD();
}

void WindowApp::OnProvisionedStateChanged(bool isProvisioned)
{
    UpdateLEDs();
    UpdateLCD();
}

void WindowApp::OnConnectivityStateChanged(bool isConnected)
{
    UpdateLEDs();
}

void WindowApp::OnBLEConnectionsChanged(bool haveConnections)
{
    UpdateLEDs();
}

void WindowApp::OnResetWarning()
{
    EFR32_LOG("Factory Reset Triggered. Release button within %ums to cancel.", FACTORY_RESET_WINDOW_TIMEOUT);
    // Turn off all LEDs before starting blink to make sure blink is
    // co-ordinated.
    UpdateLEDs();
}

void WindowApp::OnResetCanceled()
{
    EFR32_LOG("Factory Reset has been Canceled");
    UpdateLEDs();
}

void WindowApp::OnReset()
{
    ConfigurationMgr().InitiateFactoryReset();
}

void WindowApp::OnIconTimeout(example::Timer & timer)
{
    sInstance.UpdateLCD();
}

void WindowApp::UpdateLEDs()
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

        if (WindowEvent::None != mLiftAction || WindowEvent::None != mTiltAction)
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

void WindowApp::UpdateLCD()
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
        uint16_t lift = cover.Lift().PositionGet(WindowCover::PositionUnits::Value);
        uint16_t tilt = cover.Tilt().PositionGet(WindowCover::PositionUnits::Value);
        LcdPainter::Paint(cover.TypeGet(), (uint8_t) lift, (uint8_t) tilt, icon);
    }
    else
    {
        LCDWriteQRCode((uint8_t *) mQRCode.c_str());
    }
#endif
}

} // namespace example::efr32
