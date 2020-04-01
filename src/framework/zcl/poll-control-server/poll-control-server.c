/***************************************************************************//**
 * @file
 * @brief
 ******************************************************************************/

#include PLATFORM_HEADER
#include CONFIGURATION_HEADER
#include CHIP_AF_API_STACK
#include CHIP_AF_API_HAL
#ifdef CHIP_AF_API_DEBUG_PRINT
  #include CHIP_AF_API_DEBUG_PRINT
#endif
#include CHIP_AF_API_ZCL_CORE
#include "thread-callbacks.h"
#include "poll-control-server.h"

// The built-in cluster tick has hooks into the polling code and is therefore
// used to control both the temporary fast poll mode while waiting for
// CheckInResponse commands and the actual fast poll mode that follows one or
// more positive CheckInResponse commands.  The endpoint event is used to send
// the periodic CheckIn commands.
//
// When it is time to check in, a new fast poll period begins with the
// selection of clients.  The clients are determined by scanning the binding
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

typedef struct {
  uint8_t bindingIndex;
  int16u fastPollTimeoutQs;
} Client_t;
static Client_t clients[CHIP_AF_PLUGIN_POLL_CONTROL_SERVER_MAX_CLIENTS];

enum {
  INITIAL = 0,
  WAITING = 1,
  POLLING = 2,
};
static uint8_t state = INITIAL;
static int32u fastPollStartTimeMs;

// Define a timeout for the client checkIn response.
#define CLIENT_CHECK_IN_TIMEOUT_DURATION_MS                      \
  (CHIP_AF_PLUGIN_POLL_CONTROL_SERVER_CHECK_IN_RESPONSE_TIMEOUT \
   * MILLISECOND_TICKS_PER_QUARTERSECOND)

#define CLIENT_INDEX_NULL 0xFF

ChipEventControl chZclPollControlServerCheckInEventControl;

// The cluster 'tick' is driven from an event so we can control when the tick fires.
ChipEventControl chZclPollControlServerTickEventControl;
void chZclPollControlServerTickEventHandler(void);

static ChipZclEndpointId_t serverEndpointId = CHIP_ZCL_ENDPOINT_NULL;

static bool readServerAttribute(ChipZclAttributeId_t attributeId,
                                void* buffer,
                                size_t size);
static ChipZclStatus_t writeServerAttribute(ChipZclAttributeId_t attributeId,
                                             void* buffer,
                                             size_t size);
static void scheduleServerTick(uint32_t delayMs);
static void deactivateServerTick(void);
static void scheduleServerCheckIn(void);
static uint8_t findClientIndex(const ChipZclCommandContext_t* context);
static bool pendingCheckInResponses(void);
static bool outstandingFastPollRequests(void);
static bool validateServerCheckInInterval(uint32_t newCheckInIntervalQs);
static bool validateServerLongPollInterval(uint32_t newLongPollIntervalQs);
static bool validateServerShortPollInterval(uint16_t newShortPollIntervalQs);
static bool validateServerFastPollTimeout(uint16_t newFastPollTimeoutQs);
static bool validateServerCheckInIntervalMin(uint32_t newCheckInIntervalMinQs);
static bool validateServerLongPollIntervalMin(uint32_t newLongPollIntervalMinQs);
static bool validateServerFastPollTimeoutMax(uint16_t newFastPollTimeoutMaxQs);

//------------------------------------------------------------------------------
// Private functions

static bool readServerAttribute(ChipZclAttributeId_t attributeId,
                                void* buffer,
                                size_t size)
{
  ChipZclStatus_t status;
  status = chipZclReadAttribute(serverEndpointId,
                                 &chipZclClusterPollControlServerSpec,
                                 attributeId,
                                 buffer,
                                 size);
  return (status == CHIP_ZCL_STATUS_SUCCESS);
}

static ChipZclStatus_t writeServerAttribute(ChipZclAttributeId_t attributeId,
                                             void* buffer,
                                             size_t size)
{
  ChipZclStatus_t status;
  status = chipZclWriteAttribute(serverEndpointId,
                                  &chipZclClusterPollControlServerSpec,
                                  attributeId,
                                  buffer,
                                  size);
  return status;
}

static void scheduleServerTick(uint32_t delayMs)
{
  chipEventControlSetDelayMS(chZclPollControlServerTickEventControl,
                              delayMs);
}

static void deactivateServerTick(void)
{
  chipEventControlSetInactive(chZclPollControlServerTickEventControl);
}

static void scheduleServerCheckIn(void)
{
  uint32_t checkInIntervalQs;

  if (readServerAttribute(CHIP_ZCL_CLUSTER_POLL_CONTROL_SERVER_ATTRIBUTE_CHECK_IN_INTERVAL,
                          (void*)&checkInIntervalQs,
                          sizeof(checkInIntervalQs))
      && (checkInIntervalQs != 0)) {
    chipEventControlSetDelayMS(chZclPollControlServerCheckInEventControl,
                                (checkInIntervalQs * MILLISECOND_TICKS_PER_QUARTERSECOND));
  } else {
    chipEventControlSetInactive(chZclPollControlServerCheckInEventControl);
  }
}

static uint8_t findClientIndex(const ChipZclCommandContext_t* context)
{
  uint8_t bindingIdx;

  if ((context->endpointId != serverEndpointId)
      || (!chipZclAreClusterSpecsEqual(&chipZclClusterPollControlServerSpec, context->clusterSpec)
          && !chipZclAreClusterSpecsEqual(&chipZclClusterPollControlClientSpec, context->clusterSpec))) {
    return CLIENT_INDEX_NULL; // Return if context does not refer to a Poll Control cluster (C or S) or the endpointId is wrong.
  }

  // Check if we have a binding table entry which matches the context.
  for (bindingIdx = 0; bindingIdx < CHIP_ZCL_BINDING_TABLE_SIZE; bindingIdx++) {
    ChipZclBindingEntry_t binding = { 0 };
    if (chipZclGetBinding(bindingIdx, &binding)) {
      if (chipZclAreClusterSpecsEqual(&chipZclClusterPollControlServerSpec, &binding.clusterSpec)) {
        // Check the context ep and remote address matches the binding.
        if ((binding.endpointId == serverEndpointId)
            && (MEMCOMPARE(&context->remoteAddress, &binding.destination.network.data.address, 16) == 0)) {
          break;
        }
      }
    }
  }

  if (bindingIdx < CHIP_ZCL_BINDING_TABLE_SIZE) {
    // Matching binding found, now check for a matching reference in the clients array.
    ChipZclBindingEntry_t binding = { 0 };
    if (chipZclGetBinding(bindingIdx, &binding)) {
      uint8_t clientIdx;
      for (clientIdx = 0; clientIdx < CHIP_AF_PLUGIN_POLL_CONTROL_SERVER_MAX_CLIENTS; clientIdx++) {
        ChipZclBindingEntry_t clientBinding;
        if (chipZclGetBinding(clients[clientIdx].bindingIndex, &clientBinding)) {
          if (MEMCOMPARE(&binding, &clientBinding, sizeof(ChipZclBindingEntry_t)) == 0) {
            return clientIdx;
          }
        }
      }
    }
  }

  return CLIENT_INDEX_NULL;
}

static bool pendingCheckInResponses(void)
{
  for (uint8_t i = 0; i < CHIP_AF_PLUGIN_POLL_CONTROL_SERVER_MAX_CLIENTS; i++) {
    if (clients[i].bindingIndex != CLIENT_INDEX_NULL
        && clients[i].fastPollTimeoutQs == 0) {
      return TRUE;
    }
  }
  return FALSE;
}

static bool outstandingFastPollRequests(void)
{
  uint32_t currentTimeMs = halCommonGetInt32uMillisecondTick();
  uint32_t elapsedFastPollTimeMs = elapsedTimeInt32u(fastPollStartTimeMs,
                                                     currentTimeMs);
  uint16_t fastPollTimeoutQs = 0;
  for (uint8_t i = 0; i < CHIP_AF_PLUGIN_POLL_CONTROL_SERVER_MAX_CLIENTS; i++) {
    if (clients[i].bindingIndex != CLIENT_INDEX_NULL) {
      if (clients[i].fastPollTimeoutQs * MILLISECOND_TICKS_PER_QUARTERSECOND
          < elapsedFastPollTimeMs) {
        clients[i].bindingIndex = CLIENT_INDEX_NULL;
      } else if (fastPollTimeoutQs < clients[i].fastPollTimeoutQs) {
        fastPollTimeoutQs = clients[i].fastPollTimeoutQs;
      }
    }
  }

  if (fastPollTimeoutQs == 0) {
    return FALSE;
  } else {
    uint32_t newFastPollEndTimeMs = (fastPollStartTimeMs
                                     + (fastPollTimeoutQs * MILLISECOND_TICKS_PER_QUARTERSECOND));
    uint32_t remainingFastPollTimeMs = elapsedTimeInt32u(currentTimeMs,
                                                         newFastPollEndTimeMs);

    scheduleServerTick(remainingFastPollTimeMs);
    return TRUE;
  }
}

static bool validateServerCheckInInterval(uint32_t newCheckInIntervalQs)
{
  if (newCheckInIntervalQs == 0) {
    return true;
  }

  uint32_t longPollIntervalQs;
  if (!readServerAttribute(CHIP_ZCL_CLUSTER_POLL_CONTROL_SERVER_ATTRIBUTE_LONG_POLL_INTERVAL,
                           (void*)&longPollIntervalQs,
                           sizeof(longPollIntervalQs))
      || (newCheckInIntervalQs < longPollIntervalQs)) {
    return false;
  }

  // Validate against optional CheckIn_Interval_Min attribute.
  if (chZclFindAttribute(&chipZclClusterPollControlServerSpec,
                         CHIP_ZCL_CLUSTER_POLL_CONTROL_SERVER_ATTRIBUTE_CHECK_IN_INTERVAL_MIN,
                         false)) { // exclude remote
    uint32_t checkInIntervalMinQs;
    if (!readServerAttribute(CHIP_ZCL_CLUSTER_POLL_CONTROL_SERVER_ATTRIBUTE_CHECK_IN_INTERVAL_MIN,
                             (void*)&checkInIntervalMinQs,
                             sizeof(checkInIntervalMinQs))
        || (newCheckInIntervalQs < checkInIntervalMinQs)) {
      return false;
    }
  }

  return true;
}

static bool validateServerLongPollInterval(uint32_t newLongPollIntervalQs)
{
  uint32_t checkInIntervalQs;
  if (!readServerAttribute(CHIP_ZCL_CLUSTER_POLL_CONTROL_SERVER_ATTRIBUTE_CHECK_IN_INTERVAL,
                           (void*)&checkInIntervalQs,
                           sizeof(checkInIntervalQs))
      || (newLongPollIntervalQs > checkInIntervalQs)) {
    return false;
  }

  uint16_t shortPollIntervalQs;
  if (!readServerAttribute(CHIP_ZCL_CLUSTER_POLL_CONTROL_SERVER_ATTRIBUTE_SHORT_POLL_INTERVAL,
                           (void*)&shortPollIntervalQs,
                           sizeof(shortPollIntervalQs))
      || (newLongPollIntervalQs < shortPollIntervalQs)) {
    return false;
  }

  // Validate against optional LongPoll_Interval_Min attribute.
  if (chZclFindAttribute(&chipZclClusterPollControlServerSpec,
                         CHIP_ZCL_CLUSTER_POLL_CONTROL_SERVER_ATTRIBUTE_LONG_POLL_INTERVAL_MIN,
                         false)) { // exclude remote
    uint32_t longPollIntervalMinQs;
    if (!readServerAttribute(CHIP_ZCL_CLUSTER_POLL_CONTROL_SERVER_ATTRIBUTE_LONG_POLL_INTERVAL_MIN,
                             (void*)&longPollIntervalMinQs,
                             sizeof(longPollIntervalMinQs))
        || (newLongPollIntervalQs < longPollIntervalMinQs)) {
      return false;
    }
  }

  return true;
}

static bool validateServerShortPollInterval(uint16_t newShortPollIntervalQs)
{
  uint32_t longPollIntervalQs;
  if (!readServerAttribute(CHIP_ZCL_CLUSTER_POLL_CONTROL_SERVER_ATTRIBUTE_LONG_POLL_INTERVAL,
                           (void*)&longPollIntervalQs,
                           sizeof(longPollIntervalQs))
      || (newShortPollIntervalQs > longPollIntervalQs)) {
    return false;
  }

  return true;
}

static bool validateServerFastPollTimeout(uint16_t newFastPollTimeoutQs)
{
  // Validate new timeout value against optional Fast Poll Timeout_Max attribute.
  if (chZclFindAttribute(&chipZclClusterPollControlServerSpec,
                         CHIP_ZCL_CLUSTER_POLL_CONTROL_SERVER_ATTRIBUTE_FAST_POLL_TIMEOUT_MAX,
                         false)) { // exclude remote
    uint16_t fastPollTimeoutMaxQs;
    if (!readServerAttribute(CHIP_ZCL_CLUSTER_POLL_CONTROL_SERVER_ATTRIBUTE_FAST_POLL_TIMEOUT_MAX,
                             (uint8_t*)&fastPollTimeoutMaxQs,
                             sizeof(fastPollTimeoutMaxQs))
        || (newFastPollTimeoutQs > fastPollTimeoutMaxQs)) {
      return false;
    }
  }

  return true;
}

static bool validateServerCheckInIntervalMin(uint32_t newCheckInIntervalMinQs)
{
  uint32_t checkInIntervalQs;
  if (!readServerAttribute(CHIP_ZCL_CLUSTER_POLL_CONTROL_SERVER_ATTRIBUTE_CHECK_IN_INTERVAL,
                           (void*)&checkInIntervalQs,
                           sizeof(checkInIntervalQs))
      || (newCheckInIntervalMinQs > checkInIntervalQs)) {
    return false;
  }

  return true;
}

static bool validateServerLongPollIntervalMin(uint32_t newLongPollIntervalMinQs)
{
  uint32_t longPollIntervalQs;
  if (!readServerAttribute(CHIP_ZCL_CLUSTER_POLL_CONTROL_SERVER_ATTRIBUTE_LONG_POLL_INTERVAL,
                           (void*)&longPollIntervalQs,
                           sizeof(longPollIntervalQs))
      || (newLongPollIntervalMinQs > longPollIntervalQs)) {
    return false;
  }

  return true;
}

static bool validateServerFastPollTimeoutMax(uint16_t newFastPollTimeoutMaxQs)
{
  uint16_t fastPollTimeoutQs;
  if (!readServerAttribute(CHIP_ZCL_CLUSTER_POLL_CONTROL_SERVER_ATTRIBUTE_FAST_POLL_TIMEOUT,
                           (void*)&fastPollTimeoutQs,
                           sizeof(fastPollTimeoutQs))
      || (newFastPollTimeoutMaxQs < fastPollTimeoutQs)) {
    return false;
  }

  return true;
}

//------------------------------------------------------------------------------
// Public functions

void chipZclPollControlServerInitHandler(void)
{
  // Set server endpoint to the first ep with a Poll Control Server cluster.
  for (ChipZclEndpointIndex_t i = 0; i < chZclEndpointCount; i++) {
    serverEndpointId
      = chipZclEndpointIndexToId(i, &chipZclClusterPollControlServerSpec);
    if (serverEndpointId != CHIP_ZCL_ENDPOINT_NULL) {
      break;
    }
  }
  if (serverEndpointId == CHIP_ZCL_ENDPOINT_NULL) {
    assert(false); // Fatal error, Poll Control Server cluster not found.
  }

  uint32_t longPollIntervalQs;
  if (readServerAttribute(CHIP_ZCL_CLUSTER_POLL_CONTROL_SERVER_ATTRIBUTE_LONG_POLL_INTERVAL,
                          (void*)&longPollIntervalQs,
                          sizeof(longPollIntervalQs))) {
    chipAfSetLongPollIntervalQsCallback(longPollIntervalQs);
  }
  uint16_t shortPollIntervalQs;
  if (readServerAttribute(CHIP_ZCL_CLUSTER_POLL_CONTROL_SERVER_ATTRIBUTE_SHORT_POLL_INTERVAL,
                          (void*)&shortPollIntervalQs,
                          sizeof(shortPollIntervalQs))) {
    chipAfSetShortPollIntervalQsCallback(shortPollIntervalQs);
  }
  uint16_t fastPollTimeoutQs;
  if (readServerAttribute(CHIP_ZCL_CLUSTER_POLL_CONTROL_SERVER_ATTRIBUTE_FAST_POLL_TIMEOUT,
                          (void*)&fastPollTimeoutQs,
                          sizeof(fastPollTimeoutQs))) {
    chipAfSetFastPollTimeoutQsCallback(fastPollTimeoutQs);
  }

  // TODO: Begin checking in after the network comes up instead of at startup.
  scheduleServerCheckIn();
}

void chZclPollControlServerTickEventHandler(void)
{
  if (state == WAITING) {
    uint16_t fastPollTimeoutQs = 0;
    for (uint8_t i = 0; i < CHIP_AF_PLUGIN_POLL_CONTROL_SERVER_MAX_CLIENTS; i++) {
      if ((clients[i].bindingIndex != CLIENT_INDEX_NULL)
          && (fastPollTimeoutQs < clients[i].fastPollTimeoutQs)) {
        fastPollTimeoutQs = clients[i].fastPollTimeoutQs;
      }
    }

    if (fastPollTimeoutQs != 0) {
      state = POLLING;
      fastPollStartTimeMs = halCommonGetInt32uMillisecondTick();
      scheduleServerTick(fastPollTimeoutQs * MILLISECOND_TICKS_PER_QUARTERSECOND);
      return;
    }
  }

  chipAfCorePrintln("End Polling");
  state = INITIAL;
  deactivateServerTick();
}

void chZclPollControlServerCheckInEventHandler(void)
{
  // Clear clients array.
  for (uint8_t i = 0; i < CHIP_AF_PLUGIN_POLL_CONTROL_SERVER_MAX_CLIENTS; i++) {
    clients[i].bindingIndex = CLIENT_INDEX_NULL;
  }

  // Send checkIn cmd to all matching Poll Control Client entries in binding table.
  uint8_t clientIdx = 0;
  for (uint8_t bindingIdx = 0; bindingIdx < CHIP_ZCL_BINDING_TABLE_SIZE; bindingIdx++) {
    ChipZclBindingEntry_t binding = { 0 };
    if (chipZclGetBinding(bindingIdx, &binding)) {
      if (chipZclAreClusterSpecsEqual(&chipZclClusterPollControlServerSpec,
                                       &binding.clusterSpec)) {
        ChipIpv6Address destClientIpv6;
        MEMCOPY(&destClientIpv6,
                &binding.destination.network.data.address,
                sizeof(ChipIpv6Address));
        ChipZclDestination_t destClientZcl = {
          .network = {
            .address = destClientIpv6,
            .flags = CHIP_ZCL_HAVE_IPV6_ADDRESS_FLAG,
          },
          .application = {
            .data.endpointId = binding.destination.application.data.endpointId,
            .type = CHIP_ZCL_APPLICATION_DESTINATION_TYPE_ENDPOINT,
          },
        };

        if (chipZclSendClusterPollControlClientCommandCheckInRequest(
              &destClientZcl,
              NULL,           // no request payload for checkIn cmd.
              (ChipZclClusterPollControlClientCommandCheckInResponseHandler)chipZclClusterPollControlServerCommandCheckInResponseHandler)
            == CHIP_SUCCESS) {
          clients[clientIdx].bindingIndex = bindingIdx;
          clients[clientIdx].fastPollTimeoutQs = 0;
          clientIdx++;
        }
      }
    }
  } // for

  if (clientIdx == 0) {
    state = INITIAL;
    deactivateServerTick();
  } else {
    state = WAITING;
    scheduleServerTick(CLIENT_CHECK_IN_TIMEOUT_DURATION_MS);
  }

  scheduleServerCheckIn();
}

bool chZclPollControlServerPreAttributeChangeHandler(ChipZclEndpointId_t endpointId,
                                                     const ChipZclClusterSpec_t* clusterSpec,
                                                     ChipZclAttributeId_t attributeId,
                                                     const void* buffer,
                                                     size_t bufferLength)
{
  if ((!chipZclAreClusterSpecsEqual(&chipZclClusterPollControlServerSpec, clusterSpec))
      || (endpointId != serverEndpointId)
      || (const uint8_t *)buffer == 0) {
    return true;
  }

  switch (attributeId) {
    case CHIP_ZCL_CLUSTER_POLL_CONTROL_SERVER_ATTRIBUTE_CHECK_IN_INTERVAL: {
      uint32_t newCheckInIntervalQs;
      MEMCOPY(&newCheckInIntervalQs, buffer, bufferLength);
      return validateServerCheckInInterval(newCheckInIntervalQs);
    }
    case CHIP_ZCL_CLUSTER_POLL_CONTROL_SERVER_ATTRIBUTE_LONG_POLL_INTERVAL: {
      uint32_t newLongPollIntervalQs;
      MEMCOPY(&newLongPollIntervalQs, buffer, bufferLength);
      return validateServerLongPollInterval(newLongPollIntervalQs);
    }
    case CHIP_ZCL_CLUSTER_POLL_CONTROL_SERVER_ATTRIBUTE_SHORT_POLL_INTERVAL: {
      uint16_t newShortPollIntervalQs;
      MEMCOPY(&newShortPollIntervalQs, buffer, bufferLength);
      return validateServerShortPollInterval(newShortPollIntervalQs);
    }
    case CHIP_ZCL_CLUSTER_POLL_CONTROL_SERVER_ATTRIBUTE_FAST_POLL_TIMEOUT: {
      uint16_t newFastPollTimeoutQs;
      MEMCOPY(&newFastPollTimeoutQs, buffer, bufferLength);
      return validateServerFastPollTimeout(newFastPollTimeoutQs);
    }
    case CHIP_ZCL_CLUSTER_POLL_CONTROL_SERVER_ATTRIBUTE_CHECK_IN_INTERVAL_MIN: {
      uint32_t newCheckInIntervalMinQs;
      MEMCOPY(&newCheckInIntervalMinQs, buffer, bufferLength);
      return validateServerCheckInIntervalMin(newCheckInIntervalMinQs);
    }
    case CHIP_ZCL_CLUSTER_POLL_CONTROL_SERVER_ATTRIBUTE_LONG_POLL_INTERVAL_MIN: {
      uint32_t newLongPollIntervalMinQs;
      MEMCOPY(&newLongPollIntervalMinQs, buffer, bufferLength);
      return validateServerLongPollIntervalMin(newLongPollIntervalMinQs);
    }
    case CHIP_ZCL_CLUSTER_POLL_CONTROL_SERVER_ATTRIBUTE_FAST_POLL_TIMEOUT_MAX: {
      uint32_t newFastPollTimeoutMaxQs;
      MEMCOPY(&newFastPollTimeoutMaxQs, buffer, bufferLength);
      return validateServerFastPollTimeoutMax(newFastPollTimeoutMaxQs);
    }
    default: {
      break;
    }
  } // switch

  return true; // attribute not found.
}

void chZclPollControlServerPostAttributeChangeHandler(ChipZclEndpointId_t endpointId,
                                                      const ChipZclClusterSpec_t* clusterSpec,
                                                      ChipZclAttributeId_t attributeId,
                                                      const void* buffer,
                                                      size_t bufferLength)
{
  if (endpointId != serverEndpointId) {
    return;
  }

  switch (attributeId) {
    case CHIP_ZCL_CLUSTER_POLL_CONTROL_SERVER_ATTRIBUTE_CHECK_IN_INTERVAL: {
      scheduleServerCheckIn();
      break;
    }
    case CHIP_ZCL_CLUSTER_POLL_CONTROL_SERVER_ATTRIBUTE_LONG_POLL_INTERVAL: {
      uint32_t longPollIntervalQs;
      if (readServerAttribute(CHIP_ZCL_CLUSTER_POLL_CONTROL_SERVER_ATTRIBUTE_LONG_POLL_INTERVAL,
                              (void*)&longPollIntervalQs,
                              sizeof(longPollIntervalQs))) {
        chipAfSetLongPollIntervalQsCallback(longPollIntervalQs);
      }
      break;
    }
    case CHIP_ZCL_CLUSTER_POLL_CONTROL_SERVER_ATTRIBUTE_SHORT_POLL_INTERVAL: {
      uint16_t shortPollIntervalQs;
      if (readServerAttribute(CHIP_ZCL_CLUSTER_POLL_CONTROL_SERVER_ATTRIBUTE_SHORT_POLL_INTERVAL,
                              (void*)&shortPollIntervalQs,
                              sizeof(shortPollIntervalQs))) {
        chipAfSetShortPollIntervalQsCallback(shortPollIntervalQs);
      }
      break;
    }
    case CHIP_ZCL_CLUSTER_POLL_CONTROL_SERVER_ATTRIBUTE_FAST_POLL_TIMEOUT: {
      uint16_t fastPollTimeoutQs;
      if (readServerAttribute(CHIP_ZCL_CLUSTER_POLL_CONTROL_SERVER_ATTRIBUTE_FAST_POLL_TIMEOUT,
                              (void*)&fastPollTimeoutQs,
                              sizeof(fastPollTimeoutQs))) {
        chipAfSetShortPollIntervalQsCallback(fastPollTimeoutQs);
      }
      break;
    }
    default: {
      break;
    }
  }
}

void chipZclClusterPollControlServerCommandCheckInResponseHandler(ChipZclMessageStatus_t status,
                                                                   const ChipZclCommandContext_t* context,
                                                                   const ChipZclClusterPollControlClientCommandCheckInResponse_t* response)

{
  if (status == CHIP_ZCL_MESSAGE_STATUS_COAP_RESPONSE) {
    chipAfCorePrintln("RX: CheckInResponse 0x%x, 0x%2x", response->startFastPolling, response->fastPollTimeout);

    uint8_t startFastPolling = response->startFastPolling;
    uint16_t fastPollTimeoutQs = response->fastPollTimeout;

    uint8_t clientIndex = findClientIndex(context);

    if (clientIndex != CLIENT_INDEX_NULL) {
      if (state == WAITING) {
        if (startFastPolling) {
          ChipZclStatus_t zclStatus = CHIP_ZCL_STATUS_FAILURE;
          if (fastPollTimeoutQs == 0) {
            if (readServerAttribute(CHIP_ZCL_CLUSTER_POLL_CONTROL_SERVER_ATTRIBUTE_FAST_POLL_TIMEOUT,
                                    (void*)&fastPollTimeoutQs,
                                    sizeof(fastPollTimeoutQs))) {
              zclStatus = CHIP_ZCL_STATUS_SUCCESS;
            }
          } else if (validateServerFastPollTimeout(fastPollTimeoutQs)) {
            zclStatus = CHIP_ZCL_STATUS_SUCCESS;
          }
          if (zclStatus == CHIP_ZCL_STATUS_SUCCESS) {
            clients[clientIndex].fastPollTimeoutQs = fastPollTimeoutQs;
          } else {
            clients[clientIndex].bindingIndex = CLIENT_INDEX_NULL;
          }
        } else {
          clients[clientIndex].bindingIndex = CLIENT_INDEX_NULL;
        }

        // Calling the tick directly when in the waiting state will cause the
        // temporarily fast poll mode to stop and will begin the actual fast poll
        // mode if applicable.
        if (!pendingCheckInResponses()) {
          chZclPollControlServerTickEventHandler();
        }
      }
    }
  }
}

void chipZclClusterPollControlServerCommandFastPollStopRequestHandler(const ChipZclCommandContext_t* context,
                                                                       const ChipZclClusterPollControlServerCommandFastPollStopRequest_t* request)
{
  ChipZclStatus_t status = CHIP_ZCL_STATUS_ACTION_DENIED;

  chipAfCorePrintln("RX: FastPollStop");

  uint8_t clientIndex = findClientIndex(context);
  if (clientIndex != CLIENT_INDEX_NULL) {
    if (state == POLLING) {
      status = CHIP_ZCL_STATUS_SUCCESS;
      clients[clientIndex].bindingIndex = CLIENT_INDEX_NULL;

      // Calling the tick directly in the polling state will cause the fast
      // poll mode to stop.
      if (!outstandingFastPollRequests()) {
        chZclPollControlServerTickEventHandler();
      }
    } else {
      status = CHIP_ZCL_STATUS_TIMEOUT;
    }
  }

  chipZclSendDefaultResponse(context, status);
}

void chipZclClusterPollControlServerCommandSetLongPollIntervalRequestHandler(const ChipZclCommandContext_t* context,
                                                                              const ChipZclClusterPollControlServerCommandSetLongPollIntervalRequest_t* request)
{
#ifdef CHIP_AF_PLUGIN_POLL_CONTROL_SERVER_ACCEPT_SET_LONG_POLL_INTERVAL_COMMAND
  ChipZclStatus_t status = CHIP_ZCL_STATUS_UNSUPPORTED_ATTRIBUTE;

  if (context->endpointId == serverEndpointId) {
    uint32_t newLongPollIntervalQs = request->newLongPollInterval;

    chipAfCorePrintln("RX: SetLongPollInterval 0x%4x", newLongPollIntervalQs);

    // Trying to write the attribute will trigger the PreAttributeChanged
    // callback, which will handle validation.  If the write is successful, the
    // PostAttributeChanged callback will fire, which will handle setting the new
    // long poll interval.
    status = writeServerAttribute(CHIP_ZCL_CLUSTER_POLL_CONTROL_SERVER_ATTRIBUTE_LONG_POLL_INTERVAL,
                                  (void*)&newLongPollIntervalQs,
                                  sizeof(newLongPollIntervalQs));
  }

  chipZclSendDefaultResponse(context, status);
#endif
}

void chipZclClusterPollControlServerCommandSetShortPollIntervalRequestHandler(const ChipZclCommandContext_t* context,
                                                                               const ChipZclClusterPollControlServerCommandSetShortPollIntervalRequest_t* request)
{
#ifdef CHIP_AF_PLUGIN_POLL_CONTROL_SERVER_ACCEPT_SET_SHORT_POLL_INTERVAL_COMMAND
  ChipZclStatus_t status = CHIP_ZCL_STATUS_UNSUPPORTED_ATTRIBUTE;

  if (context->endpointId == serverEndpointId) {
    uint16_t newShortPollIntervalQs = request->newShortPollInterval;

    chipAfCorePrintln("RX: SetShortPollInterval 0x%2x", newShortPollIntervalQs);

    // Trying to write the attribute will trigger the PreAttributeChanged
    // callback, which will handle validation.  If the write is successful, the
    // AttributeChanged callback will fire, which will handle setting the new
    // short poll interval.
    status = writeServerAttribute(CHIP_ZCL_CLUSTER_POLL_CONTROL_SERVER_ATTRIBUTE_SHORT_POLL_INTERVAL,
                                  (void*)&newShortPollIntervalQs,
                                  sizeof(newShortPollIntervalQs));
  }

  chipZclSendDefaultResponse(context, status);
#endif
}
