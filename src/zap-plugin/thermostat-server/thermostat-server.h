/***************************************************************************//**
 * @file
 * @brief
 ******************************************************************************/

#ifndef ZCL_THERMOSTAT_SERVER_H
#define ZCL_THERMOSTAT_SERVER_H

#include EMBER_AF_API_ZCL_CORE

// Define Thermostat plugin Scenes sub-table structure.
// NOTE: When modifying this structure take into account NVM token space and
// backward compatibility considerations
typedef struct {
  bool hasOccupiedCoolingSetpointValue;
  int16_t occupiedCoolingSetpointValue;
  bool hasOccupiedHeatingSetpointValue;
  int16_t occupiedHeatingSetpointValue;
  bool hasSystemModeValue;
  uint8_t systemModeValue;
} EmZclThermostatSceneSubTableEntry_t;

#endif // ZCL_THERMOSTAT_SERVER_H
