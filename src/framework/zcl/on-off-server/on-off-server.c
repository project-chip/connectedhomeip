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
#ifdef CHIP_AF_API_ZCL_LEVEL_CONTROL_SERVER
  #include CHIP_AF_API_ZCL_LEVEL_CONTROL_SERVER
  #define SET_ON_OFF(endpointId, onOff) chipZclLevelControlServerSetOnOff(endpointId, onOff)
#else
static void setOnOff(ChipZclEndpointId_t endpointId, bool onOff);
  #define SET_ON_OFF(endpointId, onOff) setOnOff(endpointId, onOff)
#endif
#ifdef CHIP_AF_API_ZCL_SCENES_SERVER
  #include CHIP_AF_API_ZCL_SCENES_SERVER
#endif

#include "on-off-server.h"

// Globals

#ifdef CHIP_AF_API_ZCL_SCENES_SERVER
  #ifdef DEFINETOKENS
// Token based storage.
    #define retrieveSceneSubTableEntry(entry, i) \
  halCommonGetIndexedToken(&entry, TOKEN_ZCL_CORE_ON_OFF_SCENE_SUBTABLE, i)
    #define saveSceneSubTableEntry(entry, i) \
  halCommonSetIndexedToken(TOKEN_ZCL_CORE_ON_OFF_SCENE_SUBTABLE, i, &entry)
  #else
// RAM based storage.
ChZclOnOffSceneSubTableEntry_t chZclPluginOnOffServerSceneSubTable[CHIP_AF_PLUGIN_SCENES_SERVER_TABLE_SIZE] = { { 0 } };
    #define retrieveSceneSubTableEntry(entry, i) \
  (entry = chZclPluginOnOffServerSceneSubTable[i])
    #define saveSceneSubTableEntry(entry, i) \
  (chZclPluginOnOffServerSceneSubTable[i] = entry)
  #endif
#endif

static void setOnOffHandler(const ChipZclCommandContext_t *context, bool onOff);
static bool getOnOff(ChipZclEndpointId_t endpointId);

void chipZclClusterOnOffServerCommandOffRequestHandler(const ChipZclCommandContext_t *context,
                                                        const ChipZclClusterOnOffServerCommandOffRequest_t *request)
{
  chipAfCorePrintln("RX: Off");
  setOnOffHandler(context, false); // off
}

void chipZclClusterOnOffServerCommandOnRequestHandler(const ChipZclCommandContext_t *context,
                                                       const ChipZclClusterOnOffServerCommandOnRequest_t *request)
{
  chipAfCorePrintln("RX: On");
  setOnOffHandler(context, true); // on
}

void chipZclClusterOnOffServerCommandToggleRequestHandler(const ChipZclCommandContext_t *context,
                                                           const ChipZclClusterOnOffServerCommandToggleRequest_t *request)
{
  chipAfCorePrintln("RX: Toggle");
  setOnOffHandler(context, !getOnOff(context->endpointId));
}

static bool getOnOff(ChipZclEndpointId_t endpointId)
{
  bool onOff;
  if (chipZclReadAttribute(endpointId,
                            &chipZclClusterOnOffServerSpec,
                            CHIP_ZCL_CLUSTER_ON_OFF_SERVER_ATTRIBUTE_ON_OFF,
                            &onOff,
                            sizeof(onOff))
      == CHIP_ZCL_STATUS_SUCCESS) {
    return onOff;
  } else {
    return false;
  }
}

#ifndef CHIP_AF_API_ZCL_LEVEL_CONTROL_SERVER
static void setOnOff(ChipZclEndpointId_t endpointId, bool onOff)
{
  chipZclWriteAttribute(endpointId,
                         &chipZclClusterOnOffServerSpec,
                         CHIP_ZCL_CLUSTER_ON_OFF_SERVER_ATTRIBUTE_ON_OFF,
                         &onOff,
                         sizeof(onOff));
}
#endif

static void setOnOffHandler(const ChipZclCommandContext_t *context, bool onOff)
{
  SET_ON_OFF(context->endpointId, onOff);
  chipZclSendDefaultResponse(context, CHIP_ZCL_STATUS_SUCCESS);
}

#ifdef CHIP_AF_API_ZCL_SCENES_SERVER
// Scenes callback handlers...

void chZclOnOffServerEraseSceneHandler(uint8_t tableIdx)
{
  ChZclOnOffSceneSubTableEntry_t entry;

  entry.hasOnOffValue = false;

  saveSceneSubTableEntry(entry, tableIdx);
}

bool chZclOnOffServerAddSceneHandler(ChipZclClusterId_t clusterId,
                                     uint8_t tableIdx,
                                     const uint8_t *sceneData,
                                     uint8_t length)
{
  if (clusterId == CHIP_ZCL_CLUSTER_ON_OFF) {
    if (length < 1) {
      return false; // ext field format error (OnOffValue byte must be present).
    }

    // Extract bytes from input data block and update scene subtable fields.
    ChZclOnOffSceneSubTableEntry_t entry = { 0 };
    uint8_t *pData = (uint8_t *)sceneData;

    // We only know of one extension for the On/Off cluster and it is just one byte,
    // which means we can skip some logic for this cluster.
    // If other extensions are added in this cluster, more logic will be needed here.
    entry.hasOnOffValue = true;
    entry.onOffValue = chipZclPluginScenesServerGetUint8FromBuffer(&pData);

    saveSceneSubTableEntry(entry, tableIdx);

    return true;
  }

  return false;
}

void chZclOnOffServerRecallSceneHandler(ChipZclEndpointId_t endpointId,
                                        uint8_t tableIdx,
                                        uint32_t transitionTime100mS)
{
  // Handles the recallScene command for the onOff cluster.
  // Note- this handler presently just updates (writes) the relevant cluster
  // attribute(s), in a production system this could be replaced by a call
  // to the relevant onOff command handler to actually change the hw state.

  ChZclOnOffSceneSubTableEntry_t entry;
  retrieveSceneSubTableEntry(entry, tableIdx);

  if (entry.hasOnOffValue) {
    chipZclWriteAttribute(endpointId,
                           &chipZclClusterOnOffServerSpec,
                           CHIP_ZCL_CLUSTER_ON_OFF_SERVER_ATTRIBUTE_ON_OFF,
                           (uint8_t *)&entry.onOffValue,
                           sizeof(entry.onOffValue));
  }
}

void chZclOnOffServerStoreSceneHandler(ChipZclEndpointId_t endpointId,
                                       uint8_t tableIdx)
{
  ChZclOnOffSceneSubTableEntry_t entry;

  entry.hasOnOffValue = (chipZclReadAttribute(endpointId,
                                               &chipZclClusterOnOffServerSpec,
                                               CHIP_ZCL_CLUSTER_ON_OFF_SERVER_ATTRIBUTE_ON_OFF,
                                               (uint8_t *)&entry.onOffValue,
                                               sizeof(entry.onOffValue))
                         == CHIP_ZCL_STATUS_SUCCESS);

  saveSceneSubTableEntry(entry, tableIdx);
}

void chZclOnOffServerCopySceneHandler(uint8_t srcTableIdx, uint8_t dstTableIdx)
{
  ChZclOnOffSceneSubTableEntry_t entry;
  retrieveSceneSubTableEntry(entry, srcTableIdx);

  saveSceneSubTableEntry(entry, dstTableIdx);
}

void chZclOnOffServerViewSceneHandler(uint8_t tableIdx, uint8_t **ppExtFldData)
{
  ChZclOnOffSceneSubTableEntry_t entry;
  retrieveSceneSubTableEntry(entry, tableIdx);

  if (entry.hasOnOffValue) {
    chipZclPluginScenesServerPutUint16InBuffer(ppExtFldData,
                                                CHIP_ZCL_CLUSTER_ON_OFF);
    chipZclPluginScenesServerPutUint8InBuffer(ppExtFldData, 1);  // length=1
    chipZclPluginScenesServerPutUint8InBuffer(ppExtFldData, entry.onOffValue);
  }
}

void chZclOnOffServerPrintInfoSceneHandler(uint8_t tableIdx)
{
  ChZclOnOffSceneSubTableEntry_t entry;
  retrieveSceneSubTableEntry(entry, tableIdx);

  chipAfCorePrint(" on/off:%x", entry.onOffValue);
}
#endif
