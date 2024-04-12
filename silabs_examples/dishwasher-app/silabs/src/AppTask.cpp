/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2019 Google LLC.
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
#include "AppConfig.h"
#include "AppEvent.h"
#include "operational-state-delegate-impl.h"

#include <app/clusters/on-off-server/on-off-server.h>
#include <app/server/OnboardingCodesUtil.h>
#include <app/server/Server.h>
#include <app/util/attribute-storage.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/attribute-type.h>

#include <assert.h>

#include <platform/silabs/platformAbstraction/SilabsPlatform.h>

#include <setup_payload/QRCodeSetupPayloadGenerator.h>
#include <setup_payload/SetupPayload.h>

#include <lib/support/CodeUtils.h>

#include <platform/CHIPDeviceLayer.h>

#define APP_FUNCTION_BUTTON 0
#define APP_CONTROL_BUTTON 1

using namespace chip;
using namespace chip::app;
using namespace ::chip::DeviceLayer;
using namespace ::chip::DeviceLayer::Silabs;

using namespace chip::TLV;
using namespace ::chip::DeviceLayer;

AppTask AppTask::sAppTask;

CHIP_ERROR AppTask::Init()
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::DeviceLayer::Silabs::GetPlatform().SetButtonsCb(AppTask::ButtonEventHandler);

#ifdef DISPLAY_ENABLED
    GetLCD().Init((uint8_t *) "Dishwasher-App");
#endif

    err = BaseApplication::Init();
    if (err != CHIP_NO_ERROR)
    {
        SILABS_LOG("BaseApplication::Init() failed");
        appError(err);
    }

    err = DishwasherMgr().Init();
    if (err != CHIP_NO_ERROR)
    {
        SILABS_LOG("DishwasherMgr::Init() failed");
        appError(err);
    }

    DishwasherMgr().SetCallbacks(ActionInitiated, ActionCompleted);

// Update the LCD with the Stored value. Show QR Code if not provisioned
#ifdef DISPLAY_ENABLED
    GetLCD().WriteDemoUI((DishwasherMgr().GetOperationalState() == OperationalStateEnum::kRunning));
#ifdef QR_CODE_ENABLED
#ifdef SL_WIFI
    if (!ConnectivityMgr().IsWiFiStationProvisioned())
#else
    if (!ConnectivityMgr().IsThreadProvisioned())
#endif /* !SL_WIFI */
    {
        GetLCD().ShowQRCode(true);
    }
#endif // QR_CODE_ENABLED
#endif

    return err;
}

CHIP_ERROR AppTask::StartAppTask()
{
    return BaseApplication::StartAppTask(AppTaskMain);
}

void AppTask::AppTaskMain(void * pvParameter)
{
    AppEvent event;
    QueueHandle_t sAppEventQueue = *(static_cast<QueueHandle_t *>(pvParameter));

    CHIP_ERROR err = sAppTask.Init();
    if (err != CHIP_NO_ERROR)
    {
        SILABS_LOG("AppTask.Init() failed");
        appError(err);
    }

#if !(defined(CHIP_CONFIG_ENABLE_ICD_SERVER) && CHIP_CONFIG_ENABLE_ICD_SERVER)
    sAppTask.StartStatusLEDTimer();
#endif

    SILABS_LOG("App Task started");

    while (true)
    {
        BaseType_t eventReceived = xQueueReceive(sAppEventQueue, &event, portMAX_DELAY);
        while (eventReceived == pdTRUE)
        {
            sAppTask.DispatchEvent(&event);
            eventReceived = xQueueReceive(sAppEventQueue, &event, 0);
        }
    }
}

void AppTask::DishwasherActionEventHandler(AppEvent * aEvent)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    if (aEvent->Type != AppEvent::kEventType_Button)
    {
        err = APP_ERROR_UNHANDLED_EVENT;
    }

    if (err == CHIP_NO_ERROR)
    {
        DishwasherMgr().CycleOperationalState();
    }
}

void AppTask::ButtonEventHandler(uint8_t button, uint8_t btnAction)
{
    AppEvent button_event           = {};
    button_event.Type               = AppEvent::kEventType_Button;
    button_event.ButtonEvent.Action = btnAction;

    if (button == APP_CONTROL_BUTTON && btnAction == static_cast<uint8_t>(SilabsPlatform::ButtonAction::ButtonPressed))
    {
        button_event.Handler = DishwasherActionEventHandler;
        AppTask::GetAppTask().PostEvent(&button_event);
    }
    else if (button == APP_FUNCTION_BUTTON)
    {
        button_event.Handler = BaseApplication::ButtonHandler;
        AppTask::GetAppTask().PostEvent(&button_event);
    }
}

void AppTask::ActionInitiated(OperationalStateEnum action)
{
    // Action initiated, update the dishwasher state led
    if (action == OperationalStateEnum::kRunning)
    {
        SILABS_LOG("Starting dishwasher");
    }
    else if (action == OperationalStateEnum::kStopped)
    {
        SILABS_LOG("Stoping dishwasher");
    }
    else if (action == OperationalStateEnum::kPaused)
    {
        SILABS_LOG("Pausing dishwasher");
    }
    else
    {
        SILABS_LOG("Action error"); 
        action = OperationalStateEnum::kError;
    }

    PlatformMgr().LockChipStack();
    CHIP_ERROR err = Clusters::OperationalState::GetInstance()->SetOperationalState(to_underlying(action));
    PlatformMgr().UnlockChipStack();
    if (err != CHIP_NO_ERROR)
    {
        SILABS_LOG("ERR: updating Operational state %x", err);
    }
    else
    {
        DishwasherMgr().UpdateOperationState(action);
    } 
}

void AppTask::ActionCompleted()
{
#ifdef DISPLAY_ENABLED
    sAppTask.GetLCD().WriteDemoUI((DishwasherMgr().GetOperationalState() == OperationalStateEnum::kRunning));
#endif
}