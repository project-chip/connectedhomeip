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
#include "binding-handler.h"

LOG_MODULE_DECLARE(app, CONFIG_CHIP_APP_LOG_LEVEL);

AppTask AppTask::sAppTask;

CHIP_ERROR AppTask::Init(void)
{
    SetExampleButtonCallbacks(SwitchActionEventHandler);
    InitCommonParts();

    // Configure Bindings
    CHIP_ERROR err = InitBindingHandler();
    if (err != CHIP_NO_ERROR)
    {
        LOG_ERR("InitBindingHandler fail");
        return err;
    }

    return CHIP_NO_ERROR;
}

void AppTask::SwitchActionEventHandler(AppEvent * aEvent)
{
    if (aEvent->Type == AppEvent::kEventType_Button)
    {
        LOG_INF("App button pressed");

        BindingCommandData * data = chip::Platform::New<BindingCommandData>();
        data->commandId           = chip::app::Clusters::OnOff::Commands::Toggle::Id;
        data->clusterId           = chip::app::Clusters::OnOff::Id;

        PlatformMgr().ScheduleWork(SwitchWorkerFunction, reinterpret_cast<intptr_t>(data));
    }
}

void AppTask::UpdateClusterState() {}
