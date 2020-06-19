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
 * @brief Interface for initiating and processing ZDO service discovery
 * (match descriptor) requests and response.
 *******************************************************************************
   ******************************************************************************/

#ifndef SILABS_SERVICE_DISCOVERY_H
#define SILABS_SERVICE_DISCOVERY_H

#include "../include/af.h"
#define EMBER_SUPPORTED_SERVICE_DISCOVERY_STATES_PER_NETWORK 4
// Internal Functions
void emAfServiceDiscoveryMatched(EmberNodeId nodeId,
                                 uint8_t endpointListLength,
                                 uint8_t* endpointList);

// Platform specific call to send a match descriptor request.
EmberStatus emAfSendMatchDescriptor(EmberNodeId target,
                                    EmberAfProfileId profileId,
                                    EmberAfClusterId clusterId,
                                    bool serverCluster);

bool emAfServiceDiscoveryIncoming(EmberNodeId sender,
                                  EmberApsFrame* apsFrame,
                                  const uint8_t* message,
                                  uint16_t length);

extern EmberEventControl emAfServiceDiscoveryEventControls[EMBER_SUPPORTED_SERVICE_DISCOVERY_STATES_PER_NETWORK][EMBER_SUPPORTED_NETWORKS];
void emAfServiceDiscoveryTimeoutHandler(EmberEventControl *control);
void emAfServiceDiscoveryComplete(uint8_t networkIndex, uint8_t sequenceNumber);

#if EMBER_SUPPORTED_NETWORKS == 1
  #define EM_AF_SERVICE_DISCOVERY_EVENT_STRINGS \
  "Svc Disc Request State 0 NWK 0",             \
  "Svc Disc Request State 1 NWK 0",             \
  "Svc Disc Request State 2 NWK 0",             \
  "Svc Disc Request State 3 NWK 0",
  #define EM_AF_SERVICE_DISCOVERY_EVENTS                                                           \
  { &emAfServiceDiscoveryEventControls[0][0], (void(*)(void))emAfServiceDiscoveryTimeoutHandler }, \
  { &emAfServiceDiscoveryEventControls[1][0], (void(*)(void))emAfServiceDiscoveryTimeoutHandler }, \
  { &emAfServiceDiscoveryEventControls[2][0], (void(*)(void))emAfServiceDiscoveryTimeoutHandler }, \
  { &emAfServiceDiscoveryEventControls[3][0], (void(*)(void))emAfServiceDiscoveryTimeoutHandler },
#elif EMBER_SUPPORTED_NETWORKS == 2
  #define EM_AF_SERVICE_DISCOVERY_EVENT_STRINGS \
  "Svc Disc Request State 0 NWK 0",             \
  "Svc Disc Request State 1 NWK 0",             \
  "Svc Disc Request State 2 NWK 0",             \
  "Svc Disc Request State 3 NWK 0",             \
  "Svc Disc Request State 0 NWK 1",             \
  "Svc Disc Request State 1 NWK 1",             \
  "Svc Disc Request State 2 NWK 1",             \
  "Svc Disc Request State 3 NWK 1",
  #define EM_AF_SERVICE_DISCOVERY_EVENTS                                                           \
  { &emAfServiceDiscoveryEventControls[0][0], (void(*)(void))emAfServiceDiscoveryTimeoutHandler }, \
  { &emAfServiceDiscoveryEventControls[1][0], (void(*)(void))emAfServiceDiscoveryTimeoutHandler }, \
  { &emAfServiceDiscoveryEventControls[2][0], (void(*)(void))emAfServiceDiscoveryTimeoutHandler }, \
  { &emAfServiceDiscoveryEventControls[3][0], (void(*)(void))emAfServiceDiscoveryTimeoutHandler }, \
  { &emAfServiceDiscoveryEventControls[0][1], (void(*)(void))emAfServiceDiscoveryTimeoutHandler }, \
  { &emAfServiceDiscoveryEventControls[1][1], (void(*)(void))emAfServiceDiscoveryTimeoutHandler }, \
  { &emAfServiceDiscoveryEventControls[2][1], (void(*)(void))emAfServiceDiscoveryTimeoutHandler }, \
  { &emAfServiceDiscoveryEventControls[3][1], (void(*)(void))emAfServiceDiscoveryTimeoutHandler },
#elif EMBER_SUPPORTED_NETWORKS == 3
  #define EM_AF_SERVICE_DISCOVERY_EVENT_STRINGS \
  "Svc Disc Request State 0 NWK 0",             \
  "Svc Disc Request State 1 NWK 0",             \
  "Svc Disc Request State 2 NWK 0",             \
  "Svc Disc Request State 3 NWK 0",             \
  "Svc Disc Request State 0 NWK 1",             \
  "Svc Disc Request State 1 NWK 1",             \
  "Svc Disc Request State 2 NWK 1",             \
  "Svc Disc Request State 3 NWK 1",             \
  "Svc Disc Request State 0 NWK 2",             \
  "Svc Disc Request State 1 NWK 2",             \
  "Svc Disc Request State 2 NWK 2",             \
  "Svc Disc Request State 3 NWK 2",
  #define EM_AF_SERVICE_DISCOVERY_EVENTS                                                           \
  { &emAfServiceDiscoveryEventControls[0][0], (void(*)(void))emAfServiceDiscoveryTimeoutHandler }, \
  { &emAfServiceDiscoveryEventControls[1][0], (void(*)(void))emAfServiceDiscoveryTimeoutHandler }, \
  { &emAfServiceDiscoveryEventControls[2][0], (void(*)(void))emAfServiceDiscoveryTimeoutHandler }, \
  { &emAfServiceDiscoveryEventControls[3][0], (void(*)(void))emAfServiceDiscoveryTimeoutHandler }, \
  { &emAfServiceDiscoveryEventControls[0][1], (void(*)(void))emAfServiceDiscoveryTimeoutHandler }, \
  { &emAfServiceDiscoveryEventControls[1][1], (void(*)(void))emAfServiceDiscoveryTimeoutHandler }, \
  { &emAfServiceDiscoveryEventControls[2][1], (void(*)(void))emAfServiceDiscoveryTimeoutHandler }, \
  { &emAfServiceDiscoveryEventControls[3][1], (void(*)(void))emAfServiceDiscoveryTimeoutHandler }, \
  { &emAfServiceDiscoveryEventControls[0][2], (void(*)(void))emAfServiceDiscoveryTimeoutHandler }, \
  { &emAfServiceDiscoveryEventControls[1][2], (void(*)(void))emAfServiceDiscoveryTimeoutHandler }, \
  { &emAfServiceDiscoveryEventControls[2][2], (void(*)(void))emAfServiceDiscoveryTimeoutHandler }, \
  { &emAfServiceDiscoveryEventControls[3][2], (void(*)(void))emAfServiceDiscoveryTimeoutHandler },
#elif EMBER_SUPPORTED_NETWORKS == 4
  #define EM_AF_SERVICE_DISCOVERY_EVENT_STRINGS \
  "Svc Disc Request State 0 NWK 0",             \
  "Svc Disc Request State 1 NWK 0",             \
  "Svc Disc Request State 2 NWK 0",             \
  "Svc Disc Request State 3 NWK 0",             \
  "Svc Disc Request State 0 NWK 1",             \
  "Svc Disc Request State 1 NWK 1",             \
  "Svc Disc Request State 2 NWK 1",             \
  "Svc Disc Request State 3 NWK 1",             \
  "Svc Disc Request State 0 NWK 2",             \
  "Svc Disc Request State 1 NWK 2",             \
  "Svc Disc Request State 2 NWK 2",             \
  "Svc Disc Request State 3 NWK 2",             \
  "Svc Disc Request State 0 NWK 3",             \
  "Svc Disc Request State 1 NWK 3",             \
  "Svc Disc Request State 2 NWK 3",             \
  "Svc Disc Request State 3 NWK 3",
  #define EM_AF_SERVICE_DISCOVERY_EVENTS                                                           \
  { &emAfServiceDiscoveryEventControls[0][0], (void(*)(void))emAfServiceDiscoveryTimeoutHandler }, \
  { &emAfServiceDiscoveryEventControls[1][0], (void(*)(void))emAfServiceDiscoveryTimeoutHandler }, \
  { &emAfServiceDiscoveryEventControls[2][0], (void(*)(void))emAfServiceDiscoveryTimeoutHandler }, \
  { &emAfServiceDiscoveryEventControls[3][0], (void(*)(void))emAfServiceDiscoveryTimeoutHandler }, \
  { &emAfServiceDiscoveryEventControls[0][1], (void(*)(void))emAfServiceDiscoveryTimeoutHandler }, \
  { &emAfServiceDiscoveryEventControls[1][1], (void(*)(void))emAfServiceDiscoveryTimeoutHandler }, \
  { &emAfServiceDiscoveryEventControls[2][1], (void(*)(void))emAfServiceDiscoveryTimeoutHandler }, \
  { &emAfServiceDiscoveryEventControls[3][1], (void(*)(void))emAfServiceDiscoveryTimeoutHandler }, \
  { &emAfServiceDiscoveryEventControls[0][2], (void(*)(void))emAfServiceDiscoveryTimeoutHandler }, \
  { &emAfServiceDiscoveryEventControls[1][2], (void(*)(void))emAfServiceDiscoveryTimeoutHandler }, \
  { &emAfServiceDiscoveryEventControls[2][2], (void(*)(void))emAfServiceDiscoveryTimeoutHandler }, \
  { &emAfServiceDiscoveryEventControls[3][2], (void(*)(void))emAfServiceDiscoveryTimeoutHandler }, \
  { &emAfServiceDiscoveryEventControls[0][3], (void(*)(void))emAfServiceDiscoveryTimeoutHandler }, \
  { &emAfServiceDiscoveryEventControls[1][3], (void(*)(void))emAfServiceDiscoveryTimeoutHandler }, \
  { &emAfServiceDiscoveryEventControls[2][3], (void(*)(void))emAfServiceDiscoveryTimeoutHandler }, \
  { &emAfServiceDiscoveryEventControls[3][3], (void(*)(void))emAfServiceDiscoveryTimeoutHandler },
#else
  #error
#endif

#endif // SILABS_SERVICE_DISCOVERY_H
