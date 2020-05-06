/***************************************************************************/ /**
                                                                               *
                                                                               *    <COPYRIGHT>
                                                                               *
                                                                               *    Licensed under the Apache License, Version 2.0
                                                                               *(the "License"); you may not use this file except in
                                                                               *compliance with the License. You may obtain a copy
                                                                               *of the License at
                                                                               *
                                                                               *        http://www.apache.org/licenses/LICENSE-2.0
                                                                               *
                                                                               *    Unless required by applicable law or agreed to
                                                                               *in writing, software distributed under the License
                                                                               *is distributed on an "AS IS" BASIS, WITHOUT
                                                                               *WARRANTIES OR CONDITIONS OF ANY KIND, either express
                                                                               *or implied. See the License for the specific
                                                                               *language governing permissions and limitations under
                                                                               *the License.
                                                                               *
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
#define CREATOR_ZCL_CORE_LEVEL_CONTROL_SCENE_SUBTABLE 0x534C // == Scene/Levelcontrol == "SL"

DEFINE_INDEXED_TOKEN(ZCL_CORE_LEVEL_CONTROL_SCENE_SUBTABLE, ZapLevelControlSceneSubTableEntry_t,
                     ZAP_AF_PLUGIN_SCENES_SERVER_TABLE_SIZE,
                     {
                         ZAP_SCENE_NULL,
                     })
#endif // DEFINETOKENS

#endif // ZCL_LEVEL_CONTROL_SERVER_TOKENS_H
