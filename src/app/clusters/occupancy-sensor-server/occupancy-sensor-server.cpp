/**
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

/**
 *
 *    Copyright (c) 2020 Silicon Labs
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
/***************************************************************************/
/**
 * @file
 * @brief Routines for the Occupancy plugin, which
 *implements the Occupancy server cluster.
 *******************************************************************************
 ******************************************************************************/
#include "occupancy-sensor-server.h"

#include <app/util/af.h>

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/enums.h>

#include "occupancy-hal.h"

using namespace chip;
using namespace chip::app::Clusters::OccupancySensing;

//******************************************************************************
// Plugin init function
//******************************************************************************
void emberAfOccupancySensingClusterServerInitCallback(EndpointId endpoint)
{
    auto deviceType = halOccupancyGetSensorType(endpoint);
    Attributes::OccupancySensorType::Set(endpoint, deviceType);

    uint8_t deviceTypeBitmap = 0;
    switch (deviceType)
    {
    case HAL_OCCUPANCY_SENSOR_TYPE_PIR:
        deviceTypeBitmap = EMBER_AF_OCCUPANCY_SENSOR_TYPE_BITMAP_PIR;
        break;

    case HAL_OCCUPANCY_SENSOR_TYPE_ULTRASONIC:
        deviceTypeBitmap = EMBER_AF_OCCUPANCY_SENSOR_TYPE_BITMAP_ULTRASONIC;
        break;

    case HAL_OCCUPANCY_SENSOR_TYPE_PIR_AND_ULTRASONIC:
        deviceTypeBitmap = (EMBER_AF_OCCUPANCY_SENSOR_TYPE_BITMAP_PIR | EMBER_AF_OCCUPANCY_SENSOR_TYPE_BITMAP_ULTRASONIC);
        break;

    case HAL_OCCUPANCY_SENSOR_TYPE_PHYSICAL:
        deviceTypeBitmap = EMBER_AF_OCCUPANCY_SENSOR_TYPE_BITMAP_PHYSICAL_CONTACT;
        break;

    default:
        break;
    }
    Attributes::OccupancySensorTypeBitmap::Set(endpoint, deviceTypeBitmap);

    emberAfPluginOccupancyClusterServerPostInitCallback(endpoint);
}

//******************************************************************************
// Notification callback from the HAL plugin
//******************************************************************************
void halOccupancyStateChangedCallback(EndpointId endpoint, HalOccupancyState occupancyState)
{
    if (occupancyState == HAL_OCCUPANCY_STATE_OCCUPIED)
    {
        emberAfOccupancySensingClusterPrintln("Occupancy detected");
    }
    else
    {
        emberAfOccupancySensingClusterPrintln("Occupancy no longer detected");
    }

    Attributes::Occupancy::Set(endpoint, occupancyState);
}

void emberAfPluginOccupancyClusterServerPostInitCallback(EndpointId endpoint) {}

HalOccupancySensorType __attribute__((weak)) halOccupancyGetSensorType(EndpointId endpoint)
{
    return HAL_OCCUPANCY_SENSOR_TYPE_PIR;
}

void MatterOccupancySensingPluginServerInitCallback() {}
