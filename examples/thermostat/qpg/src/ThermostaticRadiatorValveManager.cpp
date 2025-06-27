/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include "ThermostaticRadiatorValveManager.h"
#include <FreeRTOS.h>

#include "AppConfig.h"
#include "AppTask.h"

#include "StatusLed.h"
#include "qPinCfg.h"

#include <app-common/zap-generated/attributes/Accessors.h>
#include <lib/support/logging/CHIPLogging.h>

#include "gpSched.h"
#include "qDrvTsens.h"

ThermostaticRadiatorValveManager ThermostaticRadiatorValveManager::sThermostaticRadiatorValve;
using namespace chip;
using namespace chip::app::Clusters;
using namespace chip::DeviceLayer;

TimerHandle_t sThermostaticRadiatorValveTimer;

#if defined(CHIP_CONFIG_FREERTOS_USE_STATIC_TASK) && CHIP_CONFIG_FREERTOS_USE_STATIC_TASK
StaticTimer_t sThermostaticRadiatorValveTimerBuffer;
#endif

static Int16 lastMeasuredTemperature;
static qDrvTsens_Config_t tsensConfig  = Q_DRV_TSENS_CONFIG_DEFAULT(0, 0, qDrvTsens_HanningCycles128);
static const qDrvTsens_Callbacks_t cbs = { .aboveMaximum = NULL, .belowMinimum = NULL, .fifoNotEmpty = NULL };

Int16 (*resultGetFunction)(void) = NULL;

#define DEGREE_FAHRENHEIT_CONVERSION(x) (int) ((float) x * 1.8) + 3200 // in unit of 0.01

#define ONE_SECOND_MS 1000 // 1000 mseconds
#define ONE_MIN_MS 60 * ONE_SECOND_MS
#define TRV_MEASUREMENT_PERIOD ONE_MIN_MS

#define QPG_THERMOSTATIC_ENDPOINT_ID (1)

static void DelayInit(void)
{
    // measure temperature for the first time
    lastMeasuredTemperature = resultGetFunction();

    // start operation
    ThermostaticRadiatorValveMgr().StartNormalOperation();

    ThermostaticRadiatorValveMgr().SetAction(ThermostaticRadiatorValveManager::TRV_OFF_ACTION);
}

CHIP_ERROR ThermostaticRadiatorValveManager::Init()
{
    ChipLogProgress(NotSpecified, "ThermostaticRadiatorValve init");
#if defined(CHIP_CONFIG_FREERTOS_USE_STATIC_TASK) && CHIP_CONFIG_FREERTOS_USE_STATIC_TASK
    sThermostaticRadiatorValveTimer =
        xTimerCreateStatic("ThermostaticRadiatorValveTmr",        // Just a text name, not used by the RTOS kernel
                           TRV_MEASUREMENT_PERIOD,                // == default timer period (mS)
                           true,                                  // with timer reload (==periodic timer)
                           (void *) this,                         // init timer id = ble obj context
                           TimerEventHandler,                     // timer callback handler
                           &sThermostaticRadiatorValveTimerBuffer // static buffer for timer

        );
#else
    // Create FreeRTOS sw timer for ThermostaticRadiatorValve timer.
    sThermostaticRadiatorValveTimer = xTimerCreate("ThermostaticRadiatorValveTmr", // Just a text name, not used by the RTOS kernel
                                                   TRV_MEASUREMENT_PERIOD,         // == default timer period (mS)
                                                   true,                           // with timer reload (==periodic timer)
                                                   (void *) this,                  // init timer id = lock obj context
                                                   TimerEventHandler               // timer callback handler
    );
#endif
    if (sThermostaticRadiatorValveTimer == NULL)
    {
        ChipLogProgress(NotSpecified, "sThermostaticRadiatorValveTimer timer create failed");
        return CHIP_ERROR_UNINITIALIZED;
    }

    if (qDrvTsens_InitCheck())
    {
        // Temperature sensor can be used by the calibration module.
        // qDrvTsens_InitCheck() returns true if the temperature sensor is already initialized.
        // In this case, we need to use the non-blocking function to get the temperature as the calibration module
        // triggers the temperature sensor in the background.
        resultGetFunction = qDrvTsens_ResultGetNonBlocking;
    }
    else
    {
        // Initiate temperature sensor here.
        qResult_t result = qDrvTsens_Init(&tsensConfig, &cbs);
        if (result != Q_OK)
        {
            return CHIP_ERROR_UNINITIALIZED;
        }

        // Set the function pointer to the blocking function as the application is the only one using the temperature sensor.
        resultGetFunction = qDrvTsens_ResultGetBlocking;
    }

    lastMeasuredTemperature = resultGetFunction();

    gpSched_ScheduleEvent(ONE_SECOND_US, DelayInit);

    return CHIP_NO_ERROR;
}

void ThermostaticRadiatorValveManager::SetAction(Action_t action)
{
    mThermostaticRadiatorValve_action = action;
}

void ThermostaticRadiatorValveManager::StartNormalOperation(void)
{
    // start periodic timer
    if (mThermostaticRadiatorValve_action == TRV_OFF_ACTION)
    {
        ChipLogProgress(NotSpecified, "Start Normal Operation");
        // update ThermostaticRadiatorValve status right away and start timer
        UpdateThermostaticRadiatorValveStatus();

        // set timer period
        mPeriodicDuration = TRV_MEASUREMENT_PERIOD;

        // start timer
        StartTimer(mPeriodicDuration);
    }
}

void ThermostaticRadiatorValveManager::StopNormalOperation(void)
{
    if (mThermostaticRadiatorValve_action > TRV_OFF_ACTION)
    {
        ChipLogError(NotSpecified, "Stop Normal Operation");
        CancelTimer();

        // set the action to OFF
        mThermostaticRadiatorValve_action = TRV_OFF_ACTION;
    }
}

void ThermostaticRadiatorValveManager::StartTimer(uint32_t aTimeoutMs)
{
    ChipLogProgress(NotSpecified, "ThermostaticRadiatorValveTimer start timer with %lu", aTimeoutMs);

    if (xTimerIsTimerActive(sThermostaticRadiatorValveTimer))
    {
        ChipLogError(NotSpecified, "app timer already started!");
        CancelTimer();
    }

    // timer is not active, change its period to required value (== restart).
    // FreeRTOS- Block for a maximum of 100 ticks if the change period command
    // cannot immediately be sent to the timer command queue.
    if (xTimerChangePeriod(sThermostaticRadiatorValveTimer, (aTimeoutMs / portTICK_PERIOD_MS), 100) != pdPASS)
    {
        ChipLogError(NotSpecified, "sThermostaticRadiatorValveTimer timer start() failed");
    }
}

void ThermostaticRadiatorValveManager::CancelTimer(void)
{
    if (xTimerStop(sThermostaticRadiatorValveTimer, 0) == pdFAIL)
    {
        ChipLogError(NotSpecified, "Lock timer timer stop() failed");
        // appError(APP_ERROR_STOP_TIMER_FAILED);
    }
}

/* Timer event handler*/
void ThermostaticRadiatorValveManager::TimerEventHandler(TimerHandle_t xTimer)
{
    // Get ThermostaticRadiatorValve obj context from timer id.
    ThermostaticRadiatorValveManager * thermostaticRadiatorValve =
        static_cast<ThermostaticRadiatorValveManager *>(pvTimerGetTimerID(xTimer));

    // The timer event handler will be called in the context of the timer task
    // once sThermostaticRadiatorValveTimer expires. Post an event to apptask queue with the actual handler
    // so that the event can be handled in the context of the apptask.
    AppEvent event;
    event.Type               = AppEvent::kEventType_Timer;
    event.TimerEvent.Context = thermostaticRadiatorValve;
    event.Handler            = PeriodicTimerEventHandler;

    AppTask::GetAppTask().PostEvent(&event);
}

/* periodic event handler */
void ThermostaticRadiatorValveManager::PeriodicTimerEventHandler(AppEvent * aEvent)
{
    ThermostaticRadiatorValveManager * thermostaticRadiatorValve =
        static_cast<ThermostaticRadiatorValveManager *>(aEvent->TimerEvent.Context);
    ChipLogProgress(NotSpecified, "PeriodicTimerEventHandler");

    // periodically check the temperature and update thermostaticRadiatorValve status
    thermostaticRadiatorValve->UpdateThermostaticRadiatorValveStatus();
}

/* Read local temperature from temperature sensor */
int16_t ThermostaticRadiatorValveManager::GetLocalTemperature()
{
    int temp_integerPart;
    int temp_floatingPart;
    int temp = 0;

    // measure temperature through ADC peripheral
    if (resultGetFunction)
    {
        lastMeasuredTemperature = resultGetFunction();
    }

    temp_integerPart  = (int) HAL_ADC_TEMPERATURE_GET_INTEGER_PART(lastMeasuredTemperature);
    temp_floatingPart = (int) HAL_ADC_TEMPERATURE_GET_FLOATING_PART(lastMeasuredTemperature);

    temp = (int) (temp_integerPart * 100 + temp_floatingPart / 10);

    /* ADC module will be resume after SDP012-576*/
    // measure temperature through ADC peripheral
    // ADC_GetTemperatureValue(&temp);

    ChipLogDetail(NotSpecified, "GetLocalTemperature (0.01 degC) - %d", temp);

    return (int16_t) temp;
}

/* Update the status of thermostaticRadiatorValve according to the existing attributes configuration and temperature */
void ThermostaticRadiatorValveManager::UpdateThermostaticRadiatorValveStatus(void)
{

    int16_t localTemperature;
    int16_t heatingSetpoint;
    int16_t coolingSetpoint;
    ThermostaticRadiatorValveManager::SystemMode_t systemMode;

    ChipLogProgress(NotSpecified, "UpdateThermostaticRadiatorValveStatus");
    mThermostaticRadiatorValve_action = TRV_IDLE_ACTION;

    // read temperature value and attributes
    localTemperature = GetLocalTemperature();
    heatingSetpoint  = GetOccupiedHeatingSetpoint();
    coolingSetpoint  = GetOccupiedCoolingSetpoint();
    systemMode       = GetSystemMode();

    // check for the suitable action
    // Heat or Auto mode - start heating if temperature < heating setpoint
    // Cool or Auto mode - start cooling if temperature > cooling setpoint
    if (localTemperature < heatingSetpoint)
    {
        if ((systemMode == SystemMode_t::kAuto) || (systemMode == SystemMode_t::kHeat))
        {
            // Example only - simply set the PI value to the different between local temperature and setpoint
            float delta = (float) (heatingSetpoint - localTemperature);
            float pi;

            pi = delta / heatingSetpoint * 100;
            SetPIHeatingDemand((uint8_t) pi);
            // configure LED
            StatusLed_SetLed(SYSTEM_OPERATING_LED, true);
            // update the state
            mThermostaticRadiatorValve_action = TRV_HEATING_ACTION;
        }
    }
    else if (localTemperature > coolingSetpoint)
    {
        if ((systemMode == SystemMode_t::kAuto) || (systemMode == SystemMode_t::kCool))
        {
            // Example only - simply set the PI value to the different between local temperature and setpoint
            float delta = (float) (localTemperature - coolingSetpoint);
            float pi;

            pi = delta / coolingSetpoint * 100;
            SetPICoolingDemand((uint8_t) pi);

            // configure LED
            StatusLed_SetLed(SYSTEM_OPERATING_LED, true);
            // update the state
            mThermostaticRadiatorValve_action = TRV_COOLING_ACTION;
        }
    }

    if (mThermostaticRadiatorValve_action == TRV_IDLE_ACTION)
    {
        // no heating/cooling
        SetPICoolingDemand((uint8_t) 0);
        SetPIHeatingDemand((uint8_t) 0);

        StatusLed_SetLed(SYSTEM_OPERATING_LED, false);
    }

    UpdateLocalTemperature(localTemperature);
}

void ThermostaticRadiatorValveManager::DisplayTemperature(void)
{
// Display the temperature using logging
#ifdef GP_DIVERSITY_LOG
    int localTemperature;
    ThermostaticRadiatorValveManager::TempDisplayMode_t displaymode;

    ChipLogProgress(NotSpecified, "DisplayTemperature");

    localTemperature = GetLocalTemperature();
    displaymode      = GetTemperatureDisplayMode();

    if (displaymode == ThermostaticRadiatorValveManager::TempDisplayMode_t::kFahrenheit)
    {
        localTemperature = DEGREE_FAHRENHEIT_CONVERSION(localTemperature);
        ChipLogProgress(NotSpecified, "Temperature is %d.%02d degF", localTemperature / 100, localTemperature % 100);
    }
    else
    {
        ChipLogProgress(NotSpecified, "Temperature is %d.%02d degC", localTemperature / 100, localTemperature % 100);
    }

#endif
}

/*==================================================
    Handling ThermostaticRadiatorValve cluster attributes
==================================================*/

void ThermostaticRadiatorValveManager::UpdateLocalTemperature(int16_t aLocalTemperature)
{
    SystemLayer().ScheduleLambda([aLocalTemperature] {
        ChipLogProgress(NotSpecified, "UpdateLocalTemperature with value (0.01 degC) %u", aLocalTemperature);
        if (Protocols::InteractionModel::Status::Success !=
            Thermostat::Attributes::LocalTemperature::Set(QPG_THERMOSTATIC_ENDPOINT_ID, aLocalTemperature))
        {
            ChipLogProgress(NotSpecified, "UpdateLocalTemperature failure");
        }
    });
}

uint8_t ThermostaticRadiatorValveManager::GetPICoolingDemand(void)
{
    uint8_t value;

    Thermostat::Attributes::PICoolingDemand::Get(QPG_THERMOSTATIC_ENDPOINT_ID, &value);

    ChipLogError(NotSpecified, "GetPICoolingDemand - %d", value);
    return value;
}

void ThermostaticRadiatorValveManager::SetPICoolingDemand(uint8_t aPI)
{
    SystemLayer().ScheduleLambda([aPI] {
        Thermostat::Attributes::PICoolingDemand::Set(QPG_THERMOSTATIC_ENDPOINT_ID, aPI);
        ChipLogError(NotSpecified, "SetPICoolingDemand - %d", aPI);
    });
}

uint8_t ThermostaticRadiatorValveManager::GetPIHeatingDemand(void)
{
    uint8_t value;

    Thermostat::Attributes::PIHeatingDemand::Get(QPG_THERMOSTATIC_ENDPOINT_ID, &value);

    ChipLogError(NotSpecified, "GetPIHeatingDemand - %d", value);
    return value;
}

void ThermostaticRadiatorValveManager::SetPIHeatingDemand(uint8_t aPI)
{
    SystemLayer().ScheduleLambda([aPI] {
        Thermostat::Attributes::PIHeatingDemand::Set(QPG_THERMOSTATIC_ENDPOINT_ID, aPI);
        ChipLogError(NotSpecified, "SetPIHeatingDemand - %d", aPI);
    });
}

int16_t ThermostaticRadiatorValveManager::GetOccupiedCoolingSetpoint(void)
{
    int16_t value;

    Thermostat::Attributes::OccupiedCoolingSetpoint::Get(QPG_THERMOSTATIC_ENDPOINT_ID, &value);

    ChipLogError(NotSpecified, "GetOccupiedCoolingSetpoint (0.01 degC) - %d", value);

    return value;
}

void ThermostaticRadiatorValveManager::SetOccupiedCoolingSetpoint(int16_t aSetpoint)
{
    SystemLayer().ScheduleLambda([aSetpoint] {
        Thermostat::Attributes::OccupiedCoolingSetpoint::Set(QPG_THERMOSTATIC_ENDPOINT_ID, aSetpoint);

        ChipLogError(NotSpecified, "SetOccupiedCoolingSetpoint - %d", aSetpoint);
    });
}

int16_t ThermostaticRadiatorValveManager::GetOccupiedHeatingSetpoint(void)
{
    int16_t value;

    Thermostat::Attributes::OccupiedHeatingSetpoint::Get(QPG_THERMOSTATIC_ENDPOINT_ID, &value);

    ChipLogError(NotSpecified, "GetOccupiedHeatingSetpoint (0.01 degC) - %d", value);

    return value;
}

void ThermostaticRadiatorValveManager::SetOccupiedHeatingSetpoint(int16_t aSetpoint)
{
    SystemLayer().ScheduleLambda([aSetpoint] {
        Thermostat::Attributes::OccupiedHeatingSetpoint::Set(QPG_THERMOSTATIC_ENDPOINT_ID, aSetpoint);

        ChipLogError(NotSpecified, "SetOccupiedHeatingSetpoint - status %d", aSetpoint);
    });
}

ThermostaticRadiatorValveManager::Operation_t ThermostaticRadiatorValveManager::GetControlSequenceOfOperation(void)
{
    ThermostaticRadiatorValveManager::Operation_t value;

    Thermostat::Attributes::ControlSequenceOfOperation::Get(QPG_THERMOSTATIC_ENDPOINT_ID, &value);

    ChipLogError(NotSpecified, "GetControlSequenceOfOperation - %d", (uint8_t) value);

    return value;
}

void ThermostaticRadiatorValveManager::SetControlSequenceOfOperation(ThermostaticRadiatorValveManager::Operation_t aOperation)
{
    SystemLayer().ScheduleLambda([aOperation] {
        Thermostat::Attributes::ControlSequenceOfOperation::Set(QPG_THERMOSTATIC_ENDPOINT_ID, aOperation);

        ChipLogError(NotSpecified, "SetControlSequenceOfOperation value %d", (int) aOperation);
    });
}

ThermostaticRadiatorValveManager::SystemMode_t ThermostaticRadiatorValveManager::GetSystemMode(void)
{
    chip::app::Clusters::Thermostat::SystemModeEnum value;

    Thermostat::Attributes::SystemMode::Get(QPG_THERMOSTATIC_ENDPOINT_ID, &value);

    ChipLogError(NotSpecified, "GetSystemMode -  %d", (uint8_t) value);

    return (ThermostaticRadiatorValveManager::SystemMode_t) value;
}

void ThermostaticRadiatorValveManager::SetSystemMode(ThermostaticRadiatorValveManager::SystemMode_t aSystemMode)
{
    SystemLayer().ScheduleLambda([aSystemMode] {
        Thermostat::Attributes::SystemMode::Set(QPG_THERMOSTATIC_ENDPOINT_ID, aSystemMode);

        ChipLogError(NotSpecified, "SetSystemMode value %d", (uint8_t) aSystemMode);
    });
}

ThermostaticRadiatorValveManager::TempDisplayMode_t ThermostaticRadiatorValveManager::GetTemperatureDisplayMode(void)
{
    ThermostaticRadiatorValveManager::TempDisplayMode_t value;

    ThermostatUserInterfaceConfiguration::Attributes::TemperatureDisplayMode::Get(QPG_THERMOSTATIC_ENDPOINT_ID, &value);

    ChipLogError(NotSpecified, "GetTemperatureDisplayMode -  %d", (uint8_t) value);

    return value;
}

void ThermostaticRadiatorValveManager::SetTemperatureDisplayMode(ThermostaticRadiatorValveManager::TempDisplayMode_t aMode)
{
    SystemLayer().ScheduleLambda([aMode] {
        ThermostatUserInterfaceConfiguration::Attributes::TemperatureDisplayMode::Set(QPG_THERMOSTATIC_ENDPOINT_ID, aMode);

        ChipLogError(NotSpecified, "SetTemperatureDisplayMode value %d", (uint8_t) aMode);
    });
}

// Reserved for future use
void ThermostaticRadiatorValveManager::SetCallbacks(ThermostaticRadiatorValveCallback_fn aActionInitiated_CB,
                                                    ThermostaticRadiatorValveCallback_fn aActionCompleted_CB)
{
    mActionInitiated_CB = aActionInitiated_CB;
    mActionCompleted_CB = aActionCompleted_CB;
}
