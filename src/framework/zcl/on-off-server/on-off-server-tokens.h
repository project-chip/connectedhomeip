/***************************************************************************//**
 * @file
 * @brief
 ******************************************************************************/

#ifndef ZCL_ON_OFF_SERVER_TOKENS_H
#define ZCL_ON_OFF_SERVER_TOKENS_H

#include "on-off-server.h"

/**
 * Custom Application Tokens
 */

// Defines for on-off cluster Scenes tokens (each cluster plugin
// has its own scenes sub-table structure)
#ifdef DEFINETOKENS
  #define CREATOR_ZCL_CORE_ON_OFF_SCENE_SUBTABLE  0x534F  // == Scene/Onoff == "SO"

DEFINE_INDEXED_TOKEN(ZCL_CORE_ON_OFF_SCENE_SUBTABLE,
                     ChZclOnOffSceneSubTableEntry_t,
                     CHIP_AF_PLUGIN_SCENES_SERVER_TABLE_SIZE,
                     { CHIP_ZCL_SCENE_NULL, })
#endif // DEFINETOKENS

#endif // ZCL_ON_OFF_SERVER_TOKENS_H
