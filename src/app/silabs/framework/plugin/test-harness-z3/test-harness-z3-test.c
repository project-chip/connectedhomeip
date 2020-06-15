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
 * @brief Unit test for ZigBee 3.0 Test Harness functionality.
 *******************************************************************************
   ******************************************************************************/

#include "app/framework/include/af.h"
#include "app/framework/security/security-config.h"

#include "app/framework/test/script/afv2-scripted.h"

#include "app/util/zigbee-framework/zigbee-device-host.h"
#include "app/util/zigbee-framework/zigbee-device-common.h"

#include "stack/mac/command.h"
#include "stack/include/zll-types.h"

#include "test-harness-z3-core.h"
#include "test-harness-z3-zll.h"

#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

// -----------------------------------------------------------------------------
// WHATEVER.

#define WHATEVER 0xAC

static void checkDeviceMode(long type)
{
  assert(type == emAfPluginTestHarnessZ3DeviceMode);
}

// -----------------------------------------------------------------------------
// NETWORK STUFF.

#define MY_NODE_ID            (0xABBA)
#define MY_PAN_ID             (0xB000)
#define MY_NETWORK_UPDATE_ID  (WHATEVER)
#define MY_NETWORK_MANAGER_ID (((WHATEVER & 0xF) << 4) | ((WHATEVER & 0xF0) >> 4))
#define MY_CHANNEL            (11)
#define MY_TRANSACTION_ID     (0x12345678)

static EmberEUI64  remoteEui64  = { 1, 2, 3, 4, 5, 6, 7, 8, };
static EmberNodeId remoteNodeId = 0xABCD;

static EmberNodeType myNodeType = EMBER_ROUTER;
EmberEUI64 emLocalEui64 = { 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, };
static uint8_t myExtPanId[] = { 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, };
static EmberEUI64 trustCenterEui64 = { 0xBB, 0xBB, 0xBB, 0xBB, 0xBB, 0xBB, 0xBB, 0xBB, };

EmberStatus emberLookupEui64ByNodeId(EmberNodeId nodeId, EmberEUI64 eui64)
{
  EmberStatus status = EMBER_SUCCESS;

  if (nodeId == remoteNodeId) {
    memcpy(eui64, remoteEui64, EUI64_SIZE);
  } else if (nodeId == MY_NODE_ID) {
    memcpy(eui64, emLocalEui64, EUI64_SIZE);
  } else if (nodeId == EMBER_TRUST_CENTER_NODE_ID) {
    memcpy(eui64, trustCenterEui64, EUI64_SIZE);
  } else {
    status = EMBER_ERR_FATAL;
  }

  return status;
}

EmberNodeId emberLookupNodeIdByEui64(EmberEUI64 eui64)
{
  if (memcmp(eui64, emLocalEui64, EUI64_SIZE) == 0) {
    return MY_NODE_ID;
  } else if (memcmp(eui64, remoteEui64, EUI64_SIZE) == 0) {
    return remoteNodeId;
  } else {
    return EMBER_NULL_NODE_ID;
  }
}

static EmberStatus getNetworkParametersReturn = EMBER_SUCCESS;

static void setGetNetworkParametersReturn(EmberStatus status)
{
  getNetworkParametersReturn = status;
}

EmberStatus emberGetNetworkParameters(EmberNetworkParameters *networkParams)
{
  EmberNodeType nodeType;
  emberAfGetNetworkParameters(&nodeType, networkParams);
  return getNetworkParametersReturn;
}

EmberApsFrame *emCurrentApsStruct;
EmberNodeId emCurrentSender;

static EmAfZigbeeProNetwork zigbeeNetwork
  = { EMBER_ROUTER, EMBER_AF_SECURITY_PROFILE_Z3 };
const EmAfZigbeeProNetwork *emAfCurrentZigbeeProNetwork = &zigbeeNetwork;

uint32_t emAfPluginNetworkSteeringPrimaryChannelMask = BIT32(17);

// -----------------------------------------------------------------------------
// CLI.

static Parcel *cliBuffer[8];
const char *lastCommandString = "n";

long setLastCommandArgument(const char *string)
{
  lastCommandString = string;
  return 0;
}

uint8_t *emberStringCommandArgument(int8_t argNum, uint8_t *length)
{
  return (uint8_t *)lastCommandString;
}

uint8_t emberCopyBigEndianEui64Argument(int8_t index, EmberEUI64 destination)
{
  uint8_t i;

  for (i = 0; i < 8; i++) {
    destination[i] = cliBuffer[index]->contents[i];
  }

  return 8;
}

uint32_t emberUnsignedCommandArgument(uint8_t argNum)
{
  uint32_t value, i;
  Parcel *argument = cliBuffer[argNum];

  for (value = i = 0; i < argument->length; i++) {
    value <<= 8;
    value |= argument->contents[i];
  }

  return value;
}

PERFORMER(cliCall)
{
  void (*function)(void)   = (void (*)(void))action->contents[0];
  const char *format       = (const char *)  action->contents[2];
  Parcel *args             = (Parcel *)      action->contents[3];
  char *c;
  int i, offset, size;
  static unsigned int totalCliParcels = 0
  ;

  // Free old CLI parcels.
  for (; totalCliParcels > 0; free(cliBuffer[--totalCliParcels])) {
    ;
  }

  for (i = 0, offset = 0, c = (char *)format; *c; i++, c++) {
    Parcel *newParcel = NULL;
    switch (*c) {
      case '1': size = 1; break;
      case '2': size = 2; break;
      case '4': size = 4; break;
      case '8': size = 8; break;
      default:  assert(0);
    }
    newParcel = makeParcel(size);
    memcpy(newParcel->contents, args->contents + offset, size);
    offset += size;
    cliBuffer[i] = newParcel;
  }
  totalCliParcels = i;

  function();
}

PRINTER(cliCall)
{
  const char *functionName = (const char *) action->contents[1];
  const char *format       = (const char *) action->contents[2];
  Parcel *args             = (Parcel *) action->contents[3];

  fprintf(stderr, " running function %s with format %s and args",
          functionName, format);
  printParcelBytes(args);
}

ACTION(cliCall, issp); // function, functionName, format, args

#define addCliCallAction(function, format, args) \
  extern void function(void);                    \
  addAction(&cliCallActionType,                  \
            (function),                          \
            #function,                           \
            (format),                            \
            (args))

// -----------------------------------------------------------------------------
// ZCL.

static uint8_t zclBuffer[64];
static uint16_t zclBufferLength;
static EmberApsFrame zclApsFrame;

// From zdo-cli.c.
uint8_t emAfCliInClCount, emAfCliOutClCount;
uint16_t emAfCliZdoInClusters[2], emAfCliZdoOutClusters[2];

static long setClCounts(uint8_t inClCount, uint8_t outClCount)
{
  emAfCliInClCount = inClCount;
  emAfCliOutClCount = outClCount;

  return 0;
}

#define ENDPOINT_COUNT (2)
#define ENDPOINT_1     (1)
#define ENDPOINT_2     (2)
uint8_t emberAfEndpointCount(void)
{
  return ENDPOINT_COUNT;
}
EmberAfDefinedEndpoint emAfEndpoints[ENDPOINT_COUNT] = {
  {
    .endpoint      = ENDPOINT_1,
    .profileId     = WHATEVER,
    .deviceId      = WHATEVER,
    .deviceVersion = WHATEVER,
    .endpointType  = NULL,
    .networkIndex  = WHATEVER,
  },
  {
    .endpoint      = ENDPOINT_2,
    .profileId     = WHATEVER,
    .deviceId      = WHATEVER,
    .deviceVersion = WHATEVER,
    .endpointType  = NULL,
    .networkIndex  = WHATEVER,
  },
};

// -----------------------------------------------------------------------------
// ZLL.

uint32_t emAfZllPrimaryChannelMask = EMBER_ZLL_PRIMARY_CHANNEL_MASK;
uint32_t emAfZllSecondaryChannelMask = EMBER_ZLL_SECONDARY_CHANNEL_MASK;

static bool checkZllChannelMasks(uint32_t primary, uint32_t secondary)
{
  if (scriptDebug) {
    fprintf(stderr, "[checking primary (0x%08X) and secondary (0x%08X) masks]",
            primary, secondary);
  }

  return (emAfZllPrimaryChannelMask == primary
          && emAfZllSecondaryChannelMask == secondary);
}

static void callScanCompleteCallbacks(void)
{
  EmberZllNetwork network;

  MEMSET(&network, 0, sizeof(EmberZllNetwork));

  network.zigbeeNetwork.panId = MY_PAN_ID;
  network.zigbeeNetwork.channel = MY_CHANNEL;

  network.securityAlgorithm.transactionId = MY_TRANSACTION_ID;

  MEMMOVE(network.eui64, remoteEui64, EUI64_SIZE);
  network.nodeId = remoteNodeId;
  network.nodeType = EMBER_ROUTER;

  emAfPluginTestHarnessZ3ZllNetworkFoundCallback(&network);
  emAfPluginTestHarnessZ3ZllScanCompleteCallback(EMBER_SUCCESS);
}

EmberStatus emberZllStartScan(uint32_t channelMask,
                              int8_t radioPowerForScan,
                              EmberNodeType nodeType)
{
  functionCallCheck("emberZllStartScan",
                    "ii",
                    channelMask,
                    nodeType);
  return EMBER_SUCCESS;
}

EmberStatus emberZllJoinTarget(const EmberZllNetwork *zllNetwork)
{
  functionCallCheck("emberZllJoinTarget",
                    "ii",
                    zllNetwork->securityAlgorithm.transactionId,
                    zllNetwork->nodeType);
  return EMBER_SUCCESS;
}

EmberStatus emberSetLogicalAndRadioChannel(uint8_t channel)
{
  return EMBER_SUCCESS;
}

static EmberTokTypeStackZllData zllToken = { 0, };

void emberZllGetTokenStackZllData(EmberTokTypeStackZllData *data)
{
  functionCallCheck("emberZllGetTokenStackZllData",
                    "");
  MEMMOVE(data, &zllToken, sizeof(EmberTokTypeStackZllData));
}

void emberZllSetTokenStackZllData(EmberTokTypeStackZllData *data)
{
  functionCallCheck("emberZllSetTokenStackZllData",
                    "iiiii",
                    data->bitmask,
                    data->freeNodeIdMin,
                    data->freeNodeIdMax,
                    data->freeGroupIdMin,
                    data->freeGroupIdMax);
  MEMMOVE(&zllToken, data, sizeof(EmberTokTypeStackZllData));
}

// -----------------------------------------------------------------------------
// INTERPAN OUT CHECK.

EmberStatus emberAfSendInterPan(EmberPanId panId,
                                const EmberEUI64 destinationLongId,
                                EmberNodeId destinationShortId,
                                EmberMulticastId multicastId,
                                EmberAfClusterId clusterId,
                                EmberAfProfileId profileId,
                                uint16_t messageLength,
                                uint8_t* messageBytes)
{
  functionCallCheck("interpanOut",
                    "ipiiip",
                    panId,
                    makeMessage("s", destinationLongId, EUI64_SIZE),
                    destinationShortId,
                    multicastId,
                    profileId,
                    makeMessage("s", messageBytes, messageLength));
  return EMBER_SUCCESS;
}

#define addInterpanOutCheck(panId,        \
                            destLongId,   \
                            destShortId,  \
                            multicastId,  \
                            profileId,    \
                            messageBytes) \
  addSimpleCheck("interpanOut",           \
                 "ipiiip",                \
                 (panId),                 \
                 (destLongId),            \
                 (destShortId),           \
                 (multicastId),           \
                 (profileId),             \
                 (messageBytes))

// -----------------------------------------------------------------------------
// APS OUT CHECK.

EmberStatus emberSendBroadcast(EmberNodeId destination,
                               EmberApsFrame *apsFrame,
                               uint8_t radius,
                               EmberMessageBuffer message)
{
  Parcel *messageParcel = bufferToParcel(message);

  functionCallCheck("apsOut",
                    "iiiiiip",
                    destination,
                    apsFrame->profileId,
                    apsFrame->clusterId,
                    apsFrame->sourceEndpoint,
                    apsFrame->destinationEndpoint,
                    ((apsFrame->options & EMBER_APS_OPTION_ENCRYPTION)
                     ? true : false),
                    messageParcel);

  free(messageParcel);

  return EMBER_SUCCESS;
}

EmberStatus emberSendUnicast(EmberOutgoingMessageType type,
                             uint16_t indexOrDestination,
                             EmberApsFrame *apsFrame,
                             EmberMessageBuffer message)
{
  Parcel *messageParcel = bufferToParcel(message);

  functionCallCheck("apsOut",
                    "iiiiiip",
                    indexOrDestination,
                    apsFrame->profileId,
                    apsFrame->clusterId,
                    apsFrame->sourceEndpoint,
                    apsFrame->destinationEndpoint,
                    ((apsFrame->options & EMBER_APS_OPTION_ENCRYPTION)
                     ? true : false),
                    messageParcel);

  free(messageParcel);

  return EMBER_SUCCESS;
}

#define addApsOutCheck(destination,         \
                       profileId,           \
                       clusterId,           \
                       sourceEndpoint,      \
                       destinationEndpoint, \
                       encryption,          \
                       message)             \
  addSimpleCheck("apsOut",                  \
                 "iiiiiip",                 \
                 (destination),             \
                 (profileId),               \
                 (clusterId),               \
                 (sourceEndpoint),          \
                 (destinationEndpoint),     \
                 (encryption),              \
                 (message))

#define addZdoOutCheck(destination,    \
                       clusterId,      \
                       message)        \
  addApsOutCheck((destination),        \
                 EMBER_ZDO_PROFILE_ID, \
                 (clusterId),          \
                 EMBER_ZDO_ENDPOINT,   \
                 EMBER_ZDO_ENDPOINT,   \
                 false,                \
                 (message))

// -----------------------------------------------------------------------------
// RAW OUT CHECK.

EmberStatus emberSendRawMessage(EmberMessageBuffer message)
{
  Parcel *messageParcel = bufferToParcel(message);

  functionCallCheck("rawMessageOut",
                    "p",
                    messageParcel);

  free(messageParcel);

  return EMBER_SUCCESS;
}

#define addRawMessageOutCheck(message) \
  addSimpleCheck("rawMessageOut", "p", (message))

// -----------------------------------------------------------------------------
// NETWORK OUT CHECK.

bool emNetworkSendCommand(EmberNodeId destination,
                          uint8_t *commandFrame,
                          uint8_t length,
                          bool tryToInsertLongDest,
                          EmberEUI64 destinationEui)
{
  functionCallCheck("networkOut",
                    "pii",
                    makeMessage("s", commandFrame, length),
                    destination,
                    tryToInsertLongDest);

  return true;
}

#define addNetworkOutCheck(data, dstShort, tryToInsertLongDest) \
  addSimpleCheck("networkOut",                                  \
                 "pii",                                         \
                 (data),                                        \
                 (dstShort),                                    \
                 (tryToInsertLongDest))

// -----------------------------------------------------------------------------
// NETWORK OUT CHECK.

bool emSendApsCommand(EmberNodeId destination,
                      EmberEUI64 longDestination,
                      EmberMessageBuffer payload,
                      uint8_t options)
{
  functionCallCheck("emSendApsCommand",
                    "ippi",
                    destination,
                    makeMessage("s", longDestination, EUI64_SIZE),
                    bufferToParcel(payload),
                    options);

  return true;
}

#define addApsCommandOutCheck(destShort, destLong, payload, options) \
  addSimpleCheck("emSendApsCommand",                                 \
                 "ippi",                                             \
                 (destShort),                                        \
                 (destLong),                                         \
                 (payload),                                          \
                 (options))

// -----------------------------------------------------------------------------
// SET KEY CHECK.

EmberStatus emberAddOrUpdateKeyTableEntry(EmberEUI64 address,
                                          bool linkKey,
                                          EmberKeyData* keyData)
{
  functionCallCheck("emberAddOrUpdateKeyTableEntry",
                    "pp",
                    makeMessage("s", address, EUI64_SIZE),
                    makeMessage("s", emberKeyContents(keyData), EMBER_ENCRYPTION_KEY_SIZE));
  return EMBER_SUCCESS;
}

EmberStatus emSetKeyTableEntry(bool erase,
                               uint8_t index,
                               EmberEUI64 address,
                               EmberKeyData* keyData)
{
  return EMBER_SUCCESS;
}

#define addSetKeyCheck(partner, keyData)          \
  addSimpleCheck("emberAddOrUpdateKeyTableEntry", \
                 "pp",                            \
                 partner,                         \
                 keyData)

// -----------------------------------------------------------------------------
// assorted commands tests

static void macCommandsTest(void)
{
  setFreeBufferBenchmark();

  // plugin test-harness z3 beacon beacon-req
  addCliCallAction(emAfPluginTestHarnessZ3BeaconBeaconReqCommand,
                   "",
                   makeParcel(0));
  addSimpleCheck("emberStartScan",
                 "ii",
                 EMBER_ACTIVE_SCAN,
                 BIT32(MY_CHANNEL));

  addSimpleAction("setGetNetworkParametersReturn(0x%02X)",
                  setGetNetworkParametersReturn,
                  1,
                  EMBER_ERR_FATAL);
  addCliCallAction(emAfPluginTestHarnessZ3BeaconBeaconReqCommand,
                   "",
                   makeParcel(0));
  addSimpleCheck("emberStartScan",
                 "ii",
                 EMBER_ACTIVE_SCAN,
                 emAfPluginNetworkSteeringPrimaryChannelMask);

  addSimpleAction("setGetNetworkParametersReturn(0x%02X)",
                  setGetNetworkParametersReturn,
                  1,
                  EMBER_SUCCESS);

  // plugin test-harness z3 beacon beacon-config <options:4>
  addCliCallAction(emAfPluginTestHarnessZ3BeaconBeaconsConfigCommand,
                   "1",
                   makeMessage("1", 0));
  addRawMessageOutCheck(makeMessage("<21<2<2<211111p1111",
                                    0x8000,     // frame control
                                    0x00,       // sequence (set by stack API)
                                    MY_PAN_ID,  // dst pan id
                                    MY_NODE_ID, // src node id
                                    0xCFFF,     // superframe junk
                                    0x00,       // gts junk
                                    0x00,       // pending address junk
                                    0x00,       // zigbee protocol id
                                    0x22,       // more procotol stuff
                                    0x84,       // capacity bits
                                    makeMessage("s",
                                                myExtPanId,
                                                EXTENDED_PAN_ID_SIZE),
                                    0xFF, 0xFF, 0xFF,       // tx offset
                                    MY_NETWORK_UPDATE_ID)); // network update id

  addBufferTestAction(0);
  runScript();
}

static void nwkCommandsTest(void)
{
  setFreeBufferBenchmark();

  // plugin test-harness z3 nwk nwk-rejoin-request <dstShort:2>
  addCliCallAction(emAfPluginTestHarnessZ3NwkNwkRejoinRequestCommand,
                   "2",
                   makeMessage("2", remoteNodeId));
  addNetworkOutCheck(makeMessage("11", 0x06, BIT(1) | BIT(2) | BIT(3) | BIT(7)),
                     remoteNodeId,
                     true);

  // plugin test-harness z3 nwk nwk-rejoin-response <networkAddress:2>
  // <rejoinStatus:1> <dstShort:2>
  addCliCallAction(emAfPluginTestHarnessZ3NwkNwkRejoinResponseCommand,
                   "212",
                   makeMessage("212", 0x1234, WHATEVER, remoteNodeId));
  addNetworkOutCheck(makeMessage("1<21", 0x07, 0x1234, WHATEVER),
                     remoteNodeId,
                     true);

  // plugin test-harness z3 nwk nwk-leave <rejoin:1> <request:1>
  // <removeChildren:1> <dstShort:2> <options:4>
  addCliCallAction(emAfPluginTestHarnessZ3NwkNwkLeaveCommand,
                   "11124",
                   makeMessage("11124", true, false, false, remoteNodeId, 0));
  addNetworkOutCheck(makeMessage("11", 0x04, BIT(5)),
                     remoteNodeId,
                     true);

  addCliCallAction(emAfPluginTestHarnessZ3NwkNwkLeaveCommand,
                   "11124",
                   makeMessage("11124", false, true, false, remoteNodeId, BIT32(0)));
  addNetworkOutCheck(makeMessage("11", 0x04, BIT(6)),
                     remoteNodeId,
                     false);

  addBufferTestAction(0);
  runScript();
}

static void apsCommandsTest(void)
{
  EmberKeyData zigbeeAlliance09 = ZIGBEE_PROFILE_INTEROPERABILITY_LINK_KEY;

  setFreeBufferBenchmark();

  // plugin test-harness z3 aps aps-remove-device <parentLong:8> <dstLong:8> <options:4>
  addCliCallAction(emAfPluginTestHarnessZ3ApsApsRemoveDevice,
                   "881",
                   makeMessage("ss",
                               remoteEui64, // we pretend that the remote device
                               EUI64_SIZE,  // is a router so that we can just
                               remoteEui64, // send the command straight to them
                               EUI64_SIZE,
                               0));
  addSimpleCheck("emberSendRemoveDevice",
                 "ipp",
                 remoteNodeId,
                 makeMessage("s", remoteEui64, EUI64_SIZE),
                 makeMessage("s", remoteEui64, EUI64_SIZE));

  addCliCallAction(emAfPluginTestHarnessZ3ApsApsRemoveDevice,
                   "881",
                   makeMessage("ss1",
                               remoteEui64, // we pretend that the remote device
                               EUI64_SIZE,  // is a router so that we can just
                               remoteEui64, // send the command straight to them
                               EUI64_SIZE,
                               BIT32(0))); // aps encryption uses ZigBeeAlliance09
  addSetKeyCheck(makeMessage("s",
                             remoteEui64,
                             EUI64_SIZE),
                 makeMessage("s",
                             emberKeyContents(&zigbeeAlliance09),
                             EMBER_ENCRYPTION_KEY_SIZE));
  addSimpleCheck("emberSendRemoveDevice",
                 "ipp",
                 remoteNodeId,
                 makeMessage("s", remoteEui64, EUI64_SIZE),
                 makeMessage("s", remoteEui64, EUI64_SIZE));

  // plugin test-harness z3 aps aps-request-key <dstShort:2> <keyType:1>
  // <parentLong:8> <options:4>
  addCliCallAction(emAfPluginTestHarnessZ3ApsApsRequestKeyCommand,
                   "2184",
                   makeMessage("21s4",
                               remoteNodeId,
                               0x04, // trust center link key type
                               remoteEui64,
                               EUI64_SIZE,
                               0));
  addApsCommandOutCheck(remoteNodeId,
                        makeMessage("s", trustCenterEui64, EUI64_SIZE),
                        makeMessage("11",
                                    0x08,  // aps request key command
                                    0x04), // trust center link key type
                        0x01 | 0x02); // nwk + aps encryption

  addCliCallAction(emAfPluginTestHarnessZ3ApsApsRequestKeyCommand,
                   "2184",
                   makeMessage("21s4",
                               remoteNodeId,
                               0x02, // app link key type
                               remoteEui64,
                               EUI64_SIZE,
                               0));
  addApsCommandOutCheck(remoteNodeId,
                        makeMessage("s", trustCenterEui64, EUI64_SIZE),
                        makeMessage("11s",
                                    0x08,  // aps request key command
                                    0x02,  // app link key type
                                    remoteEui64,
                                    EUI64_SIZE),
                        0x01 | 0x02); // nwk + aps encryption

  addCliCallAction(emAfPluginTestHarnessZ3ApsApsRequestKeyCommand,
                   "2184",
                   makeMessage("21s4",
                               remoteNodeId,
                               0x02, // app link key type
                               remoteEui64,
                               EUI64_SIZE,
                               BIT32(0))); // no partner address
  addApsCommandOutCheck(remoteNodeId,
                        makeMessage("s", trustCenterEui64, EUI64_SIZE),
                        makeMessage("11",
                                    0x08,  // aps request key command
                                    0x02), // app link key type
                        0x01 | 0x02); // nwk + aps encryption

  addCliCallAction(emAfPluginTestHarnessZ3ApsApsRequestKeyCommand,
                   "2184",
                   makeMessage("21s4",
                               remoteNodeId,
                               0x04, // trust center link key type
                               remoteEui64,
                               EUI64_SIZE,
                               BIT32(6))); // no encryption
  addApsCommandOutCheck(remoteNodeId,
                        makeMessage("s", trustCenterEui64, EUI64_SIZE),
                        makeMessage("11",
                                    0x08,  // aps request key command
                                    0x04), // trust center link key type
                        0x00); // no encryption

  addCliCallAction(emAfPluginTestHarnessZ3ApsApsRequestKeyCommand,
                   "2184",
                   makeMessage("21s4",
                               remoteNodeId,
                               0x04, // trust center link key type
                               remoteEui64,
                               EUI64_SIZE,
                               BIT32(7))); // key type is 0x01 (network key)
  addApsCommandOutCheck(remoteNodeId,
                        makeMessage("s", trustCenterEui64, EUI64_SIZE),
                        makeMessage("11",
                                    0x08,  // aps request key command
                                    0x01), // network key type
                        0x01 | 0x02); // nwk + aps encryption

  addCliCallAction(emAfPluginTestHarnessZ3ApsApsRequestKeyCommand,
                   "2184",
                   makeMessage("21s4",
                               remoteNodeId,
                               0x04, // trust center link key type
                               remoteEui64,
                               EUI64_SIZE,
                               BIT32(8))); // key type is 0x02 (app link key)
  addApsCommandOutCheck(remoteNodeId,
                        makeMessage("s", trustCenterEui64, EUI64_SIZE),
                        makeMessage("11s",
                                    0x08,  // aps request key command
                                    0x02,  // app link key type
                                    remoteEui64,
                                    EUI64_SIZE),
                        0x01 | 0x02); // nwk + aps encryption

  addCliCallAction(emAfPluginTestHarnessZ3ApsApsRequestKeyCommand,
                   "2184",
                   makeMessage("21s4",
                               remoteNodeId,
                               0x04, // trust center link key type
                               remoteEui64,
                               EUI64_SIZE,
                               BIT32(11))); // key type is 0x04 and key is dtclk
  addSetKeyCheck(makeMessage("s",
                             trustCenterEui64,
                             EUI64_SIZE),
                 makeMessage("s",
                             emberKeyContents(&zigbeeAlliance09),
                             EMBER_ENCRYPTION_KEY_SIZE));
  addApsCommandOutCheck(remoteNodeId,
                        makeMessage("s", trustCenterEui64, EUI64_SIZE),
                        makeMessage("11",
                                    0x08,  // aps request key command
                                    0x04), // app link key type
                        0x01 | 0x02); // nwk + aps encryption

  addBufferTestAction(0);
  runScript();
}

static void zllCommandsTest(void)
{
  setFreeBufferBenchmark();

  addCliCallAction(emAfPluginTestHarnessZ3SetDeviceModeCommand,
                   "1",
                   makeMessage("1",
                               EM_AF_PLUGIN_TEST_HARNESS_Z3_DEVICE_MODE_ZR_ADDRESS_ASSIGNABLE));
  addSimpleAction("checkDeviceMode(0x%02X)",
                  checkDeviceMode,
                  1,
                  EM_AF_PLUGIN_TEST_HARNESS_Z3_DEVICE_MODE_ZR_ADDRESS_ASSIGNABLE);

  // plugin test-harness z3 set-device-mode <mode:1>
  // plugin test-harness z3 touchlink scan-request-process <linkInitiator:1>
  // <unused:1> <options:4>
  addCliCallAction(emAfPluginTestHarnessZ3TouchlinkScanRequestProcessCommand,
                   "114",
                   makeMessage("114", 0, 0, BIT(3)));
  addSimpleCheck("emberZllSetPolicy",
                 "i",
                 EMBER_ZLL_POLICY_DISABLED);

  addCliCallAction(emAfPluginTestHarnessZ3SetDeviceModeCommand,
                   "1",
                   makeMessage("1",
                               EM_AF_PLUGIN_TEST_HARNESS_Z3_DEVICE_MODE_ZR_NOT_ADDRESS_ASSIGNABLE));
  addSimpleAction("checkDeviceMode(0x%02X)",
                  checkDeviceMode,
                  1,
                  EM_AF_PLUGIN_TEST_HARNESS_Z3_DEVICE_MODE_ZR_NOT_ADDRESS_ASSIGNABLE);
  addCliCallAction(emAfPluginTestHarnessZ3TouchlinkScanRequestProcessCommand,
                   "114",
                   makeMessage("114", 0, 0, 0));
  addSimpleCheck("emberZllSetPolicy",
                 "i",
                 EMBER_ZLL_POLICY_ENABLED);
  addSimpleCheck("emberZllGetTokenStackZllData",
                 "");
  addSimpleCheck("emberZllSetTokenStackZllData",
                 "iiiii",
                 zllToken.bitmask & ~EMBER_ZLL_STATE_ADDRESS_ASSIGNMENT_CAPABLE,
                 zllToken.freeNodeIdMin,
                 zllToken.freeNodeIdMin,
                 zllToken.freeGroupIdMin,
                 zllToken.freeGroupIdMax);
  addSimpleCheck("emberZllStartScan",
                 "ii",
                 emAfZllPrimaryChannelMask | emAfZllSecondaryChannelMask,
                 EMBER_ROUTER);
  addFunctionCheck("checkZllChannelMasks(0x%08X, 0x%08X)",
                   true,
                   checkZllChannelMasks,
                   2,
                   EMBER_ZLL_PRIMARY_CHANNEL_MASK,
                   EMBER_ZLL_SECONDARY_CHANNEL_MASK);

  addCliCallAction(emAfPluginTestHarnessZ3SetDeviceModeCommand,
                   "1",
                   makeMessage("1",
                               EM_AF_PLUGIN_TEST_HARNESS_Z3_DEVICE_MODE_ZR_ADDRESS_ASSIGNABLE));
  addCliCallAction(emAfPluginTestHarnessZ3TouchlinkScanRequestProcessCommand,
                   "114",
                   makeMessage("114", 0, 0, BIT32(1))); // scan current channel
  addSimpleCheck("emberZllSetPolicy",
                 "i",
                 EMBER_ZLL_POLICY_ENABLED);
  addSimpleCheck("emberZllStartScan",
                 "ii",
                 BIT32(MY_CHANNEL),
                 EMBER_ROUTER);
  addFunctionCheck("checkZllChannelMasks(0x%08X, 0x%08X)",
                   true,
                   checkZllChannelMasks,
                   2,
                   BIT32(MY_CHANNEL), // primary channel mask
                   0);                // secondary channel mask

  addCliCallAction(emAfPluginTestHarnessZ3TouchlinkScanRequestProcessCommand,
                   "114",
                   makeMessage("114", 0, 0, BIT32(4))); // secondary mask
  addSimpleCheck("emberZllSetPolicy",
                 "i",
                 EMBER_ZLL_POLICY_ENABLED);
  addSimpleCheck("emberZllStartScan",
                 "ii",
                 EMBER_ZLL_SECONDARY_CHANNEL_MASK,
                 EMBER_ROUTER);
  addFunctionCheck("checkZllChannelMasks(0x%08X, 0x%08X)",
                   true,
                   checkZllChannelMasks,
                   2,
                   0,                                 // primary channel mask
                   EMBER_ZLL_SECONDARY_CHANNEL_MASK); // secondary channel mask

  addCliCallAction(emAfPluginTestHarnessZ3SetDeviceModeCommand,
                   "1",
                   makeMessage("1",
                               EM_AF_PLUGIN_TEST_HARNESS_Z3_DEVICE_MODE_ZED_ADDRESS_ASSIGNABLE));

  // Right now, we are under the assumption that the other ZLL commands
  // cannot be called unless we have found a target in the scan request
  // process above.
  addCliCallAction(emAfPluginTestHarnessZ3TouchlinkDeviceInformationRequestCommand,
                   "14",
                   makeMessage("14", 0x00, 0));
  addSimpleAction("callScanCompleteCallbacks",
                  callScanCompleteCallbacks,
                  0);

  // plugin test-harness z3 touchlink device-information-request
  // <startIndex:1> <options:4>
  addCliCallAction(emAfPluginTestHarnessZ3TouchlinkDeviceInformationRequestCommand,
                   "14",
                   makeMessage("14", 0x00, 0));
  addInterpanOutCheck(0xFFFF, // dst pan id
                      makeMessage("s", remoteEui64, EUI64_SIZE),
                      EMBER_NULL_NODE_ID, // node id - ignored
                      0x0000,             // group id - ignored
                      EMBER_ZLL_PROFILE_ID,
                      makeMessage("111<41",
                                  EM_AF_PLUGIN_TEST_HARNESS_Z3_ZLL_CLIENT_TO_SERVER_FRAME_CONTROL,
                                  0x00, // zcl sequence number
                                  ZCL_DEVICE_INFORMATION_REQUEST_COMMAND_ID,
                                  MY_TRANSACTION_ID,
                                  0x00));// start index

  addCliCallAction(emAfPluginTestHarnessZ3TouchlinkDeviceInformationRequestCommand,
                   "14",
                   makeMessage("14", 0x00, BIT(0)));
  addInterpanOutCheck(0xFFFF, // dst pan id
                      makeMessage("s", remoteEui64, EUI64_SIZE),
                      EMBER_NULL_NODE_ID, // node id - ignored
                      0x0000,             // group id - ignored
                      EMBER_ZLL_PROFILE_ID,
                      makeMessage("111<41",
                                  EM_AF_PLUGIN_TEST_HARNESS_Z3_ZLL_CLIENT_TO_SERVER_FRAME_CONTROL,
                                  0x00, // zcl sequence number
                                  ZCL_DEVICE_INFORMATION_REQUEST_COMMAND_ID,
                                  MY_TRANSACTION_ID - 1,
                                  0x00));// start index

  addCliCallAction(emAfPluginTestHarnessZ3TouchlinkDeviceInformationRequestCommand,
                   "14",
                   makeMessage("14", 0xDD, 0x00));
  addInterpanOutCheck(0xFFFF, // dst pan id
                      makeMessage("s", remoteEui64, EUI64_SIZE),
                      EMBER_NULL_NODE_ID, // node id - ignored
                      0x0000,             // group id - ignored
                      EMBER_ZLL_PROFILE_ID,
                      makeMessage("111<41",
                                  EM_AF_PLUGIN_TEST_HARNESS_Z3_ZLL_CLIENT_TO_SERVER_FRAME_CONTROL,
                                  0x00, // zcl sequence number
                                  ZCL_DEVICE_INFORMATION_REQUEST_COMMAND_ID,
                                  MY_TRANSACTION_ID,
                                  0xDD));// start index

  // plugin test-harness z3 touchlink identify-request <duration:2> <options:4>
  addCliCallAction(emAfPluginTestHarnessZ3TouchlinkIdentifyRequestCommand,
                   "24",
                   makeMessage("24", 0x4321, 0));
  addInterpanOutCheck(0xFFFF, // dst pan id
                      makeMessage("s", remoteEui64, EUI64_SIZE),
                      EMBER_NULL_NODE_ID, // node id - ignored
                      0x0000,             // group id - ignored
                      EMBER_ZLL_PROFILE_ID,
                      makeMessage("111<4<2",
                                  EM_AF_PLUGIN_TEST_HARNESS_Z3_ZLL_CLIENT_TO_SERVER_FRAME_CONTROL,
                                  0x00, // zcl sequence number
                                  ZCL_IDENTIFY_REQUEST_COMMAND_ID,
                                  MY_TRANSACTION_ID,
                                  0x4321)); // identify duration

  addCliCallAction(emAfPluginTestHarnessZ3TouchlinkIdentifyRequestCommand,
                   "24",
                   makeMessage("24", 0x4321, BIT(0)));
  addInterpanOutCheck(0xFFFF, // dst pan id
                      makeMessage("s", remoteEui64, EUI64_SIZE),
                      EMBER_NULL_NODE_ID, // node id - ignored
                      0x0000,             // group id - ignored
                      EMBER_ZLL_PROFILE_ID,
                      makeMessage("111<4<2",
                                  EM_AF_PLUGIN_TEST_HARNESS_Z3_ZLL_CLIENT_TO_SERVER_FRAME_CONTROL,
                                  0x00, // zcl sequence number
                                  ZCL_IDENTIFY_REQUEST_COMMAND_ID,
                                  MY_TRANSACTION_ID - 1,
                                  0x4321)); // identify duration

  addCliCallAction(emAfPluginTestHarnessZ3TouchlinkIdentifyRequestCommand,
                   "24",
                   makeMessage("24", 0xDCBA, 0));
  addInterpanOutCheck(0xFFFF, // dst pan id
                      makeMessage("s", remoteEui64, EUI64_SIZE),
                      EMBER_NULL_NODE_ID, // node id - ignored
                      0x0000,             // group id - ignored
                      EMBER_ZLL_PROFILE_ID,
                      makeMessage("111<4<2",
                                  EM_AF_PLUGIN_TEST_HARNESS_Z3_ZLL_CLIENT_TO_SERVER_FRAME_CONTROL,
                                  0x00, // zcl sequence number
                                  ZCL_IDENTIFY_REQUEST_COMMAND_ID,
                                  MY_TRANSACTION_ID,
                                  0xDCBA)); // identify duration

  // plugin test-harness z3 touchlink reset-to-factory-new-request <options:4>
  addCliCallAction(emAfPluginTestHarnessZ3TouchlinkRTFNRequestCommand,
                   "4",
                   makeMessage("4", 0));
  addInterpanOutCheck(0xFFFF, // dst pan id
                      makeMessage("s", remoteEui64, EUI64_SIZE),
                      EMBER_NULL_NODE_ID, // node id - ignored
                      0x0000,             // group id - ignored
                      EMBER_ZLL_PROFILE_ID,
                      makeMessage("111<4",
                                  EM_AF_PLUGIN_TEST_HARNESS_Z3_ZLL_CLIENT_TO_SERVER_FRAME_CONTROL,
                                  0x00, // zcl sequence number
                                  ZCL_RESET_TO_FACTORY_NEW_REQUEST_COMMAND_ID,
                                  MY_TRANSACTION_ID));

  // plugin test-harness z3 touchlink network-start-request <dstAddress:2>
  // <freeAddrBegin:2> <freeAddrEnd:2> <groupIdBegin:2> <groupIdEnd:2> <options:4>
  addSimpleAction("setLastCommandArgument(%s)",
                  setLastCommandArgument,
                  1,
                  "network-start-request");
  addCliCallAction(emAfPluginTestHarnessZ3TouchlinkNetworkCommand,
                   "222224",
                   makeMessage("222224",
                               remoteNodeId,
                               0x0010,
                               0x0012,
                               0x1000,
                               0x1200,
                               0));
  addSimpleCheck("emberZllGetTokenStackZllData",
                 "");
  addSimpleCheck("emberZllSetTokenStackZllData",
                 "iiiii",
                 zllToken.bitmask | EMBER_ZLL_STATE_FACTORY_NEW,
                 0x0010,
                 0x0012,
                 0x1000,
                 0x1200);
  addSimpleCheck("emberZllJoinTarget",
                 "ii",
                 MY_TRANSACTION_ID,
                 EMBER_ROUTER);

  // plugin test-harness z3 touchlink network-join-router-request <dstAddress:2>
  // <freeAddrBegin:2> <freeAddrEnd:2> <groupIdBegin:2> <groupIdEnd:2> <options:4>
  addSimpleAction("setLastCommandArgument(%s)",
                  setLastCommandArgument,
                  1,
                  "network-join-router-request");
  addCliCallAction(emAfPluginTestHarnessZ3TouchlinkNetworkCommand,
                   "222224",
                   makeMessage("222224",
                               remoteNodeId,
                               0x0010,
                               0x0012,
                               0x1000,
                               0x1200,
                               0));
  addSimpleCheck("emberZllGetTokenStackZllData",
                 "");
  addSimpleCheck("emberZllSetTokenStackZllData",
                 "iiiii",
                 zllToken.bitmask & ~EMBER_ZLL_STATE_FACTORY_NEW,
                 0x0010,
                 0x0012,
                 0x1000,
                 0x1200);
  addSimpleCheck("emberZllJoinTarget",
                 "ii",
                 MY_TRANSACTION_ID,
                 EMBER_ROUTER);

  addCliCallAction(emAfPluginTestHarnessZ3TouchlinkNetworkCommand,
                   "222224",
                   makeMessage("222224",
                               remoteNodeId,
                               0x0010,
                               0x0012,
                               0x1000,
                               0x1200,
                               BIT32(3)));
  addSimpleCheck("emberZllGetTokenStackZllData",
                 "");
  addSimpleCheck("emberZllSetTokenStackZllData",
                 "iiiii",
                 zllToken.bitmask & ~EMBER_ZLL_STATE_FACTORY_NEW,
                 0x0010,
                 0x0012,
                 0x1000,
                 0x1200);
  addSimpleCheck("emberZllJoinTarget",
                 "ii",
                 MY_TRANSACTION_ID - 1,
                 EMBER_ROUTER);

  // plugin test-harness z3 touchlink network-join-end-device-request <dstAddress:2>
  // <freeAddrBegin:2> <freeAddrEnd:2> <groupIdBegin:2> <groupIdEnd:2> <options:4>
  addSimpleAction("setLastCommandArgument(%s)",
                  setLastCommandArgument,
                  1,
                  "network-join-end-device-request");
  addCliCallAction(emAfPluginTestHarnessZ3TouchlinkNetworkCommand,
                   "222224",
                   makeMessage("222224",
                               remoteNodeId,
                               0x0010,
                               0x0012,
                               0x1000,
                               0x1200,
                               0));
  addSimpleCheck("emberZllGetTokenStackZllData",
                 "");
  addSimpleCheck("emberZllSetTokenStackZllData",
                 "iiiii",
                 zllToken.bitmask & ~EMBER_ZLL_STATE_FACTORY_NEW,
                 0x0010,
                 0x0012,
                 0x1000,
                 0x1200);
  addSimpleCheck("emberZllJoinTarget",
                 "ii",
                 MY_TRANSACTION_ID,
                 EMBER_END_DEVICE);

  addCliCallAction(emAfPluginTestHarnessZ3TouchlinkNetworkCommand,
                   "222224",
                   makeMessage("222224",
                               remoteNodeId,
                               0x000A,
                               0x000B,
                               0x000C,
                               0x000D,
                               BIT32(3)));
  addSimpleCheck("emberZllGetTokenStackZllData",
                 "");
  addSimpleCheck("emberZllSetTokenStackZllData",
                 "iiiii",
                 zllToken.bitmask & ~EMBER_ZLL_STATE_FACTORY_NEW,
                 0x000A,
                 0x000B,
                 0x000C,
                 0x000D);
  addSimpleCheck("emberZllJoinTarget",
                 "ii",
                 MY_TRANSACTION_ID - 1,
                 EMBER_END_DEVICE);

  // plugin test-harness z3 touchlink network-update-request <options:4>
  addCliCallAction(emAfPluginTestHarnessZ3TouchlinkNetworkUpdateRequestCommand,
                   "4",
                   makeMessage("4", 0));
  addInterpanOutCheck(0xFFFF, // dst pan id
                      makeMessage("s", remoteEui64, EUI64_SIZE),
                      EMBER_NULL_NODE_ID, // node id - ignored
                      0x0000,             // group id - ignored
                      EMBER_ZLL_PROFILE_ID,
                      makeMessage("111<4p11<2<2",
                                  EM_AF_PLUGIN_TEST_HARNESS_Z3_ZLL_CLIENT_TO_SERVER_FRAME_CONTROL,
                                  0x00, // zcl sequence number
                                  ZCL_NETWORK_UPDATE_REQUEST_COMMAND_ID,
                                  MY_TRANSACTION_ID,
                                  makeMessage("s", myExtPanId, EXTENDED_PAN_ID_SIZE),
                                  MY_NETWORK_UPDATE_ID,
                                  MY_CHANNEL,
                                  MY_PAN_ID,
                                  MY_NODE_ID));

  addCliCallAction(emAfPluginTestHarnessZ3TouchlinkNetworkUpdateRequestCommand,
                   "4",
                   makeMessage("4", BIT32(0)));
  addInterpanOutCheck(0xFFFF, // dst pan id
                      makeMessage("s", remoteEui64, EUI64_SIZE),
                      EMBER_NULL_NODE_ID, // node id - ignored
                      0x0000,             // group id - ignored
                      EMBER_ZLL_PROFILE_ID,
                      makeMessage("111<4p11<2<2",
                                  EM_AF_PLUGIN_TEST_HARNESS_Z3_ZLL_CLIENT_TO_SERVER_FRAME_CONTROL,
                                  0x00, // zcl sequence number
                                  ZCL_NETWORK_UPDATE_REQUEST_COMMAND_ID,
                                  MY_TRANSACTION_ID,
                                  makeMessage("s", myExtPanId, EXTENDED_PAN_ID_SIZE),
                                  MY_NETWORK_UPDATE_ID + 5,
                                  MY_CHANNEL,
                                  MY_PAN_ID,
                                  MY_NODE_ID));

  addBufferTestAction(0);
  runScript();
}

static void zdoCommandsTest(void)
{
  setFreeBufferBenchmark();

  // plugin test-harness z3 mgmt permit-joining-req <timeS:2> <dstShort:2>
  addCliCallAction(emAfPluginTestHarnessZ3MgmtPermitJoiningReqCommand,
                   "224",
                   makeMessage("224", 123, remoteNodeId, 0));
  addSimpleCheck("emberPermitJoiningRequest",
                 "iii",
                 remoteNodeId,
                 0x01, // TC_Significance
                 123);
  addCliCallAction(emAfPluginTestHarnessZ3MgmtPermitJoiningReqCommand,
                   "224",
                   makeMessage("224", 123, remoteNodeId, BIT32(0)));
  addSimpleCheck("emberPermitJoiningRequest",
                 "iii",
                 remoteNodeId,
                 0x00, // TC_Significance
                 123);

  // plugin test-harness z3 mgmt leave <dstShort:2> <removeChildren:1>
  // <rejoin:1> <optionBitmask:4>
  addCliCallAction(emAfPluginTestHarnessZ3MgmtLeaveCommand,
                   "2114",
                   makeMessage("2114", remoteNodeId, true, false, 0));
  addZdoOutCheck(remoteNodeId,
                 LEAVE_REQUEST,
                 makeMessage("1s1",
                             emberNextZigDevRequestSequence(), // cheating...
                             remoteEui64,
                             EUI64_SIZE,
                             BIT(6)));

  addCliCallAction(emAfPluginTestHarnessZ3MgmtLeaveCommand,
                   "2114",
                   makeMessage("2114", remoteNodeId, true, false, BIT(0)));
  addZdoOutCheck(remoteNodeId,
                 LEAVE_REQUEST,
                 makeMessage("11",
                             emberNextZigDevRequestSequence(), // cheating...
                             BIT(6)));

  addCliCallAction(emAfPluginTestHarnessZ3MgmtLeaveCommand,
                   "2114",
                   makeMessage("2114", remoteNodeId, true, false, BIT(1)));
  addZdoOutCheck(remoteNodeId,
                 LEAVE_REQUEST,
                 makeMessage("1s",
                             emberNextZigDevRequestSequence(), // cheating...
                             remoteEui64,
                             EUI64_SIZE));

  // plugin test-harness z3 nwk-update-request <scanChannel:2> <scanDuration:2>
  // <scanCount:1> <dstShort:2> <option:4>
  addCliCallAction(emAfPluginTestHarnessZ3MgmtNwkUpdateRequestCommand,
                   "22124",
                   makeMessage("22124",
                               11,
                               5,
                               3,
                               remoteNodeId,
                               0));
  addZdoOutCheck(remoteNodeId,
                 0x0038, // network update cluster
                 makeMessage("1<411",
                             emberNextZigDevRequestSequence(), // cheating...
                             BIT32(11),
                             5,
                             3));

  addCliCallAction(emAfPluginTestHarnessZ3MgmtNwkUpdateRequestCommand,
                   "22124",
                   makeMessage("22124",
                               11,
                               0xFF,
                               3,
                               remoteNodeId,
                               0));
  addZdoOutCheck(remoteNodeId,
                 0x0038, // network update cluster
                 makeMessage("1<4111",
                             emberNextZigDevRequestSequence(), // cheating...
                             BIT32(11),
                             0xFF,
                             MY_NETWORK_UPDATE_ID,
                             MY_NETWORK_MANAGER_ID));

  addCliCallAction(emAfPluginTestHarnessZ3MgmtNwkUpdateRequestCommand,
                   "22124",
                   makeMessage("22124",
                               11,
                               0xFE,
                               3,
                               remoteNodeId,
                               BIT32(0)));
  addZdoOutCheck(remoteNodeId,
                 0x0038, // network update cluster
                 makeMessage("1<411",
                             emberNextZigDevRequestSequence(), // cheating...
                             BIT32(11),
                             0xFE,
                             MY_NETWORK_UPDATE_ID - 1));

  addCliCallAction(emAfPluginTestHarnessZ3MgmtNwkUpdateRequestCommand,
                   "22124",
                   makeMessage("22124",
                               11,
                               0xFE,
                               3,
                               remoteNodeId,
                               BIT32(1)));
  addZdoOutCheck(remoteNodeId,
                 0x0038, // network update cluster
                 makeMessage("1<411",
                             emberNextZigDevRequestSequence(), // cheating...
                             BIT32(11),
                             0xFE,
                             MY_NETWORK_UPDATE_ID + 1));

  // plugin test-harness z3 zdo active-endpoint-request <dstShort:2>
  // <nwkAddrOfInterest:2> <options:4>
  addCliCallAction(emAfPluginTestHarnessZ3ZdoActiveEndpointRequestCommand,
                   "224",
                   makeMessage("224", remoteNodeId, 0x1234, 0));
  addZdoOutCheck(remoteNodeId,
                 ACTIVE_ENDPOINTS_REQUEST,
                 makeMessage("1<2",
                             emberNextZigDevRequestSequence(), // cheating...
                             0x1234));

  addCliCallAction(emAfPluginTestHarnessZ3ZdoActiveEndpointRequestCommand,
                   "224",
                   makeMessage("224", remoteNodeId, 0x1234, BIT(0)));
  addZdoOutCheck(remoteNodeId,
                 ACTIVE_ENDPOINTS_REQUEST,
                 makeMessage("1",
                             emberNextZigDevRequestSequence())); // cheating...

  // plugin test-harness z3 zdo match-desc-req <dstShort:2>
  // <nwkAddrOfInterest:2> <profileId:4> <options:4>
  emAfCliZdoInClusters[0] = 0x0001;
  emAfCliZdoInClusters[1] = 0x0002;
  emAfCliZdoOutClusters[0] = 0x0101;
  emAfCliZdoOutClusters[1] = 0x0102;

  addSimpleAction("setClCounts(%d, %d)", setClCounts, 2, 1, 2);
  addCliCallAction(emAfPluginTestHarnessZ3ZdoMatchDescReqCommand,
                   "2224",
                   makeMessage("2224", remoteNodeId, 0x1234, HA_PROFILE_ID, 0));
  addZdoOutCheck(remoteNodeId,
                 MATCH_DESCRIPTORS_REQUEST,
                 makeMessage("1<2<21p1p",
                             emberNextZigDevRequestSequence(), // cheating...
                             0x1234,
                             HA_PROFILE_ID,
                             1,
                             makeMessage("<2", 0x0001),
                             2,
                             makeMessage("<2<2", 0x0101, 0x0102)));

  addSimpleAction("setClCounts(%d, %d)", setClCounts, 2, 2, 0);
  addCliCallAction(emAfPluginTestHarnessZ3ZdoMatchDescReqCommand,
                   "2224",
                   makeMessage("2224", remoteNodeId, 0x4321, HA_PROFILE_ID, 0));
  addZdoOutCheck(remoteNodeId,
                 MATCH_DESCRIPTORS_REQUEST,
                 makeMessage("1<2<21p1p",
                             emberNextZigDevRequestSequence(), // cheating...
                             0x4321,
                             HA_PROFILE_ID,
                             2,
                             makeMessage("<2<2", 0x0001, 0x0002),
                             0,
                             makeParcel(0)));

  addSimpleAction("setClCounts(%d, %d)", setClCounts, 2, 1, 1);
  addCliCallAction(emAfPluginTestHarnessZ3ZdoMatchDescReqCommand,
                   "2224",
                   makeMessage("2224", remoteNodeId, 0x1234, HA_PROFILE_ID, BIT(0)));
  addZdoOutCheck(remoteNodeId,
                 MATCH_DESCRIPTORS_REQUEST,
                 makeMessage("1<21p1p",
                             emberNextZigDevRequestSequence(), // cheating...
                             HA_PROFILE_ID,
                             1,
                             makeMessage("<2", 0x0001),
                             1,
                             makeMessage("<2", 0x0101)));

  addSimpleAction("setClCounts(%d, %d)", setClCounts, 2, 1, 1);
  addCliCallAction(emAfPluginTestHarnessZ3ZdoMatchDescReqCommand,
                   "2224",
                   makeMessage("2224", remoteNodeId, 0x1234, HA_PROFILE_ID, BIT(1)));
  addZdoOutCheck(remoteNodeId,
                 MATCH_DESCRIPTORS_REQUEST,
                 makeMessage("1<2<211",
                             emberNextZigDevRequestSequence(), // cheating...
                             0x1234,
                             HA_PROFILE_ID,
                             1,
                             1));

  addSimpleAction("setClCounts(%d, %d)", setClCounts, 2, 1, 1);
  addCliCallAction(emAfPluginTestHarnessZ3ZdoMatchDescReqCommand,
                   "2224",
                   makeMessage("2224", remoteNodeId, 0x1234, 0xACAC, 0));
  addZdoOutCheck(remoteNodeId,
                 MATCH_DESCRIPTORS_REQUEST,
                 makeMessage("1<2<21<21<2",
                             emberNextZigDevRequestSequence(), // cheating...
                             0x1234,
                             0xACAC,
                             1,
                             0x0001,
                             1,
                             0x0101));

  addSimpleAction("setClCounts(%d, %d)", setClCounts, 2, 2, 2);
  addCliCallAction(emAfPluginTestHarnessZ3ZdoMatchDescReqCommand,
                   "2224",
                   makeMessage("2224", remoteNodeId, 0x1234, 0xACAC, BIT(2)));
  addZdoOutCheck(remoteNodeId,
                 MATCH_DESCRIPTORS_REQUEST,
                 makeMessage("1<2<21<21<2<2",
                             emberNextZigDevRequestSequence(), // cheating...
                             0x1234,
                             0xACAC,
                             2,
                             0x0001,
                             2,
                             0x0101,
                             0x0102));

  addSimpleAction("setClCounts(%d, %d)", setClCounts, 2, 2, 2);
  addCliCallAction(emAfPluginTestHarnessZ3ZdoMatchDescReqCommand,
                   "2224",
                   makeMessage("2224", remoteNodeId, 0x1234, 0xACAC, BIT(3)));
  addZdoOutCheck(remoteNodeId,
                 MATCH_DESCRIPTORS_REQUEST,
                 makeMessage("1<2<21<2<21<2",
                             emberNextZigDevRequestSequence(), // cheating...
                             0x1234,
                             0xACAC,
                             2,
                             0x0001,
                             0x0002,
                             2,
                             0x0101));

  // plugin test-harness z3 simple-desc-req <dstShort:2> <dstEndpoint:1>
  // <nwkAddrOfInterest:2> <options:4>
  addCliCallAction(emAfPluginTestHarnessZ3ZdoSimpleDescReqCommand,
                   "2124",
                   makeMessage("2124", remoteNodeId, 8, 0x1234, 0));
  addZdoOutCheck(remoteNodeId,
                 SIMPLE_DESCRIPTOR_REQUEST,
                 makeMessage("1<21",
                             emberNextZigDevRequestSequence(), // cheating...
                             0x1234,
                             8));

  addCliCallAction(emAfPluginTestHarnessZ3ZdoSimpleDescReqCommand,
                   "2124",
                   makeMessage("2124", remoteNodeId, 8, 0x1234, BIT(0)));
  addZdoOutCheck(remoteNodeId,
                 SIMPLE_DESCRIPTOR_REQUEST,
                 makeMessage("11",
                             emberNextZigDevRequestSequence(), // cheating...
                             8));

  addCliCallAction(emAfPluginTestHarnessZ3ZdoSimpleDescReqCommand,
                   "2124",
                   makeMessage("2124", remoteNodeId, 8, 0x1234, BIT(1)));
  addZdoOutCheck(remoteNodeId,
                 SIMPLE_DESCRIPTOR_REQUEST,
                 makeMessage("1<2",
                             emberNextZigDevRequestSequence(), // cheating...
                             0x1234));

  // plugin test-harness z3 zdo bind-group <shortAddress:2> <srcEndpoint:1>
  // <dstEndpoint:1> <dstAddress:2> <cluster:2>
  addCliCallAction(emAfPluginTestHarnessZ3ZdoBindGroupCommand,
                   "211228",
                   makeMessage("21122s",
                               remoteNodeId,
                               1,
                               2,
                               0xABCD,
                               0x1234,
                               emLocalEui64,
                               EUI64_SIZE));
  addZdoOutCheck(remoteNodeId,
                 BIND_REQUEST,
                 makeMessage("1s1<21<2",
                             emberNextZigDevRequestSequence(), // cheating...
                             emLocalEui64,
                             EUI64_SIZE,
                             1,
                             0x1234,
                             0x01,
                             0xABCD));

  // plugin test-harness z3 zdo nwk-addr-req <ieee:8> <requestType:1>
  // <startIndex:1> <dstShort:2> <options:4>
  addCliCallAction(emAfPluginTestHarnessZ3ZdoNwkAddrReqCommand,
                   "81124",
                   makeMessage("s1124",
                               remoteEui64,
                               EUI64_SIZE,
                               0,
                               1,
                               remoteNodeId,
                               0));
  addZdoOutCheck(remoteNodeId,
                 NETWORK_ADDRESS_REQUEST,
                 makeMessage("1s11",
                             emberNextZigDevRequestSequence(), // cheating...
                             remoteEui64,
                             EUI64_SIZE,
                             0,
                             1));

  addCliCallAction(emAfPluginTestHarnessZ3ZdoNwkAddrReqCommand,
                   "81124",
                   makeMessage("s1124",
                               remoteEui64,
                               EUI64_SIZE,
                               0,
                               1,
                               remoteNodeId,
                               BIT(0)));
  addZdoOutCheck(remoteNodeId,
                 NETWORK_ADDRESS_REQUEST,
                 makeMessage("1",
                             emberNextZigDevRequestSequence())); // cheating...

  // plugin test-harness z3 zdo ieee-addr-req <nwkAddrOfInterest:2>
  // <requestType:1> <startIndex:1> <dstShort:2> <options:4>
  addCliCallAction(emAfPluginTestHarnessZ3ZdoIeeeAddrReqCommand,
                   "21124",
                   makeMessage("21124", 0x1234, 0, 1, remoteNodeId, 0));
  addZdoOutCheck(remoteNodeId,
                 IEEE_ADDRESS_REQUEST,
                 makeMessage("1<211",
                             emberNextZigDevRequestSequence(), // cheating...
                             0x1234,
                             0,
                             1));

  addCliCallAction(emAfPluginTestHarnessZ3ZdoIeeeAddrReqCommand,
                   "21124",
                   makeMessage("21124", 0x1234, 0, 1, remoteNodeId, BIT(0)));
  addZdoOutCheck(remoteNodeId,
                 IEEE_ADDRESS_REQUEST,
                 makeMessage("1",
                             emberNextZigDevRequestSequence())); // cheating...

  // plugin test-harness z3 zdo bc-device-announce <shortAddress:2> <ieeeAddress:8>
  addCliCallAction(emAfPluginTestHarnessZ3ZdoBcDeviceAnnounceCommand,
                   "28",
                   makeMessage("2s",
                               remoteNodeId,
                               remoteEui64,
                               EUI64_SIZE));
  addZdoOutCheck(EMBER_RX_ON_WHEN_IDLE_BROADCAST_ADDRESS,
                 END_DEVICE_ANNOUNCE,
                 makeMessage("1<2s1",
                             emberNextZigDevRequestSequence(), // cheating...
                             remoteNodeId,
                             remoteEui64,
                             EUI64_SIZE,
                             0xE0)); // munged capabitlities byte

  addBufferTestAction(0);
  runScript();
}

static void zclCommandsTest(void)
{
  setFreeBufferBenchmark();

  // plugin test-harness z3 read <cluster:2> <attribute:2> <dstShort:2>
  // <srcEndpoint:1> <dstEndpoint:1> <profileId:2>
  addCliCallAction(emAfPluginTestHarnessZ3ZclReadCommand,
                   "222112",
                   makeMessage("222112",
                               0x0001,
                               0x0002,
                               remoteNodeId,
                               0x04,
                               0x05,
                               0x0006));
  addApsOutCheck(remoteNodeId,
                 0x0006,
                 0x0001,
                 0x04,
                 0x05,
                 false,
                 makeMessage("111<2",
                             0x00,
                             0xAC,
                             0x00,
                             0x0002));

  addBufferTestAction(0);
  runScript();
}

// -----------------------------------------------------------------------------
// incomingCommandTest

#define getIncomingCommandContents(action)                       \
  EmberZigbeeCommandType commandType;                            \
  Parcel *commandBuffer;                                         \
  uint8_t indexOfCommand;                                        \
  Parcel *data;                                                  \
  commandType    = (EmberZigbeeCommandType) action->contents[0]; \
  commandBuffer  = (Parcel *)               action->contents[1]; \
  indexOfCommand = (uint8_t)                action->contents[2]; \
  data           = (Parcel *)               action->contents[3];

PERFORMER(incomingCommand)
{
  EmberMessageBuffer reallyCommandBuffer;
  uint8_t packet[127];
  uint8_t length;
  Parcel *commandAftermath = NULL;

  getIncomingCommandContents(action);
  reallyCommandBuffer = parcelToBuffer(commandBuffer);
  length = emberMessageBufferLength(reallyCommandBuffer) - indexOfCommand;
  emberCopyFromLinkedBuffers(reallyCommandBuffer, indexOfCommand, packet, length);
  emberAfIncomingPacketFilterCallback(commandType,
                                      packet,
                                      &length,
                                      data);

  commandAftermath = bufferToParcel(reallyCommandBuffer);
  simpleScriptCheck("emberAfIncomingPacketFilterCallback",
                    "incomingCommandAftermath",
                    "p",
                    commandAftermath);

  emberReleaseMessageBuffer(reallyCommandBuffer);
  free(commandAftermath);
}

PRINTER(incomingCommand)
{
}

ACTION(incomingCommand, ipip);

#define addIncomingCommandAction(commandType,    \
                                 commandBuffer,  \
                                 indexOfCommand, \
                                 data)           \
  addAction(&incomingCommandActionType,          \
            (commandType),                       \
            (commandBuffer),                     \
            (indexOfCommand),                    \
            (data))

void incomingCommandTest(void)
{
  setFreeBufferBenchmark();

  // plugin test-harness z3 nwk nwk-leave-supression on
  // plugin test-harness z3 nwk nwk-leave-supression off
  addSimpleAction("setLastCommandArgument(%s)",
                  setLastCommandArgument,
                  1,
                  "on");
  addCliCallAction(emAfPluginTestHarnessZ3NwkNwkLeaveSupression,
                   "",
                   makeParcel(0));
  addIncomingCommandAction(EMBER_ZIGBEE_COMMAND_TYPE_NWK,
                           makeMessage("11",
                                       0x04,    // leave command
                                       BIT(6)), // request
                           0,
                           makeParcel(0));
  addSimpleCheck("incomingCommandAftermath",
                 "p",
                 makeMessage("11",
                             0x04, // leave command
                             0));  // no options

  addSimpleAction("setLastCommandArgument(%s)",
                  setLastCommandArgument,
                  1,
                  "off");
  addCliCallAction(emAfPluginTestHarnessZ3NwkNwkLeaveSupression,
                   "",
                   makeParcel(0));
  addIncomingCommandAction(EMBER_ZIGBEE_COMMAND_TYPE_NWK,
                           makeMessage("11",
                                       0x04,    // leave command
                                       BIT(6)), // request
                           0,
                           makeParcel(0));
  addSimpleCheck("incomingCommandAftermath",
                 "p",
                 makeMessage("11",
                             0x04,     // leave command
                             BIT(6))); // same options

  addBufferTestAction(0);
  runScript();
}

// -----------------------------------------------------------------------------
// resetTest

void emberAfZllResetToFactoryNew(void)
{
  functionCallCheck("emberAfZllResetToFactoryNew", "");
}

static void resetTest(void)
{
  // plugin test-harness z3 reset
  addCliCallAction(emAfPluginTestHarnessZ3ResetCommand, "", makeParcel(0));

  addSimpleCheck("emberAfZllResetToFactoryNew", "");

  runScript();
}

// -----------------------------------------------------------------------------
// MAIN.

static Test tests[] = {
  { "mac-commands", macCommandsTest     },
  { "nwk-commands", nwkCommandsTest     },
  { "aps-commands", apsCommandsTest     },
  { "zll-commands", zllCommandsTest     },
  { "zdo-commands", zdoCommandsTest     },
  { "zcl-commands", zclCommandsTest     },
  { "incoming-command", incomingCommandTest },
  { "reset", resetTest           },
  { NULL, NULL                },
};

int main(int argc, char *argv[])
{
  Thunk test = parseTestArgument(argc, argv, tests);

  {
    // Network initialization.
    EmberNetworkParameters params;
    MEMMOVE(params.extendedPanId, myExtPanId, EXTENDED_PAN_ID_SIZE);
    params.panId = MY_PAN_ID;
    params.radioTxPower = 0;
    params.radioChannel = MY_CHANNEL;
    params.nwkManagerId = 0;
    params.nwkUpdateId = MY_NETWORK_UPDATE_ID;
    params.nwkManagerId = MY_NETWORK_MANAGER_ID;

    testFrameworkNodeId = MY_NODE_ID;
    setLocalEui64(emLocalEui64);
    setNetworkParameters(&myNodeType, &params);
  }

  {
    // AF initialization.
    emberAfSetExternalBuffer(zclBuffer,
                             sizeof(zclBuffer),
                             &zclBufferLength,
                             &zclApsFrame);
  }

  {
    // CLI initialization.
    MEMSET(cliBuffer, 0, sizeof(cliBuffer));
  }

  test();

  fprintf(stderr, " done ]\n");
  return 0;
}

void scriptTickCallback(void)
{
}

// -----------------------------------------------------------------------------
// STUBS.

// Stack.
uint8_t emberFindKeyTableEntry(EmberEUI64 address, bool linkKey)
{
  return 0xFF;
}
EmberStatus emberFindAndRejoinNetwork(bool haveCurrentNetworkKey,
                                      uint32_t channelMask)
{
  functionCallCheck("emberFindAndRejoinNetwork",
                    "ii",
                    haveCurrentNetworkKey,
                    channelMask);
  return EMBER_SUCCESS;
}
EmberStatus emberLeaveNetwork(void)
{
  functionCallCheck("emberLeaveNetwork",
                    "");
  return EMBER_SUCCESS;
}

EmberStatus emberSendZigbeeLeave(EmberNodeId nodeId,
                                 EmberLeaveRequestFlags flags)
{
  functionCallCheck("emberSendZigbeeLeave",
                    "ii",
                    nodeId,
                    flags);
  return EMBER_SUCCESS;
}
EmberStatus emberStartScan(EmberNetworkScanType type,
                           uint32_t channelMask,
                           uint8_t scanDuration)
{
  functionCallCheck("emberStartScan",
                    "ii",
                    type,
                    channelMask);
  return EMBER_SUCCESS;
}
EmberStatus emberSendRemoveDevice(EmberNodeId destShort,
                                  EmberEUI64 destLong,
                                  EmberEUI64 deviceToRemoveLong)
{
  functionCallCheck("emberSendRemoveDevice",
                    "ipp",
                    destShort,
                    makeMessage("s", destLong, EUI64_SIZE),
                    makeMessage("s", deviceToRemoveLong, EUI64_SIZE));
  return EMBER_SUCCESS;
}
EmberStatus emberZllSetPolicy(EmberZllPolicy p)
{
  functionCallCheck("emberZllSetPolicy",
                    "i",
                    p);
  return EMBER_SUCCESS;
}

EmberStatus emberAddTransientLinkKey(EmberEUI64 partnerEUI64, EmberKeyData *key)
{
  return EMBER_SUCCESS;
}

// ZDO.
EmberStatus emberIeeeAddressRequest(EmberNodeId target,
                                    bool reportKids,
                                    uint8_t childStartIndex,
                                    EmberApsOption options)
{
  functionCallCheck("emberIeeeAddressRequest",
                    "iiii",
                    target,
                    reportKids,
                    childStartIndex,
                    options);
  return EMBER_SUCCESS;
}
EmberStatus emberNetworkAddressRequest(EmberEUI64 target,
                                       bool reportKids,
                                       uint8_t childStartIndex)
{
  functionCallCheck("emberNetworkAddressRequest",
                    "pii",
                    makeMessage("s", target, EUI64_SIZE),
                    reportKids,
                    childStartIndex);
  return EMBER_SUCCESS;
}
EmberStatus emberPermitJoiningRequest(EmberNodeId target,
                                      uint8_t duration,
                                      uint8_t authentication,
                                      EmberApsOption options)
{
  functionCallCheck("emberPermitJoiningRequest",
                    "iii",
                    target,
                    authentication,
                    duration);

  return EMBER_SUCCESS;
}
uint8_t emberNextZigDevRequestSequence(void)
{
  return WHATEVER;
}

// App.
uint32_t emberAfGetCurrentTime(void)
{
  return (  (WHATEVER << 0x00)
            | (WHATEVER << 0x08)
            | (WHATEVER << 0x10)
            | (WHATEVER << 0x18));
}
EmberStatus emberAfGetNodeType(EmberNodeType *nodeType)
{
  *nodeType = myNodeType;
  return EMBER_SUCCESS;
}
EmberPanId emberAfGetPanId(void)
{
  return MY_PAN_ID;
}
void emberAfCopyInt16u(uint8_t *data, uint16_t index, uint16_t x)
{
  data[index]   = (uint8_t) ( ((x)    ) & 0xFF);
  data[index + 1] = (uint8_t) ( ((x) >> 8) & 0xFF);
}
void emberAfCopyInt32u(uint8_t *data, uint16_t index, uint32_t x)
{
  data[index]   = (uint8_t) ( ((x)    ) & 0xFF);
  data[index + 1] = (uint8_t) ( ((x) >> 8) & 0xFF);
  data[index + 2] = (uint8_t) ( ((x) >> 16) & 0xFF);
  data[index + 3] = (uint8_t) ( ((x) >> 24) & 0xFF);
}
uint8_t emberAfResponseType;
uint8_t emberAfIndexFromEndpoint(uint8_t endpoint)
{
  return 0;
}
void emAfCliVersionCommand(void)
{
}
uint8_t emberAfNextSequence(void)
{
  return 0x00;
}
EmberStatus emberAfSendBroadcastWithAliasWithCallback(EmberNodeId destination,
                                                      EmberApsFrame *apsFrame,
                                                      uint16_t messageLength,
                                                      uint8_t *message,
                                                      EmberNodeId alias,
                                                      uint8_t sequence,
                                                      EmberAfMessageSentFunction callback)
{
  return EMBER_SUCCESS;
}
EmberStatus emberAfSendBroadcastWithCallback(EmberNodeId destination,
                                             EmberApsFrame *apsFrame,
                                             uint16_t messageLength,
                                             uint8_t* message,
                                             EmberAfMessageSentFunction callback)
{
  return EMBER_SUCCESS;
}
EmberStatus emberAfSendUnicastWithCallback(EmberOutgoingMessageType type,
                                           uint16_t indexOrDestination,
                                           EmberApsFrame *apsFrame,
                                           uint16_t messageLength,
                                           uint8_t* message,
                                           EmberAfMessageSentFunction callback)
{
  return EMBER_SUCCESS;
}
EmberStatus emberAfSendMulticastWithAliasWithCallback(EmberMulticastId multicastId,
                                                      EmberApsFrame *apsFrame,
                                                      uint16_t messageLength,
                                                      uint8_t *message,
                                                      EmberNodeId alias,
                                                      uint8_t sequence,
                                                      EmberAfMessageSentFunction callback)
{
  return EMBER_SUCCESS;
}
EmberStatus emberAfSendMulticastWithCallback(EmberMulticastId multicastId,
                                             EmberApsFrame *apsFrame,
                                             uint16_t messageLength,
                                             uint8_t* message,
                                             EmberAfMessageSentFunction callback)
{
  return EMBER_SUCCESS;
}
EmberStatus emberAfSendUnicastToBindings(EmberApsFrame *apsFrame,
                                         uint16_t messageLength,
                                         uint8_t* message)
{
  return EMBER_SUCCESS;
}
EmberStatus emberAfSendUnicastToBindingsWithCallback(EmberApsFrame *apsFrame,
                                                     uint16_t messageLength,
                                                     uint8_t* message,
                                                     EmberAfMessageSentFunction callback)
{
  return EMBER_SUCCESS;
}
EmberStatus emberAfSendMulticastToBindings(EmberApsFrame *apsFrame,
                                           uint16_t messageLength,
                                           uint8_t* message)
{
  return EMBER_SUCCESS;
}
EmberStatus emberAfZllSetInitialSecurityState(void)
{
  return EMBER_SUCCESS;
}

EmberStatus emAfZllFormNetwork(uint8_t channel, int8_t power, EmberPanId panId)
{
  return EMBER_SUCCESS;
}
uint8_t emberAfGetRadioChannel(void)
{
  return MY_CHANNEL;
}
