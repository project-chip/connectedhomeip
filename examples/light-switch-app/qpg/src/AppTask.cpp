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

#include <lib/support/CHIPMem.h>

#include "binding-handler.h"
#include "gpSched.h"

#include "AppConfig.h"
#include "AppEvent.h"
#include "AppTask.h"
#include "App_Battery.h"
#include "ota.h"

#include "ButtonHandler.h"
#include "StatusLed.h"
#include "qPinCfg.h"

#if defined(GP_UPGRADE_DIVERSITY_DUAL_BOOT)
#include "combo-handler.h"
#endif // GP_UPGRADE_DIVERSITY_DUAL_BOOT

#include "DiagnosticLogsProviderDelegateImpl.h"
#include <app/server/Server.h>

using namespace ::chip;
using namespace ::chip::app;
using namespace ::chip::TLV;
using namespace ::chip::Credentials;
using namespace ::chip::DeviceLayer;
using namespace chip::app::Clusters::DiagnosticLogs;

#include <platform/CHIPDeviceLayer.h>

#define SWITCH_MULTIPRESS_WINDOW_MS 500
#define SWITCH_LONGPRESS_WINDOW_MS 3000

bool sSwitchButtonPressed = false;
bool sIsMultipressOngoing = false;
bool sLongPressDetected   = false;

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

    // Setup button handler
    ButtonHandler_Init(buttons, Q_ARRAY_SIZE(buttons), BUTTON_LOW, AppTask::ButtonEventHandlerWrapper);

    PlatformMgr().ScheduleWork(InitBindingManager);

    SwitchMgr().Init();

    gpSched_ScheduleEvent(1 * ONE_SECOND_US, Application_StartPeriodicBatteryUpdate);

    return err;
}

void AppTask::InitServer(intptr_t arg)
{
    BaseAppTask::InitServer(arg);

    // Open commissioning after boot if no fabric was available
    if (chip::Server::GetInstance().GetFabricTable().FabricCount() == 0)
    {
        PlatformMgr().ScheduleWork(OpenCommissioning, 0);
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
    if (btnIdx != APP_LEVEL_BUTTON && btnIdx != APP_SWITCH_BUTTON && btnIdx != APP_COLOR_BUTTON && btnIdx != APP_TOGGLE_BUTTON)
    {
        return false;
    }

    CHIP_ERROR err = CHIP_NO_ERROR;

    ChipLogDetail(NotSpecified, "ButtonEventHandler %d, %d", btnIdx, btnPressed);

    AppEvent button_event              = {};
    button_event.Type                  = AppEvent::kEventType_Button;
    button_event.ButtonEvent.ButtonIdx = btnIdx;
    button_event.ButtonEvent.Action    = btnPressed;

    if (btnIdx == APP_LEVEL_BUTTON && btnPressed == true)
    {
        ChipLogProgress(NotSpecified, "Level Button pressed");
        button_event.Handler = SwitchMgr().LevelHandler;
    }
    else if (btnIdx == APP_SWITCH_BUTTON)
    {
        if (!btnPressed)
        {
            ChipLogDetail(NotSpecified, "Switch button released");

            button_event.Handler =
                sLongPressDetected ? SwitchMgr().GenericSwitchLongReleaseHandler : SwitchMgr().GenericSwitchShortReleaseHandler;

            sIsMultipressOngoing = true;
            sSwitchButtonPressed = false;
            sLongPressDetected   = false;

            chip::DeviceLayer::SystemLayer().CancelTimer(MultiPressTimeoutHandler, NULL);
            // we start the MultiPress feature window after releasing the button
            err = chip::DeviceLayer::SystemLayer().StartTimer(chip::System::Clock::Milliseconds32(SWITCH_MULTIPRESS_WINDOW_MS),
                                                              MultiPressTimeoutHandler, NULL);

            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(NotSpecified, "StartTimer failed %s: ", chip::ErrorStr(err));
            }
        }
        else
        {
            ChipLogDetail(NotSpecified, "Switch button pressed");

            sSwitchButtonPressed = true;

            chip::DeviceLayer::SystemLayer().CancelTimer(LongPressTimeoutHandler, NULL);
            // we need to check if this is short or long press
            err = chip::DeviceLayer::SystemLayer().StartTimer(chip::System::Clock::Milliseconds32(SWITCH_LONGPRESS_WINDOW_MS),
                                                              LongPressTimeoutHandler, NULL);

            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(NotSpecified, "StartTimer failed %s: ", chip::ErrorStr(err));
            }

            // if we have active multipress window we need to send extra event
            if (sIsMultipressOngoing)
            {
                ChipLogDetail(NotSpecified, "Switch MultipressOngoing");
                button_event.Handler = SwitchMgr().GenericSwitchInitialPressHandler;
                sAppTask.PostEvent(&button_event);
                chip::DeviceLayer::SystemLayer().CancelTimer(MultiPressTimeoutHandler, NULL);
                button_event.Handler = SwitchMgr().GenericSwitchMultipressOngoingHandler;
            }
            else
            {
                button_event.Handler = SwitchMgr().GenericSwitchInitialPressHandler;
            }
        }
    }
    else if (btnIdx == APP_COLOR_BUTTON && btnPressed == true)
    {
        ChipLogProgress(NotSpecified, "Color button pressed");
        button_event.Handler = SwitchMgr().ColorHandler;
    }
    else if (btnIdx == APP_TOGGLE_BUTTON && btnPressed == true)
    {
        ChipLogProgress(NotSpecified, "Toggle Button pressed");
        button_event.Handler = SwitchMgr().ToggleHandler;
    }
    else
    {
        return false;
    }

    sAppTask.PostEvent(&button_event);

    return true;
}

void AppTask::MultiPressTimeoutHandler(chip::System::Layer * aLayer, void * aAppState)
{
    ChipLogDetail(NotSpecified, "MultiPressTimeoutHandler");

    sIsMultipressOngoing = false;

    AppEvent multipress_event = {};
    multipress_event.Type     = AppEvent::kEventType_Button;
    multipress_event.Handler  = SwitchMgr().GenericSwitchMultipressCompleteHandler;

    sAppTask.PostEvent(&multipress_event);
}

void AppTask::LongPressTimeoutHandler(chip::System::Layer * aLayer, void * aAppState)
{
    ChipLogDetail(NotSpecified, "LongPressTimeoutHandler");

    // if the button is still pressed after threshold time, this is a LongPress, otherwise jsut ignore it
    if (sSwitchButtonPressed == true && sIsMultipressOngoing == false)
    {
        sLongPressDetected       = true;
        AppEvent longpress_event = {};
        longpress_event.Type     = AppEvent::kEventType_Button;
        longpress_event.Handler  = SwitchMgr().GenericSwitchLongPressHandler;

        sAppTask.PostEvent(&longpress_event);
    }
}
