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
#include "identify.h"

// this file contains all the common includes for clusters in the util
#include <app/Command.h>
#include <app/common/gen/attribute-id.h>
#include <app/common/gen/attribute-type.h>
#include <app/common/gen/cluster-id.h>
#include <app/common/gen/command-id.h>
#include <app/util/af.h>
#include <app/util/common.h>
#include <support/CodeUtils.h>

using namespace chip;

typedef struct
{
    bool identifying;
    uint16_t identifyTime;
} EmAfIdentifyState;

static EmAfIdentifyState stateTable[EMBER_AF_IDENTIFY_CLUSTER_SERVER_ENDPOINT_COUNT];

static EmberAfStatus readIdentifyTime(EndpointId endpoint, uint16_t * identifyTime);
static EmberAfStatus writeIdentifyTime(EndpointId endpoint, uint16_t identifyTime);
static EmberStatus scheduleIdentifyTick(EndpointId endpoint);

static EmAfIdentifyState * getIdentifyState(EndpointId endpoint);

static EmAfIdentifyState * getIdentifyState(EndpointId endpoint)
{
    uint8_t ep = emberAfFindClusterServerEndpointIndex(endpoint, ZCL_IDENTIFY_CLUSTER_ID);
    return (ep == 0xFF ? NULL : &stateTable[ep]);
}

void emberAfIdentifyClusterServerInitCallback(EndpointId endpoint)
{
    scheduleIdentifyTick(endpoint);
}

void emberAfIdentifyClusterServerTickCallback(EndpointId endpoint)
{
    uint16_t identifyTime;
    if (readIdentifyTime(endpoint, &identifyTime) == EMBER_ZCL_STATUS_SUCCESS)
    {
        // This tick writes the new attribute, which will trigger the Attribute
        // Changed callback below, which in turn will schedule or cancel the tick.
        // Because of this, the tick does not have to be scheduled here.
        writeIdentifyTime(endpoint, static_cast<uint16_t>(identifyTime == 0 ? 0 : identifyTime - 1));
    }
}

void emberAfIdentifyClusterServerAttributeChangedCallback(EndpointId endpoint, AttributeId attributeId)
{
    if (attributeId == ZCL_IDENTIFY_TIME_ATTRIBUTE_ID)
    {
        scheduleIdentifyTick(endpoint);
    }
}

bool emberAfIdentifyClusterIdentifyCallback(chip::app::Command * commandObj, uint16_t time)
{
    EmberStatus sendStatus = EMBER_SUCCESS;
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

bool emberAfIdentifyClusterIdentifyQueryCallback(chip::app::Command * commandObj)
{
    EmberAfStatus status;
    EmberStatus sendStatus = EMBER_SUCCESS;
    uint16_t identifyTime;
    CHIP_ERROR err = CHIP_NO_ERROR;

    emberAfIdentifyClusterPrintln("RX identify:QUERY");

    // According to the 075123r02ZB, a device shall not send an Identify Query
    // Response if it is not currently identifying.  Instead, or if reading the
    // Identify Time attribute fails, send a Default Response.
    status = readIdentifyTime(emberAfCurrentEndpoint(), &identifyTime);
    if (status != EMBER_ZCL_STATUS_SUCCESS || identifyTime == 0)
    {
        if (status != EMBER_ZCL_STATUS_SUCCESS)
        {
            emberAfIdentifyClusterPrintln("Error reading identify time");
        }
        else
        {
            emberAfIdentifyClusterPrintln("identifyTime is at 0");
        }
        emberAfIdentifyClusterPrintln("Sending back default response");
        sendStatus = emberAfSendImmediateDefaultResponse(status);
        if (EMBER_SUCCESS != sendStatus)
        {
            emberAfIdentifyClusterPrintln("Identify: failed to send %s response: "
                                          "0x%x",
                                          "default", sendStatus);
        }
        return true;
    }

    emberAfIdentifyClusterPrintln("Identifying for %d more seconds", identifyTime);
    if (commandObj == nullptr)
    {
        emberAfFillExternalBuffer((ZCL_CLUSTER_SPECIFIC_COMMAND | ZCL_FRAME_CONTROL_SERVER_TO_CLIENT), ZCL_IDENTIFY_CLUSTER_ID,
                                  ZCL_IDENTIFY_QUERY_RESPONSE_COMMAND_ID, "v", identifyTime);
        sendStatus = emberAfSendResponse();
    }
    else
    {
        app::CommandPathParams cmdParams = { emberAfCurrentEndpoint(), /* group id */ 0, ZCL_IDENTIFY_CLUSTER_ID,
                                             ZCL_IDENTIFY_QUERY_RESPONSE_COMMAND_ID,
                                             (chip::app::CommandPathFlags::kEndpointIdValid) };
        TLV::TLVWriter * writer          = nullptr;

        VerifyOrExit(commandObj != nullptr, err = CHIP_ERROR_INCORRECT_STATE);

        SuccessOrExit(err = commandObj->PrepareCommand(cmdParams));
        VerifyOrExit((writer = commandObj->GetCommandDataElementTLVWriter()) != nullptr, err = CHIP_ERROR_INCORRECT_STATE);
        SuccessOrExit(err = writer->Put(TLV::ContextTag(0), identifyTime));
        SuccessOrExit(err = commandObj->FinishCommand());
    }

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "Failed to encode response command.");
    }
    if (EMBER_SUCCESS != sendStatus)
    {
        emberAfIdentifyClusterPrintln("Identify: failed to send %s response: 0x%x", "query", sendStatus);
    }
    return true;
}

EmberAfStatus readIdentifyTime(EndpointId endpoint, uint16_t * identifyTime)
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

static EmberAfStatus writeIdentifyTime(EndpointId endpoint, uint16_t identifyTime)
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

static EmberStatus scheduleIdentifyTick(EndpointId endpoint)
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

bool emberAfPluginIdentifyStartFeedbackCallback(EndpointId endpoint, uint16_t identifyTime)
{
    emberAfPrintln(EMBER_AF_PRINT_IDENTIFY_CLUSTER, "Start identify callback on endpoint %d time %d", endpoint, identifyTime);
    return false;
}

bool emberAfPluginIdentifyStopFeedbackCallback(EndpointId endpoint)
{
    emberAfPrintln(EMBER_AF_PRINT_IDENTIFY_CLUSTER, "Stop identify callback on endpoint %d", endpoint);
    return false;
}
