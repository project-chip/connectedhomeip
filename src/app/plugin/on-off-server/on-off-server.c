
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
 *    @file
 *      This file provides an implementation of the CHIP ZCL Application
 *      Layer's On Off Cluster Server
 */

#ifdef ZAP_TEST
#include "utest.h"
#endif

#include PLATFORM_HEADER
#include CONFIGURATION_HEADER
#include ZAP_AF_API_STACK
#ifdef ZAP_AF_API_DEBUG_PRINT
#include ZAP_AF_API_DEBUG_PRINT
#endif
#include ZAP_AF_API_ZCL_CORE
#ifdef ZAP_AF_API_ZCL_LEVEL_CONTROL_SERVER
#include ZAP_AF_API_ZCL_LEVEL_CONTROL_SERVER
#define SET_ON_OFF(endpointId, onOff) zapLevelControlServerSetOnOff(endpointId, onOff)
#else
static void setOnOff(ZapEndpointId_t endpointId, bool onOff);
#define SET_ON_OFF(endpointId, onOff) setOnOff(endpointId, onOff)
#endif
#ifdef ZAP_AF_API_ZCL_SCENES_SERVER
#include ZAP_AF_API_ZCL_SCENES_SERVER
#endif

#include "on-off-server.h"

// Globals

#ifdef ZAP_AF_API_ZCL_SCENES_SERVER
#ifdef DEFINETOKENS
// Token based storage.
#define retrieveSceneSubTableEntry(entry, i) halCommonGetIndexedToken(&entry, TOKEN_ZCL_CORE_ON_OFF_SCENE_SUBTABLE, i)
#define saveSceneSubTableEntry(entry, i) halCommonSetIndexedToken(TOKEN_ZCL_CORE_ON_OFF_SCENE_SUBTABLE, i, &entry)
#else
// RAM based storage.
ZapOnOffSceneSubTableEntry_t zapPluginOnOffServerSceneSubTable[ZAP_AF_PLUGIN_SCENES_SERVER_TABLE_SIZE] = { { 0 } };
#define retrieveSceneSubTableEntry(entry, i) (entry = zapPluginOnOffServerSceneSubTable[i])
#define saveSceneSubTableEntry(entry, i) (zapPluginOnOffServerSceneSubTable[i] = entry)
#endif
#endif

static void setOnOffHandler(const ZapCommandContext_t * context, bool onOff);
static bool getOnOff(ZapEndpointId_t endpointId);

void zapClusterOnOffServerCommandOffRequestHandler(const ZapCommandContext_t * context,
                                                   const ZapClusterOnOffServerCommandOffRequest_t * request)
{
    zapCorePrintln("RX: Off");
    setOnOffHandler(context, false); // off
}

void zapClusterOnOffServerCommandOnRequestHandler(const ZapCommandContext_t * context,
                                                  const ZapClusterOnOffServerCommandOnRequest_t * request)
{
    zapCorePrintln("RX: On");
    setOnOffHandler(context, true); // on
}

void zapClusterOnOffServerCommandToggleRequestHandler(const ZapCommandContext_t * context,
                                                      const ZapClusterOnOffServerCommandToggleRequest_t * request)
{
    zapCorePrintln("RX: Toggle");
    setOnOffHandler(context, !getOnOff(context->endpointId));
}

static bool getOnOff(ZapEndpointId_t endpointId)
{
    bool onOff;
    if (zapReadAttribute(endpointId, &zapClusterOnOffServerSpec, ZAP_CLUSTER_ON_OFF_SERVER_ATTRIBUTE_ON_OFF, &onOff,
                         sizeof(onOff)) == ZAP_STATUS_SUCCESS)
    {
        return onOff;
    }
    else
    {
        return false;
    }
}

#ifndef ZAP_AF_API_ZCL_LEVEL_CONTROL_SERVER
static void setOnOff(ZapEndpointId_t endpointId, bool onOff)
{
    zapWriteAttribute(endpointId, &zapClusterOnOffServerSpec, ZAP_CLUSTER_ON_OFF_SERVER_ATTRIBUTE_ON_OFF, &onOff, sizeof(onOff));
}
#endif

static void setOnOffHandler(const ZapCommandContext_t * context, bool onOff)
{
    SET_ON_OFF(context->endpointId, onOff);
    zapSendDefaultResponse(context, ZAP_STATUS_SUCCESS);
}

#ifdef ZAP_AF_API_ZCL_SCENES_SERVER
// Scenes callback handlers...

void zapOnOffServerEraseSceneHandler(uint8_t tableIdx)
{
    ZapOnOffSceneSubTableEntry_t entry;

    entry.hasOnOffValue = false;

    saveSceneSubTableEntry(entry, tableIdx);
}

bool zapOnOffServerAddSceneHandler(ZapClusterId_t clusterId, uint8_t tableIdx, const uint8_t * sceneData, uint8_t length)
{
    if (clusterId == ZAP_CLUSTER_ON_OFF)
    {
        if (length < 1)
        {
            return false; // ext field format error (OnOffValue byte must be present).
        }

        // Extract bytes from input data block and update scene subtable fields.
        ZapOnOffSceneSubTableEntry_t entry = { 0 };
        uint8_t * pData                    = (uint8_t *) sceneData;

        // We only know of one extension for the On/Off cluster and it is just one byte,
        // which means we can skip some logic for this cluster.
        // If other extensions are added in this cluster, more logic will be needed here.
        entry.hasOnOffValue = true;
        entry.onOffValue    = zapPluginScenesServerGetUint8FromBuffer(&pData);

        saveSceneSubTableEntry(entry, tableIdx);

        return true;
    }

    return false;
}

void zapOnOffServerRecallSceneHandler(ZapEndpointId_t endpointId, uint8_t tableIdx, uint32_t transitionTime100mS)
{
    // Handles the recallScene command for the onOff cluster.
    // Note- this handler presently just updates (writes) the relevant cluster
    // attribute(s), in a production system this could be replaced by a call
    // to the relevant onOff command handler to actually change the hw state.

    ZapOnOffSceneSubTableEntry_t entry;
    retrieveSceneSubTableEntry(entry, tableIdx);

    if (entry.hasOnOffValue)
    {
        zapWriteAttribute(endpointId, &zapClusterOnOffServerSpec, ZAP_CLUSTER_ON_OFF_SERVER_ATTRIBUTE_ON_OFF,
                          (uint8_t *) &entry.onOffValue, sizeof(entry.onOffValue));
    }
}

void zapOnOffServerStoreSceneHandler(ZapEndpointId_t endpointId, uint8_t tableIdx)
{
    ZapOnOffSceneSubTableEntry_t entry;

    entry.hasOnOffValue = (zapReadAttribute(endpointId, &zapClusterOnOffServerSpec, ZAP_CLUSTER_ON_OFF_SERVER_ATTRIBUTE_ON_OFF,
                                            (uint8_t *) &entry.onOffValue, sizeof(entry.onOffValue)) == ZAP_STATUS_SUCCESS);

    saveSceneSubTableEntry(entry, tableIdx);
}

void zapOnOffServerCopySceneHandler(uint8_t srcTableIdx, uint8_t dstTableIdx)
{
    ZapOnOffSceneSubTableEntry_t entry;
    retrieveSceneSubTableEntry(entry, srcTableIdx);

    saveSceneSubTableEntry(entry, dstTableIdx);
}

void zapOnOffServerViewSceneHandler(uint8_t tableIdx, uint8_t ** ppExtFldData)
{
    ZapOnOffSceneSubTableEntry_t entry;
    retrieveSceneSubTableEntry(entry, tableIdx);

    if (entry.hasOnOffValue)
    {
        zapPluginScenesServerPutUint16InBuffer(ppExtFldData, ZAP_CLUSTER_ON_OFF);
        zapPluginScenesServerPutUint8InBuffer(ppExtFldData, 1); // length=1
        zapPluginScenesServerPutUint8InBuffer(ppExtFldData, entry.onOffValue);
    }
}

void zapOnOffServerPrintInfoSceneHandler(uint8_t tableIdx)
{
    ZapOnOffSceneSubTableEntry_t entry;
    retrieveSceneSubTableEntry(entry, tableIdx);

    zapCorePrint(" on/off:%x", entry.onOffValue);
}
#endif
