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
 * @brief Routines for the ZLL Commissioning Server plugin, which implement the
 *        server side of the ZLL protocol.
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

//------------------------------------------------------------------------------
// Globals

#ifdef EMBER_AF_PLUGIN_ZLL_COMMISSIONING_SERVER_STEALING_ALLOWED
  #define STEALING_ALLOWED (true)
#else
  #define STEALING_ALLOWED (false)
#endif
static const boolean stealingAllowed = STEALING_ALLOWED;
#ifdef EMBER_AF_PLUGIN_ZLL_COMMISSIONING_SERVER_REMOTE_RESET_ALLOWED
  #define REMOTE_RESET_ALLOWED (true)
#else
  #define REMOTE_RESET_ALLOWED (false)
#endif
static const boolean remoteResetAllowed = REMOTE_RESET_ALLOWED;

#ifdef PLUGIN_DEBUG
static const uint8_t emAfZllCommissioningPluginName[] = "ZLL Commissioning Server";
#define PLUGIN_NAME emAfZllCommissioningPluginName
#endif

#ifdef EMBER_AF_PLUGIN_ZLL_COMMISSIONING_SERVER_RX_ALWAYS_ON
  #define RX_ON_PERIOD_MS 0xFFFFFFFFUL
#else
  #define RX_ON_PERIOD_MS (EMBER_AF_PLUGIN_ZLL_COMMISSIONING_SERVER_RX_ON_AT_STARTUP_PERIOD * 1000UL)
#endif // EMBER_AF_PLUGIN_ZLL_COMMISSIONING_SERVER_RX_ALWAYS_ON

// The target network - used here for the deviceInformationRequestHandler to
// determine if the request was from a legacy device.
extern EmberZllNetwork emAfZllNetwork;

// Private ZLL commissioning functions
bool emAfZllAmFactoryNew(void);
EmberNodeType emAfZllGetLogicalNodeType(void);
EmberZllPolicy emAfZllGetPolicy(void);

//------------------------------------------------------------------------------
// Module private functions

static void deviceInformationRequestHandler(const EmberEUI64 source,
                                            uint32_t transaction,
                                            uint8_t startIndex)
{
  EmberEUI64 eui64;
  EmberStatus status;
  uint8_t total = emberAfEndpointCount();
  uint8_t i, *count;

  emberAfZllCommissioningClusterPrintln("RX: DeviceInformationRequest 0x%4x, 0x%x",
                                        transaction,
                                        startIndex);

  emberAfFillExternalBuffer((ZCL_CLUSTER_SPECIFIC_COMMAND
                             | ZCL_FRAME_CONTROL_SERVER_TO_CLIENT
                             | ZCL_DISABLE_DEFAULT_RESPONSE_MASK),
                            ZCL_ZLL_COMMISSIONING_CLUSTER_ID,
                            ZCL_DEVICE_INFORMATION_RESPONSE_COMMAND_ID,
                            "wuu",
                            transaction,
                            total,
                            startIndex);

  emberAfGetEui64(eui64);
  count = &appResponseData[appResponseLength];
  emberAfPutInt8uInResp(0); // temporary count
  // EMAPPFWKV2-1450: only send up to 4 endpoint information chunks at a time.
  // Otherwise, the payload will be too large. ZCL6 says the following in
  // 13.3.2.2.2.4 regarding the device information record count field:
  //   "This value shall be in the range 0x00 – 0x05."

  for (i = startIndex; i < total && *count < 4; i++) {
    // If the profile interop bit in the ZllInformation bitmask is cleared,
    // then we know this is a legacy ZLL app, so we set the profile ID in our
    // response to the ZLL profile ID. If the bit is set, then we use the profile
    // ID specified for the endpoint.
    uint8_t deviceVersion = emberAfDeviceVersionFromIndex(i);
    uint8_t endpoint = emberAfEndpointFromIndex(i);
    EmberAfProfileId profileId = (emAfZllNetwork.state & EMBER_ZLL_STATE_PROFILE_INTEROP
                                  ? emberAfProfileIdFromIndex(i)
                                  : EMBER_ZLL_PROFILE_ID);
    emberAfPutBlockInResp(eui64, EUI64_SIZE);
    emberAfPutInt8uInResp(endpoint);
    emberAfPutInt16uInResp(profileId);

    // Note that certain legacy initiators require a ZLL 1.0 device
    // ID for Endpoint 1.
    emberAfPutInt16uInResp(emberAfDeviceIdFromIndex(i));
    emberAfPutInt8uInResp(deviceVersion);
    emberAfPutInt8uInResp(emberAfPluginZllCommissioningServerGroupIdentifierCountCallback(endpoint));
    emberAfPutInt8uInResp(i); // sort order
    (*count)++;
  }

  status = emberAfSendCommandInterPan(0xFFFF,                // destination pan id
                                      source,
                                      EMBER_NULL_NODE_ID,    // node id - ignored
                                      0x0000,                // group id - ignored
                                      EMBER_ZLL_PROFILE_ID);
  if (status != EMBER_SUCCESS) {
    emberAfZllCommissioningClusterPrintln("%p%p failed 0x%x",
                                          "Error: ",
                                          "Device information response",
                                          status);
  }
}

static void identifyRequestHandler(const EmberEUI64 source,
                                   uint32_t transaction,
                                   uint16_t identifyDurationS)
{
  if ((emAfZllGetPolicy() & EMBER_ZLL_POLICY_STEALING_ENABLED)
      || emAfZllAmFactoryNew()) {
    emberAfZllCommissioningClusterPrintln("RX: IdentifyRequest 0x%4x, 0x%2x",
                                          transaction,
                                          identifyDurationS);
    emberAfPluginZllCommissioningServerIdentifyCallback(identifyDurationS);
  } else {
    emberAfZllCommissioningClusterPrintln("ignoring identify request as touchlinking policy is disabled for NFN");
  }
}

static void resetToFactoryNewRequestHandler(const EmberEUI64 source,
                                            uint32_t transaction)
{
  emberAfZllCommissioningClusterPrintln("RX: ResetToFactoryNewRequest 0x%4x",
                                        transaction);
  if (!emAfZllAmFactoryNew()) {
    EmberCurrentSecurityState securityState;

    // Remote reset may be disabled, but only for a device on a centralized network.
    if (emberGetCurrentSecurityState(&securityState) != EMBER_SUCCESS  // not joined
        || (securityState.bitmask & EMBER_DISTRIBUTED_TRUST_CENTER_MODE)
        || (emAfZllGetPolicy() & EMBER_ZLL_POLICY_REMOTE_RESET_ENABLED)) {
      emAfZllFlags = TOUCH_LINK_TARGET;  // note reset via interpan
      emberAfZllResetToFactoryNew();
    }
  }
}

//------------------------------------------------------------------------------
// ZLL commissioning private functions

bool emAfZllStealingAllowed(void)
{
  return stealingAllowed;
}

bool emAfZllRemoteResetAllowed(void)
{
  return remoteResetAllowed;
}

void emAfZllInitializeRadio(void)
{
  // Turn the receiver on for a user-specified period, in order to allow incoming.
  // scan requests, but cancel any outstanding rx on period first, in case we've
  // got here by a touchlink reset.
  emberZllCancelRxOnWhenIdle(); // cancel rx on
#if RX_ON_PERIOD_MS != 0
  emberAfAppPrintln("Setting rx on period to %d", RX_ON_PERIOD_MS);
  emberZllSetRxOnWhenIdle(RX_ON_PERIOD_MS);
#endif

  // Tune the radio to the user-specified default channel - this will be used
  // to listen for an incoming scan request. (Note that the original ZLL specification
  // mandates a random primary channel, but the new BDB spec appears not to)
  emberSetLogicalAndRadioChannel(EMBER_AF_PLUGIN_ZLL_COMMISSIONING_SERVER_DEFAULT_RADIO_CHANNEL);
  emberAfAppPrintln("Setting default channel to %d", EMBER_AF_PLUGIN_ZLL_COMMISSIONING_SERVER_DEFAULT_RADIO_CHANNEL);
}

bool emAfPluginZllCommissioningServerInterpanPreMessageReceivedCallback(const EmberAfInterpanHeader *header,
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
      && frameControl != ZLL_FRAME_CONTROL_CLIENT_TO_SERVER) {
    return false;
  }

  msgIndex = ZLL_HEADER_TRANSACTION_ID_OFFSET;
  transaction = emberAfGetInt32u(message, msgIndex, msgLen);
  msgIndex += 4;

  switch (commandId) {
    case ZCL_DEVICE_INFORMATION_REQUEST_COMMAND_ID:
      if (msgIndex + 1 <= msgLen) {
        uint8_t startIndex = emberAfGetInt8u(message, msgIndex, msgLen);
        deviceInformationRequestHandler(header->longAddress,
                                        transaction,
                                        startIndex);
      }
      return true;
    case ZCL_IDENTIFY_REQUEST_COMMAND_ID:
      if (msgIndex + 2 <= msgLen) {
        uint16_t identifyDurationS = emberAfGetInt16u(message, msgIndex, msgLen);
        identifyRequestHandler(header->longAddress,
                               transaction,
                               identifyDurationS);
      }
      return true;
    case ZCL_RESET_TO_FACTORY_NEW_REQUEST_COMMAND_ID:
      // Ignore reset request if we are a co-ordinator
      if (emAfZllGetLogicalNodeType() != EMBER_COORDINATOR) {
        resetToFactoryNewRequestHandler(header->longAddress, transaction);
      }
      return true;
  }

  // Return false for unhandled messages, since we are multiplexing the pre-message handler.
  return false;
}

//------------------------------------------------------------------------------
// Public functions

EmberStatus emberAfZllNoTouchlinkForNFN(void)
{
  // Note, to reenable stealing, it is necessary to call emberAfZllEnable().
  EmberZllPolicy policy = emAfZllGetPolicy();
  EmberStatus status;
#ifndef EZSP_HOST
  status = emberZllSetPolicy(policy & ~EMBER_ZLL_POLICY_STEALING_ENABLED);
#else
  status = emberAfSetEzspPolicy(EZSP_ZLL_POLICY,
                                policy & ~EMBER_ZLL_POLICY_STEALING_ENABLED,
                                "ZLL policy with no touchlink for NFN",
                                "enable");
#endif
  return status;
}

EmberStatus emberAfZllNoResetForNFN(void)
{
  // Note, to reenable remote reset, it is necessary to call emberAfZllEnable().
  // Note also that the policy is only honoured if the device is on a centralized
  // security network.
  EmberZllPolicy policy = emAfZllGetPolicy();
  EmberStatus status;
#ifndef EZSP_HOST
  status = emberZllSetPolicy(policy & ~EMBER_ZLL_POLICY_REMOTE_RESET_ENABLED);
#else
  status = emberAfSetEzspPolicy(EZSP_ZLL_POLICY,
                                policy & ~EMBER_ZLL_POLICY_REMOTE_RESET_ENABLED,
                                "ZLL policy with no remote reset for NFN",
                                "enable");
#endif
  return status;
}

void ezspZllTouchLinkTargetHandler(EmberZllNetwork *networkInfo)
{
  debugPrintln("%p: touchlink target handler", PLUGIN_NAME);
  MEMMOVE(&emAfZllNetwork, networkInfo, sizeof(EmberZllNetwork));
#ifdef EMBER_AF_PLUGIN_ZLL_COMMISSIONING_CLIENT
  emAfZllSubDeviceCount = 0;
#endif
  emAfZllFlags = TOUCH_LINK_TARGET;
}

void emberZllTouchLinkTargetHandler(const EmberZllNetwork *networkInfo)
{
  debugPrintln("%p: touchlink target handler", PLUGIN_NAME);
  MEMMOVE(&emAfZllNetwork, networkInfo, sizeof(EmberZllNetwork));
#ifdef EMBER_AF_PLUGIN_ZLL_COMMISSIONING_CLIENT
  emAfZllSubDeviceCount = 0;
#endif
  emAfZllFlags = TOUCH_LINK_TARGET;
}
