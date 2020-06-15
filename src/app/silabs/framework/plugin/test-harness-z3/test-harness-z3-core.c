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
 * @brief ZigBee 3.0 core test harness functionality
 *******************************************************************************
   ******************************************************************************/

#include "app/framework/include/af.h"
#include "app/framework/security/af-security.h"
#include "app/framework/util/attribute-storage.h" // emberAfResetAttributes()
#include "app/framework/util/af-main.h" // emAfCliVersionCommand()
#include "app/framework/security/security-config.h" // key definitions

#include "app/util/zigbee-framework/zigbee-device-common.h"

#ifdef EMBER_AF_API_ZLL_PROFILE
  #include EMBER_AF_API_ZLL_PROFILE // emberAfZllResetToFactoryNew()
#elif defined(EMBER_SCRIPTED_TEST)
  #include "../zll-commissioning/zll-commissioning.h"
#endif

#ifdef EMBER_AF_API_NETWORK_STEERING
  #include EMBER_AF_API_NETWORK_STEERING
#elif defined(EMBER_SCRIPTED_TEST)
  #include "../network-steering/network-steering.h"
#endif

#include "stack/include/packet-buffer.h"
#include "stack/include/network-formation.h"

#include "test-harness-z3-core.h"
#include "test-harness-z3-nwk.h"
#include "test-harness-z3-zll.h"
#include "test-harness-z3-zdo.h"
#include "test-harness-z3-mac.h"

// -----------------------------------------------------------------------------
// Constants

// -----------------------------------------------------------------------------
// Extern functions

uint8_t emMacHeaderLength(uint8_t *macFrame);
EmberMessageBuffer emPacketHeaderPayload(EmberMessageBuffer commandBuffer);
void emAfZllSetPanId(EmberPanId panId);

// -----------------------------------------------------------------------------
// Globals

EmAfPluginTestHarnessZ3DeviceMode emAfPluginTestHarnessZ3DeviceMode;
EmberEventControl emberAfPluginTestHarnessZ3OpenNetworkEventControl;
uint16_t emAfPluginTestHarnessZ3TouchlinkProfileId = EMBER_ZLL_PROFILE_ID;

// -----------------------------------------------------------------------------
// Util

uint32_t emAfPluginTestHarnessZ3GetSignificantBit(uint8_t commandIndex)
{
  uint32_t mask = emberUnsignedCommandArgument(commandIndex);
  uint32_t significantBit = (1UL << 31);

  while (!(significantBit & mask) && significantBit) {
    significantBit >>= 1;
  }

  return significantBit;
}

// -----------------------------------------------------------------------------
// State

#define PRINTING_MASK_BEACONS BIT(0)
#define PRINTING_MASK_ZDO     BIT(1)
#define PRINTING_MASK_NWK     BIT(2)
#define PRINTING_MASK_APS     BIT(3)
#define PRINTING_MASK_ZCL     BIT(4)
#define PRINTING_MASK_RAW     BIT(5)
static uint8_t printingMask = PRINTING_MASK_ZCL; // always print zcl commands

// -----------------------------------------------------------------------------
// Stack Callbacks

EmberPacketAction emberAfIncomingPacketFilterCallback(EmberZigbeePacketType packetType,
                                                      uint8_t* packetData,
                                                      uint8_t* size_p,
                                                      void *data)
{
  #if !defined(ESZP_HOST)
  EmberPacketAction act = EMBER_ACCEPT_PACKET;
  uint8_t commandId, packetLength;
  commandId = packetData[0];
  packetLength = *size_p;

  switch (packetType) {
    case EMBER_ZIGBEE_PACKET_TYPE_RAW_MAC: {
      if (printingMask & PRINTING_MASK_RAW) {
        emberAfCorePrint("raw mac:rx %d bytes [", packetLength);
        emberAfCorePrintBuffer(packetData, packetLength, true);
        emberAfCorePrintln("]");
      }
      break;
    }
    case EMBER_ZIGBEE_PACKET_TYPE_BEACON: {
      if (printingMask & PRINTING_MASK_BEACONS) {
        uint16_t panId = *(uint16_t*) data;
        emberAfCorePrint("beacon:rx 0x%2X, AP 0x%p, EP ",
                         panId,
                         ((packetData[BEACON_HEADER_SIZE + 1] & BEACON_ASSOCIATION_PERMIT_BIT)
                          ? "1"
                          : "0"));
        emberAfCorePrintBuffer(packetData + BEACON_HEADER_SIZE + BEACON_EXTENDED_PAN_ID_INDEX,
                               EXTENDED_PAN_ID_SIZE,
                               true); // spaces?
        emberAfCorePrintln("");

        // Print out the full payload except beacon header, for the benefit of the beanshell tests
        emberAfCorePrint("payload[");
        emberAfCorePrintBuffer(packetData + BEACON_HEADER_SIZE,
                               packetLength - BEACON_HEADER_SIZE,
                               true); // spaces?
        emberAfCorePrintln("]");
      }
      break;
    }
    case EMBER_ZIGBEE_PACKET_TYPE_MAC_COMMAND: {
      if (printingMask & PRINTING_MASK_BEACONS) {
        if (commandId == BEACON_REQUEST_COMMAND) {
          emberAfCorePrintln("beacon-req:rx");
        }
      }
      break;
    }
    case EMBER_ZIGBEE_PACKET_TYPE_NWK_COMMAND: {
      if (printingMask & PRINTING_MASK_NWK) {
        emberAfCorePrint("nwk:rx seq AC sec 28 cmd %X payload[",
                         commandId);
        emberAfCorePrintBuffer(packetData + 1, packetLength - 1, true); // spaces?
        emberAfCorePrintln("]");
      }
      if (commandId == NWK_LEAVE_COMMAND
          && emAfPluginTestHarnessZ3IgnoreLeaveCommands) {
        // Ignore the leave by turning off the request bit in the options.
        uint8_t newOptions = packetData[1] & ~BIT(6);
        packetData[1] = newOptions;
        act = EMBER_MANGLE_PACKET;
      }
      break;
    }
    case EMBER_ZIGBEE_PACKET_TYPE_APS_COMMAND: {
      if (printingMask & PRINTING_MASK_APS) {
        uint8_t fc = *(uint8_t*) data;
        emberAfCorePrint("aps:rx seq AC fc %X cmd %X payload[",
                         fc,
                         commandId);
        emberAfCorePrintBuffer(packetData + 1, packetLength - 1, true); // spaces?
        emberAfCorePrintln("]");
      }
      break;
    }
    case EMBER_ZIGBEE_PACKET_TYPE_ZDO: {
      EmberStatus status;
      EmberApsFrame *apsFrame = (EmberApsFrame *) data;
      if (printingMask & PRINTING_MASK_ZDO) {
        emberAfCorePrint("zdo:t%4X:%p seq %X cmd %2X payload[",
                         emberAfGetCurrentTime(),
                         "rx",
                         packetData[0],
                         apsFrame->clusterId);
        emberAfCorePrintBuffer(packetData + ZDO_MESSAGE_OVERHEAD,
                               packetLength - ZDO_MESSAGE_OVERHEAD,
                               true); // spaces?
        emberAfCorePrintln("]");
      }
      status = emAfPluginTestHarnessZ3ZdoCommandResponseHandler(packetData,
                                                                packetLength,
                                                                apsFrame);
      if (status != EMBER_INVALID_CALL) {
        emberAfCorePrintln("%p: %p: cluster: 0x%02X status: 0x%X",
                           TEST_HARNESS_Z3_PRINT_NAME,
                           "ZDO negative command",
                           apsFrame->clusterId | 0x8000,
                           status);
      }
      break;
    }
    case EMBER_ZIGBEE_PACKET_TYPE_ZCL: {
      if (printingMask & PRINTING_MASK_ZCL) {
        emberAfCorePrint("t%4x:rx len %d, ep %X, clus 0x1000 (ZLL Commissioning) FC %X seq %X cmd %X payload[",
                         emberAfGetCurrentTime(),
                         packetLength,
                         1,             // fake endpoint 1 for zll
                         packetData[0], // frame control
                         packetData[1], // sequence
                         packetData[2]); // command
        emberAfCorePrintBuffer(packetData + 3,
                               packetLength - 3,
                               true); // spaces?
        emberAfCorePrintln("]");
        act = emAfPluginTestHarnessZ3ZllCommandCallback(packetData,
                                                        data); // source eui64
      }
      break;
    }
    default:
      emberAfDebugPrintln("Error: unsupported incoming EmberZigbeePacketType %X.", packetType);
      break;
  }
  return act;
  #else
  return EMBER_ACCEPT_PACKET;
  #endif // ESZP_HOST
}

EmberPacketAction emberAfOutgoingPacketFilterCallback(EmberZigbeePacketType packetType,
                                                      uint8_t* packetData,
                                                      uint8_t* size_p,
                                                      void *data)
{
  #if !defined(ESZP_HOST)
  EmberPacketAction act = EMBER_ACCEPT_PACKET;
  uint8_t packetLength = *size_p;

  switch (packetType) {
    case EMBER_ZIGBEE_PACKET_TYPE_BEACON: {
      emberAfDebugPrintln("emberOutgoingPH - sending a beacon");
      uint8_t payloadLength = BEACON_PAYLOAD_SIZE;
      act = emAfPluginTestHarnessZ3ModifyBeaconPayload(packetData + MAC_BEACON_SIZE,
                                                       &payloadLength);
      if (payloadLength != BEACON_PAYLOAD_SIZE) {
        emberAfDebugPrintln("Old buffer length = %d", packetLength);
        *size_p = packetLength - BEACON_PAYLOAD_SIZE + payloadLength;
        emberAfDebugPrintln("New buffer length = %d", *size_p);
      }
      break;
    }
    case EMBER_ZIGBEE_PACKET_TYPE_MAC_COMMAND: {
      uint8_t macHeaderLength = emMacHeaderLength(packetData);
      uint8_t payloadDataLength = packetLength - macHeaderLength;
      assert(macHeaderLength + payloadDataLength <= COMMAND_DATA_SIZE);

      emberAfDebugPrint("mac hdr:tx %d bytes [", macHeaderLength);
      emberAfDebugPrintBuffer(packetData, macHeaderLength, true); // spaces?
      emberAfDebugPrintln("]");
      emberAfDebugPrint("mac pay:tx %d bytes [", payloadDataLength);
      emberAfDebugPrintBuffer(packetData + macHeaderLength, payloadDataLength, true);
      emberAfDebugPrintln("]");

      // look at the APS header to see if we have an interpan msg.
      if (packetData[macHeaderLength + 2] == 0x03) {
        emberAfDebugPrintln("emberOutgoingPH - sending interpan msg");
        act = emAfPluginTestHarnessZ3ZllModifyInterpanCommand(packetData,
                                                              macHeaderLength);
      }
      break;
    }
    default:
      emberAfCorePrintln("Error: unsupported outgoing EmberZigbeePacketType %X.", packetType);
      break;
  }
  return act;
  #else
  return EMBER_ACCEPT_PACKET;
  #endif // ESZP_HOST
}

// -----------------------------------------------------------------------------
// Printing

void emAfPluginTestHarnessZ3PrintingCommand(void)
{
  bool enabled = (emberStringCommandArgument(-1, NULL)[0] == 'e');
  uint8_t mask;

  switch (emberStringCommandArgument(-2, NULL)[0]) {
    case 'b':
      mask = PRINTING_MASK_BEACONS;
      break;
    case 'z':
      mask = PRINTING_MASK_ZDO;
      break;
    case 'n':
      mask = PRINTING_MASK_NWK;
      break;
    case 'a':
      mask = PRINTING_MASK_APS;
      break;
    case 'r':
      mask = PRINTING_MASK_RAW;
      break;
    default:
      mask = 0xFF;
  }

  if (enabled) {
    SETBITS(printingMask, mask);
  } else {
    CLEARBITS(printingMask, mask);
  }

  emberAfCorePrintln("%puccessfully %p printing.",
                     (mask == 0xFF ? "Uns" : "S"),
                     (enabled ? "enabled" : "disabled"));
}

// -----------------------------------------------------------------------------
// Framework Callbacks

void emberAfPluginTestHarnessZ3StackStatusCallback(EmberStatus status)
{
  if (status == EMBER_NETWORK_UP
      && emberAfGetNodeId() == EMBER_TRUST_CENTER_NODE_ID) {
    // Force the network open for joining if we are the trust center.
    // This is terrible security, but this plugin is for a terrible test
    // harness app, so I feel like it lines up.
    emberEventControlSetActive(emberAfPluginTestHarnessZ3OpenNetworkEventControl);
  }
}

void emberAfPluginTestHarnessZ3OpenNetworkEventHandler(void)
{
  emberEventControlSetInactive(emberAfPluginTestHarnessZ3OpenNetworkEventControl);

  if (emberAfGetNodeId() == EMBER_TRUST_CENTER_NODE_ID) {
    EmberEUI64 wildcardEui64 = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, };
    EmberKeyData centralizedKey = ZIGBEE_PROFILE_INTEROPERABILITY_LINK_KEY;
    uint16_t transientKeyTimeoutS;

    // Make sure the trust center lets everyone on the network and
    // allows key requests.
    emberAddTransientLinkKey(wildcardEui64, &centralizedKey);
#ifdef EZSP_HOST
    ezspSetPolicy(EZSP_TC_KEY_REQUEST_POLICY, EMBER_ALLOW_TC_LINK_KEY_REQUEST_AND_SEND_CURRENT_KEY);
    ezspGetConfigurationValue(EZSP_CONFIG_TRANSIENT_KEY_TIMEOUT_S,
                              &transientKeyTimeoutS);
#else
    emberTrustCenterLinkKeyRequestPolicy = EMBER_ALLOW_TC_LINK_KEY_REQUEST_AND_SEND_CURRENT_KEY;
    transientKeyTimeoutS = emberTransientKeyTimeoutS;
#endif

    // Make sure we reopen the network before the transient key disappears.
    // Add in a timing slop of a second to prevent any race conditions.
    emberEventControlSetDelayMS(emberAfPluginTestHarnessZ3OpenNetworkEventControl,
                                ((transientKeyTimeoutS * MILLISECOND_TICKS_PER_SECOND)
                                 - MILLISECOND_TICKS_PER_SECOND));
  }
}

// -----------------------------------------------------------------------------
// Core CLI commands

// plugin test-harness z3 reset
void emAfPluginTestHarnessZ3ResetCommand(void)
{
  emberAfZllResetToFactoryNew();

  emberAfCorePrintln("%p: %p: 0x%X",
                     TEST_HARNESS_Z3_PRINT_NAME,
                     "Reset",
                     EMBER_SUCCESS);
}

// plugin test-harness z3 set-device-mode <mode:1>
void emAfPluginTestHarnessZ3SetDeviceModeCommand(void)
{
  EmberStatus status = EMBER_BAD_ARGUMENT;
  EmAfPluginTestHarnessZ3DeviceMode mode
    = (EmAfPluginTestHarnessZ3DeviceMode)emberUnsignedCommandArgument(0);

  if (mode <= EM_AF_PLUGIN_TEST_HARNESS_Z3_DEVICE_MODE_MAX) {
    emAfPluginTestHarnessZ3DeviceMode = mode;
    status = EMBER_SUCCESS;
  }

  emberAfCorePrintln("%p: %p: 0x%X",
                     TEST_HARNESS_Z3_PRINT_NAME,
                     "Set device mode",
                     status);
}

// plugin test-harness z3 set-short-address
void emAfPluginTestHarnessZ3SetShortAddressCommand(void)
{
  emberAfCorePrintln("%p: %p: 0x%X",
                     TEST_HARNESS_Z3_PRINT_NAME,
                     "Set short address",
                     EMBER_INVALID_CALL);
}

// plugin test-harness z3 legacy-profile-id
void emAfPluginTestHarnessZ3LegacyProfileCommand(void)
{
  emAfPluginTestHarnessZ3TouchlinkProfileId = (emberStringCommandArgument(-1, NULL)[0] == 'e')
                                              ? EMBER_ZLL_PROFILE_ID
                                              : HA_PROFILE_ID;
}

// plugin test-harness z3 set-pan-id
void emAfPluginTestHarnessSetNetworkCreatorPanId(void)
{
  EmberPanId panId = (EmberPanId)emberUnsignedCommandArgument(0);
  emAfZllSetPanId(panId);

  emberAfCorePrintln("Network Creator PAN ID = 0x%2X", panId);
}

// plugin test-harness z3 platform
void emAfPluginTestHarnessZ3PlatformCommand(void)
{
  emberAfCorePrintln("Platform: Silicon Labs");
  emberAfCorePrint("EmberZNet ");
  emAfCliVersionCommand();
}

// plugin test-harness z3 install-code clear
// plugin test-harness z3 install-code set <code>
void emAfPluginTestHarnessZ3InstallCodeClearOrSetCommand(void)
{
  bool doClear = (emberStringCommandArgument(-1, NULL)[0] == 'c');

#ifndef EZSP_HOST
#if !defined(EMBER_AF_HAS_SECURITY_PROFILE_NONE)
  if (false == doClear) {
    EmberEUI64 eui64 = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff }; //wild card
    EmberKeyData key;
    EmberStatus status;
    uint8_t code[16 + 2]; // 6, 8, 12, or 16 bytes plus two-byte CRC
    uint8_t length;

    length = emberCopyStringArgument(0, code, sizeof(code), false);

    // Convert the install code to a key
    status = emAfInstallCodeToKey(code, length, &key);

    if (EMBER_SUCCESS != status) {
      if (EMBER_SECURITY_DATA_INVALID == status) {
        emberAfAppPrintln("ERR: Calculated CRC does not match -len");
      } else if (EMBER_BAD_ARGUMENT == status) {
        emberAfAppPrintln("ERR: Install Code must be 8, 10, 14, or 18 bytes in "
                          "length");
      } else {
        emberAfAppPrintln("ERR: AES-MMO hash failed: 0x%x", status);
      }
      return;
    }

    // Add the key to transient key.
    status = emberAddTransientLinkKey(eui64, &key);
    emberAfAppDebugExec(emAfPrintStatus("Set joining link key", status));
    emberAfAppPrintln("");
    emberAfAppFlush();
  } else {
    // We currently clear all the transient keys.
    emberClearTransientLinkKeys();
    emberAfAppPrintln("Success: Clear joining link key");
  }
#else
  emberAfAppPrintln("Security Core Library must be included to use this command.");
#endif
#else
  emberAfCorePrintln("%p: %p %p: 0x%X",
                     TEST_HARNESS_Z3_PRINT_NAME,
                     "Install code",
                     (doClear ? "clear" : "set"),
                     EMBER_INVALID_CALL);
#endif
}
