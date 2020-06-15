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
 * @brief ZigBee 3.0 nwk test harness functionality
 *******************************************************************************
   ******************************************************************************/

#include "app/framework/include/af.h"

#include "test-harness-z3-core.h"
#include "test-harness-z3-nwk.h"

// -----------------------------------------------------------------------------
// Globals

bool emAfPluginTestHarnessZ3IgnoreLeaveCommands = false;

// -----------------------------------------------------------------------------
// NWK CLI Commands

#ifdef EZSP_HOST
  #define emNetworkSendCommand(...) false
#else

// Internal stack API's.
extern bool emNetworkSendCommand(EmberNodeId destination,
                                 uint8_t *commandFrame,
                                 uint8_t length,
                                 bool tryToInsertLongDest,
                                 EmberEUI64 destinationEui);
extern uint8_t emSetNwkUpdateId(uint8_t id);

#endif /* EZSP_HOST */

static EmberStatus sendNetworkCommand(EmberNodeId destinationShort,
                                      uint8_t *commandFrame,
                                      uint8_t length)
{
  EmberEUI64 destinationLong;
  bool haveDestinationLong = false;

  haveDestinationLong
    = (emberLookupEui64ByNodeId(destinationShort, destinationLong)
       == EMBER_SUCCESS);

  return (emNetworkSendCommand(destinationShort,
                               commandFrame,
                               length,
                               haveDestinationLong,
                               destinationLong)
          ? EMBER_SUCCESS
          : EMBER_DELIVERY_FAILED);
}

// plugin test-harness z3 nwk nwk-rejoin-request <dstShort:2>
void emAfPluginTestHarnessZ3NwkNwkRejoinRequestCommand(void)
{
  EmberStatus status;
  EmberNodeId destinationShort = (EmberNodeId)emberUnsignedCommandArgument(0);
  EmberNodeType nodeType;
  uint8_t commandFrame[2];

  status = emberAfGetNodeType(&nodeType);
  if (status != EMBER_SUCCESS) {
    goto done;
  }

  commandFrame[0] = NWK_REJOIN_REQUEST_COMMAND;
  commandFrame[1] = 0;
  switch (nodeType) {
    case EMBER_COORDINATOR:
      commandFrame[1] |= BIT(0); // alternate pan coordinator
    // fallthrough
    case EMBER_ROUTER:
      commandFrame[1] |= BIT(1); // device type
    // fallthrough
    case EMBER_END_DEVICE:
      commandFrame[1] |= BIT(2); // mains-powered
      commandFrame[1] |= BIT(3); // rx on when idle
    // fallthrough
    case EMBER_SLEEPY_END_DEVICE:
      commandFrame[1] |= BIT(7); // allocate address
      break;
    default:
      status = EMBER_ERR_FATAL;
  }

  status = sendNetworkCommand(destinationShort,
                              commandFrame,
                              sizeof(commandFrame));

  done:
  emberAfCorePrintln("%p: %p: 0x%X",
                     TEST_HARNESS_Z3_PRINT_NAME,
                     "Rejoin request",
                     status);
}

// plugin test-harness z3 nwk nwk-rejoin-response <addr:2> <status:1> <dstShort:2>
void emAfPluginTestHarnessZ3NwkNwkRejoinResponseCommand(void)
{
  uint8_t     frame[4];
  EmberStatus status;

  EmberNodeId networkAddress   = (EmberNodeId)emberUnsignedCommandArgument(0);
  uint8_t     rejoinStatus     = (uint8_t)emberUnsignedCommandArgument(1);
  EmberNodeId destinationShort = (EmberNodeId)emberUnsignedCommandArgument(2);

  frame[0] = NWK_REJOIN_RESPONSE_COMMAND;
  frame[1] = LOW_BYTE(networkAddress);
  frame[2] = HIGH_BYTE(networkAddress);
  frame[3] = rejoinStatus;

  status = sendNetworkCommand(destinationShort, frame, sizeof(frame));

  emberAfCorePrintln("%p: %p: 0x%X",
                     TEST_HARNESS_Z3_PRINT_NAME,
                     "Rejoin response",
                     status);
}

// plugin test-harness z3 nwk nwk-leave <rejoin:1> <request:1>
// <removeChildren:1> <dstShort:2> <options:4>
void emAfPluginTestHarnessZ3NwkNwkLeaveCommand(void)
{
  EmberStatus status;
  uint8_t frame[2] = { NWK_LEAVE_COMMAND, 0, };

  bool rejoin                  = (bool)emberUnsignedCommandArgument(0);
  bool request                 = (bool)emberUnsignedCommandArgument(1);
  // CCB 2047
  // - CCB makes the first step to depracate the 'leave and remove children' functionality.
  // - We were proactive here and deprecated it right away.
  // bool removeChildren          = (bool)emberUnsignedCommandArgument(2);
  EmberNodeId destinationShort = (EmberNodeId)emberUnsignedCommandArgument(3);
  uint32_t options             = emAfPluginTestHarnessZ3GetSignificantBit(4);

  if (rejoin) {
    frame[1] |= BIT(5);
  }
  if (request) {
    frame[1] |= BIT(6);
  }

  switch (options) {
    case BIT(0): // no ieee addr field
      status = (emNetworkSendCommand(destinationShort,
                                     frame,
                                     sizeof(frame),
                                     false,
                                     NULL)
                ? EMBER_SUCCESS
                : EMBER_DELIVERY_FAILED);
      break;
    case BIT(1): // don't actually leave network
    // fallthrough
    default:
      status = sendNetworkCommand(destinationShort, frame, sizeof(frame));
  }

  emberAfCorePrintln("%p: %p: 0x%X",
                     TEST_HARNESS_Z3_PRINT_NAME,
                     "Leave",
                     status);
}

// plugin test-harness z3 nwk nwk-leave-supression on
// plugin test-harness z3 nwk nwk-leave-supression off
void emAfPluginTestHarnessZ3NwkNwkLeaveSupression(void)
{
  emAfPluginTestHarnessZ3IgnoreLeaveCommands
    = (emberStringCommandArgument(-1, NULL)[1] == 'n');
  emberAfCorePrintln("%p: %p: %p",
                     TEST_HARNESS_Z3_PRINT_NAME,
                     "Supress network leave",
                     (emAfPluginTestHarnessZ3IgnoreLeaveCommands
                      ? "YES"
                      : "NO"));
}

// plugin test-harness z3 nwk nwk-key-supression on
// plugin test-harness z3 nwk nwk-key-supression off
void emAfPluginTestHarnessZ3NwkNwkKeySupression(void)
{
  // TODO:
}

// plugin test-harness z3 nwk get-network-update-id
void emAfPluginTestHarnessZ3NwkGetNetworkUpdateId(void)
{
  EmberNetworkParameters networkParameters;
  EmberNodeType nodeType;
  emberAfGetNetworkParameters(&nodeType, &networkParameters);
  emberAfCorePrintln("%p: %p: 0x%X",
                     TEST_HARNESS_Z3_PRINT_NAME,
                     "Network Update Id",
                     networkParameters.nwkUpdateId);
}

// plugin test-harness z3 nwk set-network-update-id
void emAfPluginTestHarnessZ3NwkSetNetworkUpdateId(void)
{
  uint8_t updateId = (uint8_t)emberUnsignedCommandArgument(0);
  // Note, we can't use emberSetNwkUpdateId here, since that only sets
  // the update ID in a variable that is used when forming a network
  emSetNwkUpdateId(updateId);
  emberAfCorePrintln("%p: %p: 0x%X",
                     TEST_HARNESS_Z3_PRINT_NAME,
                     "Set network update id",
                     updateId);
}
