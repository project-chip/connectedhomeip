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
/***************************************************************************//**
 * @file
 * @brief Definitions for the Sub-Ghz Server plugin.
 *******************************************************************************
   ******************************************************************************/

#ifndef SUB_GHZ_SERVER_H_INCLUDED
#define SUB_GHZ_SERVER_H_INCLUDED

/** @brief Sends the 'Suspend ZCL Messages' command.
 *
 * This is a server command. The server sends it to temporarily suspend ZCL messages
 * from clients it identifies as causing too much traffic.
 * It is also sent in response to the 'Get Suspend ZCL Messages Status' command.
 *
 * @param nodeId  The client's node ID.
 * @param endpoint The client's endpoint (ignored, backwards compatibility only).
 * @param period The suspension period in minutes.
 *
 * Note:
 * If endpoint == 0, the framework will choose the suitable endpoint based on
 * the past communication with the given node. If GetSuspendZclMessagesStatus has
 * never been received by the server, the server will not know which enpoint
 * implements the Sub-GHz Client and will default to endpoint 1. The application
 * can "teach" the server the client endpoint by discovering the Sub-GHz clients
 * and call this function for each of them with period == 0.
 */
EmberStatus emberAfPluginSubGhzServerSendSuspendZclMessagesCommand(EmberNodeId nodeId,
                                                                   uint8_t endpoint,
                                                                   uint8_t period);

/** @brief Gets the current suspend status for a given client.
 *
 * @param nodeId The client's node ID.
 *
 * @return  The number of seconds remaining; 0 = not suspended.
 */
uint16_t emberAfPluginSubGhzServerSuspendZclMessagesStatus(EmberNodeId nodeId);

//------------------------------------------------------------------------------
// Application Framework Internal Functions
//
// The functions below are non-public internal function used by the application
// framework. They are NOT to be used by the application.

/** @brief An incoming message handler.
 *
 * Used to handle any incoming message from clients.
 * Based on the Duty Cycle state, client's ID (has the client been suspended?)
 * and the message type, determines the right action, which could be one of:
 * - let the message through to allow the framework to process it further
 * - respond with 'Suspend ZCL Messages' and suppress this message
 * - suppress the message silently.
 *
 * @return True if the message has been processed by the Sub-GHz plugin and
 *         should be suppressed, false to allow the message through.
 */
boolean emAfSubGhzServerIncomingMessage(EmberIncomingMessageType type,
                                        EmberApsFrame *apsFrame,
                                        EmberNodeId sender,
                                        uint16_t messageLength,
                                        uint8_t *messageContents);

/** @brief Incoming ZDO message handler.
 *
 * Used to handle the incoming Mgmt_NWK_Unsolicited_Enhanced_Update_notify command.
 * Calls the user's emberAfPluginSubGhzUnsolicitedEnhancedUpdateNotifyCallback in response.
 */
void emAfSubGhzServerZDOMessageReceivedCallback(EmberNodeId sender,
                                                const uint8_t* message,
                                                uint16_t length);
#endif // SUB_GHZ_SERVER_H_INCLUDED
