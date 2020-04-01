/***************************************************************************//**
 * @file
 * @brief
 ******************************************************************************/

#ifndef ZCL_LEVEL_CONTROL_SERVER_TOKENS_H
#define ZCL_LEVEL_CONTROL_SERVER_TOKENS_H

#include "level-control-server.h"

/**
 * Custom Application Tokens
 */

// Defines for Level Control cluster Scenes tokens (each cluster plugin
// has its own scenes sub-table structure)
#ifdef DEFINETOKENS
  #define CREATOR_ZCL_CORE_LEVEL_CONTROL_SCENE_SUBTABLE  0x534C  // == Scene/Levelcontrol == "SL"

DEFINE_INDEXED_TOKEN(ZCL_CORE_LEVEL_CONTROL_SCENE_SUBTABLE,
                     EmZclLevelControlSceneSubTableEntry_t,
                     EMBER_AF_PLUGIN_SCENES_SERVER_TABLE_SIZE,
                     { EMBER_ZCL_SCENE_NULL, })
#endif // DEFINETOKENS

#endif // ZCL_LEVEL_CONTROL_SERVER_TOKENS_H
