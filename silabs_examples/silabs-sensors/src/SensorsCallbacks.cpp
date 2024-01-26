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

/*
 * Callbacks for All of the Sensor Applications
 */

#include "SensorsCallbacks.h"
#include "AppTask.h"
#include "AppConfig.h"
#include "SensorsUI.h"
#include <platform/silabs/platformAbstraction/SilabsPlatform.h>

SilabsSensors SilabsSensors::sSensorManager;
uint16_t SilabsSensors::currentSensorMode;
bool SilabsSensors::mIsSensorTriggered;

#define stringify( name ) #name

// Sensor mode increments on APP_FUNCTION_BUTTON press
#ifdef QR_CODE_ENABLED
uint16_t sensorMode = CurrentSensorEnum::QrCode;
#else
uint16_t sensorMode = CurrentSensorEnum::OccupancySensor;
#endif

#include <app/clusters/occupancy-sensor-server/occupancy-sensor-server.h>
#include <app/clusters/occupancy-sensor-server/occupancy-hal.h>
#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>

#define SIMULATED_TEMP_ARRAY_SIZE 9
static uint8_t mCurrentTempIndex = 0;
static int16_t mSimulatedTempArray[SIMULATED_TEMP_ARRAY_SIZE] = {2152,2435,1966,1533,1111,3333,2921,2566, -4242} ;
static int16_t mMinMeasuredTemp = mSimulatedTempArray[mCurrentTempIndex];
static int16_t mMaxMeasuredTemp = mSimulatedTempArray[mCurrentTempIndex];

using namespace ::chip::DeviceLayer::Silabs;

CHIP_ERROR SilabsSensors::Init()
{
    CHIP_ERROR err = CHIP_NO_ERROR;

#ifdef QR_CODE_ENABLED
    currentSensorMode = CurrentSensorEnum::QrCode;
#else
    currentSensorMode = CurrentSensorEnum::OccupancySensor;
#endif

    UpdateSensorDisplay();

    return err;

}

void SilabsSensors::UpdateSensorDisplay(void)
{
    if (currentSensorMode == CurrentSensorEnum::OccupancySensor)
    {
#ifdef DISPLAY_ENABLED
        AppTask::GetAppTask().GetLCD().SetCustomUI(SensorsUI::DrawUI);
        AppTask::GetAppTask().GetLCD().WriteDemoUI(mIsSensorTriggered);
#endif
    }
    else if (currentSensorMode == CurrentSensorEnum::TemperatureSensor)
    {
#ifdef DISPLAY_ENABLED
        AppTask::GetAppTask().GetLCD().SetCustomUI(SilabsSensors::TemperatureUI);
        AppTask::GetAppTask().GetLCD().WriteDemoUI(mIsSensorTriggered);

#endif
    }
    else if (currentSensorMode == CurrentSensorEnum::ContactSensor)
    {
#ifdef DISPLAY_ENABLED
        AppTask::GetAppTask().GetLCD().SetCustomUI(SensorsUI::DrawUI);
        AppTask::GetAppTask().GetLCD().WriteDemoUI(mIsSensorTriggered);
#endif
    }
    else if (currentSensorMode == CurrentSensorEnum::StatusScreen)
    {
#ifdef DISPLAY_ENABLED
        BaseApplication::UpdateLCDStatusScreen();
        AppTask::GetAppTask().GetLCD().WriteStatus();
#endif
    }
#ifdef QR_CODE_ENABLED
    else if (currentSensorMode == CurrentSensorEnum::QrCode)
    {
#ifdef DISPLAY_ENABLED
        AppTask::GetAppTask().GetLCD().ShowQRCode(true);
#endif
    }
#endif


}

inline const char* convert_enum_to_string[] = {
    stringify(CurrentSensorEnum::OccupancySensor),
    stringify(CurrentSensorEnum::TemperatureSensor),
    stringify(CurrentSensorEnum::ContactSensor),
    stringify(CurrentSensorEnum::StatusScreen),
#ifdef QR_CODE_ENABLED
    stringify(CurrentSensorEnum::QrCode)
#endif
};

// Swap LCD screens and apps
void SilabsSensors::CycleSensor(AppEvent * aEvent)
{

#ifdef QR_CODE_ENABLED
    if (sensorMode < CurrentSensorEnum::QrCode)
#else
    if (sensorMode < CurrentSensorEnum::StatusScreen)
#endif
    {
        sensorMode++;
    }
    else
    {
        sensorMode = CurrentSensorEnum::OccupancySensor;
    }

    currentSensorMode = sensorMode;

    SILABS_LOG("Current Sensor Mode is %s", convert_enum_to_string[currentSensorMode]);

}



void SilabsSensors::ActionTriggered(AppEvent * aEvent)
{
    if(currentSensorMode == CurrentSensorEnum::OccupancySensor)
    {
        if (aEvent->Type == AppEvent::kEventType_Button)
        {
            if(aEvent->ButtonEvent.Action == static_cast<uint8_t>(SilabsPlatform::ButtonAction::ButtonPressed))
            {
                mIsSensorTriggered = true;
            }
            else if (aEvent->ButtonEvent.Action == static_cast<uint8_t>(SilabsPlatform::ButtonAction::ButtonReleased))
            {
                mIsSensorTriggered = false;
            }
            UpdateBinarySensor(mIsSensorTriggered);
            chip::DeviceLayer::PlatformMgr().LockChipStack();
            halOccupancyStateChangedCallback(1, static_cast<HalOccupancyState>(mIsSensorTriggered));
            chip::DeviceLayer::PlatformMgr().UnlockChipStack();
        }
    }
    
    else if(currentSensorMode == CurrentSensorEnum::TemperatureSensor)
    {
        if (aEvent->Type == AppEvent::kEventType_Button)
        {
            if(aEvent->ButtonEvent.Action == static_cast<uint8_t>(SilabsPlatform::ButtonAction::ButtonPressed))
            {
                if(++mCurrentTempIndex >= SIMULATED_TEMP_ARRAY_SIZE)
                {
                    mCurrentTempIndex = 0;
                }
    
                if (mSimulatedTempArray[mCurrentTempIndex] > mMaxMeasuredTemp)
                {
                    mMaxMeasuredTemp = mSimulatedTempArray[mCurrentTempIndex];
                }
    
                if (mSimulatedTempArray[mCurrentTempIndex] < mMinMeasuredTemp)
                {
                    mMinMeasuredTemp = mSimulatedTempArray[mCurrentTempIndex];
                }
    
                chip::DeviceLayer::PlatformMgr().LockChipStack();
                chip::app::Clusters::TemperatureMeasurement::Attributes::MeasuredValue::Set(2, mSimulatedTempArray    [mCurrentTempIndex]);
                chip::app::Clusters::TemperatureMeasurement::Attributes::MinMeasuredValue::Set(2, mMinMeasuredTemp);
                chip::app::Clusters::TemperatureMeasurement::Attributes::MaxMeasuredValue::Set(2, mMaxMeasuredTemp);
                chip::DeviceLayer::PlatformMgr().UnlockChipStack();
                #ifdef DISPLAY_ENABLED
                AppTask::GetAppTask().GetLCD().WriteDemoUI(false); // State doesn't Matter
                #endif
            }
        }
    }
    
    else if(currentSensorMode == CurrentSensorEnum::ContactSensor)
    {
        if (aEvent->Type == AppEvent::kEventType_Button)
        {
            if(aEvent->ButtonEvent.Action == static_cast<uint8_t>(SilabsPlatform::ButtonAction::ButtonPressed))
            {
                mIsSensorTriggered = true;
            }
            else if (aEvent->ButtonEvent.Action == static_cast<uint8_t>(SilabsPlatform::ButtonAction::ButtonReleased))
            {
                mIsSensorTriggered = false;
            }
            UpdateBinarySensor(mIsSensorTriggered);
            chip::DeviceLayer::PlatformMgr().LockChipStack();
            chip::app::Clusters::BooleanState::Attributes::StateValue::Set(3, mIsSensorTriggered);
            chip::DeviceLayer::PlatformMgr().UnlockChipStack();
        }
    }

}

void SilabsSensors::UpdateBinarySensor(bool state)
{
    if(currentSensorMode == CurrentSensorEnum::ContactSensor || currentSensorMode == CurrentSensorEnum::OccupancySensor)
    {
        AppTask::GetAppTask().SetAppLED(state);

#ifdef DISPLAY_ENABLED
        SensorUIMgr().UpdateDemoState(state);
#endif
    }
}



// UI For Temperature Sensor
#ifdef DISPLAY_ENABLED
#include "dmd.h"
#include "glib.h"
void SilabsSensors::TemperatureUI(GLIB_Context_t * glibContext)
{
    char str[30]={0};
    int16_t temp = mSimulatedTempArray[mCurrentTempIndex];
    char isNegative = ' ';

    if( temp < 0 )
    {
        isNegative = '-';
        temp *= -1;
    }

    GLIB_clear(glibContext);
    GLIB_drawStringOnLine(glibContext, "Temp. Sensor", 1, GLIB_ALIGN_LEFT, 0, 0, true);
    sprintf(str, "Current: %c%d.%d", isNegative, (uint8_t)(temp / 100), (uint8_t)(temp % 100));
    GLIB_drawStringOnLine(glibContext, str, 2, GLIB_ALIGN_LEFT, 0, 0, true);
    isNegative = ' ';
    temp = mMaxMeasuredTemp;
    if( temp < 0 )
    {
        isNegative = '-';
        temp *= -1;
    }
    sprintf(str, "Max: %c%d.%d", isNegative, (uint8_t)(temp / 100), (uint8_t)(temp % 100));
    GLIB_drawStringOnLine(glibContext, str, 3, GLIB_ALIGN_LEFT, 0, 0, true);
    isNegative = ' ';
    temp = mMinMeasuredTemp;
    if( temp < 0 )
    {
        isNegative = '-';
        temp *= -1;
    }
    sprintf(str, "Min: %c%d.%d", isNegative, (uint8_t)(temp / 100), (uint8_t)(temp % 100));
    GLIB_drawStringOnLine(glibContext, str, 4, GLIB_ALIGN_LEFT, 0, 0, true);
    DMD_updateDisplay();
}

#endif
