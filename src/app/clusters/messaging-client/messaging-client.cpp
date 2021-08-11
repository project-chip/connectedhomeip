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
/***************************************************************************/
/**
 * @file
 * @brief Routines for the Messaging Client plugin,
 *which implements the client side of the Messaging
 *cluster.
 *******************************************************************************
 ******************************************************************************/

#include "messaging-client.h"
#include "../../include/af.h"
#include "../../util/common.h"

#include "app/framework/plugin/esi-management/esi-management.h"
#include <app/CommandHandler.h>

using namespace chip;

static EmberAfPluginMessagingClientMessage messageTable[EMBER_AF_MESSAGING_CLUSTER_CLIENT_ENDPOINT_COUNT];

#define MESSAGE_CONTROL_INTER_PAN_TRANSMISSION_ONLY (0x2)
/**
 * The callback function passed to the ESI management plugin. It handles
 * ESI entry deletions.
 */
static void esiDeletionCallback(uint8_t esiIndex)
{
    uint8_t i;
    for (i = 0; i < EMBER_AF_MESSAGING_CLUSTER_CLIENT_ENDPOINT_COUNT; i++)
    {
        messageTable[i].esiBitmask &= ~EMBER_BIT(esiIndex);
    }
}

void emberAfMessagingClusterClientInitCallback(EndpointId endpoint)
{
    emAfPluginMessagingClientClearMessage(endpoint);
    // Subscribing for ESI Management plugin deletion announcements.
    emberAfPluginEsiManagementSubscribeToDeletionAnnouncements(esiDeletionCallback);
}

void emAfPluginMessagingClientClearMessage(EndpointId endpoint)
{
    uint8_t ep = emberAfFindClusterClientEndpointIndex(endpoint, ZCL_MESSAGING_CLUSTER_ID);
    if (ep != 0xFF)
    {
        messageTable[ep].valid             = false;
        messageTable[ep].active            = false;
        messageTable[ep].messageId         = 0x00000000UL;
        messageTable[ep].messageControl    = 0x00;
        messageTable[ep].startTime         = 0x00000000UL;
        messageTable[ep].endTime           = 0x00000000UL;
        messageTable[ep].durationInMinutes = 0x0000;
        messageTable[ep].esiBitmask        = 0;
        messageTable[ep].message[0]        = 0;
    }
}

void emberAfMessagingClusterClientTickCallback(EndpointId endpoint)
{
    uint8_t ep = emberAfFindClusterClientEndpointIndex(endpoint, ZCL_MESSAGING_CLUSTER_ID);

    if (ep == 0xFF)
    {
        return;
    }

    // The tick is used to deactivate messages that are active or to activate
    // messages that are inactive.  So, first, just flip the active state and
    // notify the application of the change.
    messageTable[ep].active = !messageTable[ep].active;
    if (messageTable[ep].active)
    {
        emberAfPluginMessagingClientDisplayMessageCallback(&messageTable[ep]);
    }
    else
    {
        emberAfPluginMessagingClientCancelMessageCallback(&messageTable[ep]);
    }

    // If the new state is active and the duration is not "until changed," then we
    // have to schedule a tick to deactivate the message after the duration has
    // elapsed.  Otherwise, cancel the tick because no further processing is
    // required as either the message is no longer active or it will live until
    // changed.
    if (messageTable[ep].active && (messageTable[ep].durationInMinutes != ZCL_MESSAGING_CLUSTER_DURATION_UNTIL_CHANGED))
    {
        emberAfScheduleClientTick(endpoint, ZCL_MESSAGING_CLUSTER_ID,
                                  ((messageTable[ep].endTime - messageTable[ep].startTime) * MILLISECOND_TICKS_PER_SECOND));
    }
}

bool emberAfMessagingClusterDisplayMessageCallback(app::CommandHandler * commandObj, uint32_t messageId, uint8_t messageControl,
                                                   uint32_t startTime, uint16_t durationInMinutes, uint8_t * msg,
                                                   uint8_t optionalExtendedMessageControl)
{
    EndpointId endpoint = emberAfCurrentEndpoint();
    uint8_t ep          = emberAfFindClusterClientEndpointIndex(endpoint, ZCL_MESSAGING_CLUSTER_ID);
    uint32_t now        = emberAfGetCurrentTime();
    uint8_t esiIndex    = emberAfPluginEsiManagementUpdateEsiAndGetIndex(emberAfCurrentCommand());

    emberAfMessagingClusterPrint("RX: DisplayMessage"
                                 " 0x%4x, 0x%x, 0x%4x, 0x%2x, \"",
                                 messageId, messageControl, startTime, durationInMinutes);
    emberAfMessagingClusterPrintString(msg);
    emberAfMessagingClusterPrintln("\"");

    if (emberAfPluginMessagingClientPreDisplayMessageCallback(messageId, messageControl, startTime, durationInMinutes, msg,
                                                              optionalExtendedMessageControl))
    {
        goto kickout;
    }

    if (ep == 0xFF)
    {
        return false;
    }

    // Use of Inter-PAN transmission is deprecated.
    if ((messageControl & 0x03) == MESSAGE_CONTROL_INTER_PAN_TRANSMISSION_ONLY)
    {
        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_INVALID_FIELD);
        return true;
    }

    if (messageTable[ep].active)
    {
        // First check whether it is a duplicate message
        if (messageId == messageTable[ep].messageId)
        {
            // Duplicate message from a different ESI, add the ESI to the bitmask
            if (esiIndex < EMBER_AF_PLUGIN_ESI_MANAGEMENT_ESI_TABLE_SIZE &&
                (messageTable[ep].esiBitmask & EMBER_BIT(esiIndex)) == 0)
            {
                messageTable[ep].esiBitmask |= EMBER_BIT(esiIndex);
            }
            // Either way, we send back a default response.
            emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_SUCCESS);
            return true;
        }
        else
        {
            // If we already have another message, notify the application that it
            // should no longer display it.
            messageTable[ep].active = false;
            emberAfPluginMessagingClientCancelMessageCallback(&messageTable[ep]);
            emberAfDeactivateClientTick(emberAfCurrentEndpoint(), ZCL_MESSAGING_CLUSTER_ID);
        }
    }

    if (esiIndex < EMBER_AF_PLUGIN_ESI_MANAGEMENT_ESI_TABLE_SIZE)
    {
        messageTable[ep].esiBitmask = EMBER_BIT(esiIndex);
    }

    messageTable[ep].clientEndpoint = emberAfCurrentCommand()->apsFrame->destinationEndpoint;
    messageTable[ep].messageId      = messageId;
    messageTable[ep].messageControl = messageControl;
    messageTable[ep].startTime      = (startTime == ZCL_MESSAGING_CLUSTER_START_TIME_NOW ? now : startTime);
    messageTable[ep].endTime =
        (durationInMinutes == ZCL_MESSAGING_CLUSTER_DURATION_UNTIL_CHANGED ? ZCL_MESSAGING_CLUSTER_END_TIME_NEVER
                                                                           : messageTable[ep].startTime + durationInMinutes * 60);
    messageTable[ep].durationInMinutes = durationInMinutes;
    emberAfCopyString(messageTable[ep].message, msg, EMBER_AF_PLUGIN_MESSAGING_CLIENT_MESSAGE_SIZE);

    if (messageTable[ep].startTime <= now)
    {
        if (messageTable[ep].durationInMinutes == ZCL_MESSAGING_CLUSTER_DURATION_UNTIL_CHANGED)
        {
            // If a message starts in the past and the duration is "until changed," it
            // gets marked as active and no tick is scheduled becaues the message will
            // live until we receive a new message or a cancel command.
            messageTable[ep].active = true;
            emberAfPluginMessagingClientDisplayMessageCallback(&messageTable[ep]);
        }
        else if (messageTable[ep].endTime > now)
        {
            // If a message starts in the past and the duration has not elapsed, it
            // gets marked as active and a tick is scheduled for the time remaining in
            // the duration to wake us up and mark the message as inactive.
            messageTable[ep].active = true;
            emberAfPluginMessagingClientDisplayMessageCallback(&messageTable[ep]);
            emberAfScheduleClientTick(emberAfCurrentEndpoint(), ZCL_MESSAGING_CLUSTER_ID,
                                      ((messageTable[ep].endTime - now) * MILLISECOND_TICKS_PER_SECOND));
        }
        else
        {
            // Finally, if a message starts in the past and the duration has elapsed,
            // it gets marked as inactive and no tick is scheduled because no further
            // processing is needed.
            messageTable[ep].active = false;
        }
    }
    else
    {
        // If a message starts in the future, it gets marked as inactive and a tick
        // is scheduled to wake us up at startTime to mark the message as active.
        messageTable[ep].active = false;
        emberAfScheduleClientTick(emberAfCurrentEndpoint(), ZCL_MESSAGING_CLUSTER_ID,
                                  ((messageTable[ep].startTime - now) * MILLISECOND_TICKS_PER_SECOND));
    }

    // Once we receive a message, the data in the structure becomes valid and
    // remains valid forever.  When the message expires or is canceled, it gets
    // marked as inactive, but stays valid.  This allows the application to
    // display or confirm old messages.
    messageTable[ep].valid = true;

    emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_SUCCESS);
kickout:
    return true;
}

bool emberAfMessagingClusterCancelMessageCallback(app::CommandHandler * commandObj, uint32_t messageId, uint8_t messageControl)
{
    EndpointId endpoint = emberAfCurrentEndpoint();
    uint8_t ep          = emberAfFindClusterClientEndpointIndex(endpoint, ZCL_MESSAGING_CLUSTER_ID);
    EmberAfStatus status;

    if (ep == 0xFF)
    {
        return false;
    }

    emberAfMessagingClusterPrintln("RX: CancelMessage 0x%4x, 0x%x", messageId, messageControl);

    if (messageTable[ep].active && messageTable[ep].messageId == messageId)
    {
        messageTable[ep].active         = false;
        messageTable[ep].messageControl = messageControl;
        emberAfPluginMessagingClientCancelMessageCallback(&messageTable[ep]);
        emberAfDeactivateClientTick(emberAfCurrentEndpoint(), ZCL_MESSAGING_CLUSTER_ID);
        status = EMBER_ZCL_STATUS_SUCCESS;
    }
    else
    {
        status = EMBER_ZCL_STATUS_NOT_FOUND;
    }

    emberAfSendImmediateDefaultResponse(status);
    return true;
}

void emAfPluginMessagingClientPrintInfo(EndpointId endpoint)
{
    uint8_t ep = emberAfFindClusterClientEndpointIndex(endpoint, ZCL_MESSAGING_CLUSTER_ID);

    if (ep == 0xFF)
    {
        return;
    }

    emberAfMessagingClusterPrintln("= Client Message =");
    emberAfMessagingClusterFlush();

    emberAfMessagingClusterPrintln(" vld: %p", (messageTable[ep].valid ? "YES" : "NO"));
    emberAfMessagingClusterPrintln(" act: %p", (messageTable[ep].active ? "YES" : "NO"));
    emberAfMessagingClusterPrintln("  id: 0x%4x", messageTable[ep].messageId);
    emberAfMessagingClusterPrintln("  mc: 0x%x", messageTable[ep].messageControl);
    emberAfMessagingClusterPrintln("  st: 0x%4x", messageTable[ep].startTime);
    emberAfMessagingClusterPrintln("  et: 0x%4x", messageTable[ep].endTime);
    emberAfMessagingClusterPrintln("time: 0x%4x", emberAfGetCurrentTime());
    emberAfMessagingClusterPrintln(" dur: 0x%2x", messageTable[ep].durationInMinutes);
    emberAfMessagingClusterFlush();
    emberAfMessagingClusterPrint(" mes: \"");
    emberAfMessagingClusterPrintString(messageTable[ep].message);
    emberAfMessagingClusterPrintln("\"");
    emberAfMessagingClusterFlush();
}

EmberAfStatus emberAfPluginMessagingClientConfirmMessage(EndpointId endpoint)
{
    EmberAfStatus status = EMBER_ZCL_STATUS_NOT_FOUND;
    uint8_t ep           = emberAfFindClusterClientEndpointIndex(endpoint, ZCL_MESSAGING_CLUSTER_ID);
    uint8_t i;

    if (ep == 0xFF)
    {
        return EMBER_ZCL_STATUS_FAILURE;
    }

    if (messageTable[ep].valid && messageTable[ep].active)
    {
        for (i = 0; i < EMBER_AF_PLUGIN_ESI_MANAGEMENT_ESI_TABLE_SIZE; i++)
        {
            EmberAfPluginEsiManagementEsiEntry * esiEntry = emberAfPluginEsiManagementEsiLookUpByIndex(i);
            EmberNodeId nodeId;
            if ((messageTable[ep].esiBitmask & EMBER_BIT(i)) == 0 || esiEntry == NULL)
            {
                continue;
            }

            nodeId = emberLookupNodeIdByEui64(esiEntry->eui64);

            emberAfFillExternalBuffer((ZCL_CLUSTER_SPECIFIC_COMMAND | ZCL_FRAME_CONTROL_CLIENT_TO_SERVER), ZCL_MESSAGING_CLUSTER_ID,
                                      ZCL_MESSAGE_CONFIRMATION_COMMAND_ID, "wwus", messageTable[ep].messageId,
                                      emberAfGetCurrentTime(), 0x00, "");
            // The source and destination are reversed for the confirmation.
            emberAfSetCommandEndpoints(messageTable[ep].clientEndpoint, esiEntry->endpoint);
            const MessageSendDestination destination = MessageSendDestination::Direct(nodeId);
            status =
                ((emberAfSendCommandUnicast(destination) == EMBER_SUCCESS) ? EMBER_ZCL_STATUS_SUCCESS : EMBER_ZCL_STATUS_FAILURE);
        }
    }
    else
    {
        emberAfMessagingClusterPrintln("Err: No Valid Msg");
    }
    return status;
}
