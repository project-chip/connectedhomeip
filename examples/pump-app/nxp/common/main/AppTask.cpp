/*
 *
 *    Copyright (c) 2021-2023 Project CHIP Authors
 *    Copyright (c) 2021 Google LLC.
 *    Copyright 2023-2024 NXP
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
#include "CHIPDeviceManager.h"
#include "ICDUtil.h"
#include <app/InteractionModelEngine.h>
#include <app/util/attribute-storage.h>
#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include "PumpManager.h"

static bool mSyncClusterToButtonAction = false;

using namespace chip;
using namespace chip::TLV;
using namespace ::chip::Credentials;
using namespace ::chip::DeviceLayer;
using namespace ::chip::DeviceManager;
using namespace ::chip::app::Clusters;


void PumpApp::AppTask::PreInitMatterStack()
{
    ChipLogProgress(DeviceLayer, "Welcome to NXP Pump Demo App");
}

void PumpApp::AppTask::PostInitMatterStack()
{
	CHIP_ERROR err = CHIP_NO_ERROR;
    chip::app::InteractionModelEngine::GetInstance()->RegisterReadHandlerAppCallback(&chip::NXP::App::GetICDUtil());
	if (PumpMgr().Init() != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "Init pump failed: %s", ErrorStr(err));
    }
	
    PumpMgr().SetCallbacks(ActionInitiated, ActionCompleted);
}

// This returns an instance of this class.
PumpApp::AppTask & PumpApp::AppTask::GetDefaultInstance()
{
    static PumpApp::AppTask sAppTask;
    return sAppTask;
}

chip::NXP::App::AppTaskBase & chip::NXP::App::GetAppTask()
{
    return PumpApp::AppTask::GetDefaultInstance();
}

void PumpApp::AppTask::PumpActionEventHandler(const AppEvent & aEvent)
{
    bool initiated = false;
    PumpManager::Action_t action;
    int32_t actor;
    CHIP_ERROR err = CHIP_NO_ERROR;

    if (aEvent.Type == AppEvent::kEventType_PumpTimer)
    {
        action = static_cast<PumpManager::Action_t>(aEvent.PumpEvent.Action);
        actor  = aEvent.PumpEvent.Actor;
    }
    else if (aEvent.Type == AppEvent::kEventType_Button)
    {
        action = (PumpMgr().IsStopped()) ? PumpManager::START_ACTION : PumpManager::STOP_ACTION;
        actor  = AppEvent::kEventType_Button;
    }
    else
    {
        err = CHIP_ERROR_UNEXPECTED_EVENT;
    }

    if (err == CHIP_NO_ERROR)
    {
        initiated = PumpMgr().InitiateAction(actor, action);

        if (!initiated)
        {
            ChipLogError(DeviceLayer, "Action is already in progress or active.");
        }
    }
}

void PumpApp::AppTask::PumpButtonEventHandler()
{
    AppEvent button_event           = {};
    button_event.Type               = AppEvent::kEventType_Button;
    button_event.Handler            = PumpActionEventHandler;
	
    sAppTask.PostEvent(button_event);
}

void PumpApp::AppTask::ActionInitiated(PumpManager::Action_t aAction, int32_t aActor)
{
    // Action initiated, update the light led
    bool pumpOn = aAction == PumpManager::START_ACTION;
    ChipLogProgress(DeviceLayer, " Pump %s", (pumpOn) ? "starting" : "stopping")

    if (aActor == AppEvent::kEventType_Button)
    {
        mSyncClusterToButtonAction = true;
    }
}

void PumpApp::AppTask::ActionCompleted(PumpManager::Action_t aAction, int32_t aActor)
{
    // action has been completed bon the light
    if (aAction == PumpManager::STOP_ACTION)
    {
        ChipLogProgress(DeviceLayer,"Pump Stopped")
    }
    else if (aAction == PumpManager::STOP_ACTION)
    {
        ChipLogProgress(DeviceLayer,"Pump OFF")
    }

    if (mSyncClusterToButtonAction)
    {
        PlatformMgr().ScheduleWork(UpdateClusterState, 0);
        mSyncClusterToButtonAction = false;
    }
}

void PumpApp::AppTask::UpdateClusterState(intptr_t context)
{
    // Set On/Off state
    Protocols::InteractionModel::Status status;
    bool onOffState = !PumpMgr().IsStopped();
    status          = OnOff::Attributes::OnOff::Set(1, onOffState);
    if (status != Protocols::InteractionModel::Status::Success)
    {
        ChipLogError(DeviceLayer, "Updating On/Off state failed!");
    }
}