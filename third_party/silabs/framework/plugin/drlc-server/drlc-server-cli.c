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
 * @brief CLI for the DRLC plugin.
 *******************************************************************************
   ******************************************************************************/

#include "app/framework/include/af.h"
#include "app/util/serial/command-interpreter2.h"
#include "app/framework/plugin/drlc-server/drlc-server.h"

void emberAfPluginDrlcServerPrintCommand(void);
void emberAfPluginDrlcServerSlceCommand(void);
void emberAfPluginDrlcServerSslceCommand(void);
void emberAfPluginDrlcServerCslceCommand(void);

#if !defined(EMBER_AF_GENERATE_CLI)
EmberCommandEntry emberAfPluginDrlcServerCommands[] = {
  emberCommandEntryAction("print", emberAfPluginDrlcServerPrintCommand, "u", ""),
  emberCommandEntryAction("slce", emberAfPluginDrlcServerSlceCommand, "uuub", ""),
  emberCommandEntryAction("sslce", emberAfPluginDrlcServerSslceCommand, "vuuu", ""),
  emberCommandEntryAction("cslce", emberAfPluginDrlcServerCslceCommand, "u", ""),
  emberCommandEntryTerminator(),
};
#endif // EMBER_AF_GENERATE_CLI

// plugin drlc-server slce <endpoint:1> <index:1> <length:1> <load control event bytes>
// load control event bytes are expected as 23 raw bytes in the form
// {<eventId:4> <deviceClass:2> <ueg:1> <startTime:4> <duration:2> <criticalityLevel:1>
//  <coolingTempOffset:1> <heatingTempOffset:1> <coolingTempSetPoint:2> <heatingTempSetPoint:2>
//  <afgLoadPercentage:1> <dutyCycle:1> <eventControl:1> } all multibyte values should be
// little endian as though they were coming over the air.
// Example: plug drlc-server slce 0 23 { ab 00 00 00 ff 0f 00 00 00 00 00 01 00 01 00 00 09 1a 09 1a 0a 00 }
void emberAfPluginDrlcServerSlceCommand(void)
{
  EmberAfLoadControlEvent event;
  EmberStatus status;
  uint8_t endpoint = (uint8_t)emberUnsignedCommandArgument(0);
  uint8_t index = (uint8_t)emberUnsignedCommandArgument(1);
  uint8_t length = (uint8_t)emberUnsignedCommandArgument(2);
  uint8_t slceBuffer[sizeof(EmberAfLoadControlEvent)];
  status = emAfGetScheduledLoadControlEvent(endpoint, index, &event);

  if (status != EMBER_SUCCESS) {
    emberAfDemandResponseLoadControlClusterPrintln("slce fail: 0x%x", status);
    return;
  }
  if (length > sizeof(EmberAfLoadControlEvent)) {
    emberAfDemandResponseLoadControlClusterPrintln("slce fail, length: %x, max: %x", length, sizeof(EmberAfLoadControlEvent));
    return;
  }
  emberCopyStringArgument(3, slceBuffer, length, false);

  event.eventId = emberAfGetInt32u(slceBuffer, 0, length);
  event.deviceClass = emberAfGetInt16u(slceBuffer, 4, length);
  event.utilityEnrollmentGroup = emberAfGetInt8u(slceBuffer, 6, length);
  event.startTime = emberAfGetInt32u(slceBuffer, 7, length);
  event.duration = emberAfGetInt16u(slceBuffer, 11, length);
  event.criticalityLevel = emberAfGetInt8u(slceBuffer, 13, length);
  event.coolingTempOffset = emberAfGetInt8u(slceBuffer, 14, length);
  event.heatingTempOffset = emberAfGetInt8u(slceBuffer, 15, length);
  event.coolingTempSetPoint = emberAfGetInt16u(slceBuffer, 16, length);
  event.heatingTempSetPoint = emberAfGetInt16u(slceBuffer, 18, length);
  event.avgLoadPercentage = emberAfGetInt8u(slceBuffer, 20, length);
  event.dutyCycle = emberAfGetInt8u(slceBuffer, 21, length);
  event.eventControl = emberAfGetInt8u(slceBuffer, 22, length);
  event.source[0] = 0x00; //activate the event in the table
  status = emAfSetScheduledLoadControlEvent(endpoint, index, &event);
  emberAfDemandResponseLoadControlClusterPrintln("DRLC event scheduled on server: 0x%x", status);
}

// plugin drlc-server lce-schedule-mand <endpoint:1> <index:1> <eventId:4> <class:2> <ueg:1> <startTime:4>
//                <durationMins:2> <criticalLevel:1> <eventCtrl:1>
void emberAfPluginDrlcServerScheduleMandatoryLce(void)
{
  EmberAfLoadControlEvent event;
  EmberStatus status;

  uint8_t endpoint = (uint8_t)emberUnsignedCommandArgument(0);
  uint8_t index = (uint8_t)emberUnsignedCommandArgument(1);
  status = emAfGetScheduledLoadControlEvent(endpoint, index, &event);
  if ( status != EMBER_SUCCESS ) {
    emberAfDemandResponseLoadControlClusterPrintln("Error: Get LCE status=0x%x", status);
  }
  event.eventId                 = (uint32_t)emberUnsignedCommandArgument(2);
  event.deviceClass             = (uint16_t)emberUnsignedCommandArgument(3);
  event.utilityEnrollmentGroup  = (uint8_t) emberUnsignedCommandArgument(4);
  event.startTime               = (uint32_t)emberUnsignedCommandArgument(5);
  event.duration                = (uint16_t)emberUnsignedCommandArgument(6);
  event.criticalityLevel        = (uint8_t) emberUnsignedCommandArgument(7);
  event.eventControl            = (uint8_t) emberUnsignedCommandArgument(8);
  // Optionals
  event.coolingTempOffset = 0xFF;
  event.heatingTempOffset = 0xFF;
  event.coolingTempSetPoint = 0x8000;
  event.heatingTempSetPoint = 0x8000;
  event.avgLoadPercentage = 0x80;
  event.dutyCycle = 0xFF;

  event.source[0] = 0x00; //activate the event in the table
  status = emAfSetScheduledLoadControlEvent(endpoint, index, &event);
  if ( status == EMBER_SUCCESS ) {
    emberAfDemandResponseLoadControlClusterPrintln("DRLC event scheduled");
  } else {
    emberAfDemandResponseLoadControlClusterPrintln("Error: Schedule DRLC event: 0x%x", status);
  }
}

// plugin drlc-server sslce <nodeId:2> <srcEndpoint:1> <dstEndpoint:1> <index:1>
void emberAfPluginDrlcServerSslceCommand(void)
{
  emAfPluginDrlcServerSlceMessage((EmberNodeId)emberUnsignedCommandArgument(0),
                                  (uint8_t)emberUnsignedCommandArgument(1),
                                  (uint8_t)emberUnsignedCommandArgument(2),
                                  (uint8_t)emberUnsignedCommandArgument(3));
}
// plugin drlc-server print <endpoint:1>
void emberAfPluginDrlcServerPrintCommand(void)
{
  emAfPluginDrlcServerPrintInfo((uint8_t)emberUnsignedCommandArgument(0));
}

// plugin drlc-server cslce <endpoint:1>
void emberAfPluginDrlcServerCslceCommand(void)
{
  emAfClearScheduledLoadControlEvents((uint8_t)emberUnsignedCommandArgument(0));
}
