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
#include "CustomerAppTask.h"
#include "ThermostatConfig.h"

#if SL_MATTER_DISPLAY_ENABLED
#include "ThermostatUI.h"
#include "lcd.h"
#endif // SL_MATTER_DISPLAY_ENABLED

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/callback.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/clusters/thermostat-server/ThermostatCluster.h>
#include <app/server/Server.h>
#include <app/util/attribute-storage.h>
#include <thermostat-delegate-impl.h>
#include <cmsis_os2.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/TypeTraits.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/PlatformError.h>
#include <platform/silabs/platformAbstraction/SilabsPlatform.h>

#if defined(SL_MATTER_USE_SI70XX_SENSOR) && SL_MATTER_USE_SI70XX_SENSOR
#include "Si70xxSensor.h"
#endif // defined(SL_MATTER_USE_SI70XX_SENSOR) && SL_MATTER_USE_SI70XX_SENSOR

#ifdef SL_MATTER_ENABLE_AWS
#include "MatterAwsControl.h"
#endif // SL_MATTER_ENABLE_AWS

#define APP_FUNCTION_BUTTON 0

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace ::chip::DeviceLayer;

namespace ThermAttr = chip::app::Clusters::Thermostat::Attributes;

namespace {

CustomerAppTask & AppInstance()
{
    return CustomerAppTask::GetAppTask();
}

// Defaults live in ThermostatConfig.h; consumers tune via the Configuration Wizard.
constexpr EndpointId kThermostatEndpoint = THERMOSTAT_ENDPOINT;
constexpr uint16_t kSensorTimerPeriodMs  = SENSOR_TIMER_PERIOD_MS;
constexpr uint16_t kMinTemperatureDelta  = MIN_TEMPERATURE_DELTA;

osTimerId_t sSensorTimer = nullptr;

int8_t ConvertToPrintableTemp(int16_t temperature)
{
    constexpr uint8_t kRoundUpValue = 50;

    // Round up the temperature as we won't print decimals on LCD.
    if (temperature < 0)
    {
        temperature -= kRoundUpValue;
    }
    else
    {
        temperature += kRoundUpValue;
    }

    return static_cast<int8_t>(temperature / 100);
}

} // namespace

CHIP_ERROR AppTask::AppInit()
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::DeviceLayer::Silabs::GetPlatform().SetButtonsCb(&CustomerAppTask::ButtonEventHandler);

#if SL_MATTER_DISPLAY_ENABLED
    GetLCD().SetCustomUI(ThermostatUI::DrawUI);
#endif

    err = AppInstance().InitThermostat();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "InitThermostat() failed: %" CHIP_ERROR_FORMAT, err.Format());
        appError(err);
    }

    return err;
}

CHIP_ERROR AppTask::InitThermostat()
{
    sSensorTimer = osTimerNew(&CustomerAppTask::SensorTimerEventHandler, osTimerPeriodic, nullptr, nullptr);
    if (sSensorTimer == nullptr)
    {
        ChipLogError(AppServer, "sSensorTimer timer create failed");
        return APP_ERROR_CREATE_TIMER_FAILED;
    }

    CHIP_ERROR err = AppInstance().InitSensor();
    VerifyOrReturnError(err == CHIP_NO_ERROR, err,
                        ChipLogError(AppServer, "InitSensor() failed: %" CHIP_ERROR_FORMAT, err.Format()));

    PlatformMgr().LockChipStack();
    AppTask::UpdateThermoStatUI();
    PlatformMgr().UnlockChipStack();

    CustomerAppTask::SensorTimerEventHandler(nullptr); // prime one sensor read so we don't wait 30s
    osTimerStart(sSensorTimer, pdMS_TO_TICKS(kSensorTimerPeriodMs));

    return CHIP_NO_ERROR;
}

CHIP_ERROR AppTask::InitSensor()
{
#if defined(SL_MATTER_USE_SI70XX_SENSOR) && SL_MATTER_USE_SI70XX_SENSOR
    sl_status_t status = Si70xxSensor::Init();
    if (status != SL_STATUS_OK)
    {
        ChipLogError(AppServer, "Failed to Init Sensor with error code: %lx", status);
        return MATTER_PLATFORM_ERROR(status);
    }
#endif // defined(SL_MATTER_USE_SI70XX_SENSOR) && SL_MATTER_USE_SI70XX_SENSOR
    return CHIP_NO_ERROR;
}

CHIP_ERROR AppTask::StartAppTask()
{
    return BaseApplication::StartAppTask(&AppTask::AppTaskMain);
}

void AppTask::AppTaskMain(void * pvParameter)
{
    AppEvent event;
    osMessageQueueId_t sAppEventQueue = *(static_cast<osMessageQueueId_t *>(pvParameter));

    CHIP_ERROR err = GetAppTask().Init();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "AppTask.Init() failed: %" CHIP_ERROR_FORMAT, err.Format());
        appError(err);
    }

#if !(defined(CHIP_CONFIG_ENABLE_ICD_SERVER) && CHIP_CONFIG_ENABLE_ICD_SERVER)
    GetAppTask().StartStatusLEDTimer();
#endif

    ChipLogProgress(AppServer, "App Task started");
    while (true)
    {
        osStatus_t eventReceived = osMessageQueueGet(sAppEventQueue, &event, nullptr, osWaitForever);
        while (eventReceived == osOK)
        {
            GetAppTask().DispatchEvent(&event);
            eventReceived = osMessageQueueGet(sAppEventQueue, &event, nullptr, 0);
        }
    }
}

void AppTask::UpdateThermoStatUI()
{
    DataModel::Nullable<int16_t> currentTempRaw;
    int16_t coolingSetpointRaw            = 0;
    int16_t heatingSetpointRaw            = 0;
    Thermostat::SystemModeEnum systemMode = Thermostat::SystemModeEnum::kOff;

    ThermAttr::LocalTemperature::Get(kThermostatEndpoint, currentTempRaw);
    ThermAttr::OccupiedCoolingSetpoint::Get(kThermostatEndpoint, &coolingSetpointRaw);
    ThermAttr::OccupiedHeatingSetpoint::Get(kThermostatEndpoint, &heatingSetpointRaw);
    ThermAttr::SystemMode::Get(kThermostatEndpoint, &systemMode);

    const int8_t currentTempC = ConvertToPrintableTemp(currentTempRaw.IsNull() ? static_cast<int16_t>(0) : currentTempRaw.Value());
    const int8_t coolingC     = ConvertToPrintableTemp(coolingSetpointRaw);
    const int8_t heatingC     = ConvertToPrintableTemp(heatingSetpointRaw);
    const uint8_t modeForUi   = chip::to_underlying(systemMode);

#if SL_MATTER_DISPLAY_ENABLED
    ThermostatUI::SetMode(modeForUi);
    ThermostatUI::SetHeatingSetPoint(heatingC);
    ThermostatUI::SetCoolingSetPoint(coolingC);
    ThermostatUI::SetCurrentTemp(currentTempC);

    if (BaseApplication::GetProvisionStatus())
    {
        GetLCD().WriteDemoUI(false); // State doesn't matter
    }
#else
    ChipLogProgress(AppServer, "Thermostat Status - M:%d T:%d'C H:%d'C C:%d'C", modeForUi, currentTempC, heatingC, coolingC);
#endif // SL_MATTER_DISPLAY_ENABLED
}

void AppTask::ButtonEventHandler(uint8_t button, uint8_t btnAction)
{
    AppEvent aEvent           = {};
    aEvent.Type               = AppEvent::kEventType_Button;
    aEvent.ButtonEvent.Action = btnAction;

    if (button == APP_FUNCTION_BUTTON)
    {
        aEvent.Handler = BaseApplication::ButtonHandler;
        AppInstance().PostEvent(&aEvent);
    }
}

void AppTask::SensorTimerEventHandler(void * /* arg */)
{
    AppEvent event;
    event.Type    = AppEvent::kEventType_Timer;
    event.Handler = &CustomerAppTask::TemperatureUpdateEventHandler;
    AppInstance().PostEvent(&event);
}

void AppTask::TemperatureUpdateEventHandler(AppEvent * /* aEvent */)
{
    // Remembers the last value pushed to `LocalTemperature` so we can mark the attribute
    // dirty only when the new reading differs by at least `kMinTemperatureDelta`.
    static int16_t sLastTemperature = 0;

    int16_t temperature = 0;
    CHIP_ERROR err      = AppInstance().GetTemperature(temperature);
    VerifyOrReturn(
        err == CHIP_NO_ERROR,
        ChipLogError(AppServer, "GetTemperature() failed: %" CHIP_ERROR_FORMAT ", skipping LocalTemperature::Set", err.Format()));

    ChipLogDetail(AppServer, "Sensor Temp is : %d", temperature);

    MarkAttributeDirty reportState = MarkAttributeDirty::kNo;
    if ((temperature >= (sLastTemperature + kMinTemperatureDelta)) || temperature <= (sLastTemperature - kMinTemperatureDelta))
    {
        reportState = MarkAttributeDirty::kIfChanged;
    }

    sLastTemperature = temperature;
    PlatformMgr().LockChipStack();
    Thermostat::Attributes::LocalTemperature::Set(kThermostatEndpoint, temperature, reportState);
    PlatformMgr().UnlockChipStack();
}

CHIP_ERROR AppTask::GetTemperature(int16_t & temperature)
{
#if defined(SL_MATTER_USE_SI70XX_SENSOR) && SL_MATTER_USE_SI70XX_SENSOR
    int32_t tempSum   = 0;
    uint16_t humidity = 0;
    int16_t sample    = 0;

    for (uint8_t i = 0; i < 100; i++)
    {
        sl_status_t status = Si70xxSensor::GetSensorData(humidity, sample);
        if (status != SL_STATUS_OK)
        {
            ChipLogError(AppServer, "Failed to read Temperature sample: %lx", status);
            return MATTER_PLATFORM_ERROR(status);
        }
        tempSum += sample;
    }
    temperature = static_cast<int16_t>(tempSum / 100);
#else
    // Simulated readings for boards without an Si70xx.
    static constexpr int16_t kSimulatedTemp[]            = { 2300, 2400, 2800, 2550, 2200, 2125, 2100, 2600, 1800, 2700 };
    static constexpr uint16_t kSimulatedReadingFrequency = (60000 / kSensorTimerPeriodMs);
    static uint8_t sSimulatedIndex                       = 0;
    static uint8_t sNbOfRepetition                       = 0;

    if (sSimulatedIndex >= MATTER_ARRAY_SIZE(kSimulatedTemp))
    {
        sSimulatedIndex = 0;
    }
    temperature = kSimulatedTemp[sSimulatedIndex];

    sNbOfRepetition++;
    if (sNbOfRepetition >= kSimulatedReadingFrequency)
    {
        sSimulatedIndex++;
        sNbOfRepetition = 0;
    }
#endif // defined(SL_MATTER_USE_SI70XX_SENSOR) && SL_MATTER_USE_SI70XX_SENSOR

    return CHIP_NO_ERROR;
}

void AppTask::DMPostAttributeChangeCallback(const ConcreteAttributePath & attributePath, uint8_t type, uint16_t size,
                                            uint8_t * value)
{
    ClusterId clusterId     = attributePath.mClusterId;
    AttributeId attributeId = attributePath.mAttributeId;
    ChipLogDetail(Zcl, "Cluster callback: " ChipLogFormatMEI, ChipLogValueMEI(clusterId));

    if (clusterId == Clusters::Identify::Id)
    {
        ChipLogProgress(Zcl, "Identify attribute ID: " ChipLogFormatMEI " Type: %u Value: %u, length %u",
                        ChipLogValueMEI(attributeId), type, *value, size);
        return;
    }

    if (clusterId != Thermostat::Id)
    {
        return;
    }

    switch (attributeId)
    {
    case ThermAttr::LocalTemperature::Id:
        ChipLogDetail(Zcl, "Local temp %d", ConvertToPrintableTemp(*reinterpret_cast<int16_t *>(value)));
        break;

    case ThermAttr::OccupiedCoolingSetpoint::Id:
        ChipLogDetail(Zcl, "CoolingSetpoint %d", ConvertToPrintableTemp(*reinterpret_cast<int16_t *>(value)));
        break;

    case ThermAttr::OccupiedHeatingSetpoint::Id:
        ChipLogDetail(Zcl, "HeatingSetpoint %d", ConvertToPrintableTemp(*reinterpret_cast<int16_t *>(value)));
        break;

    case ThermAttr::SystemMode::Id:
        ChipLogDetail(Zcl, "SystemMode %d", static_cast<uint8_t>(*value));
        break;

    default:
        ChipLogDetail(Zcl, "Unhandled thermostat attribute " ChipLogFormatMEI, ChipLogValueMEI(attributeId));
        return;
    }

    AppTask::UpdateThermoStatUI();

#ifdef SL_MATTER_ENABLE_AWS
    matterAws::control::AttributeHandler(attributePath.mEndpointId, attributeId);
#endif // SL_MATTER_ENABLE_AWS
}

void AppTask::DMThermostatClusterInit(chip::EndpointId endpoint)
{
    using namespace chip::app::Clusters::Thermostat;
    auto & delegate = ThermostatDelegate::GetInstance();
    SetDefaultDelegate(endpoint, &delegate);
}

// emberAfThermostatClusterInitCallback — weak ZAP entry point. CRTP forwarder into AppTask.
void emberAfThermostatClusterInitCallback(chip::EndpointId endpoint)
{
    CustomerAppTask::GetAppTask().DMThermostatClusterInit(endpoint);
}
