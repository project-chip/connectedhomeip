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
 * @brief A library for retrieving Ember stack counters over the air.
 *******************************************************************************
   ******************************************************************************/

#include "app/framework/include/af.h"
#include "app/util/serial/command-interpreter2.h"
#include "app/util/common/common.h"
#include "counters.h"
#include "counters-ota.h"

//Include counters-ota if enabled by the plugin
#if defined(EMBER_AF_PLUGIN_COUNTERS_OTA)

static EmberStatus sendCluster(EmberNodeId destination,
                               uint16_t clusterId,
                               EmberMessageBuffer payload)
{
  EmberApsFrame apsFrame;
  apsFrame.profileId = EMBER_PRIVATE_PROFILE_ID;
  apsFrame.clusterId = clusterId;
  apsFrame.sourceEndpoint = 0;
  apsFrame.destinationEndpoint = 0;
  apsFrame.options = (EMBER_APS_OPTION_RETRY
                      | EMBER_APS_OPTION_ENABLE_ROUTE_DISCOVERY);
  return emberSendUnicast(EMBER_OUTGOING_DIRECT,
                          destination,
                          &apsFrame,
                          payload);
}

EmberStatus emberAfPluginCountersSendRequest(EmberNodeId destination,
                                             bool clearCounters)
{
  return sendCluster(destination,
                     (clearCounters
                      ? EMBER_REPORT_AND_CLEAR_COUNTERS_REQUEST
                      : EMBER_REPORT_COUNTERS_REQUEST),
                     EMBER_NULL_MESSAGE_BUFFER);
}

bool emberAfPluginCountersIsIncomingRequest(EmberApsFrame *apsFrame, EmberNodeId sender)
{
  EmberMessageBuffer reply = EMBER_NULL_MESSAGE_BUFFER;
  uint8_t length = 0;
  uint8_t i;
  uint8_t temp[3];

  if (apsFrame->profileId != EMBER_PRIVATE_PROFILE_ID
      || (apsFrame->clusterId != EMBER_REPORT_COUNTERS_REQUEST
          && apsFrame->clusterId != EMBER_REPORT_AND_CLEAR_COUNTERS_REQUEST)) {
    return false;
  }

  for (i = 0; i < EMBER_COUNTER_TYPE_COUNT; i++) {
    uint16_t val = emberCounters[i];
    if (val != 0) {
      temp[0] = i;
      temp[1] = LOW_BYTE(val);
      temp[2] = HIGH_BYTE(val);
      if (reply == EMBER_NULL_MESSAGE_BUFFER) {
        reply = emberAllocateStackBuffer();
        if (reply == EMBER_NULL_MESSAGE_BUFFER) {
          return true;
        }
      }
      if (emberAppendToLinkedBuffers(reply, temp, 3) != EMBER_SUCCESS) {
        emberReleaseMessageBuffer(reply);
        return true;
      }
      length += 3;
    }
    if (length + 3 > MAX_PAYLOAD_LENGTH
        || (length
            && (i + 1 == EMBER_COUNTER_TYPE_COUNT))) {
      // The response cluster is the request id with the high bit set.
      sendCluster(sender, apsFrame->clusterId | 0x8000, reply);
      emberReleaseMessageBuffer(reply);
      reply = EMBER_NULL_MESSAGE_BUFFER;
      length = 0;
    }
  }

  return true;
}

bool emberAfPluginCountersIsIncomingResponse(EmberApsFrame *apsFrame)
{
  return (apsFrame->profileId == EMBER_PRIVATE_PROFILE_ID
          && (apsFrame->clusterId == EMBER_REPORT_AND_CLEAR_COUNTERS_RESPONSE
              || apsFrame->clusterId == EMBER_REPORT_COUNTERS_RESPONSE));
}

bool emberAfPluginCountersIsOutgoingResponse(EmberApsFrame *apsFrame,
                                             EmberStatus status)
{
  bool isResponse = emberIsIncomingCountersResponse(apsFrame);
  if (isResponse
      && apsFrame->clusterId == EMBER_REPORT_AND_CLEAR_COUNTERS_RESPONSE
      && status == EMBER_SUCCESS) {
    emberClearCounters();
  }
  return isResponse;
}

#endif //EMBER_AF_PLUGIN_COUNTERS_COUNTERSOTA
