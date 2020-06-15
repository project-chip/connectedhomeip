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
 * @brief CLI for the ZLL Commissioning Common plugin.
 *******************************************************************************
   ******************************************************************************/

#include "app/framework/include/af.h"
#include "app/util/serial/command-interpreter2.h"
#include "app/framework/plugin/zll-commissioning-common/zll-commissioning-common.h"
#include "app/framework/plugin/zll-commissioning-client/zll-commissioning-client.h"
#include "app/framework/plugin/zll-commissioning-server/zll-commissioning-server.h"

// ZLL commissioning commands.
void formNetwork(void);
void initiateTouchLink(void);
void scanTouchLink(void);
void abortTouchLink(void);

// ZLL ZCL commands.
void endpointInformation(void);
void getGroupIdentifiersRequest(void);
void getEndpointListRequest(void);

// ZLL misc commands
void printChannels(void);
void printZllTokens(void);
void setScanChannel(void);
void setScanMask(void);
void setIdentifyDuration(void);
void statusCommand(void);

// ZLL network interoperability commands.
void joinable(void);
void unused(void);

// ZLL stealing control commands.
void disable(void);
void enable(void);
void noTouchlinkForNFN(void);

#ifndef EZSP_HOST
// ZLL rx-on-when-idle commands.
void setRxOn(void);
void cancelRxOn(void);
void rxOnStatus(void);
#endif

// Internal APIs.
EmberStatus emAfZllFormNetwork(uint8_t channel, int8_t power, EmberPanId panId);
EmberZllPolicy emAfZllGetPolicy(void);

#ifndef EMBER_AF_GENERATE_CLI
EmberCommandEntry emberAfPluginZllCommissioningCommands[] = {
  emberCommandEntryAction("form", formNetwork, "usv", "Forms a ZLL network"),
  emberCommandEntryAction("link", initiateTouchLink, "?", "Initiates a touch link"),
  emberCommandEntryAction("abort", abortTouchLink, "", "Aborts a previously initiated touch link"),
  emberCommandEntryAction("info", endpointInformation, "vuu", "Sends an Endpoint Information request"),
  emberCommandEntryAction("groups", getGroupIdentifiersRequest, "vuuu", "Sends a Group Identifier Request"),
  emberCommandEntryAction("endpoints", getEndpointListRequest, "vuuu", "Sends a Get Endpoint List request"),
  emberCommandEntryAction("tokens", printZllTokens, "", "Prints the internal ZLL tokens"),
  emberCommandEntryAction("channel", setScanChannel, "u", "Sets the primary scan channel"),
  emberCommandEntryAction("secondary-channel", setSecondaryScanChannel, "u", "Sets the secondary scan channel"),
  emberCommandEntryAction("mask", setScanMask, "u", "Sets the scan channel mask"),
  emberCommandEntryAction("identify", setIdentifyDuration, "v", "Sets the identify duration"),
  emberCommandEntryAction("status", statusCommand, "", "Prints the ZLL status"),
  emberCommandEntryAction("joinable", joinable, "", "Attempts to join any Zigbee network"),
  emberCommandEntryAction("unused", unused, "", "Attempt to form on an unused PAN ID"),
  emberCommandEntryAction("reset", emberAfZllResetToFactoryNew, "", "Resets the local device to factory new"),
  emberCommandEntryAction("enable", enable, "", "enable touchlinking for target"),
  emberCommandEntryAction("disable", disable, "", "disable touchlinking for target"),
  emberCommandEntryAction("notouchlink-nfn", noTouchlinkForNFN, "", "No touchlinking for NFN for target"),
  emberCommandEntryAction("set-rx-on", setRxOn, "", "Set Rx On When Idle duration"),
  emberCommandEntryAction("cancel-rx-on", cancelRxOn, "", "Cancel Rx On When Idle"),
  emberCommandEntryAction("rx-on-active", rxOnStatus, "", "Get Rx On When Idle status"),
  emberCommandEntryTerminator(),
};
#endif //  EMBER_AF_GENERATE_CLI

static uint32_t channelMasks[] = {
  0x02108800UL, // standard (11, 15, 20, 25)
  0x04211000UL, // +1 (12, 16, 21, 26)
  0x004A2000UL, // +2 (13, 17, 22, 19)
  0x01844000UL, // +3 (14, 18, 23, 24)
  0x07FFF800UL, // all (11--26)
};

// plugin zll-commissioning form <channel:1> <power:1> <pan id:2>
void formNetwork(void)
{
  EmberStatus status = emAfZllFormNetwork((uint8_t)emberUnsignedCommandArgument(0),       // channel
                                          (int8_t)emberSignedCommandArgument(1),         // power
                                          (EmberPanId)emberUnsignedCommandArgument(2)); // pan id
  emberAfAppPrintln("%p 0x%x", "form", status);
}

// Leaving intact for legacy CLI purposes, but for generated CLI, the options have been moved to
// scanTouchLink().

// plugin zll-commissioning link [device|identify|reset]
void initiateTouchLink(void)
{
#ifdef EMBER_AF_PLUGIN_ZLL_COMMISSIONING_CLIENT
  EmberStatus status;
  switch (emberCommandArgumentCount()) {
    case 0:
      status = emberAfZllInitiateTouchLink();
      emberAfAppPrintln("%p 0x%x", "touch link", status);
      return;
    case 1:
      // -1 because we're smarter than command-interpreter2.
      switch (emberStringCommandArgument(0, NULL)[-1]) {
        case 'd':
          status = emberAfZllDeviceInformationRequest();
          emberAfAppPrintln("%p 0x%x", "device information", status);
          return;
        case 'i':
          status = emberAfZllIdentifyRequest();
          emberAfAppPrintln("%p 0x%x", "identify", status);
          return;
        case 'r':
          status = emberAfZllResetToFactoryNewRequest();
          emberAfAppPrintln("%p 0x%x", "reset to factory new", status);
          return;
      }
  }
  emberAfAppPrintln("Usage:");
  emberAfAppPrintln("plugin zll-commissioning link");
  emberAfAppPrintln("plugin zll-commissioning link device");
  emberAfAppPrintln("plugin zll-commissioning link identify");
  emberAfAppPrintln("plugin zll-commissioning link reset");
#else
  emberAfAppPrintln("zll commissioning client plugin not found");
#endif
}

// Generated CLI version of "link" commands

// plugin zll-commissioning scan [device|identify|reset]
void scanTouchLink(void)
{
#ifdef EMBER_AF_PLUGIN_ZLL_COMMISSIONING_CLIENT
  EmberStatus status;
  switch (emberStringCommandArgument(-1, NULL)[0]) {
    case 'd':
      status = emberAfZllDeviceInformationRequest();
      emberAfAppPrintln("%p 0x%x", "device information", status);
      return;
    case 'i':
      status = emberAfZllIdentifyRequest();
      emberAfAppPrintln("%p 0x%x", "identify", status);
      return;
    case 'r':
      status = emberAfZllResetToFactoryNewRequest();
      emberAfAppPrintln("%p 0x%x", "reset to factory new", status);
      return;
  }
#else
  emberAfAppPrintln("zll commissioning client plugin not found");
#endif
}

// plugin zll-commissioning abort
void abortTouchLink(void)
{
#ifdef EMBER_AF_PLUGIN_ZLL_COMMISSIONING_CLIENT
  emberAfZllAbortTouchLink();
  emberAfAppPrintln("%p", "touch link aborted");
#else
  emberAfAppPrintln("zll commissioning client plugin not found");
#endif
}

// plugin zll-commissioning info <destination:2> <src endpoint:1> <dst endpoint:1>
void endpointInformation(void)
{
  EmberEUI64 eui64;
  EmberStatus status = EMBER_INVALID_ENDPOINT;
  uint8_t endpoint = (uint8_t)emberUnsignedCommandArgument(1);
  uint8_t index = emberAfIndexFromEndpoint(endpoint);
  if (index != 0xFF) {
    // If the profile interop bit in the ZllInformation bitmask is cleared,
    // then we set the profile ID in our response to the ZLL profile ID.
    // If the bit is set, then we use the profile ID specified for the endpoint.
    uint8_t deviceVersion = emberAfDeviceVersionFromIndex(index);
    EmberTokTypeStackZllData token;
    emberZllGetTokenStackZllData(&token);
    EmberAfProfileId profileId = (token.bitmask & EMBER_ZLL_STATE_PROFILE_INTEROP
                                  ? emberAfProfileIdFromIndex(index)
                                  : EMBER_ZLL_PROFILE_ID);
    emberAfGetEui64(eui64);
    emberAfFillCommandZllCommissioningClusterEndpointInformation(eui64,
                                                                 emberAfGetNodeId(),
                                                                 endpoint,
                                                                 profileId,
                                                                 emberAfDeviceIdFromIndex(index),
                                                                 deviceVersion);
    emberAfSetCommandEndpoints(endpoint,
                               (uint8_t)emberUnsignedCommandArgument(2));
    status = emberAfSendCommandUnicast(EMBER_OUTGOING_DIRECT,
                                       (uint16_t)emberUnsignedCommandArgument(0));
  }
  emberAfAppPrintln("%p 0x%x", "endpoint information", status);
}

// plugin zll-commissioning groups <destination:2> <src endpoint:1> <dst endpoint:1> <startIndex:1>
void getGroupIdentifiersRequest(void)
{
  EmberStatus status = EMBER_INVALID_ENDPOINT;
  uint8_t endpoint = (uint8_t)emberUnsignedCommandArgument(1);
  uint8_t index = emberAfIndexFromEndpoint(endpoint);
  if (index != 0xFF) {
    emberAfFillCommandZllCommissioningClusterGetGroupIdentifiersRequest((uint8_t)emberUnsignedCommandArgument(3));
    emberAfSetCommandEndpoints(endpoint,
                               (uint8_t)emberUnsignedCommandArgument(2));
    status = emberAfSendCommandUnicast(EMBER_OUTGOING_DIRECT,
                                       (uint16_t)emberUnsignedCommandArgument(0));
  }
  emberAfAppPrintln("%p 0x%x", "get group identifiers", status);
}

// plugin zll-commissioning endpoints <destination:2> <src endpoint:1> <dst endpoint:1> <startIndex:1>
void getEndpointListRequest(void)
{
  EmberStatus status = EMBER_INVALID_ENDPOINT;
  uint8_t endpoint = (uint8_t)emberUnsignedCommandArgument(1);
  uint8_t index = emberAfIndexFromEndpoint(endpoint);
  if (index != 0xFF) {
    emberAfFillCommandZllCommissioningClusterGetEndpointListRequest((uint8_t)emberUnsignedCommandArgument(3));
    emberAfSetCommandEndpoints(endpoint,
                               (uint8_t)emberUnsignedCommandArgument(2));
    status = emberAfSendCommandUnicast(EMBER_OUTGOING_DIRECT,
                                       (uint16_t)emberUnsignedCommandArgument(0));
  }
  emberAfAppPrintln("%p 0x%x", "get endpoint list", status);
}

// plugin zll-commissioning set-rx-on <duration:4>
void setRxOn(void)
{
  uint32_t duration = (uint32_t)emberUnsignedCommandArgument(0);
  emberZllSetRxOnWhenIdle(duration);
  emberAfAppPrintln("%p %p %d %p", "Rx-On-When-Idle", "set for", duration, "ms");
}

// plugin zll-commissioning cancel-rx-on
void cancelRxOn(void)
{
  emberZllCancelRxOnWhenIdle(); // cancel rx on
  emberAfAppPrintln("%p %p", "Rx-On-When-Idle", "cancelled");
}

// plugin zll-commissioning rx-on-active
void rxOnStatus(void)
{
  bool status = emberZllRxOnWhenIdleGetActive();
  emberAfAppPrintln("%p %p", "Rx-On-When-Idle", status ? "active" : "inactive");
}

void printZllTokens(void)
{
  EmberTokTypeStackZllData token;
  EmberTokTypeStackZllSecurity security;
  emberZllGetTokenStackZllData(&token);
  emberZllGetTokenStackZllSecurity(&security);

  emberAfAppFlush();
  emberAfAppPrintln("ZLL Tokens");
  emberAfAppPrintln("Bitmask: 0x%4x", token.bitmask);
  emberAfAppPrintln("Free Node IDs,  Min: 0x%2x, Max: 0x%2x",
                    token.freeNodeIdMin,
                    token.freeNodeIdMax);
  emberAfAppFlush();
  emberAfAppPrintln("Free Group IDs, Min: 0x%2x, Max: 0x%2x",
                    token.freeGroupIdMin,
                    token.freeGroupIdMax);
  emberAfAppFlush();
  emberAfAppPrintln("My Group ID Min: 0x%2x",
                    token.myGroupIdMin);
  emberAfAppFlush();
  emberAfAppPrintln("RSSI Correction: %d",
                    token.rssiCorrection);
  emberAfAppFlush();

  emberAfAppPrintln("Security Bitmask: 0x%4x", security.bitmask);
  emberAfAppFlush();
  emberAfAppPrintln("Security Key Index: %d", security.keyIndex);
  emberAfAppFlush();
  emberAfAppPrint("Security Encryption Key: ");
  emberAfAppDebugExec(emberAfPrintZigbeeKey(security.encryptionKey));
  emberAfAppPrintln("");
  emberAfAppFlush();
}

void setScanChannel(void)
{
  uint8_t channel = (uint8_t)emberUnsignedCommandArgument(0);

  if (channel == 0) {
    emberSetZllPrimaryChannelMask(0);
  } else {
    if (channel > EMBER_MAX_802_15_4_CHANNEL_NUMBER
        || channel < EMBER_MIN_802_15_4_CHANNEL_NUMBER) {
      emberAfAppPrintln("Invalid channel %d", channel);
      return;
    }
    emberSetZllPrimaryChannelMask(1 << channel);

#ifdef EMBER_AF_PLUGIN_ZLL_COMMISSIONING_SERVER
    if (emberNetworkState() == EMBER_NO_NETWORK) {
      // If we are not joined, then we also change the default channel.
      emberSetLogicalAndRadioChannel(channel);
      emberAfAppPrintln("Setting default channel to %d", channel);
    }
#endif
  }

  printChannels();
}

void setSecondaryScanChannel(void)
{
  uint8_t channel = (uint8_t)emberUnsignedCommandArgument(0);

  if (channel == 0) {
    emberSetZllSecondaryChannelMask(0);
  } else {
    if (channel > EMBER_MAX_802_15_4_CHANNEL_NUMBER
        || channel < EMBER_MIN_802_15_4_CHANNEL_NUMBER) {
      emberAfAppPrintln("Invalid channel %d", channel);
      return;
    }
    emberSetZllSecondaryChannelMask(1 << channel);
  }
  printChannels();
}

void setScanMask(void)
{
  uint8_t index = (uint8_t)emberUnsignedCommandArgument(0);
  if (COUNTOF(channelMasks) <= index) {
    emberAfAppPrintln("Invalid channel mask index %d", index);
    return;
  }
  emberSetZllPrimaryChannelMask(channelMasks[index]);
  printChannels();
}

void setIdentifyDuration(void)
{
#ifdef EMBER_AF_PLUGIN_ZLL_COMMISSIONING_CLIENT
  emAfZllIdentifyDurationSec = (uint16_t)emberUnsignedCommandArgument(0);
#endif //EMBER_AF_PLUGIN_ZLL_COMMISSIONING_LINK_INITIATOR
}

void statusCommand(void)
{
  printChannels();
  printZllTokens();

  // Use the Ember API, not the framework API, for host-side
  emberAfAppPrintln("%p %d", "Current channel:", emberGetRadioChannel());
  emberAfAppPrintln("%p %d", "Touchlinking policy:", emAfZllGetPolicy());
}

void printChannels(void)
{
  emberAfAppPrint("%p channels: ", "Primary");
  emberAfAppDebugExec(emberAfPrintChannelListFromMask(emberGetZllPrimaryChannelMask()));
  emberAfAppPrintln(" (0x%4x)", emberGetZllPrimaryChannelMask());
#ifdef EMBER_AF_PLUGIN_ZLL_COMMISSIONING_COMMON_SCAN_SECONDARY_CHANNELS
  emberAfAppPrint("%p channels: ", "Secondary");
  emberAfAppDebugExec(emberAfPrintChannelListFromMask(emberGetZllSecondaryChannelMask()));
  emberAfAppPrintln(" (0x%4x)", emberGetZllSecondaryChannelMask());
#endif
}

void joinable(void)
{
#ifdef EMBER_AF_PLUGIN_ZLL_COMMISSIONING_NETWORK
  EmberStatus status = emberAfStartSearchForJoinableNetworkCallback();
  emberAfAppPrintln("%p 0x%x", "joinable", status);
#else
  emberAfAppPrintln("zll commissioning network plugin not found");
#endif
}

void unused(void)
{
#ifdef EMBER_AF_PLUGIN_ZLL_COMMISSIONING_NETWORK
  EmberStatus status = emberAfFindUnusedPanIdAndFormCallback();
  emberAfAppPrintln("%p 0x%x", "unused", status);
#else
  emberAfAppPrintln("zll commissioning network plugin not found");
#endif
}

void noTouchlinkForNFN(void)
{
#ifdef EMBER_AF_PLUGIN_ZLL_COMMISSIONING_SERVER
  emberAfZllNoTouchlinkForNFN();
#else
  emberAfAppPrintln("zll commissioning server plugin not found");
#endif
}

void noResetForNFN(void)
{
#ifdef EMBER_AF_PLUGIN_ZLL_COMMISSIONING_SERVER
  emberAfZllNoResetForNFN();
#else
  emberAfAppPrintln("zll commissioning server plugin not found");
#endif
}

void enable(void)
{
  emberAfZllEnable();
}

void disable(void)
{
  emberAfZllDisable();
}
