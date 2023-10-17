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
#include <platform/silabs/platformAbstraction/SilabsPlatform.h>

#if SL_SENSOR_TYPE == 1
#include <app/clusters/occupancy-sensor-server/occupancy-sensor-server.h>
#include <app/clusters/occupancy-sensor-server/occupancy-hal.h>
#elif SL_SENSOR_TYPE == 2
#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#elif SL_SENSOR_TYPE == 3
#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#endif

#if SL_SENSOR_TYPE == 2
#define SIMULATED_TEMP_ARRAY_SIZE 9
static uint8_t mCurrentTempIndex = 0;
static int16_t mSimulatedTempArray[SIMULATED_TEMP_ARRAY_SIZE] = {2152,2435,1966,1533,1111,3333,2921,2566, -4242} ;
static int16_t mMinMeasuredTemp = mSimulatedTempArray[mCurrentTempIndex];
static int16_t mMaxMeasuredTemp = mSimulatedTempArray[mCurrentTempIndex];
#endif

using namespace ::chip::DeviceLayer::Silabs;

void SilabsSensors::InitSensor(void)
{

    // TODO Add Specific Hardware Init here
#if SL_SENSOR_TYPE == 1
#ifdef DISPLAY_ENABLED
    AppTask::GetAppTask().GetLCD().WriteDemoUI(mIsSensorTriggered);
#endif
#elif SL_SENSOR_TYPE == 2
#ifdef DISPLAY_ENABLED
AppTask::GetAppTask().GetLCD().SetCustomUI(SilabsSensors::TemperatureUI);
#endif

#elif SL_SENSOR_TYPE == 3
#ifdef DISPLAY_ENABLED
    AppTask::GetAppTask().GetLCD().WriteDemoUI(mIsSensorTriggered);
#endif
#endif
}

void SilabsSensors::ActionTriggered(AppEvent * aEvent)
{
#if SL_SENSOR_TYPE == 1
    if (aEvent->Type == AppEvent::kEventType_Button)
    {
        if(aEvent->ButtonEvent.Action == static_cast<uint8_t>(SilabsPlatform::ButtonAction::ButtonPressed))
        {
            mIsSensorTriggered = true;
        }
        else if (aEvent->ButtonEvent.Action == static_cast<uint8_t>(SilabsPlatform::ButtonAction::ButtonPressed))
        {
            mIsSensorTriggered = false;
        }
        UpdateBinarySensor(mIsSensorTriggered);
        chip::DeviceLayer::PlatformMgr().LockChipStack();
        halOccupancyStateChangedCallback(1, static_cast<HalOccupancyState>(mIsSensorTriggered));
        chip::DeviceLayer::PlatformMgr().UnlockChipStack();
    }

#elif SL_SENSOR_TYPE == 2

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
            chip::app::Clusters::TemperatureMeasurement::Attributes::MeasuredValue::Set(1, mSimulatedTempArray[mCurrentTempIndex]);
            chip::app::Clusters::TemperatureMeasurement::Attributes::MinMeasuredValue::Set(1, mMinMeasuredTemp);
            chip::app::Clusters::TemperatureMeasurement::Attributes::MaxMeasuredValue::Set(1, mMaxMeasuredTemp);
            chip::DeviceLayer::PlatformMgr().UnlockChipStack();
            #ifdef DISPLAY_ENABLED
            AppTask::GetAppTask().GetLCD().WriteDemoUI(false); // State doesn't Matter
            #endif
        }
    }

#elif SL_SENSOR_TYPE == 3
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
        chip::app::Clusters::BooleanState::Attributes::StateValue::Set(1, mIsSensorTriggered);
        chip::DeviceLayer::PlatformMgr().UnlockChipStack();
    }
#endif
}

#if (SL_SENSOR_TYPE == 1) || (SL_SENSOR_TYPE == 3)
bool SilabsSensors::mIsSensorTriggered;
void SilabsSensors::UpdateBinarySensor(bool state)
{
        AppTask::GetAppTask().SetAppLED(state);

#ifdef DISPLAY_ENABLED
        AppTask::GetAppTask().GetLCD().WriteDemoUI(state);
#endif
}
#endif



// UI For Temperature Sensor
#if SL_SENSOR_TYPE == 2
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
#endif
