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
 * @brief Implementations for network related CLI commands.
 *******************************************************************************
   ******************************************************************************/

#include "app/framework/include/af.h"
#include "app/framework/util/af-main.h"
#include "app/framework/util/util.h"
#include "app/util/serial/command-interpreter2.h"
#include "app/util/zigbee-framework/zigbee-device-common.h"
#include "network-cli.h"

// emberFindAndRejoinNetworkWithReason
#ifdef EZSP_HOST
  #include "app/util/ezsp/ezsp.h"
#else
  #include "stack/include/network-formation.h"
#endif

//------------------------------------------------------------------------------

void networkJoinCommand(void);
void networkRejoinCommand(void);
void networkFormCommand(void);
void networkExtendedPanIdCommand(void);
void networkLeaveCommand(void);
void networkPermitJoinCommand(void);
void findJoinableNetworkCommand(void);
void findUnusedPanIdCommand(void);
void networkChangeChannelCommand(void);
void networkSetCommand(void);
void networkIdCommand(void);
void networkMultiPhyStartCommand(void);
void networkMultiPhyStopCommand(void);
void networkChangeChildTimeoutOptionMaskCommand(void);
void changeKeepAliveModeCommand(void);

// TODO: Gate this again when we have the mechanism for doing so
// with the generated CLI.
//#if defined(EMBER_AF_TC_SWAP_OUT_TEST)
void networkInitCommand(void);
//#endif

/**
 * @addtogroup cli
 * @{
 */
/**
 * @brief
 *        <b>network form &lt;channel&gt; &lt;power> &lt;panid></b>
 *        - <i>Form a network on a given channel, with a given TX Power and PAN ID.</i>
 *           - <i>channel - uint8_t. The channel on which to form the network.</i>
 *           - <i>power   - int8_t. One byte signed value indicating the TX Power
 *                                 that the radio should be set to.</i>
 *           - <i>panId   - uint16_t. The PAN ID on which to form the network.</i>
 *
 *        <b>network join &lt;channel&gt; &lt;power&gt; &lt;panid&gt;</b>
 *        - <i>Join a network on a given channel, with a given TX Power and PAN ID.</i>
 *           - <i>channel - uint8_t. The channel on which to join the network.</i>
 *           - <i>power   - int8_t. One byte signed value indicating the TX Power
 *                                 that the radio should be set to.</i>
 *           - <i>panId   - uint16_t. The PAN ID on which to join the network.</i>
 *
 *        <b>network rejoin &lt;encrypted&gt;</b>
 *        - <i>Find and rejoin the previous network the device was connected to.</i>
 *           - <i>encrypted - uint8_t. A single byte boolean 0 or 1 indicating
 *                            whether or not the rejoin should be done with encryption.</i>
 *
 *        <b>network leave</b>
 *        - <i>Leave the current network that the device is connected to.</i>
 *
 *        <b>network pjoin &lt;seconds&gt;</b>
 *        - <i>Turn permit joining on for the amount of time indicated.</i>
 *           - <i>seconds - uint8_t. A single byte indicating how long the device
 *                         should have permit joining turn on for. A value of
 *                         0xff turns permit join indefinitely.</i>
 *
 *        <b>network broad-pjoin &lt;seconds&gt;</b>
 *        - <i>Turn permit joining on for the amount of time indicated AND
 *             broadcast a ZDO Mgmt Permit Joining request to all routers.
 *           - <i>seconds - uint8_t. A single byte indicating how long the device
 *                         should have permit joining turn on for. A value of
 *                         0xff turns permit join indefinitely.</i>
 *
 *        <b>network extpanid &lt;bytes&gt;</b>
 *        - <i>Write the extended PAN ID of the device.</i>
 *           - <i>bytes - byte array. An array of bytes which represents
 *                        the extended PAN ID for the device.</i>
 *
 *        <b>network find unused</b>
 *        - <i>Begins a search for an unused Channel and PAN ID. Will automatically
 *             form a network on the first unused Channel and PAN ID it finds.</i>
 *
 *        <b>network find joinable</b>
 *        - <i>Begins a search for a joinable network. Will automatically
 *             attempt to join the first network it finds.</i>
 */
#define EMBER_AF_DOXYGEN_CLI__NETWORK_COMMANDS
/** @} END addtogroup */

#ifndef EMBER_AF_GENERATE_CLI

// Form and join library commands.
static EmberCommandEntry findCommands[] = {
  { "joinable", findJoinableNetworkCommand, "" },
  { "unused", findUnusedPanIdCommand, "" },
  { NULL },
};

EmberCommandEntry networkCommands[] = {
  { "form", networkFormCommand, "usv" },
  { "join", networkJoinCommand, "usvu*" },
  { "rejoin", networkRejoinCommand, "uw" },
  { "leave", networkLeaveCommand, "" },
  { "pjoin", networkPermitJoinCommand, "u" },
  { "broad-pjoin", networkPermitJoinCommand, "u" },
  { "extpanid", networkExtendedPanIdCommand, "b" },
  { "find", NULL, (const char *)findCommands },

  emberCommandEntryAction("id",
                          networkIdCommand,
                          "",
                          "Print the current network IDs"),

  { "change-channel", networkChangeChannelCommand, "u" },
  { "multi-phy-start", networkMultiPhyStartCommand, "uusu*" },
  { "multi-phy-stop", networkMultiPhyStopCommand, "" },

#if defined(EMBER_AF_TC_SWAP_OUT_TEST)
  // Do not document this command.
  { "init", networkInitCommand, "" },
#endif

  { "set", networkSetCommand, "u" },
  { "change-keep-alive-mode", changeKeepAliveModeCommand, "u" },
  { "timeout-option-mask", networkChangeChildTimeoutOptionMaskCommand, "v" },

  { NULL }
};

#endif // EMBER_AF_GENERATE_CLI

uint8_t emAfCliNetworkIndex = EMBER_AF_DEFAULT_NETWORK_INDEX;

#ifdef EMBER_AF_PLUGIN_TEST_HARNESS_Z3
extern EmberEventControl emZigbeeLeaveEvent;
#endif

//------------------------------------------------------------------------------

void initNetworkParams(EmberNetworkParameters *networkParams)
{
  MEMSET(networkParams, 0, sizeof(EmberNetworkParameters));
  emberAfGetFormAndJoinExtendedPanIdCallback(networkParams->extendedPanId);
  networkParams->radioChannel = (uint8_t)emberUnsignedCommandArgument(0);
  networkParams->radioTxPower = (int8_t)emberSignedCommandArgument(1);
  networkParams->panId = (uint16_t)emberUnsignedCommandArgument(2);
  if (emberCommandArgumentCount() > 3) {
    // Map the channel page into the channel, if present.
    // Page bits: 7, 6, 5 channel bits: 4...0
    uint8_t page = (uint8_t)emberUnsignedCommandArgument(3);
    networkParams->radioChannel = (uint8_t)((page << 5) | (networkParams->radioChannel & 0x1F));
  }
}

// network join <channel> <power> <panid>
void networkJoinCommand(void)
{
  EmberStatus status;
  EmberNetworkParameters networkParams = { 0 };
  initNetworkParams(&networkParams);
  status = emberAfJoinNetwork(&networkParams);
  UNUSED_VAR(status);
  emberAfAppPrintln("%p 0x%x", "join", status);
}

// network rejoin <haveCurrentNetworkKey:1>
void networkRejoinCommand(void)
{
  bool haveCurrentNetworkKey = (bool)emberUnsignedCommandArgument(0);
  uint32_t channelMask = (uint32_t)emberUnsignedCommandArgument(1);
  if (channelMask == 0) {
    channelMask = EMBER_ALL_802_15_4_CHANNELS_MASK;
  }
  EmberStatus status = emberFindAndRejoinNetworkWithReason(haveCurrentNetworkKey,
                                                           channelMask,
                                                           EMBER_AF_REJOIN_DUE_TO_CLI_COMMAND);
  emberAfAppPrintln("%p 0x%x", "rejoin", status);
}

void networkRejoinDiffDeviceTypeCommand(void)
{
  bool haveCurrentNetworkKey = (uint8_t)emberUnsignedCommandArgument(0);
  uint32_t channelMask = (uint32_t)emberUnsignedCommandArgument(1);
  uint8_t emberNodeType = (uint8_t)emberUnsignedCommandArgument(2);
  EmberStatus status = emberFindAndRejoinNetworkWithNodeType(haveCurrentNetworkKey,
                                                             channelMask,
                                                             emberNodeType);
  emberAfAppPrintln("%p 0x%x", "rejoinDiffDeviceType", status);
}

// network form <channel> <power> <panid>
void networkFormCommand(void)
{
#ifdef EMBER_AF_HAS_COORDINATOR_NETWORK
  EmberStatus status;
  EmberNetworkParameters networkParams;
  initNetworkParams(&networkParams);
  status = emberAfFormNetwork(&networkParams);
  emberAfAppPrintln("%p 0x%x", "form", status);
  emberAfAppFlush();
#else
  emberAfAppPrintln("only coordinators can form");
#endif
}

// network extpanid <8 BYTES>
void networkExtendedPanIdCommand(void)
{
  emberCopyBigEndianEui64Argument(0, emAfExtendedPanId);
  emberAfSetFormAndJoinExtendedPanIdCallback(emAfExtendedPanId);
  emberAfAppPrint("ext. PAN ID: ");
  emberAfAppDebugExec(emberAfPrintBigEndianEui64(emAfExtendedPanId));
  emberAfAppPrintln("");
}

// network leave
void networkLeaveCommand(void)
{
  EmberStatus status;
  status = emberLeaveNetwork();
#ifdef EMBER_AF_PLUGIN_TEST_HARNESS_Z3
  // Complete the leave immediately without the usual delay.
  emberEventControlSetActive(emZigbeeLeaveEvent);
#endif
  UNUSED_VAR(status);
  emberAfAppPrintln("%p 0x%x", "leave", status);
}

// network pjoin <time>
// network broad-pjoin <time>
void networkPermitJoinCommand(void)
{
  uint8_t duration = (uint8_t)emberUnsignedCommandArgument(0);
  emAfPermitJoin(duration,
                 ('b'
                  == emberStringCommandArgument(-1,
                                                NULL)[0]));  // broadcast permit join?
}

void findJoinableNetworkCommand(void)
{
  EmberStatus status = emberAfStartSearchForJoinableNetwork();
  emberAfCorePrintln("find joinable: 0x%X", status);
}

void findUnusedPanIdCommand(void)
{
  EmberStatus status = emberAfFindUnusedPanIdAndForm();
  emberAfCorePrintln("find unused: 0x%X", status);
}

void networkChangeChannelCommand(void)
{
  uint8_t channel = (uint8_t)emberUnsignedCommandArgument(0);
  EmberStatus status = emberChannelChangeRequest(channel);
  emberAfAppPrintln("Changing to channel %d: 0x%X",
                    channel,
                    status);
}

// TODO: Put gating back in after we have a mechanism in place to do so
// for the generated CLI.
//#if defined(EMBER_AF_TC_SWAP_OUT_TEST)
void networkInitCommand(void)
{
  EmberNetworkInitStruct networkInitStruct = {
    EMBER_AF_CUSTOM_NETWORK_INIT_OPTIONS   // EmberNetworkInitBitmask value
  };
  EmberStatus status = emberNetworkInit(&networkInitStruct);
  emberAfAppPrintln("Network Init returned: 0x%X", status);
}
//#endif

void networkSetCommand(void)
{
  uint8_t index = (uint8_t)emberUnsignedCommandArgument(0);
  if (EMBER_SUPPORTED_NETWORKS <= index) {
    emberAfCorePrintln("invalid network index");
    return;
  }
  emAfCliNetworkIndex = index;
}

void networkIdCommand(void)
{
  EmberEUI64 eui64;
  emberAfGetEui64(eui64);
  emberAfCorePrint("Short ID: 0x%2X, EUI64: ", emberAfGetNodeId());
  emberAfPrintBigEndianEui64(eui64);
  emberAfCorePrintln(", Pan ID: 0x%2X", emberAfGetPanId());
}

void networkMultiPhyStartCommand(void)
{
  EmberStatus status;
  uint8_t page = emberUnsignedCommandArgument(0);
  uint8_t channel = emberUnsignedCommandArgument(1);
  int8_t power = (int8_t)emberSignedCommandArgument(2);
  uint8_t optionsMask = 0;

  if (emberCommandArgumentCount() > 3) {
    optionsMask = (uint8_t) emberUnsignedCommandArgument(3);
  }

  status = emberMultiPhyStart(PHY_INDEX_PRO2PLUS, page, channel, power, optionsMask);

  if (status == EMBER_SUCCESS) {
    emberAfCorePrintln("Started multi-phy interface");
  } else {
    emberAfCorePrintln("Failed to %s %s 0x%X",
                       "start",
                       "multi-phy interface",
                       status);
  }
}

void networkMultiPhyStopCommand(void)
{
  uint8_t status = emberMultiPhyStop(PHY_INDEX_PRO2PLUS);

  if (status == EMBER_SUCCESS) {
    emberAfCorePrintln("Terminated %s 0x%X",
                       "multi-phy interface",
                       status);
  } else {
    emberAfCorePrintln("Failed to %s %s 0x%X",
                       "stop",
                       "multi-phy interface",
                       status);
  }
}

/* ToDo: keep or remove? needed for testing. no ezsp support. */
void changeKeepAliveModeCommand(void)
{
  uint8_t keepAliveMode = emberUnsignedCommandArgument(0);
  if (!emberSetKeepAliveMode(keepAliveMode)) {
    emberAfAppPrintln("Keep alive support enabled.");
  } else {
    emberAfAppPrintln("failed to set keep alive mode.");
  }
}

void networkChangeChildTimeoutOptionMaskCommand(void)
{
  uint16_t mask = (uint16_t)emberUnsignedCommandArgument(0);
  if (!emberSetChildTimeoutOptionMask(mask)) {
    emberAfAppPrintln("successfully set the child timeout option mask.");
  } else {
    emberAfAppPrintln("failed to set the child timeout option mask.");
  }
}
