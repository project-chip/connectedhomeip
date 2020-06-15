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
 * @brief This plugin is used for testing. It creates MAC address filters to
 * artificially drop packets from certain devices to force a multi-hop
 * topology even if all devices are within radio range.
 * The packets are dropped above the lower MAC before the stack has a chance
 * to really process them but AFTER the radio has generated MAC ACKs for them.
 *******************************************************************************
 * # License
 * <b>Copyright 2018 Silicon Laboratories, Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc.  Your use of this
 * software is governed by the terms of  Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement.  This
 * software  is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 ******************************************************************************/

#include PLATFORM_HEADER

#ifdef PRO_COMPLIANCE
 #include "app/test/pro-compliance.h"
 #include "app/test/pro-compliance-printing.h"
#else // PRO_COMPLIANCE
// this file contains all the common includes for clusters in the zcl-util
 #include "app/framework/include/af.h"
 #include "app/framework/util/common.h"
 #include "app/framework/util/util.h"
 #include "app/util/serial/command-interpreter2.h"
#endif // PRO_COMPLIANCE

#include "stack/include/ember-types.h"
#include "app/framework/plugin-soc/mac-address-filtering/mac-address-filtering.h"

#if defined(EMBER_SCRIPTED_TEST)
  #include "app/framework/test/test-framework.h"
#else
  #define expectCheckpoint(...)
  #define debug(...)
#endif

#if defined (EMBER_SCRIPTED_TEST)
  #define EMBER_AF_PLUGIN_MAC_ADDRESS_FILTERING_SHORT_ADDRESS_FILTER_LENGTH 10
  #define EMBER_AF_PLUGIN_MAC_ADDRESS_FILTERING_LONG_ADDRESS_FILTER_LENGTH  10
  #define EMBER_AF_PLUGIN_MAC_ADDRESS_FILTERING_PAN_ID_FILTER_LENGTH 10

  #undef EMBER_SUPPORTED_NETWORKS
  #define EMBER_SUPPORTED_NETWORKS 1
#elif defined (PRO_COMPLIANCE)
  #define EMBER_AF_PLUGIN_MAC_ADDRESS_FILTERING_SHORT_ADDRESS_FILTER_LENGTH 4
  #define EMBER_AF_PLUGIN_MAC_ADDRESS_FILTERING_LONG_ADDRESS_FILTER_LENGTH  4
  #define EMBER_AF_PLUGIN_MAC_ADDRESS_FILTERING_PAN_ID_FILTER_LENGTH 2

  #undef EMBER_SUPPORTED_NETWORKS
  #define EMBER_SUPPORTED_NETWORKS 1
#endif // EMBER_SCRIPTED_TEST || PRO_COMPLIANCE

// *****************************************************************************
// Typedefs

// These MUST correspond to the bit numbers defined for the filter options mask.
typedef enum {
  MAC_ADDRESS_MODE_NONE  = 0,
  MAC_ADDRESS_MODE_SHORT = 1,
  MAC_ADDRESS_MODE_LONG  = 2,
} MacAddressMode;

// NOTE:  We don't include the PAN ID as an "address mode" above because
// the PAN ID is separate from the address mode and it can be filtered
// on in addition to the source address.  However,
// we need to uniquely identify the list mask below in order to
// disambiguate the options we use to store stats and other items.

typedef enum {
  MAC_FILTER_TYPE_NO_ADDRESS    = 0,
  MAC_FILTER_TYPE_SHORT_ADDRESS = 1,
  MAC_FILTER_TYPE_LONG_ADDRESS  = 2,
  MAC_FILTER_TYPE_PAN_ID        = 3,
} MacFilterType;

#define MAC_FILTER_OPTIONS_NO_ADDRESS_MASK \
  (1 << MAC_FILTER_TYPE_NO_ADDRESS)
#define MAC_FILTER_OPTIONS_SHORT_ADDRESS_LIST_MASK \
  (1 << MAC_FILTER_TYPE_SHORT_ADDRESS)
#define MAC_FILTER_OPTIONS_LONG_ADDRESS_LIST_MASK \
  (1 << MAC_FILTER_TYPE_LONG_ADDRESS)
#define MAC_FILTER_OPTIONS_DEST_PAN_ID_LIST_MASK \
  (1 << MAC_FILTER_TYPE_PAN_ID)

// All possible options listed above
#define MAC_FILTER_OPTIONS_COUNT 4

#define MAC_FILTER_OPTIONS_DROP_NO_ADDRESS         0x00
#define MAC_FILTER_OPTIONS_SHORT_ADDRESS_BLACKLIST 0x00
#define MAC_FILTER_OPTIONS_LONG_ADDRESS_BLACKLIST  0x00

#define MAC_FILTER_OPTIONS_ALLOW_NO_ADDRESS        0x01
#define MAC_FILTER_OPTIONS_SHORT_ADDRESS_WHITELIST 0x02
#define MAC_FILTER_OPTIONS_LONG_ADDRESS_WHITELIST  0x04

#define MAC_FILTER_OPTIONS_DEST_PAN_ID_WHITELIST 0x08
#define MAC_FILTER_OPTIONS_DEST_PAN_ID_BLACKLIST 0x00

#define BROADCAST_PAN_ID 0xFFFF

// By default when a blacklist is enabled and there are no addresess in the
// list, then all messages will be accepted.
#define MAC_FILTER_OPTIONS_DEFAULTS (MAC_FILTER_OPTIONS_SHORT_ADDRESS_BLACKLIST  \
                                     | MAC_FILTER_OPTIONS_LONG_ADDRESS_BLACKLIST \
                                     | MAC_FILTER_OPTIONS_ALLOW_NO_ADDRESS       \
                                     | MAC_FILTER_OPTIONS_DEST_PAN_ID_BLACKLIST)

typedef uint8_t MacAddressFilterOptions;

typedef struct {
  MacAddressFilterOptions bit;
  const char* optionName;
  const char* enabledDescription;
  const char* disabledDescription;
} MacAddressFilterOptionsDescription;

// A union could probably be used for the short/long and combine these two
// structs but that potentially creates developer errors if the wrong item
// in the struct is referenced.  Creating two structs will create compiler
// warnings/errors if used incorrectly.
typedef struct {
  uint32_t matchCount;
  EmberEUI64 longAddress;
} LongAddressFilterMatchStruct;

typedef struct {
  uint32_t matchCount;
  EmberNodeId shortAddress;
} ShortAddressFilterMatchStruct;

typedef struct {
  uint32_t matchCount;
  EmberPanId panId;
} PanIdFilterMatchStruct;

// This filter list should work regardless of the multi-MAC configuration
//   (i.e. the Dual-radio Smart Energy 2.4 ghz / sub-ghz devices)
// Short addresses are universal to the 2.4 ghz / sub-ghz MACs since
// there is one network across both.
// Multi-network will have a problem with Short address filtering because they
// will clash.  I am NOT making this code multi-network aware for the sake of
// simplicity.
typedef struct {
  ShortAddressFilterMatchStruct macShortAddressList[EMBER_AF_PLUGIN_MAC_ADDRESS_FILTERING_SHORT_ADDRESS_FILTER_LENGTH];
  LongAddressFilterMatchStruct  macLongAddressList[EMBER_AF_PLUGIN_MAC_ADDRESS_FILTERING_LONG_ADDRESS_FILTER_LENGTH];
  PanIdFilterMatchStruct panIdFilterMatchList[EMBER_AF_PLUGIN_MAC_ADDRESS_FILTERING_PAN_ID_FILTER_LENGTH];

  MacAddressFilterOptions options;

  uint32_t allowedPacketCount[MAC_FILTER_OPTIONS_COUNT];
  uint32_t droppedPacketCount[MAC_FILTER_OPTIONS_COUNT];
  uint32_t totalPackets;
} MacAddressFilterGlobals;

#if defined(EMBER_SUPPORTED_NETWORKS)
  #if EMBER_SUPPORTED_NETWORKS > 1
    #error "MAC Address Filtering plugin does NOT support multiple networks."
  #endif
#endif

typedef enum {
  ALLOW_PACKET = 0,
  DROP_PACKET  = 1,
} FilterResult;

#define MAC_FRAME_CONTROL_FRAME_TYPE_MASK    0x0007
#define MAC_FRAME_CONTROL_FRAME_TYPE_BEACON  0x0000
#define MAC_FRAME_CONTROL_FRAME_TYPE_DATA    0x0001
#define MAC_FRAME_CONTROL_FRAME_TYPE_ACK     0x0002
#define MAC_FRAME_CONTROL_FRAME_TYPE_COMMAND 0x0003

#define MAC_FRAME_CONTROL_VERSION_MASK  0x3000
#define MAC_FRAME_CONTROL_VERSION_SHIFT 12
#define MAC_FRAME_CONTROL_VERSION_2003  0x0000
#define MAC_FRAME_CONTROL_VERSION_2012  0x0002

#define MAC_FRAME_CONTROL_SOURCE_ADDRESS_MODE_SHIFT 14

#define MAC_FRAME_CONTROL_ADDRESS_MODE_MASK     0x0003
#define MAC_FRAME_CONTROL_ADDRESS_MODE_NONE     0x0000
#define MAC_FRAME_CONTROL_ADDRESS_MODE_RESERVED 0x0001
#define MAC_FRAME_CONTROL_ADDRESS_MODE_SHORT    0x0002
#define MAC_FRAME_CONTROL_ADDRESS_MODE_LONG     0x0003

#define MAC_FRAME_CONTROL_DEST_ADDRESS_MODE_SHIFT 10

#define MAC_FRAME_CONTROL_DEST_ADDRESS_MODE_NONE 0x0000

#define MAC_FRAME_CONTROL_PANID_COMPRESSION 0x0040

// *****************************************************************************
// Globals

static const EmberEUI64 zeroEui64 = { 0, 0, 0, 0, 0, 0, 0, 0 };

static const MacAddressFilterOptionsDescription optionsDescription[] = {
  { MAC_FILTER_OPTIONS_NO_ADDRESS_MASK, "No Address Packets  ", "Accept   ", "Drop     " },
  { MAC_FILTER_OPTIONS_SHORT_ADDRESS_LIST_MASK, "Short Address Filter", "Whitelist", "Blacklist" },
  { MAC_FILTER_OPTIONS_LONG_ADDRESS_LIST_MASK, "Long Address Filter ", "Whitelist", "Blacklist" },
  { MAC_FILTER_OPTIONS_DEST_PAN_ID_LIST_MASK, "Dest PAN ID Filter", "Whitelist", "Blacklist" },

  { 0, NULL, NULL, NULL }, // terminator
};

static MacAddressFilterGlobals macAddressFilterData;

// *****************************************************************************
// Internal Functions

static FilterResult filterPacket(MacAddressMode mode,
                                 EmberNodeId shortAddress,
                                 EmberEUI64 longAddress,
                                 EmberPanId panId)
{
  uint8_t i;
  FilterResult result = ALLOW_PACKET;

  // We process Dest PAN ID separately because
  // although the packet may pass pan ID filtering, it may get filtered later
  // for other resaons.
  // PAN ID is the first order filtering, as is done in the real radio.
  // If the message is to the broadcast PAN ID always accept because it would
  // add unnecessary complexity making that yet another filter criteria.
  if (panId != BROADCAST_PAN_ID) {
    // PAN ID Filtering

    // PAN ID is a whitelist
    //   Any Dest PAN IDs *not* on the whitelist would be dropped
    //     If no PAN IDs are listed, all messages with a Source PAN ID would be dropped.
    //   Broadcast PAN ID would be accepted.
    //   Messages without PAN ID would also be accepted.

    // PAN ID is a blacklist
    //   Any Dest PAN ID on the list would be dropped.
    //   Broadcast PAN ID would be accepted
    //   Messages without PAN ID would also be accepted.

    boolean found = false;
    result = ((macAddressFilterData.options & MAC_FILTER_OPTIONS_DEST_PAN_ID_WHITELIST)
              ? ALLOW_PACKET
              : DROP_PACKET);
    debug("Dest PAN ID: 0x%04X\n", panId);
    for (i = 0; !found && i < EMBER_AF_PLUGIN_MAC_ADDRESS_FILTERING_PAN_ID_FILTER_LENGTH; i++) {
      debug("Considering entry %d: 0x%04X\n",
            i,
            macAddressFilterData.panIdFilterMatchList[i].panId);
      if (macAddressFilterData.panIdFilterMatchList[i].panId != BROADCAST_PAN_ID
          && macAddressFilterData.panIdFilterMatchList[i].panId == panId) {
        macAddressFilterData.panIdFilterMatchList[i].matchCount++;
        found = true;
      }
    }

    if (!found) {
      // If not found, the result is the opposite of the list is configured to be.
      result = (result == ALLOW_PACKET ? DROP_PACKET : ALLOW_PACKET);
    }

    if (result == ALLOW_PACKET) {
      macAddressFilterData.allowedPacketCount[MAC_FILTER_TYPE_PAN_ID]++;

      // Keep going...
    } else {
      macAddressFilterData.droppedPacketCount[MAC_FILTER_TYPE_PAN_ID]++;
      expectCheckpoint("Message DROPPED due to PAN ID filtering.");
      return result;
    }
  }

  if (mode == MAC_ADDRESS_MODE_NONE) {
    if (!(macAddressFilterData.options & MAC_FILTER_OPTIONS_ALLOW_NO_ADDRESS)) {
      result = DROP_PACKET;
    }

    // We want to skip down to the end because we don't want to
    // invert the result as we do for the whitelists/blacklists for
    // short and long addressing.
    goto filterPacketDone;
  } else if (mode == MAC_ADDRESS_MODE_SHORT) {
    debug("Short Address mode, source: 0x%04X\n", shortAddress);
    result = ((macAddressFilterData.options & MAC_FILTER_OPTIONS_SHORT_ADDRESS_WHITELIST)
              ? ALLOW_PACKET
              : DROP_PACKET);

    for (i = 0; i < EMBER_AF_PLUGIN_MAC_ADDRESS_FILTERING_SHORT_ADDRESS_FILTER_LENGTH; i++) {
      debug("Considering entry %d: 0x%04X\n",
            i,
            macAddressFilterData.macShortAddressList[i].shortAddress);
      if (shortAddress == macAddressFilterData.macShortAddressList[i].shortAddress
          && (macAddressFilterData.macShortAddressList[i].shortAddress
              != EMBER_NULL_NODE_ID)) {
        macAddressFilterData.macShortAddressList[i].matchCount++;
        goto filterPacketDone;
      }
    }

    // Keep going.
  } else if (mode == MAC_ADDRESS_MODE_LONG) {
    result = ((macAddressFilterData.options & MAC_FILTER_OPTIONS_LONG_ADDRESS_WHITELIST)
              ? ALLOW_PACKET
              : DROP_PACKET);
    debug("Long address mode, source: (>) 0x%02X%02X%02X%02X%02X%02X%02X%02X\n",
          longAddress[7],
          longAddress[6],
          longAddress[5],
          longAddress[4],
          longAddress[3],
          longAddress[2],
          longAddress[1],
          longAddress[0]);

    for (i = 0; i < EMBER_AF_PLUGIN_MAC_ADDRESS_FILTERING_SHORT_ADDRESS_FILTER_LENGTH; i++) {
      if ((0 == MEMCOMPARE(longAddress,
                           macAddressFilterData.macLongAddressList[i].longAddress,
                           EUI64_SIZE))
          && (0 != MEMCOMPARE(longAddress, zeroEui64, EUI64_SIZE))) {
        debug("Found match to IEEE Address on list.\n");
        macAddressFilterData.macLongAddressList[i].matchCount++;
        goto filterPacketDone;
      }
    }

    // Keep going
  } else {
    // We should never reach here unless the caller has a bug.  That's okay in
    // real hardware but we want to catch that in simulation & unit tests.
    EMBER_TEST_ASSERT(0);
  }

  {
    // This should only be executed if there was no match on the above
    // whitelist/blacklists.
    // If there was no match, the result is the opposite of the configured option.
    result = (result == ALLOW_PACKET ? DROP_PACKET : ALLOW_PACKET);
  }

  // Fall through to below.

  filterPacketDone:
  expectCheckpoint("Parsed packet for address mode %s, filter result: %s",
                   ((mode == MAC_ADDRESS_MODE_NONE)
                    ? "NONE"
                    : ((mode == MAC_ADDRESS_MODE_SHORT)
                       ? "SHORT"
                       : "LONG")),
                   (result == ALLOW_PACKET
                    ? "ALLOWED"
                    : "DROPPED"));

  if (result == DROP_PACKET) {
    macAddressFilterData.droppedPacketCount[mode]++;
  } else {
    macAddressFilterData.allowedPacketCount[mode]++;
  }
  return result;
}

static FilterResult parseAndMaybeFilterPacket(uint8_t* rawMacContents,
                                              uint8_t macFrameControlStartIndex)
{
  MacAddressMode sourceMode;
  EmberNodeId macSourceShortAddress = EMBER_NULL_NODE_ID;
  EmberEUI64 macSourceLongAddress;
  EmberPanId panId = BROADCAST_PAN_ID;
  MEMCOPY(macSourceLongAddress, zeroEui64, EUI64_SIZE);

  uint16_t frameControl = (rawMacContents[macFrameControlStartIndex]
                           + (rawMacContents[macFrameControlStartIndex + 1] << 8));
  uint16_t frameType = (frameControl & MAC_FRAME_CONTROL_FRAME_TYPE_MASK);

  debug("FC: 0x%04X, Type: 0x%02X\n", frameControl, frameType);

  switch (frameType) {
    case MAC_FRAME_CONTROL_FRAME_TYPE_BEACON:
    case MAC_FRAME_CONTROL_FRAME_TYPE_DATA:
    case MAC_FRAME_CONTROL_FRAME_TYPE_COMMAND: {
      // 3 = 2 bytes for FC and 1 byte seq
      uint16_t index = macFrameControlStartIndex + 3;
      uint16_t sourceAddressModeFrameControl = ((frameControl
                                                 >> MAC_FRAME_CONTROL_SOURCE_ADDRESS_MODE_SHIFT)
                                                & MAC_FRAME_CONTROL_ADDRESS_MODE_MASK);
      uint16_t destAddressModeFrameControl = ((frameControl
                                               >> MAC_FRAME_CONTROL_DEST_ADDRESS_MODE_SHIFT)
                                              & MAC_FRAME_CONTROL_ADDRESS_MODE_MASK);
      debug("Src Mode: 0x%02X, Dest Mode: 0x%02X\n",
            sourceAddressModeFrameControl,
            destAddressModeFrameControl);

      if (sourceAddressModeFrameControl == MAC_FRAME_CONTROL_ADDRESS_MODE_RESERVED
          || destAddressModeFrameControl == MAC_FRAME_CONTROL_ADDRESS_MODE_RESERVED) {
        // Ignore packets with invalid address modes.
        break;
      }

      uint16_t fcVersion = (frameControl & MAC_FRAME_CONTROL_VERSION_MASK) >> MAC_FRAME_CONTROL_VERSION_SHIFT;

      if ((fcVersion != MAC_FRAME_CONTROL_VERSION_2003)     // enhanced beacon has version 2012
          && (!(fcVersion == MAC_FRAME_CONTROL_VERSION_2012 && frameType == MAC_FRAME_CONTROL_FRAME_TYPE_BEACON))) {
        // Ignore packets not using ZigBee's IEEE-802.15.4 2003 version since
        // this effects the "Intra-pan" field, a.k.a. PAN ID Compression,
        // and where address fields are interpreted.
        break;
      }

      sourceMode = ((sourceAddressModeFrameControl
                     == MAC_FRAME_CONTROL_ADDRESS_MODE_SHORT)
                    ? MAC_ADDRESS_MODE_SHORT
                    : ((sourceAddressModeFrameControl
                        == MAC_FRAME_CONTROL_ADDRESS_MODE_LONG)
                       ? MAC_ADDRESS_MODE_LONG
                       : MAC_ADDRESS_MODE_NONE));

      debug("Source Mode enum: %d\n", sourceMode);

      if (((destAddressModeFrameControl == MAC_FRAME_CONTROL_ADDRESS_MODE_NONE)
           && (sourceMode != MAC_ADDRESS_MODE_NONE))
          || ((destAddressModeFrameControl != MAC_FRAME_CONTROL_ADDRESS_MODE_NONE)
              && (sourceMode == MAC_ADDRESS_MODE_NONE))) {
        // The interpretation of the PAN ID compression bit in IEEE 802.15.4
        // is just crazy.  Instead of making that bit indicate the absence or
        // presence of both source and dest PAN, the bit is contigent
        // on the addressing modes.  This is further complicated by the fact
        // the addresses come after the PAN ID fields and thus we have
        // a strange interdependency between the two.

        // If dest is present but not source, or vice versa, then the
        // PAN ID Compression bit is set zero and only a single PAN ID
        // is included in the message.
        // Otherwise if both src and dest addresses are present,
        // the PAN ID compression bit indicates whether there is both
        // source and dest PAN ID (0), or just dest PAN ID (1).

        // If source address is present then the source PAN ID is present
        // If dest address is present, then dest PAN ID is present
        // Ultimately we only care about the dest address, which
        // is located at the end of the header after any PAN ID field.
        panId = (rawMacContents[index] + (rawMacContents[index + 1] << 8));

        index += 2; // PAN ID is 2-bytes.
      } else {  // Both source and dest address are present
        panId = (rawMacContents[index] + (rawMacContents[index + 1] << 8));

        index += 2; // dest PAN ID is 2-bytes

        if (!(frameControl & MAC_FRAME_CONTROL_PANID_COMPRESSION)) {
          // Technically the source and dest PAN ID can be different.
          // We don't record that because it not really used in ZigBee.
          // The only time it happens is for Association request, and then
          // the source PAN ID is 0xFFFF.  It really begs the question
          // of why bother including it then.
          // For simplicity, just filter on the previously parsed PAN ID.

          index += 2; // source PAN ID is 2-bytes
        }
      }

      if (destAddressModeFrameControl == MAC_FRAME_CONTROL_ADDRESS_MODE_SHORT) {
        index += 2;  // short addresses are 2-bytes
      } else if (destAddressModeFrameControl == MAC_FRAME_CONTROL_ADDRESS_MODE_LONG) {
        index += EUI64_SIZE;
      }

      if (sourceMode == MAC_ADDRESS_MODE_SHORT) {
        macSourceShortAddress = (rawMacContents[index] + (rawMacContents[index + 1] << 8));
      } else if (sourceMode == MAC_ADDRESS_MODE_LONG) {
        MEMCOPY(macSourceLongAddress, rawMacContents + index, EUI64_SIZE);
      }

      return (filterPacket(sourceMode,
                           macSourceShortAddress,
                           macSourceLongAddress,
                           panId));
    }

      // Default: Fall through
  }

  // By default accept the packet and let the stack sort it out.
  // This is the safest and more future proof thing to do.  We don't
  // want this plugin to require an update when the stack changes.
  return ALLOW_PACKET;
}

static void clearStatsForFilterType(MacFilterType type)
{
  uint8_t i;
  macAddressFilterData.allowedPacketCount[type] = 0;
  macAddressFilterData.droppedPacketCount[type] = 0;

  if (type == MAC_FILTER_TYPE_SHORT_ADDRESS) {
    for (i = 0; i < EMBER_AF_PLUGIN_MAC_ADDRESS_FILTERING_SHORT_ADDRESS_FILTER_LENGTH; i++) {
      macAddressFilterData.macShortAddressList[i].matchCount = 0;
    }
  } else if (type == MAC_FILTER_TYPE_LONG_ADDRESS) {
    for (i = 0; i < EMBER_AF_PLUGIN_MAC_ADDRESS_FILTERING_LONG_ADDRESS_FILTER_LENGTH; i++) {
      macAddressFilterData.macLongAddressList[i].matchCount = 0;
    }
  } else if (type == MAC_FILTER_TYPE_PAN_ID) {
    for (i = 0; i < EMBER_AF_PLUGIN_MAC_ADDRESS_FILTERING_PAN_ID_FILTER_LENGTH; i++) {
      macAddressFilterData.panIdFilterMatchList[i].matchCount = 0;
    }
  }
}

static void clearShortFilterList(void)
{
  // We need to set the short address list to be EMBER_NULL_NODE_ID by default
  // (0xFFFF) because 0x0000 is a valid address.  Thus our MEMSET() to 0 for
  // the global struct is not good enough.
  // We can't use MEMSET() because the ShortAddressFilterMatchStruct contains
  // a count variable that should be set to 0.
  uint8_t i;
  for (i = 0; i < EMBER_AF_PLUGIN_MAC_ADDRESS_FILTERING_SHORT_ADDRESS_FILTER_LENGTH; i++) {
    macAddressFilterData.macShortAddressList[i].shortAddress
      = EMBER_NULL_NODE_ID;
  }
  clearStatsForFilterType(MAC_FILTER_TYPE_SHORT_ADDRESS);
}

static void clearLongFilterlist(void)
{
  MEMSET(macAddressFilterData.macLongAddressList,
         0,
         sizeof(LongAddressFilterMatchStruct)
         * EMBER_AF_PLUGIN_MAC_ADDRESS_FILTERING_LONG_ADDRESS_FILTER_LENGTH);
}

static void clearPanIdFilterList(void)
{
  MEMSET(macAddressFilterData.panIdFilterMatchList,
         0xFF,
         sizeof(PanIdFilterMatchStruct)
         * EMBER_AF_PLUGIN_MAC_ADDRESS_FILTERING_PAN_ID_FILTER_LENGTH);
}

// *****************************************************************************
// Public Functions

EmberPacketAction emberAfIncomingPacketFilterCallback(EmberZigbeePacketType packetType,
                                                      uint8_t* packetData,
                                                      uint8_t* size_p,
                                                      void *data)
{
  FilterResult result;
  if (packetType != EMBER_ZIGBEE_PACKET_TYPE_RAW_MAC) {
    expectCheckpoint("Ignoring non-matching callback type.");
    return EMBER_ACCEPT_PACKET;
  }

  macAddressFilterData.totalPackets++;

  // @TODO figure decide on how to deal with index being 0
  result = parseAndMaybeFilterPacket(packetData, 0);
  if (result == DROP_PACKET) {
    return EMBER_DROP_PACKET;
  } else {
    return EMBER_ACCEPT_PACKET;
  }
}

void emberAfPluginMacAddressFilteringInitCallback(void)
{
  MEMSET(&macAddressFilterData, 0, sizeof(MacAddressFilterGlobals));
  macAddressFilterData.options = MAC_FILTER_OPTIONS_DEFAULTS;
  clearShortFilterList();
  clearLongFilterlist();
  clearPanIdFilterList();
}

// <generate-cli-xml> Plugin CLI Description: Commands to manipulate the MAC address filtering tables.

// <generate-cli-xml> Command: plugin mac-address-filtering set-config no-address-filter <enable>
// <generate-cli-xml> Description: Enable/disable filtering of no-address messages.
// <generate-cli-xml> Arg: enable | boolean | True means drop no-address messages, false means allow.

void emberAfPluginMacAddressFilteringFilterNoAddressCommand(void)
{
  bool filterNoAddress = (bool)emberUnsignedCommandArgument(0);

  if (filterNoAddress) {
    macAddressFilterData.options &= ~MAC_FILTER_OPTIONS_ALLOW_NO_ADDRESS;
  } else {
    macAddressFilterData.options |= MAC_FILTER_OPTIONS_ALLOW_NO_ADDRESS;
  }
}

// <generate-cli-xml> Command: plugin mac-address-filtering print-config
// <generate-cli-xml> Description: Prints the current filtering config and statistics.

void emberAfPluginMacAddressFilteringPrintConfigCommand(void)
{
  uint8_t i;
  emberAfCorePrintln("Total MAC Packets received: %lu", macAddressFilterData.totalPackets);
  emberAfCorePrintln("Options bitmask: 0x%X", macAddressFilterData.options);
  for ( i = 0; i < MAC_FILTER_OPTIONS_COUNT; i++ ) {
    const char* enabledDisabledTextPointer = ( (macAddressFilterData.options & optionsDescription[i].bit)
                                               ? optionsDescription[i].enabledDescription
                                               : optionsDescription[i].disabledDescription);
    emberAfCorePrintln("%p: %p  Packets Accepted %lu, Dropped: %lu",
                       optionsDescription[i].optionName,
                       enabledDisabledTextPointer,
                       macAddressFilterData.allowedPacketCount[i],
                       macAddressFilterData.droppedPacketCount[i]);
  }

  emberAfCorePrintln("Short Address Filter List");

  for ( i = 0; i < EMBER_AF_PLUGIN_MAC_ADDRESS_FILTERING_SHORT_ADDRESS_FILTER_LENGTH; i++ ) {
    emberAfCorePrint("%d: ", i);
    if (macAddressFilterData.macShortAddressList[i].shortAddress
        == EMBER_NULL_NODE_ID) {
      emberAfCorePrintln("---");
    } else {
      emberAfCorePrintln("0x%2X  Match Count: %lu",
                         macAddressFilterData.macShortAddressList[i].shortAddress,
                         macAddressFilterData.macShortAddressList[i].matchCount);
    }
  }

  emberAfCorePrintln("\nLong Address Filter List");

  for ( i = 0; i < EMBER_AF_PLUGIN_MAC_ADDRESS_FILTERING_LONG_ADDRESS_FILTER_LENGTH; i++ ) {
    emberAfCorePrint("%d: ", i);
    if (0 == MEMCOMPARE(macAddressFilterData.macLongAddressList[i].longAddress,
                        zeroEui64,
                        EUI64_SIZE)) {
      emberAfCorePrintln("---");
    } else {
      emberAfPrintBigEndianEui64(macAddressFilterData.macLongAddressList[i].longAddress);
      emberAfCorePrintln("  Match Count: %lu",
                         macAddressFilterData.macLongAddressList[i].matchCount);
    }
  }
}

// <generate-cli-xml> Command: plugin mac-address-filtering short-address-list add <node-id>
// <generate-cli-xml> Description: Adds an address to the short address list.
// <generate-cli-xml> Arg: node-id | EmberNodeId | The node-id to add to the short list.

void emberAfPluginMacAddressFilteringAddShortAddressCommand(void)
{
  EmberNodeId nodeId = (uint16_t)emberUnsignedCommandArgument(0);
  uint8_t i = 0;
  for (i = 0; i < EMBER_AF_PLUGIN_MAC_ADDRESS_FILTERING_SHORT_ADDRESS_FILTER_LENGTH; i++) {
    if (EMBER_NULL_NODE_ID
        == macAddressFilterData.macShortAddressList[i].shortAddress) {
      macAddressFilterData.macShortAddressList[i].shortAddress = nodeId;
      macAddressFilterData.macShortAddressList[i].matchCount = 0;
      emberAfCorePrintln("Added short address to list.");
      return;
    }
  }

  emberAfCorePrintln("Error: No free entries in the short list.");
}

#define WHITELIST_CHARACTER_INDEX 4

static void changeListConfig(bool currentConfigIsWhitelist,
                             MacFilterType type,
                             MacAddressFilterOptions valueForWhitelist)
{
  bool changeToWhitelist = false;
  uint8_t* pointer
    = emberStringCommandArgument(-1,   // argument number
                                 NULL); // returned length

  if (pointer[WHITELIST_CHARACTER_INDEX] == 'w') {
    changeToWhitelist = true;
  }

  if ((changeToWhitelist && !currentConfigIsWhitelist)
      || (!changeToWhitelist && currentConfigIsWhitelist)) {
    // If we are setting the list to the opposite of its current value,
    // clear the stats for that item.  Otherwise don't.
    // Prevents a problem where accidentally setting the list type
    // to the same thing clears the stats when that doesn't seem
    // like what the user wants.
    clearStatsForFilterType(type);
    emberAfCorePrintln("Cleared dropped packet count for list.");
  }

  if (changeToWhitelist) {
    macAddressFilterData.options |= valueForWhitelist;
  } else {
    macAddressFilterData.options &= ~valueForWhitelist;
  }

  emberAfCorePrintln("%p list changed to %plist.\n",
                     ((type == MAC_FILTER_TYPE_SHORT_ADDRESS)
                      ? "Short address"
                      : ((type == MAC_FILTER_TYPE_LONG_ADDRESS)
                         ? "Long address"
                         : ((type == MAC_FILTER_TYPE_PAN_ID)
                            ? "PAN ID"
                            : "No Address"))),
                     (changeToWhitelist
                      ? "white"
                      : "black"));
}

// <generate-cli-xml> Command: plugin mac-address-filtering short-address-list set-whitelist
// <generate-cli-xml> Description:  Sets the short address list filter to a whitelist.

// NOTE 1:
//   A bit of hack to make the generate-cli-xml.pl script happy.
//   Since the same function can make the short list a white or black-list
//   we don't need two actual C functions.  But the script wants
//   to parse the next C function line as the command to be executed.
//   Just add a forward declaration and all is well.
void emberAfPluginMacAddressFilteringSetShortAddressListType(void);

// <generate-cli-xml> Command: plugin mac-address-filtering short-address-list set-blacklist
// <generate-cli-xml> Description:  Sets the short address list filter to a blacklist.

void emberAfPluginMacAddressFilteringSetShortAddressListType(void)
{
  changeListConfig((macAddressFilterData.options
                    & MAC_FILTER_OPTIONS_SHORT_ADDRESS_WHITELIST),
                   MAC_FILTER_TYPE_SHORT_ADDRESS,
                   MAC_FILTER_OPTIONS_SHORT_ADDRESS_WHITELIST);
}

// <generate-cli-xml> Command: plugin mac-address-filtering long-address-list set-whitelist
// <generate-cli-xml> Description: Sets the long address list filter to a whitelist.

// NOTE 2:  Forward declaration on purpose.  See Note 1 above.
void emberAfPluginMacAddressFilteringSetLongAddressListType(void);

// <generate-cli-xml> Command: plugin mac-address-filtering long-address-list set-blacklist
// <generate-cli-xml> Description: Sets the long address list filter to a blacklist.

void emberAfPluginMacAddressFilteringSetLongAddressListType(void)
{
  changeListConfig((macAddressFilterData.options
                    & MAC_FILTER_OPTIONS_LONG_ADDRESS_WHITELIST),
                   MAC_FILTER_TYPE_LONG_ADDRESS,
                   MAC_FILTER_OPTIONS_LONG_ADDRESS_WHITELIST);
}

// <generate-cli-xml> Command: plugin mac-address-filtering long-address-list add <eui64>
// <generate-cli-xml> Description: Adds a long address to the long address filter.
// <generate-cli-xml> Arg: eui64 | EmberEUI64 | The long address to add.

void emberAfPluginMacAddressFilteringAddLongAddressCommand(void)
{
  uint8_t i;
  EmberEUI64 eui64;
  emberCopyBigEndianEui64Argument(0, eui64);
  for (i = 0; i < EMBER_AF_PLUGIN_MAC_ADDRESS_FILTERING_LONG_ADDRESS_FILTER_LENGTH; i++) {
    if (0 == MEMCOMPARE(macAddressFilterData.macLongAddressList[i].longAddress,
                        zeroEui64,
                        EUI64_SIZE)) {
      MEMCOPY(macAddressFilterData.macLongAddressList[i].longAddress,
              eui64,
              EUI64_SIZE);
      macAddressFilterData.macLongAddressList[i].matchCount = 0;
      return;
    }
  }

  emberAfCorePrintln("Error: No free entries in the long list.");
}

// <generate-cli-xml> Command: plugin mac-address-filtering short-address-list clear
// <generate-cli-xml> Description: Clears the short address list filter.

void emberAfPluginMacAddressFilteringClearShortAddressList(void)
{
  clearShortFilterList();
  emberAfCorePrintln("Short address list cleared.");
}

// <generate-cli-xml> Command: plugin mac-address-filtering long-address-list clear
// <generate-cli-xml> Description: Clears the long address list filter.

void emberAfPluginMacAddressFilteringClearLongAddressList(void)
{
  clearLongFilterlist();
  emberAfCorePrintln("Long address list cleared.");
}

// <generate-cli-xml> Command: plugin mac-address-filtering pan-id-list add <pan-id>
// <generate-cli-xml> Description: Adds a PAN ID to the filter.
// <generate-cli-xml> Arg: pan-id | uint16_t | The PAN ID to add to the list.

void emberAfPluginMacAddressFilteringAddPanIdCommand(void)
{
  EmberPanId panId = (EmberPanId)emberUnsignedCommandArgument(0);
  uint8_t i = 0;
  for (i = 0; i < EMBER_AF_PLUGIN_MAC_ADDRESS_FILTERING_PAN_ID_FILTER_LENGTH; i++) {
    if (BROADCAST_PAN_ID
        == macAddressFilterData.panIdFilterMatchList[i].panId) {
      macAddressFilterData.panIdFilterMatchList[i].panId = panId;
      macAddressFilterData.panIdFilterMatchList[i].matchCount = 0;
      emberAfCorePrintln("Added PAN ID to list.");
      return;
    }
  }

  emberAfCorePrintln("Error: No free entries in the PAN ID list.");
}

// <generate-cli-xml> Command: plugin mac-address-filtering pan-id-list delete <index>
// <generate-cli-xml> Description: Deletes an entry in the PAN ID list filter.
// <generate-cli-xml> Arg: index | uint8_t | The index of the entry to delete.

void emberAfPluginMacAddressFilteringPanIdDeleteEntry(void)
{
  uint8_t index = emberUnsignedCommandArgument(0);
  if (index >= EMBER_AF_PLUGIN_MAC_ADDRESS_FILTERING_PAN_ID_FILTER_LENGTH) {
    emberAfCorePrintln("Error: Index is greater than or equal to list length (%d)",
                       EMBER_AF_PLUGIN_MAC_ADDRESS_FILTERING_PAN_ID_FILTER_LENGTH);
    return;
  }

  macAddressFilterData.panIdFilterMatchList[index].panId = BROADCAST_PAN_ID;
  emberAfCorePrintln("Entry deleted.");
}

// <generate-cli-xml> Command: plugin mac-address-filtering pan-id-list clear
// <generate-cli-xml> Description: Clears the PAN ID list filter.

void emberAfPluginMacAddressFilteringClearPanIdList(void)
{
  clearPanIdFilterList();
}

// <generate-cli-xml> Command: plugin mac-address-filtering pan-id-list set-whitelist
// <generate-cli-xml> Description:  Sets the PAN-ID list filter to a whitelist.

// NOTE 3: See Note 1 as to why we have a forward declaration here.
void emberAfPluginMacAddressFilteringSetPanIdListType(void);

// <generate-cli-xml> Command: plugin mac-address-filtering pan-id-list set-blacklist
// <generate-cli-xml> Description:  Sets the PAN_ID list filter to a blacklist.

void emberAfPluginMacAddressFilteringSetPanIdListType(void)
{
  changeListConfig(macAddressFilterData.options & MAC_FILTER_OPTIONS_DEST_PAN_ID_WHITELIST,
                   MAC_FILTER_TYPE_PAN_ID,
                   MAC_FILTER_OPTIONS_DEST_PAN_ID_WHITELIST);
}

// <generate-cli-xml> Command: plugin mac-address-filtering reset
// <generate-cli-xml> Description: Resets the state of the plugin to the default of no filtering.

void emberAfPluginMacAddressFilteringReset(void)
{
  emberAfPluginMacAddressFilteringInitCallback();
  emberAfCorePrintln("All filter data cleared.");
}

// <generate-cli-xml> Command: plugin mac-address-filtering short-address-list delete <index>
// <generate-cli-xml> Description: Deletes an entry in the short address list filter.
// <generate-cli-xml> Arg: index | uint8_t | The index of the entry to delete.

void emberAfPluginMacAddressFilteringShortAddressDeleteEntry(void)
{
  uint8_t index = emberUnsignedCommandArgument(0);
  if (index >= EMBER_AF_PLUGIN_MAC_ADDRESS_FILTERING_SHORT_ADDRESS_FILTER_LENGTH) {
    emberAfCorePrintln("Error: Index is greater than or equal to list length (%d)",
                       EMBER_AF_PLUGIN_MAC_ADDRESS_FILTERING_SHORT_ADDRESS_FILTER_LENGTH);
    return;
  }

  macAddressFilterData.macShortAddressList[index].shortAddress = EMBER_NULL_NODE_ID;
  emberAfCorePrintln("Entry deleted.");
}

// <generate-cli-xml> Command: plugin mac-address-filtering long-address-list delete <index>
// <generate-cli-xml> Description: Deletes an entry in the long address list filter.
// <generate-cli-xml> Arg: index | uint8_t | The entry in the long address list to delete.

void emberAfPluginMacAddressFilteringLongAddressDeleteEntry(void)
{
  uint8_t index = emberUnsignedCommandArgument(0);
  if (index >= EMBER_AF_PLUGIN_MAC_ADDRESS_FILTERING_LONG_ADDRESS_FILTER_LENGTH) {
    emberAfCorePrintln("Error: Index is greater than or equal to list length (%d)",
                       EMBER_AF_PLUGIN_MAC_ADDRESS_FILTERING_LONG_ADDRESS_FILTER_LENGTH);
    return;
  }

  MEMSET(macAddressFilterData.macLongAddressList[index].longAddress,
         0,
         EUI64_SIZE);
  emberAfCorePrintln("Entry deleted.");
}

EmberStatus emberAfPluginMacAddressFilteringGetStatsForShortAddress(uint8_t index,
                                                                    uint32_t* matchCountForIndexPtr)
{
  if (index >= EMBER_AF_PLUGIN_MAC_ADDRESS_FILTERING_SHORT_ADDRESS_FILTER_LENGTH) {
    return EMBER_BAD_ARGUMENT;
  }

  *matchCountForIndexPtr = macAddressFilterData.macShortAddressList[index].matchCount;

  return EMBER_SUCCESS;
}

void emberAfPluginMacAddressFilteringGetStats(boolean shortMode,
                                              uint32_t* allowedPacketCountPtr,
                                              uint32_t* droppedPacketCountPtr,
                                              uint32_t* totalPacketCountPtr)
{
  MacAddressMode mode = (shortMode
                         ? MAC_ADDRESS_MODE_SHORT
                         : MAC_ADDRESS_MODE_LONG);

  *allowedPacketCountPtr = macAddressFilterData.allowedPacketCount[mode];
  *droppedPacketCountPtr = macAddressFilterData.droppedPacketCount[mode];
  *totalPacketCountPtr   = macAddressFilterData.totalPackets;
}

// <generate-cli-xml> Command: plugin mac-address-filtering clear-stats
// <generate-cli-xml> Description:  Clears all packet counts for all modes.  Leaves config and lists intact.

void emberAfPluginMacAddressFilteringClearStats(void)
{
  clearStatsForFilterType(MAC_FILTER_TYPE_NO_ADDRESS);
  clearStatsForFilterType(MAC_FILTER_TYPE_SHORT_ADDRESS);
  clearStatsForFilterType(MAC_FILTER_TYPE_LONG_ADDRESS);
  clearStatsForFilterType(MAC_FILTER_TYPE_PAN_ID);
}
