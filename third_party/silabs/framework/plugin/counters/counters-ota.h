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
 * @brief A library for retrieving Ember stack counters over the air.
 *
 * The library allows the application to request counters from a remote
 * node.  The request specifies whether the remote node should clear
 * its counters after sending back the response.
 * The library also handles and responds to incoming counter requests apropriately.
 * A convenience method recognizes incoming responses.
 *
 * Summary of requirements:
 * - Include counters.c and counters-ota.c in the build on an EmberZNet processor.
 *   Include only counters-ota-host.c in the build on an EZSP host processor.
 * - Call ::emberIsIncomingCountersRequest() in the incoming message handler.
 * - Call ::emberIsIncomingCountersResponse() in the incoming message handler.
 * - Call ::emberIsOutgoingCountersResponse() in the message sent handler.
 *
 * The request and response messages use profile id EMBER_PRIVATE_PROFILE_ID,
 * and the following cluster ids:
 *
 * - EMBER_REPORT_COUNTERS_REQUEST
 * - EMBER_REPORT_AND_CLEAR_COUNTERS_REQUEST
 * - EMBER_REPORT_COUNTERS_RESPONSE
 * - EMBER_REPORT_AND_CLEAR_COUNTERS_RESPONSE
 *
 * The request has no payload.  The payload of the response is a list of the
 * non-zero counters.  Each entry in the list consists of the one-byte counter
 * id followed by a two-byte counter, low byte first.  If all entries do not
 * fit into a single payload, multiple response messages are sent.
 *******************************************************************************
   ******************************************************************************/

#ifndef SILABS_PLUGIN_COUNTERS_OTA_H
#define SILABS_PLUGIN_COUNTERS_OTA_H

/** To ensure the counters response does not exceed the maximum
 * payload length, it is divided into separate messages if necessary.
 * The maximum length of 48 leaves sufficient room for headers, all
 * security modes, and source routing subframes if present.
 * It should not be necessary to change this value, but if you do,
 * make sure you know what you're doing.
 */
#define MAX_PAYLOAD_LENGTH 48

/** Sends an request to the specified destination to send back a report
 * of the non-zero counters.
 *
 * @param destination the address of the node to send the request to.
 * @param clearCounters whether or not the destination should reset its
 * counters to zero after successfully reporting them back to the requestor.
 * Note that due to technical constraints, counters are always cleared
 * on a network coprocessor such as the EM260.
 * @return EMBER_SUCCESS if the request was successfully submitted for
 * sending.  See ::emberSendUnicast() or ::ezspSendUnicast for possible
 * failure statuses.
 */
EmberStatus emberAfPluginCountersSendRequest(EmberNodeId destination,
                                             bool clearCounters);

/** The application must call this function at the beginning of
 * the incoming message handler.  It returns true if the incoming
 * message was a counters request and should be ignored by
 * the rest of the incoming message handler.
 *
 * @param apsFrame the APS frame passed to the incoming message handler.
 * @param sender the node id of the sender of the request.
 * @return true if the message was a counters request and should be
 * ignored by the rest of the incoming message handler.
 */
bool emberAfPluginCountersIsIncomingRequest(EmberApsFrame *apsFrame,
                                            EmberNodeId sender);

/** The application may call this function within the incoming
 * message handler to determine if the message is a counters
 * response.  If so, it is up to the application to decode
 * the payload, whose format is described above.
 *
 * @param apsFrame the APS frame passed to the incoming message handler.
 * @return true if the message is a counters response.
 */
bool emberAfPluginCountersIsIncomingResponse(EmberApsFrame *apsFrame);

/** The application must call this function at the begining of the
 * message sent handler.  It returns true if the message was a
 * counters response and should be ignored by the rest of the handler.
 *
 * @param apsFrame the APS frame passed to the message sent handler.
 * @param status the status passed to the message sent handler.
 * @param return true if the message was a counters response and should
 * be ignored by the rest of the message sent handler.
 */
bool emberAfPluginCountersIsOutgoingResponse(EmberApsFrame *apsFrame,
                                             EmberStatus status);

#endif // SILABS_PLUGIN_COUNTERS_OTA_H
