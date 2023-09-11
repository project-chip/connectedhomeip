/*
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 * SPDX-FileCopyrightText: 2019 Google LLC.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**********************************************************
 * Includes
 *********************************************************/

#include "AppTask.h"
#include "AppConfig.h"
#include "AppEvent.h"

#include "LEDWidget.h"

#ifdef DISPLAY_ENABLED
#include "ThermostatUI.h"
#include "lcd.h"
#ifdef QR_CODE_ENABLED
#include "qrcodegen.h"
#endif // QR_CODE_ENABLED
#endif // DISPLAY_ENABLED

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/callback.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <app-common/zap-generated/enums.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app/server/OnboardingCodesUtil.h>
#include <app/server/Server.h>
#include <app/util/attribute-storage.h>
#include <assert.h>
#include <lib/support/CodeUtils.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/silabs/platformAbstraction/SilabsPlatform.h>
#include <setup_payload/QRCodeSetupPayloadGenerator.h>
#include <setup_payload/SetupPayload.h>

/**********************************************************
 * Defines and Constants
 *********************************************************/

#define APP_FUNCTION_BUTTON 0
#define APP_THERMOSTAT 1

#define MODE_TIMER 1000 // 1s timer period

using namespace chip;
using namespace chip::app;
using namespace chip::TLV;
using namespace ::chip::DeviceLayer;

/**********************************************************
 * Variable declarations
 *********************************************************/

/**********************************************************
 * AppTask Definitions
 *********************************************************/

AppTask AppTask::sAppTask;

CHIP_ERROR AppTask::Init()
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::DeviceLayer::Silabs::GetPlatform().SetButtonsCb(AppTask::ButtonEventHandler);

#ifdef DISPLAY_ENABLED
    GetLCD().Init((uint8_t *) "Thermostat-App");
    GetLCD().SetCustomUI(ThermostatUI::DrawUI);
#endif

    err = BaseApplication::Init();
    if (err != CHIP_NO_ERROR)
    {
        SILABS_LOG("BaseApplication::Init() failed");
        appError(err);
    }
    err = SensorMgr().Init();
    if (err != CHIP_NO_ERROR)
    {
        SILABS_LOG("SensorMgr::Init() failed");
        appError(err);
    }
    err = TempMgr().Init();
    if (err != CHIP_NO_ERROR)
    {
        SILABS_LOG("TempMgr::Init() failed");
        appError(err);
    }

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

void AppTask::UpdateThermoStatUI()
{
#ifdef DISPLAY_ENABLED
    ThermostatUI::SetMode(TempMgr().GetMode());
    ThermostatUI::SetHeatingSetPoint(TempMgr().GetHeatingSetPoint());
    ThermostatUI::SetCoolingSetPoint(TempMgr().GetCoolingSetPoint());
    ThermostatUI::SetCurrentTemp(TempMgr().GetCurrentTemp());

#ifdef SL_WIFI
    if (ConnectivityMgr().IsWiFiStationProvisioned())
#else
    if (ConnectivityMgr().IsThreadProvisioned())
#endif /* !SL_WIFI */
    {
        AppTask::GetAppTask().GetLCD().WriteDemoUI(false); // State doesn't Matter
    }
#else
    SILABS_LOG("Thermostat Status - M:%d T:%d'C H:%d'C C:%d'C", TempMgr().GetMode(), TempMgr().GetCurrentTemp(),
               TempMgr().GetHeatingSetPoint(), TempMgr().GetCoolingSetPoint());
#endif // DISPLAY_ENABLED
}

void AppTask::ButtonEventHandler(uint8_t button, uint8_t btnAction)
{
    AppEvent aEvent           = {};
    aEvent.Type               = AppEvent::kEventType_Button;
    aEvent.ButtonEvent.Action = btnAction;

    if (button == APP_FUNCTION_BUTTON)
    {
        aEvent.Handler = BaseApplication::ButtonHandler;
        sAppTask.PostEvent(&aEvent);
    }
}
