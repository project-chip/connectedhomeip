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
/***************************************************************************
 * @file
 * @brief Routines for the Messaging Server plugin,
 *which implements the server side of the Messaging
 *cluster.
 *******************************************************************************
 ******************************************************************************/

#include "messaging-server.h"
#include "../../include/af.h"
#include <app/CommandHandler.h>

using namespace chip;

// The internal message is stored in the same structure type that is defined
// publicly.  The internal state of the message is stored in the
// messageStatusControl field
static EmberAfPluginMessagingServerMessage msgTable[EMBER_AF_MESSAGING_CLUSTER_SERVER_ENDPOINT_COUNT];

// These bits are used by the messageStatusControl to indicate whether or not
// a message is valid, active, or if it is a "send now" message
#define VALID EMBER_BIT(0)
#define ACTIVE EMBER_BIT(1)
#define NOW EMBER_BIT(2)

#define messageIsValid(ep) (msgTable[ep].messageStatusControl & VALID)
#define messageIsActive(ep) (msgTable[ep].messageStatusControl & ACTIVE)
#define messageIsNow(ep) (msgTable[ep].messageStatusControl & NOW)
#define messageIsForever(ep) (msgTable[ep].durationInMinutes == ZCL_MESSAGING_CLUSTER_DURATION_UNTIL_CHANGED)
static bool messageIsCurrentOrScheduled(EndpointId endpoint)
{
    uint8_t ep = emberAfFindClusterServerEndpointIndex(endpoint, ZCL_MESSAGING_CLUSTER_ID);

    if (ep == 0xFF)
    {
        return false;
    }

    return (messageIsValid(ep) && messageIsActive(ep) &&
            (messageIsForever(ep) ||
             (emberAfGetCurrentTime() < msgTable[ep].startTime + (uint32_t) msgTable[ep].durationInMinutes * 60)));
}

void emberAfMessagingClusterServerInitCallback(EndpointId endpoint)
{
    uint8_t ep = emberAfFindClusterServerEndpointIndex(endpoint, ZCL_MESSAGING_CLUSTER_ID);

    if (ep == 0xFF)
    {
        return;
    }

    msgTable[ep].messageStatusControl &= ~VALID;
}

bool emberAfMessagingClusterGetLastMessageCallback(chip::app::CommandHandler * commandObj)
{
    EndpointId endpoint = emberAfCurrentEndpoint();
    EmberAfPluginMessagingServerMessage message;
    emberAfMessagingClusterPrintln("RX: GetLastMessage");
    if (emberAfPluginMessagingServerGetMessage(endpoint, &message))
    {
        emberAfFillExternalBuffer((ZCL_CLUSTER_SPECIFIC_COMMAND | ZCL_FRAME_CONTROL_SERVER_TO_CLIENT), ZCL_MESSAGING_CLUSTER_ID,
                                  ZCL_DISPLAY_MESSAGE_COMMAND_ID, "wuwvsu", message.messageId, message.messageControl,
                                  message.startTime, message.durationInMinutes, message.message, message.extendedMessageControl);
        emberAfGetCommandApsFrame()->options |= EMBER_APS_OPTION_SOURCE_EUI64;
        emberAfSendResponse();
    }
    else
    {
        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_NOT_FOUND);
    }
    return true;
}

#if defined(EMBER_AF_HAS_SPEC_VERSIONS_SE_1_0) || defined(EMBER_AF_HAS_SPEC_VERSIONS_SE_1_1B) ||                                   \
    defined(EMBER_AF_HAS_SPEC_VERSIONS_SE_1_0) || defined(EMBER_AF_HAS_SPEC_VERSIONS_SE_1_1) ||                                    \
    defined(EMBER_AF_HAS_SPEC_VERSIONS_SE_1_1A)
bool emberAfMessagingClusterMessageConfirmationCallback(chip::app::CommandHandler * commandObj, uint32_t messageId,
                                                        uint32_t confirmationTime)
#else
bool emberAfMessagingClusterMessageConfirmationCallback(chip::app::CommandHandler * commandObj, uint32_t messageId,
                                                        uint32_t confirmationTime, uint8_t messageConfirmationControl,
                                                        uint8_t * messageResponse)
#endif
{
    emberAfMessagingClusterPrintln("RX: MessageConfirmation 0x%4x, 0x%4x", messageId, confirmationTime);
    emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_SUCCESS);
    return true;
}

bool emberAfPluginMessagingServerGetMessage(EndpointId endpoint, EmberAfPluginMessagingServerMessage * message)
{
    uint8_t ep = emberAfFindClusterServerEndpointIndex(endpoint, ZCL_MESSAGING_CLUSTER_ID);

    if (ep == 0xFF)
    {
        return false;
    }

    MEMMOVE(message, &msgTable[ep], sizeof(EmberAfPluginMessagingServerMessage));

    // Clear out our internal bits from the message control.
    message->messageStatusControl &= ~ZCL_MESSAGING_CLUSTER_RESERVED_MASK;

    // If the message is expired or it has an absolute time, set the start time
    // and duration to the original start time and duration.  For "start now"
    // messages that are current or scheduled, set the start time to the special
    // value for "now" and set the duration to the remaining time, if it is not
    // already the special value for "until changed."
    if (messageIsCurrentOrScheduled(endpoint) && messageIsNow(ep))
    {
        message->startTime = ZCL_MESSAGING_CLUSTER_START_TIME_NOW;
        if (!messageIsForever(ep))
        {
            message->durationInMinutes -= ((emberAfGetCurrentTime() - msgTable[ep].startTime) / 60);
        }
    }
    return messageIsCurrentOrScheduled(endpoint);
}

void emberAfPluginMessagingServerSetMessage(EndpointId endpoint, const EmberAfPluginMessagingServerMessage * message)
{
    uint8_t ep = emberAfFindClusterServerEndpointIndex(endpoint, ZCL_MESSAGING_CLUSTER_ID);

    if (ep == 0xFF)
    {
        return;
    }

    if (message == NULL)
    {
        msgTable[ep].messageStatusControl &= ~ACTIVE;
        return;
    }

    MEMMOVE(&msgTable[ep], message, sizeof(EmberAfPluginMessagingServerMessage));

    // Rember if this is a "start now" message, but store the start time as the
    // current time so the duration can be adjusted.
    if (msgTable[ep].startTime == ZCL_MESSAGING_CLUSTER_START_TIME_NOW)
    {
        msgTable[ep].messageStatusControl |= NOW;
        msgTable[ep].startTime = emberAfGetCurrentTime();
    }
    else
    {
        msgTable[ep].messageStatusControl &= ~NOW;
    }

    msgTable[ep].messageStatusControl |= (VALID | ACTIVE);
}

void emAfPluginMessagingServerPrintInfo(EndpointId endpoint)
{
    uint8_t ep = emberAfFindClusterServerEndpointIndex(endpoint, ZCL_MESSAGING_CLUSTER_ID);

    if (ep == 0xFF)
    {
        return;
    }

    emberAfMessagingClusterPrintln("= Server Message =");
    emberAfMessagingClusterFlush();

    emberAfMessagingClusterPrintln(" vld: %s", (messageIsValid(ep) ? "YES" : "NO"));
    emberAfMessagingClusterPrintln(" act: %s", (messageIsCurrentOrScheduled(endpoint) ? "YES" : "NO"));
    emberAfMessagingClusterPrintln("  id: 0x%4x", msgTable[ep].messageId);
    emberAfMessagingClusterPrintln("  mc: 0x%x", (msgTable[ep].messageControl & ~ZCL_MESSAGING_CLUSTER_RESERVED_MASK));
    emberAfMessagingClusterPrintln("  st: 0x%4x", msgTable[ep].startTime);
    emberAfMessagingClusterPrintln(" now: %s", (messageIsNow(ep) ? "YES" : "NO"));
    emberAfMessagingClusterPrintln("time: 0x%4x", emberAfGetCurrentTime());
    emberAfMessagingClusterPrintln(" dur: 0x%2x", msgTable[ep].durationInMinutes);
    emberAfMessagingClusterFlush();
    emberAfMessagingClusterPrint(" mes: \"");
    emberAfMessagingClusterPrintString(msgTable[ep].message);
    emberAfMessagingClusterPrintln("\"");
    emberAfMessagingClusterFlush();
}

void emberAfPluginMessagingServerDisplayMessage(EmberNodeId nodeId, uint8_t srcEndpoint, uint8_t dstEndpoint)
{
    EmberStatus status;
    EmberAfPluginMessagingServerMessage message;
    if (!emberAfPluginMessagingServerGetMessage(srcEndpoint, &message))
    {
        emberAfMessagingClusterPrintln("invalid msg");
        return;
    }

    emberAfFillExternalBuffer((ZCL_CLUSTER_SPECIFIC_COMMAND | ZCL_FRAME_CONTROL_SERVER_TO_CLIENT), ZCL_MESSAGING_CLUSTER_ID,
                              ZCL_DISPLAY_MESSAGE_COMMAND_ID, "wuwvsu", message.messageId, message.messageControl,
                              message.startTime, message.durationInMinutes, message.message, message.extendedMessageControl);
    emberAfSetCommandEndpoints(srcEndpoint, dstEndpoint);
    emberAfGetCommandApsFrame()->options |= EMBER_APS_OPTION_SOURCE_EUI64;
    status = emberAfSendCommandUnicast(MessageSendDestination::Direct(nodeId));
    if (status != EMBER_SUCCESS)
    {
        emberAfMessagingClusterPrintln("Error in display %x", status);
    }
}

void emberAfPluginMessagingServerCancelMessage(EmberNodeId nodeId, uint8_t srcEndpoint, uint8_t dstEndpoint)
{
    EmberStatus status;
    EmberAfPluginMessagingServerMessage message;

    // Nullify the current message before sending the cancellation.
    emberAfPluginMessagingServerSetMessage(srcEndpoint, NULL);

    // Then send the response
    emberAfPluginMessagingServerGetMessage(srcEndpoint, &message);

    emberAfFillExternalBuffer((ZCL_CLUSTER_SPECIFIC_COMMAND | ZCL_FRAME_CONTROL_SERVER_TO_CLIENT), ZCL_MESSAGING_CLUSTER_ID,
                              ZCL_CANCEL_MESSAGE_COMMAND_ID, "wu", message.messageId, message.messageControl);
    emberAfSetCommandEndpoints(srcEndpoint, dstEndpoint);
    emberAfGetCommandApsFrame()->options |= EMBER_APS_OPTION_SOURCE_EUI64;
    status = emberAfSendCommandUnicast(MessageSendDestination::Direct(nodeId));
    if (status != EMBER_SUCCESS)
    {
        emberAfMessagingClusterPrintln("Error in cancel %x", status);
    }
}
