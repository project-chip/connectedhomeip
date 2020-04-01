/***************************************************************************//**
 * @file
 * @brief
 *******************************************************************************
 * # License
 * <b>Copyright 2018 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 ******************************************************************************/

#include PLATFORM_HEADER
#include CONFIGURATION_HEADER
#include EMBER_AF_API_STACK
#ifdef EMBER_AF_API_DEBUG_PRINT
  #include EMBER_AF_API_DEBUG_PRINT
#endif
#include EMBER_AF_API_ZCL_CORE
#ifdef EMBER_AF_API_ZCL_SCENES_SERVER
  #include EMBER_AF_API_ZCL_SCENES_SERVER
#endif
#include "thread-callbacks.h"
#include "window-covering-server.h"

#ifdef EMBER_AF_API_ZCL_SCENES_SERVER
  #ifdef DEFINETOKENS
// Token based storage.
    #define retrieveSceneSubTableEntry(entry, i) \
  halCommonGetIndexedToken(&entry, TOKEN_ZCL_CORE_WINDOW_COVERING_SCENE_SUBTABLE, i)
    #define saveSceneSubTableEntry(entry, i) \
  halCommonSetIndexedToken(TOKEN_ZCL_CORE_WINDOW_COVERING_SCENE_SUBTABLE, i, &entry)
  #else
// RAM based storage.
EmZclWindowCoveringSceneSubTableEntry_t emZclPluginWindowCoveringServerSceneSubTable[EMBER_AF_PLUGIN_SCENES_SERVER_TABLE_SIZE] = { { 0 } };
    #define retrieveSceneSubTableEntry(entry, i) \
  (entry = emZclPluginWindowCoveringServerSceneSubTable[i])
    #define saveSceneSubTableEntry(entry, i) \
  (emZclPluginWindowCoveringServerSceneSubTable[i] = entry)
  #endif
#endif

// ZCL server command handlers...

void emberZclClusterWindowCoveringServerCommandWindowCoveringUpOpenRequestHandler(
  const EmberZclCommandContext_t *context,
  const EmberZclClusterWindowCoveringServerCommandWindowCoveringUpOpenRequest_t *request)
{
  emberZclSendDefaultResponse(context, EMBER_ZCL_STATUS_UNSUP_CLUSTER_COMMAND);
}

void emberZclClusterWindowCoveringServerCommandWindowCoveringDownCloseRequestHandler(
  const EmberZclCommandContext_t *context,
  const EmberZclClusterWindowCoveringServerCommandWindowCoveringDownCloseRequest_t *request)
{
  emberZclSendDefaultResponse(context, EMBER_ZCL_STATUS_UNSUP_CLUSTER_COMMAND);
}

void emberZclClusterWindowCoveringServerCommandWindowCoveringStopRequestHandler(
  const EmberZclCommandContext_t *context,
  const EmberZclClusterWindowCoveringServerCommandWindowCoveringStopRequest_t *request)
{
  emberZclSendDefaultResponse(context, EMBER_ZCL_STATUS_UNSUP_CLUSTER_COMMAND);
}

void emberZclClusterWindowCoveringServerCommandWindowCoveringGoToLiftValueRequestHandler(
  const EmberZclCommandContext_t *context,
  const EmberZclClusterWindowCoveringServerCommandWindowCoveringGoToLiftValueRequest_t *request)
{
  emberZclSendDefaultResponse(context, EMBER_ZCL_STATUS_UNSUP_CLUSTER_COMMAND);
}

void emberZclClusterWindowCoveringServerCommandWindowCoveringGoToLiftPercentageRequestHandler(
  const EmberZclCommandContext_t *context,
  const EmberZclClusterWindowCoveringServerCommandWindowCoveringGoToLiftPercentageRequest_t *request)
{
  emberZclSendDefaultResponse(context, EMBER_ZCL_STATUS_UNSUP_CLUSTER_COMMAND);
}

void emberZclClusterWindowCoveringServerCommandWindowCoveringGoToTiltValueRequestHandler(
  const EmberZclCommandContext_t *context,
  const EmberZclClusterWindowCoveringServerCommandWindowCoveringGoToTiltValueRequest_t *request)
{
  emberZclSendDefaultResponse(context, EMBER_ZCL_STATUS_UNSUP_CLUSTER_COMMAND);
}

void emberZclClusterWindowCoveringServerCommandWindowCoveringGoToTiltPercentageRequestHandler(
  const EmberZclCommandContext_t *context,
  const EmberZclClusterWindowCoveringServerCommandWindowCoveringGoToTiltPercentageRequest_t *request)
{
  emberZclSendDefaultResponse(context, EMBER_ZCL_STATUS_UNSUP_CLUSTER_COMMAND);
}

#ifdef EMBER_AF_API_ZCL_SCENES_SERVER
// Scenes callback handlers...

void emZclWindowCoveringServerEraseSceneHandler(uint8_t tableIdx)
{
  EmZclWindowCoveringSceneSubTableEntry_t entry;

  entry.hasCurrentPositionLiftPercentageValue = false;
  entry.hasCurrentPositionTiltPercentageValue = false;

  saveSceneSubTableEntry(entry, tableIdx);
}

bool emZclWindowCoveringServerAddSceneHandler(EmberZclClusterId_t clusterId,
                                              uint8_t tableIdx,
                                              const uint8_t *sceneData,
                                              uint8_t length)
{
  if (clusterId == EMBER_ZCL_CLUSTER_WINDOW_COVERING) {
    if (length < 1) {
      return false; // ext field format error (currentPositionLiftPercentageValue byte must be present).
    }

    // Extract bytes from input data block and update scene subtable fields.
    EmZclWindowCoveringSceneSubTableEntry_t entry = { 0 };
    uint8_t *pData = (uint8_t *)sceneData;

    entry.hasCurrentPositionLiftPercentageValue = true;
    entry.currentPositionLiftPercentageValue = emberZclPluginScenesServerGetUint8FromBuffer(&pData);
    length -= 1;
    if (length >= 1) {
      entry.hasCurrentPositionTiltPercentageValue = true;
      entry.currentPositionTiltPercentageValue = emberZclPluginScenesServerGetUint8FromBuffer(&pData);
    }

    saveSceneSubTableEntry(entry, tableIdx);

    return true;
  }

  return false;
}

void emZclWindowCoveringServerRecallSceneHandler(EmberZclEndpointId_t endpointId,
                                                 uint8_t tableIdx,
                                                 uint32_t transitionTime100mS)
{
  // Handles the recallScene command for the window covering cluster.
  // Note- this handler presently just updates (writes) the relevant cluster
  // attribute(s), in a production system this could be replaced by a call
  // to the relevant window covering command handler to actually change the
  // hw state at the rate specified by the transition time.

  EmZclWindowCoveringSceneSubTableEntry_t entry;
  retrieveSceneSubTableEntry(entry, tableIdx);

  if (entry.hasCurrentPositionLiftPercentageValue) {
    emberZclWriteAttribute(endpointId,
                           &emberZclClusterWindowCoveringServerSpec,
                           EMBER_ZCL_CLUSTER_WINDOW_COVERING_SERVER_ATTRIBUTE_CURRENT_LIFT_PERCENTAGE,
                           (uint8_t *)&entry.currentPositionLiftPercentageValue,
                           sizeof(entry.currentPositionLiftPercentageValue));
  }
  if (entry.hasCurrentPositionTiltPercentageValue) {
    emberZclWriteAttribute(endpointId,
                           &emberZclClusterWindowCoveringServerSpec,
                           EMBER_ZCL_CLUSTER_WINDOW_COVERING_SERVER_ATTRIBUTE_CURRENT_TILT_PERCENTAGE,
                           (uint8_t *)&entry.currentPositionTiltPercentageValue,
                           sizeof(entry.currentPositionTiltPercentageValue));
  }
}

void emZclWindowCoveringServerStoreSceneHandler(EmberZclEndpointId_t endpointId,
                                                uint8_t tableIdx)
{
  EmZclWindowCoveringSceneSubTableEntry_t entry;

  entry.hasCurrentPositionLiftPercentageValue =
    (emberZclReadAttribute(endpointId,
                           &emberZclClusterWindowCoveringServerSpec,
                           EMBER_ZCL_CLUSTER_WINDOW_COVERING_SERVER_ATTRIBUTE_CURRENT_LIFT_PERCENTAGE,
                           (uint8_t *)&entry.currentPositionLiftPercentageValue,
                           sizeof(entry.currentPositionLiftPercentageValue)) == EMBER_ZCL_STATUS_SUCCESS);
  entry.hasCurrentPositionTiltPercentageValue =
    (emberZclReadAttribute(endpointId,
                           &emberZclClusterWindowCoveringServerSpec,
                           EMBER_ZCL_CLUSTER_WINDOW_COVERING_SERVER_ATTRIBUTE_CURRENT_TILT_PERCENTAGE,
                           (uint8_t *)&entry.currentPositionTiltPercentageValue,
                           sizeof(entry.currentPositionTiltPercentageValue)) == EMBER_ZCL_STATUS_SUCCESS);

  saveSceneSubTableEntry(entry, tableIdx);
}

void emZclWindowCoveringServerCopySceneHandler(uint8_t srcTableIdx,
                                               uint8_t dstTableIdx)
{
  EmZclWindowCoveringSceneSubTableEntry_t entry;
  retrieveSceneSubTableEntry(entry, srcTableIdx);

  saveSceneSubTableEntry(entry, dstTableIdx);
}

void emZclWindowCoveringServerViewSceneHandler(uint8_t tableIdx,
                                               uint8_t **ppExtFldData)
{
  EmZclWindowCoveringSceneSubTableEntry_t entry;
  retrieveSceneSubTableEntry(entry, tableIdx);

  if ((entry.hasCurrentPositionLiftPercentageValue)
      || (entry.hasCurrentPositionTiltPercentageValue)) {
    emberZclPluginScenesServerPutUint16InBuffer(ppExtFldData,
                                                EMBER_ZCL_CLUSTER_WINDOW_COVERING);

    uint8_t *pLength = *ppExtFldData;  // Save pointer to length byte.
    *pLength = 0;
    emberZclPluginScenesServerPutUint8InBuffer(ppExtFldData, *pLength); // Insert temporary length value.

    if (entry.hasCurrentPositionLiftPercentageValue) {
      emberZclPluginScenesServerPutUint8InBuffer(ppExtFldData,
                                                 entry.currentPositionLiftPercentageValue);
    }
    if (entry.hasCurrentPositionTiltPercentageValue) {
      emberZclPluginScenesServerPutUint8InBuffer(ppExtFldData,
                                                 entry.currentPositionTiltPercentageValue);
    }

    // Update length byte value.
    *pLength = *ppExtFldData - pLength - 1;
  }
}

void emZclWindowCoveringServerPrintInfoSceneHandler(uint8_t tableIdx)
{
  EmZclWindowCoveringSceneSubTableEntry_t entry;
  retrieveSceneSubTableEntry(entry, tableIdx);

  emberAfCorePrint(" window covering:%x %x",
                   entry.currentPositionLiftPercentageValue,
                   entry.currentPositionTiltPercentageValue);

  emberAfCoreFlush();
}
#endif
