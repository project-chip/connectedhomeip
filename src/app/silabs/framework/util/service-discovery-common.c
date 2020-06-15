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
 * @brief Service discovery code that is common to different types of service
 * discovery, e.g. match descriptor, NWK address lookup, and IEEE address
 * lookup.
 *******************************************************************************
   ******************************************************************************/

#include "app/framework/include/af.h"
#include "app/util/zigbee-framework/zigbee-device-common.h"
#ifdef EZSP_HOST
  #include "app/util/zigbee-framework/zigbee-device-host.h"
#endif
#include "service-discovery.h"

#if EMBER_SUPPORTED_NETWORKS > 4
  #error "Service discovery is limited to four networks."
#endif

#define MAX_SIMPLE_DESCRIPTOR_RESPONSE_PAYLOAD                          \
  (128   /* 802.15.4 MTU */                                             \
   - 1   /* PHY length byte */                                          \
   - 11  /* MAC header 9 + 2 byte CRC */                                \
   - 8   /* NWK header */                                               \
   - 18  /* NWK security header + 4-byte MIC */                         \
   - 8   /* APS header */                                               \
   - EMBER_AF_ZDO_RESPONSE_OVERHEAD                                     \
   - 3   /* Active Endpoint Response overhead: Address(2), length(1) */ \
  )

#define SIMPLE_DESCRIPTOR_RESPONSE_ENDPOINT_OFFSET \
  (EMBER_AF_ZDO_RESPONSE_OVERHEAD                  \
   + 2 /* address of interest */                   \
   + 1 /* length value */                          \
  )

#define SIMPLE_DESCRIPTOR_RESPONSE_PROFILE_ID_OFFSET \
  (SIMPLE_DESCRIPTOR_RESPONSE_ENDPOINT_OFFSET        \
   + 1 /* endpoint */                                \
  )

#define SIMPLE_DESCRIPTOR_RESPONSE_DEVICE_ID_OFFSET \
  (SIMPLE_DESCRIPTOR_RESPONSE_PROFILE_ID_OFFSET + 2)

#define SIMPLE_DESCRIPTOR_RESPONSE_INPUT_CLUSTER_LIST_COUNT_INDEX \
  (SIMPLE_DESCRIPTOR_RESPONSE_DEVICE_ID_OFFSET                    \
   + 2  /* device ID length */                                    \
   + 1  /* version (4-bits), reserved (4-bits) */                 \
  )

#define SIMPLE_DESCRIPTOR_RESPONSE_INPUT_CLUSTER_LIST_INDEX \
  (SIMPLE_DESCRIPTOR_RESPONSE_INPUT_CLUSTER_LIST_COUNT_INDEX + 1)

#define MAX_CLUSTERS_IN_ACTIVE_ENDPOINT_RESPONSE (MAX_SIMPLE_DESCRIPTOR_RESPONSE_PAYLOAD >> 1)

#define ACTIVE_ENDPOINT_RESPONSE_COUNT_OFFSET \
  (EMBER_AF_ZDO_RESPONSE_OVERHEAD             \
   + 2) // Address of Interest

//==============================================================================
// Service discovery state machine
//
//   This code handles initiating a limited set of ZDO, receiving
//   the response and sending it back to the cluster or code element that
//   requested it.  Unfortunately the ZDO message does not have any distinct
//   identifiers that would allow us to determine what cluster/endpoint on our
//   local device initiated the request.  Therefore we can only allow one
//    outstanding request at a time.

EmberEventControl emAfServiceDiscoveryEventControls[EMBER_SUPPORTED_SERVICE_DISCOVERY_STATES_PER_NETWORK][EMBER_SUPPORTED_NETWORKS];

typedef struct {
  bool active;
  EmberAfServiceDiscoveryCallback *callback;
  // This will contain the target type: broadcast or unicast (high bit)
  // and the ZDO cluster ID of the request.  Since ZDO requests
  // clear the high bit (only responses use it), we can use that leftover bit
  // for something else.
  uint16_t requestData;
  // Tracking the sequence when sending out discovery requests to be able to get
  // the correct discovery state for the request when the response comes back.
  // If the state is not active then it can be considered uninitialized.
  uint8_t requestSequence;
  // tracking the state index in Service Discovery States
  uint8_t stateIndex;
  //tracking the network index in Service Discovery States
  uint8_t networkIndex;
} ServiceDiscoveryState;
static ServiceDiscoveryState states[EMBER_SUPPORTED_SERVICE_DISCOVERY_STATES_PER_NETWORK][EMBER_SUPPORTED_NETWORKS];
static uint8_t stateLimit = EMBER_SUPPORTED_SERVICE_DISCOVERY_STATES_PER_NETWORK;
static bool statesInitialized = false;
#define UNICAST_QUERY_BIT (0x8000)
#define isUnicastQuery(state) (UNICAST_QUERY_BIT == ((state)->requestData & UNICAST_QUERY_BIT))
#define setUnicastQuery(state) ((state)->requestData |= UNICAST_QUERY_BIT)
#define getRequestCluster(state) ((state)->requestData & ~UNICAST_QUERY_BIT)
#define serviceDiscoveryInProgress(state) ((state)->active)

#ifndef EMBER_AF_DISCOVERY_TIMEOUT_QS
  #define EMBER_AF_DISCOVERY_TIMEOUT_QS (2 * 4)
#endif

// seq. number (1), status (1), address (2), length (1)
#define MATCH_DESCRIPTOR_OVERHEAD               5
#define MINIMUM_MATCH_DESCRIPTOR_SUCCESS_LENGTH MATCH_DESCRIPTOR_OVERHEAD

// seq. number (1), status (1)
#define EMBER_AF_ZDO_RESPONSE_OVERHEAD 2
// EUI64 (8), node ID (2),
#define MINIMUM_ADDRESS_REQEUST_SUCCESS (EMBER_AF_ZDO_RESPONSE_OVERHEAD + 10)
#define ADDRESS_RESPONSE_NODE_ID_OFFSET (EMBER_AF_ZDO_RESPONSE_OVERHEAD + EUI64_SIZE)

// Address (2), EP Count (1)
#define MINIMUM_ACTIVE_ENDPOINT_RESPONSE (EMBER_AF_ZDO_RESPONSE_OVERHEAD + 3)
#define ACTIVE_ENDPOINT_RESPONSE_NODE_ID_OFFSET (EMBER_AF_ZDO_RESPONSE_OVERHEAD)
#define ACTIVE_ENDPOINT_RESPONSE_LIST_OFFSET (ACTIVE_ENDPOINT_RESPONSE_NODE_ID_OFFSET + 3)

#define PREFIX "Svc Disc: "

//==============================================================================
// Forward Declarations

static void setupDiscoveryData(ServiceDiscoveryState *state,
                               EmberNodeId messageDest,
                               EmberAfServiceDiscoveryCallback *callback,
                               uint16_t zdoClusterId,
                               uint8_t sequenceNumber);
static void firstTimeInitStates(void);
static ServiceDiscoveryState *findUnusedState(void);
static ServiceDiscoveryState *findStateForResponse(uint8_t sequenceNumber, uint8_t networkIndex);
uint8_t emberGetLastAppZigDevRequestSequence(void);
static ServiceDiscoveryState *findStateForEventControl(EmberEventControl *control);
static bool isStateActiveInCurrentNetwork(void);
#ifdef EMBER_TEST
void setDiscoveryStateLimit(uint8_t limit);
#endif

//==============================================================================

EmberStatus emberAfFindDevicesByProfileAndCluster(EmberNodeId target,
                                                  EmberAfProfileId profileId,
                                                  EmberAfClusterId clusterId,
                                                  bool serverCluster,
                                                  EmberAfServiceDiscoveryCallback *callback)
{
  ServiceDiscoveryState *state = findUnusedState();
  if (state == NULL) {
    emberAfServiceDiscoveryPrintln("%pService Discovery State limits reached\n", PREFIX);
    return EMBER_INVALID_CALL;
  }
  EmberStatus status;

  if (EMBER_BROADCAST_ADDRESS <= target
      && target != EMBER_RX_ON_WHEN_IDLE_BROADCAST_ADDRESS) {
    // Note:  The core spec. only allows a Match Descriptor broadcast to
    // the 'rx on when idle' address.  No other broadcast addresses are allowed.
    // The Ember stack will silently discard broadcast match descriptors
    // to invalid broadcast addresses.
    emberAfServiceDiscoveryPrintln("%pIllegal broadcast address, remapping to valid one.",
                                   PREFIX);
    target = EMBER_RX_ON_WHEN_IDLE_BROADCAST_ADDRESS;
  }

  status = emAfSendMatchDescriptor(target, profileId, clusterId, serverCluster);
  // Get the sequence number sent in the request.
  uint8_t sequenceNumber = emberGetLastAppZigDevRequestSequence();
  if (status != EMBER_SUCCESS) {
    emberAfServiceDiscoveryPrintln("%pFailed to send match discovery: 0x%x",
                                   PREFIX,
                                   status);
    return status;
  }

  emberAfServiceDiscoveryPrintln("%pStarting discovery for cluster 0x%2x",
                                 PREFIX,
                                 clusterId);
  setupDiscoveryData(state,
                     target,
                     callback,
                     MATCH_DESCRIPTORS_REQUEST,
                     sequenceNumber);

  return EMBER_SUCCESS;
}

EmberStatus emberAfFindActiveEndpoints(EmberNodeId target,
                                       EmberAfServiceDiscoveryCallback *callback)
{
  ServiceDiscoveryState *state = findUnusedState();
  if (state == NULL) {
    emberAfServiceDiscoveryPrintln("%pService Discovery State limits reached\n", PREFIX);
    return EMBER_INVALID_CALL;
  }
  EmberStatus status;

  status = emberActiveEndpointsRequest(target, EMBER_AF_DEFAULT_APS_OPTIONS);
  uint8_t sequenceNumber = emberGetLastAppZigDevRequestSequence();
  if (status != EMBER_SUCCESS) {
    emberAfServiceDiscoveryPrintln("%pFailed to send active endpoint request: 0x%x",
                                   PREFIX,
                                   status);
    return status;
  }

  setupDiscoveryData(state,
                     target,
                     callback,
                     ACTIVE_ENDPOINTS_REQUEST,
                     sequenceNumber);
  return status;
}

EmberStatus emberAfFindClustersByDeviceAndEndpoint(EmberNodeId target,
                                                   uint8_t targetEndpoint,
                                                   EmberAfServiceDiscoveryCallback *callback)
{
  ServiceDiscoveryState *state = findUnusedState();
  if (state == NULL) {
    emberAfServiceDiscoveryPrintln("%pService Discovery State limits reached\n", PREFIX);
    return EMBER_INVALID_CALL;
  }
  EmberStatus status;

  status = emberSimpleDescriptorRequest(target,
                                        targetEndpoint,
                                        EMBER_AF_DEFAULT_APS_OPTIONS);
  //Get the sequence number sent in the request.
  uint8_t sequenceNumber = emberGetLastAppZigDevRequestSequence();

  if (status != EMBER_SUCCESS) {
    emberAfServiceDiscoveryPrintln("%pFailed to send simple descriptor request: 0x%x",
                                   PREFIX,
                                   status);
    return status;
  }

  setupDiscoveryData(state,
                     target,
                     callback,
                     SIMPLE_DESCRIPTOR_REQUEST,
                     sequenceNumber);

  return status;
}

EmberStatus emberAfFindIeeeAddress(EmberNodeId shortAddress,
                                   EmberAfServiceDiscoveryCallback *callback)
{
  ServiceDiscoveryState *state = findUnusedState();
  if (state == NULL) {
    emberAfServiceDiscoveryPrintln("%pService Discovery State limits reached\n", PREFIX);
    return EMBER_INVALID_CALL;
  }
  EmberStatus status;

  status = emberIeeeAddressRequest(shortAddress,
                                   false,         // report kids?
                                   0,             // child start index
                                   EMBER_APS_OPTION_RETRY);
  // Get the sequence number sent in the request.
  // For SoC, most requests use the application layer's half
  // of the sequence numbering space (0x00-0x7f), but this request
  // uses the stack's half of the sequence numbering space (0x80-0xFF).
  // For Host, all requests use the application layer space.
#if defined(EZSP_HOST)
  uint8_t sequenceNumber = emberGetLastAppZigDevRequestSequence();
#else
  uint8_t sequenceNumber = emberGetLastStackZigDevRequestSequence();
#endif

  if (status != EMBER_SUCCESS) {
    emberAfServiceDiscoveryPrintln("%pFailed to send IEEE address request: 0x%x",
                                   PREFIX,
                                   status);
    return status;
  }

  setupDiscoveryData(state,
                     shortAddress,
                     callback,
                     IEEE_ADDRESS_REQUEST,
                     sequenceNumber);

  return status;
}

EmberStatus emberAfFindNodeId(EmberEUI64 longAddress,
                              EmberAfServiceDiscoveryCallback *callback)
{
  ServiceDiscoveryState *state = findUnusedState();
  if (state == NULL) {
    emberAfServiceDiscoveryPrintln("%pService Discovery State limits reached\n", PREFIX);
    return EMBER_INVALID_CALL;
  }
  EmberStatus status;

  status = emberNetworkAddressRequest(longAddress,
                                      false,         // report kids?
                                      0);            // child start index
  // Get the sequence number sent in the request.
  // For SoC, most requests use the application layer's half
  // of the sequence numbering space (0x00-0x7f), but this request
  // uses the stack's half of the sequence numbering space (0x80-0xFF).
  // For Host, all requests use the application layer space.
#if defined(EZSP_HOST)
  uint8_t sequenceNumber = emberGetLastAppZigDevRequestSequence();
#else
  uint8_t sequenceNumber = emberGetLastStackZigDevRequestSequence();
#endif
  if (status != EMBER_SUCCESS) {
    emberAfServiceDiscoveryPrintln("%pFailed to send NWK address request: 0x%x",
                                   PREFIX,
                                   status);
    return status;
  }

  setupDiscoveryData(state,
                     EMBER_BROADCAST_ADDRESS,
                     callback,
                     NETWORK_ADDRESS_REQUEST,
                     sequenceNumber);

  return status;
}

static void setupDiscoveryData(ServiceDiscoveryState *state,
                               EmberNodeId messageDest,
                               EmberAfServiceDiscoveryCallback *callback,
                               uint16_t zdoClusterRequest,
                               uint8_t sequenceNumber)
{
  state->active = true;
  state->requestData = zdoClusterRequest;
  if (messageDest < EMBER_BROADCAST_ADDRESS) {
    setUnicastQuery(state);
  }
  state->callback = callback;
  state->requestSequence = sequenceNumber;
  emberAfServiceDiscoveryPrintln("%pWaiting %d sec for discovery to complete",
                                 PREFIX,
                                 EMBER_AF_DISCOVERY_TIMEOUT_QS >> 2);
  emberAfNetworkEventControlSetDelayQS(emAfServiceDiscoveryEventControls[state->stateIndex],
                                       EMBER_AF_DISCOVERY_TIMEOUT_QS);
  // keep sleepy end devices out of hibernation until
  // service discovery is complete
  emberAfAddToCurrentAppTasks(EMBER_AF_WAITING_FOR_SERVICE_DISCOVERY);
}

static void serviceDiscoveryComplete(ServiceDiscoveryState *state)
{
  if (state == NULL) {
    emberAfServiceDiscoveryPrintln("State corresponding to the the request not found\n");
    return;
  }

  emberAfPushNetworkIndex(state->networkIndex);
  state->active = false;
  emberAfServiceDiscoveryPrintln("%pcomplete.\n", PREFIX);
  emberAfNetworkEventControlSetInactive(emAfServiceDiscoveryEventControls[state->stateIndex]);
  // Since there can be multiple states active on the same network,
  // emberAfRemoveFromCurrentAppTasks should be called only when all the states
  // in the current network are inactive.
  if (!isStateActiveInCurrentNetwork()) {
    // allow sleepy end devices to go into hibernation now.
    emberAfRemoveFromCurrentAppTasks(EMBER_AF_WAITING_FOR_SERVICE_DISCOVERY);
  }

  if (state->callback != NULL) {
    EmberAfServiceDiscoveryResult result;
    result.status = (isUnicastQuery(state)
                     ? EMBER_AF_UNICAST_SERVICE_DISCOVERY_TIMEOUT
                     : EMBER_AF_BROADCAST_SERVICE_DISCOVERY_COMPLETE);
    result.zdoRequestClusterId = getRequestCluster(state);
    result.matchAddress = EMBER_NULL_NODE_ID;
    result.responseData = NULL;
    (*state->callback)(&result);
  }

  emberAfPopNetworkIndex();
}

void emAfServiceDiscoveryTimeoutHandler(EmberEventControl *control)
{
  emberEventControlSetInactive(*control);
  ServiceDiscoveryState *state = findStateForEventControl(control);
  serviceDiscoveryComplete(state);
}

void emAfServiceDiscoveryComplete(uint8_t networkIndex, uint8_t sequenceNumber)
{
  ServiceDiscoveryState *state = findStateForResponse(sequenceNumber, networkIndex);
  serviceDiscoveryComplete(state);
}

static void executeCallback(ServiceDiscoveryState *state,
                            const EmberAfServiceDiscoveryResult *result)
{
  (*state->callback)(result);
  if (isUnicastQuery(state)
      || result->status == EMBER_AF_BROADCAST_SERVICE_DISCOVERY_COMPLETE_WITH_RESPONSE) {
    // If the request was unicast and we received a response then we are done.
    // No need to wait for the timer to expire.
    // Alternatively certain broadcast requests are only expected to get 1
    // response, or we only care about the 1st response.

    // We NULL the callback as a way of indicating we already fired it.
    // For timeouts, the callback will not be NULL and still fire.
    state->callback = NULL;
    serviceDiscoveryComplete(state);
  }
}

static bool processMatchDescriptorResponse(ServiceDiscoveryState *state,
                                           const uint8_t *message,
                                           uint16_t length)
{
  EmberNodeId matchId;
  uint8_t listLength;

  if (length < MINIMUM_MATCH_DESCRIPTOR_SUCCESS_LENGTH) {
    emberAfServiceDiscoveryPrintln("%pMessage too short", PREFIX);
    return true;
  }

  // This will now be used as the length of the match list.
  length -= MATCH_DESCRIPTOR_OVERHEAD;

  // If the parent of a sleepy device supports caching its descriptor
  // information then the sender of the response may not be the device
  // that actually matches the request.  The device id that matches
  // is included in the message.
  matchId = message[2] + (message[3] << 8);
  listLength = message[4];

  if (listLength != length) {
    emberAfServiceDiscoveryPrintln("%pMessage too short for num. endpoints",
                                   PREFIX);
    return true;
  }

  emberAfServiceDiscoveryPrintln("%pMatch%p found from 0x%2x.",
                                 PREFIX,
                                 (listLength > 0
                                  ? ""
                                  : " NOT"),
                                 matchId);

  EmberAfServiceDiscoveryResult result;
  EmberAfEndpointList endpointList;
  endpointList.count = length;
  endpointList.list = &(message[MATCH_DESCRIPTOR_OVERHEAD]);
  if (listLength) {
    result.status = (isUnicastQuery(state)
                     ? EMBER_AF_UNICAST_SERVICE_DISCOVERY_COMPLETE_WITH_RESPONSE
                     : EMBER_AF_BROADCAST_SERVICE_DISCOVERY_RESPONSE_RECEIVED);
  } else {
    // If we got an active response with an empty list then report it accordingly.
    result.status = (isUnicastQuery(state)
                     ? EMBER_AF_UNICAST_SERVICE_DISCOVERY_COMPLETE_WITH_EMPTY_RESPONSE
                     : EMBER_AF_BROADCAST_SERVICE_DISCOVERY_COMPLETE_WITH_EMPTY_RESPONSE);
  }
  result.zdoRequestClusterId = getRequestCluster(state);
  result.matchAddress = matchId;
  result.responseData = &endpointList;
  executeCallback(state, &result);
  return true;
}

static bool processSimpleDescriptorResponse(ServiceDiscoveryState *state,
                                            const uint8_t *message,
                                            uint16_t length)
{
  EmberAfServiceDiscoveryResult result;
  EmberAfClusterList clusterList;

  clusterList.profileId = message[SIMPLE_DESCRIPTOR_RESPONSE_PROFILE_ID_OFFSET]
                          + (message[SIMPLE_DESCRIPTOR_RESPONSE_PROFILE_ID_OFFSET + 1] << 8);
  clusterList.deviceId = message[SIMPLE_DESCRIPTOR_RESPONSE_DEVICE_ID_OFFSET]
                         + (message[SIMPLE_DESCRIPTOR_RESPONSE_DEVICE_ID_OFFSET + 1] << 8);
  clusterList.endpoint = message[SIMPLE_DESCRIPTOR_RESPONSE_ENDPOINT_OFFSET];

  // Copying the cluster list to a separate array
  // is primarily necessary on Big endian CPUs where
  // the over-the-air Zigbee messages are all little-endian
  // and we cannot just cast the message pointer into an uint16_t* array.
  // However we also do it for the little endian CPUs because the
  // message can actually be truncated due to bad Zigbee stacks.
  uint16_t clusters[MAX_CLUSTERS_IN_ACTIVE_ENDPOINT_RESPONSE];
  uint8_t index = SIMPLE_DESCRIPTOR_RESPONSE_INPUT_CLUSTER_LIST_INDEX;
  clusterList.inClusterCount = message[SIMPLE_DESCRIPTOR_RESPONSE_INPUT_CLUSTER_LIST_COUNT_INDEX];
  // The +1 is for the input cluster count length.
  clusterList.outClusterCount = message[SIMPLE_DESCRIPTOR_RESPONSE_INPUT_CLUSTER_LIST_COUNT_INDEX
                                        + 1 + (clusterList.inClusterCount * 2)];

  if ((length - index - 1) % 2 != 0) {  // subtract 1 for the output cluster count.
    emberAfServiceDiscoveryPrintln("Error: %p bad simple descriptor length.", PREFIX);
    // Return true because we still tried to process the message.
    return true;
  }
  uint8_t loop;
  uint8_t clusterIndex = 0;
  for (loop = 0; loop < 2; loop++) {
    uint8_t count;
    uint8_t i;
    if (loop == 0) {
      clusterList.inClusterList = &(clusters[clusterIndex]);
      count = clusterList.inClusterCount;
    } else {
      clusterList.outClusterList = &(clusters[clusterIndex]);
      count = clusterList.outClusterCount;
    }
    for (i = 0; i < count; i++) {
      clusters[clusterIndex] = message[index] + (message[index + 1] << 8);
      clusterIndex++;
      index += 2;
    }
    if (loop == 0) {
      // This is the output cluster count
      index++;
    }
  }

  result.status = (isUnicastQuery(state)
                   ? EMBER_AF_UNICAST_SERVICE_DISCOVERY_COMPLETE_WITH_RESPONSE
                   : EMBER_AF_BROADCAST_SERVICE_DISCOVERY_RESPONSE_RECEIVED);
  result.matchAddress = message[2] + (message[3] << 8);
  result.zdoRequestClusterId = getRequestCluster(state);
  result.responseData = &clusterList;

  executeCallback(state, &result);
  return true;
}

// Both NWK and IEEE responses have the same exact format.
static bool processAddressResponse(ServiceDiscoveryState *state,
                                   const uint8_t *message,
                                   uint16_t length)
{
  EmberAfServiceDiscoveryResult result;
  EmberEUI64 eui64LittleEndian;

  if (length < MINIMUM_ADDRESS_REQEUST_SUCCESS) {
    emberAfServiceDiscoveryPrintln("%pMessage too short", PREFIX);
    return true;
  }
  MEMMOVE(eui64LittleEndian, message + EMBER_AF_ZDO_RESPONSE_OVERHEAD, EUI64_SIZE);
  // Given that we only expect one response, we set the status to complete even
  // though the request was broadcast.
  result.status = (isUnicastQuery(state)
                   ? EMBER_AF_UNICAST_SERVICE_DISCOVERY_COMPLETE_WITH_RESPONSE
                   : EMBER_AF_BROADCAST_SERVICE_DISCOVERY_COMPLETE_WITH_RESPONSE);
  result.matchAddress = (message[ADDRESS_RESPONSE_NODE_ID_OFFSET]
                         + (message[ADDRESS_RESPONSE_NODE_ID_OFFSET + 1] << 8));
  result.zdoRequestClusterId = getRequestCluster(state);
  result.responseData = eui64LittleEndian;

  executeCallback(state, &result);
  return true;
}

static bool processActiveEndpointResponse(ServiceDiscoveryState* state,
                                          const uint8_t* message,
                                          uint16_t length)
{
  EmberAfServiceDiscoveryResult result;
  EmberAfEndpointList listStruct;
  if (length < MINIMUM_ACTIVE_ENDPOINT_RESPONSE) {
    emberAfServiceDiscoveryPrintln("%pMessage too short", PREFIX);
    return true;
  }
  result.matchAddress = (message[ACTIVE_ENDPOINT_RESPONSE_NODE_ID_OFFSET]
                         + (message[ACTIVE_ENDPOINT_RESPONSE_NODE_ID_OFFSET + 1] << 8));
  uint8_t count = message[ACTIVE_ENDPOINT_RESPONSE_COUNT_OFFSET];

  // +1 for count value iteslf, +2 for node ID
  uint8_t expectedLength = EMBER_AF_ZDO_RESPONSE_OVERHEAD + 1 + 2 + count;

  if (expectedLength != length) {
    emberAfServiceDiscoveryPrintln("Error: %p invalid length (%d != %d) for Active Endpoint response",
                                   PREFIX,
                                   expectedLength,
                                   length);
    return true;
  }
  emberAfServiceDiscoveryPrintln("%pgot active EP response", PREFIX);
  result.status = (isUnicastQuery(state)
                   ? EMBER_AF_UNICAST_SERVICE_DISCOVERY_COMPLETE_WITH_RESPONSE
                   : EMBER_AF_BROADCAST_SERVICE_DISCOVERY_RESPONSE_RECEIVED);
  result.zdoRequestClusterId = getRequestCluster(state);
  result.responseData = &listStruct;
  listStruct.list = &(message[ACTIVE_ENDPOINT_RESPONSE_LIST_OFFSET]);
  listStruct.count = message[ACTIVE_ENDPOINT_RESPONSE_COUNT_OFFSET];
  executeCallback(state, &result);
  return true;
}

bool emAfServiceDiscoveryIncoming(EmberNodeId sender,
                                  EmberApsFrame *apsFrame,
                                  const uint8_t *message,
                                  uint16_t length)
{
  ServiceDiscoveryState *state = findStateForResponse(message[0], emberGetCurrentNetwork());
  if (state == NULL) {
    return false;
  }

  if (!(serviceDiscoveryInProgress(state)
        && (apsFrame->profileId == EMBER_ZDO_PROFILE_ID
            // ZDO Responses set the high bit on the request cluster ID
            && (apsFrame->clusterId == (CLUSTER_ID_RESPONSE_MINIMUM
                                        | getRequestCluster(state)))))) {
    return false;
  }

  // The second byte is the status code
  if (message[1] != EMBER_ZDP_SUCCESS) {
    return true;
  }

  switch (apsFrame->clusterId) {
    case SIMPLE_DESCRIPTOR_RESPONSE:
      return processSimpleDescriptorResponse(state, message, length);

    case MATCH_DESCRIPTORS_RESPONSE:
      return processMatchDescriptorResponse(state, message, length);

    case NETWORK_ADDRESS_RESPONSE:
    case IEEE_ADDRESS_RESPONSE:
      return processAddressResponse(state, message, length);

    case ACTIVE_ENDPOINTS_RESPONSE:
      return processActiveEndpointResponse(state, message, length);

    default:
      // Some ZDO request we don't care about.
      break;
  }

  return false;
}

// Perform one-time initialization of states[][] entries.
// Sets the self-identifying array indices stateIndex and networkIndex
// for convenient reference when the state entry is accessed.
static void firstTimeInitStates(void)
{
  if (!statesInitialized) {
    for (uint8_t si = 0; si < stateLimit; ++si) {
      for (uint8_t ni = 0; ni < EMBER_SUPPORTED_NETWORKS; ++ni) {
        ServiceDiscoveryState *state = &states[si][ni];
        // Coordinates of this state entry, for convenient reference.
        state->stateIndex = si;
        state->networkIndex = ni;
        // Operational state.
        state->active = false;
        state->callback = NULL;
        state->requestData = 0;
        state->requestSequence = 0;
      }
    }
    // Do this only once.
    statesInitialized = true;
  }
}

/**
   Given: N/A.
   Return: A pointer to the first unused state for the current network
   Description: The unused state is found by looking for the first state
   which is inactive in the the current network.
 */
static ServiceDiscoveryState *findUnusedState(void)
{
  firstTimeInitStates();
  for (int si = 0; si < stateLimit; si++) {
    if (!states[si][emberGetCurrentNetwork()].active) {
      return &states[si][emberGetCurrentNetwork()];
    }
  }
  return NULL;
}

/**
   Given: The sequence number of the request and the network index
   Return: The active service discovery state with that sequence number
   in the given network index.
 */
static ServiceDiscoveryState *findStateForResponse(uint8_t sequenceNumber, uint8_t networkIndex)
{
  firstTimeInitStates();
  for (int si = 0; si < stateLimit; si++) {
    if (states[si][networkIndex].requestSequence == sequenceNumber
        && states[si][networkIndex].active) {
      return &states[si][networkIndex];
    }
  }
  return NULL;
}

/**
   Given: Ember event control
   Return: A pointer to the service discovery state which corresponds to the
   control.
 */
static ServiceDiscoveryState *findStateForEventControl(EmberEventControl *control)
{
  firstTimeInitStates();
  for (uint8_t si = 0; si < stateLimit; si++) {
    for (uint8_t ni = 0; ni < EMBER_SUPPORTED_NETWORKS; ni++) {
      if (control == &emAfServiceDiscoveryEventControls[si][ni]) {
        return &states[si][ni];
      }
    }
  }
  return NULL;
}

/**
   Description: Checking if there are any active states in the current network.
 */
static bool isStateActiveInCurrentNetwork(void)
{
  uint8_t currentNetwork = emberGetCurrentNetwork();
  for (uint8_t si = 0; si < stateLimit; si++) {
    if (states[si][currentNetwork].active) {
      return true;
    }
  }
  return false;
}

/**
   Given: The limit the number of networks allowed
   Description: Even though EMBER_SUPPORTED_SERVICE_DISCOVERY_STATES_PER_NETWORK
   can be set to 4, the state limit can be set to a smaller number using this
   function for testing purposes.
 */
#ifdef EMBER_TEST
void setDiscoveryStateLimit(uint8_t limit)
{
  if (limit > EMBER_SUPPORTED_SERVICE_DISCOVERY_STATES_PER_NETWORK) {
    emberAfServiceDiscoveryPrintln("Limit exceeds the maximum number of service discovery states allowed."
                                   "Setting the limit to %d which is the maximum number if states allowed",
                                   EMBER_SUPPORTED_SERVICE_DISCOVERY_STATES_PER_NETWORK);
    stateLimit = EMBER_SUPPORTED_SERVICE_DISCOVERY_STATES_PER_NETWORK;
  } else {
    stateLimit = limit;
  }
}
#endif
