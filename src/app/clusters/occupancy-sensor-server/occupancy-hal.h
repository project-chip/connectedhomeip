/*
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <app/util/basic-types.h>

// enum used to track the type of occupancy sensor being implemented
typedef enum
{
    HAL_OCCUPANCY_SENSOR_TYPE_PIR                = 0x00,
    HAL_OCCUPANCY_SENSOR_TYPE_ULTRASONIC         = 0x01,
    HAL_OCCUPANCY_SENSOR_TYPE_PIR_AND_ULTRASONIC = 0x02,
    HAL_OCCUPANCY_SENSOR_TYPE_PHYSICAL           = 0x03,
} HalOccupancySensorType;

typedef enum
{
    HAL_OCCUPANCY_STATE_UNOCCUPIED = 0x00,
    HAL_OCCUPANCY_STATE_OCCUPIED   = 0x01,
} HalOccupancyState;

/** @brief Initializes the occupancy sensor, along with any hardware
 * peripherals necessary to interface with the hardware.  The application
 * framework will generally initialize this plugin automatically.  Customers who
 * do not use the framework must ensure the plugin is initialized by calling
 * this function.
 */
void halOccupancyInit(chip::EndpointId endpoint);

/** @brief Get the hardware mechanism used to detect occupancy
 *
 * This function should be used to determine what kind of hardware mechanism
 * is driving the occupancy functionality.
 *
 * @return HAL_OCCUPANCY_SENSOR_TYPE_PIR, HAL_OCCUPANCY_SENSOR_TYPE_ULTRASONIC,
 * or HAL_OCCUPANCY_SENSOR_TYPE_PIR_AND_ULTRASONIC, which are defined to match
 * the values used by the ZCL defined SensorType attribute
 */
HalOccupancySensorType halOccupancyGetSensorType(chip::EndpointId endpoint);

/** @brief Notify the system of a change in occupancy state
 *
 * This function will be called whenever the occupancy state of the system
 * changes.
 *
 * @param occupancyState The new occupancy state
 */
void halOccupancyStateChangedCallback(chip::EndpointId endpoint, HalOccupancyState occupancyState);
