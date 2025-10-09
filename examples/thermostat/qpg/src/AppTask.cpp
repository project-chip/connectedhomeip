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

#include "gpSched.h"
#include "powercycle_counting.h"

#include "AppConfig.h"
#include "AppEvent.h"
#include "AppTask.h"
#include "ota.h"

#include "App_Battery.h"
#include "ButtonHandler.h"
#include "StatusLed.h"
#include "ThermostaticRadiatorValveManager.h"
#include "qPinCfg.h"

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app/server/Server.h>

using namespace ::chip;
using namespace ::chip::app;
using namespace ::chip::TLV;
using namespace ::chip::Credentials;
using namespace ::chip::DeviceLayer;

#include <platform/CHIPDeviceLayer.h>

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

    err = ThermostaticRadiatorValveMgr().Init();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(NotSpecified, "ThermostaticRadiatorValveMgr().Init() failed");
        return err;
    }

    gpSched_ScheduleEvent(1 * ONE_SECOND_US, Application_StartPeriodicBatteryUpdate);

    return err;
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
    if (btnIdx != APP_READ_TEMPERATURE_BUTTON)
    {
        return false;
    }

    ChipLogProgress(NotSpecified, "ButtonEventHandler %d, %d", btnIdx, btnPressed);

    AppEvent button_event              = {};
    button_event.Type                  = AppEvent::kEventType_Button;
    button_event.ButtonEvent.ButtonIdx = btnIdx;
    button_event.ButtonEvent.Action    = btnPressed;

    if (btnIdx == APP_READ_TEMPERATURE_BUTTON && btnPressed == true)
    {
        ThermostaticRadiatorValveMgr().DisplayTemperature();
    }
    else
    {
        return false;
    }

    sAppTask.PostEvent(&button_event);

    return true;
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
