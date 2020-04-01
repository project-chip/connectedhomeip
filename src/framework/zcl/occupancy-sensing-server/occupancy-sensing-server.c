/***************************************************************************//**
 * @file
 * @brief
 *******************************************************************************
 * # License
 * <b>Copyright 2018 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 ******************************************************************************/

#include PLATFORM_HEADER
#include CONFIGURATION_HEADER
#include CHIP_AF_API_STACK
#ifdef CHIP_AF_API_DEBUG_PRINT
  #include CHIP_AF_API_DEBUG_PRINT
#endif
#include CHIP_AF_API_ZCL_CORE
#ifdef CHIP_AF_API_COMMAND_INTERPRETER2
  #include CHIP_AF_API_COMMAND_INTERPRETER2
#endif
#include CHIP_AF_API_OCCUPANCY

#include "thread-callbacks.h"

static void writeOccupancyState(HalOccupancyState occupancyState);

// On initialization, set the device type attribute to the type reported by the
// hardware plugin, in case the user forgot to set a default type for their
// project
void chZclOccupancySensingServerInitHandler(void)
{
  ChipZclStatus_t status;
  HalOccupancySensorType deviceType;
  ChipZclEndpointId_t endpoint;
  ChipZclEndpointIndex_t i;

  deviceType = halOccupancyGetSensorType();

  for (i = 0; i < chZclEndpointCount; i++) {
    endpoint = chipZclEndpointIndexToId(i,
                                         &chipZclClusterOccupancySensingServerSpec);
    if (endpoint != CHIP_ZCL_ENDPOINT_NULL) {
      status
        = chipZclWriteAttribute(endpoint,
                                 &chipZclClusterOccupancySensingServerSpec,
                                 CHIP_ZCL_CLUSTER_OCCUPANCY_SENSING_SERVER_ATTRIBUTE_OCCUPANCY_SENSOR_TYPE,
                                 (uint8_t *) &deviceType,
                                 sizeof(uint8_t));
      if (status != CHIP_ZCL_STATUS_SUCCESS) {
        chipAfCorePrintln("Occupancy Sensing Server: failed to write value "
                           "0x%x to sensor type attribute of endpoint %d",
                           deviceType,
                           endpoint);
      }
    }
  }
}

void halOccupancyStateChangedCallback(HalOccupancyState occupancyState)
{
  if (occupancyState == HAL_OCCUPANCY_STATE_OCCUPIED) {
    chipAfCorePrintln("Occupancy detected");
  } else {
    chipAfCorePrintln("Occupancy no longer detected");
  }

  writeOccupancyState(occupancyState);

  // This could also be handled by having the consuming plugin/implementation.c
  // consume the attribute changed callback, but that would not cause a callback
  // in the case that occupancy is detected while the occupancy attribute is
  // already set to occupied, which can be helpful when doing things like
  // implementing a calibration mode
  chipZclOccupancySensingServerOccupancyStateChangedCallback(occupancyState);
}

// Plugin CLI functions
void occupancySetCommand(void)
{
  uint8_t occupancyState;

  occupancyState = chipUnsignedCommandArgument(0);

  chipAfAppPrintln("setting occupied state to: %d", occupancyState);
  halOccupancyStateChangedCallback((HalOccupancyState)occupancyState);
}

// Plugin private functions

static void writeOccupancyState(HalOccupancyState occupancyState)
{
  ChipZclStatus_t status;
  ChipZclEndpointId_t endpoint;
  ChipZclEndpointIndex_t i;

  for (i = 0; i < chZclEndpointCount; i++) {
    endpoint = chipZclEndpointIndexToId(i,
                                         &chipZclClusterOccupancySensingServerSpec);
    if (endpoint != CHIP_ZCL_ENDPOINT_NULL) {
      status
        = chipZclWriteAttribute(endpoint,
                                 &chipZclClusterOccupancySensingServerSpec,
                                 CHIP_ZCL_CLUSTER_OCCUPANCY_SENSING_SERVER_ATTRIBUTE_OCCUPANCY,
                                 (uint8_t *) &occupancyState,
                                 sizeof(uint8_t));
      if (status != CHIP_ZCL_STATUS_SUCCESS) {
        chipAfCorePrintln("Occupancy Sensing Server: failed to write value "
                           "0x%x to occupancy state attribute of endpoint %d",
                           occupancyState,
                           endpoint);
      }
    }
  }
}
