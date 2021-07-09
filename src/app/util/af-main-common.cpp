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
 * @brief Code common to both the Host and SOC (system
 *on a chip) versions of the Application Framework.
 *******************************************************************************
 ******************************************************************************/

//#include PLATFORM_HEADER // Micro and compiler specific typedefs and macros

#if defined EZSP_HOST
#include "stack/include/ember-types.h"
#include "stack/include/error.h"
#include "stack/include/library.h"
#else
// Ember stack and related utilities
//#include "stack/include/cbke-crypto-engine.h"
//#include "stack/include/ember.h" // Main stack definitions
#endif

// HAL - hardware abstraction layer
//#include "hal/hal.h"
//#include "plugin/serial/serial.h" // Serial utility APIs

// CLI - command line interface
//#include "app/util/serial/command-interpreter2.h"

#if defined EZSP_HOST
// EZSP - ember serial host protocol
#include "app/util/ezsp/ezsp-protocol.h"
#include "app/util/ezsp/ezsp-utils.h"
#include "app/util/ezsp/ezsp.h"
#include "app/util/ezsp/serial-interface.h"
#endif

// Sub-GHz client, for a last-minute chance to block sending ZCL messgaes within the suspend period
#ifdef EMBER_AF_PLUGIN_SUB_GHZ_CLIENT
#include "app/framework/plugin/sub-ghz-client/sub-ghz-client.h"
#endif

// Sub-GHz server, for an automatic reply if a client attempt to communicate within the suspend period
#ifdef EMBER_AF_PLUGIN_SUB_GHZ_SERVER
#include "app/framework/plugin/sub-ghz-server/sub-ghz-server.h"
#endif

// Fragmentation.
#ifdef EMBER_AF_PLUGIN_FRAGMENTATION
#include "app/framework/plugin/fragmentation/fragmentation.h"
#endif

#ifdef EMBER_AF_PLUGIN_CRITICAL_MESSAGE_QUEUE
#include "app/framework/plugin/critical-message-queue/critical-message-queue.h"
#endif // EMBER_AF_PLUGIN_CRITICAL_MESSAGE_QUEUE

// Service discovery library
//#include "service-discovery.h"

// determines the number of in-clusters and out-clusters based on defines
// in config.h
#include <app/util/af-main.h>

//#include "app/framework/security/af-security.h"
//#include "app/framework/security/crypto-state.h"
#include "app/util/common.h"
#include <app/common/gen/callback.h>
#include <app/util/attribute-storage.h>
#include <app/util/attribute-table.h>
#include <app/util/config.h>
//#include "print.h"
#include <app/util/binding-table.h>
#include <app/util/chip-message-send.h>
#include <app/util/util.h>

using namespace chip;

// Querying the Ember Stack for what libraries are present.
//#include "app/util/common/library.h"

// ZDO - ZigBee Device Object
//#include "app/util/zigbee-framework/zigbee-device-common.h"

//#include "app/framework/plugin/ota-storage-common/ota-storage.h"
//#include "app/framework/plugin/partner-link-key-exchange/partner-link-key-exchange.h"

//------------------------------------------------------------------------------

#define INVALID_MESSAGE_TAG 0xFF

#if defined(EMBER_AF_HAS_COORDINATOR_NETWORK)
#if !defined(EMBER_AF_PLUGIN_CONCENTRATOR)
#error "A Coordinator device (Trust Center) MUST enable the concentrator plugin to function correctly."
#endif
#endif

#ifdef EMBER_AF_GENERATED_PLUGIN_STACK_STATUS_FUNCTION_DECLARATIONS
EMBER_AF_GENERATED_PLUGIN_STACK_STATUS_FUNCTION_DECLARATIONS
#endif

#ifdef EMBER_AF_GENERATED_PLUGIN_MESSAGE_SENT_FUNCTION_DECLARATIONS
EMBER_AF_GENERATED_PLUGIN_MESSAGE_SENT_FUNCTION_DECLARATIONS
#endif

#ifdef EMBER_AF_GENERATED_PLUGIN_ZDO_MESSAGE_RECEIVED_FUNCTION_DECLARATIONS
EMBER_AF_GENERATED_PLUGIN_ZDO_MESSAGE_RECEIVED_FUNCTION_DECLARATIONS
#endif

static CallbackTableEntry messageSentCallbacks[EMBER_AF_MESSAGE_SENT_CALLBACK_TABLE_SIZE];

// We declare this variable 'const' but NOT const.  Those functions that we may use
// this variable would also have to declare it const in order to function
// correctly, which is not the case (e.g. emberFindKeyTableEntry()).
const EmberEUI64 emberAfNullEui64 = { 0, 0, 0, 0, 0, 0, 0, 0 };

//------------------------------------------------------------------------------
// Forward declarations
static uint8_t getMessageSentCallbackIndex(void);
static void invalidateMessageSentCallbackEntry(uint8_t messageTag);
static EmberAfMessageSentFunction getMessageSentCallback(uint8_t tag);

static uint8_t getMessageSentCallbackIndex(void)
{
    uint8_t i;
    for (i = 0; i < EMBER_AF_MESSAGE_SENT_CALLBACK_TABLE_SIZE; i++)
    {
        if (messageSentCallbacks[i].tag == INVALID_MESSAGE_TAG)
        {
            return i;
        }
    }

    return INVALID_MESSAGE_TAG;
}

static void invalidateMessageSentCallbackEntry(uint8_t tag)
{
    uint8_t i;
    for (i = 0; i < EMBER_AF_MESSAGE_SENT_CALLBACK_TABLE_SIZE; i++)
    {
        if (messageSentCallbacks[i].tag == tag)
        {
            messageSentCallbacks[i].tag      = INVALID_MESSAGE_TAG;
            messageSentCallbacks[i].callback = NULL;
            return;
        }
    }
}

static EmberAfMessageSentFunction getMessageSentCallback(uint8_t tag)
{
    uint8_t i;
    for (i = 0; i < EMBER_AF_MESSAGE_SENT_CALLBACK_TABLE_SIZE; i++)
    {
        if (messageSentCallbacks[i].tag == tag)
        {
            return messageSentCallbacks[i].callback;
        }
    }

    return NULL;
}

void emAfInitializeMessageSentCallbackArray(void)
{
    uint8_t i;
    for (i = 0; i < EMBER_AF_MESSAGE_SENT_CALLBACK_TABLE_SIZE; i++)
    {
        messageSentCallbacks[i].tag      = INVALID_MESSAGE_TAG;
        messageSentCallbacks[i].callback = NULL;
    }
}

static EmberStatus send(const MessageSendDestination & destination, EmberApsFrame * apsFrame, uint16_t messageLength,
                        uint8_t * message, bool broadcast, EmberNodeId alias, uint8_t sequence, EmberAfMessageSentFunction callback)
{
    EmberStatus status;
    uint16_t index;
    uint8_t messageSentIndex;
    uint8_t messageTag = INVALID_MESSAGE_TAG;

    // The send APIs only deal with ZCL messages, so they must at least contain
    // the ZCL header.
    if (messageLength < EMBER_AF_ZCL_OVERHEAD)
    {
        return EMBER_ERR_FATAL;
    }
    else if ((message[0] & ZCL_MANUFACTURER_SPECIFIC_MASK) != 0U)
    {
        if (messageLength < EMBER_AF_ZCL_MANUFACTURER_SPECIFIC_OVERHEAD)
        {
            return EMBER_ERR_FATAL;
        }
    }

    messageSentIndex = getMessageSentCallbackIndex();
    if (callback != NULL && messageSentIndex == INVALID_MESSAGE_TAG)
    {
        return EMBER_TABLE_FULL;
    }

    // The source endpoint in the APS frame MUST be valid at this point.  We use
    // it to set the appropriate outgoing network in the APS frame.
    EmberAfEndpointInfoStruct endpointInfo;
    uint8_t networkIndex = 0;
    if (emberAfGetEndpointInfoCallback(apsFrame->sourceEndpoint, &networkIndex, &endpointInfo))
    {
        //        status              = emberAfPushNetworkIndex(networkIndex);
        //        if (status != EMBER_SUCCESS)
        //        {
        //            return status;
        //        }
    }
    else
    {
        index = emberAfIndexFromEndpoint(apsFrame->sourceEndpoint);
        if (index == 0xFFFF)
        {
            return EMBER_INVALID_ENDPOINT;
        }
        //        status = emberAfPushEndpointNetworkIndex(apsFrame->sourceEndpoint);
        //        if (status != EMBER_SUCCESS)
        //        {
        //            return status;
        //        }
    }

#ifdef EMBER_AF_PLUGIN_SUB_GHZ_CLIENT
    // If the Sub-GHz client is present and currently in the "suspended" state,
    // block any outgoing message unless it comes from the Sub-GHz client itself.
    if (emberAfPluginSubGhzClientIsSendingZclMessagesSuspended() && apsFrame->clusterId != ZCL_SUB_GHZ_CLUSTER_ID)
    {
        return EMBER_TRANSMISSION_SUSPENDED;
    }
#endif

    {
        EmberAfMessageStruct messageStruct = {
            callback, apsFrame, message, destination, messageLength, broadcast,
        };
        // Called prior to fragmentation in case the mesasge does not go out over the
        // Zigbee radio, and instead goes to some other transport that does not require
        // low level ZigBee fragmentation.
        if (emberAfPreMessageSendCallback(&messageStruct, &status))
        {
            return status;
        }
    }

    // SE 1.4 requires an option to disable APS ACK and Default Response
    emAfApplyDisableDefaultResponse(&message[0]);
    emAfApplyRetryOverride(&apsFrame->options);

    if (messageLength <= EMBER_AF_MAXIMUM_SEND_PAYLOAD_LENGTH)
    {
        status = emAfSend(destination, apsFrame, messageLength, message, &messageTag, alias, sequence);
    }
    else
    {
        status = EMBER_MESSAGE_TOO_LONG;
    }

#ifdef EMBER_AF_PLUGIN_CRITICAL_MESSAGE_QUEUE
    // If this was a critical message queue entry, fire the callback
    if ((status != EMBER_SUCCESS) &&
        (callback == emberAfPluginCriticalMessageQueueEnqueueCallback || callback == emAfPluginCriticalMessageQueueRetryCallback))
    {
        callback(destination, apsFrame, messageLength, message, status);
    }
#endif // EMBER_AF_PLUGIN_CRITICAL_MESSAGE_QUEUE

    if (callback != NULL && status == EMBER_SUCCESS && messageTag != INVALID_MESSAGE_TAG &&
        messageSentIndex < EMBER_AF_MESSAGE_SENT_CALLBACK_TABLE_SIZE)
    {
        messageSentCallbacks[messageSentIndex].tag      = messageTag;
        messageSentCallbacks[messageSentIndex].callback = callback;
    }

    if (status == EMBER_OPERATION_IN_PROGRESS && apsFrame->options & EMBER_APS_OPTION_DSA_SIGN)
    {
        // We consider "in progress" signed messages as being sent successfully.
        // The stack will send the message after signing.
        status = EMBER_SUCCESS;
        // TODO: Can we actually hit this case in CHIP, or can this whole block
        // go away?
        // emAfSetCryptoOperationInProgress();
    }

    if (status == EMBER_SUCCESS)
    {
        emberAfAddToCurrentAppTasks(EMBER_AF_WAITING_FOR_DATA_ACK | EMBER_AF_WAITING_FOR_ZCL_RESPONSE);
    }

    // emberAfPopNetworkIndex();
    return status;
}

EmberStatus emberAfSendMulticastWithAliasWithCallback(GroupId multicastId, EmberApsFrame * apsFrame, uint16_t messageLength,
                                                      uint8_t * message, EmberNodeId alias, uint8_t sequence,
                                                      EmberAfMessageSentFunction callback)
{
    apsFrame->groupId      = multicastId;
    const auto destination = MessageSendDestination::MulticastWithAlias(multicastId);
    return send(destination, apsFrame, messageLength, message,
                true, // broadcast
                alias, sequence, callback);
}

EmberStatus emberAfSendMulticastWithCallback(GroupId multicastId, EmberApsFrame * apsFrame, uint16_t messageLength,
                                             uint8_t * message, EmberAfMessageSentFunction callback)
{
    apsFrame->groupId      = multicastId;
    const auto destination = MessageSendDestination::Multicast(multicastId);
    return send(destination, apsFrame, messageLength, message,
                true, // broadcast?
                0,    // alias
                0,    // sequence
                callback);
}

EmberStatus emberAfSendMulticast(GroupId multicastId, EmberApsFrame * apsFrame, uint16_t messageLength, uint8_t * message)
{
    return emberAfSendMulticastWithCallback(multicastId, apsFrame, messageLength, message, NULL);
}

EmberStatus emberAfSendMulticastToBindings(EmberApsFrame * apsFrame, uint16_t messageLength, uint8_t * message)
{
    EmberStatus status = EMBER_INVALID_BINDING_INDEX;
    uint8_t i;
    EmberBindingTableEntry binding;
    GroupId groupDest;

    if ((NULL == apsFrame) || (0 == messageLength) || (NULL == message))
    {
        return EMBER_BAD_ARGUMENT;
    }

    for (i = 0; i < EMBER_BINDING_TABLE_SIZE; i++)
    {
        status = emberGetBinding(i, &binding);
        if (status != EMBER_SUCCESS)
        {
            return status;
        }

        if (binding.type == EMBER_MULTICAST_BINDING && binding.local == apsFrame->sourceEndpoint &&
            binding.clusterId == apsFrame->clusterId)
        {
            groupDest                     = binding.groupId;
            apsFrame->groupId             = groupDest;
            apsFrame->destinationEndpoint = binding.remote;

            status = emberAfSendMulticast(groupDest, // multicast ID
                                          apsFrame, messageLength, message);

            if (status != EMBER_SUCCESS)
            {
                return status;
            }
        }
    }

    return status;
}

// TODO: Figure out whether we need emberAfSendBroadcastWithCallback at all.
// EmberStatus emberAfSendBroadcastWithCallback(EmberNodeId destination, EmberApsFrame * apsFrame, uint16_t messageLength,
//                                             uint8_t * message, EmberAfMessageSentFunction callback)
//{
//    return send(EMBER_OUTGOING_BROADCAST, destination, apsFrame, messageLength, message,
//                true, // broadcast?
//                0,    // alias
//                0,    // sequence
//                callback);
//}

// TODO: Figure out whether we need emberAfSendBroadcastWithAliasWithCallback at all.
// EmberStatus emberAfSendBroadcastWithAliasWithCallback(EmberNodeId destination, EmberApsFrame * apsFrame, uint16_t messageLength,
//                                                      uint8_t * message, EmberNodeId alias, uint8_t sequence,
//                                                      EmberAfMessageSentFunction callback)
//{
//    return send(EMBER_OUTGOING_BROADCAST_WITH_ALIAS, destination, apsFrame, messageLength, message,
//                true,     // broadcast?
//                alias,    // alias
//                sequence, // sequence
//                callback);
//}

// TODO: Figure out whether we need emberAfSendBroadcast at all.
// EmberStatus emberAfSendBroadcast(EmberNodeId destination, EmberApsFrame * apsFrame, uint16_t messageLength, uint8_t * message)
//{
//    return emberAfSendBroadcastWithCallback(destination, apsFrame, messageLength, message, NULL);
//}

EmberStatus emberAfSendUnicastWithCallback(const MessageSendDestination & destination, EmberApsFrame * apsFrame,
                                           uint16_t messageLength, uint8_t * message, EmberAfMessageSentFunction callback)
{
    // The source endpoint in the APS frame MAY NOT be valid at this point if the
    // outgoing type is "via binding."
    if (destination.IsViaBinding())
    {
        // If using binding, set the endpoints based on those in the binding.  The
        // cluster in the binding is not used because bindings can be used to send
        // messages with any cluster id, not just the one set in the binding.
        EmberBindingTableEntry binding;
        // TODO: This cast should go away once
        // https://github.com/project-chip/connectedhomeip/issues/3584 is fixed.
        EmberStatus status = emberGetBinding(destination.GetBindingIndex(), &binding);
        if (status != EMBER_SUCCESS)
        {
            return status;
        }
        apsFrame->sourceEndpoint      = binding.local;
        apsFrame->destinationEndpoint = binding.remote;
    }
    return send(destination, apsFrame, messageLength, message,
                false, // broadcast?
                0,     // alias
                0,     // sequence
                callback);
}

EmberStatus emberAfSendUnicast(const MessageSendDestination & destination, EmberApsFrame * apsFrame, uint16_t messageLength,
                               uint8_t * message)
{
    return emberAfSendUnicastWithCallback(destination, apsFrame, messageLength, message, NULL);
}

EmberStatus emberAfSendUnicastToBindingsWithCallback(EmberApsFrame * apsFrame, uint16_t messageLength, uint8_t * message,
                                                     EmberAfMessageSentFunction callback)
{
    EmberStatus status = EMBER_INVALID_BINDING_INDEX;
    uint8_t i;

    for (i = 0; i < EMBER_BINDING_TABLE_SIZE; i++)
    {
        EmberBindingTableEntry binding;
        status = emberGetBinding(i, &binding);
        if (status != EMBER_SUCCESS)
        {
            return status;
        }
        if (binding.type == EMBER_UNICAST_BINDING && binding.local == apsFrame->sourceEndpoint &&
            binding.clusterId == apsFrame->clusterId)
        {
            apsFrame->destinationEndpoint            = binding.remote;
            const MessageSendDestination destination = MessageSendDestination::ViaBinding(i);
            status                                   = send(destination, apsFrame, messageLength, message,
                          false, // broadcast?
                          0,     // alias
                          0,     // sequence
                          callback);
            if (status != EMBER_SUCCESS)
            {
                return status;
            }
        }
    }

    return status;
}

EmberStatus emberAfSendUnicastToBindings(EmberApsFrame * apsFrame, uint16_t messageLength, uint8_t * message)
{
    return emberAfSendUnicastToBindingsWithCallback(apsFrame, messageLength, message, NULL);
}

EmberStatus emberAfSendInterPan(EmberPanId panId, const EmberEUI64 destinationLongId, EmberNodeId destinationShortId,
                                GroupId multicastId, ClusterId clusterId, uint16_t messageLength, uint8_t * messageBytes)
{
    EmberAfInterpanHeader header;
    memset(&header, 0, sizeof(EmberAfInterpanHeader));
    header.panId        = panId;
    header.shortAddress = destinationShortId;
    if (destinationLongId != NULL)
    {
        memmove(header.longAddress, destinationLongId, EUI64_SIZE);
        header.options |= EMBER_AF_INTERPAN_OPTION_MAC_HAS_LONG_ADDRESS;
        header.messageType = EMBER_AF_INTER_PAN_UNICAST;
    }
    else if (multicastId != 0)
    {
        header.groupId     = multicastId;
        header.messageType = EMBER_AF_INTER_PAN_MULTICAST;
    }
    else
    {
        header.messageType =
            (destinationShortId < EMBER_BROADCAST_ADDRESS ? EMBER_AF_INTER_PAN_UNICAST : EMBER_AF_INTER_PAN_BROADCAST);
    }
    header.clusterId = clusterId;
    return emberAfInterpanSendMessageCallback(&header, messageLength, messageBytes);
}

void emberAfPrintMessageData(uint8_t * data, uint16_t length)
{
#if defined EMBER_AF_PRINT_APP
    emberAfAppPrint(" payload (len %2x) [", length);
    emberAfAppPrintBuffer(data, length, true);
    emberAfAppPrintln("]");
#endif
}

void emAfPrintStatus(const char * task, EmberStatus status)
{
    if (status == EMBER_SUCCESS)
    {
        emberAfPrint(emberAfPrintActiveArea, "%p: %p", "Success", task);
    }
    else
    {
        emberAfPrint(emberAfPrintActiveArea, "%p: %p: 0x%x", "Error", task, status);
    }
}

// ******************************************************************
// Functions called by the Serial Command Line Interface (CLI)
// ******************************************************************

static void printMessage(EmberApsFrame * apsFrame, uint16_t messageLength, uint8_t * messageContents)
{
    emberAfAppPrint("Cluster: 0x%2X, %d bytes,", apsFrame->clusterId, messageLength);
    if (messageLength >= 3)
    {
        emberAfAppPrint(" ZCL %p Cmd ID: %d", (messageContents[0] & ZCL_CLUSTER_SPECIFIC_COMMAND ? "Cluster" : "Global"),
                        messageContents[2]);
    }
    emberAfAppPrintln("");
}

void emAfMessageSentHandler(const MessageSendDestination & destination, EmberApsFrame * apsFrame, EmberStatus status,
                            uint16_t messageLength, uint8_t * messageContents, uint8_t messageTag)
{
    EmberAfMessageSentFunction callback;
    if (status != EMBER_SUCCESS)
    {
        emberAfAppPrint("%ptx %x, ", "ERROR: ", status);
        printMessage(apsFrame, messageLength, messageContents);
    }

    callback = getMessageSentCallback(messageTag);
    invalidateMessageSentCallbackEntry(messageTag);

    emberAfRemoveFromCurrentAppTasks(EMBER_AF_WAITING_FOR_DATA_ACK);

    if (messageContents != NULL && messageContents[0] & ZCL_CLUSTER_SPECIFIC_COMMAND)
    {
        emberAfClusterMessageSentWithMfgCodeCallback(
            destination, apsFrame, messageLength, messageContents, status,
            // If the manufacturer specific flag is set
            // get read it as next part of message
            // else use null code.
            (((messageContents[0] & ZCL_MANUFACTURER_SPECIFIC_MASK) == ZCL_MANUFACTURER_SPECIFIC_MASK)
                 ? emberAfGetInt16u(messageContents, 1, messageLength)
                 : EMBER_AF_NULL_MANUFACTURER_CODE));
    }

    if (callback != NULL)
    {
        (*callback)(destination, apsFrame, messageLength, messageContents, status);
    }

#ifdef EMBER_AF_GENERATED_PLUGIN_MESSAGE_SENT_FUNCTION_CALLS
    EMBER_AF_GENERATED_PLUGIN_MESSAGE_SENT_FUNCTION_CALLS
#endif

    emberAfMessageSentCallback(destination, apsFrame, messageLength, messageContents, status);
}

#ifdef EMBER_AF_PLUGIN_FRAGMENTATION
void emAfFragmentationMessageSentHandler(const MessageSendDestination & destination, EmberApsFrame * apsFrame, uint8_t * buffer,
                                         uint16_t bufLen, EmberStatus status, uint8_t messageTag)
{
    // the fragmented message is no longer in process
    emberAfDebugPrintln("%pend.", "Fragmentation:");
    emAfMessageSentHandler(destination, apsFrame, status, bufLen, buffer, messageTag);

    // EMZIGBEE-4437: setting back the buffers to the original in case someone set
    // that to something else.
    emberAfSetExternalBuffer(appResponseData, EMBER_AF_RESPONSE_BUFFER_LEN, &appResponseLength, &emberAfResponseApsFrame);
}
#endif // EMBER_AF_PLUGIN_FRAGMENTATION

EmberStatus emAfSend(const MessageSendDestination & destination, EmberApsFrame * apsFrame, uint16_t messageLength,
                     uint8_t * message, uint8_t * messageTag, EmberNodeId alias, uint8_t sequence)
{
    // TODO: There's an impedance mismatch here in a few ways:
    // 1) The caller expects to get a messageTag out that will identify this
    // message somewhat uniquely.  Right now we just ignore that and claim an
    // invalid tag, which means message-sent callbacks don't get called.
    //
    // 2) The caller expects us to call emAfMessageSentHandler when we get an
    // ack or time out, and pass it the original contents of the message, so it
    // can invoke message-sent callbacks as needed.  But we may not have the
    // contents of the message if we've done in-place encryption since then.
    // Need to figure out whether any of this matters.
    //
    // https://github.com/project-chip/connectedhomeip/issues/2435 sort of
    // tracks this.
    *messageTag        = INVALID_MESSAGE_TAG;
    EmberStatus status = EMBER_SUCCESS;
    if (destination.IsViaBinding())
    {
        EmberBindingTableEntry binding;
        status = emberGetBinding(destination.GetBindingIndex(), &binding);
        if (status != EMBER_SUCCESS)
        {
            return status;
        }
        if (binding.type != EMBER_UNICAST_BINDING)
        {
            return EMBER_INVALID_BINDING_INDEX;
        }
        return chipSendUnicast(binding.nodeId, apsFrame, messageLength, message);
    }
    else if (destination.IsDirect())
    {
        return chipSendUnicast(destination.GetDirectNodeId(), apsFrame, messageLength, message);
    }
    else if (destination.IsViaExchange())
    {
        return chipSendUnicast(destination.GetExchangeContext(), apsFrame, messageLength, message);
    }
    else
    {
        // No implementation yet.
        return EMBER_ERR_FATAL;
    }
}
