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

#ifdef DIC_ENABLE
#define DECIMAL 10
#define BYTE 5
#include "dic.h"
#endif // DIC_ENABLE

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

#ifdef DIC_ENABLE
namespace ThermAttr = chip::app::Clusters::Thermostat::Attributes;
#endif // DIC_ENABLE

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

#ifdef DIC_ENABLE
void AppTask::DIC_AttrubiteHandler(AttributeId attributeId){
    switch (attributeId)
    {
    case ThermAttr::LocalTemperature::Id: {
        int8_t CurrentTemp = TempMgr().GetCurrentTemp();
        char buffer[BYTE];
        itoa(CurrentTemp, buffer, DECIMAL);
        dic_sendmsg("LocalTemperature/Temp",(const char *)(buffer));
    }
    break;
    case ThermAttr::OccupiedCoolingSetpoint::Id: {
        int8_t coolingTemp =  TempMgr().GetCoolingSetPoint();
        char buffer[BYTE];
        itoa(coolingTemp, buffer, DECIMAL);
        dic_sendmsg("OccupiedCoolingSetpoint/coolingTemp",(const char *)(buffer));
    }
    break;
    case ThermAttr::OccupiedHeatingSetpoint::Id: {
        int8_t heatingTemp = TempMgr().GetHeatingSetPoint();
        char buffer[BYTE];
        itoa(heatingTemp, buffer, DECIMAL);
        dic_sendmsg("OccupiedHeatingSetpoint/heatingTemp", (const char *)(buffer));
    }
    break;
    case ThermAttr::SystemMode::Id: {
        int8_t mode = TempMgr().GetMode();
        char buffer[BYTE];
        const char* Mode;
        switch (mode){
            case 0:
                Mode = "OFF";
                break;
            case 1:
                Mode = "HEAT&COOL";
                break;
            case 3:
                Mode ="COOL";
                break;
            case 4:
                Mode ="HEAT";
                break;
            default:
                Mode = "INVALID MODE";
                break;
        }
        uint16_t current_temp = TempMgr().GetCurrentTemp();
        itoa(current_temp, buffer, DECIMAL);
        dic_sendmsg("thermostat/systemMode", Mode);
        dic_sendmsg("thermostat/currentTemp", (const char *)(buffer));
    }
    break;

    default: {
        SILABS_LOG("Unhandled thermostat attribute %x", attributeId);
        return;
    }
    break;
    }
}
#endif // DIC_ENABLE