/*
 *
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "AppTask.h"
#include "SmokeCoAlarmManager.h"
#include "platform/CHIPDeviceLayer.h"

LOG_MODULE_DECLARE(app, CONFIG_CHIP_APP_LOG_LEVEL);

AppTask AppTask::sAppTask;

CHIP_ERROR AppTask::Init(void)
{
#if APP_USE_EXAMPLE_START_BUTTON
    SetExampleButtonCallbacks(SelfTestEventHandler);
#endif
    InitCommonParts();

    CHIP_ERROR err = AlarmMgr().Init();
    if (err != CHIP_NO_ERROR)
    {
        LOG_ERR("AlarmMgr::Init() failed");
        return err;
    }

    err = ConnectivityMgr().SetBLEDeviceName("TelinkCOSensor");
    if (err != CHIP_NO_ERROR)
    {
        LOG_ERR("SetBLEDeviceName fail");
        return err;
    }

    return CHIP_NO_ERROR;
}

void AppTask::SelfTestHandler(AppEvent * aEvent)
{
    chip::DeviceLayer::PlatformMgr().LockChipStack();
    bool success = SmokeCoAlarmServer::Instance().RequestSelfTest(1);
    chip::DeviceLayer::PlatformMgr().UnlockChipStack();
    if (!success)
    {
        LOG_ERR("Manual self-test failed");
    }
}

void AppTask::SelfTestEventHandler(AppEvent * aEvent)
{
    AppEvent event;
    if (aEvent->Type == AppEvent::kEventType_Button)
    {
        event.ButtonEvent.Action = kButtonPushEvent;
        event.Handler            = SelfTestHandler;
        GetAppTask().PostEvent(&event);
    }
}
