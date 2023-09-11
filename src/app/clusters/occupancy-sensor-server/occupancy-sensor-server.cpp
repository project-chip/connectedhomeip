/**
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
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

    chip::BitMask<OccupancySensorTypeBitmap> deviceTypeBitmap = 0;
    switch (deviceType)
    {
    case HAL_OCCUPANCY_SENSOR_TYPE_PIR:
        deviceTypeBitmap.Set(OccupancySensorTypeBitmap::kPir);
        Attributes::OccupancySensorType::Set(endpoint, OccupancySensorTypeEnum::kPir);
        break;

    case HAL_OCCUPANCY_SENSOR_TYPE_ULTRASONIC:
        deviceTypeBitmap.Set(OccupancySensorTypeBitmap::kUltrasonic);
        Attributes::OccupancySensorType::Set(endpoint, OccupancySensorTypeEnum::kUltrasonic);
        break;

    case HAL_OCCUPANCY_SENSOR_TYPE_PIR_AND_ULTRASONIC:
        deviceTypeBitmap.Set(OccupancySensorTypeBitmap::kPir);
        deviceTypeBitmap.Set(OccupancySensorTypeBitmap::kUltrasonic);
        Attributes::OccupancySensorType::Set(endpoint, OccupancySensorTypeEnum::kPIRAndUltrasonic);
        break;

    case HAL_OCCUPANCY_SENSOR_TYPE_PHYSICAL:
        deviceTypeBitmap.Set(OccupancySensorTypeBitmap::kPhysicalContact);
        Attributes::OccupancySensorType::Set(endpoint, OccupancySensorTypeEnum::kPhysicalContact);
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
    chip::BitMask<OccupancyBitmap> mappedOccupancyState;
    if (occupancyState & HAL_OCCUPANCY_STATE_OCCUPIED)
    {
        mappedOccupancyState.Set(OccupancyBitmap::kOccupied);
        ChipLogProgress(Zcl, "Occupancy detected");
    }
    else
    {
        ChipLogProgress(Zcl, "Occupancy no longer detected");
    }

    Attributes::Occupancy::Set(endpoint, occupancyState);
}

void emberAfPluginOccupancyClusterServerPostInitCallback(EndpointId endpoint) {}

HalOccupancySensorType __attribute__((weak)) halOccupancyGetSensorType(EndpointId endpoint)
{
    return HAL_OCCUPANCY_SENSOR_TYPE_PIR;
}

void MatterOccupancySensingPluginServerInitCallback() {}
