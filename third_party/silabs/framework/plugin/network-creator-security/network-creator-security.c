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
 * @brief Routines for the Network Creator Security plugin, which configures
 *        network permit join state and key policy for a Zigbee 3.0 network.
 *******************************************************************************
   ******************************************************************************/

#include "app/framework/include/af.h"
#include "app/framework/util/af-main.h"

#include "network-creator-security.h"

#include "app/framework/security/af-security.h" // emAfAllowTrustCenterRejoins
#include "app/util/zigbee-framework/zigbee-device-common.h" // emberLeaveRequest

#ifdef EZSP_HOST
// NCP
  #define allowTrustCenterLinkKeyRequests() \
  ezspSetPolicy(EZSP_TC_KEY_REQUEST_POLICY, EZSP_ALLOW_TC_KEY_REQUESTS_AND_SEND_CURRENT_KEY)
  #define allowTrustCenterLinkKeyRequestsAndGenerateNewKeys() \
  ezspSetPolicy(EZSP_TC_KEY_REQUEST_POLICY, EZSP_ALLOW_TC_KEY_REQUEST_AND_GENERATE_NEW_KEY)
  #define setTransientKeyTimeout(timeS) \
  ezspSetValue(EZSP_VALUE_TRANSIENT_KEY_TIMEOUT_S, 2, (uint8_t *)(&timeS));
  #define setTcRejoinsUsingWellKnownKeyAllowed(allow) \
  (void)ezspSetPolicy(EZSP_TC_REJOINS_USING_WELL_KNOWN_KEY_POLICY, (allow))
  #define setTcRejoinsUsingWellKnownKeyTimeout(timeout) \
  (void)ezspSetConfigurationValue(EZSP_CONFIG_TC_REJOINS_USING_WELL_KNOWN_KEY_TIMEOUT_S, (timeout))
#else
// SoC
extern uint16_t emAllowTcRejoinsUsingWellKnownKeyTimeoutSec;
  #define allowTrustCenterLinkKeyRequests() \
  emberTrustCenterLinkKeyRequestPolicy = EMBER_ALLOW_TC_LINK_KEY_REQUEST_AND_SEND_CURRENT_KEY
  #define allowTrustCenterLinkKeyRequestsAndGenerateNewKeys() \
  emberTrustCenterLinkKeyRequestPolicy = EMBER_ALLOW_TC_LINK_KEY_REQUEST_AND_GENERATE_NEW_KEY
  #define setTransientKeyTimeout(timeS) \
  emberTransientKeyTimeoutS = (timeS)
  #define setTcRejoinsUsingWellKnownKeyAllowed(allow) \
  emberSetTcRejoinsUsingWellKnownKeyAllowed((allow))
  #define setTcRejoinsUsingWellKnownKeyTimeout(timeout) \
  emAllowTcRejoinsUsingWellKnownKeyTimeoutSec = (timeout)
#endif

#define ZIGBEE_3_CENTRALIZED_SECURITY_LINK_KEY         \
  {                                                    \
    { 0x5A, 0x69, 0x67, 0x42, 0x65, 0x65, 0x41, 0x6C,  \
      0x6C, 0x69, 0x61, 0x6E, 0x63, 0x65, 0x30, 0x39 } \
  }

#define ZIGBEE_3_DISTRIBUTED_SECURITY_LINK_KEY         \
  {                                                    \
    { 0xD0, 0xD1, 0xD2, 0xD3, 0xD4, 0xD5, 0xD6, 0xD7,  \
      0xD8, 0xD9, 0xDA, 0xDB, 0xDC, 0xDD, 0xDE, 0xDF } \
  }

#ifndef EMBER_AF_PLUGIN_NETWORK_CREATOR_SECURITY_NETWORK_OPEN_TIME_S
  #define EMBER_AF_PLUGIN_NETWORK_CREATOR_SECURITY_NETWORK_OPEN_TIME_S (300)
#endif

#define NETWORK_OPEN_TIME_S (EMBER_AF_PLUGIN_NETWORK_CREATOR_SECURITY_NETWORK_OPEN_TIME_S)

#if defined(EMBER_AF_PLUGIN_NETWORK_CREATOR_SECURITY_ALLOW_HA_DEVICES_TO_STAY)
  #define ALLOW_HA_DEVICES true
#else
  #define ALLOW_HA_DEVICES false
#endif

// -----------------------------------------------------------------------------
// Globals

bool allowHaDevices = ALLOW_HA_DEVICES;
EmberKeyData distributedKey = ZIGBEE_3_DISTRIBUTED_SECURITY_LINK_KEY;

// -----------------------------------------------------------------------------
// Internal Declarations

EmberEventControl emberAfPluginNetworkCreatorSecurityOpenNetworkEventControl;
#define openNetworkEventControl (emberAfPluginNetworkCreatorSecurityOpenNetworkEventControl)

static uint16_t openNetworkTimeRemainingS;

// -----------------------------------------------------------------------------
// Framework Callbacks

void emberAfPluginNetworkCreatorSecurityInitCallback(void)
{
#if defined(EZSP_HOST) && defined(EMBER_AF_PLUGIN_NETWORK_CREATOR_SECURITY_BDB_JOIN_USES_INSTALL_CODE_KEY)
  EzspStatus status = emberAfSetEzspPolicy(EZSP_TRUST_CENTER_POLICY,
                                           (EZSP_DECISION_ALLOW_JOINS | EZSP_DECISION_JOINS_USE_INSTALL_CODE_KEY),
                                           "Trust Center Policy",
                                           "Joins using install code only");

  if (EZSP_SUCCESS != status) {
    emberAfCorePrintln("%p: %p: 0x%X",
                       EMBER_AF_PLUGIN_NETWORK_CREATOR_SECURITY_PLUGIN_NAME,
                       "failed to configure joining using install code only",
                       status);
  }
#endif // EZSP_HOST && EMBER_AF_PLUGIN_NETWORK_CREATOR_SECURITY_BDB_JOIN_USES_INSTALL_CODE_KEY
}

void emberAfPluginNetworkCreatorSecurityStackStatusCallback(EmberStatus status)
{
#ifdef EMBER_AF_PLUGIN_NETWORK_CREATOR_SECURITY_TRUST_CENTER_SUPPORT
  if (status == EMBER_NETWORK_UP
      && emberAfGetNodeId() == EMBER_TRUST_CENTER_NODE_ID) {
    EmberExtendedSecurityBitmask extended;

    // If we form a centralized network, make sure our trust center policy is
    // correctly set to respond to a key request.
#ifdef ZCL_USING_SL_WWAH_CLUSTER_CLIENT
    allowTrustCenterLinkKeyRequestsAndGenerateNewKeys();
#else
    allowTrustCenterLinkKeyRequests();
#endif

    // This bit is not saved to a token, so make sure that our security bitmask
    // has this bit set on reboot.
    emberGetExtendedSecurityBitmask(&extended);
    extended |= EMBER_NWK_LEAVE_REQUEST_NOT_ALLOWED;
    emberSetExtendedSecurityBitmask(extended);
  }
#endif /* EMBER_AF_PLUGIN_NETWORK_CREATOR_SECURITY_TRUST_CENTER_SUPPORT */
}

// -----------------------------------------------------------------------------
// Stack Callbacks

#if defined(EMBER_AF_PLUGIN_NETWORK_CREATOR_SECURITY_TRUST_CENTER_SUPPORT) \
  || defined(EMBER_AF_PLUGIN_TEST_HARNESS_Z3)
static bool isWildcardEui64(EmberEUI64 eui64)
{
  for (uint8_t i = 0; i < EUI64_SIZE; i++) {
    if (eui64[i] != 0xFF) {
      return false;
    }
  }
  return true;
}
#endif /* defined(EMBER_AF_PLUGIN_NETWORK_CREATOR_SECURITY_TRUST_CENTER_SUPPORT) \
       || defined(EMBER_AF_PLUGIN_TEST_HARNESS_Z3) */

#if defined(EMBER_AF_PLUGIN_TEST_HARNESS_Z3)
extern uint8_t emAfPluginTestHarnessZ3ServerMaskHigh;
#endif /* defined(EMBER_AF_PLUGIN_TEST_HARNESS_Z3) */

void emberAfPluginNetworkCreatorSecurityZigbeeKeyEstablishmentCallback(EmberEUI64 eui64,
                                                                       EmberKeyStatus keyStatus)
{
#if defined(EMBER_AF_PLUGIN_NETWORK_CREATOR_SECURITY_TRUST_CENTER_SUPPORT) \
  || defined(EMBER_AF_PLUGIN_TEST_HARNESS_Z3)
  // If we are notified that a joining node failed to verify their
  // TCLK properly, then we are going to kick them off the network,
  // as they pose a potential security hazard.

#if defined(EMBER_AF_PLUGIN_TEST_HARNESS_Z3)
  // Do nothing if we are pretending to be r20 or lower.
  if (emAfPluginTestHarnessZ3ServerMaskHigh == 0) {
    return;
  }
#endif /* defined(EMBER_AF_PLUGIN_TEST_HARNESS_Z3) */

  if (!isWildcardEui64(eui64)
      && ((keyStatus == EMBER_TC_REQUESTER_VERIFY_KEY_TIMEOUT && !allowHaDevices)
          || keyStatus == EMBER_TC_REQUESTER_VERIFY_KEY_FAILURE)) {
    EmberStatus status = EMBER_NOT_FOUND;
    EmberNodeId destinationId = emberLookupNodeIdByEui64(eui64);
    if (destinationId != EMBER_NULL_NODE_ID) {
      // We do not tell the node to rejoin, since they may be malicious.
      status = emberLeaveRequest(destinationId,
                                 eui64,
                                 0, // no leave flags
                                 (EMBER_APS_OPTION_RETRY
                                  | EMBER_APS_OPTION_ENABLE_ROUTE_DISCOVERY));
    }
    emberAfCorePrintln("%p: Remove node 0x%2X for failed key verification: 0x%X",
                       EMBER_AF_PLUGIN_NETWORK_CREATOR_SECURITY_PLUGIN_NAME,
                       destinationId,
                       status);
  }
#endif /* defined(EMBER_AF_PLUGIN_NETWORK_CREATOR_SECURITY_TRUST_CENTER_SUPPORT) \
       || defined(EMBER_AF_PLUGIN_TEST_HARNESS_Z3) */
}

// -----------------------------------------------------------------------------
// API

EmberStatus emberAfPluginNetworkCreatorSecurityStart(bool centralizedNetwork)
{
  EmberInitialSecurityState state;
  EmberExtendedSecurityBitmask extended;
  EmberStatus status = EMBER_SUCCESS;

  MEMSET(&state, 0, sizeof(state));
  state.bitmask = (EMBER_TRUST_CENTER_GLOBAL_LINK_KEY
                   | EMBER_HAVE_PRECONFIGURED_KEY
                   | EMBER_HAVE_NETWORK_KEY
                   | EMBER_NO_FRAME_COUNTER_RESET
                   | EMBER_REQUIRE_ENCRYPTED_KEY);

  extended = EMBER_JOINER_GLOBAL_LINK_KEY;

  if (!centralizedNetwork) {
    MEMMOVE(&(state.preconfiguredKey),
            emberKeyContents(&distributedKey),
            EMBER_ENCRYPTION_KEY_SIZE);

    // Use distributed trust center mode.
    state.bitmask |= EMBER_DISTRIBUTED_TRUST_CENTER_MODE;
  }
#if defined(EMBER_AF_PLUGIN_NETWORK_CREATOR_SECURITY_TRUST_CENTER_SUPPORT) \
  || defined(EMBER_AF_PLUGIN_TEST_HARNESS_Z3)
  else { // centralizedNetwork
    // Generate a random global link key.
    // This is the key the trust center will send to a joining node when it
    // updates its link key.
    status = emberAfGenerateRandomKey(&(state.preconfiguredKey));
    if (status != EMBER_SUCCESS) {
      goto kickout;
    }

    // Use hashed link keys for improved storage and speed.
    state.bitmask |= EMBER_TRUST_CENTER_USES_HASHED_LINK_KEY;

    // Tell the trust center to ignore leave requests.
    extended |= EMBER_NWK_LEAVE_REQUEST_NOT_ALLOWED;
  }
#endif /* defined(EMBER_AF_PLUGIN_NETWORK_CREATOR_SECURITY_TRUST_CENTER_SUPPORT) \
       || defined(EMBER_AF_PLUGIN_TEST_HARNESS_Z3) */

  // Generate a random network key.
  status = emberAfGenerateRandomKey(&(state.networkKey));
  if (status != EMBER_SUCCESS) {
    goto kickout;
  }

  // Set the initial security data.
  status = emberSetInitialSecurityState(&state);
  if (status != EMBER_SUCCESS) {
    goto kickout;
  }
  status = emberSetExtendedSecurityBitmask(extended);

  kickout:
  emberAfCorePrintln("%p: %p: 0x%X",
                     EMBER_AF_PLUGIN_NETWORK_CREATOR_SECURITY_PLUGIN_NAME,
                     "Start",
                     status);
  return status;
}

EmberStatus emberAfPluginNetworkCreatorSecurityOpenNetwork(void)
{
  EmberStatus status = EMBER_SUCCESS;

  if (emberAfNetworkState() != EMBER_JOINED_NETWORK) {
    return EMBER_ERR_FATAL;
  }
#if defined(EMBER_AF_PLUGIN_NETWORK_CREATOR_SECURITY_ALLOW_TC_REJOIN_WITH_WELL_KNOWN_KEY)
  #if defined(EMBER_TEST)
    #define EMBER_AF_PLUGIN_NETWORK_CREATOR_SECURITY_ALLOW_TC_REJOINS_USING_WELL_KNOWN_KEY_TIMEOUT_SEC 0
  #endif
  setTcRejoinsUsingWellKnownKeyTimeout(EMBER_AF_PLUGIN_NETWORK_CREATOR_SECURITY_ALLOW_TC_REJOINS_USING_WELL_KNOWN_KEY_TIMEOUT_SEC);
  setTcRejoinsUsingWellKnownKeyAllowed(true);
#endif

#if defined(EMBER_AF_PLUGIN_NETWORK_CREATOR_SECURITY_TRUST_CENTER_SUPPORT) \
  || defined(EMBER_AF_PLUGIN_TEST_HARNESS_Z3)
  if (emberAfGetNodeId() == EMBER_TRUST_CENTER_NODE_ID) {
    EmberEUI64 wildcardEui64 = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, };
    EmberKeyData centralizedKey = ZIGBEE_3_CENTRALIZED_SECURITY_LINK_KEY;
    status = emberAddTransientLinkKey(wildcardEui64, &centralizedKey);
  }
#endif /* defined(EMBER_AF_PLUGIN_NETWORK_CREATOR_SECURITY_TRUST_CENTER_SUPPORT) \
       || defined(EMBER_AF_PLUGIN_TEST_HARNESS_Z3) */

  if (status == EMBER_SUCCESS) {
    openNetworkTimeRemainingS = NETWORK_OPEN_TIME_S;
    setTransientKeyTimeout(openNetworkTimeRemainingS);
    emberEventControlSetActive(openNetworkEventControl);
  }

  return status;
}

EmberStatus emberAfPluginNetworkCreatorSecurityCloseNetwork(void)
{
  EmberStatus status = EMBER_ERR_FATAL;

  if (emberAfNetworkState() == EMBER_JOINED_NETWORK) {
    emberClearTransientLinkKeys();
    emberEventControlSetInactive(openNetworkEventControl);
    zaTrustCenterSetJoinPolicy(EMBER_NO_ACTION);
    status = emberAfPermitJoin(0, true); // broadcast
  }

  return status;
}

EmberStatus emberAfPluginNetworkCreatorSecurityOpenNetworkWithKeyPair(EmberEUI64 eui64,
                                                                      EmberKeyData keyData)
{
  EmberStatus status = EMBER_SUCCESS;

  if (emberAfNetworkState() != EMBER_JOINED_NETWORK) {
    return EMBER_ERR_FATAL;
  }

#if defined(EMBER_AF_PLUGIN_NETWORK_CREATOR_SECURITY_TRUST_CENTER_SUPPORT) \
  || defined(EMBER_AF_PLUGIN_TEST_HARNESS_Z3)
  if (emberAfGetNodeId() == EMBER_TRUST_CENTER_NODE_ID) {
    status = emberAddTransientLinkKey(eui64, &keyData);
  }
#endif /* defined(EMBER_AF_PLUGIN_NETWORK_CREATOR_SECURITY_TRUST_CENTER_SUPPORT) \
       || defined(EMBER_AF_PLUGIN_TEST_HARNESS_Z3) */

  if (status == EMBER_SUCCESS) {
    openNetworkTimeRemainingS = NETWORK_OPEN_TIME_S;
    setTransientKeyTimeout(openNetworkTimeRemainingS);
    emberEventControlSetActive(openNetworkEventControl);
  }

  return status;
}

// -----------------------------------------------------------------------------
// Internal Definitions

void emberAfPluginNetworkCreatorSecurityOpenNetworkEventHandler(void)
{
  EmberStatus status = EMBER_SUCCESS;
  uint8_t permitJoinTime;

  emberEventControlSetInactive(openNetworkEventControl);

  // If we have left the network, then we don't need to proceed further.
  if (emberAfNetworkState() != EMBER_JOINED_NETWORK) {
    return;
  }

  if (openNetworkTimeRemainingS > EMBER_AF_PERMIT_JOIN_MAX_TIMEOUT) {
    permitJoinTime = EMBER_AF_PERMIT_JOIN_MAX_TIMEOUT;
    openNetworkTimeRemainingS -= EMBER_AF_PERMIT_JOIN_MAX_TIMEOUT;
    emberEventControlSetDelayQS(openNetworkEventControl,
                                EMBER_AF_PERMIT_JOIN_MAX_TIMEOUT << 2);
  } else {
    permitJoinTime = openNetworkTimeRemainingS;
    openNetworkTimeRemainingS = 0;
  }

  if (status == EMBER_SUCCESS) {
    zaTrustCenterSetJoinPolicy(EMBER_USE_PRECONFIGURED_KEY);
    status = emberAfPermitJoin(permitJoinTime, true); // broadcast permit join
  }

  emberAfCorePrintln("%p: %p: 0x%X",
                     EMBER_AF_PLUGIN_NETWORK_CREATOR_SECURITY_PLUGIN_NAME,
                     "Open network",
                     status);
}
