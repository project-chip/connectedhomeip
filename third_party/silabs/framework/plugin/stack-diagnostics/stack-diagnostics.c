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
 * @brief Prints various information abouth the stack.
 *******************************************************************************
   ******************************************************************************/

#include "app/framework/include/af.h"
#include "app/framework/util/af-main.h"
#include "app/util/serial/command-interpreter2.h"
#include "stack/include/network-formation.h" // for emberGetNeighborLastLqiRssi

//------------------------------------------------------------------------------
// Forward Declarations

void printChildTable(void);
void printNeighborTable(void);
void printRouteTable(void);
void printInfo(void);

//------------------------------------------------------------------------------
// Globals

#ifndef EMBER_AF_GENERATE_CLI
EmberCommandEntry emberAfPluginStackDiagnosticsCommands[] = {
  emberCommandEntryAction("info", printInfo, "", "Print general stack info"),
  emberCommandEntryAction("child-table", printChildTable, "", "Print the child table"),
  emberCommandEntryAction("neighbor-table", printNeighborTable, "", "Print the neighbor table"),
  emberCommandEntryAction("route-table", printRouteTable, "", "Print the route table"),
  emberCommandEntryTerminator(),
};
#endif // EMBER_AF_GENERATE_CLI

#define UNKNOWN_LEAVE_REASON_INDEX     5
#define APP_EVENT_1_LEAVE_REASON_INDEX 6
static const char * leaveReasonStrings[] = {
  "-",                 // None
  "NWK Leave message",
  "APS Remove message",
  "ZDO Leave message",
  "ZLL Touchlink",
  "???",
  "App Event 1",
};

#define UNKNOWN_REJOIN_REASON_INDEX       5
#define APP_FRAMEWORK_REJOIN_REASON_INDEX 6
#define CUSTOM_APP_EVENTS_INDEX           9
static const char * rejoinReasonStrings[] = {
  "-",
  "NWK Key Update",
  "Leave & Rejoin Message",
  "No Parent",
  "ZLL Touchlink",
  "???",

  // General App. Framework events defined in 'af.h'
  "End Device Move",
  "TC Keepalive failure",
  "CLI Command",

  // Non App. Framework, Application specific custom events
  "App Event 5",
  "App Event 4",
  "App Event 3",
  "App Event 2",
  "App Event 1",
};

static const char * nodeTypeStrings[] = {
  "Unknown",
  "Coordinator",
  "Router",
  "End Device",
  "Sleep End Device",
};

#if !defined(EZSP_HOST) && defined(EMBER_AF_PLUGIN_STACK_DIAGNOSTICS_RECORD_LQI_RSSI)
  #define RECORD_LQI_RSSI
#endif // !EZSP_HOST && EMBER_AF_PLUGIN_STACK_DIAGNOSTICS_RECORD_LQI_RSSI

#ifdef RECORD_LQI_RSSI
#define DEFAULT_LQI    0
#define DEFAULT_RSSI  -127
static uint8_t neighborLqiTable[EMBER_MAX_NEIGHBOR_TABLE_SIZE] = { DEFAULT_LQI };
static int8_t neighborRssiTable[EMBER_MAX_NEIGHBOR_TABLE_SIZE] = { DEFAULT_RSSI };
#endif // RECORD_LQI_RSSI

//------------------------------------------------------------------------------
// Externs

#ifdef RECORD_LQI_RSSI
extern EmberNodeId emCurrentSender;
extern uint8_t emCurrentLqi;
extern int8_t emCurrentRssi;
extern uint8_t emNeighborEntryIndex(EmberNodeId neighbor);
#endif // RECORD_LQI_RSSI

//------------------------------------------------------------------------------
// Functions

void printChildTable(void)
{
  uint8_t size = emberAfGetChildTableSize();
  uint8_t i;
  const char * types[] = {
    "Unknown",
    "Coordin",
    "Router ",
    "RxOn   ",
    "Sleepy ",
    "???    ",
  };
  uint8_t used = 0;
  uint32_t returnFrameCounter = 0;

  emberAfAppPrintln("");
  emberAfAppPrintln("#  type    id     eui64               timeout         framecounter");
  for (i = 0; i < size; i++) {
    EmberChildData childData;
    EmberStatus status = emberAfGetChildData(i, &childData);
    if (status != EMBER_SUCCESS) {
      continue;
    }
    if (childData.type > EMBER_SLEEPY_END_DEVICE) {
      childData.type = EMBER_SLEEPY_END_DEVICE + 1;
    }
    used++;
    emberAfAppPrint("%d: %p 0x%2X ", i, types[childData.type], childData.id);
    emberAfAppDebugExec(emberAfPrintBigEndianEui64(childData.eui64));
    emberAfAppPrint(" ");
    if (childData.timeout == SECONDS_10) {
      emberAfAppPrint("10 sec");
    } else {
      emberAfAppPrint("%d min", (2 << (childData.timeout - 1)));
    }

    //Add in the Frame Counter information
    status = emberGetNeighborFrameCounter(childData.eui64, &returnFrameCounter);
    if (status != EMBER_SUCCESS) {
      returnFrameCounter = 0xFFFFFFFF;
    }

    emberAfAppPrint(" 0x%4X ", returnFrameCounter);
    emberAfAppPrintln("");
  }
  emberAfAppPrintln("\n%d of %d entries used.",
                    used,
                    emberAfGetChildTableSize());
}

void printNeighborTable(void)
{
  uint8_t used = 0;
  uint8_t i;
  EmberNeighborTableEntry n;
  uint32_t returnFrameCounter = 0;

  emberAfAppPrintln("\n#  id     lqi  in  out  age  "
                    #ifdef RECORD_LQI_RSSI
                    "lastRssi lastLqi "
                    #endif // RECORD_LQI_RSSI
                    "framecounter eui");
  for (i = 0; i < emberAfGetNeighborTableSize(); i++) {
    EmberStatus status = emberGetNeighbor(i, &n);
    if ((status != EMBER_SUCCESS)
        || (n.shortId == EMBER_NULL_NODE_ID)) {
      continue;
    }
    used++;

    //Add in Frame Counter information
    status = emberGetNeighborFrameCounter(n.longId, &returnFrameCounter);
    if (status != EMBER_SUCCESS) {
      returnFrameCounter = 0xFFFFFFFF;
    }

    emberAfAppPrint("%d: 0x%2X %d  %d   %d    %d    "
                    #ifdef RECORD_LQI_RSSI
                    "%d      %d     "
                    #endif // RECORD_LQI_RSSI
                    "0x%4X  "
                    ,
                    i,
                    n.shortId,
                    n.averageLqi,
                    n.inCost,
                    n.outCost,
                    n.age,
                    #ifdef RECORD_LQI_RSSI
                    neighborRssiTable[i],
                    neighborLqiTable[i],
                    #endif // RECORD_LQI_RSSI
                    returnFrameCounter
                    );
    emberAfAppDebugExec(emberAfPrintBigEndianEui64(n.longId));
    emberAfAppPrintln("");
    emberAfAppFlush();
  }
  emberAfAppPrintln("\n%d of %d entries used.", used, emberAfGetNeighborTableSize());
}

void printRouteTable(void)
{
  const char * zigbeeRouteStatusNames[] = {
    "active",
    "discovering",
    "??",
    "unused",
  };
  const char * concentratorNames[] = {
    "-    ",
    "low  ",
    "high ",
  };
  uint8_t used = 0;
  uint8_t i;
  EmberRouteTableEntry entry;

  emberAfAppPrintln("#  id      next    age  conc    status");
  for (i = 0; i < emberAfGetRouteTableSize(); i++) {
    if (EMBER_SUCCESS !=  emberGetRouteTableEntry(i, &entry)
        || entry.destination == EMBER_NULL_NODE_ID) {
      continue;
    }
    used++;
    emberAfAppPrintln("%d: 0x%2X  0x%2X  %d   %p  %p",
                      i,
                      entry.destination,
                      entry.nextHop,
                      entry.age,
                      concentratorNames[entry.concentratorType],
                      zigbeeRouteStatusNames[entry.status]);
  }
  emberAfAppPrintln("\n%d of %d entries used.", used, emberAfGetRouteTableSize());
}

void printInfo(void)
{
  EmberNodeId nodeThatSentLeave;
  EmberRejoinReason rejoinReason = emberGetLastRejoinReason();
  EmberLeaveReason  leaveReason =  emberGetLastLeaveReason(&nodeThatSentLeave);

  uint8_t index;

  EmberNodeId id = emberAfGetNodeId();
  EmberNodeType type = EMBER_UNKNOWN_DEVICE;
  EmberNetworkParameters parameters;

  if (!emAfProIsCurrentNetwork()) {
    return;
  }

  emberAfGetNetworkParameters(&type,
                              &parameters);

  emberAfAppPrintln("Stack Profile: %d", emberAfGetStackProfile());
  emberAfAppPrintln("Configured Node Type (%d): %p",
                    emAfCurrentZigbeeProNetwork->nodeType,
                    nodeTypeStrings[emAfCurrentZigbeeProNetwork->nodeType]);
  emberAfAppPrintln("Running Node Type    (%d): %p\n",
                    type,
                    nodeTypeStrings[type]);
  emberAfAppPrintln("Channel:       %d", parameters.radioChannel);
  emberAfAppPrintln("Node ID:       0x%2x", id);
  emberAfAppPrintln("PAN ID:        0x%2X", parameters.panId);
  emberAfAppPrint("Extended PAN:  ");
  emberAfPrintBigEndianEui64(parameters.extendedPanId);
  emberAfAppPrintln("\nNWK Update ID: 0x%X\n", parameters.nwkUpdateId);

  emberAfAppPrintln("NWK Manager ID: 0x%2X", parameters.nwkManagerId);
  emberAfAppPrint("NWK Manager Channels: ");
  emberAfPrintChannelListFromMask(parameters.channels);
  emberAfAppPrintln("\n");

  emberAfAppPrintln("Send Multicasts to sleepies: %p\n",
                    (emberAfGetSleepyMulticastConfig()
                     ? "yes"
                     : "no"));

  index = rejoinReason;
  if (rejoinReason >= EMBER_REJOIN_DUE_TO_APP_EVENT_5) {
    index = rejoinReason - EMBER_REJOIN_DUE_TO_APP_EVENT_5 + CUSTOM_APP_EVENTS_INDEX;
  } else if (rejoinReason >= EMBER_AF_REJOIN_FIRST_REASON
             && rejoinReason <= EMBER_AF_REJOIN_LAST_REASON) {
    index = rejoinReason - EMBER_AF_REJOIN_FIRST_REASON + APP_FRAMEWORK_REJOIN_REASON_INDEX;
  } else if (rejoinReason >= EMBER_REJOIN_DUE_TO_ZLL_TOUCHLINK) {
    index = UNKNOWN_REJOIN_REASON_INDEX;
  }
  emberAfAppPrintln("Last Rejoin Reason (%d): %p",
                    rejoinReason,
                    rejoinReasonStrings[index]);

  index = leaveReason;
  if (leaveReason == 0xFF) {
    index = APP_EVENT_1_LEAVE_REASON_INDEX;
  } else if (leaveReason > EMBER_LEAVE_DUE_TO_ZLL_TOUCHLINK) {
    index = UNKNOWN_LEAVE_REASON_INDEX;
  }
  emberAfAppPrintln("Last Leave Reason  (%d): %p",
                    leaveReason,
                    leaveReasonStrings[index]);
  emberAfAppPrintln("Node that sent leave: 0x%2X", nodeThatSentLeave);
}

#ifdef RECORD_LQI_RSSI
EmberPacketAction emberAfIncomingPacketFilterCallback(EmberZigbeePacketType packetType,
                                                      uint8_t* packetData,
                                                      uint8_t* size_p,
                                                      void *data)
{
  EmberNodeId macSource = EMBER_NULL_NODE_ID;

  switch (packetType) {
    case EMBER_ZIGBEE_PACKET_TYPE_RAW_MAC:
      // unprocessed by stack, ignore
      break;
    case EMBER_ZIGBEE_PACKET_TYPE_MAC_COMMAND:
      // only called when receiving beacon request, which lacks source info
      break;
    case EMBER_ZIGBEE_PACKET_TYPE_NWK_DATA:
    case EMBER_ZIGBEE_PACKET_TYPE_NWK_COMMAND:
      macSource = emCurrentSender;
      break;
    case EMBER_ZIGBEE_PACKET_TYPE_APS_DATA:
    case EMBER_ZIGBEE_PACKET_TYPE_APS_COMMAND:
    case EMBER_ZIGBEE_PACKET_TYPE_ZDO:
      // APS frames will have been called as EMBER_ZIGBEE_PACKET_TYPE_NWK_DATA
      // as well, so no need to double record them
      break;
    case EMBER_ZIGBEE_PACKET_TYPE_ZCL:
      // Only called for ZLL Interpan messages, which lack short source
      break;
    case EMBER_ZIGBEE_PACKET_TYPE_BEACON:
      // We'll only look at nwk+aps messages
      break;
    default:
      break;
  }

  if (macSource != EMBER_NULL_NODE_ID) {
    uint8_t index = emNeighborEntryIndex(macSource);

    if (index != 0xFF) {
      neighborLqiTable[index] = emCurrentLqi;
      neighborRssiTable[index] = emCurrentRssi;
    }
  }

  return EMBER_ACCEPT_PACKET;
}
#endif // RECORD_LQI_RSSI
