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
 * @brief Routines for the ZLL Commissioning Common plugin, which defines
 *        functions common to both server and client sides of the ZLL protocol.
 *******************************************************************************
   ******************************************************************************/

#include "app/framework/include/af.h"
#include "app/framework/util/af-main.h"
#include "app/framework/util/common.h"
#include "zll-commissioning-common.h"

// AppBuilder already prevents multi-network ZLL configurations.  This is here
// as a reminder that the code below assumes that there is exactly one network
// and that it is ZigBee PRO.
#if EMBER_SUPPORTED_NETWORKS != 1
  #error ZLL is not supported with multiple networks.
#endif

#ifdef EMBER_AF_PLUGIN_NETWORK_CREATOR
  #include EMBER_AF_API_NETWORK_CREATOR
#endif

//------------------------------------------------------------------------------
// Globals

#define isFactoryNew(state) ((state) & EMBER_ZLL_STATE_FACTORY_NEW)

// The target network - used by both client and server sides, the latter mainly for
// the touchlink complete callback to the application.
EmberZllNetwork emAfZllNetwork;

#ifdef EMBER_AF_PLUGIN_ZLL_COMMISSIONING_CLIENT
// Sub-device info (mainly for client, but server needs to initialize the count)
EmberZllDeviceInfoRecord emAfZllSubDevices[EMBER_AF_PLUGIN_ZLL_COMMISSIONING_CLIENT_SUB_DEVICE_TABLE_SIZE];
uint8_t emAfZllSubDeviceCount = 0;
#endif

// The module state for both client and server.
uint16_t emAfZllFlags = INITIAL;

// TL Rejoin stage allows a fixed number of retries (in case Beacon responses are missing).
#define INITIATOR_REJOIN_MAX_RETRIES   4
uint8_t emAfInitiatorRejoinRetryCount = 0;

#ifdef PLUGIN_DEBUG
static const uint8_t emAfZllCommissioningPluginName[] = "ZLL Commissioning Common";
#define PLUGIN_NAME emAfZllCommissioningPluginName
#endif

#ifdef EMBER_AF_PLUGIN_TEST_HARNESS_Z3
extern EmberEventControl emZigbeeLeaveEvent;
#endif

// Private ZLL commissioning functions
void emAfZllFinishNetworkFormationForRouter(EmberStatus status);
void emAfZllAbortTouchLink(EmberAfZllCommissioningStatus reason);
void emAfZllStackStatus(EmberStatus status);
void emAfZllInitializeRadio(void);
bool emAfZllStealingAllowed(void);
bool emAfZllRemoteResetAllowed(void);

// Forward references
bool emAfZllAmFactoryNew(void);
#ifdef EZSP_HOST
void emberAfPluginZllCommissioningCommonNcpInitCallback(bool memoryAllocation);
#else
void emberAfPluginZllCommissioningCommonInitCallback(void);
#endif

//------------------------------------------------------------------------------
// Module private functions

static void setProfileInteropState(void)
{
  EmberTokTypeStackZllData token;

  emberZllGetTokenStackZllData(&token);
  token.bitmask |= EMBER_ZLL_STATE_PROFILE_INTEROP;
  emberZllSetTokenStackZllData(&token);
}

static void initFactoryNew(void)
{
  // The initialization is only performed if we are factory new in the BDB sense,
  // i.e. not joined to a centralized or distributed network.
  if (emAfZllAmFactoryNew()) {
    emberAfAppPrintln("ZllCommInit - device is not joined to a network");

    // Set the default ZLL node type for both client and server, for Scan Request
    // and Scan Response messages respectively.
    emberSetZllNodeType((emAfCurrentZigbeeProNetwork->nodeType
                         == EMBER_COORDINATOR)
                        ? EMBER_ROUTER
                        : emAfCurrentZigbeeProNetwork->nodeType);

#ifdef EMBER_AF_PLUGIN_ZLL_COMMISSIONING_SERVER
    emAfZllInitializeRadio();
#endif

#ifdef EMBER_AF_PLUGIN_ZLL_COMMISSIONING_CLIENT
    // Set the address assignment capability bit to true for a client in all cases.
    emberSetZllAdditionalState(EMBER_ZLL_STATE_ADDRESS_ASSIGNMENT_CAPABLE);
#endif

#ifdef EMBER_AF_PLUGIN_ZLL_COMMISSIONING_COMMON_ZIGBEE3_SUPPORT
    // Set interop bit here if we support Zigbee 3.0.
    setProfileInteropState();
#endif

    // Set the security state here, in case of incoming scan requests, to ensure
    // that our security key bitmask matches that of the initiator, or in case
    // we initiate a touchlink after joining by classical commissioning.
    emberAfZllSetInitialSecurityState();

    // Set the default power for a new network in all cases.
    emberSetRadioPower(EMBER_AF_PLUGIN_ZLL_COMMISSIONING_COMMON_RADIO_TX_POWER);
  }
}

static void completeResetToFactoryNew(void)
{
  emberAfAppPrintln("Resetting to factory new");
  emberAfResetAttributes(EMBER_BROADCAST_ENDPOINT);
  emberAfGroupsClusterClearGroupTableCallback(EMBER_BROADCAST_ENDPOINT);
  emberAfScenesClusterClearSceneTableCallback(EMBER_BROADCAST_ENDPOINT);
  emberClearBindingTable();
#ifdef EZSP_HOST
  emberAfPluginZllCommissioningCommonNcpInitCallback(false);
#else
  emberAfPluginZllCommissioningCommonInitCallback();
#endif
  emberAfPluginZllCommissioningCommonResetToFactoryNewCallback();
  emAfZllFlags = INITIAL;
}

//------------------------------------------------------------------------------
// ZLL commissioning private functions

bool emAfZllAmFactoryNew(void)
{
  EmberTokTypeStackZllData token;
  emberZllGetTokenStackZllData(&token);
  return isFactoryNew(token.bitmask);
}

void emAfZllTouchLinkComplete(void)
{
  EmberNodeType nodeType;
  EmberNetworkParameters parameters;
  emberAfGetNetworkParameters(&nodeType, &parameters);
  emAfZllNetwork.zigbeeNetwork.channel = parameters.radioChannel;
  emAfZllNetwork.zigbeeNetwork.panId = parameters.panId;
  MEMMOVE(emAfZllNetwork.zigbeeNetwork.extendedPanId,
          parameters.extendedPanId,
          EXTENDED_PAN_ID_SIZE);
  emAfZllNetwork.zigbeeNetwork.nwkUpdateId = parameters.nwkUpdateId;
#ifdef EMBER_AF_PLUGIN_ZLL_COMMISSIONING_CLIENT
  emberAfPluginZllCommissioningCommonTouchLinkCompleteCallback(&emAfZllNetwork,
                                                               emAfZllSubDeviceCount,
                                                               (emAfZllSubDeviceCount == 0
                                                                ? NULL
                                                                : emAfZllSubDevices));
#else
  emberAfPluginZllCommissioningCommonTouchLinkCompleteCallback(&emAfZllNetwork, 0, NULL);
#endif //EMBER_AF_PLUGIN_ZLL_COMMISSIONING_CLIENT

  // Update module state after the callback call.
  emAfZllFlags = INITIAL;
}

EmberNodeType emAfZllGetLogicalNodeType(void)
{
  EmberNodeType nodeType;
  EmberStatus status = emberAfGetNodeType(&nodeType);

  // Note, we only report as a coordinator if we are a currently
  // coordinator on a centralized network.
  if (status == EMBER_NOT_JOINED) {
    nodeType = emAfCurrentZigbeeProNetwork->nodeType;
    if (nodeType == EMBER_COORDINATOR) {
      nodeType = EMBER_ROUTER;
    }
  }
  return nodeType;
}

// The following function is not required during standard touchlink commissioning,
// but is retained to support the following CLI command (which may be used
// either client or server side), in particular for ZTT/ZUTH:
//   plugin zll-commissioning form
// Note that it WILL now result in a 'ZLL' distributed network being created, i.e.,
// a network appropriate to a address-assignment-capable touchlink initiator,
// with address and group ranges assigned to it.
EmberStatus emAfZllFormNetwork(uint8_t channel, int8_t power, EmberPanId panId)
{
  // Create a distributed commissioning network using the ZLL link key.
  EmberZllNetwork network;
  MEMSET(&network, 0, sizeof(EmberZllNetwork));
  network.zigbeeNetwork.channel = channel;
  network.zigbeeNetwork.panId = panId;
  emberAfGetFormAndJoinExtendedPanIdCallback(network.zigbeeNetwork.extendedPanId);
  EmberTokTypeStackZllData token;
  emberZllGetTokenStackZllData(&token);
  network.state = token.bitmask & 0xffff;
  network.nodeType = emAfZllGetLogicalNodeType();
  emberAfZllSetInitialSecurityState();
  EmberStatus status = emberZllFormNetwork(&network, power);
  debugPrintln("%p: emberZllFormNetwork - status = %X, node type = %d", PLUGIN_NAME, status, network.nodeType);
  if (status == EMBER_SUCCESS) {
    emAfZllFlags |= FORMING_NETWORK;
  }
  return status;
}

EmberZllPolicy emAfZllGetPolicy(void)
{
  EmberZllPolicy policy;
#ifdef EZSP_HOST
  if (ezspGetPolicy(EZSP_ZLL_POLICY, &policy) != EZSP_SUCCESS) {
    policy = EMBER_ZLL_POLICY_DISABLED;
  }
#else
  policy = emberZllGetPolicy();
#endif
  return policy;
}

//------------------------------------------------------------------------------
// Public functions

void emberAfPluginZllCommissioningCommonInitCallback(void)
{
#ifndef EZSP_HOST
  // Set the policy for both server and client.
  EmberZllPolicy policy = EMBER_ZLL_POLICY_ENABLED;
#ifdef EMBER_AF_PLUGIN_ZLL_COMMISSIONING_SERVER
  policy |= EMBER_ZLL_POLICY_TARGET;
  if (emAfZllStealingAllowed()) {
    policy |= EMBER_ZLL_POLICY_STEALING_ENABLED;
  }
  if (emAfZllRemoteResetAllowed()) {
    policy |= EMBER_ZLL_POLICY_REMOTE_RESET_ENABLED;
  }
#endif
  emberZllSetPolicy(policy);

  // Set the primary and secondary channel masks for both server and client.
  emberSetZllPrimaryChannelMask(EMBER_AF_PLUGIN_ZLL_COMMISSIONING_COMMON_PRIMARY_CHANNEL_MASK);
#ifdef EMBER_AF_PLUGIN_ZLL_COMMISSIONING_COMMON_SECONDARY_CHANNEL_MASK
  emberSetZllSecondaryChannelMask(EMBER_AF_PLUGIN_ZLL_COMMISSIONING_COMMON_SECONDARY_CHANNEL_MASK);
#endif

  // Factory new initialization
  initFactoryNew();
#endif // #ifndef EZSP_HOST
}

void emberAfPluginZllCommissioningCommonNcpInitCallback(bool memoryAllocation)
{
#ifdef EZSP_HOST
  if (!memoryAllocation) {
    // Set the policy for both server and client.
    EmberZllPolicy policy = EMBER_ZLL_POLICY_ENABLED;
#ifdef EMBER_AF_PLUGIN_ZLL_COMMISSIONING_SERVER
    policy |= EMBER_ZLL_POLICY_TARGET;
    if (emAfZllStealingAllowed()) {
      policy |= EMBER_ZLL_POLICY_STEALING_ENABLED;
    }
    if (emAfZllRemoteResetAllowed()) {
      policy |= EMBER_ZLL_POLICY_REMOTE_RESET_ENABLED;
    }
#endif
    emberAfSetEzspPolicy(EZSP_ZLL_POLICY,
                         policy,
                         "ZLL policy",
                         "enable");

    // Set the primary and secondary channel masks for both server and client.
    emberSetZllPrimaryChannelMask(EMBER_AF_PLUGIN_ZLL_COMMISSIONING_COMMON_PRIMARY_CHANNEL_MASK);
#ifdef EMBER_AF_PLUGIN_ZLL_COMMISSIONING_COMMON_SECONDARY_CHANNEL_MASK
    emberSetZllSecondaryChannelMask(EMBER_AF_PLUGIN_ZLL_COMMISSIONING_COMMON_SECONDARY_CHANNEL_MASK);
#endif

    // Factory new initialization
    initFactoryNew();
  }
#endif // #ifdef EZSP_HOST
}

EmberStatus emberAfZllSetInitialSecurityState(void)
{
  EmberKeyData networkKey;
  EmberZllInitialSecurityState securityState = {
    0, // bitmask - unused
    EMBER_ZLL_KEY_INDEX_CERTIFICATION,
    EMBER_ZLL_CERTIFICATION_ENCRYPTION_KEY,
    EMBER_ZLL_CERTIFICATION_PRECONFIGURED_LINK_KEY,
  };
  EmberStatus status;

  // We can only initialize security information while not on a network - this
  // also covers the case where we are joined as a coordinator.
  if (emberAfNetworkState() != EMBER_NO_NETWORK) {
    return EMBER_SUCCESS;
  }

  status = emberAfGenerateRandomKey(&networkKey);
  if (status != EMBER_SUCCESS) {
    emberAfAppPrintln("%p%p failed 0x%x",
                      "Error: ",
                      "Generating random key",
                      status);
    return status;
  }

  emberAfPluginZllCommissioningCommonInitialSecurityStateCallback(&securityState);
  status = emberZllSetInitialSecurityState(&networkKey, &securityState);

  if (status != EMBER_SUCCESS) {
    emberAfAppPrintln("%p%p failed 0x%x",
                      "Error: ",
                      "Initializing security",
                      status);
  }
  return status;
}

void emberAfZllResetToFactoryNew(void)
{
  // The leave will cause the ZLL state to be set to 'factory new',
  // but after a short delay.
  emAfZllFlags |= RESETTING_TO_FACTORY_NEW;

  debugPrintln("emberAfZllResetToFactoryNew - flags = %X, networkState = %X", emAfZllFlags, emberNetworkState());

  // Note that we won't get a network down stack status if we
  // are currently trying to join - the leave will complete silently.
  bool silentLeave = (emberNetworkState() == EMBER_JOINING_NETWORK) ? true : false;
  EmberStatus status = emberLeaveNetwork();
  if (status != EMBER_SUCCESS) {
    emberAfAppPrintln("Error: Failed to leave network, status: 0x%X", status);
  }

  // Complete the reset immediately if we're not expecting a network down status.
  if (silentLeave || status != EMBER_SUCCESS) {
    emberZllClearTokens();
    completeResetToFactoryNew();
  }
#ifdef EMBER_AF_PLUGIN_TEST_HARNESS_Z3
  else if (status == EMBER_SUCCESS && !touchLinkTarget()) {
    // Complete the leave immediately without the usual delay for a local reset
    // (this is to accommodate ZTT scripts which issue a 'network leave'
    // which is immediately followed by a 'reset').
    emberEventControlSetActive(emZigbeeLeaveEvent);
  }
#endif
}

void emberAfPluginZllCommissioningCommonStackStatusCallback(EmberStatus status)
{
  // If we are forming a network for a router initiator, then we handle
  // this status separately.
  // During touch linking, EMBER_NETWORK_UP means the process is complete.  Any
  // other status, unless we're busy joining or rejoining, means that the touch
  // link failed.
  debugPrintln("%p: ZllCommStackStatus: status = %X, flags = %X", PLUGIN_NAME, status, emAfZllFlags);

#if defined(EMBER_AF_PLUGIN_ZLL_COMMISSIONING_CLIENT) && defined(EMBER_AF_PLUGIN_NETWORK_CREATOR)
  if (formingNetwork()) {
    emAfZllFinishNetworkFormationForRouter(status);
  } else
#endif

  if (resettingToFactoryNew()) { // either local or remote (TOUCH_LINK_TARGET set)
    // A reset overrides a touchlink in progress.
    if (status == EMBER_NETWORK_DOWN) {
      completeResetToFactoryNew();
    }
  } else if (touchLinkInProgress()) { // including TOUCH_LINK_TARGET
    if (status == EMBER_NETWORK_UP) {
      emAfZllTouchLinkComplete();
    } else if (status == EMBER_NETWORK_DOWN) {
      // We don't do anything here for a network down.
    } else {
      emberAfAppPrintln("%p%p%p: status = %X, flags = %X",
                        "Error: ",
                        "Touch linking failed: ",
                        "joining failed",
                        status, emAfZllFlags);
#ifdef EMBER_AF_PLUGIN_ZLL_COMMISSIONING_CLIENT
      if (!touchLinkTarget()) {
        if ((emberAfNetworkState() == EMBER_JOINED_NETWORK_NO_PARENT)
            && (emAfInitiatorRejoinRetryCount < INITIATOR_REJOIN_MAX_RETRIES)) {
          // The TL initiator has joined the target's Pan but the final Rejoin
          // has failed (e.g. if the target fails to send a Beacon Response
          // during the rejoin).  We allow a number of Rejoin retries here.
          ++emAfInitiatorRejoinRetryCount;
          emberRejoinNetwork(true);
        } else {
          emAfZllAbortTouchLink(EMBER_AF_ZLL_JOINING_FAILED);
        }
      }
#endif
    }
  } else {
    // Here we catch all fresh non-ZLL network joins, and set the ZLL state accordingly.
    EmberTokTypeStackZllData token;
    emberZllGetTokenStackZllData(&token);

    if (status == EMBER_NETWORK_UP && (token.bitmask & EMBER_ZLL_STATE_FACTORY_NEW)) {
      // When either a router or an end device joins a non-ZLL network, it is
      // no longer factory new.  On a non-ZLL network, ZLL devices that are
      // normally address assignment capable do not have free network or group
      // addresses nor do they have a range of group addresses for themselves.
      // (Note, to ensure that ZLL devices will always operate as ZigBee 3.0 applications,
      // we need to set the ZLL profile interop bit even when the application
      // joins a classical ZigBee network. This way, if the device is stolen from
      // a classical ZigBee network to a ZLL network, it will operate as a
      // ZigBee 3.0 device. This is now set at plugin initialization time.)
      token.bitmask &= ~EMBER_ZLL_STATE_FACTORY_NEW;
      token.freeNodeIdMin = token.freeNodeIdMax = EMBER_ZLL_NULL_NODE_ID;
      token.myGroupIdMin = EMBER_ZLL_NULL_GROUP_ID;
      token.freeGroupIdMin = token.freeGroupIdMax = EMBER_ZLL_NULL_GROUP_ID;
      emberZllSetTokenStackZllData(&token);
      emberZllSetNonZllNetwork();
    }
    // Otherwise, we just ignore the status, for example, a network up
    // from a rejoin or a join at startup.
  }
}

bool emberAfZllTouchLinkInProgress(void)
{
  // Returns true if a touch link is in progress or false otherwise.

  return touchLinkInProgress();
}

// Note, this will cause *all* touchlink interpan messages to be dropped,
// not just touchlink requests, so it will effectively disable touchlink
// initiator, as well as touchlink target.
EmberStatus emberAfZllDisable(void)
{
  EmberZllPolicy policy = emAfZllGetPolicy();
  EmberStatus status;
#ifndef EZSP_HOST
  status = emberZllSetPolicy(policy & ~EMBER_ZLL_POLICY_ENABLED);
#else
  status = emberAfSetEzspPolicy(EZSP_ZLL_POLICY,
                                policy & ~EMBER_ZLL_POLICY_ENABLED,
                                "ZLL processing",
                                "disable");
#endif
  return status;
}

EmberStatus emberAfZllEnable(void)
{
  EmberZllPolicy policy = emAfZllGetPolicy();
  EmberStatus status;

  // Re-enable stealing and remote reset, if we have server-side, and the plugin option permits it.
  policy |= EMBER_ZLL_POLICY_ENABLED;
#ifdef EMBER_AF_PLUGIN_ZLL_COMMISSIONING_SERVER
  if (emAfZllStealingAllowed()) {
    policy |= EMBER_ZLL_POLICY_STEALING_ENABLED;
  }
  if (emAfZllRemoteResetAllowed()) {
    policy |= EMBER_ZLL_POLICY_REMOTE_RESET_ENABLED;
  }
#endif
#ifndef EZSP_HOST
  status = emberZllSetPolicy(policy);
#else
  status = emberAfSetEzspPolicy(EZSP_ZLL_POLICY,
                                policy,
                                "ZLL processing",
                                "enable");
#endif
  return status;
}
