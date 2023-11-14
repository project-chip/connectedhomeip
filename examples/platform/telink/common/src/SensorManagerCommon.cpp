/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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

#include "SensorManagerCommon.h"
#ifdef CONFIG_CHIP_USE_MARS_SENSOR
#include <zephyr/drivers/sensor.h>
#endif // CONFIG_CHIP_USE_MARS_SENSOR

LOG_MODULE_DECLARE(app, CONFIG_CHIP_APP_LOG_LEVEL);

using namespace chip;
using namespace ::chip::DeviceLayer;

constexpr float kMinTempDelta                 = 0.5;  // 0.5 degree Celsius

#ifdef CONFIG_CHIP_USE_MARS_SENSOR
k_timer sSensorBanForNextMeasurTimer;
volatile bool mSensorBanForNextMeasurFlag = false;
constexpr uint16_t kSensorBanForNextMeasurTimeout = 1000; // 1s timeout

const struct device *const sht3xd_dev = DEVICE_DT_GET_ONE(sensirion_sht3xd);
#else
constexpr float kSimulatedHum                 = 55.5; // percents
constexpr uint16_t kSimulatedReadingFrequency = 4;    // change simulated number
static float mSimulatedTemp[]                 = { 23.01, 24.02, 28.03, 25.50, 22.05, 21.25, 21.07, 26.08, 18.09, 27.11 };
#endif // CONFIG_CHIP_USE_MARS_SENSOR

SensorManager SensorManager::sSensorManager;

CHIP_ERROR SensorManager::Init()
{
#ifdef CONFIG_CHIP_USE_MARS_SENSOR
    if (!device_is_ready(sht3xd_dev))
    {
        LOG_ERR("Device %s is not ready", sht3xd_dev->name);
        return CHIP_ERROR_INCORRECT_STATE;
	}

    // Initialise the timer to ban sensor measurement
    k_timer_init(&sSensorBanForNextMeasurTimer, &SensorManager::SensorBanForNextMeasurTimerTimeoutCallback, nullptr);
    k_timer_user_data_set(&sSensorBanForNextMeasurTimer, this);
#endif // CONFIG_CHIP_USE_MARS_SENSOR

    return CHIP_NO_ERROR;
}

CHIP_ERROR SensorManager::GetTempAndHumMeasurValue(int16_t *pTempMeasured, uint16_t *pHumMeasured)
{
    static float lastTemp = 0.0;
    float temp            = 0.0;
    float hum             = 0.0;

#ifdef CONFIG_CHIP_USE_MARS_SENSOR
    static struct sensor_value sensorTemp = {0};
    static struct sensor_value sensorHum = {0};

    if (!mSensorBanForNextMeasurFlag)
    {
        int status = sensor_sample_fetch(sht3xd_dev);
        if (status)
        {
            LOG_ERR("Device %s is not ready to fetch the sensor samples (status: %d)", sht3xd_dev->name, status);
            return System::MapErrorZephyr(status);
        }

        status = sensor_channel_get(sht3xd_dev, SENSOR_CHAN_AMBIENT_TEMP, &sensorTemp);
        if (status)
        {
            LOG_ERR("Device %s is not ready to temperature measurement (status: %d)", sht3xd_dev->name, status);
            return System::MapErrorZephyr(status);
        }

        status = sensor_channel_get(sht3xd_dev, SENSOR_CHAN_HUMIDITY, &sensorHum);
        if (status)
        {
            LOG_ERR("Device %s is not ready to humidity measurement (status: %d)", sht3xd_dev->name, status);
            return System::MapErrorZephyr(status);
        }

        mSensorBanForNextMeasurFlag = true;

        // Start next timer to measurement the air quality sensor
        k_timer_start(&sSensorBanForNextMeasurTimer, K_MSEC(kSensorBanForNextMeasurTimeout), K_NO_WAIT);
    }

    temp = (float)sensor_value_to_double(&sensorTemp);
    hum = (float)sensor_value_to_double(&sensorHum);
#else
    /* Temperature simulation is used */
    static uint8_t nbOfRepetition = 0;
    static uint8_t simulatedIndex = 0;
    if (simulatedIndex >= ArraySize(mSimulatedTemp))
    {
        simulatedIndex = 0;
    }
    temp = mSimulatedTemp[simulatedIndex];

    nbOfRepetition++;

    if (nbOfRepetition >= kSimulatedReadingFrequency)
    {
        simulatedIndex++;
        nbOfRepetition = 0;
    }

    /* Humidity simulation is used */
    hum = kSimulatedHum;

#endif // CONFIG_CHIP_USE_MARS_SENSOR

    if ((temp >= (lastTemp + kMinTempDelta)) || temp <= (lastTemp - kMinTempDelta))
    {
        lastTemp = temp;
    }
    else
    {
        temp = lastTemp;
    }

    if (pTempMeasured != NULL)
    {
        // Per spec Application Clusters 2.3.4.1. : MeasuredValue = 100 x temperature [°C]
        *pTempMeasured = (int16_t) 100 * temp;
    }

    if (pHumMeasured != NULL)
    {
        *pHumMeasured =  (uint16_t) hum;
    }

    return CHIP_NO_ERROR;
}

int16_t SensorManager::GetMinMeasuredTempValue()
{
    return mMinMeasuredTempCelsius;
}

int16_t SensorManager::GetMaxMeasuredTempValue()
{
    return mMaxMeasuredTempCelsius;
}

#ifdef CONFIG_CHIP_USE_MARS_SENSOR
void SensorManager::SensorBanForNextMeasurTimerTimeoutCallback(k_timer * timer)
{
    if (!timer)
    {
        return;
    }

    mSensorBanForNextMeasurFlag = false;
}
#endif // CONFIG_CHIP_USE_MARS_SENSOR
