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

#ifndef ZCL_COLOR_CONTROL_SERVER_TOKENS_H
#define ZCL_COLOR_CONTROL_SERVER_TOKENS_H

#include "color-control-server.h"

/**
 * Custom Application Tokens
 */

// Defines for Color Control cluster Scenes tokens (each cluster plugin
// has its own scenes sub-table structure)
#ifdef DEFINETOKENS
  #define CREATOR_ZCL_CORE_COLOR_CONTROL_SCENE_SUBTABLE  0x5343  // == Scene/Colorcontrol == "SC"

DEFINE_INDEXED_TOKEN(ZCL_CORE_COLOR_CONTROL_SCENE_SUBTABLE,
                     EmZclColorControlSceneSubTableEntry_t,
                     EMBER_AF_PLUGIN_SCENES_SERVER_TABLE_SIZE,
                     { EMBER_ZCL_SCENE_NULL, })
#endif // DEFINETOKENS

#endif // ZCL_COLOR_CONTROL_SERVER_TOKENS_H
