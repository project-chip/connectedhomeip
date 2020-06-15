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
 * @brief Definitions for the Gateway plugin.
 *******************************************************************************
   ******************************************************************************/

void gatewayBackchannelStop(void);

typedef uint8_t BackchannelState;

enum {
  NO_CONNECTION = 0,
  CONNECTION_EXISTS = 1,
  NEW_CONNECTION = 2,
  CONNECTION_ERROR = 3,
};

extern const bool backchannelSupported;
extern bool backchannelEnable;
extern int backchannelSerialPortOffset;

EmberStatus backchannelStartServer(uint8_t port);
EmberStatus backchannelStopServer(uint8_t port);
EmberStatus backchannelReceive(uint8_t port, char* data);
EmberStatus backchannelSend(uint8_t port, uint8_t * data, uint8_t length);

EmberStatus backchannelClientConnectionCleanup(uint8_t port);

BackchannelState backchannelCheckConnection(uint8_t port,
                                            bool waitForConnection);

EmberStatus backchannelMapStandardInputOutputToRemoteConnection(int port);
EmberStatus backchannelCloseConnection(uint8_t port);
EmberStatus backchannelServerPrintf(const char* formatString, ...);
EmberStatus backchannelClientPrintf(uint8_t port, const char* formatString, ...);
EmberStatus backchannelClientVprintf(uint8_t port,
                                     const char* formatString,
                                     va_list ap);
