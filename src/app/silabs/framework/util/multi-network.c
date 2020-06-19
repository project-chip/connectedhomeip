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
 * @brief
 *******************************************************************************
   ******************************************************************************/

#include "app/framework/include/af.h"

const EmAfZigbeeProNetwork *emAfCurrentZigbeeProNetwork = NULL;

//#define NETWORK_INDEX_DEBUG
#if defined(EMBER_TEST) || defined(NETWORK_INDEX_DEBUG)
  #define NETWORK_INDEX_ASSERT(x) assert(x)
#else
  #define NETWORK_INDEX_ASSERT(x)
#endif

#if EMBER_SUPPORTED_NETWORKS == 1
EmberStatus emAfInitializeNetworkIndexStack(void)
{
  NETWORK_INDEX_ASSERT(emberGetCurrentNetwork() == 0);
  NETWORK_INDEX_ASSERT(EMBER_AF_DEFAULT_NETWORK_INDEX == 0);
  emAfCurrentZigbeeProNetwork = &emAfZigbeeProNetworks[0];
  return EMBER_SUCCESS;
}

EmberStatus emberAfPushNetworkIndex(uint8_t networkIndex)
{
  NETWORK_INDEX_ASSERT(networkIndex == 0);
  NETWORK_INDEX_ASSERT(emberGetCurrentNetwork() == 0);
  return (networkIndex == 0 ? EMBER_SUCCESS : EMBER_INVALID_CALL);
}

EmberStatus emberAfPushCallbackNetworkIndex(void)
{
  NETWORK_INDEX_ASSERT(emberGetCurrentNetwork() == 0);
  NETWORK_INDEX_ASSERT(emberGetCallbackNetwork() == 0);
  return EMBER_SUCCESS;
}

EmberStatus emberAfPushEndpointNetworkIndex(uint8_t endpoint)
{
  uint8_t networkIndex = emberAfNetworkIndexFromEndpoint(endpoint);
  NETWORK_INDEX_ASSERT(networkIndex != 0xFF);
  if (networkIndex == 0xFF) {
    return EMBER_INVALID_ENDPOINT;
  }
  NETWORK_INDEX_ASSERT(networkIndex == 0);
  NETWORK_INDEX_ASSERT(emberGetCurrentNetwork() == 0);
  return EMBER_SUCCESS;
}

EmberStatus emberAfPopNetworkIndex(void)
{
  NETWORK_INDEX_ASSERT(emberGetCurrentNetwork() == 0);
  return EMBER_SUCCESS;
}

void emAfAssertNetworkIndexStackIsEmpty(void)
{
  NETWORK_INDEX_ASSERT(emberGetCurrentNetwork() == 0);
}

uint8_t emberAfPrimaryEndpointForNetworkIndex(uint8_t networkIndex)
{
  NETWORK_INDEX_ASSERT(networkIndex == 0);
  return (networkIndex == 0 ? emberAfPrimaryEndpoint() : 0xFF);
}

uint8_t emberAfPrimaryEndpointForCurrentNetworkIndex(void)
{
  NETWORK_INDEX_ASSERT(emberGetCurrentNetwork() == 0);
  return emberAfPrimaryEndpoint();
}

uint8_t emberAfNetworkIndexFromEndpoint(uint8_t endpoint)
{
  uint8_t index = emberAfIndexFromEndpoint(endpoint);
  NETWORK_INDEX_ASSERT(index != 0xFF);
  if (index == 0xFF) {
    return 0xFF;
  }
  NETWORK_INDEX_ASSERT(emberAfNetworkIndexFromEndpointIndex(index) == 0);
  return 0;
}

void emberAfNetworkEventControlSetInactive(EmberEventControl *controls)
{
  NETWORK_INDEX_ASSERT(emberGetCurrentNetwork() == 0);
  emberEventControlSetInactive(controls[0]);
}

bool emberAfNetworkEventControlGetActive(EmberEventControl *controls)
{
  NETWORK_INDEX_ASSERT(emberGetCurrentNetwork() == 0);
  return emberEventControlGetActive(controls[0]);
}

void emberAfNetworkEventControlSetActive(EmberEventControl *controls)
{
  NETWORK_INDEX_ASSERT(emberGetCurrentNetwork() == 0);
  emberEventControlSetActive(controls[0]);
}

EmberStatus emberAfNetworkEventControlSetDelayMS(EmberEventControl *controls,
                                                 uint32_t delayMs)
{
  NETWORK_INDEX_ASSERT(emberGetCurrentNetwork() == 0);
  return emberAfEventControlSetDelayMS(&controls[0], delayMs);
}

EmberStatus emberAfNetworkEventControlSetDelayQS(EmberEventControl *controls,
                                                 uint32_t delayQs)
{
  NETWORK_INDEX_ASSERT(emberGetCurrentNetwork() == 0);
  return emberAfEventControlSetDelayQS(&controls[0], delayQs);
}

EmberStatus emberAfNetworkEventControlSetDelayMinutes(EmberEventControl *controls,
                                                      uint16_t delayM)
{
  NETWORK_INDEX_ASSERT(emberGetCurrentNetwork() == 0);
  return emberAfEventControlSetDelayMinutes(&controls[0], delayM);
}

#else

// We use two bits to describe a network index and sixteen bits to store our
// stack of network indices.  This limits us to a maximum of four networks
// indices and a maximum of eight in our stack.  We also remember one default
// network that we resort to when our stack is empty.
static uint16_t networkIndexStack = 0;
static uint8_t networkIndices = 0;
#define NETWORK_INDEX_BITS       2
#define NETWORK_INDEX_MAX        (1 << NETWORK_INDEX_BITS)
#define NETWORK_INDEX_MASK       (NETWORK_INDEX_MAX - 1)
#define NETWORK_INDEX_STACK_SIZE (sizeof(networkIndexStack) * 8 / NETWORK_INDEX_BITS)

static EmberStatus setCurrentNetwork(void)
{
  EmberStatus status;
  uint8_t networkIndex = (networkIndices == 0
                          ? EMBER_AF_DEFAULT_NETWORK_INDEX
                          : networkIndexStack & NETWORK_INDEX_MASK);
  status = emberSetCurrentNetwork(networkIndex);
  NETWORK_INDEX_ASSERT(status == EMBER_SUCCESS);
  NETWORK_INDEX_ASSERT(emberGetCurrentNetwork() == networkIndex);
  if (status == EMBER_SUCCESS) {
    //TODO RF4CE removal: can we get rid of the separate pro network??
    emAfCurrentZigbeeProNetwork = &emAfZigbeeProNetworks[networkIndex];
  }
  return status;
}

EmberStatus emAfInitializeNetworkIndexStack(void)
{
  EmberStatus status;
  NETWORK_INDEX_ASSERT(networkIndices == 0);
  if (networkIndices != 0) {
    return EMBER_INVALID_CALL;
  }
  status = setCurrentNetwork();
  NETWORK_INDEX_ASSERT(status == EMBER_SUCCESS);
  NETWORK_INDEX_ASSERT(networkIndices == 0);
  NETWORK_INDEX_ASSERT(emberGetCurrentNetwork() == EMBER_AF_DEFAULT_NETWORK_INDEX);
  return status;
}

EmberStatus emberAfPushNetworkIndex(uint8_t networkIndex)
{
  EmberStatus status;
  NETWORK_INDEX_ASSERT(networkIndex < NETWORK_INDEX_MAX);
  if (NETWORK_INDEX_MAX <= networkIndex) {
    return EMBER_INDEX_OUT_OF_RANGE;
  }
  NETWORK_INDEX_ASSERT(networkIndices < NETWORK_INDEX_STACK_SIZE);
  if (NETWORK_INDEX_STACK_SIZE <= networkIndices) {
    return EMBER_TABLE_FULL;
  }
  networkIndexStack <<= NETWORK_INDEX_BITS;
  networkIndexStack |= networkIndex;
  networkIndices++;
  status = setCurrentNetwork();
  NETWORK_INDEX_ASSERT(status == EMBER_SUCCESS);
  NETWORK_INDEX_ASSERT(0 < networkIndices);
  NETWORK_INDEX_ASSERT(emberGetCurrentNetwork() == networkIndex);
  return status;
}

EmberStatus emberAfPushCallbackNetworkIndex(void)
{
  EmberStatus status = emberAfPushNetworkIndex(emberGetCallbackNetwork());
  NETWORK_INDEX_ASSERT(status == EMBER_SUCCESS);
  NETWORK_INDEX_ASSERT(0 < networkIndices);
  NETWORK_INDEX_ASSERT(emberGetCurrentNetwork() == emberGetCallbackNetwork());
  return status;
}

EmberStatus emberAfPushEndpointNetworkIndex(uint8_t endpoint)
{
  EmberStatus status;
  uint8_t networkIndex = emberAfNetworkIndexFromEndpoint(endpoint);
  NETWORK_INDEX_ASSERT(networkIndex != 0xFF);
  if (networkIndex == 0xFF) {
    return EMBER_INVALID_ENDPOINT;
  }
  status = emberAfPushNetworkIndex(networkIndex);
  NETWORK_INDEX_ASSERT(status == EMBER_SUCCESS);
  NETWORK_INDEX_ASSERT(0 < networkIndices);
  NETWORK_INDEX_ASSERT(emberGetCurrentNetwork() == networkIndex);
  return status;
}

EmberStatus emberAfPopNetworkIndex(void)
{
  EmberStatus status;
  NETWORK_INDEX_ASSERT(0 < networkIndices);
  if (networkIndices == 0) {
    return EMBER_INVALID_CALL;
  }
  networkIndexStack >>= NETWORK_INDEX_BITS;
  networkIndices--;
  status = setCurrentNetwork();
  NETWORK_INDEX_ASSERT(status == EMBER_SUCCESS);
  return status;
}

void emAfAssertNetworkIndexStackIsEmpty(void)
{
  NETWORK_INDEX_ASSERT(networkIndices == 0);
  NETWORK_INDEX_ASSERT(emberGetCurrentNetwork() == EMBER_AF_DEFAULT_NETWORK_INDEX);
}

uint8_t emberAfPrimaryEndpointForNetworkIndex(uint8_t networkIndex)
{
  uint8_t i;
  NETWORK_INDEX_ASSERT(networkIndex < NETWORK_INDEX_MAX);
  for (i = 0; i < emberAfEndpointCount(); i++) {
    if (emberAfNetworkIndexFromEndpointIndex(i) == networkIndex) {
      return emberAfEndpointFromIndex(i);
    }
  }
  return 0xFF;
}

uint8_t emberAfPrimaryEndpointForCurrentNetworkIndex(void)
{
  return emberAfPrimaryEndpointForNetworkIndex(emberGetCurrentNetwork());
}

uint8_t emberAfNetworkIndexFromEndpoint(uint8_t endpoint)
{
  uint8_t index = emberAfIndexFromEndpoint(endpoint);
  NETWORK_INDEX_ASSERT(index != 0xFF);
  return (index == 0xFF ? 0xFF : emberAfNetworkIndexFromEndpointIndex(index));
}

void emberAfNetworkEventControlSetInactive(EmberEventControl *controls)
{
  EmberEventControl *control = controls + emberGetCurrentNetwork();
  emberEventControlSetInactive(*control);
}

bool emberAfNetworkEventControlGetActive(EmberEventControl *controls)
{
  EmberEventControl *control = controls + emberGetCurrentNetwork();
  return emberEventControlGetActive(*control);
}

void emberAfNetworkEventControlSetActive(EmberEventControl *controls)
{
  EmberEventControl *control = controls + emberGetCurrentNetwork();
  emberEventControlSetActive(*control);
}

EmberStatus emberAfNetworkEventControlSetDelayMS(EmberEventControl *controls,
                                                 uint32_t delayMs)
{
  EmberEventControl *control = controls + emberGetCurrentNetwork();
  return emberAfEventControlSetDelayMS(control, delayMs);
}

EmberStatus emberAfNetworkEventControlSetDelayQS(EmberEventControl *controls,
                                                 uint32_t delayQs)
{
  EmberEventControl *control = controls + emberGetCurrentNetwork();
  return emberAfEventControlSetDelayQS(control, delayQs);
}

EmberStatus emberAfNetworkEventControlSetDelayMinutes(EmberEventControl *controls,
                                                      uint16_t delayM)
{
  EmberEventControl *control = controls + emberGetCurrentNetwork();
  return emberAfEventControlSetDelayMinutes(control, delayM);
}

#endif
