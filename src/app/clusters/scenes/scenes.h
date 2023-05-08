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

#pragma once

#include <app-common/zap-generated/cluster-objects.h>
#include <app/CommandHandler.h>
#include <app/data-model/DecodableList.h>
#include <app/util/af-types.h>
#include <app/util/config.h>
#include <lib/support/Span.h>
#include <stdint.h>

/**
 * @brief A structure used to store scene table entries in RAM or in storage,
 * depending on a plugin setting.  If endpoint field is
 * ::EMBER_AF_SCENE_TABLE_UNUSED_ENDPOINT_ID, the entry is unused.
 */
typedef struct
{
    chip::EndpointId endpoint; // 0x00 when this record is not in use
    chip::GroupId groupId;     // 0x0000 if not associated with a group
    uint8_t sceneId;
#if defined(MATTER_CLUSTER_SCENE_NAME_SUPPORT) && MATTER_CLUSTER_SCENE_NAME_SUPPORT
    uint8_t name[ZCL_SCENES_CLUSTER_MAXIMUM_NAME_LENGTH + 1];
#endif
    uint16_t transitionTime;     // in seconds
    uint8_t transitionTime100ms; // in tenths of a seconds
#ifdef ZCL_USING_ON_OFF_CLUSTER_SERVER
    bool hasOnOffValue;
    bool onOffValue;
#endif
#ifdef ZCL_USING_LEVEL_CONTROL_CLUSTER_SERVER
    bool hasCurrentLevelValue;
    chip::app::DataModel::Nullable<uint8_t> currentLevelValue;
#endif
#ifdef ZCL_USING_THERMOSTAT_CLUSTER_SERVER
    bool hasOccupiedCoolingSetpointValue;
    int16_t occupiedCoolingSetpointValue;
    bool hasOccupiedHeatingSetpointValue;
    int16_t occupiedHeatingSetpointValue;
    bool hasSystemModeValue;
    uint8_t systemModeValue;
#endif
#ifdef ZCL_USING_COLOR_CONTROL_CLUSTER_SERVER
    bool hasCurrentXValue;
    uint16_t currentXValue;
    bool hasCurrentYValue;
    uint16_t currentYValue;
    bool hasEnhancedCurrentHueValue;
    uint16_t enhancedCurrentHueValue;
    bool hasCurrentSaturationValue;
    uint8_t currentSaturationValue;
    bool hasColorLoopActiveValue;
    uint8_t colorLoopActiveValue;
    bool hasColorLoopDirectionValue;
    uint8_t colorLoopDirectionValue;
    bool hasColorLoopTimeValue;
    uint16_t colorLoopTimeValue;
    bool hasColorTemperatureMiredsValue;
    uint16_t colorTemperatureMiredsValue;
#endif // ZCL_USING_COLOR_CONTROL_CLUSTER_SERVER
#ifdef ZCL_USING_DOOR_LOCK_CLUSTER_SERVER
    bool hasLockStateValue;
    chip::app::DataModel::Nullable<chip::app::Clusters::DoorLock::DlLockState> lockStateValue;
#endif
#ifdef ZCL_USING_WINDOW_COVERING_CLUSTER_SERVER
    bool hasCurrentPositionLiftPercentageValue;
    chip::app::DataModel::Nullable<chip::Percent> currentPositionLiftPercentageValue;
    bool hasCurrentPositionTiltPercentageValue;
    chip::app::DataModel::Nullable<chip::Percent> currentPositionTiltPercentageValue;
    bool hasTargetPositionLiftPercent100thsValue;
    chip::app::DataModel::Nullable<chip::Percent100ths> targetPositionLiftPercent100thsValue;
    bool hasTargetPositionTiltPercent100thsValue;
    chip::app::DataModel::Nullable<chip::Percent100ths> targetPositionTiltPercent100thsValue;
#endif
} EmberAfSceneTableEntry;

EmberAfStatus emberAfScenesSetSceneCountAttribute(chip::EndpointId endpoint, uint8_t newCount);
EmberAfStatus emberAfScenesMakeValid(chip::EndpointId endpoint, uint8_t sceneId, chip::GroupId groupId);

// DEPRECATED.
#define emberAfScenesMakeInvalid emberAfScenesClusterMakeInvalidCallback

void emAfPluginScenesServerPrintInfo(void);

extern uint8_t emberAfPluginScenesServerEntriesInUse;
#if defined(EMBER_AF_PLUGIN_SCENES_USE_TOKENS) && !defined(EZSP_HOST)
// TODO: Make the storage bits actually work
#define emberAfPluginScenesServerRetrieveSceneEntry(entry, i)
#define emberAfPluginScenesServerSaveSceneEntry(entry, i)
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
    chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath, chip::GroupId groupId,
    uint8_t sceneId, uint16_t transitionTime, const chip::CharSpan & sceneName,
    const chip::app::DataModel::DecodableList<chip::app::Clusters::Scenes::Structs::ExtensionFieldSet::DecodableType> &
        extensionFieldSets);
bool emberAfPluginScenesServerParseViewScene(chip::app::CommandHandler * commandObj,
                                             const chip::app::ConcreteCommandPath & commandPath, chip::GroupId groupId,
                                             uint8_t sceneId);

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
