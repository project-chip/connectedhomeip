/***************************************************************************//**
 * @file
 * @brief
 ******************************************************************************/

#include PLATFORM_HEADER
#include CONFIGURATION_HEADER
#include EMBER_AF_API_STACK
#ifdef EMBER_AF_API_DEBUG_PRINT
  #include EMBER_AF_API_DEBUG_PRINT
#endif
#include EMBER_AF_API_ZCL_CORE
#ifdef EMBER_AF_API_ZCL_LEVEL_CONTROL_SERVER
  #include EMBER_AF_API_ZCL_LEVEL_CONTROL_SERVER
  #define SET_ON_OFF(endpointId, onOff) emberZclLevelControlServerSetOnOff(endpointId, onOff)
#else
static void setOnOff(EmberZclEndpointId_t endpointId, bool onOff);
  #define SET_ON_OFF(endpointId, onOff) setOnOff(endpointId, onOff)
#endif
#ifdef EMBER_AF_API_ZCL_SCENES_SERVER
  #include EMBER_AF_API_ZCL_SCENES_SERVER
#endif

#include "on-off-server.h"

// Globals

#ifdef EMBER_AF_API_ZCL_SCENES_SERVER
  #ifdef DEFINETOKENS
// Token based storage.
    #define retrieveSceneSubTableEntry(entry, i) \
  halCommonGetIndexedToken(&entry, TOKEN_ZCL_CORE_ON_OFF_SCENE_SUBTABLE, i)
    #define saveSceneSubTableEntry(entry, i) \
  halCommonSetIndexedToken(TOKEN_ZCL_CORE_ON_OFF_SCENE_SUBTABLE, i, &entry)
  #else
// RAM based storage.
EmZclOnOffSceneSubTableEntry_t emZclPluginOnOffServerSceneSubTable[EMBER_AF_PLUGIN_SCENES_SERVER_TABLE_SIZE] = { { 0 } };
    #define retrieveSceneSubTableEntry(entry, i) \
  (entry = emZclPluginOnOffServerSceneSubTable[i])
    #define saveSceneSubTableEntry(entry, i) \
  (emZclPluginOnOffServerSceneSubTable[i] = entry)
  #endif
#endif

static void setOnOffHandler(const EmberZclCommandContext_t *context, bool onOff);
static bool getOnOff(EmberZclEndpointId_t endpointId);

void emberZclClusterOnOffServerCommandOffRequestHandler(const EmberZclCommandContext_t *context,
                                                        const EmberZclClusterOnOffServerCommandOffRequest_t *request)
{
  emberAfCorePrintln("RX: Off");
  setOnOffHandler(context, false); // off
}

void emberZclClusterOnOffServerCommandOnRequestHandler(const EmberZclCommandContext_t *context,
                                                       const EmberZclClusterOnOffServerCommandOnRequest_t *request)
{
  emberAfCorePrintln("RX: On");
  setOnOffHandler(context, true); // on
}

void emberZclClusterOnOffServerCommandToggleRequestHandler(const EmberZclCommandContext_t *context,
                                                           const EmberZclClusterOnOffServerCommandToggleRequest_t *request)
{
  emberAfCorePrintln("RX: Toggle");
  setOnOffHandler(context, !getOnOff(context->endpointId));
}

static bool getOnOff(EmberZclEndpointId_t endpointId)
{
  bool onOff;
  if (emberZclReadAttribute(endpointId,
                            &emberZclClusterOnOffServerSpec,
                            EMBER_ZCL_CLUSTER_ON_OFF_SERVER_ATTRIBUTE_ON_OFF,
                            &onOff,
                            sizeof(onOff))
      == EMBER_ZCL_STATUS_SUCCESS) {
    return onOff;
  } else {
    return false;
  }
}

#ifndef EMBER_AF_API_ZCL_LEVEL_CONTROL_SERVER
static void setOnOff(EmberZclEndpointId_t endpointId, bool onOff)
{
  emberZclWriteAttribute(endpointId,
                         &emberZclClusterOnOffServerSpec,
                         EMBER_ZCL_CLUSTER_ON_OFF_SERVER_ATTRIBUTE_ON_OFF,
                         &onOff,
                         sizeof(onOff));
}
#endif

static void setOnOffHandler(const EmberZclCommandContext_t *context, bool onOff)
{
  SET_ON_OFF(context->endpointId, onOff);
  emberZclSendDefaultResponse(context, EMBER_ZCL_STATUS_SUCCESS);
}

#ifdef EMBER_AF_API_ZCL_SCENES_SERVER
// Scenes callback handlers...

void emZclOnOffServerEraseSceneHandler(uint8_t tableIdx)
{
  EmZclOnOffSceneSubTableEntry_t entry;

  entry.hasOnOffValue = false;

  saveSceneSubTableEntry(entry, tableIdx);
}

bool emZclOnOffServerAddSceneHandler(EmberZclClusterId_t clusterId,
                                     uint8_t tableIdx,
                                     const uint8_t *sceneData,
                                     uint8_t length)
{
  if (clusterId == EMBER_ZCL_CLUSTER_ON_OFF) {
    if (length < 1) {
      return false; // ext field format error (OnOffValue byte must be present).
    }

    // Extract bytes from input data block and update scene subtable fields.
    EmZclOnOffSceneSubTableEntry_t entry = { 0 };
    uint8_t *pData = (uint8_t *)sceneData;

    // We only know of one extension for the On/Off cluster and it is just one byte,
    // which means we can skip some logic for this cluster.
    // If other extensions are added in this cluster, more logic will be needed here.
    entry.hasOnOffValue = true;
    entry.onOffValue = emberZclPluginScenesServerGetUint8FromBuffer(&pData);

    saveSceneSubTableEntry(entry, tableIdx);

    return true;
  }

  return false;
}

void emZclOnOffServerRecallSceneHandler(EmberZclEndpointId_t endpointId,
                                        uint8_t tableIdx,
                                        uint32_t transitionTime100mS)
{
  // Handles the recallScene command for the onOff cluster.
  // Note- this handler presently just updates (writes) the relevant cluster
  // attribute(s), in a production system this could be replaced by a call
  // to the relevant onOff command handler to actually change the hw state.

  EmZclOnOffSceneSubTableEntry_t entry;
  retrieveSceneSubTableEntry(entry, tableIdx);

  if (entry.hasOnOffValue) {
    emberZclWriteAttribute(endpointId,
                           &emberZclClusterOnOffServerSpec,
                           EMBER_ZCL_CLUSTER_ON_OFF_SERVER_ATTRIBUTE_ON_OFF,
                           (uint8_t *)&entry.onOffValue,
                           sizeof(entry.onOffValue));
  }
}

void emZclOnOffServerStoreSceneHandler(EmberZclEndpointId_t endpointId,
                                       uint8_t tableIdx)
{
  EmZclOnOffSceneSubTableEntry_t entry;

  entry.hasOnOffValue = (emberZclReadAttribute(endpointId,
                                               &emberZclClusterOnOffServerSpec,
                                               EMBER_ZCL_CLUSTER_ON_OFF_SERVER_ATTRIBUTE_ON_OFF,
                                               (uint8_t *)&entry.onOffValue,
                                               sizeof(entry.onOffValue))
                         == EMBER_ZCL_STATUS_SUCCESS);

  saveSceneSubTableEntry(entry, tableIdx);
}

void emZclOnOffServerCopySceneHandler(uint8_t srcTableIdx, uint8_t dstTableIdx)
{
  EmZclOnOffSceneSubTableEntry_t entry;
  retrieveSceneSubTableEntry(entry, srcTableIdx);

  saveSceneSubTableEntry(entry, dstTableIdx);
}

void emZclOnOffServerViewSceneHandler(uint8_t tableIdx, uint8_t **ppExtFldData)
{
  EmZclOnOffSceneSubTableEntry_t entry;
  retrieveSceneSubTableEntry(entry, tableIdx);

  if (entry.hasOnOffValue) {
    emberZclPluginScenesServerPutUint16InBuffer(ppExtFldData,
                                                EMBER_ZCL_CLUSTER_ON_OFF);
    emberZclPluginScenesServerPutUint8InBuffer(ppExtFldData, 1);  // length=1
    emberZclPluginScenesServerPutUint8InBuffer(ppExtFldData, entry.onOffValue);
  }
}

void emZclOnOffServerPrintInfoSceneHandler(uint8_t tableIdx)
{
  EmZclOnOffSceneSubTableEntry_t entry;
  retrieveSceneSubTableEntry(entry, tableIdx);

  emberAfCorePrint(" on/off:%x", entry.onOffValue);
}
#endif
