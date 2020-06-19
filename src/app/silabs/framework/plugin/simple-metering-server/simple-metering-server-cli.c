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
 * @brief CLI for the Simple Metering Server plugin.
 *******************************************************************************
   ******************************************************************************/

#include "app/framework/include/af.h"
#include "app/util/serial/command-interpreter2.h"
#include "simple-metering-test.h"
#include "simple-metering-server.h"
#if !defined(EMBER_AF_GENERATE_CLI)

void emAfPluginSimpleMeteringServerCliPrint(void);
void emAfPluginSimpleMeteringServerCliRate(void);
void emAfPluginSimpleMeteringServerCliVariance(void);
void emAfPluginSimpleMeteringServerCliAdjust(void);
void emAfPluginSimpleMeteringServerCliOff(void);

void emAfPluginSimpleMeteringServerCliElectric(void);
void emAfPluginSimpleMeteringServerCliGas(void);

#if defined(EMBER_AF_PLUGIN_SIMPLE_METERING_SERVER_TEST_METER_ERRORS)
void emAfPluginSimpleMeteringServerCliRandomError(void);
void emAfPluginSimpleMeteringServerCliSetError(void);
#endif //EMBER_AF_PLUGIN_SIMPLE_METERING_SERVER_TEST_METER_ERRORS

void emAfPluginSimpleMeteringServerCliProfiles(void);

EmberCommandEntry emberAfPluginSimpleMeteringServerCommands[] = {
#ifdef EMBER_AF_PLUGIN_SIMPLE_METERING_SERVER_TEST_METER_ENABLE
  emberCommandEntryAction("print", emAfPluginSimpleMeteringServerCliPrint, "", ""),
  emberCommandEntryAction("rate", emAfPluginSimpleMeteringServerCliRate, "v", ""),
  emberCommandEntryAction("variance", emAfPluginSimpleMeteringServerCliVariance, "v", ""),
  emberCommandEntryAction("adjust", emAfPluginSimpleMeteringServerCliAdjust, "u", ""),
  emberCommandEntryAction("off", emAfPluginSimpleMeteringServerCliOff, "u", ""),
  emberCommandEntryAction("electric", emAfPluginSimpleMeteringServerCliElectric, "u", ""),
  emberCommandEntryAction("gas", emAfPluginSimpleMeteringServerCliGas, "u", ""),
#ifdef EMBER_AF_PLUGIN_SIMPLE_METERING_SERVER_TEST_METER_ERRORS
  emberCommandEntryAction("rnd_error", emAfPluginSimpleMeteringServerCliRandomError, "u", ""),
  emberCommandEntryAction("set_error", emAfPluginSimpleMeteringServerCliSetError, "uu", ""),
#endif
  emberCommandEntryAction("profiles", emAfPluginSimpleMeteringServerCliProfiles, "u", ""),
#endif //EMBER_AF_PLUGIN_SIMPLE_METERING_SERVER_TEST_METER_ENABLE
  emberCommandEntryTerminator(),
};

#endif // EMBER_AF_GENERATE_CLI

#ifdef EMBER_AF_PLUGIN_SIMPLE_METERING_SERVER_TEST_METER_ENABLE
static uint8_t getEndpointArgument(uint8_t index)
{
  uint8_t endpoint = (uint8_t)emberUnsignedCommandArgument(index);
  return (endpoint == 0
          ? emberAfPrimaryEndpointForCurrentNetworkIndex()
          : endpoint);
}

// plugin simple-metering-server print
void emAfPluginSimpleMeteringServerCliPrint(void)
{
  afTestMeterPrint();
}

// plugin simple-metering-server rate <int:2>
void emAfPluginSimpleMeteringServerCliRate(void)
{
  afTestMeterSetConsumptionRate((uint16_t)emberUnsignedCommandArgument(0),
                                (uint8_t)emberUnsignedCommandArgument(1));
}

// plugin simple-metering-server variance <int:2>
void emAfPluginSimpleMeteringServerCliVariance(void)
{
  afTestMeterSetConsumptionVariance((uint16_t)emberUnsignedCommandArgument(0));
}

// plugin simple-metering-server adjust <endpoint:1>
void emAfPluginSimpleMeteringServerCliAdjust(void)
{
  afTestMeterAdjust(getEndpointArgument(0));
}

// plugin simple-metering-server off <endpoint: 1>
void emAfPluginSimpleMeteringServerCliOff(void)
{
  afTestMeterMode(getEndpointArgument(0), 0);
}

// plugin simple-metering-server electric <endpoint:1>
void emAfPluginSimpleMeteringServerCliElectric(void)
{
#if (EMBER_AF_PLUGIN_SIMPLE_METERING_SERVER_TEST_METER_TYPE == EMBER_AF_PLUGIN_SIMPLE_METERING_SERVER_ELECTRIC_METER)
  afTestMeterMode(getEndpointArgument(0), 1);
#else
  emberAfCorePrintln("Not applicable for a non Electric Meter.");
#endif
}

// plugin simple-metering-server gas <endpoint:1>
void emAfPluginSimpleMeteringServerCliGas(void)
{
//#if (EMBER_AF_PLUGIN_SIMPLE_METERING_SERVER_TEST_METER_TYPE == EMBER_AF_PLUGIN_SIMPLE_METERING_SERVER_GAS_METER)
  afTestMeterMode(getEndpointArgument(0), 2);
// #else
//   emberAfCorePrintln("Not applicable for a non Gas Meter.");
// #endif
}

#ifdef EMBER_AF_PLUGIN_SIMPLE_METERING_SERVER_TEST_METER_ERRORS
// plugin simple-metering-server rnd_error <data:1>
void emAfPluginSimpleMeteringServerCliRandomError(void)
{
  // enables random error setting at each tick
  afTestMeterRandomError((uint8_t)emberUnsignedCommandArgument(0));
}

// plugin simple-metering-server set_error <data:1> <endpoint:1>
void emAfPluginSimpleMeteringServerCliSetError(void)
{
  // sets error, in the process overriding random_error
  afTestMeterSetError(getEndpointArgument(1),
                      (uint8_t)emberUnsignedCommandArgument(0));
}
#endif //EMBER_AF_PLUGIN_SIMPLE_METERING_SERVER_TEST_METER_ERRORS

// plugin simple-metering-server profiles <data:1>
void emAfPluginSimpleMeteringServerCliProfiles(void)
{
#if (EMBER_AF_PLUGIN_SIMPLE_METERING_SERVER_TEST_METER_PROFILES != 0)
  afTestMeterEnableProfiles((uint8_t)emberUnsignedCommandArgument(0));
#else
  emberAfCorePrintln("Not applicable for 0 configured profiles.");
#endif
}
#endif //EMBER_AF_PLUGIN_SIMPLE_METERING_SERVER_TEST_METER_ENABLE

void emAfPluginSimpleMeteringServerCliConfigureMirror(void)
{
  EmberNodeId dstAddr = (EmberNodeId)emberUnsignedCommandArgument(0);
  uint8_t srcEndpoint =  (uint8_t)emberUnsignedCommandArgument(1);
  uint8_t dstEndpoint =  (uint8_t)emberUnsignedCommandArgument(2);
  uint32_t issuerId = (uint32_t)emberUnsignedCommandArgument(3);
  uint32_t reportingInterval = (uint32_t)emberUnsignedCommandArgument(4);
  uint8_t mirrorNotificationReporting = (uint8_t)emberUnsignedCommandArgument(5);
  uint8_t notificationScheme = (uint8_t)emberUnsignedCommandArgument(6);

  emberAfFillCommandSimpleMeteringClusterConfigureMirror(issuerId,
                                                         reportingInterval,
                                                         mirrorNotificationReporting,
                                                         notificationScheme);
  emberAfSetCommandEndpoints(srcEndpoint, dstEndpoint);
  emberAfSendCommandUnicast(EMBER_OUTGOING_DIRECT, dstAddr);
}

void emAfPluginSimpleMeteringCliDisableFastPolling(void)
{
  uint8_t toggleFastPolling = (uint8_t)emberUnsignedCommandArgument(0);
  emAfToggleFastPolling(toggleFastPolling);
}
