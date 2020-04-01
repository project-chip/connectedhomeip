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
