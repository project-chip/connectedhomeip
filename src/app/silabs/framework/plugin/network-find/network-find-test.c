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
 * @brief Form-and-join and network-find plugin test code.
 *******************************************************************************
   ******************************************************************************/

__sed_replace_me_header__

// The next line will be replaced at Makefile time with either #define SOC_TESTING or #define EZSP_HOST
  __sed_replace_me_define__

#if !defined(EZSP_HOST) && !defined(SOC_TESTING)
  #error "Error with network-find-test conversion.  Neither EZSP_HOST or SOC_TESTING defined."
#endif

#if defined(EZSP_HOST) && defined(SOC_TESTING)
  #error "Error with network-find-test conversion.  Both EZSP_HOST and SOC_TESTING defined."
#endif

// This also sets up the Stack Profile, which must be 2 for this testing
#define EMBER_AF_HAS_SECURITY_PROFILE_HA

#include "app/framework/include/af.h"
#include "app/framework/util/common.h"
#include "callback.h"
#include "app/framework/plugin/ota-storage-common/ota-storage.h"
#include "app/framework/plugin/ota-client-policy/ota-client-policy.h"
#include "app/framework/plugin/ota-client/ota-client-signature-verify.h"
#include "app/framework/plugin/ota-client/ota-client-page-request-test.h"

#include "app/framework/test/test-framework.h"
#include "app/framework/plugin/ota-common/ota.h"

#include "app/util/ezsp/ezsp-enum.h"
#include "app/util/common/form-and-join.h"
#include "app/util/ezsp/ezsp-host-configuration-defaults.h"

#if defined(SOC_TESTING)

// Yes, include the C file directly.  This is because we don't have an application
// configuration header and we don't compile a separate copy for each
// scripted test that might use this C file.
#include "config/ember-configuration.c"

#endif

//------------------------------------------------------------------------------
// Globals

static EmberStatus formNetworkReturnCode = EMBER_SUCCESS;
static EmberStatus joinNetworkReturnCode = EMBER_SUCCESS;
static EmberStatus emberStartScanReturnCode = EMBER_SUCCESS;

#define PAN_FOUND_DEFAULT 0x0001

// Defaults can be found in testEnd()
static EmberZigbeeNetwork foundNetwork;
static int8_t lqi;
static int8_t rssi;

static uint16_t chosenPanId = 0xFFFF;
static bool startScanExpected = false;
static EmberStatus scanErrorHandlerStatus = 0xFF;

// SE channel mask: 11, 14, 15, 19, 20, 24, 25
uint32_t testFrameworkChannelMask = 0x0318C800UL;

#define ENERGY_LEVEL 20

// This 'random' data is sized to hold enough for:
//   1 random channel selected from the list of energy scan results
//   16 panIds needed by the form-and-join code as potential form candidates
static uint16_t randomValues[] = {
  0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16
};
static int8_t randomValuesIndex = -1;

static bool formNetworkExpected = false;

//------------------------------------------------------------------------------
// Forward declarations

// Definitons normally in network-find.c
extern uint8_t state;
#define NETWORK_FIND_NONE 0

// From form-and-join-node-adapter.c
#if defined(SOC_TESTING)
extern EmberMessageBuffer dataCache;
extern EmberEventControl cleanupEvent;
#endif

//------------------------------------------------------------------------------
// Stubs

void emberCalibrateCurrentChannel(void)
{
}

void emberRunEvents(EmberEventData *events)
{
  assert("emberRunEvents() should never be called" == 0);
}

EmberTaskId emberTaskInit(EmberEventData *events)
{
  assert("emberTaskInit() should never be called" == 0);
  return 0;
}

void emberRunTask(EmberTaskId taskid)
{
  assert("emberRunTask() should never be called" == 0);
}

void emberCounterHandler(EmberCounterType type, EmberCounterInfo info)
{
}

bool emberMarkTaskIdle(EmberTaskId taskid)
{
  assert("emberMarkTaskIdle() should never be called" == 0);
  return false;
}

EmberStatus emberAfFormNetwork(EmberNetworkParameters *parameters)
{
  debug("emberAfFormNetwork() PAN ID: 0x%04X, returning: 0x%02X\n",
        parameters->panId,
        formNetworkReturnCode);
  expectMessage(formNetworkExpected, "formNetworkExpected != true\n");
  formNetworkExpected = false;
  assert(chosenPanId == parameters->panId);
  return formNetworkReturnCode;
}

EmberStatus emberAfJoinNetwork(EmberNetworkParameters *parameters)
{
  debug("emberAfJoinNetwork() PAN ID: 0x%04X, returning: 0x%02X\n",
        parameters->panId,
        joinNetworkReturnCode);
  expectMessage(chosenPanId == parameters->panId, "chosenPanId != parameters->panId\n");
  chosenPanId = 0xFFFF;
  return joinNetworkReturnCode;
}

void emberAfPluginNetworkFindFinishedCallback(EmberStatus status)
{
}

void emberUnusedPanIdFoundHandler(EmberPanId panId, uint8_t channel)
{
  emberAfUnusedPanIdFoundCallback(panId, channel);
}

void emberJoinableNetworkFoundHandler(EmberZigbeeNetwork *networkFound,
                                      uint8_t lqi,
                                      int8_t rssi)
{
  emberAfJoinableNetworkFoundCallback(networkFound, lqi, rssi);
}

void emberScanErrorHandler(EmberStatus status)
{
  debug("emberScanErrorHandler(): 0x%02X\n", status);
  expect(status == scanErrorHandlerStatus);
  scanErrorHandlerStatus = 0xFF;
  emberAfScanErrorCallback(status);
}

EmberStatus emberStartScan(uint8_t scanType, uint32_t channelMask, uint8_t duration)
{
  debug("emberStartScan() type:0x%02X, channelMask 0x%08X, duration: 0x%02X\n",
        scanType,
        channelMask,
        duration);
  expectMessage(startScanExpected, "startScaneExpected != true");
  startScanExpected = false;
  return emberStartScanReturnCode;
}

uint16_t halCommonGetRandomTraced(char *file, int line)
{
  randomValuesIndex++;
  // If we run out of random values, it means the code is asking for
  // more than we expect.  Probably need to re-evaluate how many random
  // values we provide.
  expectMessage(randomValuesIndex <= (sizeof(randomValues) / 2),
                "Ran out of random values!");
//  debug("halCommonGetRandomTraced(): value: %d\n",
//        randomValues[randomValuesIndex]);
  return randomValues[randomValuesIndex];
}

// A copy from network-cli.c
void emberAfPrintChannelListFromMask(uint32_t channelMask)
{
  uint8_t i;
  bool firstPrint = true;
  channelMask >>= 11;           // valid 802.15.4 channels start from 11
  for (i = 11; i <= 26; i++) {
    if (channelMask & 0x01UL) {
      emberAfAppPrint("%c %d",
                      (firstPrint
                       ? ' '
                       : ','),
                      i);
      firstPrint = false;
      emberAfAppFlush();
    }
    channelMask >>= 1;
  }
}

//------------------------------------------------------------------------------
// SOC vs. Host code

#if defined(SOC_TESTING)

void networkFoundHandler(EmberZigbeeNetwork* network,
                         uint8_t lastHopLqi,
                         int8_t lastHopRssi)
{
  emberNetworkFoundHandler(network);
}

EmberStatus emberGetLastHopRssi(int8_t* lastHopRssi)
{
  *lastHopRssi = rssi;
  return EMBER_SUCCESS;
}

EmberStatus emberGetLastHopLqi(uint8_t *lastHopLqi)
{
  *lastHopLqi = lqi;
  return EMBER_SUCCESS;
}

void simulateScanComplete(void)
{
  emberScanCompleteHandler(foundNetwork.channel, EMBER_SUCCESS);
}

void simulateEnergyScanResultHandler(uint8_t channel)
{
  emberEnergyScanResultHandler(channel, ENERGY_LEVEL);
}

void platformSpecificTestEnd(void)
{
  expect(dataCache == EMBER_NULL_MESSAGE_BUFFER);
  expect(cleanupEvent.status == EMBER_EVENT_INACTIVE);
}

//-----------------------
// EZSP_HOST

#else

EzspStatus ezspGetConfigurationValue(EzspConfigId configId,
                                     uint16_t *value)
{
  expectMessage(configId == EZSP_CONFIG_STACK_PROFILE,
                "Unexpected ezspGetConfigurationValue() config id\n");
  *value = 2;
  return EZSP_SUCCESS;
}

void networkFoundHandler(EmberZigbeeNetwork* network,
                         uint8_t lastHopLqi,
                         int8_t lastHopRssi)
{
  ezspNetworkFoundHandler(network,
                          lqi,
                          rssi);
}

void simulateScanComplete(void)
{
  ezspScanCompleteHandler(foundNetwork.channel, EMBER_SUCCESS);
  note(".");
}

void simulateEnergyScanResultHandler(uint8_t channel)
{
  ezspEnergyScanResultHandler(channel, ENERGY_LEVEL);
}

void platformSpecificTestEnd(void)
{
}

#endif

//------------------------------------------------------------------------------
// Real functionality

static void simulateNetworkFound(void)
{
  networkFoundHandler(&foundNetwork,
                      lqi,
                      rssi);

  // Increment PAN and extended PAN ID so the data is not duplicated and
  // the form-and-join code must remember them all.

  foundNetwork.panId++;
  uint16_t extendedPan = (((uint16_t)foundNetwork.extendedPanId[6] << 8)
                          + foundNetwork.extendedPanId[7]);
  extendedPan++;
  foundNetwork.extendedPanId[6] = (uint8_t)(extendedPan >> 8);
  foundNetwork.extendedPanId[7] = (uint8_t)(extendedPan);
  note(".");
}

#define testEnd() testEndFunction(true, true)

static void testEndFunction(bool callNetworkFindComplete,
                            bool printOn)
{
  uint8_t xpan[] = {
    0, 0, 0, 0, 0, 0, 0, 1
  };

  if (!printOn) {
    testFrameworkAllPrintingOff = true;
  }

  foundNetwork.channel = 11;
  foundNetwork.panId = PAN_FOUND_DEFAULT;
  foundNetwork.allowingJoin = true;
  foundNetwork.stackProfile = 2;
  foundNetwork.nwkUpdateId = 0;
  MEMMOVE(foundNetwork.extendedPanId, xpan, EUI64_SIZE);

  lqi = 1;
  rssi = 1;

  randomValuesIndex = -1;

  emberStartScanReturnCode = EMBER_SUCCESS;

  if (callNetworkFindComplete) {
    emberAfNetworkFindOperationCompleteCallback();
  }
  expect(state == NETWORK_FIND_NONE);
  expect(!formNetworkExpected);
  expect(!startScanExpected);

  platformSpecificTestEnd();

  debug("\n*** End Test \n\n");

  if (!printOn) {
    testFrameworkAllPrintingOff = false;
  }
}

static void formTest(void)
{
  uint8_t i;
  uint8_t testCase = 1;
  debug("\n");

  {
    debug("*** Test case %d: Simple Success (find an unused pan)\n", testCase++);

    // First we do an energy scan of all channels in our list
    startScanExpected = true;
    assert(EMBER_SUCCESS == emberAfFindUnusedPanIdAndFormCallback());
    note(".");
    for (i = 11; i <= 26; i++) {
      if (EMBER_AF_PLUGIN_NETWORK_FIND_CHANNEL_MASK & (1 << i)) {
        simulateEnergyScanResultHandler(i);
        note(".");
      }
    }

    // Now a PAN ID scan
    startScanExpected = true;
    simulateScanComplete();

    // No networks found at all (no conflicting PAN IDs)

    chosenPanId = randomValues[1];  // see 'randomValues' declaration
                                    // for why this is '1'
    formNetworkExpected = true;
    simulateScanComplete();
    assert(!formNetworkExpected);

    testEnd();
  }

  {
    debug("*** Test case %d: No energy scan results (error)\n", testCase++);
    startScanExpected = true;
    assert(EMBER_SUCCESS == emberAfFindUnusedPanIdAndFormCallback());
    note(".");
    scanErrorHandlerStatus = EMBER_ERR_FATAL;

    // energy scan handler is never fired, so no results.
    simulateScanComplete();

    testEndFunction(false,  // call network find complete?  No, should not be necessary.
                    true);  // print test end?
  }

  {
    debug("*** Test case %d: Energy scan failed to start\n", testCase++);
    startScanExpected = true;
    emberStartScanReturnCode = EMBER_ERR_FATAL;
    scanErrorHandlerStatus = emberStartScanReturnCode;
    assert(EMBER_SUCCESS != emberAfFindUnusedPanIdAndFormCallback());
    testEndFunction(false,  // call network find complete?  No, should not be necessary.
                    true);  // print test end?
  }

  {
    debug("*** Test case %d: PAN ID scan failed to start\n", testCase++);
    startScanExpected = true;
    assert(EMBER_SUCCESS == emberAfFindUnusedPanIdAndFormCallback());
    note(".");
    for (i = 11; i <= 26; i++) {
      if (EMBER_AF_PLUGIN_NETWORK_FIND_CHANNEL_MASK & (1 << i)) {
        simulateEnergyScanResultHandler(i);
        note(".");
      }
    }

    startScanExpected = true;
    emberStartScanReturnCode = EMBER_ERR_FATAL;
    scanErrorHandlerStatus = emberStartScanReturnCode;
    simulateScanComplete();

    testEndFunction(false,  // call network find complete?  No, should not be necessary.
                    true);  // print test end?
  }

  {
    debug("*** Test case %d: Multiple conflicting PANs\n", testCase++);

    startScanExpected = true;
    assert(EMBER_SUCCESS == emberAfFindUnusedPanIdAndFormCallback());
    note(".");
    for (i = 11; i <= 26; i++) {
      if (EMBER_AF_PLUGIN_NETWORK_FIND_CHANNEL_MASK & (1 << i)) {
        simulateEnergyScanResultHandler(i);
        note(".");
      }
    }
    startScanExpected = true;
    simulateScanComplete();

    // All the 'randomly' chosen PAN IDs, except for the last one,
    // conflict.
    for (i = 1; i < (sizeof(randomValues) / 2) - 1; i++) {
      foundNetwork.panId = randomValues[i];
      simulateNetworkFound();
    }
    chosenPanId = randomValues[i];
    formNetworkExpected = true;
    simulateScanComplete();
    assert(!formNetworkExpected);
    testEnd();
  }

  {
    debug("*** Test case %d: ALL conflicting PANs\n", testCase++);

    startScanExpected = true;
    assert(EMBER_SUCCESS == emberAfFindUnusedPanIdAndFormCallback());
    note(".");
    for (i = 11; i <= 26; i++) {
      if (EMBER_AF_PLUGIN_NETWORK_FIND_CHANNEL_MASK & (1 << i)) {
        simulateEnergyScanResultHandler(i);
        note(".");
      }
    }
    startScanExpected = true;
    simulateScanComplete();

    // All the 'randomly' chosen PAN IDs conflict.
    for (i = 1; i < (sizeof(randomValues) / 2); i++) {
      foundNetwork.panId = randomValues[i];
      simulateNetworkFound();
    }

    // We now expect a new PAN ID scan to commence with new set of random IDs
    // choosen that hopefully don't conflict.
    debug("  All random chosen PAN IDs conflict, so a new PAN ID scan should start.\n");
    startScanExpected = true;

    // This is set to 0, instead of -1, because form-and-join doesn't need to randomly
    // choose a channel from the list of energy scan results.  Only a PAN ID scan
    // will be re-executed.
    randomValuesIndex = 0;
    simulateScanComplete();

    // All the 'randomly' chosen PAN IDs, except for the last one,
    // conflict.
    for (i = 1; i < (sizeof(randomValues) / 2) - 1; i++) {
      foundNetwork.panId = randomValues[i];
      simulateNetworkFound();
    }
    chosenPanId = randomValues[i];
    formNetworkExpected = true;
    simulateScanComplete();
    assert(!formNetworkExpected);
    testEnd();
  }

  {
    debug("*** Test case %d: Too many networks\n", testCase++);

    debug("  More networks heard than we have ability to save in our cache.\n");

    startScanExpected = true;
    assert(EMBER_SUCCESS == emberAfFindUnusedPanIdAndFormCallback());
    note(".");
    for (i = 11; i <= 26; i++) {
      if (EMBER_AF_PLUGIN_NETWORK_FIND_CHANNEL_MASK & (1 << i)) {
        simulateEnergyScanResultHandler(i);
        note(".");
      }
    }
    startScanExpected = true;
    simulateScanComplete();

    for (i = 0; i < EZSP_HOST_FORM_AND_JOIN_BUFFER_SIZE + 5; i++) {
      foundNetwork.panId = 0x1000 + i;
      simulateNetworkFound();
    }
    chosenPanId = randomValues[1];  // see 'randomValues' declaration
                                    // for why this is '1'
    formNetworkExpected = true;
    simulateScanComplete();
    assert(!formNetworkExpected);
    testEnd();
  }
}

static void joinTest(void)
{
  debug("\n");

  uint8_t testCase = 1;
  uint8_t i;

  {
    debug("*** Test Case %d: Simple Success (channel 11)\n", testCase++);
    startScanExpected = true;
    assert(EMBER_SUCCESS == emberAfStartSearchForJoinableNetworkCallback());
    note(".");
    assert(!startScanExpected);

    debug("Attempt to start new scan while network-find ongoing...\n");
    assert(EMBER_SUCCESS != emberAfStartSearchForJoinableNetworkCallback());
    note(".");

    // Open network found
    simulateNetworkFound();

    chosenPanId = PAN_FOUND_DEFAULT;
    simulateScanComplete();

    testEnd();
  }

  {
    debug("*** Test Case %d: No networks at all\n", testCase++);
    startScanExpected = true;

    // this will scan channel 11
    assert(EMBER_SUCCESS == emberAfStartSearchForJoinableNetworkCallback());
    note(".");
    assert(!startScanExpected);

    startScanExpected = true;
    for (i = 12; i <= 26; i++) {
      foundNetwork.channel = i;
      if (EMBER_AF_PLUGIN_NETWORK_FIND_CHANNEL_MASK & (1 << i)) {
        startScanExpected = true;
        simulateScanComplete();
        assert(!startScanExpected);
      }
    }
    scanErrorHandlerStatus = EMBER_NO_BEACONS;
    simulateScanComplete();
    assert(!startScanExpected);

    testEnd();
  }

  {
    debug("*** Test Case %d: No networks allowing joining\n",
          testCase++);
    startScanExpected = true;

    // this will scan channel 11
    assert(EMBER_SUCCESS == emberAfStartSearchForJoinableNetworkCallback());
    note(".");

    foundNetwork.allowingJoin = false;

    startScanExpected = true;
    for (i = 12; i <= 26; i++) {
      foundNetwork.channel = i;

      if (EMBER_AF_PLUGIN_NETWORK_FIND_CHANNEL_MASK & (1 << i)) {
        simulateNetworkFound();
        startScanExpected = true;
        simulateScanComplete();
        assert(!startScanExpected);
      }
    }
    scanErrorHandlerStatus = EMBER_NO_BEACONS;
    simulateScanComplete();
    assert(!startScanExpected);

    testEnd();
  }

  // Bug 13836
  {
    debug("*** Test Case %d: First attempt to scan failed because emberStartScan() fails \n",
          testCase++);
    startScanExpected = true;
    emberStartScanReturnCode = EMBER_ERR_FATAL;
    scanErrorHandlerStatus = emberStartScanReturnCode;
    assert(EMBER_SUCCESS != emberAfStartSearchForJoinableNetworkCallback());
    assert(!startScanExpected);
    note(".");
    testEndFunction(false,  // call network-find complete?  NO, should not be necessary
                    true);  // print test end?
  }

  {
    debug("*** Test Case %d: Subsequent attempt to scan failed because emberStartScan() fails \n",
          testCase++);
    startScanExpected = true;
    assert(EMBER_SUCCESS == emberAfStartSearchForJoinableNetworkCallback());
    note(".");

    // No networks found

    // Attempt to start scanning on next channel will fail
    emberStartScanReturnCode = EMBER_ERR_FATAL;
    scanErrorHandlerStatus = emberStartScanReturnCode;
    startScanExpected = true;

    simulateScanComplete();

    testEndFunction(false,  // call network-find complete?  NO, should not be necessary
                    true);  // print test end?
  }
}

#if defined(SOC_TESTING)
  #define FORM_TEST_COMMAND "soc-form"
  #define JOIN_TEST_COMMAND "soc-join"

#else // EZSP_HOST
  #define FORM_TEST_COMMAND "host-form"
  #define JOIN_TEST_COMMAND "host-join"
#endif

static uint8_t nullExtendedPanId[EXTENDED_PAN_ID_SIZE] = { 0, 0, 0, 0, 0, 0, 0, 0 };

int main(int argc, char* argv[])
{
  const TestCase allTests[] = {
    { FORM_TEST_COMMAND, formTest },
    { JOIN_TEST_COMMAND, joinTest },
    { NULL },
  };

  testEndFunction(false, false);  // re-initalize state of test

  // Clear the extended PAN ID so we will attempt to join any network.
  emberAfSetFormAndJoinExtendedPanIdCallback(nullExtendedPanId);

  // This controls whether the software workaround is enabled
  // for a 2xx chip bug.  Let's keep it simple and assume 'no'.
  // If we don't then there are multiple channel scans for
  // each channel and that requires more knowledge of the internals
  // of form-and-join.
  emberEnableDualChannelScan = false;

  return parseCommandLineAndExecuteTest(argc,
                                        argv,
                                        "af-network-find-test",
                                        allTests);
}
