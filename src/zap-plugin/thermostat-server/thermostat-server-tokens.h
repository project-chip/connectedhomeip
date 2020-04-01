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

#ifndef ZCL_THERMOSTAT_SERVER_TOKENS_H
#define ZCL_THERMOSTAT_SERVER_TOKENS_H

#include "thermostat-server.h"

/**
 * Custom Application Tokens
 */

// Defines for Thermostat cluster Scenes tokens (each cluster plugin
// has its own scenes sub-table structure)
#ifdef DEFINETOKENS
  #define CREATOR_ZCL_CORE_THERMOSTAT_SCENE_SUBTABLE  0x5354  // == Scene/Thermostat == "ST"

DEFINE_INDEXED_TOKEN(ZCL_CORE_THERMOSTAT_SCENE_SUBTABLE,
                     EmZclThermostatSceneSubTableEntry_t,
                     EMBER_AF_PLUGIN_SCENES_SERVER_TABLE_SIZE,
                     { EMBER_ZCL_SCENE_NULL, })
#endif // DEFINETOKENS

#endif // ZCL_THERMOSTAT_SERVER_TOKENS_H
