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
 * @brief Routines for the dynamic block request feature of the OTA Server
 *        plugin. This feature tries to detect how an OTA client is treating the
 *        OTA Minimum Block Period field in an Image Block Response, and then
 *        translates the local delay value to the client's detected units.
 *******************************************************************************
   ******************************************************************************/

#include "app/framework/include/af.h"
#include "ota-server-dynamic-block-period.h"

#ifdef EMBER_AF_PLUGIN_OTA_SERVER_DYNAMIC_MIN_BLOCK_PERIOD_SUPPORT

// -----------------------------------------------------------------------------
// Globals
// -----------------------------------------------------------------------------

#define SHORT_DELAY_SECONDS   10    // Client delay if not probed yet
#define LONG_DELAY_SECONDS    60    // Client delay if no OTA sessions available

// The OTA server code dynamically finds out if the OTA client is treating the
// Minimum Block Period field in milliseconds or seconds. When an OTA transfer
// starts, we tell the client to delay value 60. If the client returns with an
// OTA request in 60-X seconds or greater, then we assume that the client is
// treating 60 as seconds. If the client queries any sooner, then we assume the
// client is using milliseconds. We store this determination (seconds/ms) in an
// array.
// Since a device consumes an entry in the array by starting an OTA transfer, we
// keep track of the last time it asked for a piece of the OTA file. This is to
// prevent a device from forever taking an entry in storage, which could
// potentially prohibit another device from starting an OTA transfer if all
// storage indices are taken (this can happen if a client begins an OTA transfer
// and then stops, for whatever reason, midway through the process)

#define MAX_DOWNLOADS EMBER_AF_PLUGIN_OTA_SERVER_MAX_NUM_DYNAMIC_OTA_DOWNLOADS

#define STATE_NONE          1 // We've never probed the client
#define STATE_IN_DISCOVERY  2 // We've told the client to delay a value
#define STATE_USES_SECONDS  3 // Client treated delay value as seconds
#define STATE_USES_MS       4 // Client treated delay value as milliseconds

// When judging a client's delay, if we send it 60, we expect it to delay
// 60 - (60 >> TOLERANCE_VALUE_BITSHIFT) seconds before we declare it uses
// seconds, else it uses milliseconds
#define TOLERANCE_VALUE_BITSHIFT  3

typedef struct {
  EmberNodeId nodeId;
  uint8_t     unitDiscoveryState; // STATE_ values
  uint32_t    msTickWhenLastSeen;
} EmAfOtaServerDownloadingNode;

EmAfOtaServerDownloadingNode downloadingNodes[MAX_DOWNLOADS];

#ifdef EMBER_TEST
 #define debugPrintln(...) emberAfOtaBootloadClusterPrintln(__VA_ARGS__)
#else
 #define debugPrintln(...)
#endif // EMBER_TEST

// -----------------------------------------------------------------------------
// Forward Declarations
// -----------------------------------------------------------------------------

uint8_t getIndexForDownloadingNodeId(EmberNodeId nodeId);

// -----------------------------------------------------------------------------
// APIs
// -----------------------------------------------------------------------------

// Returns MAX_DOWNLOADS if not found
uint8_t getIndexForDownloadingNodeId(EmberNodeId nodeId)
{
  uint8_t nodeIndex;
  for (nodeIndex = 0; nodeIndex < MAX_DOWNLOADS; nodeIndex++) {
    if (downloadingNodes[nodeIndex].nodeId == nodeId) {
      break;
    }
  }
  return nodeIndex;
}

void emAfOtaServerDynamicBlockPeriodInit()
{
  // Initialize array to have no current OTA downloads
  uint8_t nodeIndex;
  for (nodeIndex = 0; nodeIndex < MAX_DOWNLOADS; nodeIndex++) {
    downloadingNodes[nodeIndex].nodeId = EMBER_NULL_NODE_ID;
    downloadingNodes[nodeIndex].unitDiscoveryState = STATE_NONE;
    downloadingNodes[nodeIndex].msTickWhenLastSeen = 0;
  }
}

void emAfOtaServerDynamicBlockPeriodTick()
{
  uint8_t nodeIndex;
  uint32_t currentMsTick = halCommonGetInt32uMillisecondTick();
  uint32_t secondsElapsed;

  // Purge any inactive clients
  for (nodeIndex = 0; nodeIndex < MAX_DOWNLOADS; nodeIndex++) {
    if (downloadingNodes[nodeIndex].unitDiscoveryState != STATE_NONE) {
      secondsElapsed = (currentMsTick
                        - downloadingNodes[nodeIndex].msTickWhenLastSeen)
                       / MILLISECOND_TICKS_PER_SECOND;
      if (secondsElapsed >= EMBER_AF_PLUGIN_OTA_SERVER_DYNAMIC_CLIENT_TIMEOUT_SEC) {
        debugPrintln("OTA: purging 0x%2X from active downloads due to "
                     "inactivity",
                     downloadingNodes[nodeIndex].nodeId);
        downloadingNodes[nodeIndex].nodeId = EMBER_NULL_NODE_ID;
        downloadingNodes[nodeIndex].unitDiscoveryState = STATE_NONE;
        downloadingNodes[nodeIndex].msTickWhenLastSeen = 0;
      }
    }
  }
}

// This function is called from ota-server whenever a client sends an Image
// Block Request. This function either returns SUCCESS, which indicates that we
// already know this client and know how it's treating the Minimum Block
// Request, or WAIT_FOR_DATA, which means either we don't know how the client is
// treating the field or it's the first time we see this node and don't have
// room for it in the active OTA downloads array
uint8_t emAfOtaServerCheckDynamicBlockPeriodDownload(EmberAfImageBlockRequestCallbackStruct *data)
{
  uint8_t nodeIndex;
  uint32_t secondsElapsed;
  uint32_t threshold;
  uint32_t tolerance;

  // Do we already know this node and how it treats the Minimum Block Period?
  for (nodeIndex = 0; nodeIndex < MAX_DOWNLOADS; nodeIndex++) {
    if (data->source == downloadingNodes[nodeIndex].nodeId) {
      if (downloadingNodes[nodeIndex].unitDiscoveryState == STATE_IN_DISCOVERY) {
        // We told the client to delay some value. The client has queried again
        // and the delay is complete. We can now determine how the client treats
        // the Minimum Block Period field
        tolerance = EMBER_AF_PLUGIN_OTA_SERVER_TEST_BLOCK_PERIOD_VALUE >> TOLERANCE_VALUE_BITSHIFT;
        threshold = EMBER_AF_PLUGIN_OTA_SERVER_TEST_BLOCK_PERIOD_VALUE - tolerance;
        secondsElapsed = (halCommonGetInt32uMillisecondTick()
                          - downloadingNodes[nodeIndex].msTickWhenLastSeen)
                         / MILLISECOND_TICKS_PER_SECOND;
        if (secondsElapsed > threshold) {
          downloadingNodes[nodeIndex].unitDiscoveryState = STATE_USES_SECONDS;
          debugPrintln("OTA client 0x%2X uses seconds",
                       downloadingNodes[nodeIndex].nodeId);
        } else {
          downloadingNodes[nodeIndex].unitDiscoveryState = STATE_USES_MS;
          debugPrintln("OTA client 0x%2X uses milliseconds",
                       downloadingNodes[nodeIndex].nodeId);
        }
      }
      // If we're not in STATE_IN_DISCOVERY, then we already know if the client
      // uses ms or seconds
      downloadingNodes[nodeIndex].msTickWhenLastSeen =
        halCommonGetInt32uMillisecondTick();
      return EMBER_ZCL_STATUS_SUCCESS;
    }
  }

  // We have never seen this node. Search for a free entry
  for (nodeIndex = 0; nodeIndex < MAX_DOWNLOADS; nodeIndex++) {
    if (EMBER_NULL_NODE_ID == downloadingNodes[nodeIndex].nodeId) {
      break;
    }
  }

  if (MAX_DOWNLOADS != nodeIndex) {
    // Free entry found. Mark it off and send it a delay test
    downloadingNodes[nodeIndex].nodeId = data->source;
    downloadingNodes[nodeIndex].msTickWhenLastSeen = halCommonGetInt32uMillisecondTick();
    downloadingNodes[nodeIndex].unitDiscoveryState = STATE_IN_DISCOVERY;
    // Tell the node to delay some value. When we hear from it again, the time
    // elapsed will indicate whether it's treating the field as seconds or
    // milliseconds
    data->minBlockRequestPeriod = EMBER_AF_PLUGIN_OTA_SERVER_TEST_BLOCK_PERIOD_VALUE;
  } else {
    // We've never seen this node, and we don't have room for an active OTA
    // session. Tell it to delay for a while
    data->waitTimeSecondsResponse = LONG_DELAY_SECONDS;
  }

  return EMBER_ZCL_STATUS_WAIT_FOR_DATA;
}

// This function is called when a client sends an UpgradeEndRequest, signalling
// that is is complete with its OTA download
void emAfOtaServerCompleteDynamicBlockPeriodDownload(EmberNodeId clientId)
{
  uint8_t nodeIndex = getIndexForDownloadingNodeId(clientId);
  if (nodeIndex < MAX_DOWNLOADS) {
    downloadingNodes[nodeIndex].nodeId = EMBER_NULL_NODE_ID;
    downloadingNodes[nodeIndex].unitDiscoveryState = STATE_NONE;
    downloadingNodes[nodeIndex].msTickWhenLastSeen = 0;
  }
}

// NOTE: this should only be called once we've discovered how the client is
// treating the Minimum Block Period field
bool emAfOtaServerDynamicBlockPeriodClientUsesSeconds(EmberNodeId clientId)
{
  uint8_t nodeIndex = getIndexForDownloadingNodeId(clientId);

  // This function should never be called if discovery is not finished
  assert(nodeIndex < MAX_DOWNLOADS);
  assert(downloadingNodes[nodeIndex].unitDiscoveryState >= STATE_USES_SECONDS);

  return (downloadingNodes[nodeIndex].unitDiscoveryState == STATE_USES_SECONDS);
}

#endif // EMBER_AF_PLUGIN_OTA_SERVER_DYNAMIC_MIN_BLOCK_PERIOD_SUPPORT
