/***************************************************************************//**
 * @file sl_cli_command_table.c
 * @brief Declarations of relevant command structs for cli framework.
 * @version x.y.z
 *******************************************************************************
 * # License
 * <b>Copyright 2018 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * SPDX-License-Identifier: Zlib
 *
 * The licensor of this software is Silicon Laboratories Inc.
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 ******************************************************************************/

#include <stdlib.h>

#include "sl_cli_config.h"
#include "sl_cli_command.h"
#include "sl_cli_arguments.h"

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 *****************************   TEMPLATED FILE   ******************************
 ******************************************************************************/

/*******************************************************************************
 * Example syntax (.slcc or .slcp) for populating this file:
 *
 *   template_contribution:
 *     - name: cli_command          # Register a command
 *       value:
 *         name: status             # Name of command
 *         handler: status_command  # Function to be called. Must be defined
 *         help: "Prints status"    # Optional help description
 *         shortcuts:               # Optional shorcut list
 *           - name: st
 *         argument:                # Argument list, if apliccable
 *           - type: uint8          # Variable type
 *             help: "Channel"      # Optional description
 *           - type: string
 *             help: "Text"
 *     - name: cli_group            # Register a group
 *       value:
 *         name: shell              # Group name
 *         help: "Shell commands"   # Optional help description
 *         shortcuts:               # Optional shorcuts
 *           - name: sh
 *     - name: cli_command
 *       value:
 *         name: repeat
 *         handler: repeat_cmd
 *         help: "Repeat commands"
 *         shortcuts:
 *           - name: r
 *           - name: rep
 *         group: shell            # Associate command with group
 *         argument:
 *           - type: string
 *             help: "Text"
 *           - type: additional
 *             help: "More text"
 *
 * For subgroups, an optional unique id can be used to allow a particular name to
 * be used more than once. In the following case, from the command line the
 * following commands are available:
 *
 * >  root_1 shell status
 * >  root_2 shell status
 *
 *     - name: cli_group            # Register a group
 *       value:
 *         name: root_1             # Group name
 *
 *     - name: cli_group            # Register a group
 *       value:
 *         name: root_2             # Group name
 *
 *    - name: cli_group             # Register a group
 *       value:
 *         name: shell              # Group name
 *         id: shell_root_1         # Optional unique id for group
 *         group: root_1            # Add group to root_1 group
 *
 *    - name: cli_group             # Register a group
 *       value:
 *         name: shell              # Group name
 *         id: shell_root_2         # Optional unique id for group
 *         group: root_2            # Add group to root_1 group
 *
 *    - name: cli_command           # Register a command
 *       value:
 *         name: status
 *         handler: status_1
 *         group: shell_root_1      # id of subgroup
 *
 *    - name: cli_command           # Register a command
 *       value:
 *         name: status
 *         handler: status_2
 *         group: shell_root_2      # id of subgroup
 *
 ******************************************************************************/

// Provide function declarations
void emAfCliInfoCommand(sl_cli_command_arg_t *arguments);
void printAllLibraryStatus(sl_cli_command_arg_t *arguments);
void emAfCliBsendCommand(sl_cli_command_arg_t *arguments);
void sli_zigbee_cli_send_command(sl_cli_command_arg_t *arguments);
void sli_zigbee_zcl_read_cli_command(sl_cli_command_arg_t *arguments);
void sli_zigbee_zcl_write_cli_command(sl_cli_command_arg_t *arguments);
void resetCommand(sl_cli_command_arg_t *arguments);
void emAfCliRawCommand(sl_cli_command_arg_t *arguments);
void sli_zigbee_cli_send_command(sl_cli_command_arg_t *arguments);
void emAfCliSendUsingMulticastBindingCommand(sl_cli_command_arg_t *arguments);
void emAfCliTimesyncCommand(sl_cli_command_arg_t *arguments);
void sli_zigbee_cli_config_cca_mode_command(sl_cli_command_arg_t *arguments);
void sli_zigbee_cli_version_command(sl_cli_command_arg_t *arguments);
void endpointPrint(sl_cli_command_arg_t *arguments);
void enableDisableEndpoint(sl_cli_command_arg_t *arguments);
void enableDisableEndpoint(sl_cli_command_arg_t *arguments);
void printEvents(sl_cli_command_arg_t *arguments);
void getSetMfgToken(sl_cli_command_arg_t *arguments);
void getSetMfgToken(sl_cli_command_arg_t *arguments);
void sli_zigbee_debug_print_enable_stack_type_command(sl_cli_command_arg_t *arguments);
void sli_zigbee_debug_print_enable_core_type_command(sl_cli_command_arg_t *arguments);
void sli_zigbee_debug_print_enable_app_type_command(sl_cli_command_arg_t *arguments);
void sli_zigbee_debug_print_enable_zcl_type_command(sl_cli_command_arg_t *arguments);
void sli_zigbee_debug_print_enable_legacy_af_debug_type_command(sl_cli_command_arg_t *arguments);
void emberAfPluginGatewaySupportTimeSyncLocal(sl_cli_command_arg_t *arguments);
void networkFormCommand(sl_cli_command_arg_t *arguments);
void networkJoinCommand(sl_cli_command_arg_t *arguments);
void networkPermitJoinCommand(sl_cli_command_arg_t *arguments);
void networkLeaveCommand(sl_cli_command_arg_t *arguments);
void networkRejoinCommand(sl_cli_command_arg_t *arguments);
void networkRejoinDiffDeviceTypeCommand(sl_cli_command_arg_t *arguments);
void networkExtendedPanIdCommand(sl_cli_command_arg_t *arguments);
void networkCheckPjoinCommand(sl_cli_command_arg_t *arguments);
void networkPermitJoinCommand(sl_cli_command_arg_t *arguments);
void findJoinableNetworkCommand(sl_cli_command_arg_t *arguments);
void findUnusedPanIdCommand(sl_cli_command_arg_t *arguments);
void networkChangeChannelCommand(sl_cli_command_arg_t *arguments);
void networkSetCommand(sl_cli_command_arg_t *arguments);
void networkInitCommand(sl_cli_command_arg_t *arguments);
void networkIdCommand(sl_cli_command_arg_t *arguments);
void changeKeepAliveModeCommand(sl_cli_command_arg_t *arguments);
void networkChangeChildTimeoutOptionMaskCommand(sl_cli_command_arg_t *arguments);
void networkMultiPhyStartCommand(sl_cli_command_arg_t *arguments);
void networkMultiPhyStopCommand(sl_cli_command_arg_t *arguments);
void keysPrintCommand(sl_cli_command_arg_t *arguments);
void keysDeleteCommand(sl_cli_command_arg_t *arguments);
void keysClearCommand(sl_cli_command_arg_t *arguments);
void optionBindingTablePrintCommand(sl_cli_command_arg_t *arguments);
void optionBindingTableClearCommand(sl_cli_command_arg_t *arguments);
void optionBindingTableSetCommand(sl_cli_command_arg_t *arguments);
void optionPrintRxCommand(sl_cli_command_arg_t *arguments);
void optionPrintRxCommand(sl_cli_command_arg_t *arguments);
void optionRegisterCommand(sl_cli_command_arg_t *arguments);
void optionDiscoveryTargetCommand(sl_cli_command_arg_t *arguments);
void optionDiscoverCommand(sl_cli_command_arg_t *arguments);
void optionEndDeviceBindCommand(sl_cli_command_arg_t *arguments);
void optionApsRetryCommand(sl_cli_command_arg_t *arguments);
void optionApsRetryCommand(sl_cli_command_arg_t *arguments);
void optionApsRetryCommand(sl_cli_command_arg_t *arguments);
void optionApsSecurityCommand(sl_cli_command_arg_t *arguments);
void optionApsSecurityCommand(sl_cli_command_arg_t *arguments);
void optionSecurityAllowTrustCenterRejoinUsingWellKnownKey(sl_cli_command_arg_t *arguments);
void optionSecurityAllowTrustCenterRejoinUsingWellKnownKeyTimeout(sl_cli_command_arg_t *arguments);
void optionSecuritySetKeyRequestPolicy(sl_cli_command_arg_t *arguments);
void changeKeyCommand(sl_cli_command_arg_t *arguments);
void changeKeyCommand(sl_cli_command_arg_t *arguments);
void optionLinkCommand(sl_cli_command_arg_t *arguments);
void optionInstallCodeCommand(sl_cli_command_arg_t *arguments);
void zdoRouteRequestCommand(sl_cli_command_arg_t *arguments);
void zdoPowerDescriptorRequestCommand(sl_cli_command_arg_t *arguments);
void zdoMgmtLqiCommand(sl_cli_command_arg_t *arguments);
void zdoMgmtBindCommand(sl_cli_command_arg_t *arguments);
void zdoLeaveRequestCommand(sl_cli_command_arg_t *arguments);
void zdoUnbindGroupCommand(sl_cli_command_arg_t *arguments);
void zdoUnbindUnicastCommand(sl_cli_command_arg_t *arguments);
void zdoActiveEpCommand(sl_cli_command_arg_t *arguments);
void zdoBindCommand(sl_cli_command_arg_t *arguments);
void zdoNodeCommand(sl_cli_command_arg_t *arguments);
void zdoMatchCommand(sl_cli_command_arg_t *arguments);
void zdoSimpleCommand(sl_cli_command_arg_t *arguments);
void zdoIeeeAddressRequestCommand(sl_cli_command_arg_t *arguments);
void zdoNwkAddressRequestCommand(sl_cli_command_arg_t *arguments);
void zdoNetworkUpdateScanCommand(sl_cli_command_arg_t *arguments);
void zdoNetworkUpdateSetCommand(sl_cli_command_arg_t *arguments);
void zdoNetworkUpdateScanCommand(sl_cli_command_arg_t *arguments);
void zdoNetworkUpdateChannelCommand(sl_cli_command_arg_t *arguments);
void zdoNetworkUpdateChannelCommand(sl_cli_command_arg_t *arguments);
void zdoAddClusterCommand(sl_cli_command_arg_t *arguments);
void zdoClearClusterCommand(sl_cli_command_arg_t *arguments);
void zdoAddClusterCommand(sl_cli_command_arg_t *arguments);
void zdoClearClusterCommand(sl_cli_command_arg_t *arguments);
void emberAfPrintAttributeTable(sl_cli_command_arg_t *arguments);
void printTimeCommand(sl_cli_command_arg_t *arguments);

// Command structs. Names are in the format : cli_cmd_{command group name}_{command name}
// In order to support hyphen in command and group name, every occurence of it while
// building struct names will be replaced by "_hyphen_"
static const sl_cli_command_info_t cli_cmd__info = \
  SL_CLI_COMMAND(emAfCliInfoCommand,
                 "Prints information about the network state, clusters, and endpoints.",
                  "",
                 {SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd__libs = \
  SL_CLI_COMMAND(printAllLibraryStatus,
                 "Lists which optional libraries of the stack are implemented on this device.",
                  "",
                 {SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd__bsend = \
  SL_CLI_COMMAND(emAfCliBsendCommand,
                 "Sends a message.",
                  "Source endpoint" SL_CLI_UNIT_SEPARATOR,
                 {SL_CLI_ARG_UINT8, SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd__send = \
  SL_CLI_COMMAND(sli_zigbee_cli_send_command,
                 "Sends a message.",
                  "Destination" SL_CLI_UNIT_SEPARATOR "Source endpoint" SL_CLI_UNIT_SEPARATOR "Destination endpoint" SL_CLI_UNIT_SEPARATOR,
                 {SL_CLI_ARG_UINT16, SL_CLI_ARG_UINT8, SL_CLI_ARG_UINT8, SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd__read = \
  SL_CLI_COMMAND(sli_zigbee_zcl_read_cli_command,
                 "Reads a message.",
                  "Endpoint" SL_CLI_UNIT_SEPARATOR "Cluster ID" SL_CLI_UNIT_SEPARATOR "Attribute ID" SL_CLI_UNIT_SEPARATOR "1 if server direction, 0 if client direction" SL_CLI_UNIT_SEPARATOR,
                 {SL_CLI_ARG_UINT8, SL_CLI_ARG_UINT16, SL_CLI_ARG_UINT16, SL_CLI_ARG_UINT8, SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd__write = \
  SL_CLI_COMMAND(sli_zigbee_zcl_write_cli_command,
                 "Writes a message.",
                  "Endpoint" SL_CLI_UNIT_SEPARATOR "Cluster ID" SL_CLI_UNIT_SEPARATOR "Attribute ID" SL_CLI_UNIT_SEPARATOR "Mask" SL_CLI_UNIT_SEPARATOR "Data type" SL_CLI_UNIT_SEPARATOR "Data byte" SL_CLI_UNIT_SEPARATOR,
                 {SL_CLI_ARG_UINT8, SL_CLI_ARG_UINT16, SL_CLI_ARG_UINT16, SL_CLI_ARG_UINT8, SL_CLI_ARG_UINT8, SL_CLI_ARG_HEX, SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd__reset = \
  SL_CLI_COMMAND(resetCommand,
                 "Resets the node.",
                  "",
                 {SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd__raw = \
  SL_CLI_COMMAND(emAfCliRawCommand,
                 "Creates a message by specifying the raw bytes. Use the send command to send the message once it has been created. Ex: raw 0x000F {00 0A 00 11 22 33 44 55} sends a message to cluster 15 (0x000F) of length 8 which includes the ZCL header.",
                  "ClusterId" SL_CLI_UNIT_SEPARATOR "Data" SL_CLI_UNIT_SEPARATOR,
                 {SL_CLI_ARG_UINT16, SL_CLI_ARG_HEX, SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd__send_multicast = \
  SL_CLI_COMMAND(sli_zigbee_cli_send_command,
                 "Sends a pre-buffered multicast message to a given group ID from a given endpoint.",
                  "groupId" SL_CLI_UNIT_SEPARATOR "src-endpoint" SL_CLI_UNIT_SEPARATOR,
                 {SL_CLI_ARG_UINT16, SL_CLI_ARG_UINT8, SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd__send_hyphen_using_hyphen_multicast_hyphen_binding = \
  SL_CLI_COMMAND(emAfCliSendUsingMulticastBindingCommand,
                 "When sending using a binding, specifies whether a multicast binding should be used.",
                  "useMulticastBinding" SL_CLI_UNIT_SEPARATOR,
                 {SL_CLI_ARG_UINT8, SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd__timesync = \
  SL_CLI_COMMAND(emAfCliTimesyncCommand,
                 "Sends a read attr for the time of the device specified. It sets a flag so when it gets the response it writes the time to its own time attr.",
                  "Id" SL_CLI_UNIT_SEPARATOR "srcEndpoint" SL_CLI_UNIT_SEPARATOR "destEndpoint" SL_CLI_UNIT_SEPARATOR,
                 {SL_CLI_ARG_UINT16, SL_CLI_ARG_UINT8, SL_CLI_ARG_UINT8, SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd__config_hyphen_cca_hyphen_mode = \
  SL_CLI_COMMAND(sli_zigbee_cli_config_cca_mode_command,
                 "Set the configured 802.15.4 CCA mode in the radio. See documentation regarding RAIL_IEEE802154_CcaMode_t.",
                  "A RAIL_IEEE802154_CcaMode_t value" SL_CLI_UNIT_SEPARATOR,
                 {SL_CLI_ARG_UINT8, SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd__version = \
  SL_CLI_COMMAND(sli_zigbee_cli_version_command,
                 "Shows the version of the software.",
                  "",
                 {SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_endpoints_print = \
  SL_CLI_COMMAND(endpointPrint,
                 "Print status of endpoints.",
                  "",
                 {SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_endpoints_enable = \
  SL_CLI_COMMAND(enableDisableEndpoint,
                 "Enables the endpoint for ZCL messages.",
                  "Endpoint index" SL_CLI_UNIT_SEPARATOR,
                 {SL_CLI_ARG_UINT8, SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_endpoints_disable = \
  SL_CLI_COMMAND(enableDisableEndpoint,
                 "Disable the endpoint for ZCL messages.",
                  "Endpoint index" SL_CLI_UNIT_SEPARATOR,
                 {SL_CLI_ARG_UINT8, SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd__events = \
  SL_CLI_COMMAND(printEvents,
                 "Print active events.",
                  "",
                 {SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_mfg_hyphen_token_get = \
  SL_CLI_COMMAND(getSetMfgToken,
                 "Print MFG token.",
                  "",
                 {SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_mfg_hyphen_token_set = \
  SL_CLI_COMMAND(getSetMfgToken,
                 "Set MFG token, CANNOT BE UNSET BY CLI.",
                  "",
                 {SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_enable_type_stack = \
  SL_CLI_COMMAND(sli_zigbee_debug_print_enable_stack_type_command,
                 "Enable/disable debug `stack` print type.",
                  "Enable/disable" SL_CLI_UNIT_SEPARATOR,
                 {SL_CLI_ARG_UINT8, SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_enable_type_core = \
  SL_CLI_COMMAND(sli_zigbee_debug_print_enable_core_type_command,
                 "Enable/disable debug `core` print type.",
                  "Enable/disable" SL_CLI_UNIT_SEPARATOR,
                 {SL_CLI_ARG_UINT8, SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_enable_type_app = \
  SL_CLI_COMMAND(sli_zigbee_debug_print_enable_app_type_command,
                 "Enable/disable debug `app` print type.",
                  "Enable/disable" SL_CLI_UNIT_SEPARATOR,
                 {SL_CLI_ARG_UINT8, SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_enable_type_zcl = \
  SL_CLI_COMMAND(sli_zigbee_debug_print_enable_zcl_type_command,
                 "Enable/disable debug `zcl` print type.",
                  "Enable/disable" SL_CLI_UNIT_SEPARATOR,
                 {SL_CLI_ARG_UINT8, SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_enable_type_legacy_af_debug = \
  SL_CLI_COMMAND(sli_zigbee_debug_print_enable_legacy_af_debug_type_command,
                 "Enable/disable debug `legacy app framework debug` print type.",
                  "Enable/disable" SL_CLI_UNIT_SEPARATOR,
                 {SL_CLI_ARG_UINT8, SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_gateway_time_hyphen_sync_hyphen_local = \
  SL_CLI_COMMAND(emberAfPluginGatewaySupportTimeSyncLocal,
                 "",
                  "",
                 {SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_network_form = \
  SL_CLI_COMMAND(networkFormCommand,
                 "Forms a network on a given channel, with a given TX Power and PAN ID.",
                  "The channel on which to form the network" SL_CLI_UNIT_SEPARATOR "One-byte signed value indicating the TX Power that the radio should be set to" SL_CLI_UNIT_SEPARATOR "The PAN ID on which to form the network" SL_CLI_UNIT_SEPARATOR,
                 {SL_CLI_ARG_UINT8, SL_CLI_ARG_INT8, SL_CLI_ARG_UINT16, SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_network_join = \
  SL_CLI_COMMAND(networkJoinCommand,
                 "Joins a network on a given channel, with a given TX Power and PAN ID.",
                  "The channel on which to join the network" SL_CLI_UNIT_SEPARATOR "One-byte signed value indicating the TX Power that the radio should be set to" SL_CLI_UNIT_SEPARATOR "The PAN ID on which to join the network" SL_CLI_UNIT_SEPARATOR,
                 {SL_CLI_ARG_UINT8, SL_CLI_ARG_INT8, SL_CLI_ARG_UINT16, SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_network_pjoin = \
  SL_CLI_COMMAND(networkPermitJoinCommand,
                 "Turns permit joining on for the amount of time indicated.",
                  "A single byte indicating how long the device should have permit joining turn on for. A value of 0xff turns permit join indefinitely." SL_CLI_UNIT_SEPARATOR,
                 {SL_CLI_ARG_UINT8, SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_network_leave = \
  SL_CLI_COMMAND(networkLeaveCommand,
                 "Leaves a network.",
                  "",
                 {SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_network_rejoin = \
  SL_CLI_COMMAND(networkRejoinCommand,
                 "ReJoins a network.",
                  "Boolean network key availability" SL_CLI_UNIT_SEPARATOR "Channel mask" SL_CLI_UNIT_SEPARATOR,
                 {SL_CLI_ARG_UINT8, SL_CLI_ARG_INT32, SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_network_rejoin_hyphen_diff_hyphen_device_hyphen_type = \
  SL_CLI_COMMAND(networkRejoinDiffDeviceTypeCommand,
                 "Rejoins an existing network in a secure or insecure manner with a different device type.",
                  "Boolean network key availability" SL_CLI_UNIT_SEPARATOR "Channel mask" SL_CLI_UNIT_SEPARATOR "An enumeration indicating the device type to rejoin as.The stack only accepts EMBER_END_DEVICE and EMBER_SLEEPY_END_DEVICE." SL_CLI_UNIT_SEPARATOR,
                 {SL_CLI_ARG_UINT8, SL_CLI_ARG_INT32, SL_CLI_ARG_UINT8, SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_network_extpanid = \
  SL_CLI_COMMAND(networkExtendedPanIdCommand,
                 "Writes the extended pan ID for the device.",
                  "extpanid" SL_CLI_UNIT_SEPARATOR,
                 {SL_CLI_ARG_HEX, SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_network_isopen = \
  SL_CLI_COMMAND(networkCheckPjoinCommand,
                 "Checks network pjoin status.",
                  "",
                 {SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_network_broad_hyphen_pjoin = \
  SL_CLI_COMMAND(networkPermitJoinCommand,
                 "Permits joining on the network for a given number of seconds AND broadcasts a ZDO Mgmt Permit Joining request to all routers.",
                  "A single byte indicating how long the device should have permit joining turned on for. A value of 0xff turns on permit join indefinitely." SL_CLI_UNIT_SEPARATOR,
                 {SL_CLI_ARG_UINT8, SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_find_joinable = \
  SL_CLI_COMMAND(findJoinableNetworkCommand,
                 "findJoinableNetwork",
                  "",
                 {SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_find_unused = \
  SL_CLI_COMMAND(findUnusedPanIdCommand,
                 "findUnusedPanId",
                  "",
                 {SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_network_change_hyphen_channel = \
  SL_CLI_COMMAND(networkChangeChannelCommand,
                 "Attempts to change device over to a different channel given in the channel argument.",
                  "The channel to change to" SL_CLI_UNIT_SEPARATOR,
                 {SL_CLI_ARG_UINT8, SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_network_set = \
  SL_CLI_COMMAND(networkSetCommand,
                 "Sets the network index used by all future CLI commands.  Before executing a CLI command, the framework switches to this network.  After the command finishes executing, the framework switches back to the previous network.  The CLI uses the same network index until the device resets or it is changed through this command.",
                  "index" SL_CLI_UNIT_SEPARATOR,
                 {SL_CLI_ARG_UINT8, SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_network_init = \
  SL_CLI_COMMAND(networkInitCommand,
                 "Initializes a network; this is a test command used for tc-swap-out testing.",
                  "",
                 {SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_network_id = \
  SL_CLI_COMMAND(networkIdCommand,
                 "Prints the current Node ID, EUI64, and Pan ID.",
                  "",
                 {SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_network_change_hyphen_keep_hyphen_alive_hyphen_mode = \
  SL_CLI_COMMAND(changeKeepAliveModeCommand,
                 "Switches between different keep alive modes supported by a router.",
                  "Keep alive mode" SL_CLI_UNIT_SEPARATOR,
                 {SL_CLI_ARG_UINT8, SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_network_timeout_hyphen_option_hyphen_mask = \
  SL_CLI_COMMAND(networkChangeChildTimeoutOptionMaskCommand,
                 "Attempts to change the child timeout option mask to filter out undesirable values (e.g. no more than 3 days).",
                  "timeout option mask" SL_CLI_UNIT_SEPARATOR,
                 {SL_CLI_ARG_UINT16, SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_network_multi_hyphen_phy_hyphen_start = \
  SL_CLI_COMMAND(networkMultiPhyStartCommand,
                 "Used to start multi-PHY interface other than native and form the network. The stack uses same PanId as native radio network.",
                  "page" SL_CLI_UNIT_SEPARATOR "channel" SL_CLI_UNIT_SEPARATOR "power" SL_CLI_UNIT_SEPARATOR,
                 {SL_CLI_ARG_UINT8, SL_CLI_ARG_UINT8, SL_CLI_ARG_INT8, SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_network_multi_hyphen_phy_hyphen_stop = \
  SL_CLI_COMMAND(networkMultiPhyStopCommand,
                 "Terminates the multi-PHY interface",
                  "",
                 {SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_keys_print = \
  SL_CLI_COMMAND(keysPrintCommand,
                 "Prints all security keys.",
                  "",
                 {SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_keys_delete = \
  SL_CLI_COMMAND(keysDeleteCommand,
                 "Delete the specified link key index.",
                  "index" SL_CLI_UNIT_SEPARATOR,
                 {SL_CLI_ARG_UINT8, SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_keys_clear = \
  SL_CLI_COMMAND(keysClearCommand,
                 "Clears all security keys.",
                  "",
                 {SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_binding_hyphen_table_print = \
  SL_CLI_COMMAND(optionBindingTablePrintCommand,
                 "Prints the binding table to the command line.",
                  "",
                 {SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_binding_hyphen_table_clear = \
  SL_CLI_COMMAND(optionBindingTableClearCommand,
                 "Clears the binding table.",
                  "",
                 {SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_binding_hyphen_table_set = \
  SL_CLI_COMMAND(optionBindingTableSetCommand,
                 "Sets the binding table",
                  "index" SL_CLI_UNIT_SEPARATOR "cluster" SL_CLI_UNIT_SEPARATOR "localEp" SL_CLI_UNIT_SEPARATOR "remoteEp" SL_CLI_UNIT_SEPARATOR "eui64" SL_CLI_UNIT_SEPARATOR,
                 {SL_CLI_ARG_UINT8, SL_CLI_ARG_UINT16, SL_CLI_ARG_UINT8, SL_CLI_ARG_UINT8, SL_CLI_ARG_HEX, SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_print_hyphen_rx_hyphen_msgs_enable = \
  SL_CLI_COMMAND(optionPrintRxCommand,
                 "Enables printing of Rx messages.",
                  "",
                 {SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_print_hyphen_rx_hyphen_msgs_disable = \
  SL_CLI_COMMAND(optionPrintRxCommand,
                 "Disables printing of Rx messages.",
                  "",
                 {SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_option_register = \
  SL_CLI_COMMAND(optionRegisterCommand,
                 "",
                  "",
                 {SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_option_target = \
  SL_CLI_COMMAND(optionDiscoveryTargetCommand,
                 "",
                  "targetId" SL_CLI_UNIT_SEPARATOR,
                 {SL_CLI_ARG_UINT16, SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_option_disc = \
  SL_CLI_COMMAND(optionDiscoverCommand,
                 "",
                  "profileId" SL_CLI_UNIT_SEPARATOR "clusterId" SL_CLI_UNIT_SEPARATOR,
                 {SL_CLI_ARG_UINT16, SL_CLI_ARG_UINT16, SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_option_edb = \
  SL_CLI_COMMAND(optionEndDeviceBindCommand,
                 "",
                  "endpointId" SL_CLI_UNIT_SEPARATOR,
                 {SL_CLI_ARG_UINT8, SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_apsretry_on = \
  SL_CLI_COMMAND(optionApsRetryCommand,
                 "",
                  "",
                 {SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_apsretry_off = \
  SL_CLI_COMMAND(optionApsRetryCommand,
                 "",
                  "",
                 {SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_apsretry_default = \
  SL_CLI_COMMAND(optionApsRetryCommand,
                 "",
                  "",
                 {SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_aps_on = \
  SL_CLI_COMMAND(optionApsSecurityCommand,
                 "",
                  "",
                 {SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_aps_off = \
  SL_CLI_COMMAND(optionApsSecurityCommand,
                 "",
                  "",
                 {SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_security_set_hyphen_allow_hyphen_trust_hyphen_center_hyphen_rejoin_hyphen_using_hyphen_well_hyphen_known_hyphen_key = \
  SL_CLI_COMMAND(optionSecurityAllowTrustCenterRejoinUsingWellKnownKey,
                 "",
                  "Bool: allow trust center rejoin using well-known key" SL_CLI_UNIT_SEPARATOR,
                 {SL_CLI_ARG_UINT8, SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_security_set_hyphen_allow_hyphen_trust_hyphen_center_hyphen_rejoin_hyphen_using_hyphen_well_hyphen_known_hyphen_key_hyphen_timeout = \
  SL_CLI_COMMAND(optionSecurityAllowTrustCenterRejoinUsingWellKnownKeyTimeout,
                 "",
                  "timeout" SL_CLI_UNIT_SEPARATOR,
                 {SL_CLI_ARG_UINT16, SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_security_set_hyphen_key_hyphen_request_hyphen_policy = \
  SL_CLI_COMMAND(optionSecuritySetKeyRequestPolicy,
                 "",
                  "TC link key request policy" SL_CLI_UNIT_SEPARATOR "App link key request policy" SL_CLI_UNIT_SEPARATOR,
                 {SL_CLI_ARG_UINT8, SL_CLI_ARG_UINT8, SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_changekey_link = \
  SL_CLI_COMMAND(changeKeyCommand,
                 "",
                  "key" SL_CLI_UNIT_SEPARATOR,
                 {SL_CLI_ARG_HEX, SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_changekey_network = \
  SL_CLI_COMMAND(changeKeyCommand,
                 "",
                  "key" SL_CLI_UNIT_SEPARATOR,
                 {SL_CLI_ARG_HEX, SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_option_link = \
  SL_CLI_COMMAND(optionLinkCommand,
                 "",
                  "index" SL_CLI_UNIT_SEPARATOR "eui64" SL_CLI_UNIT_SEPARATOR "key" SL_CLI_UNIT_SEPARATOR,
                 {SL_CLI_ARG_UINT8, SL_CLI_ARG_HEX, SL_CLI_ARG_HEX, SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_option_install_hyphen_code = \
  SL_CLI_COMMAND(optionInstallCodeCommand,
                 "",
                  "index" SL_CLI_UNIT_SEPARATOR "eui64" SL_CLI_UNIT_SEPARATOR "installCode" SL_CLI_UNIT_SEPARATOR,
                 {SL_CLI_ARG_UINT8, SL_CLI_ARG_HEX, SL_CLI_ARG_HEX, SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_zdo_route = \
  SL_CLI_COMMAND(zdoRouteRequestCommand,
                 "Sends a ZDO route request command to the target.",
                  "target" SL_CLI_UNIT_SEPARATOR "index" SL_CLI_UNIT_SEPARATOR,
                 {SL_CLI_ARG_UINT16, SL_CLI_ARG_UINT8, SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_zdo_power = \
  SL_CLI_COMMAND(zdoPowerDescriptorRequestCommand,
                 "Sends a ZDO Power Descriptor Request to the target device.",
                  "target" SL_CLI_UNIT_SEPARATOR,
                 {SL_CLI_ARG_UINT16, SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_zdo_mgmt_hyphen_lqi = \
  SL_CLI_COMMAND(zdoMgmtLqiCommand,
                 "Sends a ZDO MGMT-LQI (LQI Table) Request to the target device.",
                  "target" SL_CLI_UNIT_SEPARATOR "startIndex" SL_CLI_UNIT_SEPARATOR,
                 {SL_CLI_ARG_UINT16, SL_CLI_ARG_UINT8, SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_zdo_mgmt_hyphen_bind = \
  SL_CLI_COMMAND(zdoMgmtBindCommand,
                 "Sends a ZDO MGMT-Bind (Binding Table) Request to the target device.",
                  "target" SL_CLI_UNIT_SEPARATOR "startIndex" SL_CLI_UNIT_SEPARATOR,
                 {SL_CLI_ARG_UINT16, SL_CLI_ARG_UINT8, SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_zdo_leave = \
  SL_CLI_COMMAND(zdoLeaveRequestCommand,
                 "Sends a ZDO Management Leave command to the target device.",
                  "target" SL_CLI_UNIT_SEPARATOR "removeChildren" SL_CLI_UNIT_SEPARATOR "rejoin" SL_CLI_UNIT_SEPARATOR,
                 {SL_CLI_ARG_UINT16, SL_CLI_ARG_UINT8, SL_CLI_ARG_UINT8, SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_unbind_group = \
  SL_CLI_COMMAND(zdoUnbindGroupCommand,
                 "Sends an unbind request for a multicast binding to the target device.",
                  "target" SL_CLI_UNIT_SEPARATOR "source eui64" SL_CLI_UNIT_SEPARATOR "source endpoint" SL_CLI_UNIT_SEPARATOR "clusterID" SL_CLI_UNIT_SEPARATOR "groupAddress" SL_CLI_UNIT_SEPARATOR,
                 {SL_CLI_ARG_UINT16, SL_CLI_ARG_HEX, SL_CLI_ARG_UINT8, SL_CLI_ARG_UINT16, SL_CLI_ARG_UINT16, SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_unbind_unicast = \
  SL_CLI_COMMAND(zdoUnbindUnicastCommand,
                 "Sends an unbind request for a unicast binding to the target device.",
                  "target" SL_CLI_UNIT_SEPARATOR "source eui64" SL_CLI_UNIT_SEPARATOR "source endpoint" SL_CLI_UNIT_SEPARATOR "clusterID" SL_CLI_UNIT_SEPARATOR "destinationEUI64" SL_CLI_UNIT_SEPARATOR "destination endpoint" SL_CLI_UNIT_SEPARATOR,
                 {SL_CLI_ARG_UINT16, SL_CLI_ARG_HEX, SL_CLI_ARG_UINT8, SL_CLI_ARG_UINT16, SL_CLI_ARG_HEX, SL_CLI_ARG_UINT8, SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_zdo_active = \
  SL_CLI_COMMAND(zdoActiveEpCommand,
                 "Sends an active endpoint request to the device with the given short ID.",
                  "nodeId" SL_CLI_UNIT_SEPARATOR,
                 {SL_CLI_ARG_UINT16, SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_zdo_bind = \
  SL_CLI_COMMAND(zdoBindCommand,
                 "Sends a ZDO Bind command to a device specified in the command arguments.",
                  "Destination" SL_CLI_UNIT_SEPARATOR "Source endpoint" SL_CLI_UNIT_SEPARATOR "Destination endpoint" SL_CLI_UNIT_SEPARATOR "clusterID" SL_CLI_UNIT_SEPARATOR "remoteEUI64" SL_CLI_UNIT_SEPARATOR "destEUI64" SL_CLI_UNIT_SEPARATOR,
                 {SL_CLI_ARG_UINT16, SL_CLI_ARG_UINT8, SL_CLI_ARG_UINT8, SL_CLI_ARG_UINT16, SL_CLI_ARG_HEX, SL_CLI_ARG_HEX, SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_zdo_node = \
  SL_CLI_COMMAND(zdoNodeCommand,
                 "Sends a node descriptor request to a given target device.",
                  "target" SL_CLI_UNIT_SEPARATOR,
                 {SL_CLI_ARG_UINT16, SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_zdo_match = \
  SL_CLI_COMMAND(zdoMatchCommand,
                 "Sends a matchDescriptorsRequest to the given destination with the given profile.",
                  "nodeId" SL_CLI_UNIT_SEPARATOR "profile" SL_CLI_UNIT_SEPARATOR,
                 {SL_CLI_ARG_UINT16, SL_CLI_ARG_UINT16, SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_zdo_simple = \
  SL_CLI_COMMAND(zdoSimpleCommand,
                 "Sends a simple descriptor request for the short address and endpoint specified.",
                  "dest" SL_CLI_UNIT_SEPARATOR "targetEndpoint" SL_CLI_UNIT_SEPARATOR,
                 {SL_CLI_ARG_UINT16, SL_CLI_ARG_UINT8, SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_zdo_ieee = \
  SL_CLI_COMMAND(zdoIeeeAddressRequestCommand,
                 "Requests an IEEE address based on a given node ID.",
                  "nodeId" SL_CLI_UNIT_SEPARATOR,
                 {SL_CLI_ARG_UINT16, SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_zdo_nwk = \
  SL_CLI_COMMAND(zdoNwkAddressRequestCommand,
                 "Sends a network address request for the given IEEE address.",
                  "ieee" SL_CLI_UNIT_SEPARATOR,
                 {SL_CLI_ARG_HEX, SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_nwk_hyphen_upd_scan_hyphen_chan_hyphen_mask = \
  SL_CLI_COMMAND(zdoNetworkUpdateScanCommand,
                 "Performs an energy scan on given channel mask.",
                  "targetNodeId" SL_CLI_UNIT_SEPARATOR "scanDuration" SL_CLI_UNIT_SEPARATOR "scanCount" SL_CLI_UNIT_SEPARATOR "channelMask" SL_CLI_UNIT_SEPARATOR,
                 {SL_CLI_ARG_UINT16, SL_CLI_ARG_UINT8, SL_CLI_ARG_UINT16, SL_CLI_ARG_UINT32, SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_nwk_hyphen_upd_set = \
  SL_CLI_COMMAND(zdoNetworkUpdateSetCommand,
                 "Broadcasts the ID of the new network manager and active channels.",
                  "nwkMgeId" SL_CLI_UNIT_SEPARATOR "channelMask" SL_CLI_UNIT_SEPARATOR,
                 {SL_CLI_ARG_UINT16, SL_CLI_ARG_UINT32, SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_nwk_hyphen_upd_scan = \
  SL_CLI_COMMAND(zdoNetworkUpdateScanCommand,
                 "Performs an energy scan.",
                  "targetNodeId" SL_CLI_UNIT_SEPARATOR "scanDuration" SL_CLI_UNIT_SEPARATOR "scanCount" SL_CLI_UNIT_SEPARATOR,
                 {SL_CLI_ARG_UINT16, SL_CLI_ARG_UINT8, SL_CLI_ARG_UINT16, SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_nwk_hyphen_upd_chanPg = \
  SL_CLI_COMMAND(zdoNetworkUpdateChannelCommand,
                 "Sends an update channel page req. The stack sends nwk enhanced update req for non-zero page.",
                  "channel" SL_CLI_UNIT_SEPARATOR "page" SL_CLI_UNIT_SEPARATOR,
                 {SL_CLI_ARG_UINT8, SL_CLI_ARG_UINT8, SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_nwk_hyphen_upd_chan = \
  SL_CLI_COMMAND(zdoNetworkUpdateChannelCommand,
                 "Sends an update channel request.",
                  "channel" SL_CLI_UNIT_SEPARATOR,
                 {SL_CLI_ARG_UINT8, SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_out_hyphen_cl_hyphen_list_add = \
  SL_CLI_COMMAND(zdoAddClusterCommand,
                 "Adds clusters to the known client (out) clusters on this device.",
                  "clusterId" SL_CLI_UNIT_SEPARATOR,
                 {SL_CLI_ARG_UINT16, SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_out_hyphen_cl_hyphen_list_clear = \
  SL_CLI_COMMAND(zdoClearClusterCommand,
                 "Clears the ZDO list of client (out) clusters.",
                  "",
                 {SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_in_hyphen_cl_hyphen_list_add = \
  SL_CLI_COMMAND(zdoAddClusterCommand,
                 "Adds clusters to the known client (in) clusters on this device.",
                  "clusterId" SL_CLI_UNIT_SEPARATOR,
                 {SL_CLI_ARG_UINT16, SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_in_hyphen_cl_hyphen_list_clear = \
  SL_CLI_COMMAND(zdoClearClusterCommand,
                 "Clears the ZDO list of client (in) clusters.",
                  "",
                 {SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_print_attr = \
  SL_CLI_COMMAND(emberAfPrintAttributeTable,
                 "Prints attribute.",
                  "",
                 {SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_print_time = \
  SL_CLI_COMMAND(printTimeCommand,
                 "Prints time.",
                  "",
                 {SL_CLI_ARG_END, });


// Create group command tables and structs if cli_groups given
// in template. Group name is suffixed with _group_table for tables
// and group commands are cli_cmd_grp_( group name )
static const sl_cli_command_entry_t endpoints_group_table[] = {
  { "print", &cli_cmd_endpoints_print, false },
  { "enable", &cli_cmd_endpoints_enable, false },
  { "disable", &cli_cmd_endpoints_disable, false },
  { NULL, NULL, false },
};
static const sl_cli_command_info_t cli_cmd_grp_endpoints = \
  SL_CLI_COMMAND_GROUP(endpoints_group_table, "endpoint related commands.");

static const sl_cli_command_entry_t mfg_hyphen_token_group_table[] = {
  { "get", &cli_cmd_mfg_hyphen_token_get, false },
  { "set", &cli_cmd_mfg_hyphen_token_set, false },
  { NULL, NULL, false },
};
static const sl_cli_command_info_t cli_cmd_grp_mfg_hyphen_token = \
  SL_CLI_COMMAND_GROUP(mfg_hyphen_token_group_table, "Get/set MFG token.");

static const sl_cli_command_entry_t aps_group_table[] = {
  { "on", &cli_cmd_aps_on, false },
  { "off", &cli_cmd_aps_off, false },
  { NULL, NULL, false },
};
static const sl_cli_command_info_t cli_cmd_grp_aps = \
  SL_CLI_COMMAND_GROUP(aps_group_table, "Security aps related commands.");

static const sl_cli_command_entry_t security_group_table[] = {
  { "set-allow-trust-center-rejoin-using-well-known-key", &cli_cmd_security_set_hyphen_allow_hyphen_trust_hyphen_center_hyphen_rejoin_hyphen_using_hyphen_well_hyphen_known_hyphen_key, false },
  { "set-allow-trust-center-rejoin-using-well-known-key-timeout", &cli_cmd_security_set_hyphen_allow_hyphen_trust_hyphen_center_hyphen_rejoin_hyphen_using_hyphen_well_hyphen_known_hyphen_key_hyphen_timeout, false },
  { "set-key-request-policy", &cli_cmd_security_set_hyphen_key_hyphen_request_hyphen_policy, false },
  { "mfg-token", &cli_cmd_grp_mfg_hyphen_token, false },
  { "aps", &cli_cmd_grp_aps, false },
  { NULL, NULL, false },
};
static const sl_cli_command_info_t cli_cmd_grp_security = \
  SL_CLI_COMMAND_GROUP(security_group_table, "security related commands");

static const sl_cli_command_entry_t enable_type_group_table[] = {
  { "stack", &cli_cmd_enable_type_stack, false },
  { "core", &cli_cmd_enable_type_core, false },
  { "app", &cli_cmd_enable_type_app, false },
  { "zcl", &cli_cmd_enable_type_zcl, false },
  { "legacy_af_debug", &cli_cmd_enable_type_legacy_af_debug, false },
  { NULL, NULL, false },
};
static const sl_cli_command_info_t cli_cmd_grp_enable_type = \
  SL_CLI_COMMAND_GROUP(enable_type_group_table, "");

static const sl_cli_command_entry_t zigbee_print_group_table[] = {
  { "enable_type", &cli_cmd_grp_enable_type, false },
  { NULL, NULL, false },
};
static const sl_cli_command_info_t cli_cmd_grp_zigbee_print = \
  SL_CLI_COMMAND_GROUP(zigbee_print_group_table, "");

static const sl_cli_command_entry_t gateway_group_table[] = {
  { "time-sync-local", &cli_cmd_gateway_time_hyphen_sync_hyphen_local, false },
  { NULL, NULL, false },
};
static const sl_cli_command_info_t cli_cmd_grp_gateway = \
  SL_CLI_COMMAND_GROUP(gateway_group_table, "gateway related commands");

static const sl_cli_command_entry_t plugin_group_table[] = {
  { "gateway", &cli_cmd_grp_gateway, false },
  { NULL, NULL, false },
};
static const sl_cli_command_info_t cli_cmd_grp_plugin = \
  SL_CLI_COMMAND_GROUP(plugin_group_table, "");

static const sl_cli_command_entry_t find_group_table[] = {
  { "joinable", &cli_cmd_find_joinable, false },
  { "unused", &cli_cmd_find_unused, false },
  { NULL, NULL, false },
};
static const sl_cli_command_info_t cli_cmd_grp_find = \
  SL_CLI_COMMAND_GROUP(find_group_table, "network find commands.");

static const sl_cli_command_entry_t network_group_table[] = {
  { "form", &cli_cmd_network_form, false },
  { "join", &cli_cmd_network_join, false },
  { "pjoin", &cli_cmd_network_pjoin, false },
  { "leave", &cli_cmd_network_leave, false },
  { "rejoin", &cli_cmd_network_rejoin, false },
  { "rejoin-diff-device-type", &cli_cmd_network_rejoin_hyphen_diff_hyphen_device_hyphen_type, false },
  { "extpanid", &cli_cmd_network_extpanid, false },
  { "isopen", &cli_cmd_network_isopen, false },
  { "broad-pjoin", &cli_cmd_network_broad_hyphen_pjoin, false },
  { "change-channel", &cli_cmd_network_change_hyphen_channel, false },
  { "set", &cli_cmd_network_set, false },
  { "init", &cli_cmd_network_init, false },
  { "id", &cli_cmd_network_id, false },
  { "change-keep-alive-mode", &cli_cmd_network_change_hyphen_keep_hyphen_alive_hyphen_mode, false },
  { "timeout-option-mask", &cli_cmd_network_timeout_hyphen_option_hyphen_mask, false },
  { "multi-phy-start", &cli_cmd_network_multi_hyphen_phy_hyphen_start, false },
  { "multi-phy-stop", &cli_cmd_network_multi_hyphen_phy_hyphen_stop, false },
  { "find", &cli_cmd_grp_find, false },
  { NULL, NULL, false },
};
static const sl_cli_command_info_t cli_cmd_grp_network = \
  SL_CLI_COMMAND_GROUP(network_group_table, "Network related commands.");

static const sl_cli_command_entry_t keys_group_table[] = {
  { "print", &cli_cmd_keys_print, false },
  { "delete", &cli_cmd_keys_delete, false },
  { "clear", &cli_cmd_keys_clear, false },
  { NULL, NULL, false },
};
static const sl_cli_command_info_t cli_cmd_grp_keys = \
  SL_CLI_COMMAND_GROUP(keys_group_table, "Security keys related commands.");

static const sl_cli_command_entry_t binding_hyphen_table_group_table[] = {
  { "print", &cli_cmd_binding_hyphen_table_print, false },
  { "clear", &cli_cmd_binding_hyphen_table_clear, false },
  { "set", &cli_cmd_binding_hyphen_table_set, false },
  { NULL, NULL, false },
};
static const sl_cli_command_info_t cli_cmd_grp_binding_hyphen_table = \
  SL_CLI_COMMAND_GROUP(binding_hyphen_table_group_table, "Binding table related commands.");

static const sl_cli_command_entry_t print_hyphen_rx_hyphen_msgs_group_table[] = {
  { "enable", &cli_cmd_print_hyphen_rx_hyphen_msgs_enable, false },
  { "disable", &cli_cmd_print_hyphen_rx_hyphen_msgs_disable, false },
  { NULL, NULL, false },
};
static const sl_cli_command_info_t cli_cmd_grp_print_hyphen_rx_hyphen_msgs = \
  SL_CLI_COMMAND_GROUP(print_hyphen_rx_hyphen_msgs_group_table, "Enables/Disables printing of Rx messages.");

static const sl_cli_command_entry_t apsretry_group_table[] = {
  { "on", &cli_cmd_apsretry_on, false },
  { "off", &cli_cmd_apsretry_off, false },
  { "default", &cli_cmd_apsretry_default, false },
  { NULL, NULL, false },
};
static const sl_cli_command_info_t cli_cmd_grp_apsretry = \
  SL_CLI_COMMAND_GROUP(apsretry_group_table, "APS retry flag force commands");

static const sl_cli_command_entry_t option_group_table[] = {
  { "register", &cli_cmd_option_register, false },
  { "target", &cli_cmd_option_target, false },
  { "disc", &cli_cmd_option_disc, false },
  { "edb", &cli_cmd_option_edb, false },
  { "link", &cli_cmd_option_link, false },
  { "install-code", &cli_cmd_option_install_hyphen_code, false },
  { "binding-table", &cli_cmd_grp_binding_hyphen_table, false },
  { "print-rx-msgs", &cli_cmd_grp_print_hyphen_rx_hyphen_msgs, false },
  { "apsretry", &cli_cmd_grp_apsretry, false },
  { "security", &cli_cmd_grp_security, false },
  { NULL, NULL, false },
};
static const sl_cli_command_info_t cli_cmd_grp_option = \
  SL_CLI_COMMAND_GROUP(option_group_table, "Option related commands.");

static const sl_cli_command_entry_t changekey_group_table[] = {
  { "link", &cli_cmd_changekey_link, false },
  { "network", &cli_cmd_changekey_network, false },
  { NULL, NULL, false },
};
static const sl_cli_command_info_t cli_cmd_grp_changekey = \
  SL_CLI_COMMAND_GROUP(changekey_group_table, "changekey related commands.");

static const sl_cli_command_entry_t unbind_group_table[] = {
  { "group", &cli_cmd_unbind_group, false },
  { "unicast", &cli_cmd_unbind_unicast, false },
  { NULL, NULL, false },
};
static const sl_cli_command_info_t cli_cmd_grp_unbind = \
  SL_CLI_COMMAND_GROUP(unbind_group_table, "Zdo unbind related commands.");

static const sl_cli_command_entry_t nwk_hyphen_upd_group_table[] = {
  { "scan-chan-mask", &cli_cmd_nwk_hyphen_upd_scan_hyphen_chan_hyphen_mask, false },
  { "set", &cli_cmd_nwk_hyphen_upd_set, false },
  { "scan", &cli_cmd_nwk_hyphen_upd_scan, false },
  { "chanPg", &cli_cmd_nwk_hyphen_upd_chanPg, false },
  { "chan", &cli_cmd_nwk_hyphen_upd_chan, false },
  { NULL, NULL, false },
};
static const sl_cli_command_info_t cli_cmd_grp_nwk_hyphen_upd = \
  SL_CLI_COMMAND_GROUP(nwk_hyphen_upd_group_table, "Zdo nwk-upd related commands.");

static const sl_cli_command_entry_t out_hyphen_cl_hyphen_list_group_table[] = {
  { "add", &cli_cmd_out_hyphen_cl_hyphen_list_add, false },
  { "clear", &cli_cmd_out_hyphen_cl_hyphen_list_clear, false },
  { NULL, NULL, false },
};
static const sl_cli_command_info_t cli_cmd_grp_out_hyphen_cl_hyphen_list = \
  SL_CLI_COMMAND_GROUP(out_hyphen_cl_hyphen_list_group_table, "Zdo out-cl-list related commands.");

static const sl_cli_command_entry_t in_hyphen_cl_hyphen_list_group_table[] = {
  { "add", &cli_cmd_in_hyphen_cl_hyphen_list_add, false },
  { "clear", &cli_cmd_in_hyphen_cl_hyphen_list_clear, false },
  { NULL, NULL, false },
};
static const sl_cli_command_info_t cli_cmd_grp_in_hyphen_cl_hyphen_list = \
  SL_CLI_COMMAND_GROUP(in_hyphen_cl_hyphen_list_group_table, "Zdo in-cl-list related commands.");

static const sl_cli_command_entry_t zdo_group_table[] = {
  { "route", &cli_cmd_zdo_route, false },
  { "power", &cli_cmd_zdo_power, false },
  { "mgmt-lqi", &cli_cmd_zdo_mgmt_hyphen_lqi, false },
  { "mgmt-bind", &cli_cmd_zdo_mgmt_hyphen_bind, false },
  { "leave", &cli_cmd_zdo_leave, false },
  { "active", &cli_cmd_zdo_active, false },
  { "bind", &cli_cmd_zdo_bind, false },
  { "node", &cli_cmd_zdo_node, false },
  { "match", &cli_cmd_zdo_match, false },
  { "simple", &cli_cmd_zdo_simple, false },
  { "ieee", &cli_cmd_zdo_ieee, false },
  { "nwk", &cli_cmd_zdo_nwk, false },
  { "unbind", &cli_cmd_grp_unbind, false },
  { "nwk-upd", &cli_cmd_grp_nwk_hyphen_upd, false },
  { "out-cl-list", &cli_cmd_grp_out_hyphen_cl_hyphen_list, false },
  { "in-cl-list", &cli_cmd_grp_in_hyphen_cl_hyphen_list, false },
  { NULL, NULL, false },
};
static const sl_cli_command_info_t cli_cmd_grp_zdo = \
  SL_CLI_COMMAND_GROUP(zdo_group_table, "Zdo related commands.");

static const sl_cli_command_entry_t print_group_table[] = {
  { "attr", &cli_cmd_print_attr, false },
  { "time", &cli_cmd_print_time, false },
  { NULL, NULL, false },
};
static const sl_cli_command_info_t cli_cmd_grp_print = \
  SL_CLI_COMMAND_GROUP(print_group_table, "print related commands.");

// Create root command table
const sl_cli_command_entry_t sl_cli_default_command_table[] = {
  { "info", &cli_cmd__info, false },
  { "libs", &cli_cmd__libs, false },
  { "bsend", &cli_cmd__bsend, false },
  { "send", &cli_cmd__send, false },
  { "read", &cli_cmd__read, false },
  { "write", &cli_cmd__write, false },
  { "reset", &cli_cmd__reset, false },
  { "raw", &cli_cmd__raw, false },
  { "send_multicast", &cli_cmd__send_multicast, false },
  { "send-using-multicast-binding", &cli_cmd__send_hyphen_using_hyphen_multicast_hyphen_binding, false },
  { "timesync", &cli_cmd__timesync, false },
  { "config-cca-mode", &cli_cmd__config_hyphen_cca_hyphen_mode, false },
  { "version", &cli_cmd__version, false },
  { "events", &cli_cmd__events, false },
  { "endpoints", &cli_cmd_grp_endpoints, false },
  { "security", &cli_cmd_grp_security, false },
  { "zigbee_print", &cli_cmd_grp_zigbee_print, false },
  { "plugin", &cli_cmd_grp_plugin, false },
  { "network", &cli_cmd_grp_network, false },
  { "net", &cli_cmd_grp_network, true },
  { "keys", &cli_cmd_grp_keys, false },
  { "option", &cli_cmd_grp_option, false },
  { "opt", &cli_cmd_grp_option, true },
  { "plugin", &cli_cmd_grp_plugin, false },
  { "plug", &cli_cmd_grp_plugin, true },
  { "changekey", &cli_cmd_grp_changekey, false },
  { "changek", &cli_cmd_grp_changekey, true },
  { "zdo", &cli_cmd_grp_zdo, false },
  { "print", &cli_cmd_grp_print, false },
  { NULL, NULL, false },
};


#ifdef __cplusplus
}
#endif
