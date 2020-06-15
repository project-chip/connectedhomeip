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
 * @brief Test code for the mac-address-filtering plugin.
 *******************************************************************************
   ******************************************************************************/

// this file contains all the common includes for clusters in the zcl-util
#include "app/framework/include/af.h"
#include "app/framework/util/common.h"
#include "app/framework/util/util.h"

#include "app/framework/test/test-framework.h"

#include "app/framework/plugin-soc/mac-address-filtering/mac-address-filtering.h"

#include "stack/include/message.h"

// *****************************************************************************
// Globals

// Set for the max packet size.  PHY Length byte reduces the size to 127.
#define MAX_DATA_SIZE 127

#define BUFFER_OVERHEAD 6  // overehead from raw MAC in the stack is 6-bytes

static uint8_t beaconRequestData[] = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // Overhead from our MAC is 6 bytes

  0x03, 0x08, // FC
  0xC0,       // Seq
  0xFF, 0xFF, // Dest PAN ID
  0xFF, 0xFF, // Dest Short Address
  0x07        // Command ID
};

static uint8_t beaconData[] = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // Overhead from our MAC is 6 bytes

  0x00, 0x80,   // FC
  0xF0,         // seq
  0x22, 0x11,   // Source PAN ID
  0x00, 0x00,   // Source Short Address
  0xFF, 0xCF,   // IEEE Beacon

  // Zigbee beacon data
  0x00, 0x00, 0x00, 0x22, 0x84, 0x1D, 0x90, 0x78, 0x56, 0x34,
  0x12, 0xDB, 0x00, 0xFF, 0xFF, 0xFF, 0x00
};

#define ASSOCIATION_REQUEST_SORUCE_ADDRESS_INDEX 16
static uint8_t associationRequestData[] = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // Overhead from our MAC is 6 bytes

  0x23, 0xC8, // FC
  0xC1,       // Seq
  0x22, 0x11, // Dest PAN ID
  0x00, 0x00, // Dest Address
  0xFF, 0xFF, // Source PAN ID
  0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // Source Long Address

  // IEEE Command
  0x01, 0x84
};

static uint8_t associationResponseData[] = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // Overhead from our MAC is 6 bytes

  0x63, 0xCC,  // FC
  0x61,        // Seq
  0x22, 0x11,  // Dest PAN ID
  0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Long Dest Address
  0x1D, 0x90, 0x78, 0x56, 0x34, 0x12, 0xDB, 0x00, // Long Source Address

  // IEEE Command
  0x02,       // Command ID
  0xB6, 0xD2, // Assigned Short Address
  0x00        // Association Status (SUCCESS)
};

#define NWK_BROADCAST_DEST_ADDRESS_INDEX 13
static uint8_t nwkBroadcastMessageData[] = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // Overhead from our MAC is 6 bytes

  0x41, 0x88, // FC
  0x69,       // Seq
  0x22, 0x11, // Dest PAN ID
  0xFF, 0xFF, // Short Dest Address
  0x00, 0x00, // Short Source Address

  // Zigbee NWK Layer
  0x09, 0x12, 0xFC, 0xFF, 0x00, 0x00, 0x1E, 0x6E,
  0x1D, 0x90, 0x78, 0x56, 0x34, 0x12, 0xDB, 0x00,

  // Security Header
  0x28, 0xC9, 0x01, 0x00, 0x00, 0x1D, 0x90, 0x78,
  0x56, 0x34, 0x12, 0xDB, 0x00, 0x00,

  // Encrypted Payload
  0xD9, 0x20, 0xD7, 0x3E, 0x76, 0xEF, 0x6B, 0xF0, 0x34,

  // MIC
  0x15, 0xE1, 0x2D, 0x9F
};

static uint8_t associationRequestIeeeSourceLittleEndian[] = {
  0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

static uint8_t associationResponseIeeeSourceLittleEndian[] = {
  0x1D, 0x90, 0x78, 0x56, 0x34, 0x12, 0xDB, 0x00,
};

//static uint8_t zclUnicast[] = {};

static uint32_t unsignedCommandArgumentValue;

#define ACCEPT_PACKET true
#define DENY_PACKET   false

static char whitelistArg[] = "set-whitelist";
static char blacklistArg[] = "set-blacklist";
static char* stringCommandArgumentPtr = whitelistArg;

static uint8_t* eui64StringArgumentPtr =
  associationRequestIeeeSourceLittleEndian;

// *****************************************************************************
// Forward declarations

static void testIncomingRawMacPacketFunction(uint8_t* packet,
                                             uint8_t length,
                                             const char* expectedMessage,
                                             bool expectedReturnValue,
                                             const char* file,
                                             int lineNumber,
                                             const char* functionName);

// *****************************************************************************
// Stubs

static uint8_t bufferData[32 * EMBER_PACKET_BUFFER_COUNT];
static uint8_t bufferLengths[EMBER_PACKET_BUFFER_COUNT];
static uint8_t referenceCounts[EMBER_PACKET_BUFFER_COUNT];
static EmberMessageBuffer bufferLinks[EMBER_PACKET_BUFFER_COUNT];
static EmberMessageBuffer queueLinks[EMBER_PACKET_BUFFER_COUNT];

uint8_t emPacketBufferCount = EMBER_PACKET_BUFFER_COUNT;
uint8_t emPacketBufferFreeCount = EMBER_PACKET_BUFFER_COUNT;
uint8_t* emPacketBufferData = bufferData;
uint8_t* emMessageBufferLengths = bufferLengths;
uint8_t* emMessageBufferReferenceCounts = referenceCounts;
EmberMessageBuffer *emPacketBufferLinks = bufferLinks;
EmberMessageBuffer *emPacketBufferQueueLinks = queueLinks;

void emCallCounterHandler(EmberCounterType type, uint8_t data)
{
}

uint32_t emberUnsignedCommandArgument(uint8_t x)
{
  return unsignedCommandArgumentValue;
}

uint8_t *emberStringCommandArgument(int8_t argNum, uint8_t *length)
{
  return (uint8_t*)stringCommandArgumentPtr;
}

void emberAfPrintBigEndianEui64(const EmberEUI64 eui64)
{
  // Should not be called during unit testing
  assert(0);
}

uint8_t emberCopyBigEndianEui64Argument(int8_t index, EmberEUI64 destination)
{
  MEMCOPY(destination, eui64StringArgumentPtr, EUI64_SIZE);
  return EUI64_SIZE;
}

EmberApsFrame emberAfResponseApsFrame;

#define testIncomingRawMacPacket(packet, length, expectedMessage, expectedReturnValue) \
  testIncomingRawMacPacketFunction(packet,                                             \
                                   length,                                             \
                                   expectedMessage,                                    \
                                   expectedReturnValue,                                \
                                   __FILE__,                                           \
                                   __LINE__,                                           \
                                   __PRETTY_FUNCTION__)

// *****************************************************************************
// Tests

static void testIncomingRawMacPacketFunction(uint8_t* packet,
                                             uint8_t length,
                                             const char* expectedMessage,
                                             bool expectedReturnValue,
                                             const char* file,
                                             int lineNumber,
                                             const char* functionName)
{
  EmberPacketAction result, expected;
  uint8_t packetLength = length - BUFFER_OVERHEAD;

  if (expectedReturnValue) {
    expected = EMBER_ACCEPT_PACKET;
  } else {
    expected = EMBER_DROP_PACKET;
  }

  if (expectedMessage != NULL) {
    setCheckpointFunction(expectedMessage, file, lineNumber, functionName);
  }
  result = emberAfIncomingPacketFilterCallback(EMBER_ZIGBEE_COMMAND_TYPE_RAW_MAC,
                                               packet + BUFFER_OVERHEAD,
                                               &packetLength,
                                               NULL);
  verifyCheckpoint();
  assert(result == expected);
  note(".");
}

static void packetCountCheckShortAddressFunction(uint8_t index,
                                                 uint32_t expectedMatchCount,
                                                 uint32_t expectedAllowedPacketCount,
                                                 uint32_t expectedDroppedPacketCount,
                                                 uint32_t expectedTotalPacketCount,
                                                 const char* file,
                                                 int lineNumber,
                                                 const char* caller)
{
  uint32_t actualMatchCount = 0;
  uint32_t actualAllowedPacketCount = 0;
  uint32_t actualDroppedPacketCount = 0;
  uint32_t actualTotalPacketCount = 0;

  expectFunction(EMBER_SUCCESS
                 == emberAfPluginMacAddressFilteringGetStatsForShortAddress(index,
                                                                            &actualMatchCount),
                 file,
                 lineNumber,
                 "emberAfPluginMacAddressFilteringGetStatsForShortAddress",
                 "Expected EMBER_SUCCESS");

  emberAfPluginMacAddressFilteringGetStats(true, // short mode
                                           &actualAllowedPacketCount,
                                           &actualDroppedPacketCount,
                                           &actualTotalPacketCount);

  expectComparison(expectedMatchCount,
                   actualMatchCount,
                   false,  // print hex?
                   "Expected matchCount for index",
                   "Actual matchCount for index",
                   file,
                   lineNumber,
                   caller);

  expectComparison(expectedAllowedPacketCount,
                   actualAllowedPacketCount,
                   false,  // print hex?
                   "Expected allowedPacketCount for index",
                   "Actual allowedPacketCount for index",
                   file,
                   lineNumber,
                   caller);

  expectComparison(expectedDroppedPacketCount,
                   actualDroppedPacketCount,
                   false,  // print hex?
                   "Expected droppedPacketCount for index",
                   "Actual droppedPacketCount for index",
                   file,
                   lineNumber,
                   caller);

  expectComparison(expectedTotalPacketCount,
                   actualTotalPacketCount,
                   false,  // print hex?
                   "Expected totalPacketCount for index",
                   "Actual totalPacketCount for index",
                   file,
                   lineNumber,
                   caller);
}

#define packetCountCheckShortAddress(v, w, x, y, z) \
  packetCountCheckShortAddressFunction(v, w, x, y, z, __FILE__, __LINE__, __PRETTY_FUNCTION__)

static void runAllTests(void)
{
  uint8_t data[MAX_DATA_SIZE];
  uint8_t length = MAX_DATA_SIZE;
  EmberMessageBuffer buffer;
  emberAfPluginMacAddressFilteringInitCallback();

  //---------------------------------------------------------

  MEMSET(data, 0, MAX_DATA_SIZE);

  debug("Testing non-matching callback is ignored for emberAfIncomingPacketFilterCallback().\n");
  buffer = emberFillLinkedBuffers(data, MAX_DATA_SIZE);
  assert(buffer != EMBER_NULL_MESSAGE_BUFFER);

  setCheckpoint("Ignoring non-matching callback type.");
  emberAfIncomingPacketFilterCallback(EMBER_ZIGBEE_COMMAND_TYPE_NWK,
                                      data,
                                      &length,  // index (don't care for this test)
                                      NULL);
  emberReleaseMessageBuffer(buffer);
  buffer = EMBER_NULL_MESSAGE_BUFFER;

  packetCountCheckShortAddress(0,  // index
                               0,  // match count
                               0,  // short address mode allowed packet count
                               0,  // short address mode dropped packet count
                               0); // total mac packet count

  //---------------------------------------------------------

  debug("\n---\nTesting beacon request is accepted (no source address info)\n");
  testIncomingRawMacPacket(beaconRequestData,
                           sizeof(beaconRequestData),
                           "Parsed packet for address mode NONE, filter result: ALLOWED",
                           ACCEPT_PACKET);

  packetCountCheckShortAddress(0,  // index
                               0,  // match count
                               0,  // short address mode allowed packet count
                               0,  // short address mode dropped packet count
                               1); // total mac packet count

  //---------------------------------------------------------

  unsignedCommandArgumentValue = 1;
  emberAfPluginMacAddressFilteringFilterNoAddressCommand();

  debug("\n---\nTesting filtering of no source address packets (i.e. beacon requests)\n");

  testIncomingRawMacPacket(beaconRequestData,
                           sizeof(beaconRequestData),
                           "Parsed packet for address mode NONE, filter result: DROPPED",
                           DENY_PACKET);
  packetCountCheckShortAddress(0,  // index
                               0,  // match count
                               0,  // short address mode allowed packet count
                               0,  // short address mode dropped packet count
                               2); // total mac packet count

  //---------------------------------------------------------
  // Test processing of beacons (source address info present
  // but no dest address info).  Filtering of no source address
  // packets should NOT apply and the packet should be accepted.

  debug("\n---\nTesting beacon is accepted (source address present, no dest present)\n");
  testIncomingRawMacPacket(beaconData,
                           sizeof(beaconData),
                           "Parsed packet for address mode SHORT, filter result: ALLOWED",
                           ACCEPT_PACKET);
  packetCountCheckShortAddress(0,  // index
                               0,  // match count
                               1,  // short address mode allowed packet count
                               0,  // short address mode dropped packet count
                               3); // total mac packet count

  //---------------------------------------------------------
  // Test processing of association request packet
  // Long Source Address, short dest address

  debug("\n---\nTesting Association Request (Long Src, Short Dest)\n");
  testIncomingRawMacPacket(associationRequestData,
                           sizeof(associationRequestData),
                           "Parsed packet for address mode LONG, filter result: ALLOWED",
                           ACCEPT_PACKET);
  packetCountCheckShortAddress(0,  // index
                               0,  // match count
                               1,  // short address mode allowed packet count
                               0,  // short address mode dropped packet count
                               4); // total mac packet count

  //---------------------------------------------------------
  // Test processing of association response packet
  // Long Source Address, long dest address

  debug("\n---\nTesting Association Response (Long Src, Long Dest)\n");
  testIncomingRawMacPacket(associationResponseData,
                           sizeof(associationResponseData),
                           "Parsed packet for address mode LONG, filter result: ALLOWED",
                           ACCEPT_PACKET);
  packetCountCheckShortAddress(0,  // index
                               0,  // match count
                               1,  // short address mode allowed packet count
                               0,  // short address mode dropped packet count
                               5); // total mac packet count

  //---------------------------------------------------------
  // Test Processing of broadcast MAC data message
  // Short source and short dest address

  debug("\n---\nTesting Broadcast MAC Data message (short source, short dest)\n");
  testIncomingRawMacPacket(nwkBroadcastMessageData,
                           sizeof(nwkBroadcastMessageData),
                           "Parsed packet for address mode SHORT, filter result: ALLOWED",
                           ACCEPT_PACKET);
  packetCountCheckShortAddress(0,  // index
                               0,  // match count
                               2,  // short address mode allowed packet count
                               0,  // short address mode dropped packet count
                               6); // total mac packet count

  //---------------------------------------------------------
  // Filtering
  //---------------------------------------------------------

  // Add short source address of 0x0000 to our BLACKLIST
  unsignedCommandArgumentValue = 0;
  emberAfPluginMacAddressFilteringAddShortAddressCommand();

  debug("\n---\nTesting FILTERED Broadcast MAC Data message (short source, short dest)\n");
  testIncomingRawMacPacket(nwkBroadcastMessageData,
                           sizeof(nwkBroadcastMessageData),
                           "Parsed packet for address mode SHORT, filter result: DROPPED",
                           DENY_PACKET);
  packetCountCheckShortAddress(0,  // index
                               1,  // match count
                               2,  // short address mode allowed packet count
                               1,  // short address mode dropped packet count
                               7); // total mac packet count

  //---------------------------------------------------------
  // Change the list to a whitelist.  We should now accept the packet.
  debug("\n---\nTesting ACCEPTED Broadcast MAC Data message (short source, short dest)\n");

  emberAfPluginMacAddressFilteringSetShortAddressListType();

  testIncomingRawMacPacket(nwkBroadcastMessageData,
                           sizeof(nwkBroadcastMessageData),
                           "Parsed packet for address mode SHORT, filter result: ALLOWED",
                           ACCEPT_PACKET);
  // NOTE: The stats for short addresses are RESET when changing from black to whilelist
  packetCountCheckShortAddress(0,  // index
                               1,  // match count
                               1,  // short address mode allowed packet count
                               0,  // short address mode dropped packet count
                               8); // total mac packet count

  //---------------------------------------------------------
  debug("\n---\nTesting DROPPED Broadcast MAC Data message (short source, short dest)\n");

  // Change the source address in packet.  Packet should be dropped,
  // not on whitelist
  nwkBroadcastMessageData[NWK_BROADCAST_DEST_ADDRESS_INDEX]   = 0x34;
  nwkBroadcastMessageData[NWK_BROADCAST_DEST_ADDRESS_INDEX + 1] = 0x12;
  testIncomingRawMacPacket(nwkBroadcastMessageData,
                           sizeof(nwkBroadcastMessageData),
                           "Parsed packet for address mode SHORT, filter result: DROPPED",
                           DENY_PACKET);
  packetCountCheckShortAddress(0,  // index
                               1,  // match count
                               1,  // short address mode allowed packet count
                               1,  // short address mode dropped packet count
                               9); // total mac packet count

  //---------------------------------------------------------
  debug("\n---\nTesting DROPPED Long source address on the blacklist\n");

  stringCommandArgumentPtr = blacklistArg;
  emberAfPluginMacAddressFilteringSetLongAddressListType();
  emberAfPluginMacAddressFilteringAddLongAddressCommand();
  testIncomingRawMacPacket(associationRequestData,
                           sizeof(associationRequestData),
                           "Parsed packet for address mode LONG, filter result: DROPPED",
                           DENY_PACKET);

  //---------------------------------------------------------
  debug("\n---\nTesting ACCEPTED Long source address on the whitelist\n");

  stringCommandArgumentPtr = whitelistArg;
  emberAfPluginMacAddressFilteringSetLongAddressListType();
  testIncomingRawMacPacket(associationRequestData,
                           sizeof(associationRequestData),
                           "Parsed packet for address mode LONG, filter result: ALLOWED",
                           ACCEPT_PACKET);

  //---------------------------------------------------------
  debug("\n---\nTesting DROPPED Long source address NOT on the whitelist\n");

  associationRequestData[ASSOCIATION_REQUEST_SORUCE_ADDRESS_INDEX] = 0x05;
  testIncomingRawMacPacket(associationRequestData,
                           sizeof(associationRequestData),
                           "Parsed packet for address mode LONG, filter result: DROPPED",
                           DENY_PACKET);

  //---------------------------------------------------------
  debug("\n---\nTesting DROPPED Long source address NOT on the whitelist (long src, long dest)\n");
  testIncomingRawMacPacket(associationResponseData,
                           sizeof(associationResponseData),
                           "Parsed packet for address mode LONG, filter result: DROPPED",
                           DENY_PACKET);

  //---------------------------------------------------------
  // Add the EUI64 to the whitelist and it should be accepted
  debug("\n---\nTesting ALLOWED Long source address on the whitelist (long src, long dest)\n");

  eui64StringArgumentPtr = associationResponseIeeeSourceLittleEndian;
  emberAfPluginMacAddressFilteringAddLongAddressCommand();
  testIncomingRawMacPacket(associationResponseData,
                           sizeof(associationResponseData),
                           "Parsed packet for address mode LONG, filter result: ALLOWED",
                           ACCEPT_PACKET);

  //---------------------------------------------------------
  // Add a PAN ID to the blacklist.

  debug("\n---\nTesting Blacklisting of PAN IDs: Source PAN ID.\n");

  stringCommandArgumentPtr = blacklistArg;
  unsignedCommandArgumentValue = 0x1122;  // same as the beacon data
  emberAfPluginMacAddressFilteringSetPanIdListType();
  emberAfPluginMacAddressFilteringAddPanIdCommand();
  testIncomingRawMacPacket(beaconData,
                           sizeof(beaconData),
                           "Message DROPPED due to PAN ID filtering.",
                           DENY_PACKET);

  //---------------------------------------------------------
  // Dest PAN ID and Source PAN ID Present.  We only filter on Dest PAN ID.

  debug("\n---\nTesting Blacklisting of PAN IDs: Dest PAN ID.\n");
  testIncomingRawMacPacket(associationRequestData,
                           sizeof(associationRequestData),
                           "Message DROPPED due to PAN ID filtering.",
                           DENY_PACKET);

  //---------------------------------------------------------
  // Change to a PAN ID whitelist - Source PAN ID (no dest PAN ID)

  debug("\n---\nTesting Whitelisting of PAN IDs: Source PAN ID.\n");
  stringCommandArgumentPtr = whitelistArg;
  emberAfPluginMacAddressFilteringSetPanIdListType();

  testIncomingRawMacPacket(beaconData,
                           sizeof(beaconData),
                           "Parsed packet for address mode SHORT, filter result: ALLOWED",
                           ACCEPT_PACKET);

  //---------------------------------------------------------
  // Source and Dest PAN ID.  Filter on Dest PAN ID.

  // Packet passes PAN ID whitelist, but is dropped later due to previously
  // set up long address filters.
  testIncomingRawMacPacket(associationRequestData,
                           sizeof(associationRequestData),
                           "Parsed packet for address mode LONG, filter result: DROPPED",
                           DENY_PACKET);
}

int main(int argc, char* argv[])
{
  const TestCase allTests[] = {
    { "all-tests", runAllTests },
    { NULL },
  };

  return parseCommandLineAndExecuteTest(argc,
                                        argv,
                                        "af-plugin-mac-address-filtering-test",
                                        allTests);
}
