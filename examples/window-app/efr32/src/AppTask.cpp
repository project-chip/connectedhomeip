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
#include <AppTask.h>
#include <LcdPainter.h>
#include <Service.h>
#include <app/common/gen/attribute-id.h>
#include <app/common/gen/attribute-type.h>
#include <app/common/gen/cluster-id.h>
#include <app/server/OnboardingCodesUtil.h>
#include <app/server/Server.h>
#include <app/util/attribute-storage.h>
#include <lcd.h>
#include <qrcodegen.h>

#include <assert.h>
#include <lib/support/CodeUtils.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/EFR32/freertos_bluetooth.h>
#include <setup_payload/QRCodeSetupPayloadGenerator.h>
#include <setup_payload/SetupPayload.h>

#if CHIP_ENABLE_OPENTHREAD
#include <platform/EFR32/ThreadStackManagerImpl.h>
#include <platform/OpenThread/OpenThreadUtils.h>
#include <platform/ThreadStackManager.h>
#endif

#define APP_TASK_STACK_SIZE (1536)
#define APP_TASK_PRIORITY 2
#define APP_EVENT_QUEUE_SIZE 10
#define EXAMPLE_VENDOR_ID 0xcafe

#define FACTORY_RESET_TRIGGER_TIMEOUT 3000
#define FACTORY_RESET_CANCEL_WINDOW_TIMEOUT 3000
#define WINDOW_COVER_TYPE_CYCLE_TIMEOUT 3000
#define LCD_ICON_TIMEOUT 1000

namespace {

uint8_t sAppEventQueueBuffer[APP_EVENT_QUEUE_SIZE * sizeof(AppEvent)];
StaticQueue_t sAppEventQueueStruct;

StackType_t sAppStack[APP_TASK_STACK_SIZE / sizeof(StackType_t)];
StaticTask_t sAppTaskStruct;
} // namespace
using namespace chip::TLV;
using namespace ::chip::DeviceLayer;

AppTask AppTask::sInstance;

AppTask & AppTask::Instance()
{
    return sInstance;
}

WindowCover & AppTask::Cover()
{
    return mCover;
}

int AppTask::Start()
{
    int err = CHIP_CONFIG_CORE_ERROR_MAX;

    mQueue = xQueueCreateStatic(APP_EVENT_QUEUE_SIZE, sizeof(AppEvent), sAppEventQueueBuffer, &sAppEventQueueStruct);
    if (mQueue == NULL)
    {
        EFR32_LOG("Failed to allocate app event queue");
        appError(err);
    }

    // Start App task.
    mHandle = xTaskCreateStatic(Main, APP_TASK_NAME, ArraySize(sAppStack), NULL, 1, sAppStack, &sAppTaskStruct);
    if (mHandle != NULL)
    {
        err = CHIP_NO_ERROR;
    }
    return err;
}

void AppTask::Main(void * pvParameter)
{
    AppTask & app             = AppTask::sInstance;
    uint64_t lastChangeTimeUS = 0;
    int err;

    err = app.Init();
    if (err != CHIP_NO_ERROR)
    {
        appError(err);
    }

    EFR32_LOG("App Task started");
    SetDeviceName("EFR32WindowCoverDemo._matter._udp.local.");

    while (true)
    {
        //
        // Dispatch events
        //
        app.DispatchEvents();

        // Collect connectivity and configuration state from the CHIP stack. Because
        // the CHIP event loop is being run in a separate task, the stack must be
        // locked while these values are queried.  However we use a non-blocking
        // lock request (TryLockCHIPStack()) to avoid blocking other UI activities
        // when the CHIP task is busy (e.g. with a long crypto operation).
        if (PlatformMgr().TryLockChipStack())
        {
            app.mIsThreadProvisioned     = ConnectivityMgr().IsThreadProvisioned();
            app.mIsThreadEnabled         = ConnectivityMgr().IsThreadEnabled();
            app.mHaveBLEConnections      = (ConnectivityMgr().NumBLEConnections() != 0);
            app.mHaveServiceConnectivity = ConnectivityMgr().HaveServiceConnectivity();
            PlatformMgr().UnlockChipStack();
        }

        if (app.mLastThreadProvisioned != app.mIsThreadProvisioned)
        {
            // Provisioned state changed
            app.mLastThreadProvisioned = app.mIsThreadProvisioned;
            app.UpdateLcd(AppEvent::EventType::None);
        }

        // Update the status LED if factory reset has not been initiated.
        //
        // If system has "full connectivity", keep the LED On constantly.
        //
        // If thread and service provisioned, but not attached to the thread network
        // yet OR no connectivity to the service OR subscriptions are not fully
        // established THEN blink the LED Off for a short period of time.
        //
        // If the system has ble connection(s) uptill the stage above, THEN blink
        // the LEDs at an even rate of 100ms.
        //
        // Otherwise, blink the LED ON for a very short time.
        if (!app.mResetWarning)
        {
            // Consider the system to be "fully connected" if it has service
            // connectivity
            if (app.mHaveServiceConnectivity)
            {
                app.mStatusLED.Set(true);
            }
            else if (app.mIsThreadProvisioned && app.mIsThreadEnabled)
            {
                app.mStatusLED.Blink(950, 50);
            }
            else if (app.mHaveBLEConnections)
            {
                app.mStatusLED.Blink(100, 100);
            }
            else
            {
                app.mStatusLED.Blink(50, 950);
            }
        }

        app.mStatusLED.Animate();
        app.mActionLED.Animate();

        uint64_t nowUS            = chip::System::Platform::Layer::GetClock_Monotonic();
        uint64_t nextChangeTimeUS = lastChangeTimeUS + 5 * 1000 * 1000UL;

        if (nowUS > nextChangeTimeUS)
        {
            PublishService();
            lastChangeTimeUS = nowUS;
        }
    }
}

int AppTask::Init()
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    // Init ZCL Data Model
    InitServer();

    // Initialize LEDs
    LEDWidget::InitGpio();
    mStatusLED.Init(APP_STATE_LED);
    mActionLED.Init(APP_ACTION_LED);

    // Initialise Buttons
    ButtonHandler::Instance().Init();

    // Timers
    mIconTimer.Init(this, "Timer:icon", LCD_ICON_TIMEOUT, IconTimerCallback);
    mCoverTypeTimer.Init(this, "Timer:cover-type", WINDOW_COVER_TYPE_CYCLE_TIMEOUT, CoverTypeTimerCallback);
    mResetTimer.Init(this, "Timer:reset", FACTORY_RESET_CANCEL_WINDOW_TIMEOUT + FACTORY_RESET_TRIGGER_TIMEOUT, ResetTimerCallback);

    ConfigurationMgr().LogDeviceConfig();

// Print setup info on LCD if available
#ifdef DISPLAY_ENABLED
    if (!GetQRCode(mQRCode, chip::RendezvousInformationFlags(chip::RendezvousInformationFlag::kBLE)) == CHIP_NO_ERROR)
    {
        EFR32_LOG("Getting QR code failed!");
    }
#else
    PrintQRCode(chip::RendezvousInformationFlags(chip::RendezvousInformationFlag::kBLE));
#endif
    mResetWarning = false;
    // Force LCD refresh
    mLastThreadProvisioned = !ConnectivityMgr().IsThreadProvisioned();

    return err;
}

void AppTask::PostEvent(const AppEvent & event)
{
    if (mQueue)
    {
        if (!xQueueSend(mQueue, &event, 1))
        {
            EFR32_LOG("Failed to post event to app task event queue");
        }
    }
}

void AppTask::DispatchEvents()
{
    AppEvent event;
    BaseType_t received = xQueueReceive(mQueue, &event, pdMS_TO_TICKS(10));
    while (received == pdTRUE)
    {
        switch (event.mType)
        {
        case AppEvent::EventType::ButtonPressed:
        case AppEvent::EventType::ButtonReleased:
            DispatchButtonEvent(event.mType, event.mContext);
            break;
        case AppEvent::EventType::CoverTypeChange:
        case AppEvent::EventType::CoverTiltModeChange:
        case AppEvent::EventType::CoverLiftUp:
        case AppEvent::EventType::CoverLiftDown:
        case AppEvent::EventType::CoverTiltUp:
        case AppEvent::EventType::CoverTiltDown:
        case AppEvent::EventType::CoverOpen:
        case AppEvent::EventType::CoverClosed:
        case AppEvent::EventType::CoverStart:
        case AppEvent::EventType::CoverStop:
            DispatchWindowCoverEvent(event.mType, event.mContext);
            break;
        default:
            break;
        }
        received = xQueueReceive(mQueue, &event, 0);
    }
}

void AppTask::DispatchButtonEvent(AppEvent::EventType type, void * context)
{
    if (context)
    {
        if (type == AppEvent::EventType::ButtonPressed)
        {
            ButtonHandler::Button * pressedButton = (ButtonHandler::Button *) context;
            ButtonHandler::Button * otherButton   = nullptr;
            if (pressedButton->mId == ButtonHandler::ButtonId::kButton_Up)
            {
                // Step UP
                otherButton = &ButtonHandler::Instance().mButtonDown;
                if (otherButton->mIsPressed)
                {
                    // Both buttons pressed at the same time
                    mCover.ToggleTiltMode();
                }
                else
                {
                    mCover.StepUp();
                    mResetTimer.Start(FACTORY_RESET_TRIGGER_TIMEOUT);
                }
            }
            else
            {
                // Step DOWN
                otherButton = &ButtonHandler::Instance().mButtonUp;
                if (otherButton->mIsPressed)
                {
                    // Both buttons pressed at the same time
                    mCover.ToggleTiltMode();
                }
                else
                {
                    mCover.StepDown();
                    mCoverTypeTimer.Start();
                }
            }
        }
        else
        {
            mCoverTypeTimer.Stop();
            mResetTimer.Stop();
            if (mResetWarning)
            {
                EFR32_LOG("Factory Reset has been Canceled");
            }
            mResetWarning = false;
        }
    }
}

void AppTask::DispatchWindowCoverEvent(AppEvent::EventType event, void * context)
{
    UpdateLog(event);
    UpdateLed(event);
    UpdateLcd(event);
    UpdateClusterState(event);
}

void AppTask::UpdateLog(AppEvent::EventType event)
{
    EFR32_LOG("Window Covering: %s, lift[%u..%u]:%u(%u%%), tilt[%u..%u]:%d(%d%%)", AppEvent::TypeString(event),
              mCover.LiftOpenLimitGet(), mCover.LiftClosedLimitGet(), mCover.LiftGet(), mCover.LiftPercentGet(),
              mCover.TiltOpenLimitGet(), mCover.TiltClosedLimitGet(), mCover.TiltGet(), mCover.TiltPercentGet());
}

void AppTask::UpdateLed(AppEvent::EventType event)
{
    if (mCover.IsMoving())
    {
        mActionLED.Blink(100);
    }
    else if (mCover.IsOpen())
    {
        mActionLED.Set(true);
    }
    else if (mCover.IsClosed())
    {
        mActionLED.Set(false);
    }
    else
    {
        mActionLED.Blink(1000);
    }
}

void AppTask::UpdateLcd(AppEvent::EventType event)
{
    // Update LCD
#ifdef DISPLAY_ENABLED
    if (mIsThreadProvisioned)
    {
        LcdIcon icon = LcdIcon::None;
        if (event == AppEvent::EventType::CoverTiltModeChange)
        {
            icon = mCover.TiltModeGet() ? LcdIcon::Tilt : LcdIcon::Lift;
            mIconTimer.Start();
        }
        LcdPainter::Paint(mCover.TypeGet(), mCover.LiftGet(), mCover.TiltGet(), icon);
    }
    else
    {
        LCDWriteQRCode((uint8_t *) mQRCode.c_str());
    }
#endif
}

void AppTask::UpdateClusterState(AppEvent::EventType event)
{
    EmberAfStatus status = EMBER_ZCL_STATUS_SUCCESS;
    switch (event)
    {
    // WindowCoveringType
    case AppEvent::EventType::CoverStatusChange: {
        uint8_t config = mCover.StatusGet();
        status = emberAfWriteAttribute(1, ZCL_WINDOW_COVERING_CLUSTER_ID, ZCL_WC_CONFIG_STATUS_ATTRIBUTE_ID, CLUSTER_MASK_SERVER,
                                       (uint8_t *) &config, ZCL_BITMAP8_ATTRIBUTE_TYPE);
        break;
    }

    // WindowCoveringType
    case AppEvent::EventType::CoverTypeChange: {
        uint8_t type = static_cast<uint8_t>(mCover.TypeGet());
        status       = emberAfWriteAttribute(1, ZCL_WINDOW_COVERING_CLUSTER_ID, ZCL_WC_TYPE_ATTRIBUTE_ID, CLUSTER_MASK_SERVER,
                                       (uint8_t *) &type, ZCL_INT8U_ATTRIBUTE_TYPE);
        break;
    }

    // CurrentPosition – Lift
    case AppEvent::EventType::CoverLiftUp:
    case AppEvent::EventType::CoverLiftDown: {
        uint16_t lift = mCover.LiftGet();
        status        = emberAfWriteAttribute(1, ZCL_WINDOW_COVERING_CLUSTER_ID, ZCL_WC_CURRENT_POSITION_LIFT_ATTRIBUTE_ID,
                                       CLUSTER_MASK_SERVER, (uint8_t *) &lift, ZCL_INT16U_ATTRIBUTE_TYPE);
        break;
    }

    // Current Position – Tilt
    case AppEvent::EventType::CoverTiltUp:
    case AppEvent::EventType::CoverTiltDown: {
        uint16_t tilt = mCover.TiltGet();
        status        = emberAfWriteAttribute(1, ZCL_WINDOW_COVERING_CLUSTER_ID, ZCL_WC_CURRENT_POSITION_TILT_ATTRIBUTE_ID,
                                       CLUSTER_MASK_SERVER, (uint8_t *) &tilt, ZCL_INT16U_ATTRIBUTE_TYPE);
        break;
    }

    default:
        break;
    }

    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        EFR32_LOG("ERR: updating ZCL %x", status);
    }
}

void AppTask::IconTimerCallback(AppTimer & timer, void * context)
{
    AppTask::Instance().UpdateLcd(AppEvent::EventType::None);
}

void AppTask::CoverTypeTimerCallback(AppTimer & timer, void * context)
{
    AppTask::Instance().Cover().TypeCycle();
    if (ButtonHandler::Instance().mButtonDown.mIsPressed)
    {
        // The button is still pressed, keep cycling;
        timer.Start();
    }
}

void AppTask::ResetTimerCallback(AppTimer & timer, void * context)
{
    AppTask & app = AppTask::Instance();

    if (app.mResetWarning)
    {
        // You have been warned!
        ConfigurationMgr().InitiateFactoryReset();
    }
    else
    {
        EFR32_LOG("Factory Reset Triggered. Release button within %ums to cancel.", FACTORY_RESET_CANCEL_WINDOW_TIMEOUT);

        // Start timer for FACTORY_RESET_CANCEL_WINDOW_TIMEOUT to allow user to
        // cancel, if required.
        app.mResetTimer.Start(FACTORY_RESET_CANCEL_WINDOW_TIMEOUT);

        // Turn off all LEDs before starting blink to make sure blink is
        // co-ordinated.
        app.mStatusLED.Set(false);
        app.mActionLED.Set(false);
        app.mStatusLED.Blink(500);
        app.mActionLED.Blink(500);
        app.mResetWarning = true;
    }
}
