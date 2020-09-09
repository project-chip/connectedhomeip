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
 * @brief Routines for the Identify plugin, which
 *implements the Identify cluster.
 *******************************************************************************
 ******************************************************************************/

// *******************************************************************
// * identify.c
// *
// *
// * Copyright 2007 by Ember Corporation. All rights reserved.              *80*
// *******************************************************************

// this file contains all the common includes for clusters in the util
#include <app/util/af.h>

#include "common.h"

typedef struct
{
    bool identifying;
    uint16_t identifyTime;
} EmAfIdentifyState;

static EmAfIdentifyState stateTable[EMBER_AF_IDENTIFY_CLUSTER_SERVER_ENDPOINT_COUNT];

static EmberAfStatus readIdentifyTime(uint8_t endpoint, uint16_t * identifyTime);
static EmberAfStatus writeIdentifyTime(uint8_t endpoint, uint16_t identifyTime);
static EmberStatus scheduleIdentifyTick(uint8_t endpoint);

static EmAfIdentifyState * getIdentifyState(uint8_t endpoint);

static EmAfIdentifyState * getIdentifyState(uint8_t endpoint)
{
    uint8_t ep = emberAfFindClusterServerEndpointIndex(endpoint, ZCL_IDENTIFY_CLUSTER_ID);
    return (ep == 0xFF ? NULL : &stateTable[ep]);
}

void emberAfIdentifyClusterServerInitCallback(uint8_t endpoint)
{
    scheduleIdentifyTick(endpoint);
}

void emberAfIdentifyClusterServerTickCallback(uint8_t endpoint)
{
    uint16_t identifyTime;
    if (readIdentifyTime(endpoint, &identifyTime) == EMBER_ZCL_STATUS_SUCCESS)
    {
        // This tick writes the new attribute, which will trigger the Attribute
        // Changed callback below, which in turn will schedule or cancel the tick.
        // Because of this, the tick does not have to be scheduled here.
        writeIdentifyTime(endpoint, (identifyTime == 0 ? 0 : identifyTime - 1));
    }
}

void emberAfIdentifyClusterServerAttributeChangedCallback(uint8_t endpoint, EmberAfAttributeId attributeId)
{
    if (attributeId == ZCL_IDENTIFY_TIME_ATTRIBUTE_ID)
    {
        scheduleIdentifyTick(endpoint);
    }
}

bool emberAfIdentifyClusterIdentifyCallback(uint16_t time)
{
    EmberStatus sendStatus;
    // This Identify callback writes the new attribute, which will trigger the
    // Attribute Changed callback above, which in turn will schedule or cancel the
    // tick.  Because of this, the tick does not have to be scheduled here.
    emberAfIdentifyClusterPrintln("RX identify:IDENTIFY 0x%2x", time);
    sendStatus = emberAfSendImmediateDefaultResponse(writeIdentifyTime(emberAfCurrentEndpoint(), time));
    if (EMBER_SUCCESS != sendStatus)
    {
        emberAfIdentifyClusterPrintln("Identify: failed to send %s response: "
                                      "0x%x",
                                      "default", sendStatus);
    }
    return true;
}

bool emberAfIdentifyClusterIdentifyQueryCallback(void)
{
    EmberAfStatus status;
    EmberStatus sendStatus;
    uint16_t identifyTime;

    emberAfIdentifyClusterPrintln("RX identify:QUERY");

    // According to the 075123r02ZB, a device shall not send an Identify Query
    // Response if it is not currently identifying.  Instead, or if reading the
    // Identify Time attribute fails, send a Default Response.
    status = readIdentifyTime(emberAfCurrentEndpoint(), &identifyTime);
    if (status != EMBER_ZCL_STATUS_SUCCESS || identifyTime == 0)
    {
        sendStatus = emberAfSendImmediateDefaultResponse(status);
        if (EMBER_SUCCESS != sendStatus)
        {
            emberAfIdentifyClusterPrintln("Identify: failed to send %s response: "
                                          "0x%x",
                                          "default", sendStatus);
        }
        return true;
    }

    emberAfFillCommandIdentifyClusterIdentifyQueryResponse(identifyTime);
    sendStatus = emberAfSendResponse();
    if (EMBER_SUCCESS != sendStatus)
    {
        emberAfIdentifyClusterPrintln("Identify: failed to send %s response: 0x%x", "query", sendStatus);
    }
    return true;
}

EmberAfStatus readIdentifyTime(uint8_t endpoint, uint16_t * identifyTime)
{
    EmberAfStatus status = emberAfReadAttribute(endpoint, ZCL_IDENTIFY_CLUSTER_ID, ZCL_IDENTIFY_TIME_ATTRIBUTE_ID,
                                                CLUSTER_MASK_SERVER, (uint8_t *) identifyTime, sizeof(*identifyTime),
                                                NULL); // data type
#if defined(EMBER_AF_PRINT_ENABLE) && defined(EMBER_AF_PRINT_IDENTIFY_CLUSTER)
    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        emberAfIdentifyClusterPrintln("ERR: reading identify time %x", status);
    }
#endif // defined(EMBER_AF_PRINT_ENABLE) && defined(EMBER_AF_PRINT_IDENTIFY_CLUSTER)
    return status;
}

static EmberAfStatus writeIdentifyTime(uint8_t endpoint, uint16_t identifyTime)
{
    EmberAfStatus status = emberAfWriteAttribute(endpoint, ZCL_IDENTIFY_CLUSTER_ID, ZCL_IDENTIFY_TIME_ATTRIBUTE_ID,
                                                 CLUSTER_MASK_SERVER, (uint8_t *) &identifyTime, ZCL_INT16U_ATTRIBUTE_TYPE);
#if defined(EMBER_AF_PRINT_ENABLE) && defined(EMBER_AF_PRINT_IDENTIFY_CLUSTER)
    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        emberAfIdentifyClusterPrintln("ERR: writing identify time %x", status);
    }
#endif // defined(EMBER_AF_PRINT_ENABLE) && defined(EMBER_AF_PRINT_IDENTIFY_CLUSTER)
    return status;
}

static EmberStatus scheduleIdentifyTick(uint8_t endpoint)
{
    EmberAfStatus status;
    EmAfIdentifyState * state = getIdentifyState(endpoint);
    uint16_t identifyTime;

    if (state == NULL)
    {
        return EMBER_BAD_ARGUMENT;
    }

    status = readIdentifyTime(endpoint, &identifyTime);
    if (status == EMBER_ZCL_STATUS_SUCCESS)
    {
        if (!state->identifying)
        {
            state->identifying  = true;
            state->identifyTime = identifyTime;
            emberAfPluginIdentifyStartFeedbackCallback(endpoint, identifyTime);
        }
        if (identifyTime > 0)
        {
            return emberAfScheduleServerTick(endpoint, ZCL_IDENTIFY_CLUSTER_ID, MILLISECOND_TICKS_PER_SECOND);
        }
    }

    state->identifying = false;
    emberAfPluginIdentifyStopFeedbackCallback(endpoint);
    return emberAfDeactivateServerTick(endpoint, ZCL_IDENTIFY_CLUSTER_ID);
}
