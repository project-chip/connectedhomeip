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
 * @brief Common routines for the Polling plugin, which controls an end device's
 *        polling behavior.
 *******************************************************************************
   ******************************************************************************/

#include "app/framework/include/af.h"
#include "app/framework/plugin/end-device-support/end-device-support.h"

// The polling task is not used on the host.
#ifdef EZSP_HOST
  #define ADD_POLLING_TASK()
  #define REMOVE_POLLING_TASK()
  #define REMOVE_SHORT_POLLING_FOR_PARENT_CONNECTIVITY_TASK()
#else
  #define ADD_POLLING_TASK()    emberAfAddToCurrentAppTasks(EMBER_AF_LAST_POLL_GOT_DATA)
  #define REMOVE_POLLING_TASK() emberAfRemoveFromCurrentAppTasks(EMBER_AF_LAST_POLL_GOT_DATA)
  #define REMOVE_SHORT_POLLING_FOR_PARENT_CONNECTIVITY_TASK() emberAfRemoveFromCurrentAppTasks(EMBER_AF_FORCE_SHORT_POLL_FOR_PARENT_CONNECTIVITY)
#endif

EmAfPollingState emAfPollingStates[EMBER_SUPPORTED_NETWORKS];

// This function is called when a poll completes and explains what happend with
// the poll.  If the number of sequential data polls not ACKed by the parent
// exceeds the threshold, we will try to find a new parent.
void emAfPollCompleteHandler(EmberStatus status, uint8_t limit)
{
  EmAfPollingState *state;
  uint8_t networkIndex;

  emberAfPushCallbackNetworkIndex();
  networkIndex = emberGetCurrentNetwork();
  state = &emAfPollingStates[networkIndex];

  if (emAfEnablePollCompletedCallback) {
    emberAfPluginEndDeviceSupportPollCompletedCallback(status);
  }

  switch (status) {
    case EMBER_SUCCESS:
      REMOVE_SHORT_POLLING_FOR_PARENT_CONNECTIVITY_TASK();
      ADD_POLLING_TASK();
      emberAfDebugPrintln("poll nwk %d: got data", networkIndex);
      state->numPollsFailing = 0;
      break;
    case EMBER_MAC_NO_DATA:
      REMOVE_SHORT_POLLING_FOR_PARENT_CONNECTIVITY_TASK();
      REMOVE_POLLING_TASK();
      emberAfDebugPrintln("poll nwk %d: no data", networkIndex);
      state->numPollsFailing = 0;
      break;
    case EMBER_PHY_TX_BLOCKED:
    case EMBER_PHY_TX_SCHED_FAIL:
    case EMBER_PHY_TX_CCA_FAIL:
      // This means the air was busy, which we don't count as a failure.
      REMOVE_POLLING_TASK();
      emberAfDebugPrintln("poll nwk %d: channel is busy", networkIndex);
      break;
    case EMBER_MAC_NO_ACK_RECEIVED:
      // If we are performing key establishment, we can ignore this since the
      // parent could go away for long periods of time while doing ECC processes.
      if (emberAfPerformingKeyEstablishment()) {
        break;
      }
      // Count failures until we hit the limit, then we try a rejoin. If rejoin
      // fails, it will trigger a move.
      state->numPollsFailing++;
      REMOVE_POLLING_TASK();
      emberAfDebugPrintln("Number of short poll attempts made to resolve parent connectivity: %d", state->numPollsFailing);
      emberAfAddToCurrentAppTasksCallback(EMBER_AF_FORCE_SHORT_POLL_FOR_PARENT_CONNECTIVITY);
      if (limit <= state->numPollsFailing) {
        if (!emberAfPluginEndDeviceSupportLostParentConnectivityCallback()) {
          emberAfStartMoveCallback();
        }
      }
      break;
    default:
      emberAfDebugPrintln("poll nwk %d: 0x%x", networkIndex, status);
  }

  emberAfPopNetworkIndex();
}
