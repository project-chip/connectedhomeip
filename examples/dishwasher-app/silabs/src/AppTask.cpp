/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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

#include <assert.h>

#include <app-common/zap-generated/attribute-type.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app/server/Server.h>
#include <app/util/attribute-storage.h>
#include <lib/support/CodeUtils.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/silabs/platformAbstraction/SilabsPlatform.h>
#include <setup_payload/OnboardingCodesUtil.h>
#include <setup_payload/QRCodeSetupPayloadGenerator.h>
#include <setup_payload/SetupPayload.h>

#include "AppConfig.h"
#include "AppEvent.h"
#include "AppTask.h"
#include "DeviceEnergyManager.h"
#include "DishwasherManager.h"
#include "ElectricalSensorManager.h"
#include "operational-state-delegate-impl.h"

#include <app/clusters/device-energy-management-server/DeviceEnergyManagementTestEventTriggerHandler.h>
#include <app/clusters/electrical-energy-measurement-server/EnergyReportingTestEventTriggerHandler.h>

#define APP_FUNCTION_BUTTON 0
#define APP_CONTROL_BUTTON 1

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::OperationalState;
using namespace chip::app::Clusters::DeviceEnergyManagement;
using namespace chip::DeviceLayer;
using namespace chip::DeviceLayer::Silabs;
using namespace chip::TLV;

AppTask AppTask::sAppTask;

#if SL_MATTER_TEST_EVENT_TRIGGER_ENABLED
static EnergyReportingTestEventTriggerHandler sEnergyReportingTestEventTriggerHandler;
static DeviceEnergyManagementTestEventTriggerHandler sDeviceEnergyManagementTestEventTriggerHandler;
#endif

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

    PlatformMgr().LockChipStack();
    err = DeviceEnergyManager::Instance().Init();
    PlatformMgr().UnlockChipStack();
    if (err != CHIP_NO_ERROR)
    {
        SILABS_LOG("DeviceEnergyMgr.Init() failed");
        appError(err);
    }

    PlatformMgr().LockChipStack();
    err = ElectricalSensorManager::Instance().Init();
    PlatformMgr().UnlockChipStack();
    if (err != CHIP_NO_ERROR)
    {
        SILABS_LOG("ElectricalSensorMgr.Init() failed");
        appError(err);
    }

#if SL_MATTER_TEST_EVENT_TRIGGER_ENABLED
    // Register DEM & Electrical Sensor Test Event Trigger
    if (Server::GetInstance().GetTestEventTriggerDelegate() != nullptr)
    {
        Server::GetInstance().GetTestEventTriggerDelegate()->AddHandler(&sEnergyReportingTestEventTriggerHandler);
        Server::GetInstance().GetTestEventTriggerDelegate()->AddHandler(&sDeviceEnergyManagementTestEventTriggerHandler);
    }
#endif

    GetDishwasherManager()->SetCallbacks(ActionInitiated, ActionCompleted);

    // Set the initial state of the dishwasher
    OperationalStateEnum state = static_cast<OperationalStateEnum>(OperationalState::GetInstance()->GetCurrentOperationalState());
    GetDishwasherManager()->UpdateOperationState(state);

// Update the LCD with the Stored value. Show QR Code if not provisioned
#ifdef DISPLAY_ENABLED
    GetLCD().WriteDemoUI((GetDishwasherManager()->GetOperationalState() == OperationalStateEnum::kRunning));
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
    osMessageQueueId_t sAppEventQueue = *(static_cast<osMessageQueueId_t *>(pvParameter));

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
        osStatus_t eventReceived = osMessageQueueGet(sAppEventQueue, &event, NULL, osWaitForever);
        while (eventReceived == osOK)
        {
            sAppTask.DispatchEvent(&event);
            eventReceived = osMessageQueueGet(sAppEventQueue, &event, NULL, 0);
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

    if (CHIP_NO_ERROR == err)
    {
        GetDishwasherManager()->CycleOperationalState();
    }
}

void AppTask::ButtonEventHandler(uint8_t button, uint8_t btnAction)
{
    AppEvent button_event           = {};
    button_event.Type               = AppEvent::kEventType_Button;
    button_event.ButtonEvent.Action = btnAction;

    if ((button == APP_CONTROL_BUTTON) && (btnAction == static_cast<uint8_t>(SilabsPlatform::ButtonAction::ButtonPressed)))
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
        GetDishwasherManager()->UpdateOperationState(action);
        ElectricalSensorManager::Instance().UpdateEPMAttributes(action);
    }
}

void AppTask::ActionCompleted()
{
#ifdef DISPLAY_ENABLED
    sAppTask.GetLCD().WriteDemoUI((GetDishwasherManager()->GetOperationalState() == OperationalStateEnum::kRunning));
#endif
}
