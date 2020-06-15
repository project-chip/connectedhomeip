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
 * @brief CLI for the Gateway Relay COAP plugin.
 *******************************************************************************
   ******************************************************************************/

// common include file
#include "app/framework/util/common.h"
#include "app/framework/plugin-host/gateway-relay-coap/gateway-relay-coap.h"
#include "app/framework/plugin-host/coap-server/coap-server.h"
#include "include/coap/coap.h"

//------------------------------------------------------------------------------
// Forward Declarations

//------------------------------------------------------------------------------
// Globals

#define STRING_LENGTH 50
static uint8_t messageString[STRING_LENGTH + 1];

//------------------------------------------------------------------------------
// Functions

void emberAfPluginGatewayRelayCoapParseCommand(void)
{
  uint8_t bytesRead;

  bytesRead = emberCopyStringArgument(0,
                                      messageString,
                                      STRING_LENGTH,
                                      false);
  messageString[bytesRead] = 0;

  emberAfCorePrintln("%d %s", bytesRead, messageString);

  emberAfGatewayRelayCoapParseDotdotMessage(bytesRead, messageString, 0, 0);
}

uint8_t serverString[STRING_LENGTH + 1];
uint16_t serverPort;

void emberAfPluginGatewayRelayCoapSetServerCommand(void)
{
  uint8_t bytesRead;

  bytesRead = emberCopyStringArgument(0,
                                      serverString,
                                      STRING_LENGTH,
                                      false);
  serverString[bytesRead] = 0;
  serverPort = (uint16_t)emberUnsignedCommandArgument(1);

  emberAfCorePrintln("Set Server %s:%d", serverString, serverPort);

  emberAfPluginCoapServerSetServerNameAndPort(serverString, serverPort);
}

void emberAfPluginGatewayRelayCoapSendCommand(void)
{
  uint8_t bytesRead;

  bytesRead = emberCopyStringArgument(0,
                                      messageString,
                                      STRING_LENGTH,
                                      false);
  messageString[bytesRead] = 0;

  emberAfCorePrintln("Message to send: \"%s\"", messageString);

  emberAfPluginCoapServerSendMessage(NULL,
                                     messageString,
                                     strlen((char *)messageString),
                                     PLUGIN_COAP_SERVER_MAX_PORTS,
                                     COAP_REQUEST_PUT);
}

void emberAfPluginGatewayRelayCoapSendPortCommand(void)
{
  uint8_t bytesRead;
  uint16_t port = (uint16_t)emberUnsignedCommandArgument(1);

  bytesRead = emberCopyStringArgument(0,
                                      messageString,
                                      STRING_LENGTH,
                                      false);
  messageString[bytesRead] = 0;

  emberAfCorePrintln("Message to send: \"%s\"", messageString);

  emberAfPluginCoapServerSendMessage(NULL,
                                     messageString,
                                     strlen((char *)messageString),
                                     port,
                                     COAP_REQUEST_PUT);
}

void emberAfPluginGatewayRelayCoapStartHeartbeat(uint16_t seconds);
void emberAfPluginGatewayRelayCoapStopHeartbeat(void);

void emberAfPluginGatewayRelayCoapStartHeartbeatCommand(void)
{
  uint16_t timeoutSeconds = (uint16_t)emberUnsignedCommandArgument(0);

  emberAfPluginGatewayRelayCoapStartHeartbeat(timeoutSeconds);
}

void emberAfPluginGatewayRelayCoapStopHeartbeatCommand(void)
{
  emberAfPluginGatewayRelayCoapStopHeartbeat();
}
