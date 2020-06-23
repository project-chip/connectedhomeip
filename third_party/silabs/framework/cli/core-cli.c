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
 * @brief Core CLI commands used by all applications regardless of profile.
 *******************************************************************************
   ******************************************************************************/

// Common include files:
#include "app/framework/util/common.h"
#include "app/framework/util/af-main.h"

#include "app/util/serial/command-interpreter2.h"
#include "stack/include/library.h"
#include "app/framework/security/af-security.h"

#if !defined(EZSP_HOST)
  #include "stack/include/cbke-crypto-engine.h"  // emberGetCertificate()
  #include "stack/include/child.h"
#endif

#include "app/framework/cli/core-cli.h"
#include "app/framework/cli/custom-cli.h"
#include "app/framework/cli/network-cli.h"
#include "app/framework/cli/plugin-cli.h"
#include "app/framework/cli/security-cli.h"
#include "app/framework/cli/zdo-cli.h"
#include "app/framework/cli/option-cli.h"
#include "app/framework/plugin/test-harness/test-harness-cli.h"
#if defined(EMBER_AF_PLUGIN_COUNTERS)
  #include "app/framework/plugin/counters/counters-cli.h"
  #include "app/framework/plugin/counters/counters.h"
#endif

#include "app/framework/util/af-event.h"

#include "app/util/common/library.h"

//------------------------------------------------------------------------------

void emberCommandActionHandler(const CommandAction action)
{
  emberAfPushNetworkIndex(emAfCliNetworkIndex);
  (*action)();
  emberAfPopNetworkIndex();

#if defined(EMBER_QA)
  emberSerialPrintfLine(APP_SERIAL, "CLI Finished");
#endif
}

extern EmberCommandEntry cbkeCommands[];

#if !defined(ZA_CLI_FULL) && !defined(EMBER_AF_GENERATE_CLI)
// Define this to satisfy external references.
EmberCommandEntry emberCommandTable[] = { { NULL } };
#endif

#if defined(ZA_CLI_FULL)

void printMfgString(void)
{
  uint8_t mfgString[MFG_STRING_MAX_LENGTH + 1];
  emberAfFormatMfgString(mfgString);

  // Note: We use '%s' here because this is a RAM string. Normally,
  // most strings are literals or constants in flash and use '%p'.
  emberAfAppPrintln("MFG String: %s", mfgString);
}

static void printPacketBuffers(void)
{
  emberAfAppPrintln("Buffs: %d / %d",
                    emAfGetPacketBufferFreeCount(),
                    emAfGetPacketBufferTotalCount());
}

bool printSmartEnergySecurityInfo(void)
{
#ifdef EMBER_AF_HAS_SECURITY_PROFILE_SE
  bool securityGood = true;
  emberAfAppPrint("SE Security Info [");
  {
    // For SE security, print the state of ECC, CBKE, and the programmed Cert.
    EmberCertificateData cert;
    EmberStatus status = emberGetCertificate(&cert);

    // Check the status of the ECC library.
    if ((emberGetLibraryStatus(EMBER_ECC_LIBRARY_ID)
         & EMBER_LIBRARY_PRESENT_MASK) != 0U) {
      emberAfAppPrint("Real163k1ECC ");
    } else {
      emberAfAppPrint("No163k1ECC ");
      securityGood = false;
    }

    // Status of EMBER_LIBRARY_NOT_PRESENT means the CBKE is not present
    // in the image and that no information is known about the certificate.
    if (status == EMBER_LIBRARY_NOT_PRESENT) {
      emberAfAppPrint("No163k1Cbke UnknownCert ");
      securityGood = false;
    } else {
      emberAfAppPrint("Real163k1Cbke ");

      // Status of EMBER_SUCCESS means the Cert is good.
      if (status == EMBER_SUCCESS) {
        emberAfAppPrint("GoodCert");
      }
      // Status of EMBER_ERR_FATAL means the Cert failed.
      else if (status == EMBER_ERR_FATAL) {
        emberAfAppPrint("BadCert");
        securityGood = false;
      } else {
        // MISRA requires ..else if.. to have a terminating else.
      }
    }
    emberAfAppPrintln("]");
  }
  emberAfAppFlush();
  return securityGood;
#else
  return false;
#endif
}

bool printSmartEnergySecurityInfo283k1(void)
{
#ifdef EMBER_AF_HAS_SECURITY_PROFILE_SE
  bool securityGood = true;
  emberAfAppPrint("SE Security 283k1 Info [");
  {
    // For SE security, print the state of ECC, CBKE, and the programmed Cert.
    EmberCertificate283k1Data cert;
    // This is a temporary step until the EZSP frames are added
    // to the EZSP code.
    EmberStatus status = EMBER_LIBRARY_NOT_PRESENT;
    status = emberGetCertificate283k1(&cert);
    // Check the status of the ECC library.
    if ((emberGetLibraryStatus(EMBER_ECC_LIBRARY_283K1_ID)
         & EMBER_LIBRARY_PRESENT_MASK) != 0U) {
      emberAfAppPrint("Real283k1ECC ");
    } else {
      emberAfAppPrint("No283k1ECC ");
      securityGood = false;
    }

    // Status of EMBER_LIBRARY_NOT_PRESENT means the CBKE is not present
    // in the image and that no information is known about the certificate.
    if (status == EMBER_LIBRARY_NOT_PRESENT) {
      emberAfAppPrint("No283k1Cbke UnknownCert");
      securityGood = false;
    } else {
      emberAfAppPrint("Real283k1Cbke ");

      // Status of EMBER_SUCCESS means the Cert is good.
      if (status == EMBER_SUCCESS) {
        emberAfAppPrint("GoodCert");
      }
      // Status of EMBER_ERR_FATAL means the Cert failed.
      else if (status == EMBER_ERR_FATAL) {
        emberAfAppPrint("BadCert");
        securityGood = false;
      } else {
        // MISRA requires ..else if.. to have a terminating else.
      }
    }
    emberAfAppPrintln("]");
  }
  emberAfAppFlush();
  return securityGood;
#else
  return false;
#endif
}

void emAfCliInfoCommand(void)
{
  EmberNodeType nodeTypeResult = 0xFF;
  uint8_t commandLength;
  EmberEUI64 myEui64;
  EmberNetworkParameters networkParams = { 0 };
  uint8_t numPhyInterfaces;

  emberStringCommandArgument(-1, &commandLength);
  printMfgString();
  emberAfAppPrintln("AppBuilder MFG Code: 0x%2X", EMBER_AF_MANUFACTURER_CODE);
  emberAfGetEui64(myEui64);
  emberAfGetNetworkParameters(&nodeTypeResult, &networkParams);
  emberAfAppPrint("node [");
  emberAfAppDebugExec(emberAfPrintBigEndianEui64(myEui64));
  emberAfAppFlush();
  emberAfAppPrintln("] chan [%d] pwr [%d]",
                    networkParams.radioChannel,
                    networkParams.radioTxPower);

  numPhyInterfaces = emberGetPhyInterfaceCount();

  if ((numPhyInterfaces > 1) && (255 != numPhyInterfaces)) {
    EmberMultiPhyRadioParameters multiPhyRadioParams;
    uint8_t i;
    EmberStatus status;

    emberAfAppPrintln("Additional interfaces");
    for (i = 1; i < numPhyInterfaces; ++i) {
      emberAfAppPrint("  %d: ", i);
      status = emberGetRadioParameters(i, &multiPhyRadioParams);
      switch (status) {
        case EMBER_SUCCESS:
          emberAfAppPrintln("page [%d] chan [%d] pwr [%d]",
                            multiPhyRadioParams.radioPage,
                            multiPhyRadioParams.radioChannel,
                            multiPhyRadioParams.radioTxPower);
          break;
        case EMBER_NETWORK_DOWN:
        case EMBER_NOT_JOINED:
          emberAfAppPrintln("not active");
          break;
        default:
          emberAfAppPrintln("error 0x%x", status);
          break;
      }
    }
  }

  emberAfAppPrint("panID [0x%2x] nodeID [0x%2x] ",
                  networkParams.panId,
                  emberAfGetNodeId());
  emberAfAppFlush();
  emberAfAppPrint("xpan [0x");
  emberAfAppDebugExec(emberAfPrintBigEndianEui64(networkParams.extendedPanId));
  emberAfAppPrintln("]");
  emberAfAppFlush();

  #ifndef EZSP_HOST
  emberAfAppPrintln("parentID [0x%2x] parentRssi [%d]", emberGetParentId(), emberGetAvgParentRssi());
  emberAfAppFlush();
  #endif // EZSP_HOST

  emAfCliVersionCommand();
  emberAfAppFlush();

  emberAfAppPrint("nodeType [");
  if (nodeTypeResult != 0xFF) {
    emberAfAppPrint("0x%x", nodeTypeResult);
  } else {
    emberAfAppPrint("unknown");
  }
  emberAfAppPrintln("]");
  emberAfAppFlush();

  emberAfAppPrintln("%p level [%x]", "Security", emberAfGetSecurityLevel());

  printSmartEnergySecurityInfo();
  printSmartEnergySecurityInfo283k1();

  emberAfAppPrint("network state [%x] ", emberNetworkState());
  printPacketBuffers();
  emberAfAppFlush();

  // Print the endpoint information.
  {
    uint8_t i, j;
    emberAfAppPrintln("Ep cnt: %d", emberAfEndpointCount());
    // Loop for each endpoint.
    for (i = 0; i < emberAfEndpointCount(); i++) {
      EmberAfEndpointType *et = emAfEndpoints[i].endpointType;
      emberAfAppPrint("ep %d [endpoint %p, device %p] ",
                      emberAfEndpointFromIndex(i),
                      (emberAfEndpointIndexIsEnabled(i)
                       ? "enabled"
                       : "disabled"),
                      emberAfIsDeviceEnabled(emberAfEndpointFromIndex(i))
                      ? "enabled"
                      : "disabled");
      emberAfAppPrintln("nwk [%d] profile [0x%2x] devId [0x%2x] ver [0x%x]",
                        emberAfNetworkIndexFromEndpointIndex(i),
                        emberAfProfileIdFromIndex(i),
                        emberAfDeviceIdFromIndex(i),
                        emberAfDeviceVersionFromIndex(i));
      // Loop for the clusters within the endpoint.
      for (j = 0; j < et->clusterCount; j++) {
        EmberAfCluster *zc = &(et->cluster[j]);
        emberAfAppPrint("    %p cluster: 0x%2x ",
                        (emberAfClusterIsClient(zc)
                         ? "out(client)"
                         : "in (server)"),
                        zc->clusterId);
        emberAfAppDebugExec(emberAfDecodeAndPrintClusterWithMfgCode(zc->clusterId, emAfGetManufacturerCodeForCluster(zc)));
        emberAfAppPrintln("");
        emberAfAppFlush();
      }
      emberAfAppFlush();
    }
  }

  {
    const char * names[] = {
      EM_AF_GENERATED_NETWORK_STRINGS
    };
    uint8_t i;
    emberAfAppPrintln("Nwk cnt: %d", EMBER_SUPPORTED_NETWORKS);
    for (i = 0; i < EMBER_SUPPORTED_NETWORKS; i++) {
      emberAfPushNetworkIndex(i);
      emberAfAppPrintln("nwk %d [%p]", i, names[i]);
      if (emAfProIsCurrentNetwork()) {
        emberAfAppPrintln("  nodeType [0x%x]",
                          emAfCurrentZigbeeProNetwork->nodeType);
        emberAfAppPrintln("  securityProfile [0x%x]",
                          emAfCurrentZigbeeProNetwork->securityProfile);
      }
      emberAfPopNetworkIndex();
    }
  }
}
#endif

#if defined(ZA_CLI_FULL)

//------------------------------------------------------------------------------
// "debugprint" commands

#ifdef ZA_CLI_FULL

void printOnCommand(void)
{
  uint16_t area = (uint16_t)emberUnsignedCommandArgument(0);
  emberAfPrintOn(area);
}

void printOffCommand(void)
{
  uint16_t area = (uint16_t)emberUnsignedCommandArgument(0);
  emberAfPrintOff(area);
}

#ifndef EMBER_AF_GENERATE_CLI

static const char * debugPrintOnOffCommandArguments[] = {
  "Number of the specified print area.",
  NULL,
};

static EmberCommandEntry debugPrintCommands[] = {
  emberCommandEntryAction("status",
                          emberAfPrintStatus,
                          "",
                          "Print the status of all the debug print areas."),
  emberCommandEntryAction("all_on",
                          emberAfPrintAllOn,
                          "",
                          "Turn all debug print areas on."),

  emberCommandEntryAction("all_off",
                          emberAfPrintAllOff,
                          "",
                          "Turn all debug print areas off."),

  emberCommandEntryActionWithDetails("on",
                                     printOnCommand,
                                     "v",
                                     "Turn on the printing for the specified area.",
                                     debugPrintOnOffCommandArguments),

  emberCommandEntryActionWithDetails("off",
                                     printOffCommand,
                                     "v",
                                     "Turn off the printing for the specified area.",
                                     debugPrintOnOffCommandArguments),
  emberCommandEntryTerminator(),
};
#endif // EMBER_AF_GENERATE_CLI

#endif

//------------------------------------------------------------------------------
// Miscellaneous commands

void helpCommand(void)
{
#if defined(EMBER_AF_PRINT_ENABLE) && defined(EMBER_AF_PRINT_APP)
  EmberCommandEntry *commandFinger = emberCommandTable;
  for (; commandFinger->name != NULL; commandFinger++) {
    emberAfAppPrintln("%p", commandFinger->name);
    emberAfAppFlush();
  }
#endif //defined(EMBER_AF_PRINT_ENABLE) && defined(EMBER_AF_PRINT_APP)
}

void resetCommand(void)
{
  halReboot();
}

void echoCommand(void)
{
  uint8_t echoOn = (uint8_t)emberUnsignedCommandArgument(0);
  if ( echoOn != 0U ) {
    emberCommandInterpreterEchoOn();
  } else {
    emberCommandInterpreterEchoOff();
  }
}

void printEvents(void)
{
  uint8_t i = 0;
  uint32_t nowMS32 = halCommonGetInt32uMillisecondTick();
  while (emAfEvents[i].control != NULL) {
    emberAfCorePrint("%p  : ", emAfEventStrings[i]);
    if (emAfEvents[i].control->status == EMBER_EVENT_INACTIVE) {
      emberAfCorePrintln("inactive");
    } else {
      emberAfCorePrintln("%l ms", emAfEvents[i].control->timeToExecute - nowMS32);
    }
    i++;
  }
}

void emberAfPrintEntropySource(void)
{
  EmberEntropySource entropySource = emberGetTrueRandomEntropySource();
  emberAfCorePrint("True random entropy source: ");
  switch (entropySource) {
    case EMBER_ENTROPY_SOURCE_RADIO:
      emberAfCorePrint("Radio");
      break;
    case EMBER_ENTROPY_SOURCE_MBEDTLS_TRNG:
      emberAfCorePrint("mbed TLS: TRNG");
      break;
    case EMBER_ENTROPY_SOURCE_MBEDTLS:
      emberAfCorePrint("mbed TLS: Other");
      break;
    case EMBER_ENTROPY_SOURCE_ERROR:
    default:
      emberAfCorePrint("Unknown");
      break;
  }
  emberAfCorePrintln(".");
}

//------------------------------------------------------------------------------
// "endpoint" commands

void endpointPrint(void)
{
  uint8_t i;
  for (i = 0; i < emberAfEndpointCount(); i++) {
    emberAfCorePrint("EP %d: %p ",
                     emAfEndpoints[i].endpoint,
                     (emberAfEndpointIndexIsEnabled(i)
                      ? "Enabled"
                      : "Disabled"));
    emAfPrintEzspEndpointFlags(emAfEndpoints[i].endpoint);
    emberAfCorePrintln("");
  }
}

void enableDisableEndpoint(void)
{
  uint8_t endpoint = (uint8_t)emberUnsignedCommandArgument(0);
  bool enable = (emberCurrentCommand->name[0] == 'e'
                 ? true
                 : false);
  if (!emberAfEndpointEnableDisable(endpoint,
                                    enable)) {
    emberAfCorePrintln("Error:  Unknown endpoint.");
  }
}

#ifndef EMBER_AF_GENERATE_CLI
static EmberCommandEntry endpointCommands[] = {
  emberCommandEntryAction("print", endpointPrint, "",
                          "Print the status of all the endpoints."),
  emberCommandEntryAction("enable", enableDisableEndpoint, "u",
                          "Enables the endpoint for processing ZCL messages."),
  emberCommandEntryAction("disable", enableDisableEndpoint, "u",
                          "Disable the endpoint from processing ZCL messages."),

  emberCommandEntryTerminator(),
};

//------------------------------------------------------------------------------
// Commands

static const char * readCommandArguments[] = {
  "Endpoint",
  "Cluster ID",
  "Attribute ID",
  "Server Attribute (bool)",
  NULL,
};

static const char * timeSyncCommandArguments[] = {
  "Node ID",
  "source endpoint",
  "dest endpoint",
  NULL,
};

static const char * writeCommandArguments[] = {
  "Endpoint",
  "Cluster ID",
  "Attribute ID",
  "Server Attribute (bool)",
  "Data Bytes",
  NULL,
};

static const char * sendUsingMulticastBindingCommandArguments[] = {
  "Use Multicast Binding",
  NULL,
};

static const char * bindSendCommandArguments[] = {
  "Source Endpoint",
  NULL,
};

static const char * rawCommandArguments[] = {
  "Cluster ID",
  "Data Bytes",
  NULL,
};

static const char * sendCommandArguments[] = {
  "Node ID",
  "Source Endpoint",
  "Dest Endpoint",
  NULL,
};

//static const char * sendZclIpCommandArguments[] = {
//  "Node EUI64",
//  "Source Endpoint",
//  "Dest Endpoint",
//  NULL,
//};

static const char * sendMulticastCommandArguments[] = {
  "Broadcast address",
  "Group Address",
  "Source Endpoint",
  NULL,
};

EmberCommandEntry emberCommandTable[] = {
#ifdef ZA_CLI_FULL
    #if (defined(ZCL_USING_KEY_ESTABLISHMENT_CLUSTER_CLIENT) \
  && defined(ZCL_USING_KEY_ESTABLISHMENT_CLUSTER_SERVER))
  emberCommandEntrySubMenu("cbke",
                           emberAfPluginKeyEstablishmentCommands,
                           "Commands to initiate CBKE"),
    #endif

    #ifdef EMBER_AF_PRINT_ENABLE
  emberCommandEntrySubMenu("print", printCommands, ""),
  emberCommandEntrySubMenu("debugprint", debugPrintCommands, ""),
    #endif

    #if defined(EMBER_AF_PRINT_ENABLE) && defined(EMBER_AF_PRINT_APP)
  emberCommandEntryAction("version",
                          emAfCliVersionCommand,
                          "",
                          "Print the version number of the ZNet software."),

  LIBRARY_COMMANDS           // Defined in app/util/common/library.h
    #endif

  emberCommandEntrySubMenu("cha",
                           changeKeyCommands,
                           "Commands to change the default NWK or Link Key"),
  emberCommandEntrySubMenu("interpan",
                           interpanCommands,
                           "Commands to send interpan ZCL messages."),
  emberCommandEntrySubMenu("option",
                           emAfOptionCommands,
                           "Commands to change the application options"),

  emberCommandEntryActionWithDetails("read",
                                     emAfCliReadCommand,
                                     "uvvu",
                                     "Construct a read attributes command to be sent.",
                                     readCommandArguments),

  emberCommandEntryActionWithDetails("time",
                                     emAfCliTimesyncCommand,
                                     "vuu",
                                     "Send a read attribute for the current time",
                                     timeSyncCommandArguments),

  emberCommandEntryActionWithDetails("write",
                                     emAfCliWriteCommand,
                                     "uvvuub",
                                     "Construct a write attributes command to send.",
                                     writeCommandArguments),

  emberCommandEntrySubMenu("zcl",
                           zclCommands,
                           "Commands for constructing ZCL cluster commands."),
#endif // ZA_CLI_FULL

  emberCommandEntryActionWithDetails("send-using-multicast-binding",
                                     emAfCliSendUsingMulticastBindingCommand,
                                     "u",
                                     "Use multicast binding when sending using binding",
                                     sendUsingMulticastBindingCommandArguments),

  emberCommandEntryActionWithDetails("bsend",
                                     emAfCliBsendCommand,
                                     "u",
                                     "Send using a binding.",
                                     bindSendCommandArguments),

  emberCommandEntrySubMenu("keys",
                           keysCommands,
                           "Commands to print or set the security keys"),

  emberCommandEntrySubMenu("network",
                           networkCommands,
                           "Commands to form or join a network."),

  emberCommandEntryActionWithDetails("raw",
                                     emAfCliRawCommand,
                                     "vb",
                                     "Create a manually formatted message.",
                                     rawCommandArguments),

  emberCommandEntryActionWithDetails("send",
                                     emAfCliSendCommand,
                                     "vuu",
                                     "Send the previously constructed command via unicast.",
                                     sendCommandArguments),

  emberCommandEntryActionWithDetails("send_multicast",
                                     emAfCliSendCommand,
                                     "vu",
                                     "Send the previously constructed command via multicast.",
                                     sendMulticastCommandArguments),

#ifndef EMBER_AF_HAS_SECURITY_PROFILE_NONE
  emberCommandEntrySubMenu("security", emAfSecurityCommands,
                           "Commands for setting/getting security parameters."),
#endif

#if defined(EMBER_AF_PLUGIN_COUNTERS)
  emberCommandEntryAction("cnt_print", emberAfPluginCountersPrintCommand, "",                      \
                          "Print all stack counters"),                                             \
  emberCommandEntryAction("cnt_clear", emberAfPluginCountersClear, "",                             \
                          "Clear all stack counters"),                                             \
  emberCommandEntryAction("cnt_threshold_print", emberAfPluginCountersPrintThresholdsCommand, "",  \
                          "Print all stack counter thresholds"),                                   \
  emberCommandEntryAction("set_counter_threshold", emberAfPluginCountersSetThresholdCommand, "uu", \
                          "Set Counter Threshold"),                                                \
  emberCommandEntryAction("reset_counter_thresholds", emberAfPluginCountersResetThresholds, "",    \
                          "Reset_counter_thresholds"),
#endif

#if defined(EMBER_AF_PLUGIN_COUNTERS_COUNTERSOTA)
  emberCommandEntryAction("cnt_req", sendCountersRequestCommand, "vu", \
                          "Request stack counters from remote device"),
#endif

  emberCommandEntryAction("help", helpCommand, "",
                          "Print the list of commands."),
  emberCommandEntryAction("reset", resetCommand, "",
                          "Perform a software reset of the device."),
  emberCommandEntryAction("echo", echoCommand, "u",
                          "Turn on/off command interpreter echoing."),
  emberCommandEntryAction("events", printEvents, "",
                          "Print the list of timer events."),
  emberCommandEntrySubMenu("endpoint", endpointCommands,
                           "Commands to manipulate the endpoints."),

#ifndef EMBER_AF_CLI_DISABLE_INFO
  emberCommandEntryAction("info", emAfCliInfoCommand, "", \
                          "Print infomation about the network state, clusters, and endpoints"),
#endif

  EMBER_AF_PLUGIN_COMMANDS
  ZDO_COMMANDS
  CUSTOM_COMMANDS
  TEST_HARNESS_CLI_COMMANDS

  emberCommandEntryTerminator(),
};

#endif // EMBER_AF_GENERATE_CLI

#else
// Stubs
void enableDisableEndpoint(void)
{
}

void endpointPrint(void)
{
}

void printOffCommand(void)
{
}

void printOnCommand(void)
{
}

void printEvents(void)
{
}

void echoCommand(void)
{
}

void emAfCliCountersCommand(void)
{
}

void emAfCliInfoCommand(void)
{
}

void helpCommand(void)
{
}

void resetCommand(void)
{
}
#endif // defined(ZA_CLI_FULL)
