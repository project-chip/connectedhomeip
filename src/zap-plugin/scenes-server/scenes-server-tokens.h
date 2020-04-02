/***************************************************************************//**
 *
 *    <COPYRIGHT>
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 *
 ******************************************************************************
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
                     EmberZclSceneEntry_t,
                     EMBER_AF_PLUGIN_SCENES_SERVER_TABLE_SIZE,
                     { EMBER_ZCL_SCENE_NULL, })
#endif // DEFINETOKENS

#endif // ZCL_SCENES_SERVER_TOKENS_H
