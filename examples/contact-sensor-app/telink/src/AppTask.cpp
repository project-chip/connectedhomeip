/*
 *
 *    Copyright (c) 2022-2024 Project CHIP Authors
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
#include "LEDManager.h"

#include <app-common/zap-generated/attributes/Accessors.h>

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

LOG_MODULE_DECLARE(app, CONFIG_CHIP_APP_LOG_LEVEL);

AppTask AppTask::sAppTask;

CHIP_ERROR AppTask::Init(void)
{
    SetExampleButtonCallbacks(ContactActionEventHandler);
    InitCommonParts();

    LedManager::getInstance().setLed(LedManager::EAppLed_App0, ContactSensorMgr().IsContactClosed());

    UpdateDeviceState();

    ContactSensorMgr().SetCallback(OnStateChanged);

    return CHIP_NO_ERROR;
}

void AppTask::OnStateChanged(ContactSensorManager::State aState)
{
    // If the contact state was changed, update LED state and cluster state (only if button was pressed).
    //  - turn on the contact LED if contact sensor is in closed state.
    //  - turn off the lock LED if contact sensor is in opened state.
    if (ContactSensorManager::State::kContactClosed == aState)
    {
        LOG_INF("Contact state changed to CLOSED");
        LedManager::getInstance().setLed(LedManager::EAppLed_App0, true);
    }
    else if (ContactSensorManager::State::kContactOpened == aState)
    {
        LOG_INF("Contact state changed to OPEN");
        LedManager::getInstance().setLed(LedManager::EAppLed_App0, false);
    }

    if (sAppTask.IsSyncClusterToButtonAction())
    {
        sAppTask.UpdateClusterState();
    }
}

void AppTask::PostContactActionRequest(ContactSensorManager::Action aAction)
{
    AppEvent event;
    event.Type               = AppEvent::kEventType_DeviceAction;
    event.DeviceEvent.Action = static_cast<uint8_t>(aAction);
    event.Handler            = ContactActionEventHandler;

    sAppTask.PostEvent(&event);
}

void AppTask::UpdateClusterStateInternal(intptr_t arg)
{
    uint8_t newValue = ContactSensorMgr().IsContactClosed();

    ChipLogProgress(NotSpecified, "StateValue::Set : %d", newValue);

    // write the new boolean state value
    Protocols::InteractionModel::Status status = app::Clusters::BooleanState::Attributes::StateValue::Set(1, newValue);
    if (status != Protocols::InteractionModel::Status::Success)
    {
        ChipLogError(NotSpecified, "ERR: updating boolean status value %x", to_underlying(status));
    }
}

void AppTask::ContactActionEventHandler(AppEvent * aEvent)
{
    ContactSensorManager::Action action = ContactSensorManager::Action::kInvalid;
    CHIP_ERROR err                      = CHIP_NO_ERROR;

    ChipLogProgress(NotSpecified, "ContactActionEventHandler");

    if (aEvent->Type == AppEvent::kEventType_DeviceAction)
    {
        action = static_cast<ContactSensorManager::Action>(aEvent->DeviceEvent.Action);
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

void AppTask::UpdateClusterState(void)
{
    PlatformMgr().ScheduleWork(UpdateClusterStateInternal, 0);
}

void AppTask::UpdateDeviceState(void)
{
    PlatformMgr().ScheduleWork(UpdateDeviceStateInternal, 0);
}

void AppTask::UpdateDeviceStateInternal(intptr_t arg)
{
    bool stateValueAttrValue = 0;

    /* get boolean state attribute value */
    (void) app::Clusters::BooleanState::Attributes::StateValue::Get(1, &stateValueAttrValue);

    LedManager::getInstance().setLed(LedManager::EAppLed_App0, stateValueAttrValue);
}

void AppTask::LinkLeds(LedManager & ledManager)
{
#if CONFIG_CHIP_ENABLE_APPLICATION_STATUS_LED
    ledManager.linkLed(LedManager::EAppLed_Status, 0);
    ledManager.linkLed(LedManager::EAppLed_App0, 1);
#else
    ledManager.linkLed(LedManager::EAppLed_App0, 0);
#endif // CONFIG_CHIP_ENABLE_APPLICATION_STATUS_LED
}
