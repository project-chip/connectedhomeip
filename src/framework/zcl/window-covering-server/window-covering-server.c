/***************************************************************************//**
 * @file
 * @brief
 ******************************************************************************/

#include PLATFORM_HEADER
#include CONFIGURATION_HEADER
#include CHIP_AF_API_STACK
#ifdef CHIP_AF_API_DEBUG_PRINT
  #include CHIP_AF_API_DEBUG_PRINT
#endif
#include CHIP_AF_API_ZCL_CORE
#ifdef CHIP_AF_API_ZCL_SCENES_SERVER
  #include CHIP_AF_API_ZCL_SCENES_SERVER
#endif
#include "thread-callbacks.h"
#include "window-covering-server.h"

#ifdef CHIP_AF_API_ZCL_SCENES_SERVER
  #ifdef DEFINETOKENS
// Token based storage.
    #define retrieveSceneSubTableEntry(entry, i) \
  halCommonGetIndexedToken(&entry, TOKEN_ZCL_CORE_WINDOW_COVERING_SCENE_SUBTABLE, i)
    #define saveSceneSubTableEntry(entry, i) \
  halCommonSetIndexedToken(TOKEN_ZCL_CORE_WINDOW_COVERING_SCENE_SUBTABLE, i, &entry)
  #else
// RAM based storage.
ChZclWindowCoveringSceneSubTableEntry_t chZclPluginWindowCoveringServerSceneSubTable[CHIP_AF_PLUGIN_SCENES_SERVER_TABLE_SIZE] = { { 0 } };
    #define retrieveSceneSubTableEntry(entry, i) \
  (entry = chZclPluginWindowCoveringServerSceneSubTable[i])
    #define saveSceneSubTableEntry(entry, i) \
  (chZclPluginWindowCoveringServerSceneSubTable[i] = entry)
  #endif
#endif

// ZCL server command handlers...

void chipZclClusterWindowCoveringServerCommandWindowCoveringUpOpenRequestHandler(
  const ChipZclCommandContext_t *context,
  const ChipZclClusterWindowCoveringServerCommandWindowCoveringUpOpenRequest_t *request)
{
  chipZclSendDefaultResponse(context, CHIP_ZCL_STATUS_UNSUP_CLUSTER_COMMAND);
}

void chipZclClusterWindowCoveringServerCommandWindowCoveringDownCloseRequestHandler(
  const ChipZclCommandContext_t *context,
  const ChipZclClusterWindowCoveringServerCommandWindowCoveringDownCloseRequest_t *request)
{
  chipZclSendDefaultResponse(context, CHIP_ZCL_STATUS_UNSUP_CLUSTER_COMMAND);
}

void chipZclClusterWindowCoveringServerCommandWindowCoveringStopRequestHandler(
  const ChipZclCommandContext_t *context,
  const ChipZclClusterWindowCoveringServerCommandWindowCoveringStopRequest_t *request)
{
  chipZclSendDefaultResponse(context, CHIP_ZCL_STATUS_UNSUP_CLUSTER_COMMAND);
}

void chipZclClusterWindowCoveringServerCommandWindowCoveringGoToLiftValueRequestHandler(
  const ChipZclCommandContext_t *context,
  const ChipZclClusterWindowCoveringServerCommandWindowCoveringGoToLiftValueRequest_t *request)
{
  chipZclSendDefaultResponse(context, CHIP_ZCL_STATUS_UNSUP_CLUSTER_COMMAND);
}

void chipZclClusterWindowCoveringServerCommandWindowCoveringGoToLiftPercentageRequestHandler(
  const ChipZclCommandContext_t *context,
  const ChipZclClusterWindowCoveringServerCommandWindowCoveringGoToLiftPercentageRequest_t *request)
{
  chipZclSendDefaultResponse(context, CHIP_ZCL_STATUS_UNSUP_CLUSTER_COMMAND);
}

void chipZclClusterWindowCoveringServerCommandWindowCoveringGoToTiltValueRequestHandler(
  const ChipZclCommandContext_t *context,
  const ChipZclClusterWindowCoveringServerCommandWindowCoveringGoToTiltValueRequest_t *request)
{
  chipZclSendDefaultResponse(context, CHIP_ZCL_STATUS_UNSUP_CLUSTER_COMMAND);
}

void chipZclClusterWindowCoveringServerCommandWindowCoveringGoToTiltPercentageRequestHandler(
  const ChipZclCommandContext_t *context,
  const ChipZclClusterWindowCoveringServerCommandWindowCoveringGoToTiltPercentageRequest_t *request)
{
  chipZclSendDefaultResponse(context, CHIP_ZCL_STATUS_UNSUP_CLUSTER_COMMAND);
}

#ifdef CHIP_AF_API_ZCL_SCENES_SERVER
// Scenes callback handlers...

void chZclWindowCoveringServerEraseSceneHandler(uint8_t tableIdx)
{
  ChZclWindowCoveringSceneSubTableEntry_t entry;

  entry.hasCurrentPositionLiftPercentageValue = false;
  entry.hasCurrentPositionTiltPercentageValue = false;

  saveSceneSubTableEntry(entry, tableIdx);
}

bool chZclWindowCoveringServerAddSceneHandler(ChipZclClusterId_t clusterId,
                                              uint8_t tableIdx,
                                              const uint8_t *sceneData,
                                              uint8_t length)
{
  if (clusterId == CHIP_ZCL_CLUSTER_WINDOW_COVERING) {
    if (length < 1) {
      return false; // ext field format error (currentPositionLiftPercentageValue byte must be present).
    }

    // Extract bytes from input data block and update scene subtable fields.
    ChZclWindowCoveringSceneSubTableEntry_t entry = { 0 };
    uint8_t *pData = (uint8_t *)sceneData;

    entry.hasCurrentPositionLiftPercentageValue = true;
    entry.currentPositionLiftPercentageValue = chipZclPluginScenesServerGetUint8FromBuffer(&pData);
    length -= 1;
    if (length >= 1) {
      entry.hasCurrentPositionTiltPercentageValue = true;
      entry.currentPositionTiltPercentageValue = chipZclPluginScenesServerGetUint8FromBuffer(&pData);
    }

    saveSceneSubTableEntry(entry, tableIdx);

    return true;
  }

  return false;
}

void chZclWindowCoveringServerRecallSceneHandler(ChipZclEndpointId_t endpointId,
                                                 uint8_t tableIdx,
                                                 uint32_t transitionTime100mS)
{
  // Handles the recallScene command for the window covering cluster.
  // Note- this handler presently just updates (writes) the relevant cluster
  // attribute(s), in a production system this could be replaced by a call
  // to the relevant window covering command handler to actually change the
  // hw state at the rate specified by the transition time.

  ChZclWindowCoveringSceneSubTableEntry_t entry;
  retrieveSceneSubTableEntry(entry, tableIdx);

  if (entry.hasCurrentPositionLiftPercentageValue) {
    chipZclWriteAttribute(endpointId,
                           &chipZclClusterWindowCoveringServerSpec,
                           CHIP_ZCL_CLUSTER_WINDOW_COVERING_SERVER_ATTRIBUTE_CURRENT_LIFT_PERCENTAGE,
                           (uint8_t *)&entry.currentPositionLiftPercentageValue,
                           sizeof(entry.currentPositionLiftPercentageValue));
  }
  if (entry.hasCurrentPositionTiltPercentageValue) {
    chipZclWriteAttribute(endpointId,
                           &chipZclClusterWindowCoveringServerSpec,
                           CHIP_ZCL_CLUSTER_WINDOW_COVERING_SERVER_ATTRIBUTE_CURRENT_TILT_PERCENTAGE,
                           (uint8_t *)&entry.currentPositionTiltPercentageValue,
                           sizeof(entry.currentPositionTiltPercentageValue));
  }
}

void chZclWindowCoveringServerStoreSceneHandler(ChipZclEndpointId_t endpointId,
                                                uint8_t tableIdx)
{
  ChZclWindowCoveringSceneSubTableEntry_t entry;

  entry.hasCurrentPositionLiftPercentageValue =
    (chipZclReadAttribute(endpointId,
                           &chipZclClusterWindowCoveringServerSpec,
                           CHIP_ZCL_CLUSTER_WINDOW_COVERING_SERVER_ATTRIBUTE_CURRENT_LIFT_PERCENTAGE,
                           (uint8_t *)&entry.currentPositionLiftPercentageValue,
                           sizeof(entry.currentPositionLiftPercentageValue)) == CHIP_ZCL_STATUS_SUCCESS);
  entry.hasCurrentPositionTiltPercentageValue =
    (chipZclReadAttribute(endpointId,
                           &chipZclClusterWindowCoveringServerSpec,
                           CHIP_ZCL_CLUSTER_WINDOW_COVERING_SERVER_ATTRIBUTE_CURRENT_TILT_PERCENTAGE,
                           (uint8_t *)&entry.currentPositionTiltPercentageValue,
                           sizeof(entry.currentPositionTiltPercentageValue)) == CHIP_ZCL_STATUS_SUCCESS);

  saveSceneSubTableEntry(entry, tableIdx);
}

void chZclWindowCoveringServerCopySceneHandler(uint8_t srcTableIdx,
                                               uint8_t dstTableIdx)
{
  ChZclWindowCoveringSceneSubTableEntry_t entry;
  retrieveSceneSubTableEntry(entry, srcTableIdx);

  saveSceneSubTableEntry(entry, dstTableIdx);
}

void chZclWindowCoveringServerViewSceneHandler(uint8_t tableIdx,
                                               uint8_t **ppExtFldData)
{
  ChZclWindowCoveringSceneSubTableEntry_t entry;
  retrieveSceneSubTableEntry(entry, tableIdx);

  if ((entry.hasCurrentPositionLiftPercentageValue)
      || (entry.hasCurrentPositionTiltPercentageValue)) {
    chipZclPluginScenesServerPutUint16InBuffer(ppExtFldData,
                                                CHIP_ZCL_CLUSTER_WINDOW_COVERING);

    uint8_t *pLength = *ppExtFldData;  // Save pointer to length byte.
    *pLength = 0;
    chipZclPluginScenesServerPutUint8InBuffer(ppExtFldData, *pLength); // Insert temporary length value.

    if (entry.hasCurrentPositionLiftPercentageValue) {
      chipZclPluginScenesServerPutUint8InBuffer(ppExtFldData,
                                                 entry.currentPositionLiftPercentageValue);
    }
    if (entry.hasCurrentPositionTiltPercentageValue) {
      chipZclPluginScenesServerPutUint8InBuffer(ppExtFldData,
                                                 entry.currentPositionTiltPercentageValue);
    }

    // Update length byte value.
    *pLength = *ppExtFldData - pLength - 1;
  }
}

void chZclWindowCoveringServerPrintInfoSceneHandler(uint8_t tableIdx)
{
  ChZclWindowCoveringSceneSubTableEntry_t entry;
  retrieveSceneSubTableEntry(entry, tableIdx);

  chipAfCorePrint(" window covering:%x %x",
                   entry.currentPositionLiftPercentageValue,
                   entry.currentPositionTiltPercentageValue);

  chipAfCoreFlush();
}
#endif
