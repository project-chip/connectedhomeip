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
 * @brief Implements emberCounterHandler() and keeps a tally
 * of the events reported by the stack.  The application must define
 * EMBER_APPLICATION_HAS_COUNTER_HANDLER in its configuration header
 * to use this file.
 *******************************************************************************
   ******************************************************************************/

#include "app/framework/include/af.h"
#include "app/util/serial/command-interpreter2.h"
#include "app/util/common/common.h"
#include "counters.h"
#include "counters-ota.h"

uint16_t emberCounters[EMBER_COUNTER_TYPE_COUNT];
uint16_t emberCountersThresholds[EMBER_COUNTER_TYPE_COUNT];
#ifndef EMBER_MULTI_NETWORK_STRIPPED
uint16_t emberMultiNetworkCounters[EMBER_SUPPORTED_NETWORKS]
[MULTI_NETWORK_COUNTER_TYPE_COUNT];
#endif

uint16_t emberMacCounters[EMBER_MAX_NUM_PHYS][EMBER_MAX_MAC_ONLY_COUNTERS];
#ifndef EMBER_MULTI_NETWORK_STRIPPED
static uint8_t getMultiNetworkCounterIndex(EmberCounterType type);
static void multiNetworkCounterHandler(EmberCounterType type, uint8_t data);
#endif

void emberAfPluginCountersInitCallback(void)
{
  emberAfPluginCountersClear();
  emberAfPluginCountersResetThresholds();
}

//Implement the stack callback by simply tallying up the counts.

void emberCounterHandler(EmberCounterType type, EmberCounterInfo info)

{
  bool phyIndexRequiredCounter = emberCounterRequiresPhyIndex(type);

  bool destionationRequiredCounter = emberCounterRequiresDestinationNodeId(type);

  uint8_t data = info.data;

  uint8_t phyIndex = 0;

  EmberNodeId destinationNodeId = EMBER_NULL_NODE_ID;

  // retreive possible counter info

  if (phyIndexRequiredCounter

      && destionationRequiredCounter) {
    phyIndex = ((EmberExtraCounterInfo *) (info.otherFields))->phy_index;

    destinationNodeId = ((EmberExtraCounterInfo *) (info.otherFields))->destinationNodeId;
  } else if (destionationRequiredCounter) {
    destinationNodeId = *((EmberNodeId*)info.otherFields);
  } else if (phyIndexRequiredCounter) {
    phyIndex = *((uint8_t*)info.otherFields);
  }

  //update the counter statistics

  if (phyIndexRequiredCounter && (type < EMBER_MAX_MAC_ONLY_COUNTERS)) {
    if (phyIndex < EMBER_MAX_NUM_PHYS) {
      if (emberMacCounters[phyIndex][type] < 0xFFFF) {
        emberMacCounters[phyIndex][type]++;
      }
    } else if (phyIndex == EMBER_MAX_NUM_PHYS) {
      // phyIndex cannot be equal to EMBER_MAX_NUM_PHYS except for
      // EMBER_COUNTER_MAC_TX_BROADCAST counter as broadcast can be sent on all interfaces.
      uint8_t i;
      // update counter for all interfaces.
      for (i = PHY_INDEX_NATIVE; i < EMBER_MAX_NUM_PHYS; i++) {
        if (emberMacCounters[i][type] < 0xFFFF) {
          emberMacCounters[i][type]++;
        }
      }
    } else {
      // adding else part for MISRA
    }
  }

  //To ensure that we call the counter rollover handler exactly once.

  bool hasCounterExceededThreshold;

  if (emberCounters[type] < emberCountersThresholds[type]) {
    hasCounterExceededThreshold = false;
  } else {
    hasCounterExceededThreshold = true;
  }

  if (emberCounters[type] < 0xFFFF) {
    emberCounters[type] += 1;
  }

  if (EMBER_COUNTER_APS_DATA_TX_UNICAST_SUCCESS == type

      || EMBER_COUNTER_APS_DATA_TX_UNICAST_FAILED == type) {
    if ((emberCounters[EMBER_COUNTER_APS_DATA_TX_UNICAST_RETRY] + data)

        < emberCountersThresholds[EMBER_COUNTER_APS_DATA_TX_UNICAST_RETRY]) {
      emberCounters[EMBER_COUNTER_APS_DATA_TX_UNICAST_RETRY] += data;
    } else {
      emberCounters[EMBER_COUNTER_APS_DATA_TX_UNICAST_RETRY] =

        emberCountersThresholds[EMBER_COUNTER_APS_DATA_TX_UNICAST_RETRY];
    }
  } else if (EMBER_COUNTER_PHY_TO_MAC_QUEUE_LIMIT_REACHED == type) {
    if ((emberCounters[type] + data) < emberCountersThresholds[type]) {
      // (data - 1) as counter is already incremented once above.
      // Note: We log this counter only if data > zero so (data - 1) won't roll
      // over back to 255.
      emberCounters[type] += (data - 1);
    } else {
      emberCounters[type] = emberCountersThresholds[type];
    }
  }

  if (emberCounters[type] >= emberCountersThresholds[type]) {
    if (!hasCounterExceededThreshold) {
      emberAfPluginCountersRolloverCallback(type);
    }
  }

#ifndef EMBER_MULTI_NETWORK_STRIPPED
  multiNetworkCounterHandler(type, data);
#endif

  if (destionationRequiredCounter
      && destinationNodeId == EMBER_NULL_NODE_ID) {
    //this might be an error
    return;
  }
}

void emberAfPluginCountersClear(void)
{
  MEMSET(emberCounters, 0, sizeof(emberCounters));
}

void emberAfPluginCountersResetThresholds(void)
{
  MEMSET(emberCountersThresholds, 0xFF, sizeof(emberCountersThresholds));
}

void emberAfPluginCountersSetThreshold(EmberCounterType type, uint16_t threshold)
{
  emberCountersThresholds[type] = threshold;
}

/*******************************************************************************
 * Multi-network counters support
 *
 * Some of the counters are per-network. Some of them are implicitly not
 * per-network because of the limited multi-network support. i.e., a node can be
 * coordinator/router/end device on just one network. The per-network counters
 * are defined in a table. The per-network counters are stored in a separate
 * two-dimensional array. We keep writing the multi-network counters also in the
 * usual single-network counters array.
 ******************************************************************************/
#ifndef EMBER_MULTI_NETWORK_STRIPPED
extern uint8_t emSupportedNetworks;

static const EmberCounterType multiNetworkCounterTable[] = {
  EMBER_COUNTER_MAC_RX_BROADCAST,
  EMBER_COUNTER_MAC_TX_BROADCAST,
  EMBER_COUNTER_MAC_RX_UNICAST,
  EMBER_COUNTER_MAC_TX_UNICAST_SUCCESS,
  EMBER_COUNTER_MAC_TX_UNICAST_RETRY,
  EMBER_COUNTER_MAC_TX_UNICAST_FAILED,
  EMBER_COUNTER_APS_DATA_RX_BROADCAST,
  EMBER_COUNTER_APS_DATA_TX_BROADCAST,
  EMBER_COUNTER_APS_DATA_RX_UNICAST,
  EMBER_COUNTER_APS_DATA_TX_UNICAST_SUCCESS,
  EMBER_COUNTER_APS_DATA_TX_UNICAST_RETRY,
  EMBER_COUNTER_APS_DATA_TX_UNICAST_FAILED,
  EMBER_COUNTER_NWK_FRAME_COUNTER_FAILURE,
  EMBER_COUNTER_APS_FRAME_COUNTER_FAILURE,
  EMBER_COUNTER_APS_LINK_KEY_NOT_AUTHORIZED,
  EMBER_COUNTER_NWK_DECRYPTION_FAILURE,
  EMBER_COUNTER_APS_DECRYPTION_FAILURE,
};

static uint8_t getMultiNetworkCounterIndex(EmberCounterType type)
{
  uint8_t i;
  for (i = 0; i < MULTI_NETWORK_COUNTER_TYPE_COUNT; i++) {
    if (multiNetworkCounterTable[i] == type) {
      return i;
    }
  }
  return 0xFF;
}

static void multiNetworkCounterHandler(EmberCounterType type, uint8_t data)
{
  uint8_t counterIndex = getMultiNetworkCounterIndex(type);

  // This function is always called in a callback context emberCounterHandler().
  // Not a multi-network counter, nothing to do.
  if (counterIndex >= MULTI_NETWORK_COUNTER_TYPE_COUNT) {
    return;
  }

  uint8_t nwkIndex = emberGetCallbackNetwork();
  assert(nwkIndex < emSupportedNetworks);

  if (emberMultiNetworkCounters[nwkIndex][counterIndex] < 0xFFFF) {
    emberMultiNetworkCounters[nwkIndex][counterIndex] += 1;
  }
}
#endif
