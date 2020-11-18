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
 * @brief APIs and defines for the Messaging Server
 *plugin.
 *******************************************************************************
 ******************************************************************************/

#pragma once

// ----------------------------------------------------------------------------
// Message Control byte
// ----------------------------------------------------------------------------

#define ZCL_MESSAGING_CLUSTER_TRANSMISSION_MASK (BIT(1) | BIT(0))
#define ZCL_MESSAGING_CLUSTER_IMPORTANCE_MASK (BIT(3) | BIT(2))
#define ZCL_MESSAGING_CLUSTER_RESERVED_MASK (BIT(6) | BIT(5) | BIT(4))
#define ZCL_MESSAGING_CLUSTER_CONFIRMATION_MASK BIT(7)

#define ZCL_MESSAGING_CLUSTER_START_TIME_NOW 0x00000000UL
#define ZCL_MESSAGING_CLUSTER_END_TIME_NEVER 0xFFFFFFFFUL
#define ZCL_MESSAGING_CLUSTER_DURATION_UNTIL_CHANGED 0xFFFF

/**
 * @brief The message and metadata used by the messaging server plugin.
 *
 * The application can get and set the message used by the plugin by calling
 * ::emberAfPluginMessagingServerGetMessage and
 * ::emberAfPluginMessagingServerSetMessage.
 */
typedef struct
{
    /** The unique unsigned 32-bit number identifier for this message. */
    uint32_t messageId;
    /** The control bitmask for this message. */
    uint8_t messageControl;
    /** The time at which the message becomes valid. */
    uint32_t startTime;
    /** The amount of time in minutes after the start time during which the
        message is displayed.
     */
    uint16_t durationInMinutes;
    /** The string containing the message to be delivered. */
    uint8_t message[EMBER_AF_PLUGIN_MESSAGING_SERVER_MESSAGE_SIZE + 1];
    /** Additional control and status information for a given message. **/
    uint8_t extendedMessageControl;
    /** Internal control state */
    uint8_t messageStatusControl;
} EmberAfPluginMessagingServerMessage;

/**
 * @brief Gets the message used by the messaging server plugin.
 *
 * This function is used to get the message and metadata that the plugin
 * will send to clients. For "start now" messages that are current or
 * scheduled, the duration is adjusted to reflect how many minutes remain for
 * the message. Otherwise, the start time and duration of "start now" messages
 * reflect the actual start and the original duration.
 *
 * @param endpoint The relevant endpoint.
 * @param message The ::EmberAfPluginMessagingServerMessage structure
 * describing the message.
 * @return True if the message is valid or false is the message does not exist
 * or is expired.
 */
bool emberAfPluginMessagingServerGetMessage(chip::EndpointId endpoint, EmberAfPluginMessagingServerMessage * message);

/**
 * @brief Sets the message used by the Messaging server plugin.
 *
 * This function can be used to set the message and metadata that the plugin
 * will send to clients. Setting the start time to zero instructs clients to
 * start the message now. For "start now" messages, the plugin will
 * automatically adjust the duration reported to clients based on the original
 * start time of the message.
 *
 * @param endpoint The relevant endpoint.
 * @param message The ::EmberAfPluginMessagingServerMessage structure
 * describing the message.  If NULL, the message is removed from the server.
 */
void emberAfPluginMessagingServerSetMessage(chip::EndpointId endpoint, const EmberAfPluginMessagingServerMessage * message);

void emAfPluginMessagingServerPrintInfo(chip::EndpointId endpoint);
void emberAfPluginMessagingServerDisplayMessage(EmberNodeId nodeId, uint8_t srcEndpoint, uint8_t dstEndpoint);
void emberAfPluginMessagingServerCancelMessage(EmberNodeId nodeId, uint8_t srcEndpoint, uint8_t dstEndpoint);
