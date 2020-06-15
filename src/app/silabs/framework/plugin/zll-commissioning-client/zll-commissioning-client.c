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
 * @brief Routines for the ZLL Commissioning Client plugin, which implements the
 *        client side of the ZLL protocol.
 *******************************************************************************
   ******************************************************************************/

#include "app/framework/include/af.h"
#include "app/framework/util/af-main.h"
#include "app/framework/util/common.h"
#include "app/framework/plugin/zll-commissioning-common/zll-commissioning.h"

// AppBuilder already prevents multi-network ZLL configurations.  This is here
// as a reminder that the code below assumes that there is exactly one network
// and that it is ZigBee PRO.
#if EMBER_SUPPORTED_NETWORKS != 1
  #error ZLL is not supported with multiple networks.
#endif

// ZigBee 3.0 test harness hooks.
#ifdef EMBER_AF_PLUGIN_TEST_HARNESS_Z3
  #include "app/framework/plugin/test-harness-z3/test-harness-z3-zll.h"
#endif

#ifdef EMBER_AF_PLUGIN_NETWORK_CREATOR
  #include EMBER_AF_API_NETWORK_CREATOR
#endif

//------------------------------------------------------------------------------
// Globals

#define ZLL_DEVICE_INFORMATION_RECORD_SIZE 16

#define isRequestingPriority(state) ((state) & EMBER_ZLL_STATE_LINK_PRIORITY_REQUEST)

static bool radioOnForRequestOnly = false;

EmberEventControl emberAfPluginZllCommissioningClientTouchLinkEventControl;
static uint8_t currentChannel;
static int8_t rssi;

#ifdef EMBER_AF_PLUGIN_ZLL_COMMISSIONING_CLIENT_CENTRALIZED_NETWORK_SUPPORT
  #define CENTRALIZED_NETWORK_SUPPORT (true)
#else
  #define CENTRALIZED_NETWORK_SUPPORT (false)
#endif
static const bool centralizedNetworkSupport = CENTRALIZED_NETWORK_SUPPORT;
uint16_t emAfZllIdentifyDurationSec = EMBER_AF_PLUGIN_ZLL_COMMISSIONING_CLIENT_IDENTIFY_DURATION;

#ifdef PLUGIN_DEBUG
static const uint8_t emAfZllCommissioningPluginName[] = "ZLL Commissioning Client";
#define PLUGIN_NAME emAfZllCommissioningPluginName
#endif

static bool scanningPrimaryChannels = false;

#ifdef EMBER_TEST
// Limit this delay for scripted tests, to prevent the transaction timing out,
// especially for a host side test.
  #define EMBER_AF_PLUGIN_ZLL_COMMISSIONING_TOUCH_LINK_MILLISECONDS_DELAY 2500
#else
  #define EMBER_AF_PLUGIN_ZLL_COMMISSIONING_TOUCH_LINK_MILLISECONDS_DELAY \
  (EMBER_AF_PLUGIN_ZLL_COMMISSIONING_CLIENT_TOUCH_LINK_SECONDS_DELAY * MILLISECOND_TICKS_PER_SECOND)
#endif

#define JOIN_RESPONSE_TIMEOUT_QS 20

// Parameters for Network Creator callbacks - note, for a client, we use the
// radio power defined for this plugin, not the power defined for the Network
// Creator plugin, in order that both client and server use the same power.
static EmberPanId networkCreatorPanId = 0xFFFF;
static int8_t networkCreatorRadioPower = EMBER_AF_PLUGIN_ZLL_COMMISSIONING_COMMON_RADIO_TX_POWER;

// Note, currently we set a random extended PAN ID for both client and server created
// networks during standard touchlink commissioning (even if the client wants otherwise.)

// Private ZLL commissioning functions
bool emAfZllAmFactoryNew(void);
EmberNodeType emAfZllGetLogicalNodeType(void);
void emAfZllTouchLinkComplete(void);

// Forward references
void emAfZllAbortTouchLink(EmberAfZllCommissioningStatus reason);

//------------------------------------------------------------------------------
// Module private functions

static uint32_t getChannelMask(void)
{
  uint32_t channelMask = 0;
  if (scanningPrimaryChannels) {
    channelMask = emberGetZllPrimaryChannelMask();
#ifdef EMBER_AF_PLUGIN_ZLL_COMMISSIONING_COMMON_SCAN_SECONDARY_CHANNELS
  } else {
    channelMask = emberGetZllSecondaryChannelMask();
#endif
  }
  debugPrintln("getChannelMask: mask = %4X, scanningPrimary = %d", channelMask, scanningPrimaryChannels);
  return channelMask;
}

static EmberStatus sendDeviceInformationRequest(uint8_t startIndex)
{
  EmberStatus status;
  emberAfFillExternalBuffer((ZCL_CLUSTER_SPECIFIC_COMMAND
                             | ZCL_FRAME_CONTROL_CLIENT_TO_SERVER
                             | ZCL_DISABLE_DEFAULT_RESPONSE_MASK),
                            ZCL_ZLL_COMMISSIONING_CLUSTER_ID,
                            ZCL_DEVICE_INFORMATION_REQUEST_COMMAND_ID,
                            "wu",
                            emAfZllNetwork.securityAlgorithm.transactionId,
                            startIndex);
  status = emberAfSendCommandInterPan(0xFFFF,                // destination pan id
                                      emAfZllNetwork.eui64,
                                      EMBER_NULL_NODE_ID,    // node id - ignored
                                      0x0000,                // group id - ignored
                                      EMBER_ZLL_PROFILE_ID);
  if (status != EMBER_SUCCESS) {
    emberAfAppPrintln("%p%p failed 0x%x",
                      "Error: ",
                      "Device information request",
                      status);
  }
  return status;
}

static EmberStatus sendIdentifyRequest(uint16_t identifyDuration)
{
  EmberStatus status;
  emberAfFillExternalBuffer((ZCL_CLUSTER_SPECIFIC_COMMAND
                             | ZCL_FRAME_CONTROL_CLIENT_TO_SERVER
                             | ZCL_DISABLE_DEFAULT_RESPONSE_MASK),
                            ZCL_ZLL_COMMISSIONING_CLUSTER_ID,
                            ZCL_IDENTIFY_REQUEST_COMMAND_ID,
                            "wv",
                            emAfZllNetwork.securityAlgorithm.transactionId,
                            identifyDuration);
  status = emberAfSendCommandInterPan(0xFFFF,                // destination pan id
                                      emAfZllNetwork.eui64,
                                      EMBER_NULL_NODE_ID,    // node id - ignored
                                      0x0000,                // group id - ignored
                                      EMBER_ZLL_PROFILE_ID);
  if (status != EMBER_SUCCESS) {
    emberAfAppPrintln("%p%p failed 0x%x",
                      "Error: ",
                      "Identify request",
                      status);
  }
  return status;
}

static EmberStatus sendResetToFactoryNewRequest(void)
{
  EmberStatus status;
  emberAfFillExternalBuffer((ZCL_CLUSTER_SPECIFIC_COMMAND
                             | ZCL_FRAME_CONTROL_CLIENT_TO_SERVER
                             | ZCL_DISABLE_DEFAULT_RESPONSE_MASK),
                            ZCL_ZLL_COMMISSIONING_CLUSTER_ID,
                            ZCL_RESET_TO_FACTORY_NEW_REQUEST_COMMAND_ID,
                            "w",
                            emAfZllNetwork.securityAlgorithm.transactionId);
  status = emberAfSendCommandInterPan(0xFFFF,                // destination pan id
                                      emAfZllNetwork.eui64,
                                      EMBER_NULL_NODE_ID,    // node id - ignored
                                      0x0000,                // group id - ignored
                                      EMBER_ZLL_PROFILE_ID);
  if (status != EMBER_SUCCESS) {
    emberAfAppPrintln("%p%p failed 0x%x",
                      "Error: ",
                      "Reset to factory new request",
                      status);
  }
  return status;
}

static void deviceInformationResponseHandler(const EmberEUI64 source,
                                             uint32_t transaction,
                                             uint8_t numberOfSubDevices,
                                             uint8_t startIndex,
                                             uint8_t deviceInformationRecordCount,
                                             uint8_t *deviceInformationRecordList)
{
  uint16_t deviceInformationRecordListLen = (deviceInformationRecordCount
                                             * ZLL_DEVICE_INFORMATION_RECORD_SIZE);
  uint16_t deviceInformationRecordListIndex = 0;
  uint8_t i;
  bool validResponse = (emberEventControlGetActive(emberAfPluginZllCommissioningClientTouchLinkEventControl)
                        && (emAfZllNetwork.securityAlgorithm.transactionId == transaction)
                        && MEMCOMPARE(emAfZllNetwork.eui64, source, EUI64_SIZE) == 0);

  emberAfZllCommissioningClusterFlush();
  emberAfZllCommissioningClusterPrint("RX: DeviceInformationResponse 0x%4x, 0x%x, 0x%x, 0x%x,",
                                      transaction,
                                      numberOfSubDevices,
                                      startIndex,
                                      deviceInformationRecordCount);
  emberAfZllCommissioningClusterFlush();
  for (i = 0; i < deviceInformationRecordCount; i++) {
    uint8_t *ieeeAddress;
    uint8_t endpointId;
    uint16_t profileId;
    uint16_t deviceId;
    uint8_t version;
    uint8_t groupIdCount;
    ieeeAddress = &deviceInformationRecordList[deviceInformationRecordListIndex];
    deviceInformationRecordListIndex += EUI64_SIZE;
    endpointId = emberAfGetInt8u(deviceInformationRecordList, deviceInformationRecordListIndex, deviceInformationRecordListLen);
    deviceInformationRecordListIndex++;
    profileId = emberAfGetInt16u(deviceInformationRecordList, deviceInformationRecordListIndex, deviceInformationRecordListLen);
    deviceInformationRecordListIndex += 2;
    deviceId = emberAfGetInt16u(deviceInformationRecordList, deviceInformationRecordListIndex, deviceInformationRecordListLen);
    deviceInformationRecordListIndex += 2;
    version = emberAfGetInt8u(deviceInformationRecordList, deviceInformationRecordListIndex, deviceInformationRecordListLen);
    deviceInformationRecordListIndex++;
    groupIdCount = emberAfGetInt8u(deviceInformationRecordList, deviceInformationRecordListIndex, deviceInformationRecordListLen);
    deviceInformationRecordListIndex++;
    deviceInformationRecordListIndex++;
    emberAfZllCommissioningClusterPrint(" [");
    emberAfZllCommissioningClusterDebugExec(emberAfPrintBigEndianEui64(ieeeAddress));
    emberAfZllCommissioningClusterPrint(" 0x%x 0x%2x 0x%2x 0x%x 0x%x 0x%x",
                                        endpointId,
                                        profileId,
                                        deviceId,
                                        version,
                                        groupIdCount,
                                        emberAfGetInt8u(deviceInformationRecordList, deviceInformationRecordListIndex, deviceInformationRecordListLen));
    emberAfZllCommissioningClusterFlush();

    if (validResponse
        && (emAfZllSubDeviceCount
            < EMBER_AF_PLUGIN_ZLL_COMMISSIONING_CLIENT_SUB_DEVICE_TABLE_SIZE)) {
      MEMMOVE(emAfZllSubDevices[emAfZllSubDeviceCount].ieeeAddress, ieeeAddress, EUI64_SIZE);
      emAfZllSubDevices[emAfZllSubDeviceCount].endpointId = endpointId;
      emAfZllSubDevices[emAfZllSubDeviceCount].profileId = profileId;
      emAfZllSubDevices[emAfZllSubDeviceCount].deviceId = deviceId;
      emAfZllSubDevices[emAfZllSubDeviceCount].version = version;
      emAfZllSubDevices[emAfZllSubDeviceCount].groupIdCount = groupIdCount;
      emAfZllSubDeviceCount++;
    } else {
      emberAfZllCommissioningClusterPrint(" (ignored)");
    }
    emberAfZllCommissioningClusterPrint("]");
    emberAfZllCommissioningClusterFlush();
  }
  emberAfZllCommissioningClusterPrintln("");

  if (validResponse
      && (emAfZllSubDeviceCount
          < EMBER_AF_PLUGIN_ZLL_COMMISSIONING_CLIENT_SUB_DEVICE_TABLE_SIZE)
      && emAfZllSubDeviceCount < numberOfSubDevices) {
    sendDeviceInformationRequest(startIndex + deviceInformationRecordCount);
  } else if (radioOnForRequestOnly) {
    emberZllSetRadioIdleMode(EMBER_RADIO_POWER_MODE_OFF);
    radioOnForRequestOnly = false;
  }
}

static bool amInitiator(void)
{
  EmberTokTypeStackZllData token;
  emberZllGetTokenStackZllData(&token);
  return token.bitmask & EMBER_ZLL_STATE_ADDRESS_ASSIGNMENT_CAPABLE;
}

static EmberStatus startScan(uint8_t purpose)
{
  EmberStatus status = EMBER_INVALID_CALL;
  if (touchLinkInProgress()) {
    emberAfAppPrintln("%pTouch linking in progress", "Error: ");
  } else if (!amInitiator()) {
    emberAfAppPrintln("%pDevice is not an initiator", "Error: ");
  } else {
    emberAfZllSetInitialSecurityState();

    // Use the Ember API, not the framework API, otherwise host-side returns zero if network down
    currentChannel = emberGetRadioChannel();
    debugPrintln("startScan: purpose = %X, flags = %2X, current channel = %d", purpose, emAfZllFlags, currentChannel);

    scanningPrimaryChannels = true;
    uint32_t channelMask = getChannelMask();
    if (channelMask == 0) {
      // If there are no primary channels, then get the secondary mask.
      scanningPrimaryChannels = false;
      channelMask = getChannelMask();
    }

    if (channelMask != 0) {
      status = emberZllStartScan(channelMask,
                                 EMBER_AF_PLUGIN_ZLL_COMMISSIONING_CLIENT_SCAN_POWER_LEVEL,
                                 emAfZllGetLogicalNodeType());
      if (status == EMBER_SUCCESS) {
        emAfZllFlags = purpose;
      } else {
        emberAfAppPrintln("%p%p%p0x%x",
                          "Error: ",
                          "Touch linking failed: ",
                          "could not start scan: ",
                          status);
      }
    } else {
      emberAfAppPrintln("%p%p0x%x",
                        "Error: ",
                        "Touch linking failed: ",
                        "no scan channels configured");
    }
  }
  return status;
}

static bool isSameNetwork(const EmberZllNetwork *network)
{
  EmberNodeType nodeType;
  EmberNetworkParameters parameters;
  EmberStatus status = emberAfGetNetworkParameters(&nodeType, &parameters);

  // Note, we only check the Extended Pan Id, not the Short Pan Id, nor the
  // Network Update Id.
  return (status == EMBER_SUCCESS
          && (MEMCOMPARE(parameters.extendedPanId,
                         network->zigbeeNetwork.extendedPanId,
                         EXTENDED_PAN_ID_SIZE) == 0));
}

static bool isTouchlinkPermitted(const EmberZllNetwork *networkInfo)
{
  // Basic checks for a Z3.0 touchlink initiator.
  if (networkInfo->nodeType == EMBER_UNKNOWN_DEVICE) {
    // The target must be a known device type.
    debugPrintln("isTouchlinkPermitted: Error - Target is unknown node type %d", networkInfo->nodeType);
    return false;
  }

  // Get current run-time network status. (Note that emberGetNodeType() may
  // report an actual node type even if a previous touchlink failed, e.g.
  // if the initiator was orphaned and was unable to rejoin)
  EmberNetworkStatus networkStatus = emberNetworkState();
  EmberCurrentSecurityState securityState;
  bool networkUp;

  if (networkStatus == EMBER_JOINED_NETWORK
      || networkStatus == EMBER_JOINED_NETWORK_NO_PARENT) {
    networkUp = true;
    if (emberGetCurrentSecurityState(&securityState) != EMBER_SUCCESS) {
      // It is possible that we're orphaned but attempting to rejoin here, but we'll
      // fail the touchlink anyway, since it would probably collide with the rejoining
      // procedure. (emberGetCurrentSecurityState() fails if we are rejoining even
      // though emberNetworkState() reports EMBER_JOINED_NETWORK_NO_PARENT)
      debugPrintln("emberGetCurrentSecurityState failure");
      return false;
    } else {
      // We're either joined or orphaned and not attempting to rejoin; in
      // either case we can proceed with the next stage of validation.
      debugPrintln("isTouchlinkPermitted: security bitmask = %2X,flags = %2X",
                   securityState.bitmask, emAfZllFlags);
    }
  } else {
    networkUp = false;
    debugPrintln("isTouchlinkPermitted: initiator is not joined");
  }

  if (scanForReset() || scanForTouchLink()) {
    // When scanning for touch linking or reset, the initiator need not be
    // on a network, but if it is, then it may not be on a different centralized
    // network, unless the (non-compliant) centralized network support is enabled.
    if (networkUp
        && (securityState.bitmask & EMBER_DISTRIBUTED_TRUST_CENTER_MODE) == 0
        && !isSameNetwork(networkInfo)
        && !centralizedNetworkSupport) {
      debugPrintln("isTouchlinkPermitted: Error - Initiator on centralized network");
      return false;
    }
  }

  if (scanForTouchLink() && !networkUp) {
    // When scanning for touch linking, and if the initiator is not on a network,
    // either initiator or target must be able to form a network. If centralized
    // network touchlink initiating is supported, the initiating device must
    // be coordinator-capable.
    if ((emAfZllGetLogicalNodeType() != EMBER_ROUTER && networkInfo->nodeType != EMBER_ROUTER)
        || (centralizedNetworkSupport
            && emAfCurrentZigbeeProNetwork->nodeType != EMBER_COORDINATOR)) {
      debugPrintln("isTouchlinkPermitted: Error - Neither end can form a network");
      debugPrintln("our type = %d, their type = %d", emAfZllGetLogicalNodeType(), networkInfo->nodeType);
      return false;
    }
  }
  return true;
}

// Returns an integer greater than, equal to, or less than zero, according to
// whether target t1 is better than, equal to, or worse than target t2 in terms
// of requested priority with corrected signal strength serving as tiebreaker.
static int8_t targetCompare(const EmberZllNetwork *t1,
                            int8_t r1,
                            const EmberZllNetwork *t2,
                            int8_t r2)
{
  // When considering two targets, if only one has requested priority, that one
  // is chosen.
  if (isRequestingPriority(t1->state)
      && !isRequestingPriority(t2->state)) {
    return 1;
  } else if (!isRequestingPriority(t1->state)
             && isRequestingPriority(t2->state)) {
    return -1;
  }

  // If the priority of both targets is the same, the correct signal strength
  // is considered.  The strongest corrected signal wins.
  emberAfDebugPrintln("Signal strengths: r1 = %d, t1->corr = %d, r2 = %d, t2->corr = %d",
                      r1, t1->rssiCorrection, r2, t2->rssiCorrection);
  if (r1 + t1->rssiCorrection < r2 + t2->rssiCorrection) {
    return -1;
  } else if (r2 + t2->rssiCorrection < r1 + t1->rssiCorrection) {
    return 1;
  }

  // If we got here, both targets are considered equal.
  return 0;
}

static void processScanComplete(EmberStatus scanStatus)
{
  EmberStatus status;
  debugPrintln("processScanComplete - status = %X", scanStatus);

  if (scanStatus != EMBER_SUCCESS) {
    emberAfAppPrintln("%p%p0x%x",
                      "Error: ",
                      "Touch linking failed due to preemption: ",
                      scanStatus);
    emAfZllAbortTouchLink(EMBER_AF_ZLL_PREEMPTED_BY_STACK);
  } else if (abortingTouchLink()) {
    debugPrintln("processScanComplete: aborted by application");
    emAfZllAbortTouchLink(EMBER_AF_ZLL_ABORTED_BY_APPLICATION);
  }

  if (touchLinkInProgress()) {
#ifdef EMBER_AF_PLUGIN_ZLL_COMMISSIONING_COMMON_SCAN_SECONDARY_CHANNELS
    if (scanningPrimaryChannels) {
      scanningPrimaryChannels = false;
      uint32_t channelMask = getChannelMask();
      if (channelMask != 0) {
        status = emberZllStartScan(channelMask,
                                   EMBER_AF_PLUGIN_ZLL_COMMISSIONING_CLIENT_SCAN_POWER_LEVEL,
                                   emAfZllGetLogicalNodeType());
        if (status == EMBER_SUCCESS) {
          // We will continue on the second scan complete callback.
          return;
        }
        emberAfAppPrintln("%p%p0x%x",
                          "Error: ",
                          "could not start secondary channel scan: ",
                          status);
      }
      debugPrintln("processScanComplete: no secondary channels, dropping thru");
    }
#endif
    emAfZllFlags |= SCAN_COMPLETE;

    // Inform the API that scanning is complete for this transaction.
    emberZllScanningComplete();

    if (targetNetworkFound()) {
      status = emberSetLogicalAndRadioChannel(emAfZllNetwork.zigbeeNetwork.channel);
      if (status != EMBER_SUCCESS) {
        emberAfAppPrintln("%p%p%p0x%x",
                          "Error: ",
                          "Touch linking failed: ",
                          "could not change channel: ",
                          status);
        emAfZllAbortTouchLink(EMBER_AF_ZLL_CHANNEL_CHANGE_FAILED);
        return;
      }

      // When scanning for the purposes of touch linking or requesting device
      // information and the target has more than one sub device, turn the
      // receiver on (so we can actually receive the response) and send out the
      // first request.  If the target only has one sub device, its data will
      // have already been received in the ScanRequest.
      if ((scanForTouchLink() || scanForDeviceInformation())
          && emAfZllNetwork.numberSubDevices != 1) {
        debugPrintln("processScanComplete: sending device info request");

        // Turn receiver on if it is currently off so we can receive
        // the Device Info Response.
        if (emberZllGetRadioIdleMode() == EMBER_RADIO_POWER_MODE_OFF) {
          if (emAfZllAmFactoryNew()) {
            // If we're factory new, start a short rx-on period to cover the request.
            emberZllSetRxOnWhenIdle(JOIN_RESPONSE_TIMEOUT_QS * 250);
          } else {
            // We're joined as a sleepy end-device - rx-on may be active and
            // suspended, or expired, or never started in the first place.
            emberZllSetRadioIdleMode(EMBER_RADIO_POWER_MODE_RX_ON);
            radioOnForRequestOnly = true;
          }
        }

        status = sendDeviceInformationRequest(0x00); // start index
        if (status != EMBER_SUCCESS) {
          emberAfAppPrintln("%p%p%p0x%x",
                            "Error: ",
                            "Touch linking failed: ",
                            "could not send device information request: ",
                            status);
          emAfZllAbortTouchLink(EMBER_AF_ZLL_SENDING_DEVICE_INFORMATION_REQUEST_FAILED);
          return;
        }
      }

      status = sendIdentifyRequest(emAfZllIdentifyDurationSec);
      if (status != EMBER_SUCCESS) {
        emberAfAppPrintln("%p%p%p0x%x",
                          "Error: ",
                          "Touch linking failed: ",
                          "could not send identify request: ",
                          status);
        emAfZllAbortTouchLink(EMBER_AF_ZLL_SENDING_IDENTIFY_REQUEST_FAILED);
        return;
      }
      emberEventControlSetDelayMS(emberAfPluginZllCommissioningClientTouchLinkEventControl,
                                  EMBER_AF_PLUGIN_ZLL_COMMISSIONING_TOUCH_LINK_MILLISECONDS_DELAY);
    } else {
      emberAfAppPrintln("%p%p%p",
                        "Error: ",
                        "Touch linking failed: ",
                        "no networks were found");
      emAfZllAbortTouchLink(EMBER_AF_ZLL_NO_NETWORKS_FOUND);
    }
  } else {
    debugPrintln("processScanComplete: no touchlink in progress");
  }
}

//------------------------------------------------------------------------------
// ZLL commissioning private functions

void emAfZllAbortTouchLink(EmberAfZllCommissioningStatus reason)
{
  emAfZllFlags = INITIAL;
  debugPrintln("emAfZllAbortTouchLink: reason = %d", reason);
  if (emberEventControlGetActive(emberAfPluginZllCommissioningClientTouchLinkEventControl)) {
    emberEventControlSetInactive(emberAfPluginZllCommissioningClientTouchLinkEventControl);
    if (radioOnForRequestOnly) {
      emberZllSetRadioIdleMode(EMBER_RADIO_POWER_MODE_OFF);
      radioOnForRequestOnly = false;
    }
    sendIdentifyRequest(0x0000); // exit identify mode
  }
  EmberStatus status;
  if (emberNetworkState() == EMBER_JOINED_NETWORK
      || emberNetworkState() == EMBER_JOINED_NETWORK_NO_PARENT) {
    status = emberSetRadioChannel(currentChannel);
  } else {
    status = emberSetLogicalAndRadioChannel(currentChannel);
  }
  if (status != EMBER_SUCCESS) {
    emberAfAppPrintln("%p0x%x%p%d",
                      "Error: ",
                      status,
                      " - could not restore channel to: ",
                      currentChannel);
  }
  if (reason == EMBER_AF_ZLL_NETWORK_UPDATE_OPERATION) {
    emAfZllTouchLinkComplete();
  } else {
    emberAfPluginZllCommissioningTouchLinkFailedCallback(reason);
  }

  // Inform the API that scanning is complete for this transaction.
  emberZllScanningComplete();
}

#ifdef EMBER_AF_PLUGIN_NETWORK_CREATOR
void emAfZllFinishNetworkFormationForRouter(EmberStatus status)
{
  if (status == EMBER_NETWORK_UP) {
    // Make ourselves not factory new, and reset the ZLL address and group space
    // we are using node id 0x0001 ourselves.
    EmberTokTypeStackZllData token;
    emberZllGetTokenStackZllData(&token);
    token.bitmask &= ~EMBER_ZLL_STATE_FACTORY_NEW;
    token.freeNodeIdMin = EMBER_ZLL_MIN_NODE_ID + 1;  // start with 0x0002
    token.freeNodeIdMax = EMBER_ZLL_MAX_NODE_ID;
    token.myGroupIdMin = (EMBER_ZLL_GROUP_ADDRESSES == 0
                          ? EMBER_ZLL_NULL_GROUP_ID
                          : EMBER_ZLL_MIN_GROUP_ID);
    token.freeGroupIdMin = EMBER_ZLL_MIN_GROUP_ID + EMBER_ZLL_GROUP_ADDRESSES;
    token.freeGroupIdMax = EMBER_ZLL_MAX_GROUP_ID;
    emberZllSetTokenStackZllData(&token);

    if (touchLinkInProgress()) {
      // Save the new channel.
      // (Use the Ember API, not the framework API, otherwise host-side returns zero if network down)
      currentChannel = emberGetRadioChannel();

      // Switch back to the target's current channel to send the network request,
      // in case we formed on a different one.
      emberSetLogicalAndRadioChannel(emAfZllNetwork.zigbeeNetwork.channel);

      // Set the target's new channel to our current channel.
      emAfZllNetwork.zigbeeNetwork.channel = currentChannel;

      debugPrintln("Zll Comm stack status: our chan = %d, target chan = %d", currentChannel, emAfZllNetwork.zigbeeNetwork.channel);

      // Kick the client touchlinking event.
      emberEventControlSetActive(emberAfPluginZllCommissioningClientTouchLinkEventControl);
    }
  } else if (touchLinkInProgress()) {
    emberAfAppPrintln("%p%p%p0x%x",
                      "Error: ",
                      "Touch linking failed: ",
                      "could not form network: ",
                      status);
    emAfZllAbortTouchLink(EMBER_AF_ZLL_NETWORK_FORMATION_FAILED);
  }
  emAfZllFlags &= ~FORMING_NETWORK;
}

EmberStatus emAfZllFormNetworkForRouterInitiator(uint8_t channel, int8_t radioPower, EmberPanId panId)
{
  EmberStatus status;

  // Initialize ZLL security.
  status = emberAfZllSetInitialSecurityState();
  if (status != EMBER_SUCCESS) {
    debugPrintln("emAfZllFormNetworkForRouterInitiator: unable to initialize security, status = %X", status);
    return status;
  }

  // Form network with a random extended PAN ID, if no channel is specified, otherwise
  // assume the caller has set the extended PAN ID. (The emAfExtendedPanId value
  // is global to the framework and used in the network-creator plugin.)
  if (channel == 0) {
    MEMSET(emAfExtendedPanId, 0xFF, EXTENDED_PAN_ID_SIZE);
  }

  // Per the BDB spec, we scan on the ZLL primary channels, unless a channel
  // has been specified (e.g. for a 'plugin test-harness z3 start-as-router')
  emAfPluginNetworkCreatorPrimaryChannelMask = channel ? (1 << channel) : emberGetZllPrimaryChannelMask();
  emAfPluginNetworkCreatorSecondaryChannelMask = 0;

  // Set node id for emberFormNetwork, to override random id generation.
  emberSetNodeId(EMBER_ZLL_MIN_NODE_ID);  // our node id = 0x0001

  // Set the radio transmit power and PAN id for the Network Creator callbacks.
  networkCreatorRadioPower = radioPower;
  networkCreatorPanId = panId;

  debugPrintln("emAfZllFormNetworkForRouterInitiator: chan mask = %4X", emAfPluginNetworkCreatorPrimaryChannelMask);

  status = emberAfPluginNetworkCreatorStart(false); // distributed network

  if (status == EMBER_SUCCESS) {
    emAfZllFlags |= FORMING_NETWORK;
  }
  return status;
}
#endif // #ifdef EMBER_AF_PLUGIN_NETWORK_CREATOR

// This function is required to support the following CLI commands:
//   plugin test-harness z3 set-pan-id
//   plugin test-harness z3 start-as-router
void emAfZllSetPanId(EmberPanId panId)
{
  networkCreatorPanId = panId;
}

bool emAfPluginZllCommissioningClientInterpanPreMessageReceivedCallback(const EmberAfInterpanHeader *header,
                                                                        uint8_t msgLen,
                                                                        uint8_t *message)
{
  uint32_t transaction;
  uint8_t frameControl, commandId, msgIndex;

  // If the message isn't for the ZLL Commissioning cluster, drop it with an
  // indication that we didn't handle it.
  if (header->profileId != EMBER_ZLL_PROFILE_ID
      || header->clusterId != ZCL_ZLL_COMMISSIONING_CLUSTER_ID) {
    return false;
  }

  if (header->messageType != EMBER_AF_INTER_PAN_UNICAST
      || !(header->options & EMBER_AF_INTERPAN_OPTION_MAC_HAS_LONG_ADDRESS)
      || msgLen < ZLL_HEADER_OVERHEAD) {
    return false;
  }

  // Verify that the frame control byte makes sense.  Accept only the legacy
  // format or simple client-to-server or server-to-client messages (i.e., no
  // manufacturer-specific commands, etc.)  For non-legacy messages, check that
  // the frame control is correct for the command.  The check is based on
  // DeviceInformationResponse because it is the only server-to-client command
  // we care about.
  frameControl = message[ZLL_HEADER_FRAME_CONTROL_OFFSET];
  commandId = message[ZLL_HEADER_COMMAND_ID_OFFSET];
  if (frameControl != ZLL_FRAME_CONTROL_LEGACY
      && frameControl != ZLL_FRAME_CONTROL_SERVER_TO_CLIENT) {
    return false;
  }

  msgIndex = ZLL_HEADER_TRANSACTION_ID_OFFSET;
  transaction = emberAfGetInt32u(message, msgIndex, msgLen);
  msgIndex += 4;

  if (commandId == ZCL_DEVICE_INFORMATION_RESPONSE_COMMAND_ID) {
    if (msgIndex + 3 <= msgLen) {
      uint8_t numberOfSubDevices, startIndex, deviceInformationRecordCount;
      numberOfSubDevices = emberAfGetInt8u(message, msgIndex, msgLen);
      msgIndex++;
      startIndex = emberAfGetInt8u(message, msgIndex, msgLen);
      msgIndex++;
      deviceInformationRecordCount = emberAfGetInt8u(message, msgIndex, msgLen);
      msgIndex++;
      if ((msgIndex
           + deviceInformationRecordCount * ZLL_DEVICE_INFORMATION_RECORD_SIZE)
          <= msgLen) {
        uint8_t *deviceInformationRecordList = message + msgIndex;
        deviceInformationResponseHandler(header->longAddress,
                                         transaction,
                                         numberOfSubDevices,
                                         startIndex,
                                         deviceInformationRecordCount,
                                         deviceInformationRecordList);
      }
    }
    return true;
  }

  // Return false for unhandled messages, since we are multiplexing the pre-message handler.
  return false;
}

//------------------------------------------------------------------------------
// Public functions

EmberStatus emberAfZllInitiateTouchLink(void)
{
  return startScan(SCAN_FOR_TOUCH_LINK);
}

EmberStatus emberAfZllDeviceInformationRequest(void)
{
  return startScan(SCAN_FOR_DEVICE_INFORMATION);
}

EmberStatus emberAfZllIdentifyRequest(void)
{
  return startScan(SCAN_FOR_IDENTIFY);
}

EmberStatus emberAfZllResetToFactoryNewRequest(void)
{
  return startScan(SCAN_FOR_RESET);
}

void emberAfZllAbortTouchLink(void)
{
  if (touchLinkInProgress()) {
    // If the scanning portion of touch linking is already finished, we can
    // abort right away.  If not, we need to stop the scan and wait for the
    // stack to inform us when the scan is done.
    emberAfAppPrintln("%p%p%p",
                      "Error: ",
                      "Touch linking failed: ",
                      "aborted by application");
    if (scanComplete()) {
      emAfZllAbortTouchLink(EMBER_AF_ZLL_ABORTED_BY_APPLICATION);
    } else {
      emAfZllFlags |= ABORTING_TOUCH_LINK;
      emberStopScan();
    }
  }
}

void ezspZllNetworkFoundHandler(EmberZllNetwork *networkInfo,
                                bool isDeviceInfoNull,
                                EmberZllDeviceInfoRecord *deviceInfo,
                                uint8_t lastHopLqi,
                                int8_t lastHopRssi)
{
  debugPrintln("ezspZllNwkFound: node type = %d, flags = %0x2X", networkInfo->nodeType, emAfZllFlags);
  if (touchLinkInProgress()) {
    if (isTouchlinkPermitted(networkInfo)) {
      if (!targetNetworkFound()
          || 0 < targetCompare(networkInfo, lastHopRssi, &emAfZllNetwork, rssi)) {
        MEMMOVE(&emAfZllNetwork, networkInfo, sizeof(EmberZllNetwork));
        emAfZllSubDeviceCount = 0;
        if (!isDeviceInfoNull) {
          MEMMOVE(&emAfZllSubDevices[0], deviceInfo, sizeof(EmberZllDeviceInfoRecord));
          emAfZllSubDeviceCount++;
        }
        rssi = lastHopRssi;
        emAfZllFlags |= TARGET_NETWORK_FOUND;
      }
    } else {
      debugPrintln("ezspZllNwkFound: touchlink is not permitted");
    }
  }
}

#ifndef EZSP_HOST
void emberZllNetworkFoundHandler(const EmberZllNetwork *networkInfo,
                                 const EmberZllDeviceInfoRecord *deviceInfo)
{
  debugPrintln("emberZllNwkFound: chan = %d, node type = %d, flags = %02X", networkInfo->zigbeeNetwork.channel, networkInfo->nodeType, emAfZllFlags);
  if (touchLinkInProgress()) {
    if (isTouchlinkPermitted(networkInfo)) {
      int8_t lastHopRssi;
      emberGetLastHopRssi(&lastHopRssi);
      if (!targetNetworkFound()
          || 0 < targetCompare(networkInfo, lastHopRssi, &emAfZllNetwork, rssi)) {
        MEMMOVE(&emAfZllNetwork, networkInfo, sizeof(EmberZllNetwork));
        emAfZllSubDeviceCount = 0;
        if (deviceInfo != NULL) {
          MEMMOVE(&emAfZllSubDevices[0], deviceInfo, sizeof(EmberZllDeviceInfoRecord));
          emAfZllSubDeviceCount++;
        }
        rssi = lastHopRssi;
        emAfZllFlags |= TARGET_NETWORK_FOUND;
      }
    } else {
      debugPrintln("emberZllNwkFound: touchlink is not permitted");
    }
  }

#ifdef EMBER_AF_PLUGIN_TEST_HARNESS_Z3
  emAfPluginTestHarnessZ3ZllNetworkFoundCallback(networkInfo);
#endif
}
#endif // EZSP_HOST

void ezspZllScanCompleteHandler(EmberStatus scanStatus)
{
  debugPrintln("%p: scan complete", PLUGIN_NAME);
  processScanComplete(scanStatus);
}

void emberZllScanCompleteHandler(EmberStatus scanStatus)
{
  debugPrintln("%p: scan complete", PLUGIN_NAME);
  processScanComplete(scanStatus);

#ifdef EMBER_AF_PLUGIN_TEST_HARNESS_Z3
  emAfPluginTestHarnessZ3ZllScanCompleteCallback(scanStatus);
#endif
}

void ezspZllAddressAssignmentHandler(EmberZllAddressAssignment *addressInfo,
                                     uint8_t lastHopLqi,
                                     int8_t lastHopRssi)
{
  if (touchLinkInProgress()) {
    emAfZllNetwork.nodeId = addressInfo->nodeId;
  }
}

void emberZllAddressAssignmentHandler(const EmberZllAddressAssignment *addressInfo)
{
  if (touchLinkInProgress()) {
    debugPrintln("emberZllAddressAssignmentHandler: node id = %2X", addressInfo->nodeId);
    emAfZllNetwork.nodeId = addressInfo->nodeId;
  }
}

void emberAfPluginZllCommissioningClientTouchLinkEventHandler(void)
{
  emberEventControlSetInactive(emberAfPluginZllCommissioningClientTouchLinkEventControl);
  debugPrintln("TouchlinkEventHandler: flags = %X", emAfZllFlags);
  if (touchLinkInProgress()) {
    EmberStatus status;

    sendIdentifyRequest(0x0000); // exit identify mode

    if (scanForTouchLink()) {
      // If we are not factory new, we want to bring the target into our
      // existing network, so we set the channel to our own channel, either
      // our original channel, or the channel we formed on, if we are (were)
      // a factory-new router initiator.
      // Otherwise, we'll use whatever channel the target is on presently.
      if (!emAfZllAmFactoryNew()) {
        emAfZllNetwork.zigbeeNetwork.channel = currentChannel;
      }
#ifdef EMBER_AF_PLUGIN_NETWORK_CREATOR
      else if (emAfCurrentZigbeeProNetwork != NULL
               && emAfCurrentZigbeeProNetwork->nodeType <= EMBER_ROUTER) {
        // If we are factory new and a router, then we need to form a network
        // to become non-factory new. See the BDB spec's touchlink procedure
        // for an initiator, steps 13, 21 and 22.
        status = emAfZllFormNetworkForRouterInitiator(0,
                                                      EMBER_AF_PLUGIN_ZLL_COMMISSIONING_COMMON_RADIO_TX_POWER,
                                                      0xffff);
        if (status != EMBER_SUCCESS) {
          emberAfAppPrintln("%p%p%p0x%x",
                            "Error: ",
                            "Touch linking failed: ",
                            "could not start network formation: ",
                            status);
          emAfZllAbortTouchLink(EMBER_AF_ZLL_NETWORK_FORMATION_FAILED);
        }
        return;
      }
#endif // #ifdef EMBER_AF_PLUGIN_NETWORK_CREATOR
      else {
        // If we are a factory new (end device) initiator, then let the
        // target chose the channel.
        emAfZllNetwork.zigbeeNetwork.channel = 0;
      }
      emberAfZllSetInitialSecurityState();
      emberAfAppPrintln("%p%p%p",
                        "Sending ",
                        emAfZllAmFactoryNew() ? "start" : "join",
                        " request to target");
      emAfInitiatorRejoinRetryCount = 0;
      status = emberZllJoinTarget(&emAfZllNetwork);
      if (status != EMBER_SUCCESS) {
        // Check that it's not a non-fatal status, for example, if
        // emberZllJoinTarget() is sending a network update.
        if (status == EMBER_OPERATION_IN_PROGRESS) {
          // Abort, but report touchlink complete to application.
          emAfZllAbortTouchLink(EMBER_AF_ZLL_NETWORK_UPDATE_OPERATION);
        } else {
          emberAfAppPrintln("%p%p%p0x%x",
                            "Error: ",
                            "Touch linking failed: ",
                            "could not send start/join: ",
                            status);
          emAfZllAbortTouchLink(EMBER_AF_ZLL_SENDING_START_JOIN_FAILED);
        }
        return;
      }
    } else {
      if (scanForReset()) {
        status = sendResetToFactoryNewRequest();
        if (status != EMBER_SUCCESS) {
          emberAfAppPrintln("%p%p%p0x%x",
                            "Error: ",
                            "Touch linking failed: ",
                            "could not send reset: ",
                            status);
          emAfZllAbortTouchLink(EMBER_AF_ZLL_SENDING_RESET_TO_FACTORY_NEW_REQUEST_FAILED);
          return;
        }
      }
      emberSetRadioChannel(currentChannel);
      emAfZllTouchLinkComplete();
    }
  }
}

/** @brief Get Pan Id
 *
 * This callback is called when the Network Creator plugin needs the PAN ID for
 * the network it is about to create. By default, the callback will return a
 * random 16-bit value.
 *
 */
EmberPanId emberAfPluginNetworkCreatorGetPanIdCallback(void)
{
  return (networkCreatorPanId == 0xFFFF
          ? emberGetPseudoRandomNumber()
          : networkCreatorPanId);
}

/** @brief Get Power For Radio Channel
 *
 * This callback is called when the Network Creator plugin needs the radio power for
 * the network it is about to create. By default, the callback will use the radio
 * power specified in the relevant plugin option.
 *
 */
int8_t emberAfPluginNetworkCreatorGetRadioPowerCallback(void)
{
  return networkCreatorRadioPower;
}
