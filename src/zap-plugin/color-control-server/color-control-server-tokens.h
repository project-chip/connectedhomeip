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
