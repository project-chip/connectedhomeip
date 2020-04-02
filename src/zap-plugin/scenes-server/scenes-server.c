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

#include PLATFORM_HEADER
#include CONFIGURATION_HEADER
#include EMBER_AF_API_HAL
#ifdef EMBER_AF_API_DEBUG_PRINT
  #include EMBER_AF_API_DEBUG_PRINT
#endif
#include EMBER_AF_API_ZCL_CORE
#include "scenes-server.h"
#include "thread-bookkeeping.h"

#define EMBER_ZCL_SCENE_TABLE_NULL_INDEX          0xFF
#define EMBER_ZCL_SCENE_TABLE_UNUSED_ENDPOINT_ID  EMBER_ZCL_ENDPOINT_NULL

// Globals
uint8_t emberZclPluginScenesServerEntriesInUse = 0;

#ifdef DEFINETOKENS
// Token based storage.
  #define emberZclPluginScenesServerRetrieveSceneEntry(entry, i) \
  halCommonGetIndexedToken(&entry, TOKEN_ZCL_CORE_SCENE_TABLE, i)
  #define emberZclPluginScenesServerSaveSceneEntry(entry, i) \
  halCommonSetIndexedToken(TOKEN_ZCL_CORE_SCENE_TABLE, i, &entry)
  #define emberZclPluginScenesServerGetNumSceneEntriesInUse()                                      \
  (halCommonGetToken(&emberZclClusterScenesServerEntriesInUse, TOKEN_ZCL_CORE_SCENES_NUM_ENTRIES), \
   emberZclClusterScenesServerEntriesInUse)
  #define emberZclPluginScenesServerSetNumSceneEntriesInUse(x) \
  (emberZclClusterScenesServerEntriesInUse = x,                \
   halCommonSetToken(TOKEN_ZCL_CORE_SCENES_NUM_ENTRIES, &emberZclClusterScenesServerEntriesInUse))
  #define emberZclPluginScenesServerIncNumSceneEntriesInUse()                                       \
  ((halCommonGetToken(&emberZclClusterScenesServerEntriesInUse, TOKEN_ZCL_CORE_SCENES_NUM_ENTRIES), \
    ++emberZclPluginScenesServerEntriesInUse),                                                      \
   halCommonSetToken(TOKEN_ZCL_CORE_SCENES_NUM_ENTRIES, &emberZclClusterScenesServerEntriesInUse))
  #define emberZclPluginScenesServerDecNumSceneEntriesInUse()                                       \
  ((halCommonGetToken(&emberZclClusterScenesServerEntriesInUse, TOKEN_ZCL_CORE_SCENES_NUM_ENTRIES), \
    --emberZclClusterScenesServerEntriesInUse),                                                     \
   halCommonSetToken(TOKEN_ZCL_CORE_SCENES_NUM_ENTRIES, &emberZclClusterScenesServerEntriesInUse))
#else
// RAM based storage.
EmberZclSceneEntry_t emberZclPluginScenesServerSceneTable[EMBER_AF_PLUGIN_SCENES_SERVER_TABLE_SIZE] = { { 0 } };   // Ram-based Scenes table.
  #define emberZclPluginScenesServerRetrieveSceneEntry(entry, i) \
  (entry = emberZclPluginScenesServerSceneTable[i])
  #define emberZclPluginScenesServerSaveSceneEntry(entry, i) \
  (emberZclPluginScenesServerSceneTable[i] = entry)
  #define emberZclPluginScenesServerGetNumSceneEntriesInUse() \
  (emberZclPluginScenesServerEntriesInUse)
  #define emberZclPluginScenesServerSetNumSceneEntriesInUse(x) \
  (emberZclPluginScenesServerEntriesInUse = x)
  #define emberZclPluginScenesServerIncNumSceneEntriesInUse() \
  (++emberZclPluginScenesServerEntriesInUse)
  #define emberZclPluginScenesServerDecNumSceneEntriesInUse() \
  (--emberZclPluginScenesServerEntriesInUse)
#endif

static EmberZclEndpointId_t serverEndpointId = 0xFF;

static EmberZclStatus_t addSceneHelper(const EmberZclCommandContext_t *context,
                                       const EmberZclClusterScenesServerCommandAddSceneRequest_t *request);
static void viewSceneHelper(const EmberZclCommandContext_t *context,
                            const EmberZclClusterScenesServerCommandViewSceneRequest_t *request,
                            EmberZclClusterScenesServerCommandViewSceneResponse_t* response);
static EmberZclStatus_t removeSceneHelper(const EmberZclCommandContext_t *context,
                                          const EmberZclClusterScenesServerCommandRemoveSceneRequest_t *request);
static EmberZclStatus_t removeAllScenesHelper(const EmberZclCommandContext_t *context,
                                              const EmberZclClusterScenesServerCommandRemoveAllScenesRequest_t *request);
static EmberZclStatus_t storeCurrentSceneHelper(const EmberZclCommandContext_t *context,
                                                const EmberZclClusterScenesServerCommandStoreSceneRequest_t *request);
static EmberZclStatus_t recallSavedSceneHelper(const EmberZclCommandContext_t *context,
                                               const EmberZclClusterScenesServerCommandRecallSceneRequest_t *request);
static void getSceneMembershipHelper(const EmberZclCommandContext_t *context,
                                     const EmberZclClusterScenesServerCommandGetSceneMembershipRequest_t *request,
                                     EmberZclClusterScenesServerCommandGetSceneMembershipResponse_t* response);
static EmberZclStatus_t copySceneHelper(const EmberZclCommandContext_t *context,
                                        const EmberZclClusterScenesServerCommandCopySceneRequest_t *request);

// Private functions -----------------------------------------------------------

static EmberZclStatus_t addSceneHelper(const EmberZclCommandContext_t *context,
                                       const EmberZclClusterScenesServerCommandAddSceneRequest_t *request)
{
  EmberZclSceneEntry_t entry;
  bool enhanced = (context->commandId == EMBER_ZCL_CLUSTER_SCENES_SERVER_COMMAND_ENHANCED_ADD_SCENE);

  uint16_t groupId = request->groupId;
  uint8_t sceneId = request->sceneId;
  uint16_t transitionTime = request->transitionTime;

  char nameStr[ZCL_SCENES_CLUSTER_MAXIMUM_NAME_LENGTH + 1] = { 0 }; // temp storage for name string.
  MEMCOPY(nameStr, request->sceneName.ptr, request->sceneName.length); // Convert to terminated string.

  uint8_t extFieldSetsLen = request->extensionFieldSets.length;
  uint8_t *pExtFieldSetsData = request->extensionFieldSets.ptr;
  EmberZclEndpointId_t endpointId = context->endpointId;
  uint8_t i;
  uint8_t index = EMBER_ZCL_SCENE_TABLE_NULL_INDEX;

  emberAfCorePrint("%sAddScene 0x%2x, 0x%x, tr=0x%2x",
                   (enhanced ? "Enhanced" : ""),
                   groupId,
                   sceneId,
                   transitionTime);
  emberAfCorePrint(", %s, ", nameStr);
  emberAfCorePrintBuffer(pExtFieldSetsData, extFieldSetsLen, FALSE);
  emberAfCorePrintln("");

  // Add Scene commands can only reference groups to which we belong.
  if (groupId != ZCL_SCENES_GLOBAL_SCENE_GROUP_ID
      && !emberZclIsEndpointInGroup(endpointId, groupId)) {
    return EMBER_ZCL_STATUS_INVALID_FIELD;
  }

  for (i = 0; i < EMBER_AF_PLUGIN_SCENES_SERVER_TABLE_SIZE; i++) {
    emberZclPluginScenesServerRetrieveSceneEntry(entry, i);
    if (entry.endpointId == endpointId
        && entry.groupId == groupId
        && entry.sceneId == sceneId) {
      index = i;
      break;
    } else if (index == EMBER_ZCL_SCENE_TABLE_NULL_INDEX
               && entry.endpointId == EMBER_ZCL_SCENE_TABLE_UNUSED_ENDPOINT_ID) {
      index = i;
    }
  }

  // If the index is still null, the Scene table is full.
  if (index == EMBER_ZCL_SCENE_TABLE_NULL_INDEX) {
    return EMBER_ZCL_STATUS_INSUFFICIENT_SPACE;
  }

  emberZclPluginScenesServerRetrieveSceneEntry(entry, index);

  // The transition time is specified in seconds in the regular version of the
  // command and tenths of a second in the enhanced version.
  if (enhanced) {
    entry.transitionTime = transitionTime / 10;
    entry.transitionTime100ms = (uint8_t)(transitionTime - entry.transitionTime * 10);
  } else {
    entry.transitionTime = transitionTime;
    entry.transitionTime100ms = 0;
  }

#ifdef EMBER_AF_PLUGIN_SCENES_SERVER_NAME_SUPPORT
  strcpy((char*)entry.name, nameStr);
#endif

  bool newScene = (i != index);
  if (newScene) {
    // Cluster specific erase scene functionality is handled here by registered
    // cluster plugin callbacks.
    emZclEraseScene(index);
  }

  // Loop thru extension field data bytes adding cluster data to scene.
  // (Note- spec says extension field data can be omitted)
  uint8_t *pData = pExtFieldSetsData;
  while (pData < pExtFieldSetsData + extFieldSetsLen) {
    // Each extension field data set must contain a two-byte cluster id and a
    // one-byte length followed by the attribute data bytes, otherwise, the data
    // format is malformed.
    if (pData + 3 > pExtFieldSetsData + extFieldSetsLen) {
      return EMBER_ZCL_STATUS_MALFORMED_COMMAND; // data format error.
    }

    EmberZclClusterId_t clusterId = emberZclPluginScenesServerGetUint16FromBuffer(&pData);
    uint8_t length = emberZclPluginScenesServerGetUint8FromBuffer(&pData);
    if (length > 0) {
      if (pData + length > pExtFieldSetsData + extFieldSetsLen) {
        return EMBER_ZCL_STATUS_MALFORMED_COMMAND; // ext field data format error.
      }

      // Cluster specific AddScene extension field functionality is handled
      // here by registered cluster plugin callbacks.
      const uint8_t *sceneData = (const uint8_t *)pData;
      emZclAddScene(clusterId, index, sceneData, length);

      pData += length; // Set pointer to start of data for next cluster.
    }
  } //while

  // If we got this far, we either added a new Scene entry or updated an existing one.
  // In either case, save the entry.
  if (newScene) {
    // Added a new Scene entry, store the basic data and increment the scene count.
    entry.endpointId = endpointId;
    entry.groupId = groupId;
    entry.sceneId = sceneId;

    emberZclPluginScenesServerIncNumSceneEntriesInUse();

    emberZclClusterScenesServerSetSceneCountAttribute(endpointId,
                                                      emberZclPluginScenesServerGetNumSceneEntriesInUse());
  }
  emberZclPluginScenesServerSaveSceneEntry(entry, index);

  return EMBER_ZCL_STATUS_SUCCESS;
}

static void viewSceneHelper(const EmberZclCommandContext_t *context,
                            const EmberZclClusterScenesServerCommandViewSceneRequest_t *request,
                            EmberZclClusterScenesServerCommandViewSceneResponse_t* response)
{
  // Builds and sends the View Scene response.

  EmberZclSceneEntry_t entry;
  EmberZclStatus_t status = EMBER_ZCL_STATUS_NOT_FOUND;
  bool enhanced = (context->commandId == EMBER_ZCL_CLUSTER_SCENES_SERVER_COMMAND_ENHANCED_VIEW_SCENE);
  EmberZclEndpointId_t endpointId = context->endpointId;
  uint16_t groupId = request->groupId;
  uint8_t sceneId = request->sceneId;
  uint8_t index;

  emberAfCorePrintln("%sViewScene 0x%2x, 0x%x",
                     (enhanced ? "Enhanced" : ""),
                     groupId,
                     sceneId);

  // View Scene commands can only be addressed to a single device and only
  // referencing groups to which we belong.
  if (groupId != ZCL_SCENES_GLOBAL_SCENE_GROUP_ID
      && !emberZclIsEndpointInGroup(endpointId, groupId)) {
    status = EMBER_ZCL_STATUS_INVALID_FIELD;
  } else {
    for (index = 0; index < EMBER_AF_PLUGIN_SCENES_SERVER_TABLE_SIZE; index++) {
      emberZclPluginScenesServerRetrieveSceneEntry(entry, index);
      if (entry.endpointId == endpointId
          && entry.groupId == groupId
          && entry.sceneId == sceneId) {
        status = EMBER_ZCL_STATUS_SUCCESS;
        break;
      }
    }
  }

  // The status, group id, and scene id are always included in the response, but
  // the transition time, name, and extension fields are only included if the
  // scene was found.
  response->status = status;
  response->groupId = groupId;
  response->sceneId = sceneId;

  if (status == EMBER_ZCL_STATUS_SUCCESS) {
    // The transition time is returned in seconds in the regular version of the
    // command and tenths of a second in the enhanced version.
    response->transitionTime = (enhanced)
                               ? entry.transitionTime * 10 + entry.transitionTime100ms
                               : entry.transitionTime;

    char nameStr[ZCL_SCENES_CLUSTER_MAXIMUM_NAME_LENGTH + 1] = { 0 }; // temp storage for name string.
    #ifdef EMBER_AF_PLUGIN_SCENES_SERVER_NAME_SUPPORT
    MEMCOPY(nameStr, entry.name, ZCL_SCENES_CLUSTER_MAXIMUM_NAME_LENGTH);
    #endif
    response->sceneName.ptr = (uint8_t *)nameStr;
    response->sceneName.length = strlen(nameStr);

    uint8_t sceneExtFieldSetBuffer[ZCL_SCENES_CLUSTER_MAXIMUM_EXT_FIELD_LENGTH];  // temp storage for extension data bytes.
    uint8_t *pData = sceneExtFieldSetBuffer;
    response->extensionFieldSets.ptr = pData;

    // Cluster specific View scene functionality is handled here by registered plugin callbacks.
    emZclViewScene(index, &pData);

    response->extensionFieldSets.length = pData - response->extensionFieldSets.ptr; // Data has been added to buffer so update response length.
  }

  emberZclSendClusterScenesServerCommandViewSceneResponse(context, response); // Send the response.
}

static EmberZclStatus_t removeSceneHelper(const EmberZclCommandContext_t *context,
                                          const EmberZclClusterScenesServerCommandRemoveSceneRequest_t *request)
{
  EmberZclStatus_t status = EMBER_ZCL_STATUS_NOT_FOUND;

  uint16_t groupId = request->groupId;
  uint8_t sceneId = request->sceneId;
  EmberZclEndpointId_t endpointId = context->endpointId;

  emberAfCorePrintln("RemoveScene 0x%2x, 0x%x", groupId, sceneId);

  // If a group id is specified but this endpointId isn't in it, take no action.
  if (groupId != ZCL_SCENES_GLOBAL_SCENE_GROUP_ID
      && !emberZclIsEndpointInGroup(endpointId, groupId)) {
    status = EMBER_ZCL_STATUS_INVALID_FIELD;
  } else {
    uint8_t i;
    for (i = 0; i < EMBER_AF_PLUGIN_SCENES_SERVER_TABLE_SIZE; i++) {
      EmberZclSceneEntry_t entry;
      emberZclPluginScenesServerRetrieveSceneEntry(entry, i);
      if (entry.endpointId == endpointId
          && entry.groupId == groupId
          && entry.sceneId == sceneId) {
        entry.endpointId = EMBER_ZCL_SCENE_TABLE_UNUSED_ENDPOINT_ID;
        emberZclPluginScenesServerSaveSceneEntry(entry, i);
        emberZclPluginScenesServerDecNumSceneEntriesInUse();
        emberZclClusterScenesServerSetSceneCountAttribute(serverEndpointId,
                                                          emberZclPluginScenesServerGetNumSceneEntriesInUse());
        status = EMBER_ZCL_STATUS_SUCCESS;
        break;
      }
    }
  }

  return status;
}

static EmberZclStatus_t removeAllScenesHelper(const EmberZclCommandContext_t *context,
                                              const EmberZclClusterScenesServerCommandRemoveAllScenesRequest_t *request)
{
  EmberZclStatus_t status = EMBER_ZCL_STATUS_INVALID_FIELD;

  EmberZclEndpointId_t endpointId = context->endpointId;
  uint16_t groupId = request->groupId;

  emberAfCorePrintln("RemoveAllScenes 0x%2x", groupId);

  if (groupId == ZCL_SCENES_GLOBAL_SCENE_GROUP_ID
      || emberZclIsEndpointInGroup(endpointId, groupId)) {
    uint8_t i;
    status = EMBER_ZCL_STATUS_SUCCESS;
    for (i = 0; i < EMBER_AF_PLUGIN_SCENES_SERVER_TABLE_SIZE; i++) {
      EmberZclSceneEntry_t entry;
      emberZclPluginScenesServerRetrieveSceneEntry(entry, i);
      if (entry.endpointId == endpointId
          && entry.groupId == groupId) {
        entry.endpointId = EMBER_ZCL_SCENE_TABLE_UNUSED_ENDPOINT_ID;
        emberZclPluginScenesServerSaveSceneEntry(entry, i);
        emberZclPluginScenesServerDecNumSceneEntriesInUse();
      }
    }
    emberZclClusterScenesServerSetSceneCountAttribute(endpointId,
                                                      emberZclPluginScenesServerGetNumSceneEntriesInUse());
  }

  return status;
}

static EmberZclStatus_t storeCurrentSceneHelper(const EmberZclCommandContext_t *context,
                                                const EmberZclClusterScenesServerCommandStoreSceneRequest_t *request)
{
  EmberZclSceneEntry_t entry;
  uint8_t i;
  uint8_t index = EMBER_ZCL_SCENE_TABLE_NULL_INDEX;

  EmberZclEndpointId_t endpointId = context->endpointId;
  uint16_t groupId = request->groupId;
  uint8_t sceneId = request->sceneId;

  // If a group id is specified but this endpointId isn't in it, take no action.
  if (groupId != ZCL_SCENES_GLOBAL_SCENE_GROUP_ID
      && !emberZclIsEndpointInGroup(endpointId, groupId)) {
    return EMBER_ZCL_STATUS_INVALID_FIELD;
  }

  for (i = 0; i < EMBER_AF_PLUGIN_SCENES_SERVER_TABLE_SIZE; i++) {
    emberZclPluginScenesServerRetrieveSceneEntry(entry, i);
    if (entry.endpointId == endpointId
        && entry.groupId == groupId
        && entry.sceneId == sceneId) {
      index = i;
      break;
    } else if (index == EMBER_ZCL_SCENE_TABLE_NULL_INDEX
               && entry.endpointId == EMBER_ZCL_SCENE_TABLE_UNUSED_ENDPOINT_ID) {
      index = i;
    }
  }

  // If the target index is still zero, the table is full.
  if (index == EMBER_ZCL_SCENE_TABLE_NULL_INDEX) {
    return EMBER_ZCL_STATUS_INSUFFICIENT_SPACE;
  }

  emberZclPluginScenesServerRetrieveSceneEntry(entry, index);

  // Cluster specific Store scene functionality is handled here by registered plugin callbacks.
  emZclStoreScene(endpointId, index);

  // When creating a new entry, the name is set to the null string (i.e., the
  // length is set to zero) and the transition time is set to zero.  The scene
  // count must be increased and written to the attribute table when adding a
  // new scene.  Otherwise, these fields and the count are left alone.
  if (i != index) {
    entry.endpointId = endpointId;
    entry.groupId = groupId;
    entry.sceneId = sceneId;
#ifdef EMBER_AF_PLUGIN_SCENES_SERVER_NAME_SUPPORT
    entry.name[0] = 0;
#endif
    entry.transitionTime = 0;
    entry.transitionTime100ms = 0;

    emberZclPluginScenesServerIncNumSceneEntriesInUse();
    emberZclClusterScenesServerSetSceneCountAttribute(endpointId,
                                                      emberZclPluginScenesServerGetNumSceneEntriesInUse());
  }

  // Save the scene entry and mark is as valid by storing its scene and group
  // ids in the attribute table and setting valid to true.
  emberZclPluginScenesServerSaveSceneEntry(entry, index);
  emberZclClusterScenesServerMakeValid(endpointId, sceneId, groupId);

  return EMBER_ZCL_STATUS_SUCCESS;
}

static EmberZclStatus_t recallSavedSceneHelper(const EmberZclCommandContext_t *context,
                                               const EmberZclClusterScenesServerCommandRecallSceneRequest_t *request)
{
  uint16_t groupId = request->groupId;
  uint8_t sceneId = request->sceneId;
  EmberZclEndpointId_t endpointId = context->endpointId;

  if (groupId != ZCL_SCENES_GLOBAL_SCENE_GROUP_ID
      && !emberZclIsEndpointInGroup(endpointId, groupId)) {
    return EMBER_ZCL_STATUS_INVALID_FIELD;
  } else {
    for (uint8_t tblIdx = 0; tblIdx < EMBER_AF_PLUGIN_SCENES_SERVER_TABLE_SIZE; tblIdx++) {
      EmberZclSceneEntry_t entry;
      emberZclPluginScenesServerRetrieveSceneEntry(entry, tblIdx);
      if (entry.endpointId == endpointId
          && entry.groupId == groupId
          && entry.sceneId == sceneId) {
        // Calculate Recall transition time in 1/10S units.
        uint32_t transitionTime100mS = entry.transitionTime * 10 + entry.transitionTime100ms;

        // Cluster specific Recall scene functionality is handled here by registered plugin callbacks.
        emZclRecallScene(endpointId, tblIdx, transitionTime100mS);

        emberZclClusterScenesServerMakeValid(endpointId, sceneId, groupId);

        return EMBER_ZCL_STATUS_SUCCESS;
      }
    }
  }

  return EMBER_ZCL_STATUS_NOT_FOUND;
}

static void getSceneMembershipHelper(const EmberZclCommandContext_t *context,
                                     const EmberZclClusterScenesServerCommandGetSceneMembershipRequest_t *request,
                                     EmberZclClusterScenesServerCommandGetSceneMembershipResponse_t* response)
{
  // Builds the Scene Membership response.

  EmberZclStatus_t status = EMBER_ZCL_STATUS_SUCCESS;
  EmberZclEndpointId_t endpointId = context->endpointId;
  uint16_t groupId = request->groupId;

  if ((groupId != ZCL_SCENES_GLOBAL_SCENE_GROUP_ID)
      && !emberZclIsEndpointInGroup(endpointId, groupId)) {
    status = EMBER_ZCL_STATUS_INVALID_FIELD;
  }

  // The status, capacity, and group id are always included in the response, but
  // the scene count and scene list are only included if the group id matched.
  response->status = status;
  response->capacity = EMBER_AF_PLUGIN_SCENES_SERVER_TABLE_SIZE
                       - emberZclPluginScenesServerGetNumSceneEntriesInUse();
  response->groupId = groupId;

  if (status == EMBER_ZCL_STATUS_SUCCESS) {
    uint8_t sceneCount = 0;
    uint8_t sceneListBuffer[EMBER_AF_PLUGIN_SCENES_SERVER_TABLE_SIZE] = { 0 }; // local storage

    for (uint8_t i = 0; i < EMBER_AF_PLUGIN_SCENES_SERVER_TABLE_SIZE; i++) {
      EmberZclSceneEntry_t entry;
      emberZclPluginScenesServerRetrieveSceneEntry(entry, i);
      if (entry.endpointId == endpointId
          && entry.groupId == groupId) {
        sceneListBuffer[sceneCount] = entry.sceneId;
        sceneCount++;
      }
    }

    // Set the response.
    response->sceneList.ptr = (uint8_t *)sceneListBuffer;
    response->sceneList.numElementsToEncode = sceneCount;
    response->sceneList.fieldData.valueType = EMBER_ZCLIP_TYPE_UNSIGNED_INTEGER;
    response->sceneList.fieldData.valueSize = sizeof(sceneListBuffer[0]);
  }

  emberZclSendClusterScenesServerCommandGetSceneMembershipResponse(context,
                                                                   response); // Send the response.
}

static EmberZclStatus_t copySceneHelper(const EmberZclCommandContext_t *context,
                                        const EmberZclClusterScenesServerCommandCopySceneRequest_t *request)
{
  EmberZclSceneEntry_t fromEntry;
  EmberZclSceneEntry_t toEntry;
  uint8_t i;

  EmberZclEndpointId_t endpointId = context->endpointId;

  uint8_t mode = request->mode;
  bool copyAllScenes = mode & 0x01;  // If bit 0 is set all scenes in a GroupA will be copied to GroupB.
  copyAllScenes = false;             //TODO- Copy all scenes mode not presently implemented.

  uint16_t groupIdFrom = request->groupIdFrom;
  uint8_t sceneIdFrom = request->sceneIdFrom;
  uint16_t groupIdTo = request->groupIdTo;
  uint8_t sceneIdTo = request->sceneIdTo;

  // Validate from/to group id endpoints.
  if ((groupIdFrom == ZCL_SCENES_GLOBAL_SCENE_GROUP_ID)
      || (!emberZclIsEndpointInGroup(endpointId, groupIdFrom))
      || (groupIdTo == ZCL_SCENES_GLOBAL_SCENE_GROUP_ID)
      || (!emberZclIsEndpointInGroup(endpointId, groupIdTo))) {
    return EMBER_ZCL_STATUS_INVALID_FIELD;
  }

  // Find the 'from' scene.
  uint8_t fromIdx = EMBER_ZCL_SCENE_TABLE_NULL_INDEX;
  for (i = 0; i < EMBER_AF_PLUGIN_SCENES_SERVER_TABLE_SIZE; i++) {
    emberZclPluginScenesServerRetrieveSceneEntry(fromEntry, i);
    if ((fromEntry.endpointId == endpointId)
        && (fromEntry.groupId == groupIdFrom)
        && ((fromEntry.sceneId == sceneIdFrom) || copyAllScenes)) {
      fromIdx = i;
      break;
    }
  }
  if (fromIdx == EMBER_ZCL_SCENE_TABLE_NULL_INDEX) {
    return EMBER_ZCL_STATUS_NOT_FOUND;  // 'from' scene not found
  }

  // Find the 'to' scene.
  uint8_t toIdx = EMBER_ZCL_SCENE_TABLE_NULL_INDEX;
  for (i = 0; i < EMBER_AF_PLUGIN_SCENES_SERVER_TABLE_SIZE; i++) {
    emberZclPluginScenesServerRetrieveSceneEntry(toEntry, i);
    if ((toEntry.endpointId == endpointId)
        && (toEntry.groupId == groupIdTo)
        && ((toEntry.sceneId == sceneIdTo) || copyAllScenes)) {
      toIdx = i;
      break;
    }
  }
  if (toIdx == EMBER_ZCL_SCENE_TABLE_NULL_INDEX) {
    // Couldn't find exact match for destination scene, loop again
    // and allocate to first unused slot in scenes table.
    for (i = 0; i < EMBER_AF_PLUGIN_SCENES_SERVER_TABLE_SIZE; i++) {
      emberZclPluginScenesServerRetrieveSceneEntry(toEntry, i);
      if (toEntry.endpointId == EMBER_ZCL_SCENE_TABLE_UNUSED_ENDPOINT_ID) {
        toIdx = i;
        break;
      }
    }
  }
  if (toIdx == EMBER_ZCL_SCENE_TABLE_NULL_INDEX) {
    return EMBER_ZCL_STATUS_NOT_FOUND;  // 'to' scene not found or table full.
  }

  //emberAfCorePrintln("RX: CopyScene 0x%x 0x%2x 0x%2x 0x%2x 0x%2x", mode, groupIdFrom, sceneIdFrom, groupIdTo, sceneIdTo);

  // Copy the scene table entry.
  fromEntry.groupId = groupIdTo;
  fromEntry.sceneId = sceneIdTo;
  emberZclPluginScenesServerSaveSceneEntry(fromEntry, toIdx);

  // Cluster specific copy scene functionality is handled here by registered plugin callbacks.
  emZclCopyScene(fromIdx, toIdx);

  return EMBER_ZCL_STATUS_SUCCESS;
}

// Public functions ------------------------------------------------------------

void emZclScenesServerInitHandler(void)
{
  serverEndpointId = 0x01; //TODO, get this from app Metadata.

  #ifndef DEFINETOKENS
  for (uint8_t i = 0; i < EMBER_AF_PLUGIN_SCENES_SERVER_TABLE_SIZE; ++i) {
    emberZclPluginScenesServerSceneTable[i].endpointId
      = EMBER_ZCL_SCENE_TABLE_UNUSED_ENDPOINT_ID; // If using ram-based Scenes table, init scene table entry eps = unused.
  }
  #endif

  #ifdef EMBER_AF_PLUGIN_SCENES_SERVER_NAME_SUPPORT
  {
    // The high bit of Name Support indicates whether scene names are supported.
    uint8_t nameSupport = BIT(7); // Set bit.
    emberZclWriteAttribute(serverEndpointId,
                           &emberZclClusterScenesServerSpec,
                           EMBER_ZCL_CLUSTER_SCENES_SERVER_ATTRIBUTE_SCENE_NAME_SUPPORT,
                           (uint8_t *)&nameSupport,
                           sizeof(nameSupport));
  }
  #endif

  emberZclClusterScenesServerSetSceneCountAttribute(serverEndpointId,
                                                    emberZclPluginScenesServerGetNumSceneEntriesInUse());
}

EmberZclStatus_t emberZclClusterScenesServerSetSceneCountAttribute(EmberZclEndpointId_t endpointId,
                                                                   uint8_t newCount)
{
  return emberZclWriteAttribute(endpointId,
                                &emberZclClusterScenesServerSpec,
                                EMBER_ZCL_CLUSTER_SCENES_SERVER_ATTRIBUTE_SCENE_COUNT,
                                (uint8_t *)&newCount,
                                sizeof(newCount));
}

EmberZclStatus_t emberZclClusterScenesServerMakeValid(EmberZclEndpointId_t endpointId,
                                                      uint8_t sceneId,
                                                      uint16_t groupId)
{
  EmberZclStatus_t status;

  // scene ID
  status = emberZclWriteAttribute(endpointId,
                                  &emberZclClusterScenesServerSpec,
                                  EMBER_ZCL_CLUSTER_SCENES_SERVER_ATTRIBUTE_CURRENT_SCENE,
                                  (uint8_t *)&sceneId,
                                  sizeof(sceneId));
  if (status != EMBER_ZCL_STATUS_SUCCESS) {
    return status;
  }

  // group ID
  status = emberZclWriteAttribute(endpointId,
                                  &emberZclClusterScenesServerSpec,
                                  EMBER_ZCL_CLUSTER_SCENES_SERVER_ATTRIBUTE_CURRENT_GROUP,
                                  (uint8_t *)&groupId,
                                  sizeof(groupId));
  if (status != EMBER_ZCL_STATUS_SUCCESS) {
    return status;
  }

  bool valid = true;
  status = emberZclWriteAttribute(endpointId,
                                  &emberZclClusterScenesServerSpec,
                                  EMBER_ZCL_CLUSTER_SCENES_SERVER_ATTRIBUTE_SCENE_VALID,
                                  (uint8_t *)&valid,
                                  sizeof(valid));

  return status;
}

EmberZclStatus_t emberZclClusterScenesServerMakeInvalidCallback(EmberZclEndpointId_t endpointId)
{
  bool valid = FALSE;
  return emberZclWriteAttribute(endpointId,
                                &emberZclClusterScenesServerSpec,
                                EMBER_ZCL_CLUSTER_SCENES_SERVER_ATTRIBUTE_SCENE_VALID,
                                (uint8_t *)&valid,
                                sizeof(valid));
}

void emberZclClusterScenesServerPrintInfo(void)
{
  uint8_t i;
  EmberZclSceneEntry_t entry;
  emberAfCorePrintln("using 0x%x out of 0x%x table slots",
                     emberZclPluginScenesServerGetNumSceneEntriesInUse(),
                     EMBER_AF_PLUGIN_SCENES_SERVER_TABLE_SIZE);
  for (i = 0; i < EMBER_AF_PLUGIN_SCENES_SERVER_TABLE_SIZE; i++) {
    emberZclPluginScenesServerRetrieveSceneEntry(entry, i);

    emberAfCorePrint("%x: ", i);

    if (entry.endpointId != EMBER_ZCL_SCENE_TABLE_UNUSED_ENDPOINT_ID) {
      emberAfCorePrint("ep:%x grp:%2x scene:%x tt:%d",
                       entry.endpointId,
                       entry.groupId,
                       entry.sceneId,
                       entry.transitionTime);

      emberAfCorePrint(".%d", entry.transitionTime100ms);

      #ifdef EMBER_AF_PLUGIN_SCENES_SERVER_NAME_SUPPORT
      emberAfCorePrint(" name(%x)\"", strlen(entry.name));
      emberAfCorePrint("%s", entry.name);
      emberAfCorePrint("\"");
      #endif

      // Cluster specific scene print info functionality is handled here by registered plugin callbacks.
      emZclPrintInfoScene(i);

      emberAfCorePrintln("");
    }
  }
}

void emberZclClusterScenesServerClearSceneTable(EmberZclEndpointId_t endpointId)
{
  for (uint8_t i = 0; i < EMBER_AF_PLUGIN_SCENES_SERVER_TABLE_SIZE; i++) {
    EmberZclSceneEntry_t entry;
    emberZclPluginScenesServerRetrieveSceneEntry(entry, i);
    if ((endpointId == entry.endpointId)
        || (endpointId == EMBER_ZCL_SCENE_TABLE_UNUSED_ENDPOINT_ID)) {
      entry.endpointId = EMBER_ZCL_SCENE_TABLE_UNUSED_ENDPOINT_ID;
      emberZclPluginScenesServerSaveSceneEntry(entry, i);
    }
  }
  emberZclPluginScenesServerSetNumSceneEntriesInUse(0);
  emberZclClusterScenesServerSetSceneCountAttribute(endpointId, 0);
}

void emberZclClusterScenesServerRemoveScenesInGroupCallback(EmberZclEndpointId_t endpointId,
                                                            uint16_t groupId)
{
  for (uint8_t i = 0; i < EMBER_AF_PLUGIN_SCENES_SERVER_TABLE_SIZE; i++) {
    EmberZclSceneEntry_t entry;
    emberZclPluginScenesServerRetrieveSceneEntry(entry, i);
    if (entry.endpointId == endpointId
        && entry.groupId == groupId) {
      entry.groupId = ZCL_SCENES_GLOBAL_SCENE_GROUP_ID;
      entry.endpointId = EMBER_ZCL_SCENE_TABLE_UNUSED_ENDPOINT_ID;
      emberZclPluginScenesServerSaveSceneEntry(entry, i);
      emberZclPluginScenesServerDecNumSceneEntriesInUse();
      emberZclClusterScenesServerSetSceneCountAttribute(serverEndpointId,
                                                        emberZclPluginScenesServerGetNumSceneEntriesInUse());
    }
  }
}

// Zcl command handlers...

void emberZclClusterScenesServerCommandAddSceneRequestHandler(const EmberZclCommandContext_t *context,
                                                              const EmberZclClusterScenesServerCommandAddSceneRequest_t *request)
{
  EmberZclStatus_t status;

  emberAfCorePrintln("RX: AddScene");

  // Add the scene.
  status = addSceneHelper(context, request);

  // Send the response msg.
  EmberZclClusterScenesServerCommandAddSceneResponse_t response = { 0 };
  response.status = status;
  response.groupId = request->groupId;
  response.sceneId = request->sceneId;
  emberZclSendClusterScenesServerCommandAddSceneResponse(context, &response);
}

void emberZclClusterScenesServerCommandViewSceneRequestHandler(const EmberZclCommandContext_t *context,
                                                               const EmberZclClusterScenesServerCommandViewSceneRequest_t *request)
{
  emberAfCorePrintln("RX: ViewScene");

  // Build and send the response message.
  EmberZclClusterScenesServerCommandViewSceneResponse_t response = { 0 };
  viewSceneHelper(context, request, &response);
}

void emberZclClusterScenesServerCommandRemoveSceneRequestHandler(const EmberZclCommandContext_t *context,
                                                                 const EmberZclClusterScenesServerCommandRemoveSceneRequest_t *request)
{
  EmberZclStatus_t status;

  emberAfCorePrintln("RX: RemoveScene");

  // Remove the scene.
  status = removeSceneHelper(context, request);

  // Send the response msg.
  EmberZclClusterScenesServerCommandRemoveSceneResponse_t response = { 0 };
  response.status = status;
  response.groupId = request->groupId;
  response.sceneId = request->sceneId;
  emberZclSendClusterScenesServerCommandRemoveSceneResponse(context, &response);
}

void emberZclClusterScenesServerCommandRemoveAllScenesRequestHandler(const EmberZclCommandContext_t *context,
                                                                     const EmberZclClusterScenesServerCommandRemoveAllScenesRequest_t *request)
{
  EmberZclStatus_t status;

  emberAfCorePrintln("RX: RemoveAllScenes");

  // Remove all scenes in the specified group.
  status = removeAllScenesHelper(context, request);

  // Send the response msg.
  EmberZclClusterScenesServerCommandRemoveAllScenesResponse_t response = { 0 };
  response.status = status;
  response.groupId = request->groupId;
  emberZclSendClusterScenesServerCommandRemoveAllScenesResponse(context, &response);
}

void emberZclClusterScenesServerCommandStoreSceneRequestHandler(const EmberZclCommandContext_t *context,
                                                                const EmberZclClusterScenesServerCommandStoreSceneRequest_t *request)
{
  EmberZclStatus_t status;

  emberAfCorePrintln("RX: StoreScene");

  // Store current scene.
  status = storeCurrentSceneHelper(context, request);

  // Send the response msg.
  EmberZclClusterScenesServerCommandStoreSceneResponse_t response = { 0 };
  response.status = status;
  response.groupId = request->groupId;
  response.sceneId = request->sceneId;
  emberZclSendClusterScenesServerCommandStoreSceneResponse(context, &response);
}

void emberZclClusterScenesServerCommandRecallSceneRequestHandler(const EmberZclCommandContext_t *context,
                                                                 const EmberZclClusterScenesServerCommandRecallSceneRequest_t *request)
{
  EmberZclStatus_t status;

  emberAfCorePrintln("RX: RecallScene");

  // Recall specified saved scene.
  status = recallSavedSceneHelper(context, request);

  // Send default response msg.
  emberZclSendDefaultResponse(context, status);
}

void emberZclClusterScenesServerCommandGetSceneMembershipRequestHandler(const EmberZclCommandContext_t *context,
                                                                        const EmberZclClusterScenesServerCommandGetSceneMembershipRequest_t *request)
{
  emberAfCorePrintln("RX: GetSceneMembership");

  // Build and send the response message.
  EmberZclClusterScenesServerCommandGetSceneMembershipResponse_t response = { 0 };
  getSceneMembershipHelper(context, request, &response);
}

void emberZclClusterScenesServerCommandEnhancedAddSceneRequestHandler(const EmberZclCommandContext_t *context,
                                                                      const EmberZclClusterScenesServerCommandEnhancedAddSceneRequest_t *request)
{
  EmberZclStatus_t status;

  emberAfCorePrintln("RX: EnAddScene");

  // Add the scene.
  status = addSceneHelper(context,
                          (EmberZclClusterScenesServerCommandAddSceneRequest_t *)request);

  // Send the response msg.
  EmberZclClusterScenesServerCommandEnhancedAddSceneResponse_t response = { 0 };
  response.status = status;
  response.groupId = request->groupId;
  response.sceneId = request->sceneId;
  emberZclSendClusterScenesServerCommandEnhancedAddSceneResponse(context, &response);
}

void emberZclClusterScenesServerCommandEnhancedViewSceneRequestHandler(const EmberZclCommandContext_t *context,
                                                                       const EmberZclClusterScenesServerCommandEnhancedViewSceneRequest_t *request)
{
  emberAfCorePrintln("RX: EnViewScene");

  // Build and send the response message.
  EmberZclClusterScenesServerCommandEnhancedViewSceneResponse_t response = { 0 };
  viewSceneHelper(context,
                  (EmberZclClusterScenesServerCommandViewSceneRequest_t *)request,
                  (EmberZclClusterScenesServerCommandViewSceneResponse_t *)&response);
}

void emberZclClusterScenesServerCommandCopySceneRequestHandler(const EmberZclCommandContext_t *context,
                                                               const EmberZclClusterScenesServerCommandCopySceneRequest_t *request)
{
  EmberZclStatus_t status;

  emberAfCorePrintln("RX: CopyScene");

  // Copy the scene(s).
  status = copySceneHelper(context, request);

  // Send the response msg.
  EmberZclClusterScenesServerCommandCopySceneResponse_t response = { 0 };
  response.status = status;
  response.groupIdFrom = request->groupIdFrom;
  response.sceneIdFrom = request->sceneIdFrom;
  emberZclSendClusterScenesServerCommandCopySceneResponse(context, &response);
}

// Scenes utilities

uint8_t emberZclPluginScenesServerGetUint8FromBuffer(uint8_t **ptr)
{
  // This fn modifies source ptr on exit.
  uint8_t value = **ptr;
  *ptr += 1;
  return value;
}

void emberZclPluginScenesServerPutUint8InBuffer(uint8_t **ptr, uint8_t value)
{
  // This fn modifies source ptr on exit.
  **ptr = value;
  *ptr += 1;
}

uint16_t emberZclPluginScenesServerGetUint16FromBuffer(uint8_t **ptr)
{
  // (where Buffer == Big-endian)
  // This fn modifies source ptr on exit.
  uint16_t value = emberFetchHighLowInt16u(*ptr);
  *ptr += 2;
  return value;
}

void emberZclPluginScenesServerPutUint16InBuffer(uint8_t **ptr, uint16_t value)
{
  // (where Buffer == Big-endian)
  // This fn modifies source ptr on exit.
  emberStoreHighLowInt16u(*ptr, value);
  *ptr += 2;
}

// Scenes callback placeholders...

void emberZclEraseSceneCallback(uint8_t tableIdx)
{
  // Cluster specific callbacks for Delete scene function will be called in turn following
  // execution of this fn.
}

bool emberZclAddSceneCallback(EmberZclClusterId_t clusterId,
                              uint8_t tableIdx,
                              const uint8_t *sceneData,
                              uint8_t length)
{
  // Cluster specific callbacks for Add scene function will be called in turn following
  // execution of this fn.
  return false;
}

void emberZclRecallSceneCallback(EmberZclEndpointId_t endpointId,
                                 uint8_t tableIdx,
                                 uint32_t transitionTime100mS)
{
  // Cluster specific callbacks for Recall scene function will be called in turn following
  // execution of this fn. The parameter 'transitionTime100mS' enables the callbacks
  // to apply a recall transtion time (in 1/10S units) if desired.
}

void emberZclStoreSceneCallback(EmberZclEndpointId_t endpointId, uint8_t tableIdx)
{
  // Cluster specific callbacks for Store scene function will be called in turn following
  // execution of this fn.
}

void emberZclCopySceneCallback(uint8_t srcTableIdx, uint8_t dstTableIdx)
{
  // Cluster specific callbacks for Copy scene function will be called in turn following
  // execution of this fn.
}

void emberZclViewSceneCallback(uint8_t tableIdx, uint8_t **pExtFieldData)
{
  // Cluster specific callbacks for View scene function will be called in turn following
  // execution of this fn. (Note: pData references the Scene extension field byte array).
}

void emberZclPrintInfoSceneCallback(uint8_t tableIdx)
{
  // Cluster specific callbacks for PrintInfo scene function will be called in turn following
  // execution of this fn.
}
