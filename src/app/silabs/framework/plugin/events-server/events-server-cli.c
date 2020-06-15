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
 * @brief CLI for the Events Server plugin.
 *******************************************************************************
   ******************************************************************************/

#include "app/framework/include/af.h"
#include "events-server.h"

#ifndef EMBER_AF_GENERATE_CLI
  #error The Events Server plugin is not compatible with the legacy CLI.
#endif

// plugin events-server clear <endpoint:1> <logId:1>
void emAfEventsServerCliClear(void)
{
  uint8_t endpoint = (uint8_t)emberUnsignedCommandArgument(0);
  EmberAfEventLogId logId = (EmberAfEventLogId)emberUnsignedCommandArgument(1);
  emberAfEventsServerClearEventLog(endpoint, logId);
}

// plugin events-server print <endpoint:1> <logId:1>
void emAfEventsServerCliPrint(void)
{
  uint8_t endpoint = (uint8_t)emberUnsignedCommandArgument(0);
  EmberAfEventLogId logId = (EmberAfEventLogId)emberUnsignedCommandArgument(1);
  emberAfEventsServerPrintEventLog(endpoint, logId);
}

// plugin events-server eprint <endpoint:1> <logId:1> <index:1>
void emAfEventsServerCliPrintEvent(void)
{
  uint8_t endpoint = (uint8_t)emberUnsignedCommandArgument(0);
  EmberAfEventLogId logId = (EmberAfEventLogId)emberUnsignedCommandArgument(1);
  uint8_t index = (uint8_t)emberUnsignedCommandArgument(2);
  EmberAfEvent event;
  if (emberAfEventsServerGetEvent(endpoint, logId, index, &event)) {
    emberAfEventsClusterPrintln("Event at index 0x%x in log 0x%x", index, logId);
    emberAfEventsServerPrintEvent(&event);
  } else {
    emberAfEventsClusterPrintln("Event at index 0x%x in log 0x%x is not present", index, logId);
  }
}

// plugin events-server set <endpoint:1> <logId:1> <index:1> <eventId:2> <eventTime:4> <data:?>
void emAfEventsServerCliSet(void)
{
  EmberAfEvent event;
  uint8_t length;
  uint8_t endpoint = (uint8_t)emberUnsignedCommandArgument(0);
  EmberAfEventLogId logId = (EmberAfEventLogId)emberUnsignedCommandArgument(1);
  uint8_t index = (uint8_t)emberUnsignedCommandArgument(2);
  event.eventId = (uint16_t)emberUnsignedCommandArgument(3);
  event.eventTime = (uint32_t)emberUnsignedCommandArgument(4);
  length = emberCopyStringArgument(5,
                                   event.eventData + 1,
                                   EMBER_AF_PLUGIN_EVENTS_SERVER_EVENT_DATA_LENGTH,
                                   false);
  event.eventData[0] = length;
  if (!emberAfEventsServerSetEvent(endpoint, logId, index, &event)) {
    emberAfEventsClusterPrintln("Event at index 0x%x in log 0x%x is not present", index, logId);
  } else {
    emberAfEventsClusterPrintln("Event added to log 0x%x at index 0x%x", logId, index);
  }
}

// plugin events-server add <endpoint:1> <logId:1> <eventId:2> <eventTime:4> <data:?>
void emAfEventsServerCliAdd(void)
{
  uint8_t length;
  uint8_t endpoint = (uint8_t)emberUnsignedCommandArgument(0);
  EmberAfEventLogId logId = (EmberAfEventLogId)emberUnsignedCommandArgument(1);
  EmberAfEvent event;
  event.eventId = (uint16_t)emberUnsignedCommandArgument(2);
  event.eventTime = (uint32_t)emberUnsignedCommandArgument(3);
  length = emberCopyStringArgument(4,
                                   event.eventData + 1,
                                   EMBER_AF_PLUGIN_EVENTS_SERVER_EVENT_DATA_LENGTH,
                                   false);
  event.eventData[0] = length;
  emberAfEventsServerAddEvent(endpoint, logId, &event);
}

// plugin events-server publish <nodeId:2> <srcEndpoint:1> <dstEndpoint:1> <logId:1> <index:1> <eventControl:1>
void emAfEventsServerCliPublish(void)
{
  EmberNodeId nodeId = (EmberNodeId)emberUnsignedCommandArgument(0);
  uint8_t srcEndpoint = (uint8_t)emberUnsignedCommandArgument(1);
  uint8_t dstEndpoint = (uint8_t)emberUnsignedCommandArgument(2);
  EmberAfEventLogId logId = (EmberAfEventLogId)emberUnsignedCommandArgument(3);
  uint8_t index = (uint8_t)emberUnsignedCommandArgument(4);
  uint8_t eventControl = (uint8_t)emberUnsignedCommandArgument(5);
  emberAfEventsServerPublishEventMessage(nodeId,
                                         srcEndpoint,
                                         dstEndpoint,
                                         logId,
                                         index,
                                         eventControl);
}
