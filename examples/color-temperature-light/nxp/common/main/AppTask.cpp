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
#include "app/clusters/occupancy-sensor-server/occupancy-hal.h"
#include <app/clusters/occupancy-sensor-server/occupancy-sensor-server.h>
#include <app/InteractionModelEngine.h>
#include <app/util/attribute-storage.h>
#include <platform/CHIPDeviceLayer.h>

#include "LightingManager.h"
#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>

#ifdef ENABLE_CHIP_SHELL
#include <lib/shell/Engine.h>
#include <map>
using namespace chip::Shell;
#define MATTER_CLI_LOG(message) (streamer_printf(streamer_get(), message))
#endif /* ENABLE_CHIP_SHELL */

using namespace chip;
using namespace chip::TLV;
using namespace ::chip::Credentials;
using namespace ::chip::DeviceLayer;
using namespace ::chip::DeviceManager;
using namespace ::chip::app::Clusters;
using namespace chip::app::Clusters::OccupancySensing;
using namespace chip::app::Clusters::OccupancySensing::Structs;

static CHIP_ERROR occupancyCommandHandler(int argc, char * argv[])
{
    if ((argc != 1) && (argc != 2))
    {
        ChipLogError(Shell, "Invalid Argument");
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    if (!strcmp(argv[0], "present"))
    {
        ChipLogDetail(Shell, "OpOccupancy Sensor : Set to %s state", argv[0]);
		LightingApp::AppTask::GetDefaultInstance().OccupancyHandler(true);
        uint16_t * holdTime = GetHoldTimeForEndpoint(1);
		chip::DeviceLayer::SystemLayer().StartTimer(chip::System::Clock::Seconds16(*holdTime), LightingApp::AppTask::OccupancyPresentTimerHandler, nullptr);
    }
    else if (!strcmp(argv[0], "clear"))
    {
        ChipLogDetail(Shell, "OpOccupancy Sensor : Set to %s state", argv[0]);
        LightingApp::AppTask::GetDefaultInstance().OccupancyHandler(false);
    }
    else
    {
        ChipLogError(Shell, "Invalid State to set");
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    return CHIP_NO_ERROR;
}


void LightingApp::AppTask::PreInitMatterStack()
{
    ChipLogProgress(DeviceLayer, "Welcome to NXP Color Temperature Light Demo App");
}

void LightingApp::AppTask::PostInitMatterStack()
{
	CHIP_ERROR err = CHIP_NO_ERROR;
    chip::app::InteractionModelEngine::GetInstance()->RegisterReadHandlerAppCallback(&chip::NXP::App::GetICDUtil());
	if (LightingMgr().Init() != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "Init lighting failed: %s", ErrorStr(err));
    }
	
    LightingMgr().SetCallbacks(ActionInitiated, ActionCompleted);
}

void LightingApp::AppTask::AppMatter_RegisterCustomCliCommands()
{
#ifdef ENABLE_CHIP_SHELL
    /* Register application commands */
    static const shell_command_t kCommands[] = {
        { .cmd_func = occupancyCommandHandler,
          .cmd_name = "occupancy",
          .cmd_help = "Set the occupancy sensor State, present|clear" },
    };
    Engine::Root().RegisterCommands(kCommands, sizeof(kCommands) / sizeof(kCommands[0]));
#endif
}

// This returns an instance of this class.
LightingApp::AppTask & LightingApp::AppTask::GetDefaultInstance()
{
    static LightingApp::AppTask sAppTask;
    return sAppTask;
}

chip::NXP::App::AppTaskBase & chip::NXP::App::GetAppTask()
{
    return LightingApp::AppTask::GetDefaultInstance();
}

void LightingApp::AppTask::ActionInitiated(LightingManager::Action_t aAction, int32_t aActor)
{
    if (aAction == LightingManager::TURNON_ACTION)
    {
        ChipLogProgress(DeviceLayer, "Turn on Action has been initiated");
    }
    else if (aAction == LightingManager::TURNOFF_ACTION)
    {
        ChipLogProgress(DeviceLayer, "Turn off Action has been initiated");
    }

    LightingApp::AppTask::GetDefaultInstance().mFunction = kFunctionTurnOnTurnOff;
}

void LightingApp::AppTask::ActionCompleted(LightingManager::Action_t aAction)
{
    if (aAction == LightingManager::TURNON_ACTION)
    {
        ChipLogProgress(DeviceLayer, "Turn on action has been completed");
    }
    else if (aAction == LightingManager::TURNOFF_ACTION)
    {
        ChipLogProgress(DeviceLayer, "Turn off action has been completed");
    }

    LightingApp::AppTask::GetDefaultInstance().mFunction = kFunction_NoneSelected;
}

void LightingApp::AppTask::UpdateClusterState(void)
{
    uint8_t newValue = !LightingMgr().IsTurnedOff();

    // write the new on/off value
    chip::Protocols::InteractionModel::Status status = app::Clusters::OnOff::Attributes::OnOff::Set(1, newValue);
    if (status != Protocols::InteractionModel::Status::Success)
    {
        ChipLogError(DeviceLayer, "ERR: updating on/off state failed.");
    }
}

void LightingApp::AppTask::OccupancyEventHandler(const AppEvent & aEvent)
{
    if (aEvent.Type != AppEvent::kEventType_Occupancy)
    {
        ChipLogError(DeviceLayer, "A Non Occupancy type received");
        return;
    }

    uint8_t attributeValue = aEvent.OccupancytEvent.Present ? 1 : 0;

    ChipLogProgress(DeviceLayer, "################# Occupancy sensor: %u #################", attributeValue);

	if(attributeValue){
		PlatformMgr().ScheduleWork(UpdateOccupancyPresentStateInternal, 0);
	}
	else{
		PlatformMgr().ScheduleWork(UpdateOccupancyClearStateInternal, 0);
	}
}

void LightingApp::AppTask::UpdateOccupancyPresentStateInternal(intptr_t arg)
{
    uint8_t newValue = 1;
    // write new occupancy value
    Protocols::InteractionModel::Status status = OccupancySensing::Attributes::Occupancy::Set(1, newValue);
    if (status != Protocols::InteractionModel::Status::Success)
    {
        ChipLogError(NotSpecified, "ERR: updating occupancy status value failed");
    }
}

void LightingApp::AppTask::UpdateOccupancyClearStateInternal(intptr_t arg)
{
    uint8_t newValue = 0;
    // write new occupancy value
    Protocols::InteractionModel::Status status = OccupancySensing::Attributes::Occupancy::Set(1, newValue);
    if (status != Protocols::InteractionModel::Status::Success)
    {
        ChipLogError(NotSpecified, "ERR: updating occupancy status value failed");
    }
}

void LightingApp::AppTask::OccupancyHandler(bool present)
{
    AppEvent occupancy_event                = {};
    occupancy_event.Type                    = AppEvent::kEventType_Occupancy;
    occupancy_event.OccupancytEvent.Present = present;
    occupancy_event.Handler                 = OccupancyEventHandler;
    LightingApp::AppTask::GetDefaultInstance().PostEvent(occupancy_event);
}

void LightingApp::AppTask::OccupancyPresentTimerHandler(System::Layer * systemLayer, void * appState)
{
	LightingApp::AppTask::GetDefaultInstance().OccupancyHandler(false);
}