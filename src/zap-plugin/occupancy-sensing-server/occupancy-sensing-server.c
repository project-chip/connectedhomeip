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
#include EMBER_AF_API_STACK
#ifdef EMBER_AF_API_DEBUG_PRINT
  #include EMBER_AF_API_DEBUG_PRINT
#endif
#include EMBER_AF_API_ZCL_CORE
#ifdef EMBER_AF_API_COMMAND_INTERPRETER2
  #include EMBER_AF_API_COMMAND_INTERPRETER2
#endif
#include EMBER_AF_API_OCCUPANCY

#include "thread-callbacks.h"

static void writeOccupancyState(HalOccupancyState occupancyState);

// On initialization, set the device type attribute to the type reported by the
// hardware plugin, in case the user forgot to set a default type for their
// project
void emZclOccupancySensingServerInitHandler(void)
{
  EmberZclStatus_t status;
  HalOccupancySensorType deviceType;
  EmberZclEndpointId_t endpoint;
  EmberZclEndpointIndex_t i;

  deviceType = halOccupancyGetSensorType();

  for (i = 0; i < emZclEndpointCount; i++) {
    endpoint = emberZclEndpointIndexToId(i,
                                         &emberZclClusterOccupancySensingServerSpec);
    if (endpoint != EMBER_ZCL_ENDPOINT_NULL) {
      status
        = emberZclWriteAttribute(endpoint,
                                 &emberZclClusterOccupancySensingServerSpec,
                                 EMBER_ZCL_CLUSTER_OCCUPANCY_SENSING_SERVER_ATTRIBUTE_OCCUPANCY_SENSOR_TYPE,
                                 (uint8_t *) &deviceType,
                                 sizeof(uint8_t));
      if (status != EMBER_ZCL_STATUS_SUCCESS) {
        emberAfCorePrintln("Occupancy Sensing Server: failed to write value "
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
    emberAfCorePrintln("Occupancy detected");
  } else {
    emberAfCorePrintln("Occupancy no longer detected");
  }

  writeOccupancyState(occupancyState);

  // This could also be handled by having the consuming plugin/implementation.c
  // consume the attribute changed callback, but that would not cause a callback
  // in the case that occupancy is detected while the occupancy attribute is
  // already set to occupied, which can be helpful when doing things like
  // implementing a calibration mode
  emberZclOccupancySensingServerOccupancyStateChangedCallback(occupancyState);
}

// Plugin CLI functions
void occupancySetCommand(void)
{
  uint8_t occupancyState;

  occupancyState = emberUnsignedCommandArgument(0);

  emberAfAppPrintln("setting occupied state to: %d", occupancyState);
  halOccupancyStateChangedCallback((HalOccupancyState)occupancyState);
}

// Plugin private functions

static void writeOccupancyState(HalOccupancyState occupancyState)
{
  EmberZclStatus_t status;
  EmberZclEndpointId_t endpoint;
  EmberZclEndpointIndex_t i;

  for (i = 0; i < emZclEndpointCount; i++) {
    endpoint = emberZclEndpointIndexToId(i,
                                         &emberZclClusterOccupancySensingServerSpec);
    if (endpoint != EMBER_ZCL_ENDPOINT_NULL) {
      status
        = emberZclWriteAttribute(endpoint,
                                 &emberZclClusterOccupancySensingServerSpec,
                                 EMBER_ZCL_CLUSTER_OCCUPANCY_SENSING_SERVER_ATTRIBUTE_OCCUPANCY,
                                 (uint8_t *) &occupancyState,
                                 sizeof(uint8_t));
      if (status != EMBER_ZCL_STATUS_SUCCESS) {
        emberAfCorePrintln("Occupancy Sensing Server: failed to write value "
                           "0x%x to occupancy state attribute of endpoint %d",
                           occupancyState,
                           endpoint);
      }
    }
  }
}
