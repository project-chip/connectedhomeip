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
 * @brief Generic security sensor interface code.
 *
 * Implements all unit tests for the security sensor plugin.
 *******************************************************************************
   ******************************************************************************/

#include "app/framework/include/af.h"
#include "util/plugin/plugin-common/gpio-sensor/gpio-sensor.h"
#include "app/framework/test/test-framework.h"

#define CONTACT_ALARM 0x01
#define TAMPER_ALARM  0x04

HalGpioSensorState contactSensorState = HAL_GPIO_SENSOR_ACTIVE;
uint8_t expectedAlarmState = 0;
bool messageSent;
bool initReportEventIsActive = false;

// *****************************************************************************
// Forward declarations
// *****************************************************************************

// Variables from security-sensor.c we need access to
extern EmberEventControl emberAfPluginSecuritySensorInitialReportEventControl;

// callbacks the security sensor consumes that we will be using as stimulus
void emberAfPluginGpioSensorStateChangedCallback(uint8_t newStatus);
void emberAfPluginTamperSwitchTamperActiveCallback(void);
void emberAfPluginTamperSwitchTamperAlarmCallback(void);
void emberAfPluginSecuritySensorStackStatusCallback(EmberStatus status);
void emberAfPluginSecuritySensorInitialReportEventHandler(void);

// *****************************************************************************
// Stubbed out functions from other plugins
// *****************************************************************************

void emberAfPluginConnectionManagerLedNetworkFoundBlink(void)
{
}

bool emberAfStartMoveCallback(void)
{
  return true;
}

void emberAfPluginConnectionManagerResetJoinAttempts(void)
{
}

uint8_t emberAfEndpointFromIndex(uint8_t idx)
{
  return 1;
}

bool emberAfContainsServer(uint8_t endpoint, EmberAfClusterId clusterId)
{
  return true;
}

uint8_t emberAfEndpointCount()
{
  return 1;
}

EmberStatus emberAfPluginIasZoneServerUpdateZoneStatus(
  uint8_t endpoint,
  uint16_t newStatus,
  uint8_t timeSinceStatusOccurredQs)
{
  debug("Message for you");
  expectComparisonDecimal(expectedAlarmState,
                          newStatus,
                          "expectedAlarmState",
                          "newStatus");
  messageSent = true;
  return EMBER_SUCCESS;
}

uint16_t halCommonGetInt16uMillisecondTick(void)
{
  return 0x8000;
}

void halLedBlinkSetActivityLed(uint8_t led)
{
}

HalGpioSensorState halGpioSensorGetSensorValue(void)
{
  return contactSensorState;
}

void emberAfPluginConnectionManagerLeaveNetworkAndStartSearchForNewOne(void)
{
}

void emberAfPluginSecuritySensorStatusChangedCallback(uint16_t status)
{
  return;
}
// *****************************************************************************
// Unit test functions
// *****************************************************************************

static void joinMsgTest(uint8_t initAlarmState)
{
  //expectedAlarmState = initAlarmState;

  // Each time the callback is hit, an emberAfPluginIasZoneServerUpdateZoneStatus
  // will be triggered, so it is necessary to update the expected alarm state one bit at
  // a time.
  if (initAlarmState & CONTACT_ALARM) {
    expectedAlarmState |= CONTACT_ALARM;
    contactSensorState = 1;
    emberAfPluginGpioSensorStateChangedCallback(HAL_GPIO_SENSOR_ACTIVE);
  } else {
    expectedAlarmState = 0;
    contactSensorState = 0;
    emberAfPluginGpioSensorStateChangedCallback(HAL_GPIO_SENSOR_NOT_ACTIVE);
  }

  if (initAlarmState & TAMPER_ALARM) {
    expectedAlarmState |= TAMPER_ALARM;
    emberAfPluginTamperSwitchTamperAlarmCallback();
  } else {
    emberAfPluginTamperSwitchTamperActiveCallback();
  }

  // Now that we have an initial hardware state, fake a network up and verify
  // that a new packet is sent.
  messageSent = false;
  emberAfPluginSecuritySensorStackStatusCallback(EMBER_NETWORK_UP);

  // After verifying the initReportEvent is active, we can call the handler
  emberAfPluginSecuritySensorInitialReportEventHandler();
  expectComparisonDecimal(true,
                          messageSent,
                          "true",
                          "messageSent");
}

static void joinCloseTest(void)
{
  joinMsgTest(0);
}

static void joinOpenTest(void)
{
  joinMsgTest(CONTACT_ALARM);
}

static void openCloseTest(void)
{
  //When the sensor goes active, we should receive a report with the alarm set
  expectedAlarmState = CONTACT_ALARM;
  messageSent = false;
  emberAfPluginGpioSensorStateChangedCallback(HAL_GPIO_SENSOR_ACTIVE);
  expectComparisonDecimal(true,
                          messageSent,
                          "true",
                          "messageSent");
  expectedAlarmState = 0;
  messageSent = false;
  emberAfPluginGpioSensorStateChangedCallback(HAL_GPIO_SENSOR_NOT_ACTIVE);
  expectComparisonDecimal(true,
                          messageSent,
                          "true",
                          "messageSent");
}

static void TamperTest(void)
{
  expectedAlarmState = 0;
  emberAfPluginTamperSwitchTamperActiveCallback();
  expectedAlarmState = TAMPER_ALARM;
  emberAfPluginTamperSwitchTamperAlarmCallback();
}

int main(int argc, char* argv[])
{
  const TestCase tests[] = {
    { "open-close", openCloseTest },
    { "tamper", TamperTest    },
    { "join-open", joinOpenTest  },
    { "join-close", joinCloseTest },
    { NULL },
  };
  return parseCommandLineAndExecuteTest(argc, argv, "Security Sensor", tests);
}
