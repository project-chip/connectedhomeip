/*
 *
 * SPDX-FileCopyrightText: 2022-2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "AppTask.h"
#include "binding-handler.h"

LOG_MODULE_DECLARE(app, CONFIG_CHIP_APP_LOG_LEVEL);

AppTask AppTask::sAppTask;

CHIP_ERROR AppTask::Init(void)
{
#if APP_USE_EXAMPLE_START_BUTTON
    SetExampleButtonCallbacks(SwitchActionEventHandler);
#endif
    InitCommonParts();

    // Configure Bindings
    CHIP_ERROR err = InitBindingHandler();
    if (err != CHIP_NO_ERROR)
    {
        LOG_ERR("InitBindingHandler fail");
        return err;
    }

    err = ConnectivityMgr().SetBLEDeviceName("TelinkSwitch");
    if (err != CHIP_NO_ERROR)
    {
        LOG_ERR("SetBLEDeviceName fail");
        return err;
    }

    return CHIP_NO_ERROR;
}

void AppTask::SwitchActionEventHandler(AppEvent * aEvent)
{
    if (aEvent->Type == AppEvent::kEventType_Button)
    {
        BindingCommandData * data = chip::Platform::New<BindingCommandData>();
        data->commandId           = chip::app::Clusters::OnOff::Commands::Toggle::Id;
        data->clusterId           = chip::app::Clusters::OnOff::Id;

        PlatformMgr().ScheduleWork(SwitchWorkerFunction, reinterpret_cast<intptr_t>(data));
    }
}

void AppTask::UpdateClusterState() {}
