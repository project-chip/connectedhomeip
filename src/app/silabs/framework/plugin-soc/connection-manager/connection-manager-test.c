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
 * @brief Generic temperature measurement server implementation.  This requires and
 * additional hardware temperature sensor plugin to provide initialization and
 * read functions.
 *******************************************************************************
   ******************************************************************************/

#include "app/framework/include/af.h"
#include "app/framework/test/test-framework.h"
#include "app/framework/plugin-soc/connection-manager/connection-manager.h"
#include "app/framework/plugin-soc/connection-manager/connection-manager-test.h"

// emulated state variables
bool mfgLibRunning = false;
bool mfgLibIsEnabled = false;
bool bindingTableClear = false;
bool reportTableClear = false;
uint8_t attributeResetCount = 0;
uint8_t clusterClearGroupTableCount = 0;
uint8_t clusterClearSceneTableCount = 0;
bool leaveNetworkCalled = false;
bool startSearchCalled = false;
uint16_t *currentBlinkPattern;
uint8_t currentBlinkPatternLength;

uint8_t currentEndpoint = 1;
EmberStatus networkStatus;

// State variables for controlling LED blink behavior on network join/leave
#define LED_LOST_ON_TIME_MS          250
#define LED_LOST_OFF_TIME_MS         750
#define LED_BLINK_ON_TIME_MS         200
#define LED_SEARCH_BLINK_OFF_TIME_MS 1800
#define LED_FOUND_BLINK_OFF_TIME_MS  250
#define LED_FOUND_BLINK_ON_TIME_MS   250
#define LED_IDENTIFY_ON_TIME_MS      250
#define LED_IDENTIFY_OFF1_TIME_MS    250
#define LED_IDENTIFY_OFF2_TIME_MS    1250
#define DEFAULT_NUM_JOIN_BLINKS      6
#define DEFAULT_NUM_LEAVE_BLINKS     3
#define DEFAULT_NUM_SEARCH_BLINKS    100
#define DEFAULT_NUM_IDENTIFY_BLINKS  100

#define NUM_ENDPOINTS                3

uint16_t networkLostBlinkPattern[] = {
  LED_LOST_ON_TIME_MS, LED_LOST_OFF_TIME_MS
};
uint16_t networkSearchBlinkPattern[] =
{ LED_BLINK_ON_TIME_MS, LED_SEARCH_BLINK_OFF_TIME_MS };
uint16_t networkFoundBlinkPattern[] =
{ LED_FOUND_BLINK_ON_TIME_MS, LED_FOUND_BLINK_OFF_TIME_MS };
uint16_t networkIdentifyBlinkPattern[] =
{
  LED_IDENTIFY_ON_TIME_MS,
  LED_IDENTIFY_OFF1_TIME_MS,
  LED_IDENTIFY_ON_TIME_MS,
  LED_IDENTIFY_OFF2_TIME_MS
};

// Emulated functions from HAL plugins
void halLedBlinkLedOff(uint8_t time)
{
  return;
}

void halLedBlinkPattern(uint8_t count, uint8_t length, uint16_t *pattern)
{
  currentBlinkPattern = pattern;
  currentBlinkPatternLength = length;
  return;
}

void halInternalGetTokenData(void *data, uint16_t token, uint8_t index, uint8_t len)
{
  return;
}

bool emberAfMfglibEnabled(void)
{
  return mfgLibIsEnabled;
}

bool emberAfMfglibRunning(void)
{
  return mfgLibRunning;
}

void emberAfPluginConnectionManagerStartNetworkSearchCallback(void)
{
  return;
}

void emberAfPluginConnectionManagerLeaveNetworkCallback(void)
{
  return;
}

void emberAfPluginConnectionManagerStackStatusCallback(EmberStatus status);

EmberStatus emberAfPluginNetworkSteeringStart(void)
{
  expectComparisonDecimal(
    1,
    1,
    "got",
    "here");
  startSearchCalled = true;
  return networkStatus;
}

EmberStatus emberClearBindingTable(void)
{
  bindingTableClear = true;
  return EMBER_SUCCESS;
}

EmberStatus emberAfClearReportTableCallback(void)
{
  reportTableClear = true;
  return EMBER_SUCCESS;
}

void emberAfGroupsClusterClearGroupTableCallback(uint8_t endpoint)
{
  expectComparisonDecimal(
    currentEndpoint,
    endpoint,
    "currentEndpoint",
    "endpointParameter");
  clusterClearGroupTableCount++;
  return;
}

void emberAfScenesClusterClearSceneTableCallback(uint8_t endpoint)
{
  expectComparisonDecimal(
    currentEndpoint,
    endpoint,
    "currentEndpoint",
    "endpointParameter");
  clusterClearSceneTableCount++;
  return;
}

uint8_t emberAfEndpointCount()
{
  return NUM_ENDPOINTS;
}

EmberStatus emberAfResetAttributes(uint8_t endpoint)
{
  expectComparisonDecimal(
    currentEndpoint,
    endpoint,
    "currentEndpoint",
    "endpointParameter");
  attributeResetCount++;
  return EMBER_SUCCESS;
}

bool emberAfStartMoveCallback(void)
{
  expectComparisonDecimal(
    1,
    0,
    "got",
    "startMoveCallback");
  return true;
}

EmberStatus emberLeaveNetwork(void)
{
  leaveNetworkCalled = true;

  // We're going to assume that after an emberLeaveNetwork,
  // emberAfPluginConnectionManagerStackStatusCallback(EMBER_NETWORK_DOWN) will
  // eventually be called
  emberAfPluginConnectionManagerStackStatusCallback(EMBER_NETWORK_DOWN);
  return EMBER_SUCCESS;
}

uint8_t emberAfEndpointFromIndex(uint8_t idx)
{
  currentEndpoint = idx + 1;
  return currentEndpoint;
}

// Emulated callbacks
void emberAfPluginConnectionManagerFinishedCallback(EmberStatus status)
{
  return;
}

static void lostTest(void)
{
  bindingTableClear = false;
  reportTableClear = false;
  clusterClearGroupTableCount = 0;
  attributeResetCount = 0;
  clusterClearSceneTableCount = 0;

  testAfSetNetworkState(EMBER_NO_NETWORK);
  emberAfPluginConnectionManagerStackStatusCallback(EMBER_NETWORK_DOWN);

  // When the network goes down, verify that:
  //    A new network search is started
  //    The binding table is cleared
  //    the report table is cleared
  //    all attributes of all endpoints are cleared
  //    all group tables of all endpoints are cleared
  //    all scene tables of all endpoints are cleared

  expectComparisonDecimal(true,
                          bindingTableClear,
                          "expected binding table state",
                          "actual binding table state");
  expectComparisonDecimal(true,
                          reportTableClear,
                          "expected reporting table state",
                          "actual reporting table state");
  expectComparisonDecimal(NUM_ENDPOINTS,
                          clusterClearGroupTableCount,
                          "exp group tables cleared",
                          "actual group tables cleared");
  expectComparisonDecimal(NUM_ENDPOINTS,
                          attributeResetCount,
                          "exp attributes with cleared clusters",
                          "actual attributes with cleared clusters");
  expectComparisonDecimal(NUM_ENDPOINTS,
                          clusterClearSceneTableCount,
                          "exp scene tables cleared",
                          "actual scene tables cleared");
}

static void joinTest(void)
{
  emberAfPluginConnectionManagerStackStatusCallback(EMBER_NETWORK_UP);
}

static void leaveNetTest(void)
{
  leaveNetworkCalled = false;
  testAfSetNetworkState(EMBER_JOINED_NETWORK);
  emberAfPluginConnectionManagerLeaveNetworkAndStartSearchForNewOne();

  expectComparisonDecimal(
    true,
    leaveNetworkCalled,
    "expected leave network state",
    "observed leaved network state");

  leaveNetworkCalled = false;
  testAfSetNetworkState(EMBER_JOINED_NETWORK_NO_PARENT);
  emberAfPluginConnectionManagerLeaveNetworkAndStartSearchForNewOne();
  expectComparisonDecimal(
    true,
    leaveNetworkCalled,
    "expected leave network state",
    "observed leaved network state");
}

static void leaveNoNetTest(void)
{
  startSearchCalled = false;
  testAfSetNetworkState(EMBER_NO_NETWORK);
  emberAfPluginConnectionManagerLeaveNetworkAndStartSearchForNewOne();

  expectComparisonDecimal(
    true,
    startSearchCalled,
    "expected startSearch state",
    "observed startSearch state");
}

int main(int argc, char* argv[])
{
  const TestCase tests[] = {
    { "join", joinTest },
    { "lost", lostTest },
    { "leaveNoNet", leaveNoNetTest },
    { "leaveNet", leaveNetTest },
    { NULL },
  };
  return parseCommandLineAndExecuteTest(argc, argv, "Security Sensor", tests);
}
