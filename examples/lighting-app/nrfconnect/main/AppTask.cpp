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

#include "AppTask.h"

#include "AppConfig.h"
#include "AppEvent.h"
#include "LEDWidget.h"
#include "LightingManager.h"
#include "Service.h"
#include "ThreadUtil.h"
#include <app/server/OnboardingCodesUtil.h>
#include <app/server/Server.h>

#include <app/common/gen/attribute-id.h>
#include <app/common/gen/attribute-type.h>
#include <app/common/gen/cluster-id.h>
#include <app/util/attribute-storage.h>

#include <platform/CHIPDeviceLayer.h>

#include <support/ErrorStr.h>
#include <system/SystemClock.h>

// MCUMgr BT FOTA includes
#ifdef CONFIG_MCUMGR_CMD_OS_MGMT
#include "os_mgmt/os_mgmt.h"
#endif
#ifdef CONFIG_MCUMGR_CMD_IMG_MGMT
#include "img_mgmt/img_mgmt.h"
#endif
#ifdef CONFIG_MCUMGR_SMP_BT
#include <mgmt/mcumgr/smp_bt.h>
#endif
#ifdef CONFIG_BOOTLOADER_MCUBOOT
#include <dfu/mcuboot.h>
#endif

#include <dk_buttons_and_leds.h>
#include <logging/log.h>
#include <zephyr.h>

LOG_MODULE_DECLARE(app);

namespace {

constexpr int kFactoryResetTriggerTimeout      = 3000;
constexpr int kFactoryResetCancelWindowTimeout = 3000;
constexpr int kAppEventQueueSize               = 10;
constexpr int kExampleVendorID                 = 0xabcd;
constexpr uint8_t kButtonPushEvent             = 1;
constexpr uint8_t kButtonReleaseEvent          = 0;
constexpr uint32_t kPublishServicePeriodUs     = 5000000;

K_MSGQ_DEFINE(sAppEventQueue, sizeof(AppEvent), kAppEventQueueSize, alignof(AppEvent));
k_timer sFunctionTimer;

LEDWidget sStatusLED;
LEDWidget sUnusedLED;
LEDWidget sUnusedLED_1;

bool sIsThreadProvisioned     = false;
bool sIsThreadEnabled         = false;
bool sHaveBLEConnections      = false;
bool sHaveServiceConnectivity = false;

} // namespace

using namespace ::chip::DeviceLayer;

AppTask AppTask::sAppTask;

int AppTask::Init()
{
    // Initialize LEDs
    LEDWidget::InitGpio();

    sStatusLED.Init(SYSTEM_STATE_LED);
    sUnusedLED.Init(DK_LED3);
    sUnusedLED_1.Init(DK_LED4);

    // Initialize buttons
    int ret = dk_buttons_init(ButtonEventHandler);
    if (ret)
    {
        LOG_ERR("dk_buttons_init() failed");
        return ret;
    }

    // Initialize timer user data
    k_timer_init(&sFunctionTimer, &AppTask::TimerEventHandler, nullptr);
    k_timer_user_data_set(&sFunctionTimer, this);

#ifdef CONFIG_BOOTLOADER_MCUBOOT
    // Check if the image is run in the REVERT mode and eventually
    // confirm it to prevent reverting on the next boot.
    if (mcuboot_swap_type() == BOOT_SWAP_TYPE_REVERT)
    {
        if (boot_write_img_confirmed())
        {
            LOG_ERR("Confirming firmware image failed, it will be reverted on the next boot.");
        }
        else
        {
            LOG_INF("New firmware image confirmed.");
        }
    }
#endif

    ret = LightingMgr().Init(LIGHTING_PWM_DEVICE, LIGHTING_PWM_CHANNEL);
    if (ret != 0)
        return ret;

    LightingMgr().SetCallbacks(ActionInitiated, ActionCompleted);

    // Init ZCL Data Model and start server
    InitServer();
    ConfigurationMgr().LogDeviceConfig();
    PrintOnboardingCodes(chip::RendezvousInformationFlags(chip::RendezvousInformationFlag::kBLE));

#ifdef CONFIG_CHIP_NFC_COMMISSIONING
    PlatformMgr().AddEventHandler(ThreadProvisioningHandler, 0);
#endif

    return 0;
}

int AppTask::StartApp()
{
    int ret                            = Init();
    uint64_t mLastPublishServiceTimeUS = 0;

    if (ret)
    {
        LOG_ERR("AppTask.Init() failed");
        return ret;
    }

    AppEvent event = {};

    while (true)
    {
        ret = k_msgq_get(&sAppEventQueue, &event, K_MSEC(10));

        while (!ret)
        {
            DispatchEvent(&event);
            ret = k_msgq_get(&sAppEventQueue, &event, K_NO_WAIT);
        }

        // Collect connectivity and configuration state from the CHIP stack.  Because the
        // CHIP event loop is being run in a separate task, the stack must be locked
        // while these values are queried.  However we use a non-blocking lock request
        // (TryLockChipStack()) to avoid blocking other UI activities when the CHIP
        // task is busy (e.g. with a long crypto operation).

        if (PlatformMgr().TryLockChipStack())
        {
            sIsThreadProvisioned     = ConnectivityMgr().IsThreadProvisioned();
            sIsThreadEnabled         = ConnectivityMgr().IsThreadEnabled();
            sHaveBLEConnections      = (ConnectivityMgr().NumBLEConnections() != 0);
            sHaveServiceConnectivity = ConnectivityMgr().HaveServiceConnectivity();
            PlatformMgr().UnlockChipStack();
        }

        // Update the status LED if factory reset has not been initiated.
        //
        // If system has "full connectivity", keep the LED On constantly.
        //
        // If thread and service provisioned, but not attached to the thread network yet OR no
        // connectivity to the service OR subscriptions are not fully established
        // THEN blink the LED Off for a short period of time.
        //
        // If the system has ble connection(s) uptill the stage above, THEN blink the LEDs at an even
        // rate of 100ms.
        //
        // Otherwise, blink the LED ON for a very short time.
        if (sAppTask.mFunction != kFunction_FactoryReset)
        {
            if (sHaveServiceConnectivity)
            {
                sStatusLED.Set(true);
            }
            else if (sIsThreadProvisioned && sIsThreadEnabled)
            {
                sStatusLED.Blink(950, 50);
            }
            else if (sHaveBLEConnections)
            {
                sStatusLED.Blink(100, 100);
            }
            else
            {
                sStatusLED.Blink(50, 950);
            }
        }

        sStatusLED.Animate();
        sUnusedLED.Animate();
        sUnusedLED_1.Animate();

        uint64_t nowUS            = chip::System::Clock::GetMonotonicMicroseconds();
        uint64_t nextChangeTimeUS = mLastPublishServiceTimeUS + kPublishServicePeriodUs;

        if (nowUS > nextChangeTimeUS)
        {
            PublishService();
            mLastPublishServiceTimeUS = nowUS;
        }
    }
}

void AppTask::LightingActionEventHandler(AppEvent * aEvent)
{
    LightingManager::Action_t action = LightingManager::INVALID_ACTION;
    int32_t actor                    = 0;

    if (aEvent->Type == AppEvent::kEventType_Lighting)
    {
        action = static_cast<LightingManager::Action_t>(aEvent->LightingEvent.Action);
        actor  = aEvent->LightingEvent.Actor;
    }
    else if (aEvent->Type == AppEvent::kEventType_Button)
    {
        action = LightingMgr().IsTurnedOn() ? LightingManager::OFF_ACTION : LightingManager::ON_ACTION;
        actor  = AppEvent::kEventType_Button;
    }

    if (action != LightingManager::INVALID_ACTION && !LightingMgr().InitiateAction(action, actor, 0, NULL))
        LOG_INF("Action is already in progress or active.");
}

void AppTask::ButtonEventHandler(uint32_t button_state, uint32_t has_changed)
{
    AppEvent button_event;
    button_event.Type = AppEvent::kEventType_Button;

    if (LIGHTING_BUTTON_MASK & button_state & has_changed)
    {
        button_event.ButtonEvent.PinNo  = LIGHTING_BUTTON;
        button_event.ButtonEvent.Action = kButtonPushEvent;
        button_event.Handler            = LightingActionEventHandler;
        sAppTask.PostEvent(&button_event);
    }

    if (FUNCTION_BUTTON_MASK & has_changed)
    {
        button_event.ButtonEvent.PinNo  = FUNCTION_BUTTON;
        button_event.ButtonEvent.Action = (FUNCTION_BUTTON_MASK & button_state) ? kButtonPushEvent : kButtonReleaseEvent;
        button_event.Handler            = FunctionHandler;
        sAppTask.PostEvent(&button_event);
    }

    if (THREAD_START_BUTTON_MASK & button_state & has_changed)
    {
        button_event.ButtonEvent.PinNo  = THREAD_START_BUTTON;
        button_event.ButtonEvent.Action = kButtonPushEvent;
        button_event.Handler            = StartThreadHandler;
        sAppTask.PostEvent(&button_event);
    }

    if (BLE_ADVERTISEMENT_START_BUTTON_MASK & button_state & has_changed)
    {
        button_event.ButtonEvent.PinNo  = BLE_ADVERTISEMENT_START_BUTTON;
        button_event.ButtonEvent.Action = kButtonPushEvent;
        button_event.Handler            = StartBLEAdvertisementHandler;
        sAppTask.PostEvent(&button_event);
    }
}

void AppTask::TimerEventHandler(k_timer * timer)
{
    AppEvent event;
    event.Type               = AppEvent::kEventType_Timer;
    event.TimerEvent.Context = k_timer_user_data_get(timer);
    event.Handler            = FunctionTimerEventHandler;
    sAppTask.PostEvent(&event);
}

void AppTask::FunctionTimerEventHandler(AppEvent * aEvent)
{
    if (aEvent->Type != AppEvent::kEventType_Timer)
        return;

    // If we reached here, the button was held past kFactoryResetTriggerTimeout, initiate factory reset
    if (sAppTask.mFunctionTimerActive && sAppTask.mFunction == kFunction_SoftwareUpdate)
    {
        LOG_INF("Factory Reset Triggered. Release button within %ums to cancel.", kFactoryResetTriggerTimeout);

        // Start timer for kFactoryResetCancelWindowTimeout to allow user to cancel, if required.
        sAppTask.StartTimer(kFactoryResetCancelWindowTimeout);
        sAppTask.mFunction = kFunction_FactoryReset;

        // Turn off all LEDs before starting blink to make sure blink is co-ordinated.
        sStatusLED.Set(false);
        sUnusedLED_1.Set(false);
        sUnusedLED.Set(false);

        sStatusLED.Blink(500);
        sUnusedLED.Blink(500);
        sUnusedLED_1.Blink(500);
    }
    else if (sAppTask.mFunctionTimerActive && sAppTask.mFunction == kFunction_FactoryReset)
    {
        // Actually trigger Factory Reset
        sAppTask.mFunction = kFunction_NoneSelected;
        ConfigurationMgr().InitiateFactoryReset();
    }
}

int AppTask::SoftwareUpdateConfirmationHandler(uint32_t offset, uint32_t size, void * arg)
{
    // For now just print update progress and confirm data chunk without any additional checks.
    LOG_INF("Software update progress %d B / %d B", offset, size);

    return 0;
}

void AppTask::FunctionHandler(AppEvent * aEvent)
{
    if (aEvent->ButtonEvent.PinNo != FUNCTION_BUTTON)
        return;

    // To trigger software update: press the FUNCTION_BUTTON button briefly (< kFactoryResetTriggerTimeout)
    // To initiate factory reset: press the FUNCTION_BUTTON for kFactoryResetTriggerTimeout + kFactoryResetCancelWindowTimeout
    // All LEDs start blinking after kFactoryResetTriggerTimeout to signal factory reset has been initiated.
    // To cancel factory reset: release the FUNCTION_BUTTON once all LEDs start blinking within the
    // kFactoryResetCancelWindowTimeout
    if (aEvent->ButtonEvent.Action == kButtonPushEvent)
    {
        if (!sAppTask.mFunctionTimerActive && sAppTask.mFunction == kFunction_NoneSelected)
        {
            sAppTask.StartTimer(kFactoryResetTriggerTimeout);

            sAppTask.mFunction = kFunction_SoftwareUpdate;
        }
    }
    else
    {
        // If the button was released before factory reset got initiated, trigger a software update.
        if (sAppTask.mFunctionTimerActive && sAppTask.mFunction == kFunction_SoftwareUpdate)
        {
            sAppTask.CancelTimer();
            sAppTask.mFunction = kFunction_NoneSelected;

#if defined(CONFIG_MCUMGR_SMP_BT) && defined(CONFIG_MCUMGR_CMD_IMG_MGMT) && defined(CONFIG_MCUMGR_CMD_OS_MGMT)
            if (!sAppTask.mSoftwareUpdateEnabled)
            {
                sAppTask.mSoftwareUpdateEnabled = true;
                os_mgmt_register_group();
                img_mgmt_register_group();
                img_mgmt_set_upload_cb(SoftwareUpdateConfirmationHandler, NULL);
                smp_bt_register();

                LOG_INF("Enabled software update");
            }
            else
            {
                LOG_INF("Software update is already enabled");
            }

#else
            LOG_INF("Software update is disabled");
#endif
        }
        else if (sAppTask.mFunctionTimerActive && sAppTask.mFunction == kFunction_FactoryReset)
        {
            sUnusedLED.Set(false);
            sUnusedLED_1.Set(false);
            sAppTask.CancelTimer();
            sAppTask.mFunction = kFunction_NoneSelected;
            LOG_INF("Factory Reset has been Canceled");
        }
    }
}

void AppTask::StartThreadHandler(AppEvent * aEvent)
{
    if (aEvent->ButtonEvent.PinNo != THREAD_START_BUTTON)
        return;

    if (AddTestPairing() != CHIP_NO_ERROR)
    {
        LOG_ERR("Failed to add test pairing");
    }

    if (!chip::DeviceLayer::ConnectivityMgr().IsThreadProvisioned())
    {
        StartDefaultThreadNetwork();
        LOG_INF("Device is not commissioned to a Thread network. Starting with the default configuration.");
    }
    else
    {
        LOG_INF("Device is commissioned to a Thread network.");
    }
}

void AppTask::StartBLEAdvertisementHandler(AppEvent * aEvent)
{
    if (aEvent->ButtonEvent.PinNo != BLE_ADVERTISEMENT_START_BUTTON)
        return;

    // In case of having software update enabled, allow on starting BLE advertising after Thread provisioning.
    if (chip::DeviceLayer::ConnectivityMgr().IsThreadProvisioned() && !sAppTask.mSoftwareUpdateEnabled)
    {
        LOG_INF("NFC Tag emulation and BLE advertisement not started - device is commissioned to a Thread network.");
        return;
    }

#ifdef CONFIG_CHIP_NFC_COMMISSIONING
    if (NFCMgr().IsTagEmulationStarted())
    {
        LOG_INF("NFC Tag emulation is already started");
    }
    else
    {
        ShareQRCodeOverNFC(chip::RendezvousInformationFlags(chip::RendezvousInformationFlag::kBLE));
    }
#endif

    if (ConnectivityMgr().IsBLEAdvertisingEnabled())
    {
        LOG_INF("BLE Advertisement is already enabled");
        return;
    }

    if (OpenDefaultPairingWindow(chip::ResetAdmins::kNo) == CHIP_NO_ERROR)
    {
        LOG_INF("Enabled BLE Advertisement");
    }
    else
    {
        LOG_ERR("OpenDefaultPairingWindow() failed");
    }
}

#ifdef CONFIG_CHIP_NFC_COMMISSIONING
void AppTask::ThreadProvisioningHandler(const ChipDeviceEvent * event, intptr_t)
{
    if (event->Type == DeviceEventType::kCHIPoBLEAdvertisingChange && event->CHIPoBLEAdvertisingChange.Result == kActivity_Stopped)
    {
        NFCMgr().StopTagEmulation();
    }
}
#endif

void AppTask::CancelTimer()
{
    k_timer_stop(&sFunctionTimer);
    mFunctionTimerActive = false;
}

void AppTask::StartTimer(uint32_t aTimeoutInMs)
{
    k_timer_start(&sFunctionTimer, K_MSEC(aTimeoutInMs), K_NO_WAIT);
    mFunctionTimerActive = true;
}

void AppTask::ActionInitiated(LightingManager::Action_t aAction, int32_t aActor)
{
    if (aAction == LightingManager::ON_ACTION)
    {
        LOG_INF("Turn On Action has been initiated");
    }
    else if (aAction == LightingManager::OFF_ACTION)
    {
        LOG_INF("Turn Off Action has been initiated");
    }
    else if (aAction == LightingManager::LEVEL_ACTION)
    {
        LOG_INF("Level Action has been initiated");
    }
}

void AppTask::ActionCompleted(LightingManager::Action_t aAction, int32_t aActor)
{
    if (aAction == LightingManager::ON_ACTION)
    {
        LOG_INF("Turn On Action has been completed");
    }
    else if (aAction == LightingManager::OFF_ACTION)
    {
        LOG_INF("Turn Off Action has been completed");
    }
    else if (aAction == LightingManager::LEVEL_ACTION)
    {
        LOG_INF("Level Action has been completed");
    }

    if (aActor == AppEvent::kEventType_Button)
    {
        sAppTask.UpdateClusterState();
    }
}

void AppTask::PostLightingActionRequest(LightingManager::Action_t aAction)
{
    AppEvent event;
    event.Type                 = AppEvent::kEventType_Lighting;
    event.LightingEvent.Action = aAction;
    event.Handler              = LightingActionEventHandler;
    PostEvent(&event);
}

void AppTask::PostEvent(AppEvent * aEvent)
{
    if (k_msgq_put(&sAppEventQueue, aEvent, K_NO_WAIT) != 0)
    {
        LOG_INF("Failed to post event to app task event queue");
    }
}

void AppTask::DispatchEvent(AppEvent * aEvent)
{
    if (aEvent->Handler)
    {
        aEvent->Handler(aEvent);
    }
    else
    {
        LOG_INF("Event received with no handler. Dropping event.");
    }
}

void AppTask::UpdateClusterState()
{
    uint8_t onoff = LightingMgr().IsTurnedOn();

    // write the new on/off value
    EmberAfStatus status = emberAfWriteAttribute(1, ZCL_ON_OFF_CLUSTER_ID, ZCL_ON_OFF_ATTRIBUTE_ID, CLUSTER_MASK_SERVER, &onoff,
                                                 ZCL_BOOLEAN_ATTRIBUTE_TYPE);
    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        LOG_ERR("Updating on/off cluster failed: %x", status);
    }

    uint8_t level = LightingMgr().GetLevel();

    status = emberAfWriteAttribute(1, ZCL_LEVEL_CONTROL_CLUSTER_ID, ZCL_CURRENT_LEVEL_ATTRIBUTE_ID, CLUSTER_MASK_SERVER, &level,
                                   ZCL_INT8U_ATTRIBUTE_TYPE);

    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        LOG_ERR("Updating level cluster failed: %x", status);
    }
}
