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
 * @brief Network stubs for AFV2 tests.
 *******************************************************************************
   ******************************************************************************/

#include "app/framework/include/af.h"
#include "test-framework-network.h"

EmberEUI64 localEui64;
EmberNodeId testFrameworkNodeId = EMBER_NULL_NODE_ID;

static uint8_t currentNetworkIndex =  EM_AF_DEFAULT_NETWORK_INDEX_FOR_UNIT_TESTS;

EmberNetworkStatus testFrameworkNetworkState = EMBER_NO_NETWORK;

#define EXTENDED_PAN_ID \
  { 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x00, 0x01 }
#define PAN_ID 0xDEAD
#define CHANNEL 24

static EmberNodeType currentNodeType = EMBER_COORDINATOR;

static EmberNetworkParameters currentNetworkParameters = {
  EXTENDED_PAN_ID,
  PAN_ID,
  -1,      // tx power
  CHANNEL,
  EMBER_USE_MAC_ASSOCIATION,  // join type (only used for joining, duh)
  0x0000,   // NWK manager ID (the stack default)
  0,        // NWK update ID
  EMBER_ALL_802_15_4_CHANNELS_MASK  // NWK channel mask
};

void emberAfGetEui64(EmberEUI64 returnEui64)
{
  MEMMOVE(returnEui64, localEui64, EUI64_SIZE);
}

void setLocalEui64(EmberEUI64 eui64)
{
  MEMMOVE(localEui64, eui64, EUI64_SIZE);
}

EmberNodeId emberAfGetNodeId(void)
{
  return testFrameworkNodeId;
}

EmberNetworkStatus emberAfNetworkState(void)
{
  return testFrameworkNetworkState;
}

EmberStatus emberAfGetNetworkParameters(EmberNodeType* nodeType,
                                        EmberNetworkParameters* parameters)
{
  MEMMOVE(parameters, &currentNetworkParameters, sizeof(EmberNetworkParameters));
  *nodeType = currentNodeType;
  return EMBER_SUCCESS;
}

void setNetworkParameters(EmberNodeType *nodeType,
                          EmberNetworkParameters *parameters)
{
  currentNetworkParameters = *parameters;
  currentNodeType = *nodeType;
}

uint8_t emberGetCurrentNetwork(void)
{
  return currentNetworkIndex;
}

EmberStatus emberSetCurrentNetwork(uint8_t index)
{
  currentNetworkIndex = index;
  return EMBER_SUCCESS;
}

EmberStatus emberAfPushNetworkIndex(uint8_t networkIndex)
{
  return EMBER_SUCCESS;
}

EmberStatus emberAfPopNetworkIndex(void)
{
  return EMBER_SUCCESS;
}

bool emberStackIsPerformingRejoin(void)
{
  return false;
}

void emberAfNetworkEventControlSetActive(EmberEventControl *controls)
{
  emberEventControlSetActive(controls[currentNetworkIndex]);
}

EmberStatus emberAfNetworkEventControlSetDelayQS(EmberEventControl *controls,
                                                 uint32_t delayQs)
{
  controls[currentNetworkIndex].status = EMBER_EVENT_QS_TIME;
  controls[currentNetworkIndex].timeToExecute  = delayQs;
  return EMBER_SUCCESS;
}

EmberStatus emberAfNetworkEventControlSetDelayMS(EmberEventControl *controls,
                                                 uint32_t delayMs)
{
  controls[currentNetworkIndex].status = EMBER_EVENT_MS_TIME;
  controls[currentNetworkIndex].timeToExecute = delayMs;
  return EMBER_SUCCESS;
}

EmberStatus emberAfNetworkEventControlSetDelayMinutes(EmberEventControl *controls,
                                                      uint16_t delayM)
{
  controls[currentNetworkIndex].status = EMBER_EVENT_MINUTE_TIME;
  controls[currentNetworkIndex].timeToExecute = delayM;
  return EMBER_SUCCESS;
}

bool emberAfNetworkEventControlGetActive(EmberEventControl *controls)
{
  return (controls[currentNetworkIndex].status != EMBER_EVENT_INACTIVE);
}
