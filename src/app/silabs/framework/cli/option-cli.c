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
 * @brief CLI commands various options.
 *******************************************************************************
   ******************************************************************************/

// Common include files:
#include "app/framework/util/common.h"

#include "app/framework/security/af-security.h"
#include "app/framework/util/af-main.h"
#include "app/framework/util/service-discovery.h"
#include "app/util/serial/command-interpreter2.h"

// *****************************************************************************
// Forward Declarations
// *****************************************************************************

void optionPrintRxCommand(void);
void optionRegisterCommand(void);
void optionDiscoverCommand(void);
void optionBindingTablePrintCommand(void);
void optionBindingTableClearCommand(void);
void optionEndDeviceBindCommand(void);
void optionApsRetryCommand(void);
void optionApsSecurityCommand(void);
void optionLinkCommand(void);
void optionBindingTableSetCommand(void);
void optionPrintRouteCommand(void);
void optionInstallCodeCommand(void);
void optionDiscoveryTargetCommand(void);

// *****************************************************************************
// Globals
// *****************************************************************************

#ifndef EMBER_AF_GENERATE_CLI

// option print-rx-msgs enable
// option print-rx-msgs disable
static EmberCommandEntry optionPrintRxCommands[] = {
  { "enable", optionPrintRxCommand, "" },
  { "disable", optionPrintRxCommand, "" },
  { NULL }
};

// option binding-table print
// option binding-table clear
// option binding-table set <index> <cluster> <local ep> <remote ep> <EUI>
static EmberCommandEntry optionBindingCommands[] = {
  { "print", optionBindingTablePrintCommand, "" },
  { "clear", optionBindingTableClearCommand, "" },
  { "set", optionBindingTableSetCommand, "uvuub" },
  { NULL }
};

// option security aps [off|on]
static EmberCommandEntry optionApsSecurityCommands[] = {
  { "on", optionApsSecurityCommand, "" },
  { "off", optionApsSecurityCommand, "" },
  { NULL }
};

static EmberCommandEntry apsRetryCommands[] = {
  { "on", optionApsRetryCommand, "" },
  { "off", optionApsRetryCommand, "" },
  { "def", optionApsRetryCommand, "" },
  { NULL }
};

static EmberCommandEntry optionSecurityCommands[] = {
  { "aps", NULL, (const char *)optionApsSecurityCommands },
  { NULL }
};

EmberCommandEntry emAfOptionCommands[] = {
  { "print-rx-msgs", NULL, (const char *)optionPrintRxCommands },
  { "register", optionRegisterCommand, "" },

  emberCommandEntryAction("disc", optionDiscoverCommand, "vv",
                          "Perform a match descriptor request"),
  emberCommandEntryAction("target", optionDiscoveryTargetCommand, "v",
                          "Set the target address of the CLI discovery"),

  { "binding-table", NULL, (const char *)optionBindingCommands },

  { "edb", optionEndDeviceBindCommand, "u" },
  { "security", NULL, (const char *)optionSecurityCommands },
  { "apsretry", NULL, (const char *)apsRetryCommands },

#ifdef DEBUG_PRINT_FOR_ROUTING_TABLE
  { "route", optionPrintRouteCommand, "" },
#endif

#if EMBER_KEY_TABLE_SIZE > 0
  { "link", optionLinkCommand, "ubb" },
  { "install-code", optionInstallCodeCommand, "ubb" },
#endif
  { NULL }
};

#endif // EMBER_AF_GENERATE_CLI

static EmberNodeId discoveryTargetNodeId = EMBER_RX_ON_WHEN_IDLE_BROADCAST_ADDRESS;

// *****************************************************************************
// Functions
// *****************************************************************************

void emAfCliServiceDiscoveryCallback(const EmberAfServiceDiscoveryResult* result)
{
#if defined(EMBER_AF_PRINT_ENABLE) && defined(EMBER_AF_PRINT_APP)
  if (!emberAfHaveDiscoveryResponseStatus(result->status)) {
    // Do nothing
  } else if (result->zdoRequestClusterId == MATCH_DESCRIPTORS_REQUEST) {
    const EmberAfEndpointList* epList = (const EmberAfEndpointList*)result->responseData;
    emberAfAppPrintln("Match %py from 0x%2X, ep %d",
                      "discover",
                      result->matchAddress,
                      epList->list[0]);
  } else if (result->zdoRequestClusterId == NETWORK_ADDRESS_REQUEST) {
    emberAfAppPrintln("NWK Address response: 0x%2X", result->matchAddress);
  } else if (result->zdoRequestClusterId == IEEE_ADDRESS_REQUEST) {
    const uint8_t* eui64ptr = (uint8_t*)(result->responseData);
    emberAfAppPrint("IEEE Address response: ");
    emberAfPrintBigEndianEui64(eui64ptr);
    emberAfAppPrintln("");
  } else {
    // MISRA requires ..else if.. to have a terminating else.
  }

  if (result->status != EMBER_AF_BROADCAST_SERVICE_DISCOVERY_RESPONSE_RECEIVED) {
    emberAfAppPrintln("Service %py done.",
                      "discover");
  }
#endif //defined(EMBER_AF_PRINT_ENABLE) && defined(EMBER_AF_PRINT_APP)
}

// option print-rx-msgs [enable | disable]
void optionPrintRxCommand(void)
{
  if (emberCurrentCommand->name[0] == 'e') {
    emberAfPrintReceivedMessages = true;
  } else {
    emberAfPrintReceivedMessages = false;
  }
  emberAfAppPrintln("%pd print",
                    (emberAfPrintReceivedMessages
                     ? "enable"
                     : "disable"));
}

// option register
void optionRegisterCommand(void)
{
  emberAfRegistrationStartCallback();
}

void optionDiscoveryTargetCommand(void)
{
  discoveryTargetNodeId = (uint16_t)emberUnsignedCommandArgument(0);
}

// option disc <profileId> <clusterId>
void optionDiscoverCommand(void)
{
  EmberAfProfileId profileId = (uint16_t)emberUnsignedCommandArgument(0);
  EmberAfClusterId clusterId = (uint16_t)emberUnsignedCommandArgument(1);
  emberAfFindDevicesByProfileAndCluster(discoveryTargetNodeId,
                                        profileId,
                                        clusterId,
                                        EMBER_AF_SERVER_CLUSTER_DISCOVERY,
                                        emAfCliServiceDiscoveryCallback);
}

// option binding-table print
void optionBindingTablePrintCommand(void)
{
#if defined(EMBER_AF_PRINT_ENABLE) && defined(EMBER_AF_PRINT_APP)
  uint8_t i;
  EmberBindingTableEntry result;

  const char * typeStrings[] = {
    "EMPTY",
    "UNICA",
    "M2ONE",
    "MULTI",
    "?    ",
  };
  uint8_t bindings = 0;

  emberAfAppPrintln("#  type   nwk  loc   rem   clus   node   eui");
  for (i = 0; i < emberAfGetBindingTableSize(); i++) {
    EmberStatus status = emberGetBinding(i, &result);
    if (status == EMBER_SUCCESS) {
      if (result.type > EMBER_MULTICAST_BINDING) {
        result.type = 4;  // last entry in the string list above
      }
      if (result.type != EMBER_UNUSED_BINDING) {
        bindings++;
        emberAfAppPrint("%d: ", i);
        emberAfAppPrint("%p", typeStrings[result.type]);
        emberAfAppPrint("  %d    0x%x  0x%x  0x%2x 0x%2x ",
                        result.networkIndex,
                        result.local,
                        result.remote,
                        result.clusterId,
                        emberGetBindingRemoteNodeId(i));
        emberAfAppDebugExec(emberAfPrintBigEndianEui64(result.identifier));
        emberAfAppPrintln("");
      }
    } else {
      emberAfAppPrintln("0x%x: emberGetBinding Error: %x", status);
      emberAfAppFlush();
    }
    emberAfAppFlush();
  }
  emberAfAppPrintln("%d of %d bindings used",
                    bindings,
                    emberAfGetBindingTableSize());
#endif //defined(EMBER_AF_PRINT_ENABLE) && defined(EMBER_AF_PRINT_APP)
}

// option binding-table clear
void optionBindingTableClearCommand(void)
{
  emberClearBindingTable();
}

// option edb <endpoint>
void optionEndDeviceBindCommand(void)
{
  uint8_t endpoint = (uint8_t)emberUnsignedCommandArgument(0);
  emberAfSendEndDeviceBind(endpoint);
}

void optionApsRetryCommand(void)
{
  if ( emberCurrentCommand->name[1] == 'e' ) {
    emberAfSetRetryOverride(EMBER_AF_RETRY_OVERRIDE_NONE);
  } else if ( emberCurrentCommand->name[1] == 'n' ) {
    emberAfSetRetryOverride(EMBER_AF_RETRY_OVERRIDE_SET);
  } else if ( emberCurrentCommand->name[1] == 'f' ) {
    emberAfSetRetryOverride(EMBER_AF_RETRY_OVERRIDE_UNSET);
  } else {
    // MISRA requires ..else if.. to have a terminating else.
  }
}

// option security aps <off | on>
void optionApsSecurityCommand(void)
{
  emAfTestApsSecurityOverride = (emberCurrentCommand->name[1] == 'f'
                                 ? APS_TEST_SECURITY_DISABLED
                                 : APS_TEST_SECURITY_ENABLED);
}

// option link <index> <eui64 in big endian format> <key in big endian format>
void optionLinkCommand(void)
{
  EmberEUI64 partnerEUI64;
  EmberKeyData newKey;
  EmberStatus status;
  //uint8_t i;
  uint8_t index = (uint8_t)emberUnsignedCommandArgument(0);
  emberCopyBigEndianEui64Argument(1, partnerEUI64);
  emberCopyKeyArgument(2, &newKey);
  status = emberSetKeyTableEntry(index,
                                 partnerEUI64,
                                 true,
                                 &newKey);
  UNUSED_VAR(status);
  emberAfAppDebugExec(emAfPrintStatus("add link key", status));
  emberAfAppPrintln("");
  emberAfCoreFlush();
}

// option install-code <index> <eui64> <install code>
void optionInstallCodeCommand(void)
{
#if (defined(EMBER_AF_HAS_SECURITY_PROFILE_SE_TEST) \
  || defined(EMBER_AF_HAS_SECURITY_PROFILE_SE_FULL) \
  || defined (EMBER_AF_HAS_SECURITY_PROFILE_Z3))

  EmberEUI64 eui64;
  EmberKeyData key;
  EmberStatus status;
  uint8_t code[16 + 2]; // 6, 8, 12, or 16 bytes plus two-byte CRC
  uint8_t length;

  length = emberCopyStringArgument(2, code, sizeof(code), false);

  // Convert the install code to a key.
  status = emAfInstallCodeToKey(code, length, &key);

  if (EMBER_SUCCESS != status) {
    if (EMBER_SECURITY_DATA_INVALID == status) {
      emberAfAppPrintln("ERR: Calculated CRC does not match");
    } else if (EMBER_BAD_ARGUMENT == status) {
      emberAfAppPrintln("ERR: Install Code must be 8, 10, 14, or 18 bytes in "
                        "length");
    } else {
      emberAfAppPrintln("ERR: AES-MMO hash failed: 0x%x", status);
    }
    return;
  }

  emberCopyBigEndianEui64Argument(1, eui64);

  #ifndef EMBER_AF_HAS_SECURITY_PROFILE_Z3
  // Add the key to the link key table.
  status = emberSetKeyTableEntry((uint8_t)emberUnsignedCommandArgument(0),     // index
                                 eui64,
                                 true,                                       // link key
                                 &key);
  emberAfAppDebugExec(emAfPrintStatus("add link key", status));
  emberAfAppPrintln("");
  emberAfAppFlush();
  #else
  // Add the key to the transient key table.
  // This will be used while the DUT joins.
  if (EMBER_SUCCESS == status) {
    status = emberAddTransientLinkKey(eui64, &key);
    emberAfAppDebugExec(emAfPrintStatus("Set joining link key", status));
    emberAfAppPrintln("");
    emberAfAppFlush();
  }
  #endif

#else
  emberAfAppPrintln("This command only supports the Z3 or SE application profile.");
#endif
}

// option binding-table set <index> <cluster> <local ep> <remote ep> <EUI>
void optionBindingTableSetCommand(void)
{
  EmberBindingTableEntry entry;
  uint8_t index = (uint8_t)emberUnsignedCommandArgument(0);
  uint8_t endpoint = (uint8_t)emberUnsignedCommandArgument(2);
  EmberStatus status = emberAfPushEndpointNetworkIndex(endpoint);
  if (status == EMBER_SUCCESS) {
    entry.type = EMBER_UNICAST_BINDING;
    entry.clusterId = (EmberAfClusterId)emberUnsignedCommandArgument(1);
    entry.local = endpoint;
    entry.remote = (uint8_t)emberUnsignedCommandArgument(3);
    emberCopyBigEndianEui64Argument(4, entry.identifier);
    entry.networkIndex = emberGetCurrentNetwork();
    status = emberSetBinding(index, &entry);
    emberAfPopNetworkIndex();
  }
  emberAfAppPrintln("set bind %d: 0x%x", index, status);
}

#ifdef DEBUG_PRINT_FOR_ROUTING_TABLE

void optionPrintRouteCommand(void)
{
  const char * statusText[] = {
    "active",
    "discov",
    "??    ",
    "unused",
  };

  const char * concentratorText[] = {
    "- ",
    "lo",
    "hi",
  };

  const char * routeRecordStateText[] = {
    "none",
    "sent",
    "need",
  };
  uint8_t i;

  emberAfAppPrintln("Routing Table\n-----------------");

  for (i = 0; i < EMBER_ROUTE_TABLE_SIZE; i++) {
    EmberRouteTableEntry entry;
    emberGetRouteTableEntry(i, &entry);
    emberAfAppPrintln("%d: dest:0x%2X next:0x%2X status:%p age:%d conc:%p rr-state:%p",
                      i,
                      entry.destination,
                      entry.nextHop,
                      statusText[entry.status],
                      entry.age,
                      concentratorText[entry.concentratorType],
                      routeRecordStateText[entry.routeRecordState]);
    emberAfAppFlush();
  }
}
#endif //  DEBUG_PRINT_FOR_ROUTING_TABLE
