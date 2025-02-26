/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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

#include "SensorManager.h"
#include "AppConfig.h"
#include "AppEvent.h"
#include "AppTask.h"
#include <AirQualityConfig.h>
#include <air-quality-sensor-manager.h>

#ifdef USE_AIR_QUALITY_SENSOR
#include "AirQualitySensor.h"
#endif
/**********************************************************
 * Defines and Constants
 *********************************************************/

using namespace chip;
using namespace chip::DeviceLayer;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::AirQuality;

/**********************************************************
 * Variable declarations
 *********************************************************/
SensorManager SensorManager::sSensorManager;
namespace {
constexpr uint16_t kSensorTImerPeriodMs = 30000; // 30s timer period

#ifndef USE_AIR_QUALITY_SENSOR
constexpr uint16_t kSimulatedReadingFrequency =
    (60000 / kSensorTImerPeriodMs); // for every two timer cycles, a simulated sensor update is triggered.
int32_t mSimulatedAirQuality[] = { 5, 55, 105, 155, 205, 255, 305, 355, 400 };
#endif

} // namespace

/**
 * @brief Classifies the air quality based on a given sensor value.
 *
 * This function compares the input value against predefined thresholds
 * defined in the AirQualityConfig.h file. The thresholds are used
 * to classify the air quality into categories defined by the AirQualityEnum.
 * The thresholds are defined in the SensorThresholds enum.
 *
 * @param value The sensor value used to classify air quality.
 * @return AirQualityEnum The classified air quality category.
 */
AirQualityEnum classifyAirQuality(int32_t value)
{
    if (value < MIN_THRESHOLD)
    {
        return AirQualityEnum::kUnknown;
    }
    else if (value < GOOD_THRESHOLD)
    {
        return AirQualityEnum::kGood;
    }
    else if (value < FAIR_THRESHOLD)
    {
        return AirQualityEnum::kFair;
    }
    else if (value < MODERATE_THRESHOLD)
    {
        return AirQualityEnum::kModerate;
    }
    else if (value < POOR_THRESHOLD)
    {
        return AirQualityEnum::kPoor;
    }
    else if (value < VERY_POOR_THRESHOLD)
    {
        return AirQualityEnum::kVeryPoor;
    }
    else
    {
        return AirQualityEnum::kExtremelyPoor;
    }
}

void InitAirQualitySensorManager(intptr_t arg)
{
    AirQualitySensorManager::InitInstance();
}

CHIP_ERROR SensorManager::Init()
{
    DeviceLayer::PlatformMgr().ScheduleWork(InitAirQualitySensorManager);
    // Create cmsisos sw timer for air quality sensor timer.
    mSensorTimer = osTimerNew(SensorTimerEventHandler, osTimerPeriodic, nullptr, nullptr);
    if (mSensorTimer == NULL)
    {
        ChipLogDetail(AppServer, "mSensorTimer timer create failed");
        return APP_ERROR_CREATE_TIMER_FAILED;
    }

#ifdef USE_AIR_QUALITY_SENSOR
    if (SL_STATUS_OK != AirQualitySensor::Init())
    {
        ChipLogDetail(AppServer, "Failed to Init Sensor");
        return CHIP_ERROR_INTERNAL;
    }
#endif
    // Update Air Quality immediatly at bootup
    SensorTimerEventHandler(nullptr);
    // Trigger periodic update
    uint32_t delayTicks = ((uint64_t) osKernelGetTickFreq() * kSensorTImerPeriodMs) / 1000;

    // Starts or restarts the function timer
    if (osTimerStart(mSensorTimer, delayTicks))
    {
        ChipLogDetail(AppServer, "mSensor Timer start() failed");
        appError(APP_ERROR_START_TIMER_FAILED);
    }
    return CHIP_NO_ERROR;
}

void writeAirQualityToAttribute(intptr_t context)
{
    int32_t * air_quality_ptr = reinterpret_cast<int32_t *>(context);
    AirQualitySensorManager::GetInstance()->OnAirQualityChangeHandler(classifyAirQuality(*air_quality_ptr));
    ChipLogDetail(AppServer, "RAW AirQuality value: %ld and corresponding Enum value : %d", *air_quality_ptr,
                  chip::to_underlying(AirQualitySensorManager::GetInstance()->GetAirQuality()));
    AppTask::GetAppTask().UpdateAirQualitySensorUI();
    delete air_quality_ptr;
}

void SensorManager::SensorTimerEventHandler(void * arg)
{
    int32_t air_quality;
#ifdef USE_AIR_QUALITY_SENSOR
    if (SL_STATUS_OK != AirQualitySensor::GetAirQuality(air_quality))
    {
        ChipLogDetail(AppServer, "Failed to read Air Quality !!!");
        return;
    }
#else
    // Initialize static variables to keep track of the current index and repetition count
    static uint8_t nbOfRepetition = 0;
    static uint8_t simulatedIndex = 0;

    // Ensure the simulatedIndex wraps around the array size to avoid out-of-bounds access
    simulatedIndex = simulatedIndex % MATTER_ARRAY_SIZE(mSimulatedAirQuality);
    // Retrieve the current air quality value from the simulated data array using the simulatedIndex
    air_quality = mSimulatedAirQuality[simulatedIndex];

    // Increment the repetition count
    nbOfRepetition++;
    // Check if the number of repetitions has reached the threshold to simulate a new reading
    if (nbOfRepetition >= kSimulatedReadingFrequency)
    {
        // Move to the next index for the next simulated reading
        simulatedIndex++;
        // Reset the repetition count
        nbOfRepetition = 0;
    }
#endif // USE_AIR_QUALITY_SENSOR
    // create pointer for the int32_t air_quality
    int32_t * air_quality_ptr = new int32_t(air_quality);
    DeviceLayer::PlatformMgr().ScheduleWork(writeAirQualityToAttribute, reinterpret_cast<intptr_t>(air_quality_ptr));
}
