/***************************************************************************//**
 * @file
 * @brief
 ******************************************************************************/

#ifndef ZCL_SCENES_SERVER_TOKENS_H
#define ZCL_SCENES_SERVER_TOKENS_H

/**
 * Custom Application Tokens
 */

#include "scenes-server.h"

// Defines for Scenes tokens (if using Token based scene storage)
// Note: each cluster plugin has its own scenes sub-table structure.
#define CREATOR_ZCL_CORE_SCENES_NUM_ENTRIES   0x7A73
#define CREATOR_ZCL_CORE_SCENE_TABLE          0x7A74

#ifdef DEFINETOKENS
DEFINE_BASIC_TOKEN(ZCL_CORE_SCENES_NUM_ENTRIES,
                   uint8_t,
                   0x00)
DEFINE_INDEXED_TOKEN(ZCL_CORE_SCENE_TABLE,
                     ChipZclSceneEntry_t,
                     CHIP_AF_PLUGIN_SCENES_SERVER_TABLE_SIZE,
                     { CHIP_ZCL_SCENE_NULL, })
#endif // DEFINETOKENS

#endif // ZCL_SCENES_SERVER_TOKENS_H
