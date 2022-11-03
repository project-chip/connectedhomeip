/*
 * SPDX-FileCopyrightText: (c) 2020 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

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
