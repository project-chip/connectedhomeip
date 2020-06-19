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
 * @brief This defines the state machine and procedure for a newly joined device to
 * perform the necessary steps to register in a network.
 *******************************************************************************
   ******************************************************************************/

#include "app/framework/include/af.h"
#include "app/framework/util/af-main.h"                     //emberAfIsFullSmartEnergySecurityPresent
#include "app/util/zigbee-framework/zigbee-device-common.h" //emberBindRequest
#ifdef EZSP_HOST                                            //emberIeeeAddressRequest
  #include "app/util/zigbee-framework/zigbee-device-host.h"
#else //EZSP_HOST
  #include "stack/include/ember.h"
#endif //EZSP_HOST

#include "smart-energy-registration.h"
#include "app/framework/plugin/test-harness/test-harness.h"

#include "app/framework/plugin/esi-management/esi-management.h"

// A helper macro for a human readable TimeStatus flags check.
// Usage e.g.: if (IS_TIME_STATUS(bla, MASTER_CLOCK) { ... }
// Permitted values for 'mask' are defined in enums.h, under EMBER_AF_TIME_STATUS_MASK_...
#define IS_TIME_STATUS(reg, mask) ((reg) & EMBER_AF_TIME_STATUS_MASK_##mask)

extern EmberEventControl emberAfPluginSmartEnergyRegistrationTickNetworkEventControls[1];
void emberAfPluginSmartEnergyRegistrationTickNetworkEventHandler(void);

#if defined(EMBER_AF_PLUGIN_SMART_ENERGY_REGISTRATION_ESI_DISCOVERY_REQUIRED)
  #define PARTNER_KE_REQUIRED
  #define NEXT_STATE_AFTER_KE           STATE_DISCOVER_ENERGY_SERVICE_INTERFACES
  #define NEXT_STATE_AFTER_PARTNER_KE   STATE_PERFORM_BINDING
#elif defined(EMBER_AF_PLUGIN_SMART_ENERGY_REGISTRATION_ALLOW_NON_ESI_TIME_SERVERS)
  #define PARTNER_KE_REQUIRED
  #define NEXT_STATE_AFTER_KE           STATE_DISCOVER_TIME_SERVERS
  #define NEXT_STATE_AFTER_PARTNER_KE   STATE_DETERMINE_AUTHORITATIVE_TIME_SOURCE
#else
  #define NEXT_STATE_AFTER_KE           STATE_REGISTRATION_COMPLETE
#endif

typedef enum {
  STATE_INITIAL,
  STATE_DISCOVER_KEY_ESTABLISHMENT_CLUSTER,
  STATE_PERFORM_KEY_ESTABLISHMENT,
#ifdef EMBER_AF_PLUGIN_SMART_ENERGY_REGISTRATION_ESI_DISCOVERY_REQUIRED
  STATE_DISCOVER_ENERGY_SERVICE_INTERFACES,
#endif
#ifdef EMBER_AF_PLUGIN_SMART_ENERGY_REGISTRATION_ALLOW_NON_ESI_TIME_SERVERS
  // If non-ESIs are allowed to be time servers, then we neeed to discover them
  // separately. Otherwise we use the discovered ESIs as time server candidates.
  STATE_DISCOVER_TIME_SERVERS,
#endif
#ifdef PARTNER_KE_REQUIRED
  STATE_DISCOVER_IEEE_ADDRESSES,
  STATE_PERFORM_PARTNER_LINK_KEY_EXCHANGE,
#endif
#ifdef EMBER_AF_PLUGIN_SMART_ENERGY_REGISTRATION_ESI_DISCOVERY_REQUIRED
  STATE_PERFORM_BINDING,
#endif
#ifdef EMBER_AF_PLUGIN_SMART_ENERGY_REGISTRATION_TIME_SOURCE_REQUIRED
  STATE_DETERMINE_AUTHORITATIVE_TIME_SOURCE,
#endif
  STATE_REGISTRATION_COMPLETE,
  STATE_REGISTRATION_FAILED,
} EmAfPluginSmartEnergyRegistrationState;

#define UNDEFINED_ENDPOINT 0xFF

#ifdef EMBER_AF_PLUGIN_SMART_ENERGY_REGISTRATION_TIME_SOURCE_REQUIRED
typedef struct {
  bool valid;
  EmberNodeId nodeId;
  uint32_t  time;
  uint8_t   timeStatus;
  uint32_t  lastSetTime;
  uint32_t  validUntilTime;
} EmAfPluginSmartEnergyRegistrationTimeSource;
#endif //EMBER_AF_PLUGIN_SMART_ENERGY_REGISTRATION_TIME_SOURCE_REQUIRED

#ifdef EMBER_AF_PLUGIN_SMART_ENERGY_REGISTRATION_ESI_DISCOVERY_REQUIRED

#ifdef EMBER_AF_PLUGIN_SMART_ENERGY_REGISTRATION_DELAY_PERIOD
uint32_t emAfPluginSmartEnergyRegistrationDiscoveryPeriod =
  EMBER_AF_PLUGIN_SMART_ENERGY_REGISTRATION_DELAY_PERIOD;
#endif //EMBER_AF_PLUGIN_SMART_ENERGY_REGISTRATION_DELAY_PERIOD

#ifndef SE_PROFILE_ID
  #define SE_PROFILE_ID 0x0109
#endif

#define UNDEFINED_CLUSTER_ID 0xFFFF
static EmberAfClusterId clusterList[] = {
#ifdef ZCL_USING_PRICE_CLUSTER_CLIENT
  ZCL_PRICE_CLUSTER_ID,
#endif //ZCL_USING_PRICE_CLUSTER_CLIENT

#ifdef ZCL_USING_DEMAND_RESPONSE_LOAD_CONTROL_CLUSTER_CLIENT
  ZCL_DEMAND_RESPONSE_LOAD_CONTROL_CLUSTER_ID,
#endif //ZCL_USING_DEMAND_RESPONSE_LOAD_CONTROL_CLUSTER_CLIENT

#ifdef ZCL_USING_SIMPLE_METERING_CLUSTER_CLIENT
  ZCL_SIMPLE_METERING_CLUSTER_ID,
#endif //ZCL_USING_SIMPLE_METERING_CLUSTER_CLIENT

#ifdef ZCL_USING_MESSAGING_CLUSTER_CLIENT
  ZCL_MESSAGING_CLUSTER_ID,
#endif //ZCL_USING_MESSAGING_CLUSTER_CLIENT

#ifdef ZCL_USING_TUNNELING_CLUSTER_CLIENT
  ZCL_TUNNELING_CLUSTER_ID,
#endif //ZCL_USING_TUNNELING_CLUSTER_CLIENT

#ifdef ZCL_USING_PREPAYMENT_CLUSTER_CLIENT
  ZCL_PREPAYMENT_CLUSTER_ID,
#endif //ZCL_USING_PREPAYMENT_CLUSTER_CLIENT

#ifdef ZCL_USING_CALENDAR_CLUSTER_CLIENT
  ZCL_CALENDAR_CLUSTER_ID,
#endif //ZCL_USING_CALENDAR_CLUSTER_CLIENT

#ifdef ZCL_USING_DEVICE_MANAGEMENT_CLUSTER_CLIENT
  ZCL_DEVICE_MANAGEMENT_CLUSTER_ID,
#endif //ZCL_USING_DEVICE_MANAGEMENT_CLUSTER_CLIENT

#ifdef ZCL_USING_EVENTS_CLUSTER_CLIENT
  ZCL_EVENTS_CLUSTER_ID,
#endif //ZCL_USING_EVENTS_CLUSTER_CLIENT

  UNDEFINED_CLUSTER_ID,
};

#endif //EMBER_AF_PLUGIN_SMART_ENERGY_REGISTRATION_ESI_DISCOVERY_REQUIRED

static bool checkErrorCountAndSetEventControl(uint32_t delayMs,
                                              bool errorFlag,
                                              bool resetErrorCount);
static bool resumeAfterDelay(EmberStatus status, uint32_t delayMs);

#define resumeAfterFixedDelay(status) \
  resumeAfterDelay((status),          \
                   EMBER_AF_PLUGIN_SMART_ENERGY_REGISTRATION_DELAY_RETRY)

#define transition(next) transitionAfterDelay((next),                                                     \
                                              EMBER_AF_PLUGIN_SMART_ENERGY_REGISTRATION_DELAY_TRANSITION, \
                                              true)     // reset error count
static bool transitionAfterDelay(EmAfPluginSmartEnergyRegistrationState next,
                                 uint32_t delay,
                                 bool resetErrorCount);

static void performKeyEstablishment(void);

#ifdef PARTNER_KE_REQUIRED
static void performPartnerLinkKeyExchange(void);
static void partnerLinkKeyExchangeCallback(bool success);
#endif
#ifdef EMBER_AF_PLUGIN_SMART_ENERGY_REGISTRATION_ESI_DISCOVERY_REQUIRED
static void performBinding(void);
#endif
#ifdef EMBER_AF_PLUGIN_SMART_ENERGY_REGISTRATION_TIME_SOURCE_REQUIRED
static void determineAuthoritativeTimeSource(void);
#endif

static void stopRegistration(bool success);

static void performDiscovery(void);
static void discoveryCallback(const EmberAfServiceDiscoveryResult *result);

#ifdef EMBER_AF_PLUGIN_SMART_ENERGY_REGISTRATION_ALLOW_NON_ESI_TIME_SERVERS
  #define MAX_TIME_SOURCE_CANDIDATES    4
#endif

typedef struct {
  EmAfPluginSmartEnergyRegistrationState state;
  uint8_t errors;
  uint8_t trustCenterKeyEstablishmentEndpoint;
#ifdef EMBER_AF_PLUGIN_SMART_ENERGY_REGISTRATION_ESI_DISCOVERY_REQUIRED
  EmberAfPluginEsiManagementEsiEntry *esiEntry;
  uint8_t endpointIndex; // performBinding
  uint8_t clusterIndex;  // performBinding
#endif
#ifdef EMBER_AF_PLUGIN_SMART_ENERGY_REGISTRATION_ALLOW_NON_ESI_TIME_SERVERS
  struct {
    EmberNodeId nodeId;
    uint8_t endpoint;
  } timeSourceCandidates[MAX_TIME_SOURCE_CANDIDATES];
  uint8_t totalCandidates;
  uint8_t currentCandidate;
#endif
#ifdef EMBER_AF_PLUGIN_SMART_ENERGY_REGISTRATION_TIME_SOURCE_REQUIRED
  bool resuming;    // determineAuthoritativeTimeSource
  EmAfPluginSmartEnergyRegistrationTimeSource source;
#endif
} State;
static State states[EMBER_SUPPORTED_NETWORKS];

#ifdef EMBER_AF_PLUGIN_SMART_ENERGY_REGISTRATION_ALLOW_NON_ESI_TIME_SERVERS
  #define TIME_SOURCE_CANDIDATE(state)  (state)->timeSourceCandidates[(state)->currentCandidate]
#else
  #define TIME_SOURCE_CANDIDATE(state)  (*(state)->esiEntry)
#endif

//------------------------------------------------------------------------------

EmberStatus emberAfRegistrationStartCallback(void)
{
  State *state = &states[emberGetCurrentNetwork()];

  if (!emberAfIsCurrentSecurityProfileSmartEnergy()) {
    return EMBER_INVALID_CALL;
  }

  if (state->state == STATE_REGISTRATION_COMPLETE) {
    // If we got called again after registration has already completed,
    // this means that it was due to a rejoin.  The trust center keepalive
    // may have initiated this rejoin due to a TC failure.
    emberAfTrustCenterKeepaliveUpdateCallback(true);
    return EMBER_SUCCESS;
  }

  if (state->state != STATE_INITIAL) {
    return EMBER_INVALID_CALL;
  }

  if (!emAfTestHarnessAllowRegistration) {
    return EMBER_SECURITY_CONFIGURATION_INVALID;
  }

  // Registration is unnecessary for the trust center.  For other nodes, wait
  // for the network broadcast traffic to die down and neighbor information to
  // be populated before continuing.
  if (emberAfGetNodeId() == EMBER_TRUST_CENTER_NODE_ID) {
    transition(STATE_REGISTRATION_COMPLETE);
  } else {
    transitionAfterDelay(STATE_DISCOVER_KEY_ESTABLISHMENT_CLUSTER,
                         EMBER_AF_PLUGIN_SMART_ENERGY_REGISTRATION_DELAY_INITIAL,
                         true);
  }
  emberAfAddToCurrentAppTasks(EMBER_AF_WAITING_FOR_REGISTRATION);
  return EMBER_SUCCESS;
}

void emberAfRegistrationAbortCallback(void)
{
  if (emberAfIsCurrentSecurityProfileSmartEnergy()) {
    // We need registration to stop immediately because it may be started up
    // again in the same call chain.
    State *state = &states[emberGetCurrentNetwork()];
    state->state = STATE_REGISTRATION_FAILED;
    emberAfPluginSmartEnergyRegistrationTickNetworkEventHandler();
  }
}

void emberAfPluginSmartEnergyRegistrationTickNetworkEventHandler(void)
{
  State *state = &states[emberGetCurrentNetwork()];
  switch (state->state) {
    case STATE_INITIAL:
      emberAfRegistrationStartCallback();
      break;
    case STATE_DISCOVER_KEY_ESTABLISHMENT_CLUSTER:
      performDiscovery();
      break;
    case STATE_PERFORM_KEY_ESTABLISHMENT:
      performKeyEstablishment();
      break;
#ifdef EMBER_AF_PLUGIN_SMART_ENERGY_REGISTRATION_ESI_DISCOVERY_REQUIRED
    case STATE_DISCOVER_ENERGY_SERVICE_INTERFACES:
      performDiscovery();
      break;
#endif
#ifdef EMBER_AF_PLUGIN_SMART_ENERGY_REGISTRATION_ALLOW_NON_ESI_TIME_SERVERS
    case STATE_DISCOVER_TIME_SERVERS:
      performDiscovery();
      break;
#endif
#ifdef PARTNER_KE_REQUIRED
    case STATE_DISCOVER_IEEE_ADDRESSES:
      performDiscovery();
      break;
    case STATE_PERFORM_PARTNER_LINK_KEY_EXCHANGE:
      performPartnerLinkKeyExchange();
      break;
#endif
#ifdef EMBER_AF_PLUGIN_SMART_ENERGY_REGISTRATION_ESI_DISCOVERY_REQUIRED
    case STATE_PERFORM_BINDING:
      performBinding();
      break;
#endif
#ifdef EMBER_AF_PLUGIN_SMART_ENERGY_REGISTRATION_TIME_SOURCE_REQUIRED
    case STATE_DETERMINE_AUTHORITATIVE_TIME_SOURCE:
      determineAuthoritativeTimeSource();
      break;
#endif
    case STATE_REGISTRATION_COMPLETE:
    // FALLTHROUGH
    case STATE_REGISTRATION_FAILED:
      stopRegistration(state->state == STATE_REGISTRATION_COMPLETE);
      break;
    default:
      emberAfRegistrationPrintln("ERR: Invalid state (0x%x)", state->state);
      emberAfRegistrationAbortCallback();
      break;
  }
}

uint8_t emAfPluginSmartEnergyRegistrationTrustCenterKeyEstablishmentEndpoint(void)
{
  // When we start, the key establishment endpoint will be zero.  This is okay
  // internally in this plugin, but we really want to use a better "undefined"
  // value for external modules.
  State *state = &states[emberGetCurrentNetwork()];
  if (state->trustCenterKeyEstablishmentEndpoint == 0x00) {
    state->trustCenterKeyEstablishmentEndpoint = UNDEFINED_ENDPOINT;
  }
  return state->trustCenterKeyEstablishmentEndpoint;
}

static bool checkErrorCountAndSetEventControl(uint32_t delayMs,
                                              bool errorFlag,
                                              bool resetErrorCount)
{
  State *state = &states[emberGetCurrentNetwork()];

  // Increment the error count if we're delaying due to an error; otherwise,
  // reset the error count so that failures in any particular state don't affect
  // subsequent states.
  if (errorFlag) {
    state->errors++;
    emberAfRegistrationPrintln("Registration error count %d of %d",
                               state->errors,
                               EMBER_AF_PLUGIN_SMART_ENERGY_REGISTRATION_ERROR_LIMIT);
  } else if (resetErrorCount) {
    state->errors = 0;
  } else {
    // MISRA nop
  }

  if (state->errors >= EMBER_AF_PLUGIN_SMART_ENERGY_REGISTRATION_ERROR_LIMIT) {
    emberAfRegistrationFlush();
    emberAfRegistrationPrintln("ERR: Aborting registration"
                               " because error limit reached (%d)",
                               EMBER_AF_PLUGIN_SMART_ENERGY_REGISTRATION_ERROR_LIMIT);
    emberAfRegistrationAbortCallback();
    return false;
  }

  emberAfNetworkEventControlSetDelayMS(emberAfPluginSmartEnergyRegistrationTickNetworkEventControls,
                                       delayMs);
  return true;
}

static bool resumeAfterDelay(EmberStatus status, uint32_t delayMs)
{
  const bool errorFlag = (status != EMBER_SUCCESS);
  return checkErrorCountAndSetEventControl((errorFlag
                                            ? delayMs
                                            : EMBER_AF_PLUGIN_SMART_ENERGY_REGISTRATION_DELAY_RESUME),
                                           errorFlag,
                                           false);
}

static bool transitionAfterDelay(EmAfPluginSmartEnergyRegistrationState next,
                                 uint32_t delay,
                                 bool resetErrorCount)
{
  State *state = &states[emberGetCurrentNetwork()];
  state->state = next;
  return checkErrorCountAndSetEventControl(delay, false, resetErrorCount);
}

static void performKeyEstablishment(void)
{
  State *state = &states[emberGetCurrentNetwork()];
  EmberStatus status;
  emberAfRegistrationPrintln("Performing key establishment");

  // Transient failures may prevent us from performing key establishment.  If
  // so, we will try again later.
  status = emberAfInitiateKeyEstablishment(EMBER_TRUST_CENTER_NODE_ID,
                                           state->trustCenterKeyEstablishmentEndpoint);
  if (status != EMBER_SUCCESS) {
    emberAfRegistrationPrintln("ERR: Failed to start key establishment (0x%x)",
                               status);
    resumeAfterFixedDelay(status);
  }
}

bool emberAfKeyEstablishmentCallback(EmberAfKeyEstablishmentNotifyMessage status,
                                     bool amInitiator,
                                     EmberNodeId partnerShortId,
                                     uint8_t delayInSeconds)
{
  State *state = &states[emberGetCurrentNetwork()];

  // The notification only matters if we are performing key establishment.
  if (state->state == STATE_PERFORM_KEY_ESTABLISHMENT) {
    if (status == LINK_KEY_ESTABLISHED) {
      transition(NEXT_STATE_AFTER_KE);
    } else if (status >= APP_NOTIFY_ERROR_CODE_START) {
      uint32_t delayMs = delayInSeconds * MILLISECOND_TICKS_PER_SECOND;
      emberAfRegistrationPrintln("ERR: Key establishment failed (0x%x)", status);
      resumeAfterDelay(EMBER_ERR_FATAL,
                       delayMs);
    }
  }

  // Always allow key establishment to continue.
  return true;
}

#ifdef PARTNER_KE_REQUIRED
// This function deserves an explanation.
// It is a helper whose purpose is to find the next suitable candidate to query
// the IEEE address from or perform a partner Key Establishment with. Depending
// on compilation options, the candidate can be chosen from the list of ESIs,
// the list of discovered Time servers, or both. If both, we try to take care
// of avoiding duplicates.
// It could be called in two different contexts. One, before a discovery starts,
// in which case the first located candidate is used.
// The second context is within a callback, when we want to advance to the next
// suitable candidate and skip the current one. Hence the 'skipCurrent' argument.
static bool findNextNodeForDiscoveryOrPartnerKeyExchange(State *state,
                                                         bool skipCurrent)
{
#ifdef EMBER_AF_PLUGIN_SMART_ENERGY_REGISTRATION_ESI_DISCOVERY_REQUIRED
  while (state->esiEntry != NULL) {
    assert(state->esiEntry->nodeId != EMBER_NULL_NODE_ID);
    if (skipCurrent
        || state->esiEntry->nodeId == EMBER_TRUST_CENTER_NODE_ID) {
      // Skip the Trust Center: we have already done KE with it
      state->esiEntry = emberAfPluginEsiManagementGetNextEntry(state->esiEntry, 0);
      skipCurrent = false;
      continue;
    }
    return true;
  }
#endif
#ifdef EMBER_AF_PLUGIN_SMART_ENERGY_REGISTRATION_ALLOW_NON_ESI_TIME_SERVERS
  while (state->currentCandidate < state->totalCandidates) {
    if (skipCurrent
        || state->timeSourceCandidates[state->currentCandidate].nodeId == EMBER_TRUST_CENTER_NODE_ID
  #ifdef EMBER_AF_PLUGIN_SMART_ENERGY_REGISTRATION_ESI_DISCOVERY_REQUIRED
        || emberAfPluginEsiManagementEsiLookUpByShortIdAndEndpoint(state->timeSourceCandidates[state->currentCandidate].nodeId,
                                                                   state->timeSourceCandidates[state->currentCandidate].endpoint) != NULL
  #endif
        ) {
      // Skip TC and ESI nodes; we have already done partner KE with them
      state->currentCandidate++;
      skipCurrent = false;
      continue;
    }
    return true;
  }
#endif
  return false;
}

static bool retrieveNodeForDiscoveryOrPartnerKeyExchange(const State *state,
                                                         EmberNodeId *nodeId,
                                                         uint8_t *endpoint)
{
#ifdef EMBER_AF_PLUGIN_SMART_ENERGY_REGISTRATION_ESI_DISCOVERY_REQUIRED
  if (state->esiEntry != NULL) {
    *nodeId = state->esiEntry->nodeId;
    *endpoint = state->esiEntry->endpoint;
    return true;
  }
#endif
#ifdef EMBER_AF_PLUGIN_SMART_ENERGY_REGISTRATION_ALLOW_NON_ESI_TIME_SERVERS
  if (state->currentCandidate < state->totalCandidates) {
    *nodeId = state->timeSourceCandidates[state->currentCandidate].nodeId;
    *endpoint = state->timeSourceCandidates[state->currentCandidate].endpoint;
    return true;
  }
#endif
  return false;
}

static void performPartnerLinkKeyExchange(void)
{
  State *state = &states[emberGetCurrentNetwork()];
  if (findNextNodeForDiscoveryOrPartnerKeyExchange(state, false)) {
    EmberNodeId nodeId;
    uint8_t endpoint;
    EmberStatus status;

    retrieveNodeForDiscoveryOrPartnerKeyExchange(state, &nodeId, &endpoint);
    emberAfRegistrationPrintln("Perform%p partner link key exchange"
                               " with node 0x%2x endpoint 0x%x",
                               "ing",
                               nodeId,
                               endpoint);
    status = emberAfInitiatePartnerLinkKeyExchange(nodeId,
                                                   endpoint,
                                                   partnerLinkKeyExchangeCallback);
    if (status != EMBER_SUCCESS) {
      emberAfRegistrationPrintln("ERR: Failed to %p partner link key request"
                                 " with node 0x%2x endpoint 0x%x (0x%x)",
                                 "initiate",
                                 nodeId,
                                 endpoint,
                                 status);
      resumeAfterFixedDelay(status);
    }
  } else {
    // No more ESIs to partner KE with, move on to the next step.
#ifdef EMBER_AF_PLUGIN_SMART_ENERGY_REGISTRATION_ESI_DISCOVERY_REQUIRED
    state->esiEntry = emberAfPluginEsiManagementGetNextEntry(NULL, 0);
#endif
#ifdef EMBER_AF_PLUGIN_SMART_ENERGY_REGISTRATION_ALLOW_NON_ESI_TIME_SERVERS
    state->currentCandidate = 0;
#endif
    transition(NEXT_STATE_AFTER_PARTNER_KE);
  }
}

static void partnerLinkKeyExchangeCallback(bool success)
{
  State *state = &states[emberGetCurrentNetwork()];
  EmberNodeId nodeId;
  uint8_t endpoint;

  if (state->state == STATE_PERFORM_PARTNER_LINK_KEY_EXCHANGE
      && retrieveNodeForDiscoveryOrPartnerKeyExchange(state, &nodeId, &endpoint)) {
    if (success) {
      emberAfRegistrationPrintln("Perform%p partner link key exchange"
                                 " with node 0x%2x endpoint 0x%x",
                                 "ed",
                                 nodeId,
                                 endpoint);
      findNextNodeForDiscoveryOrPartnerKeyExchange(state, true);
    } else {
      emberAfRegistrationPrintln("ERR: Failed to %p partner link key exchange"
                                 " with node 0x%2x endpoint 0x%x",
                                 "perform",
                                 nodeId,
                                 endpoint);
    }
    resumeAfterFixedDelay(success ? EMBER_SUCCESS : EMBER_ERR_FATAL);
  }
}
#endif //PARTNER_KE_REQUIRED

#ifdef EMBER_AF_PLUGIN_SMART_ENERGY_REGISTRATION_ESI_DISCOVERY_REQUIRED
static void performBinding(void)
{
  State *state = &states[emberGetCurrentNetwork()];
  EmberEUI64 eui64;
  uint8_t networkIndex = emberGetCurrentNetwork();

  emberAfGetEui64(eui64);

  // The spec requires that we bind in order to inform the ESI that we want
  // unsolicited updates, but the ESI is not required to use binding and may
  // send a negative response.  In addition, there is no recourse if a binding
  // is required but the ESI refuses.  So, send the bindings, but don't pay
  // attention to whether it works or not.
  for (;
       state->esiEntry != NULL;
       state->esiEntry = emberAfPluginEsiManagementGetNextEntry(state->esiEntry, 0)) {
    assert(state->esiEntry->nodeId != EMBER_NULL_NODE_ID);
    for (;
         state->endpointIndex < emberAfEndpointCount();
         state->endpointIndex++) {
      uint8_t endpoint = emberAfEndpointFromIndex(state->endpointIndex);
      if (networkIndex
          != emberAfNetworkIndexFromEndpointIndex(state->endpointIndex)) {
        continue;
      }
      for (;
           clusterList[state->clusterIndex] != UNDEFINED_CLUSTER_ID;
           state->clusterIndex++) {
        EmberAfClusterId clusterId = clusterList[state->clusterIndex];
        if (emberAfContainsClient(endpoint, clusterId)) {
          EmberStatus status;
          emberAfRegistrationPrintln("Performing binding"
                                     " to node 0x%2x endpoint 0x%x"
                                     " from endpoint 0x%x"
                                     " for cluster 0x%2x",
                                     state->esiEntry->nodeId,
                                     state->esiEntry->endpoint,
                                     endpoint,
                                     clusterId);
          status = emberBindRequest(state->esiEntry->nodeId,
                                    state->esiEntry->eui64,
                                    state->esiEntry->endpoint,
                                    clusterId,
                                    UNICAST_BINDING,
                                    eui64,
                                    0, // multicast group identifier - ignored
                                    endpoint,
                                    EMBER_AF_DEFAULT_APS_OPTIONS);
          if (status == EMBER_SUCCESS) {
            state->clusterIndex++;
          } else {
            emberAfRegistrationPrintln("ERR: Failed to send bind request"
                                       " to node 0x%2x endpoint 0x%x"
                                       " from endpoint 0x%x"
                                       " for cluster 0x%2x (0x%x)",
                                       state->esiEntry->nodeId,
                                       state->esiEntry->endpoint,
                                       endpoint,
                                       clusterId,
                                       status);
          }

          // We may hit the error limit if we delay here due to an error.  If
          // so, we have to clear our internal static indices; otherwise, when
          // registration is restarted, we won't pick up from the beginning.
          if (!resumeAfterFixedDelay(status)) {
            state->endpointIndex = state->clusterIndex = 0;
          }
          return;
        }
      }
      state->clusterIndex = 0;
    }
    state->endpointIndex = 0;
  }

  // Point to the first active entry with age 0 (if any).
  state->esiEntry = emberAfPluginEsiManagementGetNextEntry(NULL, 0);
  transition(STATE_DETERMINE_AUTHORITATIVE_TIME_SOURCE);
}
#endif //EMBER_AF_PLUGIN_SMART_ENERGY_REGISTRATION_ESI_DISCOVERY_REQUIRED

#ifdef EMBER_AF_PLUGIN_SMART_ENERGY_REGISTRATION_TIME_SOURCE_REQUIRED
static void determineAuthoritativeTimeSource(void)
{
  State *state = &states[emberGetCurrentNetwork()];
  static const uint8_t attributeIds[] = {
    LOW_BYTE(ZCL_TIME_ATTRIBUTE_ID), HIGH_BYTE(ZCL_TIME_ATTRIBUTE_ID),
    LOW_BYTE(ZCL_TIME_STATUS_ATTRIBUTE_ID), HIGH_BYTE(ZCL_TIME_STATUS_ATTRIBUTE_ID),
    LOW_BYTE(ZCL_LAST_SET_TIME_ATTRIBUTE_ID), HIGH_BYTE(ZCL_LAST_SET_TIME_ATTRIBUTE_ID),
    LOW_BYTE(ZCL_VALID_UNTIL_TIME_ATTRIBUTE_ID), HIGH_BYTE(ZCL_VALID_UNTIL_TIME_ATTRIBUTE_ID),
  };
  const uint8_t sourceEndpoint = emberAfPrimaryEndpointForCurrentNetworkIndex();
  assert(sourceEndpoint != 0xFF);

  if (!state->resuming) {
    emberAfRegistrationPrintln("Determining authoritative time source");
    state->source.valid = false;
  }

#ifdef EMBER_AF_PLUGIN_SMART_ENERGY_REGISTRATION_ALLOW_NON_ESI_TIME_SERVERS
  while (state->currentCandidate < state->totalCandidates) {
#else
  while (state->esiEntry != NULL) {
#endif
    EmberStatus status;
    assert(TIME_SOURCE_CANDIDATE(state).nodeId != EMBER_NULL_NODE_ID);
    emberAfRegistrationPrintln("Requesting time attributes"
                               " from node 0x%2x endpoint 0x%x",
                               TIME_SOURCE_CANDIDATE(state).nodeId,
                               TIME_SOURCE_CANDIDATE(state).endpoint);

    emberAfFillCommandGlobalClientToServerReadAttributes(ZCL_TIME_CLUSTER_ID,
                                                         attributeIds,
                                                         sizeof(attributeIds));
    emberAfSetCommandEndpoints(sourceEndpoint,
                               TIME_SOURCE_CANDIDATE(state).endpoint);
    status = emberAfSendCommandUnicast(EMBER_OUTGOING_DIRECT,
                                       TIME_SOURCE_CANDIDATE(state).nodeId);

    if (status == EMBER_SUCCESS) {
#ifdef EMBER_AF_PLUGIN_SMART_ENERGY_REGISTRATION_ALLOW_NON_ESI_TIME_SERVERS
      state->currentCandidate++;
      if (state->currentCandidate >= state->totalCandidates) {
#else
      state->esiEntry = emberAfPluginEsiManagementGetNextEntry(state->esiEntry, 0);
      if (state->esiEntry == NULL) {
#endif
        state->resuming = transitionAfterDelay(STATE_DETERMINE_AUTHORITATIVE_TIME_SOURCE,
                                               EMBER_AF_PLUGIN_SMART_ENERGY_REGISTRATION_DELAY_BRIEF,
                                               false);  // do not reset error count
        return;
      }
    } else {
      emberAfRegistrationPrintln("ERR: Failed to request time attributes"
                                 " from node 0x%2x endpoint 0x%x (0x%x)",
                                 TIME_SOURCE_CANDIDATE(state).nodeId,
                                 TIME_SOURCE_CANDIDATE(state).endpoint,
                                 status);
    }
    state->resuming = resumeAfterFixedDelay(status);
    return;
  }

  // Reset the position regardless of whether we have succeeded or failed.
  // If we failed, the reset position will be used in retries.
#ifdef EMBER_AF_PLUGIN_SMART_ENERGY_REGISTRATION_ALLOW_NON_ESI_TIME_SERVERS
  state->currentCandidate = 0;
#else
  state->esiEntry = emberAfPluginEsiManagementGetNextEntry(NULL, 0);
#endif

  if (!state->source.valid) {
    emberAfRegistrationPrintln("ERR: Failed to determine"
                               " authoritative time source");
    state->resuming = resumeAfterFixedDelay(EMBER_ERR_FATAL);
  } else {
    emberAfRegistrationPrintln("Determined authoritative time source,"
                               " node 0x%2x",
                               state->source.nodeId);
    transition(STATE_REGISTRATION_COMPLETE);
  }
}

void emAfPluginSmartEnergyRegistrationReadAttributesResponseCallback(uint8_t *buffer,
                                                                     uint16_t bufLen)
{
  State *state = &states[emberGetCurrentNetwork()];
  uint32_t time           = 0x00000000UL;
  uint8_t  timeStatus     = 0x00;
  uint32_t lastSetTime    = 0x00000000UL;
  uint32_t validUntilTime = 0xFFFFFFFFUL;
  uint16_t bufIndex = 0;
  EmberAfClusterCommand *cmd = emberAfCurrentCommand();

  if (state->state != STATE_DETERMINE_AUTHORITATIVE_TIME_SOURCE) {
    return;
  }

  // Each record in the response has a two-byte attribute id and a one-byte
  // status.  If the status is SUCCESS, there will also be a one-byte type and
  // variable-length data.
  while (bufIndex + 3 <= bufLen) {
    EmberAfAttributeId attributeId;
    EmberAfStatus status;
    attributeId = (EmberAfAttributeId)emberAfGetInt16u(buffer, bufIndex, bufLen);
    bufIndex += 2;
    status = (EmberAfStatus)emberAfGetInt8u(buffer, bufIndex, bufLen);
    bufIndex++;

    if (status == EMBER_ZCL_STATUS_SUCCESS) {
      EmberAfAttributeType type;
      type = (EmberAfAttributeType)emberAfGetInt8u(buffer, bufIndex, bufLen);
      bufIndex++;

      switch (attributeId) {
        case ZCL_TIME_ATTRIBUTE_ID:
          time = emberAfGetInt32u(buffer, bufIndex, bufLen);
          break;
        case ZCL_TIME_STATUS_ATTRIBUTE_ID:
          timeStatus = emberAfGetInt8u(buffer, bufIndex, bufLen);
          break;
        case ZCL_LAST_SET_TIME_ATTRIBUTE_ID:
          lastSetTime = emberAfGetInt32u(buffer, bufIndex, bufLen);
          break;
        case ZCL_VALID_UNTIL_TIME_ATTRIBUTE_ID:
          validUntilTime = emberAfGetInt32u(buffer, bufIndex, bufLen);
          break;
        default:
          break;
      }

      uint16_t dataSize = emberAfAttributeValueSize(type, buffer + bufIndex);
      if (dataSize > bufLen - bufIndex) {
        // dataSize exceeds buffer length, terminate loop
        break;
      }
      bufIndex += dataSize;
    }
  }

  emberAfRegistrationPrintln("Received time attributes from node 0x%2x",
                             cmd->source);
  emberAfRegistrationPrintln("time 0x%4x", time);
  emberAfRegistrationPrintln("time status 0x%x", timeStatus);

  // The process for determining the most authoritative time source is outlined
  // in section 3.12.2.2.2 of 07-5123-06 (ZigBee Cluster Library Specification).
  // Devices shall synchronize to a Time server with the highest rank according
  // to the following rules, listed in order of precedence:
  //   1. A server with the Superseding bit set shall be chosen over a server
  //      without the bit set.
  //   2. A server with the Master bit set shall be chosen over a server without
  //      the bit set.
  //   3. The server with the lower short address shall be chosen (note that
  //      this means a coordinator with the Superseding and Master bit set will
  //      always be chosen as the network time server).
  //   4. A Time server with neither the Master nor Synchronized bits set should
  //      not be chosen as the network time server.

  // This logic could be reduced if needed. However, this implementation
  // is way more readable.

  if (time == 0xFFFFFFFFUL) {
    if (state->source.valid
        && state->source.nodeId == cmd->source) {
      // We received an invalid time from our current time server. We set it to
      // invalid and wait for another server.
      state->source.valid = false;
      // TODO: Should we kick off the registration process again here?
    }
  } else {
    // The candidate must have at least one of Master or Synchronized bits set.
    if ((IS_TIME_STATUS(timeStatus, MASTER_CLOCK)
         || IS_TIME_STATUS(timeStatus, SYNCHRONIZED))
        // If we do not have a server yet, use this one.
        // If we do, use the precedence list above to decide whether the new
        // candidate is better than the one we currently have.
        && (!state->source.valid
            || (IS_TIME_STATUS(timeStatus, SUPERSEDING)
                && !IS_TIME_STATUS(state->source.timeStatus, SUPERSEDING))
            || (IS_TIME_STATUS(timeStatus, SUPERSEDING) == IS_TIME_STATUS(state->source.timeStatus, SUPERSEDING)
                && IS_TIME_STATUS(timeStatus, MASTER_CLOCK)
                && !IS_TIME_STATUS(state->source.timeStatus, MASTER_CLOCK))
            || (IS_TIME_STATUS(timeStatus, SUPERSEDING) == IS_TIME_STATUS(state->source.timeStatus, SUPERSEDING)
                && IS_TIME_STATUS(timeStatus, MASTER_CLOCK) == IS_TIME_STATUS(state->source.timeStatus, MASTER_CLOCK)
                && cmd->source < state->source.nodeId))) {
      state->source.valid          = true;
      state->source.nodeId         = cmd->source;
      state->source.time           = time;
      state->source.timeStatus     = timeStatus;
      state->source.lastSetTime    = lastSetTime;
      state->source.validUntilTime = validUntilTime;
      emberAfSetTime(time);

      emberAfRegistrationPrintln("Node 0x%2x chosen as"
                                 " authoritative time source",
                                 cmd->source);
    }
  }
}
#endif //EMBER_AF_PLUGIN_SMART_ENERGY_REGISTRATION_TIME_SOURCE_REQUIRED

static void stopRegistration(bool success)
{
  State *state = &states[emberGetCurrentNetwork()];

  emberAfRemoveFromCurrentAppTasks(EMBER_AF_WAITING_FOR_REGISTRATION);
  emberAfRegistrationCallback(success);

  emberAfTrustCenterKeepaliveUpdateCallback(success);

#ifdef EMBER_AF_PLUGIN_SMART_ENERGY_REGISTRATION_DELAY_PERIOD
  if (success) {
    transitionAfterDelay(STATE_DISCOVER_ENERGY_SERVICE_INTERFACES,
                         emAfPluginSmartEnergyRegistrationDiscoveryPeriod,
                         true);
    return;
  }
#endif //EMBER_AF_PLUGIN_SMART_ENERGY_REGISTRATION_DELAY_PERIOD
  state->state = STATE_INITIAL;
}

static void performDiscovery(void)
{
  State *state = &states[emberGetCurrentNetwork()];
  EmberStatus status;
  EmberNodeId target;
  EmberAfClusterId clusterId;

  // When performing key establishment, search the trust center for the Key
  // Establishment cluster.  When searching for ESIs, broadcast for the DRLC
  // server cluster, which only ESIs should have.
  if (state->state == STATE_DISCOVER_KEY_ESTABLISHMENT_CLUSTER) {
    emberAfRegistrationPrintln("Discovering %ps", "Key Establishment cluster");
    state->trustCenterKeyEstablishmentEndpoint = UNDEFINED_ENDPOINT;
    target = EMBER_TRUST_CENTER_NODE_ID;
    clusterId = ZCL_KEY_ESTABLISHMENT_CLUSTER_ID;
#ifdef EMBER_AF_PLUGIN_SMART_ENERGY_REGISTRATION_ESI_DISCOVERY_REQUIRED
  } else if (state->state == STATE_DISCOVER_ENERGY_SERVICE_INTERFACES) {
    emberAfRegistrationPrintln("Discovering %ps", "Energy Service Interface");
    // Aging the entries in the ESI table before starting the discovery process.
    emberAfPluginEsiManagementAgeAllEntries();
    target = EMBER_RX_ON_WHEN_IDLE_BROADCAST_ADDRESS;
    clusterId = ZCL_DEMAND_RESPONSE_LOAD_CONTROL_CLUSTER_ID;
#endif
#ifdef EMBER_AF_PLUGIN_SMART_ENERGY_REGISTRATION_ALLOW_NON_ESI_TIME_SERVERS
  } else if (state->state == STATE_DISCOVER_TIME_SERVERS) {
    emberAfRegistrationPrintln("Discovering %ps", "Time server");
    state->totalCandidates = 0;
    state->currentCandidate = 0;
    target = EMBER_RX_ON_WHEN_IDLE_BROADCAST_ADDRESS;
    clusterId = ZCL_TIME_CLUSTER_ID;
#endif
#ifdef PARTNER_KE_REQUIRED
  } else if (state->state == STATE_DISCOVER_IEEE_ADDRESSES) {
    uint8_t ep; // retrieveNextNodeFor... needs it, otherwise ignored
    retrieveNodeForDiscoveryOrPartnerKeyExchange(state, &target, &ep);
    assert(target != EMBER_NULL_NODE_ID);
    emberAfRegistrationPrintln("Discovering IEEE address"
                               " for node 0x%2x",
                               target);
    status = emberAfFindIeeeAddress(target, discoveryCallback);
    goto kickout;
#endif
  } else {
    emberAfRegistrationPrintln("ERR: Invalid state for discovery (0x%x)",
                               state->state);
    emberAfRegistrationAbortCallback();
    return;
  }

  emberAfRegistrationPrintln("Calling the FindDevices");
  // Transient failures may prevent us from performing discovery.  If so, we
  // will try again later.
  status = emberAfFindDevicesByProfileAndCluster(target,
                                                 SE_PROFILE_ID,
                                                 clusterId,
                                                 EMBER_AF_SERVER_CLUSTER_DISCOVERY,
                                                 discoveryCallback);
#ifdef PARTNER_KE_REQUIRED
  kickout:
#endif
  if (status != EMBER_SUCCESS) {
    emberAfRegistrationPrintln("ERR: Failed to start discovery (0x%x)", status);
    resumeAfterFixedDelay(status);
  }
}

static void discoveryCallback(const EmberAfServiceDiscoveryResult *result)
{
  uint8_t networkIndex = emberGetCurrentNetwork();
  State *state = &states[networkIndex];

  if (emberAfHaveDiscoveryResponseStatus(result->status)) {
    if (result->zdoRequestClusterId == MATCH_DESCRIPTORS_REQUEST) {
      const EmberAfEndpointList* endpointList = (const EmberAfEndpointList *)result->responseData;
      uint8_t i;

      // Need to ignore any discovery results from ourselves
      if (result->matchAddress == emberAfGetNodeId()) {
        emberAfRegistrationPrintln("Ignoring discovery result from loopback");
        return;
      }

      for (i = 0; i < endpointList->count; i++) {
        if (state->state == STATE_DISCOVER_KEY_ESTABLISHMENT_CLUSTER) {
          // Key Establishment is global to the device so we can ignore anything
          // beyond the first endpoint that responds.
          if (state->trustCenterKeyEstablishmentEndpoint
              == UNDEFINED_ENDPOINT) {
            emberAfRegistrationPrintln("%p %p on node 0x%2x endpoint 0x%x%p",
                                       "Discovered",
                                       "Key Establishment cluster",
                                       result->matchAddress,
                                       endpointList->list[i],
                                       "");
            state->trustCenterKeyEstablishmentEndpoint = endpointList->list[i];
          } else {
            emberAfRegistrationPrintln("%p %p on node 0x%2x endpoint 0x%x%p",
                                       "INFO: Ignored",
                                       "Key Establishment cluster",
                                       result->matchAddress,
                                       endpointList->list[i],
                                       "");
          }
#ifdef EMBER_AF_PLUGIN_SMART_ENERGY_REGISTRATION_ESI_DISCOVERY_REQUIRED
        } else if (state->state == STATE_DISCOVER_ENERGY_SERVICE_INTERFACES) {
          emberAfRegistrationPrintln("%p %p on node 0x%2x endpoint 0x%x%p",
                                     "Discovered",
                                     "Energy Server Interface",
                                     result->matchAddress,
                                     endpointList->list[i],
                                     "");

          state->esiEntry = emberAfPluginEsiManagementEsiLookUpByShortIdAndEndpoint(result->matchAddress,
                                                                                    endpointList->list[i]);
          if (state->esiEntry == NULL) {
            state->esiEntry = emberAfPluginEsiManagementGetFreeEntry();
          }

          if (state->esiEntry != NULL) {
            state->esiEntry->nodeId = result->matchAddress;
            state->esiEntry->networkIndex = networkIndex;
            state->esiEntry->endpoint = endpointList->list[i];
            state->esiEntry->age = 0;
          } else {
            emberAfRegistrationPrintln("%p %p on node 0x%2x endpoint 0x%x%p",
                                       "INFO: Ignored",
                                       "Energy Server Interface",
                                       result->matchAddress,
                                       endpointList->list[i],
                                       " because table is full");
          }
#endif //EMBER_AF_PLUGIN_SMART_ENERGY_REGISTRATION_ESI_DISCOVERY_REQUIRED
#ifdef EMBER_AF_PLUGIN_SMART_ENERGY_REGISTRATION_ALLOW_NON_ESI_TIME_SERVERS
        } else if (state->state == STATE_DISCOVER_TIME_SERVERS) {
          emberAfRegistrationPrintln("%p %p on node 0x%2x endpoint 0x%x%p",
                                     "Discovered",
                                     "Time server",
                                     result->matchAddress,
                                     endpointList->list[i],
                                     "");

          if (state->totalCandidates < MAX_TIME_SOURCE_CANDIDATES) {
            state->timeSourceCandidates[state->totalCandidates].nodeId = result->matchAddress;
            state->timeSourceCandidates[state->totalCandidates].endpoint = endpointList->list[i];
            state->totalCandidates++;
          } else {
            emberAfRegistrationPrintln("%p %p on node 0x%2x endpoint 0x%x%p",
                                       "INFO: Ignored",
                                       "Time server",
                                       result->matchAddress,
                                       endpointList->list[i],
                                       " because table is full");
          }
#endif //EMBER_AF_PLUGIN_SMART_ENERGY_REGISTRATION_ALLOW_NON_ESI_TIME_SERVERS
        }
      }
    }
  }

  if (state->state == STATE_DISCOVER_KEY_ESTABLISHMENT_CLUSTER) {
    if (state->trustCenterKeyEstablishmentEndpoint == UNDEFINED_ENDPOINT) {
      emberAfRegistrationPrintln("ERR: Failed to find %p",
                                 "Key Establishment cluster");
      resumeAfterFixedDelay(EMBER_ERR_FATAL);
    } else {
      EmberKeyStruct keyStruct;
      if (emberGetKey(EMBER_TRUST_CENTER_LINK_KEY, &keyStruct)
          != EMBER_SUCCESS) {
        emberAfRegistrationPrintln("ERR: Failed to get trust center link key");
        emberAfRegistrationAbortCallback();
        return;
      }

      // If we don't have full Smart Energy Security or if the key is already
      // authorized, we can skip key establishment and move on to ESI discovery.
      if (emberAfIsFullSmartEnergySecurityPresent() == EMBER_AF_INVALID_KEY_ESTABLISHMENT_SUITE) {
        emberAfRegistrationFlush();
        emberAfRegistrationPrintln("%pSkipping key establishment%p",
                                   "WARN: ",
                                   " due to missing libraries or certificate"
                                   " - see 'info' command for more detail");
        emberAfRegistrationFlush();
        transition(NEXT_STATE_AFTER_KE);
      } else if ((keyStruct.bitmask & EMBER_KEY_IS_AUTHORIZED) != 0U) {
        emberAfRegistrationPrintln("%pSkipping key establishment%p",
                                   "",
                                   " because key is already authorized");
        transition(NEXT_STATE_AFTER_KE);
      } else {
        transition(STATE_PERFORM_KEY_ESTABLISHMENT);
      }
    }
#ifdef EMBER_AF_PLUGIN_SMART_ENERGY_REGISTRATION_ESI_DISCOVERY_REQUIRED
  } else if (state->state == STATE_DISCOVER_ENERGY_SERVICE_INTERFACES) {
    if (result->status == EMBER_AF_BROADCAST_SERVICE_DISCOVERY_COMPLETE) {
      // Point to the first active entry with age 0 (if any).
      state->esiEntry = emberAfPluginEsiManagementGetNextEntry(NULL, 0);
      // There is no entry with age 0, i.e., we did not discover any ESI during
      // this discovery cycle.
      if (state->esiEntry == NULL) {
        // TODO: For now we just return an error. We might consider checking if
        // we have an ESI in the table "young enough" before returning an error.
        emberAfRegistrationPrintln("ERR: Failed to find %p",
                                   "Energy Service Interfaces");
        resumeAfterFixedDelay(EMBER_ERR_FATAL);
      } else {
  #ifdef EMBER_AF_PLUGIN_SMART_ENERGY_REGISTRATION_ALLOW_NON_ESI_TIME_SERVERS
        transition(STATE_DISCOVER_TIME_SERVERS);
  #else
        transition(STATE_DISCOVER_IEEE_ADDRESSES);
  #endif
      }
    }
#endif //EMBER_AF_PLUGIN_SMART_ENERGY_REGISTRATION_ESI_DISCOVERY_REQUIRED
#ifdef EMBER_AF_PLUGIN_SMART_ENERGY_REGISTRATION_ALLOW_NON_ESI_TIME_SERVERS
  } else if (state->state == STATE_DISCOVER_TIME_SERVERS) {
    if (result->status == EMBER_AF_BROADCAST_SERVICE_DISCOVERY_COMPLETE) {
      if (state->totalCandidates == 0) {
        emberAfRegistrationPrintln("ERR: Failed to find %p",
                                   "Time servers");
        resumeAfterFixedDelay(EMBER_ERR_FATAL);
      } else {
        state->currentCandidate = 0;
        transition(STATE_DISCOVER_IEEE_ADDRESSES);
      }
    }
#endif //EMBER_AF_PLUGIN_SMART_ENERGY_REGISTRATION_ALLOW_NON_ESI_TIME_SERVERS
#ifdef PARTNER_KE_REQUIRED
  } else if (state->state == STATE_DISCOVER_IEEE_ADDRESSES) {
    EmberNodeId nodeId;
    uint8_t ep; // retrieveNextNodeFor... needs it, otherwise ignored
    retrieveNodeForDiscoveryOrPartnerKeyExchange(state, &nodeId, &ep);
    assert(nodeId != EMBER_NULL_NODE_ID && nodeId == result->matchAddress);
    if (result->status == EMBER_AF_UNICAST_SERVICE_DISCOVERY_COMPLETE_WITH_RESPONSE) {
      emberAfRegistrationPrintln("%p IEEE address for node 0x%2x",
                                 "Discovered",
                                 nodeId);
  #ifdef EMBER_AF_PLUGIN_SMART_ENERGY_REGISTRATION_ESI_DISCOVERY_REQUIRED
      if (state->esiEntry != NULL) {
        // The discovered IEEE address may have been of an ESI or a Time server.
        // If the former, update the relevant ESI entry.
        // Otherwise, the entry is going top be NULL.
        MEMMOVE(state->esiEntry->eui64, result->responseData, EUI64_SIZE);
      }
  #endif
      if (emberAfAddAddressTableEntry((uint8_t*)result->responseData, nodeId)
          == EMBER_NULL_ADDRESS_TABLE_INDEX) {
        emberAfRegistrationPrintln("WARN: Could not add address table entry"
                                   " for node 0x%2x",
                                   nodeId);
      }
      if (!findNextNodeForDiscoveryOrPartnerKeyExchange(state, true)) {
  #ifdef EMBER_AF_PLUGIN_SMART_ENERGY_REGISTRATION_ESI_DISCOVERY_REQUIRED
        // Point to the first active entry with age 0 (if any).
        state->esiEntry = emberAfPluginEsiManagementGetNextEntry(NULL, 0);
  #endif
  #ifdef EMBER_AF_PLUGIN_SMART_ENERGY_REGISTRATION_ALLOW_NON_ESI_TIME_SERVERS
        state->currentCandidate = 0;
  #endif
        transition(STATE_PERFORM_PARTNER_LINK_KEY_EXCHANGE);
      } else {
        resumeAfterFixedDelay(EMBER_SUCCESS);
      }
    } else {
      emberAfRegistrationPrintln("ERR: Failed to discover IEEE address"
                                 " for node 0x%2x",
                                 nodeId);
      resumeAfterFixedDelay(EMBER_ERR_FATAL);
    }
#endif //PARTNER_KE_REQUIRED
  }
}
