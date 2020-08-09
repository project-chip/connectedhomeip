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
 * @brief CLI for the Barrier Control Server plugin.
 *******************************************************************************
   ******************************************************************************/

#include "af.h"
#include "barrier-control-server.h"

static bool getBarrierPositionDescription(uint8_t endpoint,
                                          const char **descriptionLocation)
{
  bool descriptionWasSet = true;
  switch (emAfPluginBarrierControlServerGetBarrierPosition(endpoint)) {
    case EMBER_ZCL_BARRIER_CONTROL_BARRIER_POSITION_OPEN:
      *descriptionLocation = "open";
      break;
    case EMBER_ZCL_BARRIER_CONTROL_BARRIER_POSITION_CLOSED:
      *descriptionLocation = "closed";
      break;
    default:
      if (emAfPluginBarrierControlServerIsPartialBarrierSupported(endpoint)) {
        descriptionWasSet = false;
      } else {
        *descriptionLocation = "unknown";
      }
  }
  return descriptionWasSet;
}

static void printSafetyStatus(uint16_t safetyStatus)
{
  emberAfAppPrint("SafetyStatus: 0x%2X (", safetyStatus);
  if (safetyStatus != 0) {
    if (READBITS(safetyStatus,
                 EMBER_AF_BARRIER_CONTROL_SAFETY_STATUS_REMOTE_LOCKOUT)) {
      emberAfAppPrint("lockout,");
    }
    if (READBITS(safetyStatus,
                 EMBER_AF_BARRIER_CONTROL_SAFETY_STATUS_TEMPER_DETECTED)) {
      emberAfAppPrint("tamper,");
    }
    if (READBITS(safetyStatus,
                 EMBER_AF_BARRIER_CONTROL_SAFETY_STATUS_FAILED_COMMUNICATION)) {
      emberAfAppPrint("communication,");
    }
    if (READBITS(safetyStatus,
                 EMBER_AF_BARRIER_CONTROL_SAFETY_STATUS_POSITION_FAILURE)) {
      emberAfAppPrint("position,");
    }
  } else {
    emberAfAppPrint("OK");
  }
  emberAfAppPrintln(")");
}

// plugin barrier-control-server status <endpoint:1>
void emAfPluginBarrierControlServerStatusCommand(void)
{
  uint8_t endpoint = (uint8_t)emberUnsignedCommandArgument(0);
  uint8_t barrierPosition
    = emAfPluginBarrierControlServerGetBarrierPosition(endpoint);
  const char *description;
  emberAfAppPrint("BarrierPosition: %d%%", barrierPosition);
  if (getBarrierPositionDescription(endpoint, &description)) {
    emberAfAppPrint(" (%s)", description);
  }
  emberAfAppPrintln("");
  printSafetyStatus(emAfPluginBarrierControlServerGetSafetyStatus(endpoint));
}

// plugin barrier-control-server open <endpoint:1>
// plugin barrier-control-server close <endpoint:1>
void emAfPluginBarrierControlServerOpenOrCloseCommand(void)
{
  uint8_t endpoint = (uint8_t)emberUnsignedCommandArgument(0);
  bool open = emberStringCommandArgument(-1, NULL)[0] == 'o';
  uint8_t barrierPosition
    = (open
       ? EMBER_ZCL_BARRIER_CONTROL_BARRIER_POSITION_OPEN
       : EMBER_ZCL_BARRIER_CONTROL_BARRIER_POSITION_CLOSED);
  emAfPluginBarrierControlServerSetBarrierPosition(endpoint, barrierPosition);
  emAfPluginBarrierControlServerIncrementEvents(endpoint, open, false);

  const char *description;
  assert(getBarrierPositionDescription(endpoint, &description));
  emberAfAppPrintln("Barrier is now %s", description);
}

// plugin barrier-control-server remote-lockout <endpoint:1> <setOrClear:1>
// plugin barrier-control-server tamper-detected <endpoint:1> <setOrClear:1>
// plugin barrier-control-server failed-communication <endpoint:1> <setOrClear:1>
// plugin barrier-control-server position-failure <endpoint:1> <setOrClear:1>
void emAfPluginBarrierControlServerSafetyStatusCommand(void)
{
  uint8_t endpoint = (uint8_t)emberUnsignedCommandArgument(0);
  bool doSet = ((uint8_t)emberUnsignedCommandArgument(1)) == 1;
  uint8_t bit
    = (emberStringCommandArgument(-1, NULL)[0] == 'r'
       ? EMBER_AF_BARRIER_CONTROL_SAFETY_STATUS_REMOTE_LOCKOUT
       : (emberStringCommandArgument(-1, NULL)[0] == 't'
          ? EMBER_AF_BARRIER_CONTROL_SAFETY_STATUS_TEMPER_DETECTED
          : (emberStringCommandArgument(-1, NULL)[0] == 'f'
             ? EMBER_AF_BARRIER_CONTROL_SAFETY_STATUS_FAILED_COMMUNICATION
             : EMBER_AF_BARRIER_CONTROL_SAFETY_STATUS_POSITION_FAILURE)));

  uint16_t safetyStatus
    = emAfPluginBarrierControlServerGetSafetyStatus(endpoint);
  if (doSet) {
    SETBITS(safetyStatus, bit);
  } else {
    CLEARBITS(safetyStatus, bit);
  }

  EmberAfStatus status
    = emberAfWriteServerAttribute(endpoint,
                                  ZCL_BARRIER_CONTROL_CLUSTER_ID,
                                  ZCL_BARRIER_SAFETY_STATUS_ATTRIBUTE_ID,
                                  (uint8_t *)&safetyStatus,
                                  ZCL_BITMAP16_ATTRIBUTE_TYPE);
  assert(status == EMBER_ZCL_STATUS_SUCCESS);

  printSafetyStatus(safetyStatus);
}
