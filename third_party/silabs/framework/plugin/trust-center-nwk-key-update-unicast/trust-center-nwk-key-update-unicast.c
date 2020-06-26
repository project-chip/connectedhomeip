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
 * @brief Functionality for rolling the network key by unicasting the key update
 * to all devices in the Trust Center's link key table with authorized keys.
 * This is the preferred mechanism for Smart Energy.
 * This does not work for devices using a global link key.
 *
 * The process is a little more complex than a broadcast key update:
 *   1. Broadcast a Many-to-one Route Record (advertisement) to insure
 *      devices will send route record messages to the TC so that we have
 *      the latest routes.
 *   2. For each ROUTER entry in the key-table (sleepies are assumed to
 *      always miss key updates, so we don't bother):
 *      A. Broadcast a ZDO address discovery for the target device's short
 *         address.
 *      B. If we receive a response, they will also send a Route Record
 *         containing route information AND long->short ID mapping.
 *         In that case send a unicast NWK key update to the device.
 *         If we don't get any response, skip them.
 *   3. Broadcast NWK key switch
 *
 * Note: This does not PERIODICALLY update the NWK key.  It just manages the
 * process when it is told to initate the key change.  Another software
 * module must determine the policy for periodically initiating this process
 * (such as the 'Trust Center NWK Key Update Periodic' App Framework Plugin).
 *******************************************************************************
   ******************************************************************************/

#include "app/framework/include/af.h"
#include "app/framework/util/common.h"
#include "app/framework/util/util.h"

#include "app/framework/util/af-main.h"

#include "trust-center-nwk-key-update-unicast.h"

#include "app/framework/plugin/concentrator/concentrator-support.h"

// *****************************************************************************
// Globals

enum {
  KEY_UPDATE_NONE,
  BROADCAST_MTORR,
  TRAVERSE_KEY_TABLE,
  DISCOVERING_NODE_ID,
  SENDING_KEY_UPDATES,
  BROADCAST_KEY_SWITCH,
};
typedef uint8_t KeyUpdateStateId;

#define KEY_UPDATE_STATE_STRINGS { \
    "None",                        \
    "Broadcast MTORR",             \
    "Traverse Key Table",          \
    "Discovering Node Id",         \
    "Unicasting Key Updates",      \
    "Broadcast Key Switch",        \
}

#if defined(EMBER_AF_PRINT_ENABLE) && defined(EMBER_AF_PRINT_SECURITY)
const char * keyUpdateStateStrings[] = KEY_UPDATE_STATE_STRINGS;
#endif

#define KEY_MASK (EMBER_KEY_IS_AUTHORIZED \
                  | EMBER_KEY_PARTNER_IS_SLEEPY)

enum {
  OPERATION_START   = 0,
  OPERATION_FAILED  = 1,
  TIMER_EXPIRED     = 2,
};
typedef uint8_t KeyUpdateResult;

typedef void (KeyUpdateFunction)(KeyUpdateResult);

typedef struct {
  KeyUpdateFunction* function;
  KeyUpdateStateId stateId;
} KeyUpdateState;

EmberEventControl emberAfPluginTrustCenterNwkKeyUpdateUnicastMyEventControl;
#define myEvent emberAfPluginTrustCenterNwkKeyUpdateUnicastMyEventControl

static KeyUpdateStateId currentStateId = KEY_UPDATE_NONE;

#define EXTRA_MTORR_DELAY_QS 4

static int16_t keyTableIndex = -1;
static EmberNodeId discoveredNodeId;

// This is used as a per-device failure count for ZDO discovery, and a single
// failure count for broadcasting the key-switch
static uint8_t failureCount = 0;

#define FAILURE_COUNT_THRESHOLD 3
#define ZDO_DELAY_AFTER_FAILURE_QS 4
#define SEND_KEY_FAILURE_DELAY_QS  4
#define KEY_UPDATE_DELAY_QS        4
#define BROADCAST_KEY_SWITCH_DELAY_QS 4
#define BROADCAST_KEY_SWITCH_DELAY_AFTER_FAILURE_QS 4

#if defined EMBER_TEST
  #define EMBER_TEST_PUBLIC
#else
  #define EMBER_TEST_PUBLIC static
#endif

#if defined(EMBER_AF_PLUGIN_TEST_HARNESS)
// For testing, we need to support a single application that can do
// unicast AND broadcast key updates.  So we re-map the function name
// so both can be compiled in without conflict.
  #define emberAfTrustCenterStartNetworkKeyUpdate emberAfTrustCenterStartUnicastNetworkKeyUpdate
#endif

// *****************************************************************************
// State Machine

static void broadcastMtorr(KeyUpdateResult status);
static void traverseKeyTable(KeyUpdateResult status);
static void zdoDiscovery(KeyUpdateResult status);
static void sendKeyUpdate(KeyUpdateResult status);
static void broadcastKeySwitch(KeyUpdateResult status);

static const KeyUpdateState stateTable[] = {
  { NULL, KEY_UPDATE_NONE },
  { broadcastMtorr, BROADCAST_MTORR },
  { traverseKeyTable, TRAVERSE_KEY_TABLE },
  { zdoDiscovery, DISCOVERING_NODE_ID },
  { sendKeyUpdate, SENDING_KEY_UPDATES },
  { broadcastKeySwitch, BROADCAST_KEY_SWITCH },
};

// *****************************************************************************
// Forward Declarations

static void keyUpdateGotoState(KeyUpdateStateId state);
#define keyUpdateGotoNextState() keyUpdateGotoState(currentStateId + 1)

// *****************************************************************************
// Functions

void emberAfPluginTrustCenterNwkKeyUpdateUnicastMyEventHandler(void)
{
  emberEventControlSetInactive(myEvent);
  if (stateTable[currentStateId].function != NULL) {
    stateTable[currentStateId].function(TIMER_EXPIRED);
  }
}

static void keyUpdateGotoStateAfterDelay(KeyUpdateStateId stateId, uint32_t delayQs)
{
  currentStateId = stateId;
  emberAfSecurityFlush();
  emberAfSecurityPrintln("Delaying %l sec. before going to state: %s",
                         delayQs >> 2,
                         keyUpdateStateStrings[currentStateId]);
  emberAfSecurityFlush();
  emberEventControlSetDelayQS(myEvent, delayQs);
}

static void keyUpdateGotoState(KeyUpdateStateId stateId)
{
  currentStateId = stateId;
  emberAfSecurityPrintln("Key Update State: %s",
                         keyUpdateStateStrings[currentStateId]);
  stateTable[currentStateId].function(OPERATION_START);
}

static void broadcastMtorr(KeyUpdateResult result)
{
  uint16_t delayQs;

  if (result == TIMER_EXPIRED) {
    keyUpdateGotoNextState();
    return;
  }

  delayQs = emberAfPluginConcentratorQueueDiscovery() + EXTRA_MTORR_DELAY_QS;
  emberAfSecurityPrintln("NWK Key Update: Sending MTORR in %d sec.",
                         delayQs >> 2);
  emberEventControlSetDelayQS(myEvent, delayQs);
}

static void traverseKeyTable(KeyUpdateResult result)
{
  keyTableIndex++;

  for (; keyTableIndex < emberAfGetKeyTableSize(); keyTableIndex++) {
    EmberKeyStruct keyStruct;
    EmberStatus status = emberGetKeyTableEntry(keyTableIndex,
                                               &keyStruct);
    if (status == EMBER_SUCCESS) {
      if ((keyStruct.bitmask & KEY_MASK)
          == EMBER_KEY_IS_AUTHORIZED) {
        emberAfSecurityPrintln("Updating NWK key at key table index %d",
                               keyTableIndex);
        keyUpdateGotoNextState();
        return;
      } else {
        emberAfSecurityPrintln("Skipping key table index %d (unauthorized key or sleepy child)",
                               keyTableIndex);
      }
    }
  }
  emberAfSecurityPrintln("Finishing traversing key table.");
  currentStateId = BROADCAST_KEY_SWITCH;
  emberEventControlSetDelayQS(myEvent,
                              BROADCAST_KEY_SWITCH_DELAY_QS);
}

EMBER_TEST_PUBLIC void zdoDiscoveryCallback(const EmberAfServiceDiscoveryResult* result)
{
  if (result->status == EMBER_AF_BROADCAST_SERVICE_DISCOVERY_RESPONSE_RECEIVED
      || result->status == EMBER_AF_BROADCAST_SERVICE_DISCOVERY_COMPLETE_WITH_RESPONSE) {
    discoveredNodeId = result->matchAddress;
    emberAfSecurityPrintln("Key Table index %d is node ID 0x%2X",
                           keyTableIndex,
                           discoveredNodeId);
    keyUpdateGotoNextState();
  } else if (discoveredNodeId != EMBER_NULL_NODE_ID) {
    // Do nothing, this is just a callback telling us this is the end of the discovery.
    // Since we have received the node ID by now, we can safely ignore this callback.
  } else {
    emberAfSecurityPrintln("Could not find node ID for key table entry %d",
                           keyTableIndex);
    zdoDiscovery(OPERATION_FAILED);
  }
}

static EmberStatus getEui64OfCurrentKeyTableIndex(EmberEUI64 returnEui64)
{
  EmberKeyStruct linkKey;
  EmberStatus status = emberGetKeyTableEntry(keyTableIndex, &linkKey);

  if (status == EMBER_SUCCESS) {
    MEMMOVE(returnEui64, linkKey.partnerEUI64, EUI64_SIZE);
  }
  return status;
}

static void zdoDiscovery(KeyUpdateResult result)
{
  EmberEUI64 longId;
  EmberStatus status;

  discoveredNodeId = EMBER_NULL_NODE_ID;

  if (result == OPERATION_FAILED) {
    failureCount++;
    if (failureCount >= FAILURE_COUNT_THRESHOLD) {
      goto zdoDiscoveryFail;
    }
  }

  if (EMBER_SUCCESS == getEui64OfCurrentKeyTableIndex(longId)) {
    status = emberAfFindNodeId(longId, zdoDiscoveryCallback);
    emberAfSecurityPrintln("Discovering node ID for key table %d",
                           keyTableIndex);
    if (status != EMBER_SUCCESS) {
      failureCount++;
      emberAfSecurityPrintln("Failed to start Node ID dsc.  Failure count: %d",
                             failureCount);
    } else {
      return;
    }
  } else {
    emberAfSecurityPrintln("Failed to get EUI64 of current key table entry (%d).  Skipping.",
                           keyTableIndex);
    failureCount = FAILURE_COUNT_THRESHOLD;
  }

  zdoDiscoveryFail:
  if (failureCount >= FAILURE_COUNT_THRESHOLD) {
    emberAfSecurityPrintln("Maximum error count reached (%d), skipping key table entry %d",
                           FAILURE_COUNT_THRESHOLD,
                           keyTableIndex);
    traverseKeyTable(OPERATION_FAILED);
  } else {
    emberEventControlSetDelayQS(myEvent,
                                ZDO_DELAY_AFTER_FAILURE_QS);
  }
}

static bool nextNetworkKeyIsNewer(EmberKeyStruct* nextNwkKey)
{
  EmberKeyStruct currentNwkKey;
  EmberStatus status;

  // It is assumed that the current nwk key has valid data.
  emberGetKey(EMBER_CURRENT_NETWORK_KEY,
              &currentNwkKey);

  status = emberGetKey(EMBER_NEXT_NETWORK_KEY,
                       nextNwkKey);
  if (status != EMBER_SUCCESS
      || (timeGTorEqualInt8u(currentNwkKey.sequenceNumber,
                             nextNwkKey->sequenceNumber))) {
    return false;
  }

  return true;
}

static void abortKeyUpdate(EmberStatus status)
{
  emberAfSecurityPrintln("Key Update %p (0x%X)",
                         (status == EMBER_SUCCESS
                          ? "complete"
                          : "aborted"),
                         status);
  currentStateId = KEY_UPDATE_NONE;
  keyTableIndex = -1;
  failureCount = 0;
  emberAfNetworkKeyUpdateCompleteCallback(status);
}

static void sendKeyUpdate(KeyUpdateResult result)
{
  EmberKeyStruct nextNwkKey;
  EmberStatus status;

  if (result == OPERATION_START) {
    failureCount = 0;
  }

  if (!nextNetworkKeyIsNewer(&nextNwkKey)) {
    // Setting the key to all zeroes tells the stack
    // to randomly generate a new key and use that.
    MEMSET(emberKeyContents(&(nextNwkKey.key)),
           0,
           EMBER_ENCRYPTION_KEY_SIZE);
  }

  {
    EmberEUI64 eui64;
    if (EMBER_SUCCESS != getEui64OfCurrentKeyTableIndex(eui64)) {
      emberAfSecurityPrintln("Failed to get EUI64 of index %d",
                             keyTableIndex);
      return;
    }
    emberAfSecurityPrintln("Sending NWK Key update to 0x%2X",
                           discoveredNodeId);

    status = emberSendUnicastNetworkKeyUpdate(discoveredNodeId,
                                              eui64,
                                              &(nextNwkKey.key));
  }

  if (status != EMBER_SUCCESS) {
    failureCount++;
    emberAfSecurityPrintln("Failed to unicast NWK key update (%d).  Failure count: %d",
                           status,
                           failureCount);
    if (failureCount >= FAILURE_COUNT_THRESHOLD) {
      emberAfSecurityPrintln("Maximum failure count hit (%d) for sending key update, aborting.",
                             FAILURE_COUNT_THRESHOLD);
      abortKeyUpdate(status);
      return;
    }

    emberEventControlSetDelayQS(myEvent,
                                SEND_KEY_FAILURE_DELAY_QS);
  } else {
    keyUpdateGotoStateAfterDelay(TRAVERSE_KEY_TABLE,
                                 KEY_UPDATE_DELAY_QS);
  }
}

static void broadcastKeySwitch(KeyUpdateResult result)
{
  EmberStatus status;
  if (result != TIMER_EXPIRED) {
    failureCount = 0;
  }
  status = emberBroadcastNetworkKeySwitch();
  if (EMBER_SUCCESS != status) {
    failureCount++;
    emberAfSecurityPrintln("Failed to broadcast key switch, failures: %d.  Will retry.",
                           failureCount);
    if (failureCount >= FAILURE_COUNT_THRESHOLD) {
      emberAfSecurityPrintln("Max fail count hit (%d), aborting key update.");
    } else {
      emberEventControlSetDelayQS(myEvent,
                                  BROADCAST_KEY_SWITCH_DELAY_AFTER_FAILURE_QS);
    }
  }
  emberAfSecurityPrintln("Sent NWK key switch.");

  abortKeyUpdate(status);
}

EmberStatus emberAfTrustCenterStartNetworkKeyUpdate(void)
{
  if (currentStateId != KEY_UPDATE_NONE
      || emberAfNetworkState() != EMBER_JOINED_NETWORK
      || emberAfGetNodeId() != EMBER_TRUST_CENTER_NODE_ID) {
    return EMBER_INVALID_CALL;
  }

  keyUpdateGotoNextState();
  return EMBER_SUCCESS;
}
