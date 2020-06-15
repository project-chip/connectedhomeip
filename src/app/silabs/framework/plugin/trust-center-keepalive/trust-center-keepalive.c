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
 * @brief Routines for the Trust Center Keepalive plugin, which implements a
 *        mechanism for ensuring the trust center is responding on the network.
 *******************************************************************************
   ******************************************************************************/

#include "app/framework/include/af.h"
#include "app/framework/util/common.h"
#include "trust-center-keepalive.h"

//------------------------------------------------------------------------------
// Globals

#if defined(EMBER_SCRIPTED_TEST)
  #define EMBER_AF_PLUGIN_TRUST_CENTER_KEEPALIVE_DEFAULT_JITTER_PERIOD_SECONDS  60
  #define EMBER_AF_PLUGIN_TRUST_CENTER_KEEPALIVE_DEFAULT_BASE_PERIOD_MINUTES    3
#endif

typedef enum {
  STATE_DISABLED,
  STATE_NONE,
  STATE_DISCOVER_ENDPOINT,
  STATE_SEND_KEEPALIVE_SIGNAL,
  STATE_INITIATE_NETWORK_SEARCH,
} TrustCenterKeepaliveState;

extern EmberEventControl emberAfPluginTrustCenterKeepaliveTickNetworkEventControls[];

typedef struct {
  uint32_t timeOfLastResponse;
  uint16_t jitterTimeSeconds;
  uint16_t  baseTimeSeconds;
  uint8_t failures;
  uint8_t destinationEndpoint;
  bool waitingForResponse;
  TrustCenterKeepaliveState state;
  uint8_t lastResponseZclStatus;
  bool tcHasKeepAliveServerCluster;
} KeepaliveStatusStruct;

static KeepaliveStatusStruct keepaliveStatusArray[EMBER_SUPPORTED_NETWORKS];

#define keepaliveIsEnabled(statusStruct) (statusStruct->state != STATE_DISABLED)

#define KEEPALIVE_WAIT_TIME_MS (5 * MILLISECOND_TICKS_PER_SECOND)

const KeepaliveStatusStruct defaultStruct = {
  0xFFFFFFFFUL,
  EMBER_AF_PLUGIN_TRUST_CENTER_KEEPALIVE_DEFAULT_JITTER_PERIOD_SECONDS,
  EMBER_AF_PLUGIN_TRUST_CENTER_KEEPALIVE_DEFAULT_BASE_PERIOD_MINUTES * 60,
  0,      // failures
  0xFF,   // destination endpoint
  false,  // waiting for response
  STATE_NONE,
  EMBER_ZCL_STATUS_SUCCESS,
  true,  // suppose that TC has Keep Alive server cluster
};

//------------------------------------------------------------------------------
// Forward Declarations

static void initiateSearchForNewNetworkWithTrustCenter(void);
static void delayUntilNextKeepalive(void);
static void trustCenterKeepaliveStart(void);

//------------------------------------------------------------------------------

static KeepaliveStatusStruct* getCurrentStatusStruct(void)
{
  return &(keepaliveStatusArray[emberGetCurrentNetwork()]);
}

void emberAfPluginTrustCenterKeepaliveInitCallback(void)
{
  uint8_t i;

  for (i = 0; i < EMBER_SUPPORTED_NETWORKS; i++) {
    MEMCOPY(&(keepaliveStatusArray[i]), &defaultStruct, sizeof(KeepaliveStatusStruct));
  }
}

void emberAfTrustCenterKeepaliveAbortCallback(void)
{
  KeepaliveStatusStruct* currentStatusStruct = getCurrentStatusStruct();
  // Do not set to standby if not even enabled.
  if (keepaliveIsEnabled(currentStatusStruct)) {
    currentStatusStruct->state = STATE_NONE;
  }
  emberAfCorePrintln("Setting trust center keepalive inactive.");
  emberAfNetworkEventControlSetInactive(emberAfPluginTrustCenterKeepaliveTickNetworkEventControls);
}

void emberAfPluginTrustCenterKeepaliveStackStatusCallback(EmberStatus status)
{
  KeepaliveStatusStruct* currentStatusStruct = getCurrentStatusStruct();
  if (0 == currentStatusStruct) {
    emberAfSecurityPrintln("Cannot start keepalive due to invalid memory");
    return;
  }

  if (!keepaliveIsEnabled(currentStatusStruct)) {
    return;
  }

  switch (status) {
    case EMBER_NETWORK_UP:
    case EMBER_TRUST_CENTER_EUI_HAS_CHANGED:
      // If we were searching for a network after a succession of keepalive
      // failures, and now we have found the network, reset the keepalive state
      if (currentStatusStruct->state == STATE_INITIATE_NETWORK_SEARCH) {
        currentStatusStruct->state = STATE_NONE;
      }
      trustCenterKeepaliveStart();
      break;
    case EMBER_NETWORK_DOWN:
      emberAfTrustCenterKeepaliveAbortCallback();
      break;
    default:
      break;
  }
}

void emberAfPluginTrustCenterKeepaliveEnable(void)
{
  KeepaliveStatusStruct* currentStatusStruct = getCurrentStatusStruct();
  if (0 == currentStatusStruct) {
    emberAfSecurityPrintln("Trust Center Keep Alive: invalid memory");
    return;
  }

  currentStatusStruct->state = STATE_NONE;

  // Start the keep alive process if we are in a joined state already,
  // wait for the stack status callback otherwise.
  EmberStatus status = emberNetworkState();
  if (status == EMBER_JOINED_NETWORK || status == EMBER_JOINED_NETWORK_NO_PARENT) {
    trustCenterKeepaliveStart();
  }
}

void emberAfPluginTrustCenterKeepaliveDisable(void)
{
  KeepaliveStatusStruct* currentStatusStruct = getCurrentStatusStruct();
  if (0 == currentStatusStruct) {
    emberAfSecurityPrintln("Trust Center Keep Alive: invalid memory");
    return;
  }

  currentStatusStruct->state = STATE_DISABLED;
  emberAfTrustCenterKeepaliveAbortCallback();
}

static void trustCenterKeepaliveStart(void)
{
  KeepaliveStatusStruct* currentStatusStruct = getCurrentStatusStruct();
  uint16_t baseTimeSeconds, jitterTimeSeconds;

  if (emberAfGetNodeId() == EMBER_TRUST_CENTER_NODE_ID) {
    // If this code is executing, then the trust center must be alive.
    return;
  }

  if (currentStatusStruct->state != STATE_NONE) {
    return;
  }

  MEMCOPY(currentStatusStruct, &defaultStruct, sizeof(KeepaliveStatusStruct));

  // Give a chance to any third-party user to change the check-in times.
  if (emberAfTrustCenterKeepaliveOverwriteDefaultTimingCallback(&baseTimeSeconds, &jitterTimeSeconds)) {
    currentStatusStruct->baseTimeSeconds = baseTimeSeconds;
    currentStatusStruct->jitterTimeSeconds = jitterTimeSeconds;
  }

  currentStatusStruct->state = STATE_DISCOVER_ENDPOINT;

  delayUntilNextKeepalive();
}

// Here is a hack to randomize without a formal rand(x) function.
// We know that the keepalive jitter can be between 0 and 0x200 seconds.
// Therefore we will just round up to the nearest value that has all 1 bits
// set and thus can give us a mask to our emberGetPseudoRandomNumber().
// Example:  Round any value between 80 and 179 to 127 (0x007F) and use that
// as our jitter.
// If we go over the jitter amount, then we'll just subtract the random value by
// the jitter amount later.
// Example: Jitter of 80 gets us a random value up to 127. If we get, say, 112,
// we subtract 112 by the max jitter possible (80) to get 42 in this case.

// There is a better way of calculating a good random number but it requires
// several calls to emberGetPseudoRandomNumber().
// If we get a jitter of 80, first we'll calculate a random number up until its
// next lower mask (63 = 11_1111b), and then add it to a random of the
// the difference (80 - 63 = 17), also masked to the next lowest mask
// (15 = 1111b). Keep repeating that until we get a difference that is
// negligible (like 7 or less).
// Example: jitter is 97
// emberGetPseudoRandomNumber() & 11_1111b (63) = 47,   difference is 97 - 63 = 34
// emberGetPseudoRandomNumber() & 1_1111b (31) = 17,    difference is 34 - 31 = 3
// and stop. Sum is 47 + 17 = 64. It's a bit better than the previous method
// but it requires multiple calls to emberGetPseudoRandomNumber() and it may be
// overkill for what keepalive really needs.

typedef struct {
  uint16_t minValue;
  uint16_t mask;
} RoundToMaskStruct;

const static RoundToMaskStruct roundToMaskArray[] = {
  { 300, (512 - 1) },
  { 180, (256 - 1) },
  { 80, (128 - 1) },
  { 50, (64 -  1) },
  { 20, (32 -  1) },
  { 10, (16 -  1) },
  { 0, (8  -  1) },
};

static uint16_t getInt16uRandomMask(uint16_t value)
{
  uint8_t i;
  for (i = 0; i < sizeof(roundToMaskArray) / sizeof(RoundToMaskStruct); i++) {
    if (value >= roundToMaskArray[i].minValue) {
      return roundToMaskArray[i].mask;
    }
  }
  return 0x000F;
}

static void delayUntilNextKeepalive(void)
{
  KeepaliveStatusStruct* currentStatusStruct = getCurrentStatusStruct();
  uint32_t randomJitter = (emberGetPseudoRandomNumber()
                           & getInt16uRandomMask(currentStatusStruct->jitterTimeSeconds));
  // The random mask function may return a value over the value of jitter, since
  // the random function works with masks, so ensure the jitter limit
  if (randomJitter > currentStatusStruct->jitterTimeSeconds) {
    randomJitter -= currentStatusStruct->jitterTimeSeconds;
  }

  uint32_t baseTimeSeconds = currentStatusStruct->baseTimeSeconds;
  uint32_t delayTimeSeconds = baseTimeSeconds + randomJitter;

  emberAfSecurityPrintln(
    "TC Keep-Alive delay is %d sec (%d sec base + %d sec jitter [%d])",
    delayTimeSeconds,
    currentStatusStruct->baseTimeSeconds,
    randomJitter, currentStatusStruct->jitterTimeSeconds);

  emberAfNetworkEventControlSetDelayMS(emberAfPluginTrustCenterKeepaliveTickNetworkEventControls,
                                       (delayTimeSeconds
                                        * MILLISECOND_TICKS_PER_SECOND));
}

static void messageTimeout(void)
{
  KeepaliveStatusStruct* currentStatusStruct = getCurrentStatusStruct();

  currentStatusStruct->waitingForResponse = false;
  if (currentStatusStruct->failures != 255) {
    currentStatusStruct->failures++;
  }

  emberAfSecurityPrintln("ERR: Trust center did not acknowledge "
                         "previous keep-alive signal correctly (timeout"
                         " %d)", currentStatusStruct->failures);

  if (currentStatusStruct->failures >= EMBER_AF_PLUGIN_TRUST_CENTER_KEEPALIVE_FAILURE_LIMIT) {
    emberAfSecurityPrintln("ERR: keep-alive failure limit reached (%d)",
                           EMBER_AF_PLUGIN_TRUST_CENTER_KEEPALIVE_FAILURE_LIMIT);

    emberSetHubConnectivity(false);

    // The WWAH plugin wants to implement its own rejoin algorithm, so see if
    // the callback is consumed
    if (false == emberAfPluginTrustCenterKeepaliveTimeoutCallback()) {
      initiateSearchForNewNetworkWithTrustCenter();
    } else {
      delayUntilNextKeepalive();
    }
  } else {
    delayUntilNextKeepalive();
  }
}

static void serviceDiscoveryCallback(const EmberAfServiceDiscoveryResult* result)
{
  KeepaliveStatusStruct* currentStatusStruct = getCurrentStatusStruct();
  if (currentStatusStruct->state != STATE_DISCOVER_ENDPOINT) {
    return;
  }

  if (result->status == EMBER_AF_UNICAST_SERVICE_DISCOVERY_COMPLETE_WITH_RESPONSE) {
    const EmberAfEndpointList* list = (EmberAfEndpointList*)result->responseData;

    // This is the original scenario where we do care the result of the response.
    // Server does support the the Keep-Alive cluster. So we can advance to send
    // the Keep-Alive signals.
    currentStatusStruct->destinationEndpoint = list->list[0];
    emberAfPluginTrustCenterKeepaliveConnectivityEstablishedCallback();
    emAfSendKeepaliveSignal();
  } else if (result->status == EMBER_AF_UNICAST_SERVICE_DISCOVERY_COMPLETE_WITH_EMPTY_RESPONSE) {
    if (emberAfPluginTrustCenterKeepaliveServerlessIsSupportedCallback()) {
      // Some third-party users of this mechanism (eg. WWAH) might not care about
      // the actual result of the service discovery only if a response has arrived
      // from the TC.
      // Note, the use case is when the TC cannot include the server side of the
      // cluster for some reason. In this case we just stay with the service
      // discovery (ie. sending Match Descriptor Requests) instead of advancing
      // to send the Keep-Alive signals (ie. Keep-Alive Attribute Read Requests).
      currentStatusStruct->tcHasKeepAliveServerCluster = false;
      currentStatusStruct->failures = 0;
      if (emberAfTrustCenterKeepaliveServerlessIsEnabledCallback()) {
        // Third-party user of this mechanism (eg. WWAH) has the functionality
        // enabled. Verify TC Connectivity and go, setup and delay appropriately.
        emberAfPluginTrustCenterKeepaliveConnectivityEstablishedCallback();
        delayUntilNextKeepalive();
      } else {
        // Third-party user of this mechanism (eg. WWAH) has the functionality
        // disabled at the moment. Not need to do anythin more, go idle.
        emberAfPluginTrustCenterKeepaliveDisable();
      }
    } else {
      // This is the original scenario where we do care the result of the response.
      // Server does not support the Keep-Alive cluster on any endpoints
      // This is not an error until Smart Energy 1.2b is certified and support
      // of the Keep-Alive cluster is made mandatory

      // TC returned no endpoints for those which support Keep-Alive cluster
      // Count this as an error. No islands.
      emberAfSecurityPrintln("Server not reporting any endpoints that "
                             "support Keep-Alive cluster");
      messageTimeout();
    }
  } else {
    // This code means we sent a discovery packet but received no response on
    // the channel. Count it as an error. We also classify this as a failure of
    // the keepalive.
    emberAfSecurityPrintln("Failed to discover Keep-Alive service on "
                           "Trust Center (0x%X)", result->status);
    messageTimeout();
  }
}

static void discoveryKeepaliveEndpoint(void)
{
  // Use profile ID wildcard since it's not known what type of endpoint the server will have
  EmberStatus status = emberAfFindDevicesByProfileAndCluster(EMBER_TRUST_CENTER_NODE_ID,
                                                             0xFFFF,
                                                             ZCL_KEEPALIVE_CLUSTER_ID,
                                                             true,  // server
                                                             serviceDiscoveryCallback);
  if (status != EMBER_SUCCESS) {
    // We treat even internal errors as a failure of the keepalive.  It is slighly
    // more robust in the belief that if we do trigger a trust center search due to
    // internal failure it is better than never triggering one due to internal failure.
    emberAfSecurityPrintln("Failed to initiate Keep-Alive service discovery "
                           "(0x%X)", status);
    messageTimeout();
    return;
  } // else
    //   Nothing to do.  The service discovery code will return the result or time
    //   out the process.
}

void emberAfPluginTrustCenterKeepaliveTickNetworkEventHandler(void)
{
  KeepaliveStatusStruct* currentStatusStruct = getCurrentStatusStruct();
  emberAfNetworkEventControlSetInactive(emberAfPluginTrustCenterKeepaliveTickNetworkEventControls);

  if (currentStatusStruct->waitingForResponse) {
    messageTimeout();
    return;
  }

  switch (currentStatusStruct->state) {
    case STATE_DISCOVER_ENDPOINT:
      discoveryKeepaliveEndpoint();
      break;
    case STATE_SEND_KEEPALIVE_SIGNAL:
      emAfSendKeepaliveSignal();
      break;
    default:
      break;
  }
}

void emberAfKeepaliveClusterClientDefaultResponseCallback(uint8_t endpoint,
                                                          uint8_t commandId,
                                                          EmberAfStatus status)
{
  if (commandId == ZCL_READ_ATTRIBUTES_COMMAND_ID) {
    KeepaliveStatusStruct* currentStatusStruct = getCurrentStatusStruct();
    currentStatusStruct->waitingForResponse = false;
    currentStatusStruct->lastResponseZclStatus = status;
  }
}

void emAfSendKeepaliveSignal(void)
{
  KeepaliveStatusStruct* currentStatusStruct = getCurrentStatusStruct();
  EmberStatus status;
  uint8_t attributeIds[] = {
    LOW_BYTE(ZCL_KEEPALIVE_BASE_ATTRIBUTE_ID),
    HIGH_BYTE(ZCL_KEEPALIVE_BASE_ATTRIBUTE_ID),
    LOW_BYTE(ZCL_KEEPALIVE_JITTER_ATTRIBUTE_ID),
    HIGH_BYTE(ZCL_KEEPALIVE_JITTER_ATTRIBUTE_ID)
  };
  uint8_t sourceEndpoint = emberAfPrimaryEndpointForCurrentNetworkIndex();

  emberAfFillCommandGlobalClientToServerReadAttributes(ZCL_KEEPALIVE_CLUSTER_ID,
                                                       attributeIds,
                                                       sizeof(attributeIds));

  // It is possible we will retrieve an undefined endpoint (0xFF) if we rebooted
  // and the TC is not around.  Nonetheless we will still use it as the broadcast
  // endpoint in the hopes that the trust center will respond.
  emberAfSetCommandEndpoints(sourceEndpoint, currentStatusStruct->destinationEndpoint);

  // The keepalive is an attempt to read a Key Establishment attribute on the
  // trust center.  In general, APS encryption is not required for Key
  // Establishment commands, but it is required by the spec for the keepalive,
  // so the option is explicitly set here.
  emberAfGetCommandApsFrame()->options |= EMBER_APS_OPTION_ENCRYPTION;
  status = emberAfSendCommandUnicast(EMBER_OUTGOING_DIRECT,
                                     EMBER_TRUST_CENTER_NODE_ID);
  if (status != EMBER_SUCCESS) {
    emberAfSecurityPrintln("Failed to send keep-alive signal to trust "
                           "center endpoint 0x%x (0x%x)",
                           currentStatusStruct->destinationEndpoint,
                           status);
    // Consider it a failure of the timeout. We can't island ourself.
    messageTimeout();
  }
  currentStatusStruct->state = STATE_SEND_KEEPALIVE_SIGNAL;
  currentStatusStruct->waitingForResponse = (status == EMBER_SUCCESS);
  if (status == EMBER_SUCCESS) {
    emberAfNetworkEventControlSetDelayMS(emberAfPluginTrustCenterKeepaliveTickNetworkEventControls,
                                         KEEPALIVE_WAIT_TIME_MS);
  } else {
    delayUntilNextKeepalive();
  }
}

void emAfPluginTrustCenterKeepaliveReadAttributesResponseCallback(uint8_t *buffer,
                                                                  uint16_t bufLen)
{
  KeepaliveStatusStruct* currentStatusStruct = getCurrentStatusStruct();
  if (emberAfCurrentCommand()->source == EMBER_TRUST_CENTER_NODE_ID
      && currentStatusStruct->state == STATE_SEND_KEEPALIVE_SIGNAL) {
    uint8_t i;
    uint8_t index = 0;
    uint8_t attributesFound = 0;

    currentStatusStruct->waitingForResponse = false;
    currentStatusStruct->lastResponseZclStatus = EMBER_ZCL_STATUS_SUCCESS;

    for (i = 0; i < 2; i++) {
      uint16_t attributeId = HIGH_LOW_TO_INT(buffer[index + 1], buffer[index]);
      uint8_t status = buffer[index + 2];
      uint8_t type = buffer[index + 3];
      index += (sizeof(attributeId) + sizeof(status) + sizeof(type));

      if (status == EMBER_ZCL_STATUS_SUCCESS) {
        if (attributeId == ZCL_KEEPALIVE_BASE_ATTRIBUTE_ID) {
          currentStatusStruct->baseTimeSeconds = buffer[index] * 60;
          attributesFound++;
          index += sizeof(uint8_t);
        } else if (attributeId == ZCL_KEEPALIVE_JITTER_ATTRIBUTE_ID) {
          currentStatusStruct->jitterTimeSeconds = HIGH_LOW_TO_INT(buffer[index + 1], buffer[index]);
          attributesFound++;
          index += sizeof(currentStatusStruct->jitterTimeSeconds);
        } else {
          emberAfSecurityPrintln("Keep-Alive readAttribute: unexpected "
                                 "attribute ID 0x%2X", attributeId);
          goto keepaliveReadAttributeFailure;
        }
      } else if (currentStatusStruct->lastResponseZclStatus > EMBER_ZCL_STATUS_SUCCESS) {
        // Only record one of the failures if there are multiple.  This should
        // be good enough as the CLI prints out all the other failures, though
        // only when actively receiving a keepalive.
        currentStatusStruct->lastResponseZclStatus = status;
      }
    }
    if (attributesFound == 2) {
      if (currentStatusStruct->failures != 0) {
        emberAfPluginTrustCenterKeepaliveConnectivityEstablishedCallback();
        emberSetHubConnectivity(true);
      }

      currentStatusStruct->failures = 0;
      currentStatusStruct->state = STATE_SEND_KEEPALIVE_SIGNAL;

      // Delay based on the new parameters we got back from the server.
      delayUntilNextKeepalive();
      return;
    }

    keepaliveReadAttributeFailure:
    // Read failures also constitute keepalive errors. We'll just call timeout
    // to handle them for now.
    emberAfSecurityPrintln("Error in keep-alive response, %d of 2 attributes "
                           "received.", attributesFound);
    messageTimeout();
  }
}

static void initiateSearchForNewNetworkWithTrustCenter(void)
{
  KeepaliveStatusStruct* currentStatusStruct = getCurrentStatusStruct();

  EmberStatus status;
  emberAfSecurityFlush();
  emberAfSecurityPrintln("Initiating trust center search");
  emberAfSecurityFlush();
  status = emberStopWritingStackTokens();
  if (status == EMBER_SUCCESS) {
    EmberBeaconClassificationParams param;
    emberGetBeaconClassificationParams(&param);
    // Activate TC-connectivity based beacon prioritization, we won't deactivate it later
    param.beaconClassificationMask |= PRIORITIZE_BEACONS_BASED_ON_TC_CONNECTVITY;
    emberSetBeaconClassificationParams(&param);
    status = emberFindAndRejoinNetworkWithReason(false, // TC (unsecured) rejoin
                                                 EMBER_ALL_802_15_4_CHANNELS_MASK,
                                                 EMBER_AF_REJOIN_DUE_TO_TC_KEEPALIVE_FAILURE);
  } else {
    emberAfSecurityPrintln("Failed to suspend token writing");
  }

  if (status == EMBER_SUCCESS) {
    currentStatusStruct->state = STATE_INITIATE_NETWORK_SEARCH;
  } else {
    emberAfSecurityPrintln("Could not initiate TC search (0x%x)", status);
    emberStartWritingStackTokens();
  }
}

uint8_t emAfPluginTrustCenterKeepaliveGetBaseTimeMinutes(void)
{
  KeepaliveStatusStruct* currentStatusStruct = getCurrentStatusStruct();
  return (uint8_t)(currentStatusStruct->baseTimeSeconds / 60);
}

uint16_t emAfPluginTrustCenterKeepaliveGetBaseTimeSeconds(void)
{
  KeepaliveStatusStruct* currentStatusStruct = getCurrentStatusStruct();
  return currentStatusStruct->baseTimeSeconds;
}

uint16_t emAfPluginTrustCenterKeepaliveGetJitterTimeSeconds(void)
{
  KeepaliveStatusStruct* currentStatusStruct = getCurrentStatusStruct();
  return currentStatusStruct->jitterTimeSeconds;
}

bool emAfPluginTrustCenterKeepaliveTcHasServerCluster(void)
{
  KeepaliveStatusStruct* currentStatusStruct = getCurrentStatusStruct();
  return currentStatusStruct->tcHasKeepAliveServerCluster;
}
