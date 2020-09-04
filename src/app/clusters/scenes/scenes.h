/**
 *
 *    Copyright (c) 2020 Project CHIP Authors
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
 */

/**
 *
 *    Copyright (c) 2020 Silicon Labs
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
 */
/****************************************************************************
 * @file
 * @brief Definitions for the Scenes plugin.
 *******************************************************************************
 ******************************************************************************/

#include <app/util/af-types.h>
#include <stdint.h>

EmberAfStatus emberAfScenesSetSceneCountAttribute(uint8_t endpoint, uint8_t newCount);
EmberAfStatus emberAfScenesMakeValid(uint8_t endpoint, uint8_t sceneId, uint16_t groupId);

// DEPRECATED.
#define emberAfScenesMakeInvalid emberAfScenesClusterMakeInvalidCallback

void emAfPluginScenesServerPrintInfo(void);

extern uint8_t emberAfPluginScenesServerEntriesInUse;
#if defined(EMBER_AF_PLUGIN_SCENES_USE_TOKENS) && !defined(EZSP_HOST)
// In this case, we use token storage
#define emberAfPluginScenesServerRetrieveSceneEntry(entry, i) halCommonGetIndexedToken(&entry, TOKEN_SCENES_TABLE, i)
#define emberAfPluginScenesServerSaveSceneEntry(entry, i) halCommonSetIndexedToken(TOKEN_SCENES_TABLE, i, &entry)
#define emberAfPluginScenesServerNumSceneEntriesInUse()                                                                            \
    (halCommonGetToken(&emberAfPluginScenesServerEntriesInUse, TOKEN_SCENES_NUM_ENTRIES), emberAfPluginScenesServerEntriesInUse)
#define emberAfPluginScenesServerSetNumSceneEntriesInUse(x)                                                                        \
    (emberAfPluginScenesServerEntriesInUse = (x),                                                                                  \
     halCommonSetToken(TOKEN_SCENES_NUM_ENTRIES, &emberAfPluginScenesServerEntriesInUse))
#define emberAfPluginScenesServerIncrNumSceneEntriesInUse()                                                                        \
    ((halCommonGetToken(&emberAfPluginScenesServerEntriesInUse, TOKEN_SCENES_NUM_ENTRIES),                                         \
      ++emberAfPluginScenesServerEntriesInUse),                                                                                    \
     halCommonSetToken(TOKEN_SCENES_NUM_ENTRIES, &emberAfPluginScenesServerEntriesInUse))
#define emberAfPluginScenesServerDecrNumSceneEntriesInUse()                                                                        \
    ((halCommonGetToken(&emberAfPluginScenesServerEntriesInUse, TOKEN_SCENES_NUM_ENTRIES),                                         \
      --emberAfPluginScenesServerEntriesInUse),                                                                                    \
     halCommonSetToken(TOKEN_SCENES_NUM_ENTRIES, &emberAfPluginScenesServerEntriesInUse))
#else
// Use normal RAM storage
extern EmberAfSceneTableEntry emberAfPluginScenesServerSceneTable[];
#define emberAfPluginScenesServerRetrieveSceneEntry(entry, i) (entry = emberAfPluginScenesServerSceneTable[i])
#define emberAfPluginScenesServerSaveSceneEntry(entry, i) (emberAfPluginScenesServerSceneTable[i] = entry)
#define emberAfPluginScenesServerNumSceneEntriesInUse() (emberAfPluginScenesServerEntriesInUse)
#define emberAfPluginScenesServerSetNumSceneEntriesInUse(x) (emberAfPluginScenesServerEntriesInUse = (x))
#define emberAfPluginScenesServerIncrNumSceneEntriesInUse() (++emberAfPluginScenesServerEntriesInUse)
#define emberAfPluginScenesServerDecrNumSceneEntriesInUse() (--emberAfPluginScenesServerEntriesInUse)
#endif // Use tokens

bool emberAfPluginScenesServerParseAddScene(const EmberAfClusterCommand * cmd, uint16_t groupId, uint8_t sceneId,
                                            uint16_t transitionTime, uint8_t * sceneName, uint8_t * extensionFieldSets);
bool emberAfPluginScenesServerParseViewScene(const EmberAfClusterCommand * cmd, uint16_t groupId, uint8_t sceneId);
