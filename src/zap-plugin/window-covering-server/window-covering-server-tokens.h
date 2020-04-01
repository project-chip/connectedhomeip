/***************************************************************************//**
 * @file
 * @brief
 ******************************************************************************/

#ifndef ZCL_WINDOW_COVERING_SERVER_TOKENS_H
#define ZCL_WINDOW_COVERING_SERVER_TOKENS_H

#include "window-covering-server.h"

/**
 * Custom Application Tokens
 */

// Defines for Window Covering cluster Scenes tokens (each cluster plugin
// has its own scenes sub-table structure)
#ifdef DEFINETOKENS
  #define CREATOR_ZCL_CORE_WINDOW_COVERING_SCENE_SUBTABLE  0x7377  // scene/window == "sw"

DEFINE_INDEXED_TOKEN(ZCL_CORE_WINDOW_COVERING_SCENE_SUBTABLE,
                     EmZclWindowCoveringSceneSubTableEntry_t,
                     EMBER_AF_PLUGIN_SCENES_SERVER_TABLE_SIZE,
                     { EMBER_ZCL_SCENE_NULL, })
#endif // DEFINETOKENS

#endif // ZCL_WINDOW_COVERING_SERVER_TOKENS_H
