/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#if !defined(GP_APP_DIVERSITY_POWERCYCLECOUNTING)
#error This application requires powercycle counting.
#endif

#include "powercycle_counting.h"

#include "AppConfig.h"
#include "AppEvent.h"
#include "AppTask.h"
#include "ota.h"

#include "ButtonHandler.h"
#include "StatusLed.h"
#include "qPinCfg.h"

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app/clusters/on-off-server/on-off-server.h>
#include <app/server/Server.h>

using namespace ::chip;
using namespace ::chip::app;
using namespace ::chip::TLV;
using namespace ::chip::Credentials;
using namespace ::chip::DeviceLayer;

#include <platform/CHIPDeviceLayer.h>

#define QPG_LOCK_ENDPOINT_ID (1)

const uint8_t StatusLedGpios[] = QPINCFG_STATUS_LED;
const ButtonConfig_t buttons[] = QPINCFG_BUTTONS;

AppTask AppTask::sAppTask;

CHIP_ERROR AppTask::Init()
{
    // Initialize common code in base class
    CHIP_ERROR err = BaseAppTask::Init();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(NotSpecified, "BaseAppTask::Init() failed");
        return err;
    }

    // Init ZCL Data Model and start server
    PlatformMgr().ScheduleWork(AppTask::InitServerWrapper, 0);

    // Setup powercycle reset expired handler
    gpAppFramework_SetResetExpiredHandler(AppTask::PowerCycleExpiredHandlerWrapper);

    // Setup button handler
    ButtonHandler_Init(buttons, Q_ARRAY_SIZE(buttons), BUTTON_LOW, AppTask::ButtonEventHandlerWrapper);

    // Setup Bolt
    err = BoltLockMgr().Init();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(NotSpecified, "BoltLockMgr().Init() failed");
        return err;
    }
    BoltLockMgr().SetCallbacks(ActionInitiated, ActionCompleted);
    StatusLed_SetLed(LOCK_STATE_LED, !BoltLockMgr().IsUnlocked());

    return err;
}

void AppTask::JammedLockEventHandler(AppEvent * aEvent)
{
    SystemLayer().ScheduleLambda([] {
        bool retVal;

        retVal = DoorLockServer::Instance().SendLockAlarmEvent(QPG_LOCK_ENDPOINT_ID, AlarmCodeEnum::kLockJammed);
        if (!retVal)
        {
            ChipLogProgress(NotSpecified, "[BTN] Lock jammed event send failed");
        }
        else
        {
            ChipLogProgress(NotSpecified, "[BTN] Lock jammed event sent");
        }
    });
}

void AppTask::LockActionEventHandler(AppEvent * aEvent)
{
    bool initiated = false;
    BoltLockManager::Action_t action;
    int32_t actor;
    CHIP_ERROR err = CHIP_NO_ERROR;

    if (aEvent->Type == AppEvent::kEventType_Lock)
    {
        action = static_cast<BoltLockManager::Action_t>(aEvent->LockEvent.Action);
        actor  = aEvent->LockEvent.Actor;
    }
    else if (aEvent->Type == AppEvent::kEventType_Button)
    {
        if (BoltLockMgr().IsUnlocked())
        {
            action = BoltLockManager::LOCK_ACTION;
        }
        else
        {
            action = BoltLockManager::UNLOCK_ACTION;
        }
        actor = AppEvent::kEventType_Button;
    }
    else
    {
        err = CHIP_ERROR_INTERNAL;
    }

    if (err == CHIP_NO_ERROR)
    {
        initiated = BoltLockMgr().InitiateAction(actor, action);

        if (!initiated)
        {
            ChipLogProgress(NotSpecified, "Action is already in progress or active.");
        }
    }
}

bool AppTask::ButtonEventHandler(uint8_t btnIdx, bool btnPressed)
{
    // Call base class ButtonEventHandler
    bool eventHandled = BaseAppTask::ButtonEventHandler(btnIdx, btnPressed);
    if (eventHandled)
    {
        return true;
    }

    // Only go ahead if button index has a supported value
    if (btnIdx != APP_LOCK_BUTTON && btnIdx != APP_LOCK_JAMMED_BUTTON)
    {
        return false;
    }

    ChipLogProgress(NotSpecified, "ButtonEventHandler %d, %d", btnIdx, btnPressed);

    AppEvent button_event              = {};
    button_event.Type                  = AppEvent::kEventType_Button;
    button_event.ButtonEvent.ButtonIdx = btnIdx;
    button_event.ButtonEvent.Action    = btnPressed;

    if (btnIdx == APP_LOCK_BUTTON && btnPressed == true)
    {
        button_event.Handler = LockActionEventHandler;
    }
    else if (btnIdx == APP_LOCK_JAMMED_BUTTON && btnPressed == true)
    {
        button_event.Handler = JammedLockEventHandler;
    }
    else
    {
        return false;
    }

    sAppTask.PostEvent(&button_event);

    return true;
}

void AppTask::ActionInitiated(BoltLockManager::Action_t aAction, int32_t aActor)
{
    // If the action has been initiated by the lock, update the bolt lock trait
    // and start flashing the LEDs rapidly to indicate action initiation.
    if (aAction == BoltLockManager::LOCK_ACTION)
    {
        ChipLogProgress(NotSpecified, "Lock Action has been initiated");
    }
    else if (aAction == BoltLockManager::UNLOCK_ACTION)
    {
        ChipLogProgress(NotSpecified, "Unlock Action has been initiated");
    }

    if (aActor == AppEvent::kEventType_Button)
    {
        sAppTask.mSyncClusterToButtonAction = true;
    }

    if (aActor == AppEvent::kEventType_Lock)
    {
        sAppTask.mNotifyState = true;
    }

    StatusLed_BlinkLed(LOCK_STATE_LED, 50, 50);
}

void AppTask::ActionCompleted(BoltLockManager::Action_t aAction)
{
    // if the action has been completed by the lock, update the bolt lock trait.
    // Turn on the lock LED if in a LOCKED state OR
    // Turn off the lock LED if in an UNLOCKED state.
    if (aAction == BoltLockManager::LOCK_ACTION)
    {
        ChipLogProgress(NotSpecified, "Lock Action has been completed");

        StatusLed_SetLed(LOCK_STATE_LED, true);
    }
    else if (aAction == BoltLockManager::UNLOCK_ACTION)
    {
        ChipLogProgress(NotSpecified, "Unlock Action has been completed");

        StatusLed_SetLed(LOCK_STATE_LED, false);
    }

    if (sAppTask.mSyncClusterToButtonAction || sAppTask.mNotifyState)
    {
        sAppTask.UpdateClusterState();
        sAppTask.mSyncClusterToButtonAction = false;
        sAppTask.mNotifyState               = false;
    }
}

void AppTask::PostLockActionRequest(int32_t aActor, BoltLockManager::Action_t aAction)
{
    AppEvent event;
    event.Type             = AppEvent::kEventType_Lock;
    event.LockEvent.Actor  = aActor;
    event.LockEvent.Action = aAction;
    event.Handler          = LockActionEventHandler;
    PostEvent(&event);
}

/**
 * Update cluster status after application level changes
 */
void AppTask::UpdateClusterState(void)
{
    using namespace chip::app::Clusters;
    auto newValue = BoltLockMgr().IsUnlocked() ? DoorLock::DlLockState::kUnlocked : DoorLock::DlLockState::kLocked;

    SystemLayer().ScheduleLambda([newValue] {
        bool retVal = true;
        chip::app::DataModel::Nullable<chip::app::Clusters::DoorLock::DlLockState> currentLockState;
        chip::app::Clusters::DoorLock::Attributes::LockState::Get(QPG_LOCK_ENDPOINT_ID, currentLockState);

        if (currentLockState.IsNull())
        {
            Protocols::InteractionModel::Status status = DoorLock::Attributes::LockState::Set(QPG_LOCK_ENDPOINT_ID, newValue);
            if (status != Protocols::InteractionModel::Status::Success)
            {
                ChipLogError(NotSpecified, "ERR: updating DoorLock %x", to_underlying(status));
            }
        }
        else
        {
            ChipLogProgress(NotSpecified, "Updating LockState attribute");
            if (sAppTask.mSyncClusterToButtonAction)
            {
                retVal = DoorLockServer::Instance().SetLockState(QPG_LOCK_ENDPOINT_ID, newValue, OperationSourceEnum::kManual);
            }

            if (retVal && sAppTask.mNotifyState)
            {
                retVal = DoorLockServer::Instance().SetLockState(QPG_LOCK_ENDPOINT_ID, newValue, OperationSourceEnum::kRemote);
            }

            if (!retVal)
            {
                ChipLogError(NotSpecified, "ERR: updating DoorLock");
            }
        }
    });
}

void AppTask::PowerCycleExpiredHandler(uint8_t resetCounts)
{
    if (resetCounts >= NMBR_OF_RESETS_BLE_ADVERTISING)
    {
        // Open commissioning if no fabric was available
        if (chip::Server::GetInstance().GetFabricTable().FabricCount() == 0)
        {
            ChipLogProgress(NotSpecified, "No fabrics, starting commissioning.");
            AppTask::OpenCommissioning((intptr_t) 0);
        }
    }
}
