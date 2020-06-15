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
 * @brief
 *******************************************************************************
   ******************************************************************************/

// This callback file is created for your convenience. You may add application
// code to this file. If you regenerate this file over a previous version, the
// previous version will be overwritten and any code you have added will be
// lost.

#include "app/framework/include/af.h"

#include "app/ncp/sample-app/xncp-led/led-protocol.h"

/* This sample application demostrates an NCP using a custom protocol to
 * communicate with the host. As an example protocol, the NCP has defined
 * commands so that the host can control an LED on the NCP's RCM.  See
 * led-protocol.h for details.
 *
 * The host sends custom EZSP commands to the NCP, and the NCP acts on them
 * based on the functionality in the code found below.
 * This sample application is meant to be paired with the xncp-led
 * sample application in the NCP Application Framework.
 */
static void sendLedProtocolCommand(uint8_t command, uint32_t possibleParam)
{
  EmberStatus status;
  uint8_t commandLength = 0;
  uint8_t commandPayload[LED_PROTOCOL_MAX_FRAME_LENGTH];
  uint8_t replyLength = LED_PROTOCOL_MAX_FRAME_LENGTH;
  uint8_t replyPayload[LED_PROTOCOL_MAX_FRAME_LENGTH];

  // Set the command byte.
  commandPayload[LED_PROTOCOL_COMMAND_INDEX] = command;
  commandLength++;

  // Conditionally set the parameter.
  if (command == LED_PROTOCOL_COMMAND_SET_FREQ) {
    emberAfCopyInt32u(commandPayload, 1, possibleParam);
    commandLength += sizeof(possibleParam);
  }

  // Send the command to the NCP.
  status = ezspCustomFrame(commandLength,
                           commandPayload,
                           &replyLength,
                           replyPayload);
  emberAfCorePrintln("Send custom frame: 0x%X", status);

  // If we were expecting a response, display it.
  if (command == LED_PROTOCOL_COMMAND_GET_FREQ) {
    emberAfCorePrintln(" Response (frequency): %u",
                       emberAfGetInt32u(replyPayload,
                                        LED_PROTOCOL_RESPONSE_INDEX,
                                        replyLength));
  } else if (command == LED_PROTOCOL_COMMAND_GET_LED) {
    uint8_t ledState = replyPayload[LED_PROTOCOL_RESPONSE_INDEX];
    emberAfCorePrintln("  Response (state): %u (%p)",
                       ledState,
                       ledStateNames[ledState]);
  }
}

static void getFrequencyCommand(void)
{
  sendLedProtocolCommand(LED_PROTOCOL_COMMAND_GET_FREQ, 0); // no param
}

static void setFrequencyCommand(void)
{
  uint32_t frequency = (uint32_t)emberUnsignedCommandArgument(0);
  sendLedProtocolCommand(LED_PROTOCOL_COMMAND_SET_FREQ, frequency);
}

static void getLedCommand(void)
{
  sendLedProtocolCommand(LED_PROTOCOL_COMMAND_GET_LED, 0); // no param
}

static void setLedCommand(void)
{
  uint8_t command = (uint8_t)emberUnsignedCommandArgument(0);
  if (command > LED_PROTOCOL_COMMAND_STROBE_LED) {
    emberAfCorePrintln("Invalid LED command: 0x%X", command);
  } else {
    sendLedProtocolCommand(command, 0); // no param
  }
}

static void getInfoCommand(void)
{
  uint16_t version, manufacturerId;
  EmberStatus status;

  status = ezspGetXncpInfo(&manufacturerId, &version);

  emberAfCorePrintln("Get XNCP info: status: 0x%X", status);
  emberAfCorePrintln("  manufacturerId: 0x%X, version: 0x%X",
                     manufacturerId, version);
}

EmberCommandEntry emberAfCustomCommands[] = {
  emberCommandEntryAction("get-led",
                          getLedCommand,
                          "",
                          "Get the state of an LED on the NCP."),
  emberCommandEntryAction("set-led",
                          setLedCommand,
                          "u",
                          "Set the state of an LED on the NCP using a custom LED protocol command."),
  emberCommandEntryAction("get-frequency",
                          getFrequencyCommand,
                          "",
                          "Get the current frequency of the LED strobe on the NCP."),
  emberCommandEntryAction("set-frequency",
                          setFrequencyCommand,
                          "w",
                          "Set the frequency of the LED strobe on the NCP."),

  emberCommandEntryAction("get-info",
                          getInfoCommand,
                          "",
                          "Display the XNCP information on the CLI."),

  emberCommandEntryTerminator()
};
