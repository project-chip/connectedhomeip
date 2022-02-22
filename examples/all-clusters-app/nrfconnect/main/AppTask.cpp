/*
 *    Copyright (c) 2022 Project CHIP Authors
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

#include "AppTask.h"
#include "AppConfig.h"
#include "AppEvent.h"
#include "Utils.h"

#include <app/server/OnboardingCodesUtil.h>
#include <app/server/Server.h>

#include <app-common/zap-generated/attribute-id.h>
#include <app-common/zap-generated/attribute-type.h>
#include <app-common/zap-generated/cluster-id.h>
#include <app/util/attribute-storage.h>

#include <credentials/DeviceAttestationCredsProvider.h>
#include <credentials/examples/DeviceAttestationCredsExample.h>

#include <dk_buttons_and_leds.h>
#include <logging/log.h>
#include <zephyr.h>

#define FACTORY_RESET_TRIGGER_TIMEOUT 3000
#define FACTORY_RESET_CANCEL_WINDOW_TIMEOUT 3000
#define APP_EVENT_QUEUE_SIZE 10
#define BUTTON_PUSH_EVENT 1
#define BUTTON_RELEASE_EVENT 0

LOG_MODULE_DECLARE(app);
K_MSGQ_DEFINE(sAppEventQueue, sizeof(AppEvent), APP_EVENT_QUEUE_SIZE, alignof(AppEvent));

static LEDWidget sStatusLED;
static UnusedLedsWrapper<3> sUnusedLeds{ { DK_LED2, DK_LED3, DK_LED4 } };
static k_timer sFunctionTimer;

namespace LedConsts {
constexpr uint32_t kBlinkRate_ms{ 500 };
namespace StatusLed {
namespace Unprovisioned {
constexpr uint32_t kOn_ms{ 100 };
constexpr uint32_t kOff_ms{ kOn_ms };
} // namespace Unprovisioned
namespace Provisioned {
constexpr uint32_t kOn_ms{ 50 };
constexpr uint32_t kOff_ms{ 950 };
} // namespace Provisioned

} // namespace StatusLed
} // namespace LedConsts

using namespace ::chip::Credentials;
using namespace ::chip::DeviceLayer;

int AppTask::Init()
{
    // Initialize LEDs
    LEDWidget::InitGpio();
    LEDWidget::SetStateUpdateCallback(LEDStateUpdateHandler);

    sStatusLED.Init(SYSTEM_STATE_LED);

    UpdateStatusLED();

    // Initialize buttons
    auto ret = dk_buttons_init(ButtonEventHandler);
    if (ret)
    {
        LOG_ERR("dk_buttons_init() failed");
        return ret;
    }

    // Initialize timer user data
    k_timer_init(&sFunctionTimer, &AppTask::TimerEventHandler, nullptr);
    k_timer_user_data_set(&sFunctionTimer, this);

    // Init ZCL Data Model and start server
    chip::Server::GetInstance().Init();

    // Initialize device attestation config
    SetDeviceAttestationCredentialsProvider(Examples::GetExampleDACProvider());
    ConfigurationMgr().LogDeviceConfig();
    PrintOnboardingCodes(chip::RendezvousInformationFlag(chip::RendezvousInformationFlag::kBLE));
    PlatformMgr().AddEventHandler(ChipEventHandler, 0);

    InitOTARequestor();

    return 0;
}

void AppTask::InitOTARequestor()
{
#if CONFIG_CHIP_OTA_REQUESTOR
    mOTAImageProcessor.SetOTADownloader(&mBDXDownloader);
    mBDXDownloader.SetImageProcessorDelegate(&mOTAImageProcessor);
    mOTARequestorDriver.Init(&mOTARequestor, &mOTAImageProcessor);
    mOTARequestor.Init(&chip::Server::GetInstance(), &mOTARequestorDriver, &mBDXDownloader);
    chip::SetRequestorInstance(&mOTARequestor);
#endif
}

int AppTask::StartApp()
{
    auto ret = Init();

    if (ret)
    {
        LOG_ERR("AppTask.Init() failed");
        return ret;
    }

    AppEvent event{};

    while (true)
    {
        k_msgq_get(&sAppEventQueue, &event, K_FOREVER);
        DispatchEvent(&event);
    }
}

void AppTask::ButtonEventHandler(uint32_t aButtonState, uint32_t aHasChanged)
{
    AppEvent event;
    event.Type = AppEvent::Type::Button;

    if (FUNCTION_BUTTON_MASK & aHasChanged)
    {
        event.ButtonEvent.PinNo  = FUNCTION_BUTTON;
        event.ButtonEvent.Action = (FUNCTION_BUTTON_MASK & aButtonState) ? BUTTON_PUSH_EVENT : BUTTON_RELEASE_EVENT;
        event.Handler            = FunctionHandler;
        PostEvent(&event);
    }

    if (BLE_ADVERTISEMENT_START_BUTTON_MASK & aButtonState & aHasChanged)
    {
        event.ButtonEvent.PinNo  = BLE_ADVERTISEMENT_START_BUTTON;
        event.ButtonEvent.Action = BUTTON_PUSH_EVENT;
        event.Handler            = StartBLEAdvertisementHandler;
        PostEvent(&event);
    }
}

void AppTask::TimerEventHandler(k_timer * aTimer)
{
    if (!aTimer)
        return;

    AppEvent event;
    event.Type               = AppEvent::Type::Timer;
    event.TimerEvent.Context = k_timer_user_data_get(aTimer);
    event.Handler            = FunctionTimerEventHandler;
    PostEvent(&event);
}

void AppTask::FunctionTimerEventHandler(AppEvent * aEvent)
{
    if (!aEvent)
        return;
    if (aEvent->Type != AppEvent::Type::Timer)
        return;

    // If we reached here, the button was held past FACTORY_RESET_TRIGGER_TIMEOUT, initiate factory reset
    if (Instance().mFunctionTimerActive && Instance().mMode == OperatingMode::Normal)
    {
        LOG_INF("Factory Reset Triggered. Release button within %ums to cancel.", FACTORY_RESET_TRIGGER_TIMEOUT);

        // Start timer for FACTORY_RESET_CANCEL_WINDOW_TIMEOUT to allow user to cancel, if required.
        StartTimer(FACTORY_RESET_CANCEL_WINDOW_TIMEOUT);
        Instance().mMode = OperatingMode::FactoryReset;

#ifdef CONFIG_STATE_LEDS
        // Turn off all LEDs before starting blink to make sure blink is co-ordinated.
        sStatusLED.Set(false);
        sUnusedLeds.Set(false);

        sStatusLED.Blink(LedConsts::kBlinkRate_ms);
        sUnusedLeds.Blink(LedConsts::kBlinkRate_ms);
#endif
    }
    else if (Instance().mFunctionTimerActive && Instance().mMode == OperatingMode::FactoryReset)
    {
        // Actually trigger Factory Reset
        Instance().mMode = OperatingMode::Normal;
        ConfigurationMgr().InitiateFactoryReset();
    }
}

void AppTask::FunctionHandler(AppEvent * aEvent)
{
    if (!aEvent)
        return;
    if (aEvent->ButtonEvent.PinNo != FUNCTION_BUTTON)
        return;

    // To initiate factory reset: press the FUNCTION_BUTTON for FACTORY_RESET_TRIGGER_TIMEOUT + FACTORY_RESET_CANCEL_WINDOW_TIMEOUT
    // All LEDs start blinking after FACTORY_RESET_TRIGGER_TIMEOUT to signal factory reset has been initiated.
    // To cancel factory reset: release the FUNCTION_BUTTON once all LEDs start blinking within the
    // FACTORY_RESET_CANCEL_WINDOW_TIMEOUT
    if (aEvent->ButtonEvent.Action == BUTTON_PUSH_EVENT)
    {
        if (!Instance().mFunctionTimerActive && Instance().mMode == OperatingMode::Normal)
        {
            StartTimer(FACTORY_RESET_TRIGGER_TIMEOUT);
        }
    }
    else
    {
        if (Instance().mFunctionTimerActive && Instance().mMode == OperatingMode::FactoryReset)
        {
            sUnusedLeds.Set(false);

            UpdateStatusLED();
            CancelTimer();

            // Change the function to none selected since factory reset has been canceled.
            Instance().mMode = OperatingMode::Normal;

            LOG_INF("Factory Reset has been Canceled");
        }
        else if (Instance().mFunctionTimerActive)
        {
            CancelTimer();
            Instance().mMode = OperatingMode::Normal;
        }
    }
}

void AppTask::StartBLEAdvertisementHandler(AppEvent * aEvent)
{
    if (!aEvent)
        return;
    if (aEvent->ButtonEvent.PinNo != BLE_ADVERTISEMENT_START_BUTTON)
        return;

    // Don't allow on starting Matter service BLE advertising after Thread provisioning.
    if (ConnectivityMgr().IsThreadProvisioned())
    {
        LOG_INF("Matter service BLE advertising not started - device is commissioned to a Thread network.");
        return;
    }

    if (ConnectivityMgr().IsBLEAdvertisingEnabled())
    {
        LOG_INF("BLE advertising is already enabled");
        return;
    }

    if (chip::Server::GetInstance().GetCommissioningWindowManager().OpenBasicCommissioningWindow() != CHIP_NO_ERROR)
    {
        LOG_ERR("OpenBasicCommissioningWindow() failed");
    }
}

void AppTask::UpdateLedStateEventHandler(AppEvent * aEvent)
{
    if (!aEvent)
        return;
    if (aEvent->Type == AppEvent::Type::UpdateLedState)
    {
        aEvent->UpdateLedStateEvent.LedWidget->UpdateState();
    }
}

void AppTask::LEDStateUpdateHandler(LEDWidget & aLedWidget)
{
    AppEvent event;
    event.Type                          = AppEvent::Type::UpdateLedState;
    event.Handler                       = UpdateLedStateEventHandler;
    event.UpdateLedStateEvent.LedWidget = &aLedWidget;
    PostEvent(&event);
}

void AppTask::UpdateStatusLED()
{
#ifdef CONFIG_STATE_LEDS
    /* Update the status LED.
     *
     * If thread and service provisioned, keep the LED On constantly.
     *
     * If the system has ble connection(s) uptill the stage above, THEN blink the LED at an even
     * rate of 100ms.
     *
     * Otherwise, blink the LED On for a very short time. */
    if (Instance().mIsThreadProvisioned && Instance().mIsThreadEnabled)
    {
        sStatusLED.Set(true);
    }
    else if (Instance().mHaveBLEConnections)
    {
        sStatusLED.Blink(LedConsts::StatusLed::Unprovisioned::kOn_ms, LedConsts::StatusLed::Unprovisioned::kOff_ms);
    }
    else
    {
        sStatusLED.Blink(LedConsts::StatusLed::Provisioned::kOn_ms, LedConsts::StatusLed::Provisioned::kOff_ms);
    }
#endif
}

void AppTask::ChipEventHandler(const ChipDeviceEvent * aEvent, intptr_t /* aArg */)
{
    if (!aEvent)
        return;
    switch (aEvent->Type)
    {
    case DeviceEventType::kCHIPoBLEAdvertisingChange:
        Instance().mHaveBLEConnections = ConnectivityMgr().NumBLEConnections() != 0;
        UpdateStatusLED();
        break;
    case DeviceEventType::kThreadStateChange:
        Instance().mIsThreadProvisioned = ConnectivityMgr().IsThreadProvisioned();
        Instance().mIsThreadEnabled     = ConnectivityMgr().IsThreadEnabled();
        UpdateStatusLED();
        break;
    default:
        break;
    }
}

void AppTask::CancelTimer()
{
    k_timer_stop(&sFunctionTimer);
    Instance().mFunctionTimerActive = false;
}

void AppTask::StartTimer(uint32_t aTimeoutInMs)
{
    k_timer_start(&sFunctionTimer, K_MSEC(aTimeoutInMs), K_NO_WAIT);
    Instance().mFunctionTimerActive = true;
}

void AppTask::PostEvent(AppEvent * aEvent)
{
    if (!aEvent)
        return;
    if (k_msgq_put(&sAppEventQueue, aEvent, K_NO_WAIT))
    {
        LOG_INF("Failed to post event to app task event queue");
    }
}

void AppTask::DispatchEvent(AppEvent * aEvent)
{
    if (!aEvent)
        return;
    if (aEvent->Handler)
    {
        aEvent->Handler(aEvent);
    }
    else
    {
        LOG_INF("Event received with no handler. Dropping event.");
    }
}
