
/**
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2018 Silicon Laboratories Inc. www.silabs.com
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

#include "on-off-server.h"
#include "chip-zcl.h"
#include "gen.h"

// Globals

static void setOnOffHandler(const ChipZclCommandContext_t * context, bool onOff);
static bool getOnOff(ChipZclEndpointId_t endpointId);

void chipZclClusterOnOffServerCommandOffRequestHandler(const ChipZclCommandContext_t * context,
                                                       const ChipZclClusterOnOffServerCommandOffRequest_t * request)
{
    chipZclCorePrintln("RX: Off");
    setOnOffHandler(context, false); // off
}

void chipZclClusterOnOffServerCommandOnRequestHandler(const ChipZclCommandContext_t * context,
                                                      const ChipZclClusterOnOffServerCommandOnRequest_t * request)
{
    chipZclCorePrintln("RX: On");
    setOnOffHandler(context, true); // on
}

void chipZclClusterOnOffServerCommandToggleRequestHandler(const ChipZclCommandContext_t * context,
                                                          const ChipZclClusterOnOffServerCommandToggleRequest_t * request)
{
    chipZclCorePrintln("RX: Toggle");
    setOnOffHandler(context, !getOnOff(context->endpointId));
}

static bool getOnOff(ChipZclEndpointId_t endpointId)
{
    bool onOff;
    if (chipZclReadAttribute(endpointId, &chipZclClusterOnOffServerSpec, CHIP_ZCL_CLUSTER_ON_OFF_SERVER_ATTRIBUTE_ON_OFF, &onOff,
                             sizeof(onOff)) == CHIP_ZCL_STATUS_SUCCESS)
    {
        return onOff;
    }
    else
    {
        return false;
    }
}

static void setOnOff(ChipZclEndpointId_t endpointId, bool onOff)
{
    chipZclWriteAttribute(endpointId, &chipZclClusterOnOffServerSpec, CHIP_ZCL_CLUSTER_ON_OFF_SERVER_ATTRIBUTE_ON_OFF, &onOff,
                          sizeof(onOff));
}

static void setOnOffHandler(const ChipZclCommandContext_t * context, bool onOff)
{
    setOnOff(context->endpointId, onOff);
    chipZclSendDefaultResponse(context, CHIP_ZCL_STATUS_SUCCESS);
}

#ifdef CHIP_AF_API_ZCL_SCENES_SERVER
// Scenes callback handlers...

void zapOnOffServerEraseSceneHandler(uint8_t tableIdx)
{
    ChipZclOnOffSceneSubTableEntry_t entry;

    entry.hasOnOffValue = false;

    saveSceneSubTableEntry(entry, tableIdx);
}

bool zapOnOffServerAddSceneHandler(ChipZclClusterId_t clusterId, uint8_t tableIdx, const uint8_t * sceneData, uint8_t length)
{
    if (clusterId == CHIP_ZCL_CLUSTER_ON_OFF)
    {
        if (length < 1)
        {
            return false; // ext field format error (OnOffValue byte must be present).
        }

        // Extract bytes from input data block and update scene subtable fields.
        ChipZclOnOffSceneSubTableEntry_t entry = { 0 };
        uint8_t * pData                        = (uint8_t *) sceneData;

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

void zapOnOffServerRecallSceneHandler(ChipZclEndpointId_t endpointId, uint8_t tableIdx, uint32_t transitionTime100mS)
{
    // Handles the recallScene command for the onOff cluster.
    // Note- this handler presently just updates (writes) the relevant cluster
    // attribute(s), in a production system this could be replaced by a call
    // to the relevant onOff command handler to actually change the hw state.

    ChipZclOnOffSceneSubTableEntry_t entry;
    retrieveSceneSubTableEntry(entry, tableIdx);

    if (entry.hasOnOffValue)
    {
        chipZclWriteAttribute(endpointId, &chipZclClusterOnOffServerSpec, CHIP_ZCL_CLUSTER_ON_OFF_SERVER_ATTRIBUTE_ON_OFF,
                              (uint8_t *) &entry.onOffValue, sizeof(entry.onOffValue));
    }
}

void zapOnOffServerStoreSceneHandler(ChipZclEndpointId_t endpointId, uint8_t tableIdx)
{
    ChipZclOnOffSceneSubTableEntry_t entry;

    entry.hasOnOffValue =
        (chipZclReadAttribute(endpointId, &chipZclClusterOnOffServerSpec, CHIP_ZCL_CLUSTER_ON_OFF_SERVER_ATTRIBUTE_ON_OFF,
                              (uint8_t *) &entry.onOffValue, sizeof(entry.onOffValue)) == CHIP_ZCL_STATUS_SUCCESS);

    saveSceneSubTableEntry(entry, tableIdx);
}

void zapOnOffServerCopySceneHandler(uint8_t srcTableIdx, uint8_t dstTableIdx)
{
    ChipZclOnOffSceneSubTableEntry_t entry;
    retrieveSceneSubTableEntry(entry, srcTableIdx);

    saveSceneSubTableEntry(entry, dstTableIdx);
}

void zapOnOffServerViewSceneHandler(uint8_t tableIdx, uint8_t ** ppExtFldData)
{
    ChipZclOnOffSceneSubTableEntry_t entry;
    retrieveSceneSubTableEntry(entry, tableIdx);

    if (entry.hasOnOffValue)
    {
        zapPluginScenesServerPutUint16InBuffer(ppExtFldData, CHIP_ZCL_CLUSTER_ON_OFF);
        zapPluginScenesServerPutUint8InBuffer(ppExtFldData, 1); // length=1
        zapPluginScenesServerPutUint8InBuffer(ppExtFldData, entry.onOffValue);
    }
}

void zapOnOffServerPrintInfoSceneHandler(uint8_t tableIdx)
{
    ChipZclOnOffSceneSubTableEntry_t entry;
    retrieveSceneSubTableEntry(entry, tableIdx);

    zapCorePrint(" on/off:%x", entry.onOffValue);
}
#endif
