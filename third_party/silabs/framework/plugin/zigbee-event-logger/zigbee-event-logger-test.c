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
 * @brief Unit tests for Zigbee Event Logger plugin
 *******************************************************************************
   ******************************************************************************/

#include "app/framework/include/af.h"
#include "stack/include/zigbee-event-logger-gen.h"
#include "app/framework/util/common.h"

#include "app/framework/plugin/compact-logger/compact-logger.h"
#include "app/framework/plugin/simple-clock/simple-clock.h"
#include "app/framework/plugin/zigbee-event-logger/zigbee-event-logger-print-gen.h"
#include "app/framework/test/test-framework.h"

//------------------------------------------------------------------------------
// Stubs

uint16_t emberAfLongStringLength(const uint8_t *buffer)
{
  // Should not be called during unit testing
  assert(0);
  return 0;
}

uint8_t emberAfStringLength(const uint8_t *buffer)
{
  // Should not be called during unit testing
  assert(0);
  return 0;
}

void emberAfPluginCompactLoggerUtcTimeSetCallback(uint32_t currentUtcTimeSeconds)
{
  (void)currentUtcTimeSeconds;
}

static uint32_t CurrentTimeSec;
static uint16_t MillisecondsRemainder;
uint32_t emberAfGetCurrentTimeSecondsWithMsPrecision(uint16_t* millisecondsRemainderReturn)
{
  *millisecondsRemainderReturn = MillisecondsRemainder++;
  return CurrentTimeSec;
}

EmberAfPluginSimpleClockTimeSyncStatus emberAfPluginSimpleClockGetTimeSyncStatus(void)
{
  return EMBER_AF_SIMPLE_CLOCK_UTC_SYNCED;
}

void emberAfPrintTimeIsoFormat(uint32_t utcTime)
{
  emberAfCorePrint("0x%8X         ", utcTime);
}

uint32_t halCommonGetInt32uMillisecondTick(void)
{
  return (CurrentTimeSec << 10) + MillisecondsRemainder;
}

static void zbelAddEvent(void)
{
  emberAfPluginCompactLoggerInit();
  emberAfPluginZigBeeEventLoggerAddBootEvent(5, 10);
  //note("Added event\r\n");
}

static void zbelPrintEvent(void)
{
  printFunction *pf;
  uint8_t data[2] = { 5, 11 };
  pf = emberAfPluginZigBeeEventLoggerLookupPrintFunction(EMBER_LOGGER_ZIGBEE_BOOT_EVENT_ID);
  pf(data, 2);
}

int main(int argc, char* argv[])
{
  const TestCase tests[] = {
    { "zblogger-add-event", zbelAddEvent },
    { "zblogger-print-event", zbelPrintEvent },
    { NULL },
  };

  return parseCommandLineAndExecuteTest(argc, argv, "Zigbee Event Logger", tests);
}

EmberStatus emberAfPluginUpdateTcLinkKeyStart(void)
{
  return EMBER_SUCCESS;
}
