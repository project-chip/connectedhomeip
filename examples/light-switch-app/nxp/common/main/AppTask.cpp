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

#include <app/clusters/bindings/BindingManager.h>

#ifdef ENABLE_CHIP_SHELL
#include <lib/shell/Engine.h>
#include <map>
using namespace chip::Shell;
#define MATTER_CLI_LOG(message) (streamer_printf(streamer_get(), message))
#endif /* ENABLE_CHIP_SHELL */

uint8_t switch_cmd_id;

using namespace chip;
using namespace ::chip::DeviceLayer;
using namespace chip::app::Clusters;

static CHIP_ERROR cliLightSwitch(int argc, char * argv[])
{
    if ((argc != 1) && (argc != 2))
    {
        ChipLogError(Shell, "Invalid Argument");
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    if (!strcmp(argv[0], "on"))
    {
        ChipLogDetail(Shell, "OpLightSwitch : Set Light to %s state", argv[0]);
		switch_cmd_id = 0x1;
		LightSwitchApp::AppTask::GetDefaultInstance().NotifyBoundDeviceHandler();
    }
    else if (!strcmp(argv[0], "off"))
    {
        ChipLogDetail(Shell, "OpLightSwitch : Set Light to %s state", argv[0]);
        switch_cmd_id = 0x2;
		LightSwitchApp::AppTask::GetDefaultInstance().NotifyBoundDeviceHandler();
    }
	else if (!strcmp(argv[0], "toggle"))
    {
        ChipLogDetail(Shell, "OpLightSwitch : Toggle Light");
        switch_cmd_id = 0x3;
		LightSwitchApp::AppTask::GetDefaultInstance().NotifyBoundDeviceHandler();
    }
    else
    {
        ChipLogError(Shell, "Invalid State to set");
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    return CHIP_NO_ERROR;
}


void LightSwitchApp::AppTask::PreInitMatterStack()
{
    ChipLogProgress(DeviceLayer, "Welcome to NXP OnOff Light Switch Demo App");
}

void LightSwitchApp::AppTask::PostInitMatterStack()
{
    chip::app::InteractionModelEngine::GetInstance()->RegisterReadHandlerAppCallback(&chip::NXP::App::GetICDUtil());
}

void LightSwitchApp::AppTask::AppMatter_RegisterCustomCliCommands()
{
#ifdef ENABLE_CHIP_SHELL
    /* Register application commands */
    static const shell_command_t kCommands[] = {
        { .cmd_func = cliLightSwitch,
          .cmd_name = "lightswitch",
          .cmd_help = "Get/Set the light State, on|off|toggle " },
    };
    Engine::Root().RegisterCommands(kCommands, sizeof(kCommands) / sizeof(kCommands[0]));
#endif
}

// This returns an instance of this class.
LightSwitchApp::AppTask & LightSwitchApp::AppTask::GetDefaultInstance()
{
    static LightSwitchApp::AppTask sAppTask;
    return sAppTask;
}

chip::NXP::App::AppTaskBase & chip::NXP::App::GetAppTask()
{
    return LightSwitchApp::AppTask::GetDefaultInstance();
}

void LightSwitchApp::AppTask::NotifyBoundDevice(intptr_t arg)
{
    CHIP_ERROR error;
    BindingCommandData * data = chip::Platform::New<BindingCommandData>();

    switch (switch_cmd_id) {
    case 1 :
        PRINTF("-->  Turn on light\r\n");
        data->localEndpointId = 1;
        data->clusterId   = chip::app::Clusters::OnOff::Id;
        data->commandId   = chip::app::Clusters::OnOff::Commands::On::Id;
        break;
    case 2 :
        PRINTF("--> Turn off light\r\n");
        data->localEndpointId = 1;
        data->clusterId   = chip::app::Clusters::OnOff::Id;
        data->commandId   = chip::app::Clusters::OnOff::Commands::Off::Id;
        break;
    case 3 :
        data->localEndpointId = 1;
        data->clusterId = chip::app::Clusters::OnOff::Id;
        data->commandId = chip::app::Clusters::OnOff::Commands::Toggle::Id;
        break;
    default :
        break;
    }

    error = chip::BindingManager::GetInstance().NotifyBoundClusterChanged(data->localEndpointId,
                                                                          data->clusterId,
                                                                          static_cast<void *>(data));
    if (CHIP_NO_ERROR != error) {
        ChipLogProgress(DeviceLayer, "Matter command to bound device fail");
    } else {
        ChipLogProgress(DeviceLayer, "Matter command to bound device succeed");
    }
}

void LightSwitchApp::AppTask::NotifyBoundDeviceHandler(void)
{
    PlatformMgr().ScheduleWork(NotifyBoundDevice, 0);
}