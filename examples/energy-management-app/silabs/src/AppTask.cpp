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
#include "LEDWidget.h"
#include <EnergyManagementAppCommonMain.h>
#include <app-common/zap-generated/cluster-enums.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/ConcreteAttributePath.h>
#include <app/clusters/network-commissioning/network-commissioning.h>
#include <app/server/Server.h>
#include <app/util/attribute-storage.h>
#include <app/util/endpoint-config-api.h>
#include <assert.h>
#include <lib/support/BitMask.h>
#include <setup_payload/OnboardingCodesUtil.h>

#include <platform/silabs/platformAbstraction/SilabsPlatform.h>

#include <setup_payload/QRCodeSetupPayloadGenerator.h>
#include <setup_payload/SetupPayload.h>

#include <lib/support/CodeUtils.h>

#include <EnergyManagementAppCmdLineOptions.h>
#include <platform/CHIPDeviceLayer.h>

#ifdef SL_MATTER_TEST_EVENT_TRIGGER_ENABLED
#include <app/TestEventTriggerDelegate.h>

#if SL_MATTER_CONFIG_ENABLE_ENERGY_EVSE_TRIGGER
#include <app/clusters/energy-evse-server/EnergyEvseTestEventTriggerHandler.h>
#endif
#if SL_MATTER_CONFIG_ENABLE_ENERGY_REPORTING_TRIGGER
#include <app/clusters/electrical-energy-measurement-server/EnergyReportingTestEventTriggerHandler.h>
#endif
#if SL_MATTER_CONFIG_ENABLE_WATER_HEATER_MANAGEMENT_TRIGGER
#include <app/clusters/water-heater-management-server/WaterHeaterManagementTestEventTriggerHandler.h>
#endif
#if SL_MATTER_CONFIG_ENABLE_DEVICE_ENERGY_MANAGEMENT_TRIGGER
#include <app/clusters/device-energy-management-server/DeviceEnergyManagementTestEventTriggerHandler.h>
#endif

#endif // SL_MATTER_TEST_EVENT_TRIGGER_ENABLED

#if (defined(SL_CATALOG_SIMPLE_LED_LED1_PRESENT) || defined(SIWX_917))
#define EVSE_LED 1
#else
#define EVSE_LED 0
#endif

#define APP_FUNCTION_BUTTON 0
#define APP_EVSE_SWITCH 1

namespace {

LEDWidget sEnergyManagementLED;
constexpr chip::EndpointId kEvseEndpoint        = 1;
constexpr chip::EndpointId kWaterHeaterEndpoint = 2;

} // namespace

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::DeviceEnergyManagement;
using namespace chip::app::Clusters::DeviceEnergyManagement::Attributes;
using namespace chip::app::Clusters::WaterHeaterManagement;
using namespace ::chip::DeviceLayer;
using namespace ::chip::DeviceLayer::Silabs;
using namespace ::chip::DeviceLayer::Internal;
using namespace chip::TLV;

#ifdef SL_MATTER_TEST_EVENT_TRIGGER_ENABLED
static EnergyEvseTestEventTriggerHandler sEnergyEvseTestEventTriggerHandler;
static EnergyReportingTestEventTriggerHandler sEnergyReportingTestEventTriggerHandler;
static DeviceEnergyManagementTestEventTriggerHandler sDeviceEnergyManagementTestEventTriggerHandler;
#endif

namespace chip {
namespace app {
namespace Clusters {
namespace DeviceEnergyManagement {

// Keep track of the parsed featureMap option
#if (SL_MATTER_CONFIG_DEM_SUPPORT_POWER_FORECAST_REPORTING) && (SL_MATTER_CONFIG_DEM_SUPPORT_STATE_FORECAST_REPORTING)
#error Cannot define SL_MATTER_CONFIG_DEM_SUPPORT_POWER_FORECAST_REPORTING and SL_MATTER_CONFIG_DEM_SUPPORT_STATE_FORECAST_REPORTING
#endif

#if SL_MATTER_CONFIG_DEM_SUPPORT_POWER_FORECAST_REPORTING
static chip::BitMask<Feature> sFeatureMap(Feature::kPowerAdjustment, Feature::kPowerForecastReporting,
                                          Feature::kStartTimeAdjustment, Feature::kPausable, Feature::kForecastAdjustment,
                                          Feature::kConstraintBasedAdjustment);
#elif SL_MATTER_CONFIG_DEM_SUPPORT_STATE_FORECAST_REPORTING
static chip::BitMask<Feature> sFeatureMap(Feature::kPowerAdjustment, Feature::kStateForecastReporting,
                                          Feature::kStartTimeAdjustment, Feature::kPausable, Feature::kForecastAdjustment,
                                          Feature::kConstraintBasedAdjustment);
#else
static chip::BitMask<Feature> sFeatureMap(Feature::kPowerAdjustment);
#endif

chip::BitMask<Feature> GetFeatureMapFromCmdLine()
{
    return sFeatureMap;
}

} // namespace DeviceEnergyManagement
} // namespace Clusters
} // namespace app
} // namespace chip

AppTask AppTask::sAppTask;

EndpointId GetEnergyDeviceEndpointId()
{
#if SL_CONFIG_ENABLE_EXAMPLE_WATER_HEATER_DEVICE
    return kWaterHeaterEndpoint;
#else
    return kEvseEndpoint;
#endif
}

void ApplicationInit()
{
    chip::DeviceLayer::PlatformMgr().LockChipStack();
    SILABS_LOG("==================================================");
#if SL_MATTER_CONFIG_ENABLE_EXAMPLE_EVSE_DEVICE
    SILABS_LOG("energy-management-example EVSE starting. featureMap 0x%08lx", DeviceEnergyManagement::sFeatureMap.Raw());

    EvseApplicationInit();
    // Disable Water Heater Endpoint
    emberAfEndpointEnableDisable(kWaterHeaterEndpoint, false);
#endif // CONFIG_ENABLE_EXAMPLE_EVSE_DEVICE

#if SL_CONFIG_ENABLE_EXAMPLE_WATER_HEATER_DEVICE
    SILABS_LOG("energy-management-example WaterHeater starting. featureMap 0x%08lx", DeviceEnergyManagement::sFeatureMap.Raw());

    WaterHeaterApplicationInit();
    // Disable EVSE Endpoint
    emberAfEndpointEnableDisable(kEvseEndpoint, false);
#endif // CONFIG_ENABLE_EXAMPLE_WATER_HEATER_DEVICE
    SILABS_LOG("==================================================");

    sEnergyManagementLED.Init(EVSE_LED);
    chip::DeviceLayer::PlatformMgr().UnlockChipStack();
}
void ApplicationShutdown()
{
    chip::DeviceLayer::PlatformMgr().LockChipStack();
#if SL_MATTER_CONFIG_ENABLE_EXAMPLE_EVSE_DEVICE
    EvseApplicationShutdown();
#endif // CONFIG_ENABLE_EXAMPLE_EVSE_DEVICE

#if SL_CONFIG_ENABLE_EXAMPLE_WATER_HEATER_DEVICE
    WaterHeaterApplicationShutdown();
#endif // CONFIG_ENABLE_EXAMPLE_WATER_HEATER_DEVICE
    chip::DeviceLayer::PlatformMgr().UnlockChipStack();
}

CHIP_ERROR AppTask::Init()
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::DeviceLayer::Silabs::GetPlatform().SetButtonsCb(AppTask::ButtonEventHandler);

#ifdef DISPLAY_ENABLED
#if SL_MATTER_CONFIG_ENABLE_EXAMPLE_EVSE_DEVICE
    GetLCD().Init((uint8_t *) "energy-management-App (EVSE)");
#elif SL_CONFIG_ENABLE_EXAMPLE_WATER_HEATER_DEVICE
    GetLCD().Init((uint8_t *) "energy-management-App (WaterHeater)");
#endif
#endif

    err = BaseApplication::Init();
    if (err != CHIP_NO_ERROR)
    {
        SILABS_LOG("BaseApplication::Init() failed");
        appError(err);
    }

    ApplicationInit();

#ifdef SL_MATTER_TEST_EVENT_TRIGGER_ENABLED
    TestEventTriggerDelegate * pTestEventDelegate = Server::GetInstance().GetTestEventTriggerDelegate();

#if SL_MATTER_CONFIG_ENABLE_ENERGY_EVSE_TRIGGER
    static EnergyEvseTestEventTriggerHandler sEnergyEvseTestEventTriggerHandler;
    if (pTestEventDelegate != nullptr)
    {
        VerifyOrDie(pTestEventDelegate->AddHandler(&sEnergyEvseTestEventTriggerHandler) == CHIP_NO_ERROR);
    }
#endif

#if SL_MATTER_CONFIG_ENABLE_ENERGY_REPORTING_TRIGGER
    static EnergyReportingTestEventTriggerHandler sEnergyReportingTestEventTriggerHandler;
    if (pTestEventDelegate != nullptr)
    {
        VerifyOrDie(pTestEventDelegate->AddHandler(&sEnergyReportingTestEventTriggerHandler) == CHIP_NO_ERROR);
    }

#endif
#if SL_MATTER_CONFIG_ENABLE_WATER_HEATER_MANAGEMENT_TRIGGER
    static WaterHeaterManagementTestEventTriggerHandler sWaterHeaterManagementTestEventTriggerHandler;

    if (pTestEventDelegate != nullptr)
    {
        VerifyOrDie(pTestEventDelegate->AddHandler(&sWaterHeaterManagementTestEventTriggerHandler) == CHIP_NO_ERROR);
    }
#endif
#if SL_MATTER_CONFIG_ENABLE_DEVICE_ENERGY_MANAGEMENT_TRIGGER
    static DeviceEnergyManagementTestEventTriggerHandler sDeviceEnergyManagementTestEventTriggerHandler;
    if (pTestEventDelegate != nullptr)
    {
        VerifyOrDie(pTestEventDelegate->AddHandler(&sDeviceEnergyManagementTestEventTriggerHandler) == CHIP_NO_ERROR);
    }
#endif

#endif // SL_MATTER_TEST_EVENT_TRIGGER_ENABLED

// Update the LCD with the Stored value. Show QR Code if not provisioned
#ifdef DISPLAY_ENABLED
    GetLCD().WriteDemoUI(LightMgr().IsLightOn());
#ifdef QR_CODE_ENABLED
#ifdef SL_WIFI
    if (!chip::DeviceLayer::ConnectivityMgr().IsWiFiStationProvisioned())
#else
    if (!chip::DeviceLayer::ConnectivityMgr().IsThreadProvisioned())
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

void AppTask::EnergyManagementActionEventHandler(AppEvent * aEvent)
{
    bool initiated = false;
    int32_t actor;
    CHIP_ERROR err = CHIP_NO_ERROR;

    if (aEvent->Type == AppEvent::kEventType_Button)
    {
        actor = AppEvent::kEventType_Button;
        SILABS_LOG("button event %d ", actor);
    }
    else
    {
        err = APP_ERROR_UNHANDLED_EVENT;
    }

    if (err == CHIP_NO_ERROR)
    {
        if (!initiated)
        {
            SILABS_LOG("Action is already in progress or active.");
        }
    }
}

void AppTask::ButtonEventHandler(uint8_t button, uint8_t btnAction)
{
    AppEvent button_event           = {};
    button_event.Type               = AppEvent::kEventType_Button;
    button_event.ButtonEvent.Action = btnAction;

    if (button == APP_EVSE_SWITCH && btnAction == static_cast<uint8_t>(SilabsPlatform::ButtonAction::ButtonPressed))
    {
        button_event.Handler = EnergyManagementActionEventHandler;
        AppTask::GetAppTask().PostEvent(&button_event);
    }
    else if (button == APP_FUNCTION_BUTTON)
    {
        button_event.Handler = BaseApplication::ButtonHandler;
        AppTask::GetAppTask().PostEvent(&button_event);
    }
}
