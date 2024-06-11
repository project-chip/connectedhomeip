/*
 *
 *    Copyright (c) 2022-2023 Project CHIP Authors
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
#include "MotorWidget.h"
#include "binding-handler.h"
#include <app-common/zap-generated/attributes/Accessors.h>
#include <app/util/af-enums.h>
#include <app/util/attribute-storage.h>

#define SLEEP_TIME 3500

LOG_MODULE_DECLARE(app, CONFIG_CHIP_APP_LOG_LEVEL);

AppTask AppTask::sAppTask;

const struct gpio_dt_spec sFactoryResetButtonDt = GPIO_DT_SPEC_GET(DT_NODELABEL(key_switch), gpios);
MotorWidget sStatusMotor;
bool direction    = false;
bool switch_state = false;
static k_timer mButtonReleaseCheckTimer;
static int buttonPressCounter = 0;

// 按键检测标志位
// bool isButtonPressed = false;

CHIP_ERROR AppTask::Init(void)
{
#if APP_USE_EXAMPLE_START_BUTTON
    SetExampleButtonCallbacks(ContactActionEventHandler);
#endif
    SetFactoryResetButtonCallbacks(SwitchActionEventHandler);
    k_timer_init(&mButtonReleaseCheckTimer, &AppTask::buttonReleaseCheckTimerHandler, nullptr);
    k_timer_user_data_set(&mButtonReleaseCheckTimer, this);

    InitCommonParts();

    // Init motor control
    MotorWidget::SetCallback(MotorStateUpdateHandler, MotorStopHandler);
    sStatusMotor.Init();
    UpdateMotor();

    ContactSensorMgr().Init();
    ContactSensorMgr().SetCallback(OnStateChanged);

    // sStatusMotor.Start(false);
    // k_sleep(K_MSEC(2250));
    // sStatusMotor.Start(true);
    // sStatusMotor.Restart();

    // 获得上电时开关的状态(On/Off)
    if (!sStatusMotor.GetMotorState())
    {
        ContactSensorMgr().setstate(ContactSensorManager::State::kContactOpened);
    }
    else
    {
        ContactSensorMgr().setstate(ContactSensorManager::State::kContactClosed);
    }

    if (sAppTask.IsSyncClusterToButtonAction())
    {
        sAppTask.UpdateClusterState();
    }
    UpdateClusterStateInternal(NULL); // 能更新开关on/off的状态，没这个函数开关上电后第一次按可能会没反应

    // Configure Bindings
    CHIP_ERROR err = InitBindingHandler();
    if (err != CHIP_NO_ERROR)
    {
        LOG_ERR("InitBindingHandler fail");
        return err;
    }

    // k_sleep(K_MSEC(3000));
    // OnStateChanged(ContactSensorManager::State::kContactOpened);
    // int ret = gpio_pin_configure_dt(&sFactoryResetButtonDt, GPIO_INPUT | GPIO_PULL_DOWN);
    // k_sleep(K_MSEC(200));
    // ret = gpio_pin_configure_dt(&sFactoryResetButtonDt, GPIO_INPUT | GPIO_PULL_UP);
    // k_sleep(K_MSEC(3000));
    // OnStateChanged(ContactSensorManager::State::kContactClosed);
    // ret = gpio_pin_configure_dt(&sFactoryResetButtonDt, GPIO_INPUT | GPIO_PULL_DOWN);
    // k_sleep(K_MSEC(200));
    // ret = gpio_pin_configure_dt(&sFactoryResetButtonDt, GPIO_INPUT | GPIO_PULL_UP);

    return CHIP_NO_ERROR;
}

void AppTask::MotorStopHandler(MotorWidget * motorWidget)
{
    AppEvent event;
    event.Type                              = AppEvent::kEventType_MotorStop;
    event.Handler                           = UpdateMotorEventHandler;
    event.UpdateMotorStateEvent.motorWidget = motorWidget;
    GetAppTask().PostEvent(&event);
}

void AppTask::MotorStateUpdateHandler(MotorWidget * motorWidget)
{
    AppEvent event;
    event.Type                              = AppEvent::kEventType_MotorStateUpdate;
    event.Handler                           = UpdateMotorEventHandler;
    event.UpdateMotorStateEvent.motorWidget = motorWidget;
    GetAppTask().PostEvent(&event);
}

void AppTask::UpdateMotorEventHandler(AppEvent * aEvent)
{
    if (aEvent->Type == AppEvent::kEventType_MotorStateUpdate)
    {
        aEvent->UpdateMotorStateEvent.motorWidget->UpdateState();
        UpdateStatusLEDExt(true, false);
        // LOG_INF("UpdateStatusLEDExt(true, false);");
    }
    if (aEvent->Type == AppEvent::kEventType_MotorStop)
    {
        aEvent->UpdateMotorStateEvent.motorWidget->MotorStop();
        UpdateStatusLEDExt(false, false);
        // LOG_INF("UpdateStatusLEDExt(false, false);");
    }
}

void AppTask::UpdateMotor()
{
    sStatusMotor.MotorStop();
}

void AppTask::FactoryResetEventHandler(AppEvent * aEvent)
{
    if (aEvent->Type == AppEvent::kEventType_Button)
    {
        if (switch_state == false)
        {
            switch_state = true;
        }
        else
        {
            switch_state = false;
        }
        sStatusMotor.Start(switch_state);
    }
}

void AppTask::SwitchActionEventHandler(AppEvent * aEvent)
{
    if (aEvent->Type == AppEvent::kEventType_Button)
    {
        LOG_INF("App button pressed");
        k_timer_start(&mButtonReleaseCheckTimer, K_MSEC(20), K_NO_WAIT);
    }
}

void AppTask::buttonReleaseCheckTimerHandler(k_timer * timer)
{
    AppEvent event;
    event.Type    = AppEvent::kEventType_ButtonReleaseCheck;
    event.Handler = buttonReleaseCheckEventHandler;
    GetAppTask().PostEvent(&event);
}

void AppTask::buttonReleaseCheckEventHandler(AppEvent * aEvent)
{
    if (MotorWidget::sInstance.isMotorStop == false)
    {
        LOG_INF("AppTask ----> sInstance.isMotorStop == false");
        return;
    }
    if (aEvent->Type == AppEvent::kEventType_ButtonReleaseCheck)
    {
        if (gpio_pin_get_dt(&sFactoryResetButtonDt) == 0)
        {
            if (buttonPressCounter < 250)
            {
                // isButtonPressed = true;
                if (ContactSensorMgr().IsContactClosed() == true)
                {
                    LOG_INF("Switch state from close to open");
                    ContactSensorMgr().setstate(ContactSensorManager::State::kContactOpened);
                }
                else
                {
                    LOG_INF("Switch state from open to close");
                    ContactSensorMgr().setstate(ContactSensorManager::State::kContactClosed);
                }
                UpdateClusterStateInternal(NULL);
            }
            else if (buttonPressCounter < 500)
            {
                UpdateStatusLEDExt(false, false);
            }
            else
            {
                LOG_INF("Do factory reset");
                FactoryReset();
            }
            buttonPressCounter = 0;
        }
        else
        {
            k_timer_start(&mButtonReleaseCheckTimer, K_MSEC(20), K_NO_WAIT);
            buttonPressCounter++;
            if (buttonPressCounter == 250)
            {
                UpdateStatusLEDExt(false, true);
                LOG_INF("Wait another 5s to do factory reset");
            }
            if (buttonPressCounter == 500)
            {
                LOG_INF("Release to do factory reset");
                UpdateStatusLEDExt(false, false);
            }
        }
    }
}

// void AppTask::UpdateClusterState() {}
void AppTask::UpdateClusterState()
{
    PlatformMgr().ScheduleWork(UpdateClusterStateInternal, 0);
}

void AppTask::OnStateChanged(ContactSensorManager::State aState)
{
    // if (isButtonPressed == false)
    // {
    //     return;
    // }
    // If the contact state was changed, update LED state and cluster state (only if button was pressed).
    //  - turn on the contact LED if contact sensor is in closed state.
    //  - turn off the lock LED if contact sensor is in opened state.
    if (ContactSensorManager::State::kContactClosed == aState)
    {
        LOG_INF("Switch state changed to On");
        // sContactSensorLED.Set(true);
        sStatusMotor.Start(true);
    }
    else if (ContactSensorManager::State::kContactOpened == aState)
    {
        LOG_INF("Switch state changed to Off");
        // sContactSensorLED.Set(false);
        sStatusMotor.Start(false);
    }

    if (sAppTask.IsSyncClusterToButtonAction())
    {
        sAppTask.UpdateClusterState();
    }

    // isButtonPressed = false;
}

void AppTask::UpdateClusterStateInternal(intptr_t arg)
{
    bool newValue = ContactSensorMgr().IsContactClosed();
    // bool newValue = sStatusMotor.GetMotorState();

    ChipLogProgress(NotSpecified, "emberAfWriteAttribute : %d", newValue);

    // write the new boolean state value
    /*EmberAfStatus status = emberAfWriteAttribute(1, ZCL_ON_OFF_CLUSTER_ID, ZCL_STATE_VALUE_ATTRIBUTE_ID, (uint8_t *) &newValue,
                                                 ZCL_BOOLEAN_ATTRIBUTE_TYPE);*/
    EmberAfStatus status = Clusters::OnOff::Attributes::OnOff::Set(1, newValue);
    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        ChipLogError(NotSpecified, "ERR: updating boolean status value %x", status);
    }
}

void AppTask::ContactActionEventHandler(AppEvent * aEvent)
{
    ContactSensorManager::Action action = ContactSensorManager::Action::kInvalid;
    CHIP_ERROR err                      = CHIP_NO_ERROR;

    ChipLogProgress(NotSpecified, "ContactActionEventHandler");

    if (aEvent->Type == AppEvent::kEventType_Contact)
    {
        action = static_cast<ContactSensorManager::Action>(aEvent->ContactEvent.Action);
    }
    else if (aEvent->Type == AppEvent::kEventType_Button)
    {
        if (ContactSensorMgr().IsContactClosed())
        {
            action = ContactSensorManager::Action::kSignalLost;
        }
        else
        {
            action = ContactSensorManager::Action::kSignalDetected;
        }

        sAppTask.SetSyncClusterToButtonAction(true);
    }
    else
    {
        err    = APP_ERROR_UNHANDLED_EVENT;
        action = ContactSensorManager::Action::kInvalid;
    }

    if (err == CHIP_NO_ERROR)
    {
        ContactSensorMgr().InitiateAction(action);
    }
}
