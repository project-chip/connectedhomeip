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

#ifndef ZCL_LEVEL_CONTROL_SERVER_H
#define ZCL_LEVEL_CONTROL_SERVER_H

#include EMBER_AF_API_ZCL_CORE

// Define Level Control plugin Scenes sub-table structure.
// NOTE: When modifying this structure take into account NVM token space and
// backward compatibility considerations
typedef struct {
  bool hasCurrentLevelValue;
  uint8_t currentLevelValue;
} EmZclLevelControlSceneSubTableEntry_t;

void emberZclLevelControlServerSetOnOff(EmberZclEndpointId_t endpointId, bool value);

// TODO: This should be generated from types.xml the way it's already done for Zigbee
typedef enum {
  EMBER_ZCL_LEVEL_CONTROL_OPTIONS_EXECUTE_IF_OFF             = 0x01,
  EMBER_ZCL_LEVEL_CONTROL_OPTIONS_COUPLE_COLOR_TEMP_TO_LEVEL = 0x02,
} EmberAfLevelControlOptions;

#endif // ZCL_LEVEL_CONTROL_SERVER_H
