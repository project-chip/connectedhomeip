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

#ifndef ZCL_DOOR_LOCK_SERVER_TOKENS_H
#define ZCL_DOOR_LOCK_SERVER_TOKENS_H

#include "door-lock-server.h"

/**
 * Custom Application Tokens
 */

// Defines for Door Lock cluster Scenes tokens (each cluster plugin
// has its own scenes sub-table structure)
#ifdef DEFINETOKENS
  #define CREATOR_ZCL_CORE_DOOR_LOCK_SCENE_SUBTABLE  0x5344  // == Scene/Doorlock == "SD"

DEFINE_INDEXED_TOKEN(ZCL_CORE_DOOR_LOCK_SCENE_SUBTABLE,
                     EmZclDoorLockSceneSubTableEntry_t,
                     EMBER_AF_PLUGIN_SCENES_SERVER_TABLE_SIZE,
                     { EMBER_ZCL_SCENE_NULL, })
#endif // DEFINETOKENS

// Defines for Door Lock server cluster tokens.
#ifdef DEFINETOKENS
  #define CREATOR_ZCL_CORE_DOOR_LOCK_USER_TABLE  0x4455  // == Doorlock/User == "DU"

DEFINE_INDEXED_TOKEN(ZCL_CORE_DOOR_LOCK_USER_TABLE,
                     EmberAfDoorLockUser_t,
                     EMBER_AF_PLUGIN_DOOR_LOCK_SERVER_DOOR_LOCK_USER_TABLE_SIZE,
                     { 0, })

#endif // DEFINETOKENS

#endif // ZCL_DOOR_LOCK_SERVER_TOKENS_H
