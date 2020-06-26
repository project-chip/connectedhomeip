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
 * @brief  *
 * This is an implementation of a drlc server which is intended to
 * be used for testing only. Generally it is expected that the drlc
 * server will act as a passthrough for drlc events and will not maintain
 * a list of scheduled events of its own. However for testing purposes
 * it is highly useful to have the server store scheduled events so that
 * - minus a backend - the ESP can respond to get scheduled events
 * commands.
 *
 * There are a couple things about
 * this code that make it incomplete as an actual drlc server table
 *
 * 1. Scheduled events are not stored in persistent memory, which they
 *    would have to be if this were intended for an ESP which for some
 *    reason were storing events.
 *
 * Note: The EmberAfLoadControlEvent source field is used by the plugin as a
 * status field.  If the first byte is set to 0xFF, then the event is
 * considered inactive,  If it is set to 0x00, then the event is considered
 * active.  Other bytes are used internally.
 *******************************************************************************
   ******************************************************************************/

#include "../../include/af-types.h"

#ifndef EMBER_AF_PLUGIN_DRLC_SERVER_SCHEDULED_EVENT_TABLE_SIZE
  #define  EMBER_AF_PLUGIN_DRLC_SERVER_SCHEDULED_EVENT_TABLE_SIZE 2
#endif //EMBER_AF_PLUGIN_DRLC_SERVER_SCHEDULED_EVENT_TABLE_SIZE

// used by the cli to clear all scheduled events in the drlc server
void emAfClearScheduledLoadControlEvents(uint8_t endpoint);

// Retrieves the load control event at the given index.
EmberStatus emAfGetScheduledLoadControlEvent(uint8_t endpoint,
                                             uint8_t index,
                                             EmberAfLoadControlEvent *event);

// Sets the load control event at the given index.  The first byte of the
// source field should be set to 0x00 for active or 0xFF for inactive.
EmberStatus emAfSetScheduledLoadControlEvent(uint8_t endpoint,
                                             uint8_t index,
                                             const EmberAfLoadControlEvent *event);

void emAfPluginDrlcServerPrintInfo(uint8_t endpoint);

void emAfPluginDrlcServerSlceMessage(EmberNodeId nodeId,
                                     uint8_t srcEndpoint,
                                     uint8_t dstEndpoint,
                                     uint8_t index);

// These internal functions help maintain event order, which is sorted by
// start time and issuer event ID. This is needed when responding to
// GetScheduledEvents with startTime and/or issuerEventId specified, where
// the returned events must be sent back in order
uint8_t findPlacementInEventOrderToLoadControlIndex(uint8_t ep, uint8_t index);
void updateEventOrderToLoadControlIndex(uint8_t ep,
                                        uint8_t index,
                                        bool eventAdded,
                                        bool isNewEvent);
void clearEventOrderToLoadControlIndex(uint8_t ep);
