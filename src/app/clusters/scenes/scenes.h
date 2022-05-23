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

#pragma once

#include <app-common/zap-generated/cluster-objects.h>
#include <app/CommandHandler.h>
#include <app/data-model/DecodableList.h>
#include <app/util/af-types.h>
#include <lib/support/Span.h>
#include <stdint.h>

EmberAfStatus emberAfScenesSetSceneCountAttribute(chip::EndpointId endpoint, uint8_t newCount);
EmberAfStatus emberAfScenesMakeValid(chip::EndpointId endpoint, uint8_t sceneId, chip::GroupId groupId);

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

bool emberAfPluginScenesServerParseAddScene(
    chip::app::CommandHandler * commandObj, const EmberAfClusterCommand * cmd, chip::GroupId groupId, uint8_t sceneId,
    uint16_t transitionTime, const chip::CharSpan & sceneName,
    const chip::app::DataModel::DecodableList<chip::app::Clusters::Scenes::Structs::ExtensionFieldSet::DecodableType> &
        extensionFieldSets);
bool emberAfPluginScenesServerParseViewScene(chip::app::CommandHandler * commandObj, const EmberAfClusterCommand * cmd,
                                             chip::GroupId groupId, uint8_t sceneId);

/** @brief Scenes Cluster Recall Saved Scene
 *
 * This function is called by the framework when the application should recall a
 * saved scene.
 *
 * @param endpoint The endpoint.  Ver.: always
 * @param groupId The group identifier.  Ver.: always
 * @param sceneId The scene identifier.  Ver.: always
 */
EmberAfStatus emberAfScenesClusterRecallSavedSceneCallback(chip::FabricIndex fabricIndex, chip::EndpointId endpoint,
                                                           chip::GroupId groupId, uint8_t sceneId);

/** @brief Scenes Cluster Store Current Scene
 *
 * This function is called by the framework when the application should store
 * the current scene.  If an entry already exists in the scene table with the
 * same scene and group ids, the application should update the entry with the
 * current scene.  Otherwise, a new entry should be adde to the scene table, if
 * possible.
 *
 * @param endpoint The endpoint.  Ver.: always
 * @param groupId The group identifier.  Ver.: always
 * @param sceneId The scene identifier.  Ver.: always
 */
EmberAfStatus emberAfScenesClusterStoreCurrentSceneCallback(chip::FabricIndex fabricIndex, chip::EndpointId endpoint,
                                                            chip::GroupId groupId, uint8_t sceneId);

/** @brief Scenes Cluster Remove Scenes In Group
 *
 * This function removes the scenes from a specified group.
 *
 * @param endpoint Endpoint  Ver.: always
 * @param groupId Group ID  Ver.: always
 */
void emberAfScenesClusterRemoveScenesInGroupCallback(chip::EndpointId endpoint, chip::GroupId groupId);

/** @brief Scenes Cluster Make Invalid
 *
 * This function is called to invalidate the valid attribute in the Scenes
 * cluster.
 *
 * @param endpoint   Ver.: always
 */
EmberAfStatus emberAfScenesClusterMakeInvalidCallback(chip::EndpointId endpoint);
