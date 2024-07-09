/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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
#include "ButtonManager.h"
#include "WindowCovering.h"

LOG_MODULE_DECLARE(app, CONFIG_CHIP_APP_LOG_LEVEL);

namespace {
constexpr int kToggleMoveTypeTriggerTimeout = 700;

k_timer sToggleMoveTypeTimer;

bool sIsToggleMoveTypeTimerActive = false;
} // namespace

AppTask AppTask::sAppTask;

CHIP_ERROR AppTask::Init(void)
{
    InitCommonParts();

    // Initialize ToggleMoveType timer
    k_timer_init(&sToggleMoveTypeTimer, &AppTask::OpenTimerTimeoutCallback, nullptr);
    k_timer_user_data_set(&sToggleMoveTypeTimer, this);

    return CHIP_NO_ERROR;
}

void AppTask::OpenActionAndToggleMoveTypeButtonEventHandler(void)
{
    AppEvent event;

    event.Type               = AppEvent::kEventType_Button;
    event.ButtonEvent.Action = kButtonPushEvent;
    event.Handler            = ToggleMoveTypeHandler;
    sAppTask.PostEvent(&event);
}

void AppTask::CloseActionButtonEventHandler(void)
{
    AppEvent event;

    event.Type               = AppEvent::kEventType_Button;
    event.ButtonEvent.Action = kButtonPushEvent;
    event.Handler            = CloseHandler;
    sAppTask.PostEvent(&event);
}

void AppTask::OpenTimerTimeoutCallback(k_timer * timer)
{
    if (!timer)
    {
        return;
    }

    AppEvent event;
    event.Type    = AppEvent::kEventType_Timer;
    event.Handler = OpenTimerEventHandler;
    sAppTask.PostEvent(&event);
}

void AppTask::OpenTimerEventHandler(AppEvent * aEvent)
{
    if (aEvent->Type != AppEvent::kEventType_Timer)
    {
        return;
    }

    sIsToggleMoveTypeTimerActive = false;

    OpenHandler(aEvent);
}

void AppTask::ToggleMoveTypeHandler(AppEvent * aEvent)
{
    if (!sIsToggleMoveTypeTimerActive)
    {
        k_timer_start(&sToggleMoveTypeTimer, K_MSEC(kToggleMoveTypeTriggerTimeout), K_NO_WAIT);
        sIsToggleMoveTypeTimerActive = true;
    }
    else
    {
        k_timer_stop(&sToggleMoveTypeTimer);
        sIsToggleMoveTypeTimerActive = false;

        sAppTask.ToggleMoveType();
    }
}

void AppTask::OpenHandler(AppEvent * aEvent)
{
    WindowCovering::Instance().SetSingleStepTarget(OperationalState::MovingUpOrOpen);
}

void AppTask::CloseHandler(AppEvent * aEvent)
{
    WindowCovering::Instance().SetSingleStepTarget(OperationalState::MovingDownOrClose);
}

void AppTask::ToggleMoveType()
{
    if (WindowCovering::Instance().GetMoveType() == WindowCoveringType::Lift)
    {
        WindowCovering::Instance().SetMoveType(WindowCoveringType::Tilt);
        LOG_INF("Window covering move: tilt");
    }
    else
    {
        WindowCovering::Instance().SetMoveType(WindowCoveringType::Lift);
        LOG_INF("Window covering move: lift");
    }
}

void AppTask::LinkButtons(ButtonManager & buttonManager)
{
    buttonManager.addCallback(FactoryResetButtonEventHandler, 0, true);
    buttonManager.addCallback(ExampleActionButtonEventHandler, 1, true);
    buttonManager.addCallback(OpenActionAndToggleMoveTypeButtonEventHandler, 2, true);
    buttonManager.addCallback(CloseActionButtonEventHandler, 3, true);
}
