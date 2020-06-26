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
 * @brief Routines for the Command Relay plugin.
 *******************************************************************************
   ******************************************************************************/

#include "af.h"
#include "../device-table/device-table.h"
#include "command-relay.h"
#include "command-relay-local.h"

#define NULL_ENTRY 0xffff
#define EUI64_NULL_TERMINATED_STRING_SIZE 17
#define SAVE_FILE "command-relay.txt"

static const EmberEUI64 NULL_EUI = { 0, 0, 0, 0, 0, 0, 0, 0 };
#define isNullEui(eui) (MEMCOMPARE(eui, NULL_EUI, EUI64_SIZE) == 0)

static void initRelayTable(void);
static uint16_t findEmptyRelayEntry(void);
static void removeRelayEntry(uint16_t relayTableIndex);
static void forwardCommand(
  EmberAfClusterCommand* cmd,
  EmberAfPluginCommandRelayDeviceEndpoint* deviceEndpoint);
static bool matchDeviceEndpoints(
  EmberAfPluginCommandRelayDeviceEndpoint* deviceEndpoint1,
  EmberAfPluginCommandRelayDeviceEndpoint* deviceEndpoint2);
static void eui64ToString(EmberEUI64 eui, char* euiString);

#if defined(EZSP_HOST) && !defined(EMBER_TEST)
static void eui64FromString(char* euiString, EmberEUI64 eui);
#endif
static void printWithEndpoint(char* message,
                              EmberAfPluginCommandRelayDeviceEndpoint* deviceEndpoint);

static EmberAfPluginCommandRelayEntry relayTable[EMBER_AF_PLUGIN_COMMAND_RELAY_RELAY_TABLE_SIZE];

void emberAfPluginCommandRelayInitCallback(void)
{
  emberAfPluginCommandRelayLoad();
}

EmberAfPluginCommandRelayEntry* emberAfPluginCommandRelayTablePointer(void)
{
  return relayTable;
}

bool emAfPluginCommandRelayPreCommandReceivedCallback(EmberAfClusterCommand* cmd)
{
  EmberAfPluginCommandRelayDeviceEndpoint deviceEndpoint;
  deviceEndpoint.endpoint = cmd->apsFrame->sourceEndpoint;
  deviceEndpoint.clusterId = cmd->apsFrame->clusterId;
  emberAfDeviceTableGetEui64FromNodeId(cmd->source, deviceEndpoint.eui64);
  uint16_t i;

  printWithEndpoint("Command received from", &deviceEndpoint);

  for (i = 0; i < EMBER_AF_PLUGIN_COMMAND_RELAY_RELAY_TABLE_SIZE; i++) {
    bool deviceMatch = matchDeviceEndpoints(&deviceEndpoint,
                                            &relayTable[i].inDeviceEndpoint);

    if (deviceMatch) {
      printWithEndpoint("Forwarding command to",
                        &relayTable[i].outDeviceEndpoint);

      forwardCommand(cmd, &relayTable[i].outDeviceEndpoint);
    }
  }

  return false; // We still want the framework to process the command.
}

void emberAfPluginCommandRelayRemove(
  EmberAfPluginCommandRelayDeviceEndpoint* inDeviceEndpoint,
  EmberAfPluginCommandRelayDeviceEndpoint* outDeviceEndpoint)
{
  uint16_t i;
  for (i = 0; i < EMBER_AF_PLUGIN_COMMAND_RELAY_RELAY_TABLE_SIZE; i++) {
    bool inDeviceMatch = matchDeviceEndpoints(inDeviceEndpoint,
                                              &relayTable[i].inDeviceEndpoint);
    bool outDeviceMatch = matchDeviceEndpoints(outDeviceEndpoint,
                                               &relayTable[i].outDeviceEndpoint);

    if (inDeviceMatch && outDeviceMatch) {
      removeRelayEntry(i);
    }
  }

  emberAfPluginCommandRelaySave();
}

void emAfPluginCommandRelayRemoveDeviceByEui64(EmberEUI64 eui64)
{
  uint16_t i;
  for (i = 0; i < EMBER_AF_PLUGIN_COMMAND_RELAY_RELAY_TABLE_SIZE; i++) {
    bool inDeviceMatch = MEMCOMPARE(relayTable[i].inDeviceEndpoint.eui64,
                                    eui64,
                                    EUI64_SIZE) == 0;

    bool outDeviceMatch = MEMCOMPARE(relayTable[i].outDeviceEndpoint.eui64,
                                     eui64,
                                     EUI64_SIZE) == 0;

    if (inDeviceMatch || outDeviceMatch) {
      removeRelayEntry(i);
    }
  }

  emberAfPluginCommandRelaySave();
}

void emberAfPluginCommandRelayClear(void)
{
  uint16_t i;
  for (i = 0; i < EMBER_AF_PLUGIN_COMMAND_RELAY_RELAY_TABLE_SIZE; i++) {
    removeRelayEntry(i);
  }

  emberAfPluginCommandRelaySave();
}

void emberAfPluginCommandRelaySave(void)
{
#if defined(EZSP_HOST) && !defined(EMBER_TEST)
  uint16_t i;
  FILE *fp = fopen(SAVE_FILE, "w");
  char inEuiStr[EUI64_NULL_TERMINATED_STRING_SIZE] = { 0 };
  char outEuiStr[EUI64_NULL_TERMINATED_STRING_SIZE] = { 0 };

  for (i = 0; i < EMBER_AF_PLUGIN_COMMAND_RELAY_RELAY_TABLE_SIZE; i++) {
    if (!isNullEui(relayTable[i].inDeviceEndpoint.eui64)) {
      eui64ToString(relayTable[i].inDeviceEndpoint.eui64, inEuiStr);
      eui64ToString(relayTable[i].outDeviceEndpoint.eui64, outEuiStr);

      fprintf(fp, "%s %d %x %s %d %x\r\n",
              inEuiStr,
              relayTable[i].inDeviceEndpoint.endpoint,
              relayTable[i].inDeviceEndpoint.clusterId,
              outEuiStr,
              relayTable[i].outDeviceEndpoint.endpoint,
              relayTable[i].outDeviceEndpoint.clusterId);
    }
  }

  fclose(fp);
#endif // #if defined(EZSP_HOST) && !defined(EMBER_TEST)

  emberAfPluginCommandRelayChangedCallback();
}

void emberAfPluginCommandRelayLoad(void)
{
  initRelayTable();

#if defined(EZSP_HOST) && !defined(EMBER_TEST)
  FILE *fp;
  char* lineBuf = NULL;
  size_t lineBufSize = 0;

  char inEuiStr[17] = { 0 };
  char outEuiStr[17] = { 0 };
  uint8_t inEndpoint, outEndpoint;
  uint16_t inClusterId, outClusterId;

  fp = fopen(SAVE_FILE, "r");

  if (!fp) {
    return;
  }

  uint16_t index = 0;
  while ((getline(&lineBuf, &lineBufSize, fp) != -1)
         && index < EMBER_AF_PLUGIN_COMMAND_RELAY_RELAY_TABLE_SIZE) {
    if (sscanf(lineBuf,
               "%16s %hhd %hx %16s %hhd %hx",
               inEuiStr,
               &inEndpoint,
               &inClusterId,
               outEuiStr,
               &outEndpoint,
               &outClusterId) == 6) {
      eui64FromString(inEuiStr, relayTable[index].inDeviceEndpoint.eui64);
      eui64FromString(outEuiStr, relayTable[index].outDeviceEndpoint.eui64);
      relayTable[index].inDeviceEndpoint.endpoint = inEndpoint;
      relayTable[index].inDeviceEndpoint.clusterId = inClusterId;
      relayTable[index].outDeviceEndpoint.endpoint = outEndpoint;
      relayTable[index].outDeviceEndpoint.clusterId = outClusterId;

      index++;
    }
  }

  free(lineBuf);
  fclose(fp);

#endif // #if defined(EZSP_HOST) && !defined(EMBER_TEST)

  emberAfPluginCommandRelayChangedCallback();
}

void emAfPluginCommandRelayPrint(void)
{
  char inEuiStr[EUI64_NULL_TERMINATED_STRING_SIZE] = { 0 };
  char outEuiStr[EUI64_NULL_TERMINATED_STRING_SIZE] = { 0 };
  uint16_t i;

  for (i = 0; i < EMBER_AF_PLUGIN_COMMAND_RELAY_RELAY_TABLE_SIZE; i++) {
    if (!isNullEui(relayTable[i].inDeviceEndpoint.eui64)) {
      eui64ToString(relayTable[i].inDeviceEndpoint.eui64, inEuiStr);
      eui64ToString(relayTable[i].outDeviceEndpoint.eui64, outEuiStr);

      emberAfCorePrintln("%d %s %d 0x%2x %s %d 0x%2x",
                         i,
                         inEuiStr,
                         relayTable[i].inDeviceEndpoint.endpoint,
                         relayTable[i].inDeviceEndpoint.clusterId,
                         outEuiStr,
                         relayTable[i].outDeviceEndpoint.endpoint,
                         relayTable[i].outDeviceEndpoint.clusterId);
    }
  }
}

static void initRelayTable(void)
{
  uint16_t i;
  for (i = 0; i < EMBER_AF_PLUGIN_COMMAND_RELAY_RELAY_TABLE_SIZE; i++) {
    removeRelayEntry(i);
  }
}

static uint16_t findEmptyRelayEntry(void)
{
  uint16_t i;
  for (i = 0; i < EMBER_AF_PLUGIN_COMMAND_RELAY_RELAY_TABLE_SIZE; i++) {
    if (isNullEui(relayTable[i].inDeviceEndpoint.eui64)) {
      return i;
    }
  }
  return NULL_ENTRY;
}

static void removeRelayEntry(uint16_t relayTableIndex)
{
  if (relayTableIndex > EMBER_AF_PLUGIN_COMMAND_RELAY_RELAY_TABLE_SIZE) {
    return;
  }

  MEMSET(relayTable[relayTableIndex].inDeviceEndpoint.eui64, 0, EUI64_SIZE);
  relayTable[relayTableIndex].inDeviceEndpoint.endpoint = EMBER_AF_INVALID_ENDPOINT;
  MEMSET(relayTable[relayTableIndex].outDeviceEndpoint.eui64, 0, EUI64_SIZE);
  relayTable[relayTableIndex].outDeviceEndpoint.endpoint = EMBER_AF_INVALID_ENDPOINT;
}

static void forwardCommand(
  EmberAfClusterCommand* cmd,
  EmberAfPluginCommandRelayDeviceEndpoint* deviceEndpoint)
{
  MEMCOPY(emAfZclBuffer, cmd->buffer, cmd->bufLen);
  emAfZclBuffer[0] |= ZCL_DISABLE_DEFAULT_RESPONSE_MASK;
  emAfZclBuffer[1] = emberAfNextSequence();
  *emAfResponseLengthPtr = cmd->bufLen;
  emAfCommandApsFrame->clusterId = cmd->apsFrame->clusterId;
  emAfCommandApsFrame->options = EMBER_AF_DEFAULT_APS_OPTIONS;

  emberAfDeviceTableCommandSendWithEndpoint(deviceEndpoint->eui64,
                                            deviceEndpoint->endpoint);
}

static bool matchDeviceEndpoints(
  EmberAfPluginCommandRelayDeviceEndpoint* deviceEndpoint1,
  EmberAfPluginCommandRelayDeviceEndpoint* deviceEndpoint2)
{
  bool euisMatch = MEMCOMPARE(deviceEndpoint1->eui64,
                              deviceEndpoint2->eui64,
                              EUI64_SIZE) == 0;
  bool endpointsMatch = deviceEndpoint1->endpoint == deviceEndpoint2->endpoint;
  bool clustersMatch = deviceEndpoint1->clusterId == deviceEndpoint2->clusterId;

  return euisMatch && endpointsMatch && clustersMatch;
}

static void eui64ToString(EmberEUI64 eui, char* euiString)
{
  snprintf(euiString,
           EUI64_NULL_TERMINATED_STRING_SIZE,
           "%02X%02X%02X%02X%02X%02X%02X%02X",
           eui[7],
           eui[6],
           eui[5],
           eui[4],
           eui[3],
           eui[2],
           eui[1],
           eui[0]);
}

#if defined(EZSP_HOST) && !defined(EMBER_TEST)
static void eui64FromString(char* euiString, EmberEUI64 eui)
{
  if (sscanf(euiString,
             "%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx",
             &eui[7],
             &eui[6],
             &eui[5],
             &eui[4],
             &eui[3],
             &eui[2],
             &eui[1],
             &eui[0]) != 8) {
    MEMSET(eui, 0, EUI64_SIZE);
  }
}
#endif

static void printWithEndpoint(
  char* message,
  EmberAfPluginCommandRelayDeviceEndpoint* deviceEndpoint)
{
  char euiStr[EUI64_NULL_TERMINATED_STRING_SIZE] = { 0 };
  eui64ToString(deviceEndpoint->eui64, euiStr);
  emberAfCorePrintln("CommandRelay: %s %s %d",
                     message,
                     euiStr,
                     deviceEndpoint->endpoint);
}

//public API implementations
void emberAfPluginCommandRelayAdd(
  EmberAfPluginCommandRelayDeviceEndpoint* inDeviceEndpoint,
  EmberAfPluginCommandRelayDeviceEndpoint* outDeviceEndpoint)
{
  uint16_t index = findEmptyRelayEntry();
  if (index == NULL_ENTRY) {
    return;
  }

  relayTable[index].inDeviceEndpoint = *inDeviceEndpoint;
  relayTable[index].outDeviceEndpoint = *outDeviceEndpoint;

  emberAfPluginCommandRelaySave();
}
