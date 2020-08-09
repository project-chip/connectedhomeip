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
 * @brief Routines for the Poll Control Server plugin, which implement the
 *        server side of the Poll Control cluster. The Poll Control cluster
 *        provides a means to communicate with an end device with a sleep
 *        schedule.
 *******************************************************************************
   ******************************************************************************/

#include "app/framework/include/af.h"

// This plugin does not synchronize attributes between endpoints and does not
// handle multi-network issues with regard to polling.  Because of this, it is
// limited to exactly one endpoint that implements the Poll Control cluster
// server.
#if EMBER_AF_POLL_CONTROL_CLUSTER_SERVER_ENDPOINT_COUNT != 1
  #error "The Poll Control Server plugin only supports one endpoint."
#endif

// The built-in cluster tick has hooks into the polling code and is therefore
// used to control both the temporary fast poll mode while waiting for
// CheckInResponse commands and the actual fast poll mode that follows one or
// more positive CheckInResponse commands.  The endpoint event is used to send
// the periodic CheckIn commands.
//
// When it is time to check in, a new fast poll period begins with the
// selection of clients.  The clients are determined by scanning the binding
// table for nodes bound to the local endpoint for the Poll Control server.
// A CheckIn command is sent to each client, up to the limit set in the plugin
// options.  After the CheckIn commands are sent, the plugin enters a temporary
// fast poll mode until either all clients send a CheckInResponse command or
// the check-in timeout expires.  If one or more clients requests fast polling,
// the plugin continues fast polling for the maximum requested duration.  If
// FastPollStop commands are received from any clients, the fast poll duration
// is adjusted so that it reflects the maximum duration requested by all active
// clients.  Once the requested duration for all clients is satisfied, fast
// polling ends.
//
// Note that if a required check in happens to coincide with an existing fast
// poll period, the current fast poll period is terminated, all existing
// clients are forgetten, and a new fast poll period begins with the selection
// of new clients and the sending of new CheckIn commands.

extern EmberEventControl emberAfPluginPollControlServerCheckInEndpointEventControls[];

typedef struct {
  uint8_t bindingIndex;
  uint16_t fastPollTimeoutQs;
} Client;
static Client clients[EMBER_AF_PLUGIN_POLL_CONTROL_SERVER_MAX_CLIENTS];
static bool ignoreNonTrustCenter = false;

enum {
  INITIAL = 0,
  WAITING = 1,
  POLLING = 2,
};
static uint8_t state = INITIAL;
static uint32_t fastPollStartTimeMs;

// Flags used to track trust center check in failures
static bool trustCenterCheckInRequestSent;
static bool trustCenterCheckInResponseReceived;
static uint8_t trustCenterCheckInFailureCount;

// The timeout option is in quarter seconds, but we use it in milliseconds.
#define CHECK_IN_TIMEOUT_DURATION_MS                             \
  (EMBER_AF_PLUGIN_POLL_CONTROL_SERVER_CHECK_IN_RESPONSE_TIMEOUT \
   * MILLISECOND_TICKS_PER_QUARTERSECOND)
#define NULL_INDEX 0xFF

static EmberAfStatus readServerAttribute(uint8_t endpoint,
                                         EmberAfAttributeId attributeId,
                                         const char * name,
                                         uint8_t *data,
                                         uint8_t size)
{
  EmberAfStatus status = emberAfReadServerAttribute(endpoint,
                                                    ZCL_POLL_CONTROL_CLUSTER_ID,
                                                    attributeId,
                                                    data,
                                                    size);
  if (status != EMBER_ZCL_STATUS_SUCCESS) {
    emberAfPollControlClusterPrintln("ERR: %ping %p 0x%x", "read", name, status);
  }
  return status;
}

static EmberAfStatus writeServerAttribute(uint8_t endpoint,
                                          EmberAfAttributeId attributeId,
                                          const char * name,
                                          uint8_t *data,
                                          EmberAfAttributeType type)
{
  EmberAfStatus status = emberAfWriteServerAttribute(endpoint,
                                                     ZCL_POLL_CONTROL_CLUSTER_ID,
                                                     attributeId,
                                                     data,
                                                     type);
  if (status != EMBER_ZCL_STATUS_SUCCESS) {
    emberAfPollControlClusterPrintln("ERR: %ping %p 0x%x", "writ", name, status);
  }
  return status;
}

static EmberStatus scheduleServerTick(uint8_t endpoint, uint32_t delayMs)
{
  return emberAfScheduleServerTickExtended(endpoint,
                                           ZCL_POLL_CONTROL_CLUSTER_ID,
                                           delayMs,
                                           EMBER_AF_SHORT_POLL,
                                           EMBER_AF_OK_TO_SLEEP);
}

static EmberStatus deactivateServerTick(uint8_t endpoint)
{
  return emberAfDeactivateServerTick(endpoint, ZCL_POLL_CONTROL_CLUSTER_ID);
}

static void scheduleCheckIn(uint8_t endpoint)
{
  EmberAfStatus status;
  uint32_t checkInIntervalQs;
  status = readServerAttribute(endpoint,
                               ZCL_CHECK_IN_INTERVAL_ATTRIBUTE_ID,
                               "check in interval",
                               (uint8_t *)&checkInIntervalQs,
                               sizeof(checkInIntervalQs));
  if (status == EMBER_ZCL_STATUS_SUCCESS && checkInIntervalQs != 0) {
    emberAfEndpointEventControlSetDelayMS(emberAfPluginPollControlServerCheckInEndpointEventControls,
                                          endpoint,
                                          (checkInIntervalQs
                                           * MILLISECOND_TICKS_PER_QUARTERSECOND));
  } else {
    emberAfEndpointEventControlSetInactive(emberAfPluginPollControlServerCheckInEndpointEventControls,
                                           endpoint);
  }
}

static uint8_t findClientIndex(void)
{
  EmberBindingTableEntry incomingBinding;
  uint8_t incomingBindingIndex = emberAfGetBindingIndex();
  if (emberGetBinding(incomingBindingIndex, &incomingBinding)
      == EMBER_SUCCESS) {
    uint8_t clientIndex;
    for (clientIndex = 0;
         clientIndex < EMBER_AF_PLUGIN_POLL_CONTROL_SERVER_MAX_CLIENTS;
         clientIndex++) {
      EmberBindingTableEntry clientBinding;
      if ((emberGetBinding(clients[clientIndex].bindingIndex, &clientBinding)
           == EMBER_SUCCESS)
          && incomingBinding.type == clientBinding.type
          && incomingBinding.local == clientBinding.local
          && incomingBinding.remote == clientBinding.remote
          && (MEMCOMPARE(incomingBinding.identifier,
                         clientBinding.identifier,
                         EUI64_SIZE)
              == 0)) {
        return clientIndex;
      }
    }
  }
  return NULL_INDEX;
}

static bool pendingCheckInResponses(void)
{
  uint8_t i;
  for (i = 0; i < EMBER_AF_PLUGIN_POLL_CONTROL_SERVER_MAX_CLIENTS; i++) {
    if (clients[i].bindingIndex != NULL_INDEX
        && clients[i].fastPollTimeoutQs == 0) {
      return true;
    }
  }
  return false;
}

static bool isPollControlBindingTrustCenter(uint8_t endpoint, uint8_t bindingIndex)
{
  EmberBindingTableEntry binding;
  if (emberGetBinding(bindingIndex, &binding) == EMBER_SUCCESS
      && binding.type == EMBER_UNICAST_BINDING
      && binding.local == endpoint
      && binding.clusterId == ZCL_POLL_CONTROL_CLUSTER_ID) {
    EmberEUI64 trustCenterEui64;
    if (emberLookupEui64ByNodeId(EMBER_TRUST_CENTER_NODE_ID, trustCenterEui64) != EMBER_SUCCESS) {
      return false;
    }

    if (0 == MEMCOMPARE(binding.identifier, trustCenterEui64, EUI64_SIZE)) {
      return true;
    }
  }
  return false;
}

static bool outstandingFastPollRequests(uint8_t endpoint)
{
  uint32_t currentTimeMs = halCommonGetInt32uMillisecondTick();
  uint32_t elapsedFastPollTimeMs = elapsedTimeInt32u(fastPollStartTimeMs,
                                                     currentTimeMs);
  uint16_t fastPollTimeoutQs = 0;
  uint8_t i;
  for (i = 0; i < EMBER_AF_PLUGIN_POLL_CONTROL_SERVER_MAX_CLIENTS; i++) {
    if (clients[i].bindingIndex != NULL_INDEX) {
      if (clients[i].fastPollTimeoutQs * MILLISECOND_TICKS_PER_QUARTERSECOND
          < elapsedFastPollTimeMs) {
        clients[i].bindingIndex = NULL_INDEX;
      } else if (fastPollTimeoutQs < clients[i].fastPollTimeoutQs) {
        fastPollTimeoutQs = clients[i].fastPollTimeoutQs;
      }
    }
  }

  if (fastPollTimeoutQs == 0) {
    return false;
  } else {
    uint32_t newFastPollEndTimeMs = (fastPollStartTimeMs
                                     + (fastPollTimeoutQs
                                        * MILLISECOND_TICKS_PER_QUARTERSECOND));
    uint32_t remainingFastPollTimeMs = elapsedTimeInt32u(currentTimeMs,
                                                         newFastPollEndTimeMs);
    scheduleServerTick(endpoint, remainingFastPollTimeMs);
    return true;
  }
}

static EmberAfStatus validateCheckInInterval(uint8_t endpoint,
                                             uint32_t newCheckInIntervalQs)
{
  EmberAfStatus status;
  uint32_t longPollIntervalQs;

  if (newCheckInIntervalQs == 0) {
    return EMBER_ZCL_STATUS_SUCCESS;
  }

  status = readServerAttribute(endpoint,
                               ZCL_LONG_POLL_INTERVAL_ATTRIBUTE_ID,
                               "long poll interval",
                               (uint8_t *)&longPollIntervalQs,
                               sizeof(longPollIntervalQs));
  if (status != EMBER_ZCL_STATUS_SUCCESS) {
    return status;
  } else if (newCheckInIntervalQs < longPollIntervalQs) {
    return EMBER_ZCL_STATUS_INVALID_VALUE;
  }

#ifdef ZCL_USING_POLL_CONTROL_CLUSTER_CHECK_IN_INTERVAL_MIN_ATTRIBUTE
  {
    uint32_t checkInIntervalMinQs;
    status = readServerAttribute(endpoint,
                                 ZCL_CHECK_IN_INTERVAL_MIN_ATTRIBUTE_ID,
                                 "check in interval min",
                                 (uint8_t *)&checkInIntervalMinQs,
                                 sizeof(checkInIntervalMinQs));
    if (status != EMBER_ZCL_STATUS_SUCCESS) {
      return status;
    } else if (newCheckInIntervalQs < checkInIntervalMinQs) {
      return EMBER_ZCL_STATUS_INVALID_VALUE;
    }
  }
#endif

  return EMBER_ZCL_STATUS_SUCCESS;
}

static EmberAfStatus validateLongPollInterval(uint8_t endpoint,
                                              uint32_t newLongPollIntervalQs)
{
  EmberAfStatus status;
  uint32_t checkInIntervalQs;
  uint16_t shortPollIntervalQs;

  status = readServerAttribute(endpoint,
                               ZCL_CHECK_IN_INTERVAL_ATTRIBUTE_ID,
                               "check in interval",
                               (uint8_t *)&checkInIntervalQs,
                               sizeof(checkInIntervalQs));
  if (status != EMBER_ZCL_STATUS_SUCCESS) {
    return status;
  } else if (checkInIntervalQs < newLongPollIntervalQs) {
    return EMBER_ZCL_STATUS_INVALID_VALUE;
  }

  status = readServerAttribute(endpoint,
                               ZCL_SHORT_POLL_INTERVAL_ATTRIBUTE_ID,
                               "short poll interval",
                               (uint8_t *)&shortPollIntervalQs,
                               sizeof(shortPollIntervalQs));
  if (status != EMBER_ZCL_STATUS_SUCCESS) {
    return status;
  } else if (newLongPollIntervalQs < shortPollIntervalQs) {
    return EMBER_ZCL_STATUS_INVALID_VALUE;
  }

#ifdef ZCL_USING_POLL_CONTROL_CLUSTER_LONG_POLL_INTERVAL_MIN_ATTRIBUTE
  {
    uint32_t longPollIntervalMinQs;
    status = readServerAttribute(endpoint,
                                 ZCL_LONG_POLL_INTERVAL_MIN_ATTRIBUTE_ID,
                                 "long poll interval min",
                                 (uint8_t *)&longPollIntervalMinQs,
                                 sizeof(longPollIntervalMinQs));
    if (status != EMBER_ZCL_STATUS_SUCCESS) {
      return status;
    } else if (newLongPollIntervalQs < longPollIntervalMinQs) {
      return EMBER_ZCL_STATUS_INVALID_VALUE;
    }
  }
#endif

  return EMBER_ZCL_STATUS_SUCCESS;
}

static EmberAfStatus validateShortPollInterval(uint8_t endpoint,
                                               uint16_t newShortPollIntervalQs)
{
  EmberAfStatus status;
  uint32_t longPollIntervalQs;
  status = readServerAttribute(endpoint,
                               ZCL_LONG_POLL_INTERVAL_ATTRIBUTE_ID,
                               "long poll interval",
                               (uint8_t *)&longPollIntervalQs,
                               sizeof(longPollIntervalQs));
  if (status != EMBER_ZCL_STATUS_SUCCESS) {
    return status;
  } else if (longPollIntervalQs < newShortPollIntervalQs) {
    return EMBER_ZCL_STATUS_INVALID_VALUE;
  }
  return EMBER_ZCL_STATUS_SUCCESS;
}

static EmberAfStatus validateFastPollInterval(uint8_t endpoint,
                                              uint16_t newFastPollIntervalQs)
{
#ifdef ZCL_USING_POLL_CONTROL_CLUSTER_FAST_POLL_TIMEOUT_MAX_ATTRIBUTE
  EmberAfStatus status;
  uint16_t fastPollTimeoutMaxQs;
  status = readServerAttribute(endpoint,
                               ZCL_FAST_POLL_TIMEOUT_MAX_ATTRIBUTE_ID,
                               "fast poll timeout max",
                               (uint8_t *)&fastPollTimeoutMaxQs,
                               sizeof(fastPollTimeoutMaxQs));
  if (status != EMBER_ZCL_STATUS_SUCCESS) {
    return status;
  } else if (fastPollTimeoutMaxQs < newFastPollIntervalQs) {
    return EMBER_ZCL_STATUS_INVALID_VALUE;
  }
#endif

  return EMBER_ZCL_STATUS_SUCCESS;
}

static EmberAfStatus validateCheckInIntervalMin(uint8_t endpoint,
                                                uint32_t newCheckInIntervalMinQs)
{
  EmberAfStatus status;
  uint32_t checkInIntervalQs;
  status = readServerAttribute(endpoint,
                               ZCL_CHECK_IN_INTERVAL_ATTRIBUTE_ID,
                               "check in interval",
                               (uint8_t *)&checkInIntervalQs,
                               sizeof(checkInIntervalQs));
  if (status != EMBER_ZCL_STATUS_SUCCESS) {
    return status;
  } else if (checkInIntervalQs < newCheckInIntervalMinQs) {
    return EMBER_ZCL_STATUS_INVALID_VALUE;
  }
  return EMBER_ZCL_STATUS_SUCCESS;
}

static EmberAfStatus validateLongPollIntervalMin(uint8_t endpoint,
                                                 uint32_t newLongPollIntervalMinQs)
{
  EmberAfStatus status;
  uint32_t longPollIntervalQs;
  status = readServerAttribute(endpoint,
                               ZCL_LONG_POLL_INTERVAL_ATTRIBUTE_ID,
                               "long poll interval",
                               (uint8_t *)&longPollIntervalQs,
                               sizeof(longPollIntervalQs));
  if (status != EMBER_ZCL_STATUS_SUCCESS) {
    return status;
  } else if (longPollIntervalQs < newLongPollIntervalMinQs) {
    return EMBER_ZCL_STATUS_INVALID_VALUE;
  }
  return EMBER_ZCL_STATUS_SUCCESS;
}

static EmberAfStatus validateFastPollTimeoutMax(uint8_t endpoint,
                                                uint16_t newFastPollTimeoutMaxQs)
{
  EmberAfStatus status;
  uint16_t fastPollTimeoutQs;
  status = readServerAttribute(endpoint,
                               ZCL_FAST_POLL_TIMEOUT_ATTRIBUTE_ID,
                               "fast poll timeout",
                               (uint8_t *)&fastPollTimeoutQs,
                               sizeof(fastPollTimeoutQs));
  if (status != EMBER_ZCL_STATUS_SUCCESS) {
    return status;
  } else if (newFastPollTimeoutMaxQs < fastPollTimeoutQs) {
    return EMBER_ZCL_STATUS_INVALID_VALUE;
  }
  return EMBER_ZCL_STATUS_SUCCESS;
}

void emberAfPollControlClusterServerInitCallback(uint8_t endpoint)
{
  uint32_t longPollIntervalQs;
  uint16_t shortPollIntervalQs;

  if (readServerAttribute(endpoint,
                          ZCL_LONG_POLL_INTERVAL_ATTRIBUTE_ID,
                          "long poll interval",
                          (uint8_t *)&longPollIntervalQs,
                          sizeof(longPollIntervalQs))
      == EMBER_ZCL_STATUS_SUCCESS) {
    emberAfSetLongPollIntervalQsCallback(longPollIntervalQs);
  }

  if (readServerAttribute(endpoint,
                          ZCL_SHORT_POLL_INTERVAL_ATTRIBUTE_ID,
                          "short poll interval",
                          (uint8_t *)&shortPollIntervalQs,
                          sizeof(shortPollIntervalQs))
      == EMBER_ZCL_STATUS_SUCCESS) {
    emberAfSetShortPollIntervalQsCallback(shortPollIntervalQs);
  }

  // TODO: Begin checking in after the network comes up instead of at startup.
  scheduleCheckIn(endpoint);
}

void emberAfPollControlClusterServerTickCallback(uint8_t endpoint)
{
  if (state == WAITING) {
    uint16_t fastPollTimeoutQs = 0;
    uint8_t i;

    if (trustCenterCheckInRequestSent && !trustCenterCheckInResponseReceived) {
      trustCenterCheckInFailureCount++;
      emberAfPollControlClusterPrintln("ERR: Poll control check in failure (%d of %d)",
                                       trustCenterCheckInFailureCount,
                                       EMBER_AF_PLUGIN_POLL_CONTROL_SERVER_TRUST_CENTER_CHECK_IN_FAILURE_THRESHOLD);
      if (trustCenterCheckInFailureCount >= EMBER_AF_PLUGIN_POLL_CONTROL_SERVER_TRUST_CENTER_CHECK_IN_FAILURE_THRESHOLD) {
        emberAfPollControlClusterPrintln("ERR: Trust Center check in failure threshold reached");
        emberAfPluginPollControlServerCheckInTimeoutCallback();
        trustCenterCheckInFailureCount = 0;
      }
    }

    for (i = 0; i < EMBER_AF_PLUGIN_POLL_CONTROL_SERVER_MAX_CLIENTS; i++) {
      if (clients[i].bindingIndex != NULL_INDEX
          && fastPollTimeoutQs < clients[i].fastPollTimeoutQs) {
        fastPollTimeoutQs = clients[i].fastPollTimeoutQs;
      }
    }

    if (fastPollTimeoutQs != 0) {
      state = POLLING;
      fastPollStartTimeMs = halCommonGetInt32uMillisecondTick();
      scheduleServerTick(endpoint,
                         (fastPollTimeoutQs
                          * MILLISECOND_TICKS_PER_QUARTERSECOND));
      return;
    }
  }

  state = INITIAL;
  deactivateServerTick(endpoint);
}

void emberAfPluginPollControlServerCheckInEndpointEventHandler(uint8_t endpoint)
{
  uint8_t bindingIndex, clientIndex;

  trustCenterCheckInRequestSent = false;
  trustCenterCheckInResponseReceived = false;
  for (clientIndex = 0;
       clientIndex < EMBER_AF_PLUGIN_POLL_CONTROL_SERVER_MAX_CLIENTS;
       clientIndex++) {
    clients[clientIndex].bindingIndex = EMBER_NULL_BINDING;
  }

  for (bindingIndex = 0, clientIndex = 0;
       (bindingIndex < EMBER_BINDING_TABLE_SIZE
        && clientIndex < EMBER_AF_PLUGIN_POLL_CONTROL_SERVER_MAX_CLIENTS);
       bindingIndex++) {
    EmberBindingTableEntry binding;
    if (emberGetBinding(bindingIndex, &binding) == EMBER_SUCCESS
        && binding.type == EMBER_UNICAST_BINDING
        && binding.local == endpoint
        && binding.clusterId == ZCL_POLL_CONTROL_CLUSTER_ID) {
      // If ignoreNonTrustCenter is true, then we only add
      // the trust center as a client.
      if (ignoreNonTrustCenter
          && (!isPollControlBindingTrustCenter(endpoint, bindingIndex))) {
        emberAfPollControlClusterPrintln("Ignoring poll control client with bindingIndex %d in search of TC",
                                         bindingIndex);
        continue;
      }

      emberAfFillCommandPollControlClusterCheckIn();
      if (emberAfSendCommandUnicast(EMBER_OUTGOING_VIA_BINDING, bindingIndex)
          == EMBER_SUCCESS) {
        clients[clientIndex].bindingIndex = bindingIndex;
        clients[clientIndex].fastPollTimeoutQs = 0;
        clientIndex++;
        if (isPollControlBindingTrustCenter(endpoint, bindingIndex)) {
          trustCenterCheckInRequestSent = true;
        }
      }
    }
  }

  if (clientIndex == 0) {
    state = INITIAL;
    deactivateServerTick(endpoint);
  } else {
    state = WAITING;
    scheduleServerTick(endpoint, CHECK_IN_TIMEOUT_DURATION_MS);
  }

  scheduleCheckIn(endpoint);
}

EmberAfStatus emberAfPollControlClusterServerPreAttributeChangedCallback(uint8_t endpoint,
                                                                         EmberAfAttributeId attributeId,
                                                                         EmberAfAttributeType attributeType,
                                                                         uint8_t size,
                                                                         uint8_t *value)
{
  switch (attributeId) {
    case ZCL_CHECK_IN_INTERVAL_ATTRIBUTE_ID:
    {
      uint32_t newCheckInIntervalQs;
      MEMMOVE(&newCheckInIntervalQs, value, size);
      return validateCheckInInterval(endpoint, newCheckInIntervalQs);
    }
    case ZCL_LONG_POLL_INTERVAL_ATTRIBUTE_ID:
    {
      uint32_t newLongPollIntervalQs;
      MEMMOVE(&newLongPollIntervalQs, value, size);
      return validateLongPollInterval(endpoint, newLongPollIntervalQs);
    }
    case ZCL_SHORT_POLL_INTERVAL_ATTRIBUTE_ID:
    {
      uint16_t newShortPollIntervalQs;
      MEMMOVE(&newShortPollIntervalQs, value, size);
      return validateShortPollInterval(endpoint, newShortPollIntervalQs);
    }
    case ZCL_FAST_POLL_TIMEOUT_ATTRIBUTE_ID:
    {
      uint16_t newFastPollIntervalQs;
      MEMMOVE(&newFastPollIntervalQs, value, size);
      return validateFastPollInterval(endpoint, newFastPollIntervalQs);
    }
    case ZCL_CHECK_IN_INTERVAL_MIN_ATTRIBUTE_ID:
    {
      uint32_t newCheckInIntervalMinQs;
      MEMMOVE(&newCheckInIntervalMinQs, value, size);
      return validateCheckInIntervalMin(endpoint, newCheckInIntervalMinQs);
    }
    case ZCL_LONG_POLL_INTERVAL_MIN_ATTRIBUTE_ID:
    {
      uint32_t newLongPollIntervalMinQs;
      MEMMOVE(&newLongPollIntervalMinQs, value, size);
      return validateLongPollIntervalMin(endpoint, newLongPollIntervalMinQs);
    }
    case ZCL_FAST_POLL_TIMEOUT_MAX_ATTRIBUTE_ID:
    {
      uint32_t newFastPollTimeoutMaxQs;
      MEMMOVE(&newFastPollTimeoutMaxQs, value, size);
      return validateFastPollTimeoutMax(endpoint, newFastPollTimeoutMaxQs);
    }
    default:
      return EMBER_ZCL_STATUS_SUCCESS;
  }
}

void emberAfPollControlClusterServerAttributeChangedCallback(uint8_t endpoint,
                                                             EmberAfAttributeId attributeId)
{
  switch (attributeId) {
    case ZCL_CHECK_IN_INTERVAL_ATTRIBUTE_ID:
      scheduleCheckIn(endpoint);
      break;
    case ZCL_LONG_POLL_INTERVAL_ATTRIBUTE_ID:
    {
      EmberAfStatus status;
      uint32_t longPollIntervalQs;
      status = readServerAttribute(endpoint,
                                   ZCL_LONG_POLL_INTERVAL_ATTRIBUTE_ID,
                                   "long poll interval",
                                   (uint8_t *)&longPollIntervalQs,
                                   sizeof(longPollIntervalQs));
      if (status == EMBER_ZCL_STATUS_SUCCESS) {
        emberAfSetLongPollIntervalQsCallback(longPollIntervalQs);
      }
      break;
    }
    case ZCL_SHORT_POLL_INTERVAL_ATTRIBUTE_ID:
    {
      EmberAfStatus status;
      uint16_t shortPollIntervalQs;
      status = readServerAttribute(endpoint,
                                   ZCL_SHORT_POLL_INTERVAL_ATTRIBUTE_ID,
                                   "short poll interval",
                                   (uint8_t *)&shortPollIntervalQs,
                                   sizeof(shortPollIntervalQs));
      if (status == EMBER_ZCL_STATUS_SUCCESS) {
        emberAfSetShortPollIntervalQsCallback(shortPollIntervalQs);
      }
      break;
    }
    case ZCL_FAST_POLL_TIMEOUT_ATTRIBUTE_ID:
    case ZCL_CHECK_IN_INTERVAL_MIN_ATTRIBUTE_ID:
    case ZCL_LONG_POLL_INTERVAL_MIN_ATTRIBUTE_ID:
    case ZCL_FAST_POLL_TIMEOUT_MAX_ATTRIBUTE_ID:
    default:
      break;
  }
}

bool emberAfPollControlClusterCheckInResponseCallback(uint8_t startFastPolling,
                                                      uint16_t fastPollTimeoutQs)
{
  EmberAfStatus status = EMBER_ZCL_STATUS_ACTION_DENIED;
  uint8_t clientIndex = findClientIndex();

  emberAfPollControlClusterPrintln("RX: CheckInResponse 0x%x, 0x%2x",
                                   startFastPolling,
                                   fastPollTimeoutQs);

  // clientIndex will always be less than EMBER_AF_PLUGIN_POLL_CONTROL_SERVER_MAX_CLIENTS
  if (clientIndex != NULL_INDEX
      && clientIndex < EMBER_AF_PLUGIN_POLL_CONTROL_SERVER_MAX_CLIENTS) {
    if (state == WAITING) {
      uint8_t endpoint = emberAfCurrentEndpoint();

      if (isPollControlBindingTrustCenter(endpoint, clients[clientIndex].bindingIndex)) {
        trustCenterCheckInResponseReceived = true;
        if (trustCenterCheckInFailureCount > 0) {
          emberAfPollControlClusterPrintln("Poll Control: trust center "
                                           "responding to checkins after %d"
                                           "failure%s",
                                           trustCenterCheckInFailureCount,
                                           trustCenterCheckInFailureCount == 1
                                           ? "" : "s");
        }
        trustCenterCheckInFailureCount = 0;
      }

      if (startFastPolling) {
        if (fastPollTimeoutQs == 0) {
          status = readServerAttribute(endpoint,
                                       ZCL_FAST_POLL_TIMEOUT_ATTRIBUTE_ID,
                                       "fast poll timeout",
                                       (uint8_t *)&fastPollTimeoutQs,
                                       sizeof(fastPollTimeoutQs));
        } else {
          status = validateFastPollInterval(endpoint, fastPollTimeoutQs);
        }
        if (status == EMBER_ZCL_STATUS_SUCCESS) {
          clients[clientIndex].fastPollTimeoutQs = fastPollTimeoutQs;
        } else {
          clients[clientIndex].bindingIndex = NULL_INDEX;
        }
      } else {
        status = EMBER_ZCL_STATUS_SUCCESS;
        clients[clientIndex].bindingIndex = NULL_INDEX;
      }

      // Calling the tick directly when in the waiting state will cause the
      // temporarily fast poll mode to stop and will begin the actual fast poll
      // mode if applicable.
      if (!pendingCheckInResponses()) {
        emberAfPollControlClusterServerTickCallback(endpoint);
      }
    } else {
      status = EMBER_ZCL_STATUS_TIMEOUT;
    }
  }

  emberAfSendImmediateDefaultResponse(status);
  return true;
}

bool emberAfPollControlClusterFastPollStopCallback(void)
{
  EmberAfStatus status = EMBER_ZCL_STATUS_ACTION_DENIED;
  uint8_t clientIndex = findClientIndex();

  emberAfPollControlClusterPrintln("RX: FastPollStop");

  // clientIndex will always be less than EMBER_AF_PLUGIN_POLL_CONTROL_SERVER_MAX_CLIENTS
  if (clientIndex != NULL_INDEX
      && clientIndex < EMBER_AF_PLUGIN_POLL_CONTROL_SERVER_MAX_CLIENTS) {
    if (state == POLLING) {
      uint8_t endpoint = emberAfCurrentEndpoint();
      status = EMBER_ZCL_STATUS_SUCCESS;
      clients[clientIndex].bindingIndex = NULL_INDEX;

      // Calling the tick directly in the polling state will cause the fast
      // poll mode to stop.
      if (!outstandingFastPollRequests(endpoint)) {
        emberAfPollControlClusterServerTickCallback(endpoint);
      }
    } else {
      status = EMBER_ZCL_STATUS_TIMEOUT;
    }
  }

  emberAfSendImmediateDefaultResponse(status);
  return true;
}

bool emberAfPollControlClusterSetLongPollIntervalCallback(uint32_t newLongPollIntervalQs)
{
#ifdef EMBER_AF_PLUGIN_POLL_CONTROL_SERVER_ACCEPT_SET_LONG_POLL_INTERVAL_COMMAND
  EmberAfStatus status;
  uint8_t endpoint = emberAfCurrentEndpoint();

  emberAfPollControlClusterPrintln("RX: SetLongPollInterval 0x%4x",
                                   newLongPollIntervalQs);

  // Trying to write the attribute will trigger the PreAttributeChanged
  // callback, which will handle validation.  If the write is successful, the
  // AttributeChanged callback will fire, which will handle setting the new
  // long poll interval.
  status = writeServerAttribute(endpoint,
                                ZCL_LONG_POLL_INTERVAL_ATTRIBUTE_ID,
                                "long poll interval",
                                (uint8_t *)&newLongPollIntervalQs,
                                ZCL_INT32U_ATTRIBUTE_TYPE);

  emberAfSendImmediateDefaultResponse(status);
  return true;
#else
  return false;
#endif
}

bool emberAfPollControlClusterSetShortPollIntervalCallback(uint16_t newShortPollIntervalQs)
{
#ifdef EMBER_AF_PLUGIN_POLL_CONTROL_SERVER_ACCEPT_SET_SHORT_POLL_INTERVAL_COMMAND
  EmberAfStatus status;
  uint8_t endpoint = emberAfCurrentEndpoint();

  emberAfPollControlClusterPrintln("RX: SetShortPollInterval 0x%2x",
                                   newShortPollIntervalQs);

  // Trying to write the attribute will trigger the PreAttributeChanged
  // callback, which will handle validation.  If the write is successful, the
  // AttributeChanged callback will fire, which will handle setting the new
  // short poll interval.
  status = writeServerAttribute(endpoint,
                                ZCL_SHORT_POLL_INTERVAL_ATTRIBUTE_ID,
                                "short poll interval",
                                (uint8_t *)&newShortPollIntervalQs,
                                ZCL_INT16U_ATTRIBUTE_TYPE);

  emberAfSendImmediateDefaultResponse(status);
  return true;
#else
  return false;
#endif
}

void emAfPluginPollControlServerResetAttributesCallback(uint8_t endpointId)
{
  // EMAPPFWKV2-1437: when we reset our attributes, we need to re-sync with
  // the consumers of our attribute values. For example, the consumers of
  // emberAfSetLongPollIntervalQsCallback and
  // emberAfSetShortPollIntervalQsCallback will want to know that the poll
  // intervals might have changed. Therefore, we simply call the init function
  // to read the attribute values and notify interested parties.
  emberAfPollControlClusterServerInitCallback(endpointId);
}

void emberAfPluginPollControlServerStackStatusCallback(EmberStatus status)
{
  // Reset failure count if just joining network
  if (status == EMBER_NETWORK_UP) {
    trustCenterCheckInFailureCount = 0;
  }
}

void emberAfPluginPollControlServerSetIgnoreNonTrustCenter(bool ignoreNonTc)
{
  ignoreNonTrustCenter = ignoreNonTc;
}

bool emberAfPluginPollControlServerGetIgnoreNonTrustCenter(void)
{
  return ignoreNonTrustCenter;
}
