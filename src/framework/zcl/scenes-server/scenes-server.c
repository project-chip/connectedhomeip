/***************************************************************************//**
 * @file
 * @brief
 ******************************************************************************/

#include PLATFORM_HEADER
#include CONFIGURATION_HEADER
#include CHIP_AF_API_HAL
#ifdef CHIP_AF_API_DEBUG_PRINT
  #include CHIP_AF_API_DEBUG_PRINT
#endif
#include CHIP_AF_API_ZCL_CORE
#include "scenes-server.h"
#include "thread-bookkeeping.h"

#define CHIP_ZCL_SCENE_TABLE_NULL_INDEX          0xFF
#define CHIP_ZCL_SCENE_TABLE_UNUSED_ENDPOINT_ID  CHIP_ZCL_ENDPOINT_NULL

// Globals
uint8_t chipZclPluginScenesServerEntriesInUse = 0;

#ifdef DEFINETOKENS
// Token based storage.
  #define chipZclPluginScenesServerRetrieveSceneEntry(entry, i) \
  halCommonGetIndexedToken(&entry, TOKEN_ZCL_CORE_SCENE_TABLE, i)
  #define chipZclPluginScenesServerSaveSceneEntry(entry, i) \
  halCommonSetIndexedToken(TOKEN_ZCL_CORE_SCENE_TABLE, i, &entry)
  #define chipZclPluginScenesServerGetNumSceneEntriesInUse()                                      \
  (halCommonGetToken(&chipZclClusterScenesServerEntriesInUse, TOKEN_ZCL_CORE_SCENES_NUM_ENTRIES), \
   chipZclClusterScenesServerEntriesInUse)
  #define chipZclPluginScenesServerSetNumSceneEntriesInUse(x) \
  (chipZclClusterScenesServerEntriesInUse = x,                \
   halCommonSetToken(TOKEN_ZCL_CORE_SCENES_NUM_ENTRIES, &chipZclClusterScenesServerEntriesInUse))
  #define chipZclPluginScenesServerIncNumSceneEntriesInUse()                                       \
  ((halCommonGetToken(&chipZclClusterScenesServerEntriesInUse, TOKEN_ZCL_CORE_SCENES_NUM_ENTRIES), \
    ++chipZclPluginScenesServerEntriesInUse),                                                      \
   halCommonSetToken(TOKEN_ZCL_CORE_SCENES_NUM_ENTRIES, &chipZclClusterScenesServerEntriesInUse))
  #define chipZclPluginScenesServerDecNumSceneEntriesInUse()                                       \
  ((halCommonGetToken(&chipZclClusterScenesServerEntriesInUse, TOKEN_ZCL_CORE_SCENES_NUM_ENTRIES), \
    --chipZclClusterScenesServerEntriesInUse),                                                     \
   halCommonSetToken(TOKEN_ZCL_CORE_SCENES_NUM_ENTRIES, &chipZclClusterScenesServerEntriesInUse))
#else
// RAM based storage.
ChipZclSceneEntry_t chipZclPluginScenesServerSceneTable[CHIP_AF_PLUGIN_SCENES_SERVER_TABLE_SIZE] = { { 0 } };   // Ram-based Scenes table.
  #define chipZclPluginScenesServerRetrieveSceneEntry(entry, i) \
  (entry = chipZclPluginScenesServerSceneTable[i])
  #define chipZclPluginScenesServerSaveSceneEntry(entry, i) \
  (chipZclPluginScenesServerSceneTable[i] = entry)
  #define chipZclPluginScenesServerGetNumSceneEntriesInUse() \
  (chipZclPluginScenesServerEntriesInUse)
  #define chipZclPluginScenesServerSetNumSceneEntriesInUse(x) \
  (chipZclPluginScenesServerEntriesInUse = x)
  #define chipZclPluginScenesServerIncNumSceneEntriesInUse() \
  (++chipZclPluginScenesServerEntriesInUse)
  #define chipZclPluginScenesServerDecNumSceneEntriesInUse() \
  (--chipZclPluginScenesServerEntriesInUse)
#endif

static ChipZclEndpointId_t serverEndpointId = 0xFF;

static ChipZclStatus_t addSceneHelper(const ChipZclCommandContext_t *context,
                                       const ChipZclClusterScenesServerCommandAddSceneRequest_t *request);
static void viewSceneHelper(const ChipZclCommandContext_t *context,
                            const ChipZclClusterScenesServerCommandViewSceneRequest_t *request,
                            ChipZclClusterScenesServerCommandViewSceneResponse_t* response);
static ChipZclStatus_t removeSceneHelper(const ChipZclCommandContext_t *context,
                                          const ChipZclClusterScenesServerCommandRemoveSceneRequest_t *request);
static ChipZclStatus_t removeAllScenesHelper(const ChipZclCommandContext_t *context,
                                              const ChipZclClusterScenesServerCommandRemoveAllScenesRequest_t *request);
static ChipZclStatus_t storeCurrentSceneHelper(const ChipZclCommandContext_t *context,
                                                const ChipZclClusterScenesServerCommandStoreSceneRequest_t *request);
static ChipZclStatus_t recallSavedSceneHelper(const ChipZclCommandContext_t *context,
                                               const ChipZclClusterScenesServerCommandRecallSceneRequest_t *request);
static void getSceneMchipshipHelper(const ChipZclCommandContext_t *context,
                                     const ChipZclClusterScenesServerCommandGetSceneMchipshipRequest_t *request,
                                     ChipZclClusterScenesServerCommandGetSceneMchipshipResponse_t* response);
static ChipZclStatus_t copySceneHelper(const ChipZclCommandContext_t *context,
                                        const ChipZclClusterScenesServerCommandCopySceneRequest_t *request);

// Private functions -----------------------------------------------------------

static ChipZclStatus_t addSceneHelper(const ChipZclCommandContext_t *context,
                                       const ChipZclClusterScenesServerCommandAddSceneRequest_t *request)
{
  ChipZclSceneEntry_t entry;
  bool enhanced = (context->commandId == CHIP_ZCL_CLUSTER_SCENES_SERVER_COMMAND_ENHANCED_ADD_SCENE);

  uint16_t groupId = request->groupId;
  uint8_t sceneId = request->sceneId;
  uint16_t transitionTime = request->transitionTime;

  char nameStr[ZCL_SCENES_CLUSTER_MAXIMUM_NAME_LENGTH + 1] = { 0 }; // temp storage for name string.
  MEMCOPY(nameStr, request->sceneName.ptr, request->sceneName.length); // Convert to terminated string.

  uint8_t extFieldSetsLen = request->extensionFieldSets.length;
  uint8_t *pExtFieldSetsData = request->extensionFieldSets.ptr;
  ChipZclEndpointId_t endpointId = context->endpointId;
  uint8_t i;
  uint8_t index = CHIP_ZCL_SCENE_TABLE_NULL_INDEX;

  chipAfCorePrint("%sAddScene 0x%2x, 0x%x, tr=0x%2x",
                   (enhanced ? "Enhanced" : ""),
                   groupId,
                   sceneId,
                   transitionTime);
  chipAfCorePrint(", %s, ", nameStr);
  chipAfCorePrintBuffer(pExtFieldSetsData, extFieldSetsLen, FALSE);
  chipAfCorePrintln("");

  // Add Scene commands can only reference groups to which we belong.
  if (groupId != ZCL_SCENES_GLOBAL_SCENE_GROUP_ID
      && !chipZclIsEndpointInGroup(endpointId, groupId)) {
    return CHIP_ZCL_STATUS_INVALID_FIELD;
  }

  for (i = 0; i < CHIP_AF_PLUGIN_SCENES_SERVER_TABLE_SIZE; i++) {
    chipZclPluginScenesServerRetrieveSceneEntry(entry, i);
    if (entry.endpointId == endpointId
        && entry.groupId == groupId
        && entry.sceneId == sceneId) {
      index = i;
      break;
    } else if (index == CHIP_ZCL_SCENE_TABLE_NULL_INDEX
               && entry.endpointId == CHIP_ZCL_SCENE_TABLE_UNUSED_ENDPOINT_ID) {
      index = i;
    }
  }

  // If the index is still null, the Scene table is full.
  if (index == CHIP_ZCL_SCENE_TABLE_NULL_INDEX) {
    return CHIP_ZCL_STATUS_INSUFFICIENT_SPACE;
  }

  chipZclPluginScenesServerRetrieveSceneEntry(entry, index);

  // The transition time is specified in seconds in the regular version of the
  // command and tenths of a second in the enhanced version.
  if (enhanced) {
    entry.transitionTime = transitionTime / 10;
    entry.transitionTime100ms = (uint8_t)(transitionTime - entry.transitionTime * 10);
  } else {
    entry.transitionTime = transitionTime;
    entry.transitionTime100ms = 0;
  }

#ifdef CHIP_AF_PLUGIN_SCENES_SERVER_NAME_SUPPORT
  strcpy((char*)entry.name, nameStr);
#endif

  bool newScene = (i != index);
  if (newScene) {
    // Cluster specific erase scene functionality is handled here by registered
    // cluster plugin callbacks.
    chZclEraseScene(index);
  }

  // Loop thru extension field data bytes adding cluster data to scene.
  // (Note- spec says extension field data can be omitted)
  uint8_t *pData = pExtFieldSetsData;
  while (pData < pExtFieldSetsData + extFieldSetsLen) {
    // Each extension field data set must contain a two-byte cluster id and a
    // one-byte length followed by the attribute data bytes, otherwise, the data
    // format is malformed.
    if (pData + 3 > pExtFieldSetsData + extFieldSetsLen) {
      return CHIP_ZCL_STATUS_MALFORMED_COMMAND; // data format error.
    }

    ChipZclClusterId_t clusterId = chipZclPluginScenesServerGetUint16FromBuffer(&pData);
    uint8_t length = chipZclPluginScenesServerGetUint8FromBuffer(&pData);
    if (length > 0) {
      if (pData + length > pExtFieldSetsData + extFieldSetsLen) {
        return CHIP_ZCL_STATUS_MALFORMED_COMMAND; // ext field data format error.
      }

      // Cluster specific AddScene extension field functionality is handled
      // here by registered cluster plugin callbacks.
      const uint8_t *sceneData = (const uint8_t *)pData;
      chZclAddScene(clusterId, index, sceneData, length);

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

    chipZclPluginScenesServerIncNumSceneEntriesInUse();

    chipZclClusterScenesServerSetSceneCountAttribute(endpointId,
                                                      chipZclPluginScenesServerGetNumSceneEntriesInUse());
  }
  chipZclPluginScenesServerSaveSceneEntry(entry, index);

  return CHIP_ZCL_STATUS_SUCCESS;
}

static void viewSceneHelper(const ChipZclCommandContext_t *context,
                            const ChipZclClusterScenesServerCommandViewSceneRequest_t *request,
                            ChipZclClusterScenesServerCommandViewSceneResponse_t* response)
{
  // Builds and sends the View Scene response.

  ChipZclSceneEntry_t entry;
  ChipZclStatus_t status = CHIP_ZCL_STATUS_NOT_FOUND;
  bool enhanced = (context->commandId == CHIP_ZCL_CLUSTER_SCENES_SERVER_COMMAND_ENHANCED_VIEW_SCENE);
  ChipZclEndpointId_t endpointId = context->endpointId;
  uint16_t groupId = request->groupId;
  uint8_t sceneId = request->sceneId;
  uint8_t index;

  chipAfCorePrintln("%sViewScene 0x%2x, 0x%x",
                     (enhanced ? "Enhanced" : ""),
                     groupId,
                     sceneId);

  // View Scene commands can only be addressed to a single device and only
  // referencing groups to which we belong.
  if (groupId != ZCL_SCENES_GLOBAL_SCENE_GROUP_ID
      && !chipZclIsEndpointInGroup(endpointId, groupId)) {
    status = CHIP_ZCL_STATUS_INVALID_FIELD;
  } else {
    for (index = 0; index < CHIP_AF_PLUGIN_SCENES_SERVER_TABLE_SIZE; index++) {
      chipZclPluginScenesServerRetrieveSceneEntry(entry, index);
      if (entry.endpointId == endpointId
          && entry.groupId == groupId
          && entry.sceneId == sceneId) {
        status = CHIP_ZCL_STATUS_SUCCESS;
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

  if (status == CHIP_ZCL_STATUS_SUCCESS) {
    // The transition time is returned in seconds in the regular version of the
    // command and tenths of a second in the enhanced version.
    response->transitionTime = (enhanced)
                               ? entry.transitionTime * 10 + entry.transitionTime100ms
                               : entry.transitionTime;

    char nameStr[ZCL_SCENES_CLUSTER_MAXIMUM_NAME_LENGTH + 1] = { 0 }; // temp storage for name string.
    #ifdef CHIP_AF_PLUGIN_SCENES_SERVER_NAME_SUPPORT
    MEMCOPY(nameStr, entry.name, ZCL_SCENES_CLUSTER_MAXIMUM_NAME_LENGTH);
    #endif
    response->sceneName.ptr = (uint8_t *)nameStr;
    response->sceneName.length = strlen(nameStr);

    uint8_t sceneExtFieldSetBuffer[ZCL_SCENES_CLUSTER_MAXIMUM_EXT_FIELD_LENGTH];  // temp storage for extension data bytes.
    uint8_t *pData = sceneExtFieldSetBuffer;
    response->extensionFieldSets.ptr = pData;

    // Cluster specific View scene functionality is handled here by registered plugin callbacks.
    chZclViewScene(index, &pData);

    response->extensionFieldSets.length = pData - response->extensionFieldSets.ptr; // Data has been added to buffer so update response length.
  }

  chipZclSendClusterScenesServerCommandViewSceneResponse(context, response); // Send the response.
}

static ChipZclStatus_t removeSceneHelper(const ChipZclCommandContext_t *context,
                                          const ChipZclClusterScenesServerCommandRemoveSceneRequest_t *request)
{
  ChipZclStatus_t status = CHIP_ZCL_STATUS_NOT_FOUND;

  uint16_t groupId = request->groupId;
  uint8_t sceneId = request->sceneId;
  ChipZclEndpointId_t endpointId = context->endpointId;

  chipAfCorePrintln("RemoveScene 0x%2x, 0x%x", groupId, sceneId);

  // If a group id is specified but this endpointId isn't in it, take no action.
  if (groupId != ZCL_SCENES_GLOBAL_SCENE_GROUP_ID
      && !chipZclIsEndpointInGroup(endpointId, groupId)) {
    status = CHIP_ZCL_STATUS_INVALID_FIELD;
  } else {
    uint8_t i;
    for (i = 0; i < CHIP_AF_PLUGIN_SCENES_SERVER_TABLE_SIZE; i++) {
      ChipZclSceneEntry_t entry;
      chipZclPluginScenesServerRetrieveSceneEntry(entry, i);
      if (entry.endpointId == endpointId
          && entry.groupId == groupId
          && entry.sceneId == sceneId) {
        entry.endpointId = CHIP_ZCL_SCENE_TABLE_UNUSED_ENDPOINT_ID;
        chipZclPluginScenesServerSaveSceneEntry(entry, i);
        chipZclPluginScenesServerDecNumSceneEntriesInUse();
        chipZclClusterScenesServerSetSceneCountAttribute(serverEndpointId,
                                                          chipZclPluginScenesServerGetNumSceneEntriesInUse());
        status = CHIP_ZCL_STATUS_SUCCESS;
        break;
      }
    }
  }

  return status;
}

static ChipZclStatus_t removeAllScenesHelper(const ChipZclCommandContext_t *context,
                                              const ChipZclClusterScenesServerCommandRemoveAllScenesRequest_t *request)
{
  ChipZclStatus_t status = CHIP_ZCL_STATUS_INVALID_FIELD;

  ChipZclEndpointId_t endpointId = context->endpointId;
  uint16_t groupId = request->groupId;

  chipAfCorePrintln("RemoveAllScenes 0x%2x", groupId);

  if (groupId == ZCL_SCENES_GLOBAL_SCENE_GROUP_ID
      || chipZclIsEndpointInGroup(endpointId, groupId)) {
    uint8_t i;
    status = CHIP_ZCL_STATUS_SUCCESS;
    for (i = 0; i < CHIP_AF_PLUGIN_SCENES_SERVER_TABLE_SIZE; i++) {
      ChipZclSceneEntry_t entry;
      chipZclPluginScenesServerRetrieveSceneEntry(entry, i);
      if (entry.endpointId == endpointId
          && entry.groupId == groupId) {
        entry.endpointId = CHIP_ZCL_SCENE_TABLE_UNUSED_ENDPOINT_ID;
        chipZclPluginScenesServerSaveSceneEntry(entry, i);
        chipZclPluginScenesServerDecNumSceneEntriesInUse();
      }
    }
    chipZclClusterScenesServerSetSceneCountAttribute(endpointId,
                                                      chipZclPluginScenesServerGetNumSceneEntriesInUse());
  }

  return status;
}

static ChipZclStatus_t storeCurrentSceneHelper(const ChipZclCommandContext_t *context,
                                                const ChipZclClusterScenesServerCommandStoreSceneRequest_t *request)
{
  ChipZclSceneEntry_t entry;
  uint8_t i;
  uint8_t index = CHIP_ZCL_SCENE_TABLE_NULL_INDEX;

  ChipZclEndpointId_t endpointId = context->endpointId;
  uint16_t groupId = request->groupId;
  uint8_t sceneId = request->sceneId;

  // If a group id is specified but this endpointId isn't in it, take no action.
  if (groupId != ZCL_SCENES_GLOBAL_SCENE_GROUP_ID
      && !chipZclIsEndpointInGroup(endpointId, groupId)) {
    return CHIP_ZCL_STATUS_INVALID_FIELD;
  }

  for (i = 0; i < CHIP_AF_PLUGIN_SCENES_SERVER_TABLE_SIZE; i++) {
    chipZclPluginScenesServerRetrieveSceneEntry(entry, i);
    if (entry.endpointId == endpointId
        && entry.groupId == groupId
        && entry.sceneId == sceneId) {
      index = i;
      break;
    } else if (index == CHIP_ZCL_SCENE_TABLE_NULL_INDEX
               && entry.endpointId == CHIP_ZCL_SCENE_TABLE_UNUSED_ENDPOINT_ID) {
      index = i;
    }
  }

  // If the target index is still zero, the table is full.
  if (index == CHIP_ZCL_SCENE_TABLE_NULL_INDEX) {
    return CHIP_ZCL_STATUS_INSUFFICIENT_SPACE;
  }

  chipZclPluginScenesServerRetrieveSceneEntry(entry, index);

  // Cluster specific Store scene functionality is handled here by registered plugin callbacks.
  chZclStoreScene(endpointId, index);

  // When creating a new entry, the name is set to the null string (i.e., the
  // length is set to zero) and the transition time is set to zero.  The scene
  // count must be increased and written to the attribute table when adding a
  // new scene.  Otherwise, these fields and the count are left alone.
  if (i != index) {
    entry.endpointId = endpointId;
    entry.groupId = groupId;
    entry.sceneId = sceneId;
#ifdef CHIP_AF_PLUGIN_SCENES_SERVER_NAME_SUPPORT
    entry.name[0] = 0;
#endif
    entry.transitionTime = 0;
    entry.transitionTime100ms = 0;

    chipZclPluginScenesServerIncNumSceneEntriesInUse();
    chipZclClusterScenesServerSetSceneCountAttribute(endpointId,
                                                      chipZclPluginScenesServerGetNumSceneEntriesInUse());
  }

  // Save the scene entry and mark is as valid by storing its scene and group
  // ids in the attribute table and setting valid to true.
  chipZclPluginScenesServerSaveSceneEntry(entry, index);
  chipZclClusterScenesServerMakeValid(endpointId, sceneId, groupId);

  return CHIP_ZCL_STATUS_SUCCESS;
}

static ChipZclStatus_t recallSavedSceneHelper(const ChipZclCommandContext_t *context,
                                               const ChipZclClusterScenesServerCommandRecallSceneRequest_t *request)
{
  uint16_t groupId = request->groupId;
  uint8_t sceneId = request->sceneId;
  ChipZclEndpointId_t endpointId = context->endpointId;

  if (groupId != ZCL_SCENES_GLOBAL_SCENE_GROUP_ID
      && !chipZclIsEndpointInGroup(endpointId, groupId)) {
    return CHIP_ZCL_STATUS_INVALID_FIELD;
  } else {
    for (uint8_t tblIdx = 0; tblIdx < CHIP_AF_PLUGIN_SCENES_SERVER_TABLE_SIZE; tblIdx++) {
      ChipZclSceneEntry_t entry;
      chipZclPluginScenesServerRetrieveSceneEntry(entry, tblIdx);
      if (entry.endpointId == endpointId
          && entry.groupId == groupId
          && entry.sceneId == sceneId) {
        // Calculate Recall transition time in 1/10S units.
        uint32_t transitionTime100mS = entry.transitionTime * 10 + entry.transitionTime100ms;

        // Cluster specific Recall scene functionality is handled here by registered plugin callbacks.
        chZclRecallScene(endpointId, tblIdx, transitionTime100mS);

        chipZclClusterScenesServerMakeValid(endpointId, sceneId, groupId);

        return CHIP_ZCL_STATUS_SUCCESS;
      }
    }
  }

  return CHIP_ZCL_STATUS_NOT_FOUND;
}

static void getSceneMchipshipHelper(const ChipZclCommandContext_t *context,
                                     const ChipZclClusterScenesServerCommandGetSceneMchipshipRequest_t *request,
                                     ChipZclClusterScenesServerCommandGetSceneMchipshipResponse_t* response)
{
  // Builds the Scene Mchipship response.

  ChipZclStatus_t status = CHIP_ZCL_STATUS_SUCCESS;
  ChipZclEndpointId_t endpointId = context->endpointId;
  uint16_t groupId = request->groupId;

  if ((groupId != ZCL_SCENES_GLOBAL_SCENE_GROUP_ID)
      && !chipZclIsEndpointInGroup(endpointId, groupId)) {
    status = CHIP_ZCL_STATUS_INVALID_FIELD;
  }

  // The status, capacity, and group id are always included in the response, but
  // the scene count and scene list are only included if the group id matched.
  response->status = status;
  response->capacity = CHIP_AF_PLUGIN_SCENES_SERVER_TABLE_SIZE
                       - chipZclPluginScenesServerGetNumSceneEntriesInUse();
  response->groupId = groupId;

  if (status == CHIP_ZCL_STATUS_SUCCESS) {
    uint8_t sceneCount = 0;
    uint8_t sceneListBuffer[CHIP_AF_PLUGIN_SCENES_SERVER_TABLE_SIZE] = { 0 }; // local storage

    for (uint8_t i = 0; i < CHIP_AF_PLUGIN_SCENES_SERVER_TABLE_SIZE; i++) {
      ChipZclSceneEntry_t entry;
      chipZclPluginScenesServerRetrieveSceneEntry(entry, i);
      if (entry.endpointId == endpointId
          && entry.groupId == groupId) {
        sceneListBuffer[sceneCount] = entry.sceneId;
        sceneCount++;
      }
    }

    // Set the response.
    response->sceneList.ptr = (uint8_t *)sceneListBuffer;
    response->sceneList.numElementsToEncode = sceneCount;
    response->sceneList.fieldData.valueType = CHIP_ZCLIP_TYPE_UNSIGNED_INTEGER;
    response->sceneList.fieldData.valueSize = sizeof(sceneListBuffer[0]);
  }

  chipZclSendClusterScenesServerCommandGetSceneMchipshipResponse(context,
                                                                   response); // Send the response.
}

static ChipZclStatus_t copySceneHelper(const ChipZclCommandContext_t *context,
                                        const ChipZclClusterScenesServerCommandCopySceneRequest_t *request)
{
  ChipZclSceneEntry_t fromEntry;
  ChipZclSceneEntry_t toEntry;
  uint8_t i;

  ChipZclEndpointId_t endpointId = context->endpointId;

  uint8_t mode = request->mode;
  bool copyAllScenes = mode & 0x01;  // If bit 0 is set all scenes in a GroupA will be copied to GroupB.
  copyAllScenes = false;             //TODO- Copy all scenes mode not presently implemented.

  uint16_t groupIdFrom = request->groupIdFrom;
  uint8_t sceneIdFrom = request->sceneIdFrom;
  uint16_t groupIdTo = request->groupIdTo;
  uint8_t sceneIdTo = request->sceneIdTo;

  // Validate from/to group id endpoints.
  if ((groupIdFrom == ZCL_SCENES_GLOBAL_SCENE_GROUP_ID)
      || (!chipZclIsEndpointInGroup(endpointId, groupIdFrom))
      || (groupIdTo == ZCL_SCENES_GLOBAL_SCENE_GROUP_ID)
      || (!chipZclIsEndpointInGroup(endpointId, groupIdTo))) {
    return CHIP_ZCL_STATUS_INVALID_FIELD;
  }

  // Find the 'from' scene.
  uint8_t fromIdx = CHIP_ZCL_SCENE_TABLE_NULL_INDEX;
  for (i = 0; i < CHIP_AF_PLUGIN_SCENES_SERVER_TABLE_SIZE; i++) {
    chipZclPluginScenesServerRetrieveSceneEntry(fromEntry, i);
    if ((fromEntry.endpointId == endpointId)
        && (fromEntry.groupId == groupIdFrom)
        && ((fromEntry.sceneId == sceneIdFrom) || copyAllScenes)) {
      fromIdx = i;
      break;
    }
  }
  if (fromIdx == CHIP_ZCL_SCENE_TABLE_NULL_INDEX) {
    return CHIP_ZCL_STATUS_NOT_FOUND;  // 'from' scene not found
  }

  // Find the 'to' scene.
  uint8_t toIdx = CHIP_ZCL_SCENE_TABLE_NULL_INDEX;
  for (i = 0; i < CHIP_AF_PLUGIN_SCENES_SERVER_TABLE_SIZE; i++) {
    chipZclPluginScenesServerRetrieveSceneEntry(toEntry, i);
    if ((toEntry.endpointId == endpointId)
        && (toEntry.groupId == groupIdTo)
        && ((toEntry.sceneId == sceneIdTo) || copyAllScenes)) {
      toIdx = i;
      break;
    }
  }
  if (toIdx == CHIP_ZCL_SCENE_TABLE_NULL_INDEX) {
    // Couldn't find exact match for destination scene, loop again
    // and allocate to first unused slot in scenes table.
    for (i = 0; i < CHIP_AF_PLUGIN_SCENES_SERVER_TABLE_SIZE; i++) {
      chipZclPluginScenesServerRetrieveSceneEntry(toEntry, i);
      if (toEntry.endpointId == CHIP_ZCL_SCENE_TABLE_UNUSED_ENDPOINT_ID) {
        toIdx = i;
        break;
      }
    }
  }
  if (toIdx == CHIP_ZCL_SCENE_TABLE_NULL_INDEX) {
    return CHIP_ZCL_STATUS_NOT_FOUND;  // 'to' scene not found or table full.
  }

  //chipAfCorePrintln("RX: CopyScene 0x%x 0x%2x 0x%2x 0x%2x 0x%2x", mode, groupIdFrom, sceneIdFrom, groupIdTo, sceneIdTo);

  // Copy the scene table entry.
  fromEntry.groupId = groupIdTo;
  fromEntry.sceneId = sceneIdTo;
  chipZclPluginScenesServerSaveSceneEntry(fromEntry, toIdx);

  // Cluster specific copy scene functionality is handled here by registered plugin callbacks.
  chZclCopyScene(fromIdx, toIdx);

  return CHIP_ZCL_STATUS_SUCCESS;
}

// Public functions ------------------------------------------------------------

void chZclScenesServerInitHandler(void)
{
  serverEndpointId = 0x01; //TODO, get this from app Metadata.

  #ifndef DEFINETOKENS
  for (uint8_t i = 0; i < CHIP_AF_PLUGIN_SCENES_SERVER_TABLE_SIZE; ++i) {
    chipZclPluginScenesServerSceneTable[i].endpointId
      = CHIP_ZCL_SCENE_TABLE_UNUSED_ENDPOINT_ID; // If using ram-based Scenes table, init scene table entry eps = unused.
  }
  #endif

  #ifdef CHIP_AF_PLUGIN_SCENES_SERVER_NAME_SUPPORT
  {
    // The high bit of Name Support indicates whether scene names are supported.
    uint8_t nameSupport = BIT(7); // Set bit.
    chipZclWriteAttribute(serverEndpointId,
                           &chipZclClusterScenesServerSpec,
                           CHIP_ZCL_CLUSTER_SCENES_SERVER_ATTRIBUTE_SCENE_NAME_SUPPORT,
                           (uint8_t *)&nameSupport,
                           sizeof(nameSupport));
  }
  #endif

  chipZclClusterScenesServerSetSceneCountAttribute(serverEndpointId,
                                                    chipZclPluginScenesServerGetNumSceneEntriesInUse());
}

ChipZclStatus_t chipZclClusterScenesServerSetSceneCountAttribute(ChipZclEndpointId_t endpointId,
                                                                   uint8_t newCount)
{
  return chipZclWriteAttribute(endpointId,
                                &chipZclClusterScenesServerSpec,
                                CHIP_ZCL_CLUSTER_SCENES_SERVER_ATTRIBUTE_SCENE_COUNT,
                                (uint8_t *)&newCount,
                                sizeof(newCount));
}

ChipZclStatus_t chipZclClusterScenesServerMakeValid(ChipZclEndpointId_t endpointId,
                                                      uint8_t sceneId,
                                                      uint16_t groupId)
{
  ChipZclStatus_t status;

  // scene ID
  status = chipZclWriteAttribute(endpointId,
                                  &chipZclClusterScenesServerSpec,
                                  CHIP_ZCL_CLUSTER_SCENES_SERVER_ATTRIBUTE_CURRENT_SCENE,
                                  (uint8_t *)&sceneId,
                                  sizeof(sceneId));
  if (status != CHIP_ZCL_STATUS_SUCCESS) {
    return status;
  }

  // group ID
  status = chipZclWriteAttribute(endpointId,
                                  &chipZclClusterScenesServerSpec,
                                  CHIP_ZCL_CLUSTER_SCENES_SERVER_ATTRIBUTE_CURRENT_GROUP,
                                  (uint8_t *)&groupId,
                                  sizeof(groupId));
  if (status != CHIP_ZCL_STATUS_SUCCESS) {
    return status;
  }

  bool valid = true;
  status = chipZclWriteAttribute(endpointId,
                                  &chipZclClusterScenesServerSpec,
                                  CHIP_ZCL_CLUSTER_SCENES_SERVER_ATTRIBUTE_SCENE_VALID,
                                  (uint8_t *)&valid,
                                  sizeof(valid));

  return status;
}

ChipZclStatus_t chipZclClusterScenesServerMakeInvalidCallback(ChipZclEndpointId_t endpointId)
{
  bool valid = FALSE;
  return chipZclWriteAttribute(endpointId,
                                &chipZclClusterScenesServerSpec,
                                CHIP_ZCL_CLUSTER_SCENES_SERVER_ATTRIBUTE_SCENE_VALID,
                                (uint8_t *)&valid,
                                sizeof(valid));
}

void chipZclClusterScenesServerPrintInfo(void)
{
  uint8_t i;
  ChipZclSceneEntry_t entry;
  chipAfCorePrintln("using 0x%x out of 0x%x table slots",
                     chipZclPluginScenesServerGetNumSceneEntriesInUse(),
                     CHIP_AF_PLUGIN_SCENES_SERVER_TABLE_SIZE);
  for (i = 0; i < CHIP_AF_PLUGIN_SCENES_SERVER_TABLE_SIZE; i++) {
    chipZclPluginScenesServerRetrieveSceneEntry(entry, i);

    chipAfCorePrint("%x: ", i);

    if (entry.endpointId != CHIP_ZCL_SCENE_TABLE_UNUSED_ENDPOINT_ID) {
      chipAfCorePrint("ep:%x grp:%2x scene:%x tt:%d",
                       entry.endpointId,
                       entry.groupId,
                       entry.sceneId,
                       entry.transitionTime);

      chipAfCorePrint(".%d", entry.transitionTime100ms);

      #ifdef CHIP_AF_PLUGIN_SCENES_SERVER_NAME_SUPPORT
      chipAfCorePrint(" name(%x)\"", strlen(entry.name));
      chipAfCorePrint("%s", entry.name);
      chipAfCorePrint("\"");
      #endif

      // Cluster specific scene print info functionality is handled here by registered plugin callbacks.
      chZclPrintInfoScene(i);

      chipAfCorePrintln("");
    }
  }
}

void chipZclClusterScenesServerClearSceneTable(ChipZclEndpointId_t endpointId)
{
  for (uint8_t i = 0; i < CHIP_AF_PLUGIN_SCENES_SERVER_TABLE_SIZE; i++) {
    ChipZclSceneEntry_t entry;
    chipZclPluginScenesServerRetrieveSceneEntry(entry, i);
    if ((endpointId == entry.endpointId)
        || (endpointId == CHIP_ZCL_SCENE_TABLE_UNUSED_ENDPOINT_ID)) {
      entry.endpointId = CHIP_ZCL_SCENE_TABLE_UNUSED_ENDPOINT_ID;
      chipZclPluginScenesServerSaveSceneEntry(entry, i);
    }
  }
  chipZclPluginScenesServerSetNumSceneEntriesInUse(0);
  chipZclClusterScenesServerSetSceneCountAttribute(endpointId, 0);
}

void chipZclClusterScenesServerRemoveScenesInGroupCallback(ChipZclEndpointId_t endpointId,
                                                            uint16_t groupId)
{
  for (uint8_t i = 0; i < CHIP_AF_PLUGIN_SCENES_SERVER_TABLE_SIZE; i++) {
    ChipZclSceneEntry_t entry;
    chipZclPluginScenesServerRetrieveSceneEntry(entry, i);
    if (entry.endpointId == endpointId
        && entry.groupId == groupId) {
      entry.groupId = ZCL_SCENES_GLOBAL_SCENE_GROUP_ID;
      entry.endpointId = CHIP_ZCL_SCENE_TABLE_UNUSED_ENDPOINT_ID;
      chipZclPluginScenesServerSaveSceneEntry(entry, i);
      chipZclPluginScenesServerDecNumSceneEntriesInUse();
      chipZclClusterScenesServerSetSceneCountAttribute(serverEndpointId,
                                                        chipZclPluginScenesServerGetNumSceneEntriesInUse());
    }
  }
}

// Zcl command handlers...

void chipZclClusterScenesServerCommandAddSceneRequestHandler(const ChipZclCommandContext_t *context,
                                                              const ChipZclClusterScenesServerCommandAddSceneRequest_t *request)
{
  ChipZclStatus_t status;

  chipAfCorePrintln("RX: AddScene");

  // Add the scene.
  status = addSceneHelper(context, request);

  // Send the response msg.
  ChipZclClusterScenesServerCommandAddSceneResponse_t response = { 0 };
  response.status = status;
  response.groupId = request->groupId;
  response.sceneId = request->sceneId;
  chipZclSendClusterScenesServerCommandAddSceneResponse(context, &response);
}

void chipZclClusterScenesServerCommandViewSceneRequestHandler(const ChipZclCommandContext_t *context,
                                                               const ChipZclClusterScenesServerCommandViewSceneRequest_t *request)
{
  chipAfCorePrintln("RX: ViewScene");

  // Build and send the response message.
  ChipZclClusterScenesServerCommandViewSceneResponse_t response = { 0 };
  viewSceneHelper(context, request, &response);
}

void chipZclClusterScenesServerCommandRemoveSceneRequestHandler(const ChipZclCommandContext_t *context,
                                                                 const ChipZclClusterScenesServerCommandRemoveSceneRequest_t *request)
{
  ChipZclStatus_t status;

  chipAfCorePrintln("RX: RemoveScene");

  // Remove the scene.
  status = removeSceneHelper(context, request);

  // Send the response msg.
  ChipZclClusterScenesServerCommandRemoveSceneResponse_t response = { 0 };
  response.status = status;
  response.groupId = request->groupId;
  response.sceneId = request->sceneId;
  chipZclSendClusterScenesServerCommandRemoveSceneResponse(context, &response);
}

void chipZclClusterScenesServerCommandRemoveAllScenesRequestHandler(const ChipZclCommandContext_t *context,
                                                                     const ChipZclClusterScenesServerCommandRemoveAllScenesRequest_t *request)
{
  ChipZclStatus_t status;

  chipAfCorePrintln("RX: RemoveAllScenes");

  // Remove all scenes in the specified group.
  status = removeAllScenesHelper(context, request);

  // Send the response msg.
  ChipZclClusterScenesServerCommandRemoveAllScenesResponse_t response = { 0 };
  response.status = status;
  response.groupId = request->groupId;
  chipZclSendClusterScenesServerCommandRemoveAllScenesResponse(context, &response);
}

void chipZclClusterScenesServerCommandStoreSceneRequestHandler(const ChipZclCommandContext_t *context,
                                                                const ChipZclClusterScenesServerCommandStoreSceneRequest_t *request)
{
  ChipZclStatus_t status;

  chipAfCorePrintln("RX: StoreScene");

  // Store current scene.
  status = storeCurrentSceneHelper(context, request);

  // Send the response msg.
  ChipZclClusterScenesServerCommandStoreSceneResponse_t response = { 0 };
  response.status = status;
  response.groupId = request->groupId;
  response.sceneId = request->sceneId;
  chipZclSendClusterScenesServerCommandStoreSceneResponse(context, &response);
}

void chipZclClusterScenesServerCommandRecallSceneRequestHandler(const ChipZclCommandContext_t *context,
                                                                 const ChipZclClusterScenesServerCommandRecallSceneRequest_t *request)
{
  ChipZclStatus_t status;

  chipAfCorePrintln("RX: RecallScene");

  // Recall specified saved scene.
  status = recallSavedSceneHelper(context, request);

  // Send default response msg.
  chipZclSendDefaultResponse(context, status);
}

void chipZclClusterScenesServerCommandGetSceneMchipshipRequestHandler(const ChipZclCommandContext_t *context,
                                                                        const ChipZclClusterScenesServerCommandGetSceneMchipshipRequest_t *request)
{
  chipAfCorePrintln("RX: GetSceneMchipship");

  // Build and send the response message.
  ChipZclClusterScenesServerCommandGetSceneMchipshipResponse_t response = { 0 };
  getSceneMchipshipHelper(context, request, &response);
}

void chipZclClusterScenesServerCommandEnhancedAddSceneRequestHandler(const ChipZclCommandContext_t *context,
                                                                      const ChipZclClusterScenesServerCommandEnhancedAddSceneRequest_t *request)
{
  ChipZclStatus_t status;

  chipAfCorePrintln("RX: EnAddScene");

  // Add the scene.
  status = addSceneHelper(context,
                          (ChipZclClusterScenesServerCommandAddSceneRequest_t *)request);

  // Send the response msg.
  ChipZclClusterScenesServerCommandEnhancedAddSceneResponse_t response = { 0 };
  response.status = status;
  response.groupId = request->groupId;
  response.sceneId = request->sceneId;
  chipZclSendClusterScenesServerCommandEnhancedAddSceneResponse(context, &response);
}

void chipZclClusterScenesServerCommandEnhancedViewSceneRequestHandler(const ChipZclCommandContext_t *context,
                                                                       const ChipZclClusterScenesServerCommandEnhancedViewSceneRequest_t *request)
{
  chipAfCorePrintln("RX: EnViewScene");

  // Build and send the response message.
  ChipZclClusterScenesServerCommandEnhancedViewSceneResponse_t response = { 0 };
  viewSceneHelper(context,
                  (ChipZclClusterScenesServerCommandViewSceneRequest_t *)request,
                  (ChipZclClusterScenesServerCommandViewSceneResponse_t *)&response);
}

void chipZclClusterScenesServerCommandCopySceneRequestHandler(const ChipZclCommandContext_t *context,
                                                               const ChipZclClusterScenesServerCommandCopySceneRequest_t *request)
{
  ChipZclStatus_t status;

  chipAfCorePrintln("RX: CopyScene");

  // Copy the scene(s).
  status = copySceneHelper(context, request);

  // Send the response msg.
  ChipZclClusterScenesServerCommandCopySceneResponse_t response = { 0 };
  response.status = status;
  response.groupIdFrom = request->groupIdFrom;
  response.sceneIdFrom = request->sceneIdFrom;
  chipZclSendClusterScenesServerCommandCopySceneResponse(context, &response);
}

// Scenes utilities

uint8_t chipZclPluginScenesServerGetUint8FromBuffer(uint8_t **ptr)
{
  // This fn modifies source ptr on exit.
  uint8_t value = **ptr;
  *ptr += 1;
  return value;
}

void chipZclPluginScenesServerPutUint8InBuffer(uint8_t **ptr, uint8_t value)
{
  // This fn modifies source ptr on exit.
  **ptr = value;
  *ptr += 1;
}

uint16_t chipZclPluginScenesServerGetUint16FromBuffer(uint8_t **ptr)
{
  // (where Buffer == Big-endian)
  // This fn modifies source ptr on exit.
  uint16_t value = chipFetchHighLowInt16u(*ptr);
  *ptr += 2;
  return value;
}

void chipZclPluginScenesServerPutUint16InBuffer(uint8_t **ptr, uint16_t value)
{
  // (where Buffer == Big-endian)
  // This fn modifies source ptr on exit.
  chipStoreHighLowInt16u(*ptr, value);
  *ptr += 2;
}

// Scenes callback placeholders...

void chipZclEraseSceneCallback(uint8_t tableIdx)
{
  // Cluster specific callbacks for Delete scene function will be called in turn following
  // execution of this fn.
}

bool chipZclAddSceneCallback(ChipZclClusterId_t clusterId,
                              uint8_t tableIdx,
                              const uint8_t *sceneData,
                              uint8_t length)
{
  // Cluster specific callbacks for Add scene function will be called in turn following
  // execution of this fn.
  return false;
}

void chipZclRecallSceneCallback(ChipZclEndpointId_t endpointId,
                                 uint8_t tableIdx,
                                 uint32_t transitionTime100mS)
{
  // Cluster specific callbacks for Recall scene function will be called in turn following
  // execution of this fn. The parameter 'transitionTime100mS' enables the callbacks
  // to apply a recall transtion time (in 1/10S units) if desired.
}

void chipZclStoreSceneCallback(ChipZclEndpointId_t endpointId, uint8_t tableIdx)
{
  // Cluster specific callbacks for Store scene function will be called in turn following
  // execution of this fn.
}

void chipZclCopySceneCallback(uint8_t srcTableIdx, uint8_t dstTableIdx)
{
  // Cluster specific callbacks for Copy scene function will be called in turn following
  // execution of this fn.
}

void chipZclViewSceneCallback(uint8_t tableIdx, uint8_t **pExtFieldData)
{
  // Cluster specific callbacks for View scene function will be called in turn following
  // execution of this fn. (Note: pData references the Scene extension field byte array).
}

void chipZclPrintInfoSceneCallback(uint8_t tableIdx)
{
  // Cluster specific callbacks for PrintInfo scene function will be called in turn following
  // execution of this fn.
}
