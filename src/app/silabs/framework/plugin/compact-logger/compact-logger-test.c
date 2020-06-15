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
 * @brief Compact Logger Unit tests
 *******************************************************************************
   ******************************************************************************/

#include "app/framework/include/af.h"
#include "compact-logger.h"
#include "app/framework/plugin/simple-clock/simple-clock.h"

#include "app/framework/test/test-framework.h"

//------------------------------------------------------------------------------
// Globals

#define MILLISECOND_TICKS_SINCE_BOOT \
  (MILLISECOND_TICKS_PER_SECOND      \
   * 60 /* seconds */                \
   * 60 /* minutes */                \
   * 3) /* hours */

static uint32_t currentTimeSeconds = 0;
static uint32_t millisecondTick = MILLISECOND_TICKS_SINCE_BOOT;
static uint16_t currentTimeMilliseconds = 0;

static EmberAfPluginSimpleClockTimeSyncStatus syncStatus = EMBER_AF_SIMPLE_CLOCK_NEVER_UTC_SYNC;

#define MAX_LOG_DATA 20

#define TIME_INCREMENT_SECONDS 60

static uint16_t globalLogMessageId = 0;

// See compact-logger.c.  Max number of log messages without data for this unit test.
#define MAX_LOG_MESSAGES 10

// February 1, 2000 = 60 seconds * 60 minutes * 24 hours * 31 days
#define UPDATED_UTC_TIME_SECONDS \
  (60 * 60 * 24 * 31)

//------------------------------------------------------------------------------
// Forward Declarations

#define  verifyCompactLogInfo(index, info) \
  verifyCompactLogInfoWithFileAndLineNumber(index, info, __FILE__, __LINE__)

//------------------------------------------------------------------------------
// Stubs

uint32_t emberAfGetCurrentTimeCallback(void)
{
  currentTimeSeconds += TIME_INCREMENT_SECONDS;
  return currentTimeSeconds;
}

uint32_t emberAfGetCurrentTimeSecondsWithMsPrecision(uint16_t* millisecondsRemainderReturn)
{
  *millisecondsRemainderReturn = currentTimeMilliseconds;
  return emberAfGetCurrentTimeCallback();
}

uint32_t halCommonGetInt32uMillisecondTick(void)
{
  return millisecondTick++;
}

void emberAfSetTimeCallback(uint32_t utcTime)
{
  currentTimeSeconds = utcTime;
}

EmberAfPluginSimpleClockTimeSyncStatus emberAfPluginSimpleClockGetTimeSyncStatus(void)
{
  return syncStatus;
}

void emberAfPluginCompactLoggerUtcTimeSetCallback(uint32_t currentUtcTimeSeconds)
{
  (void)currentUtcTimeSeconds;
}

//------------------------------------------------------------------------------
// Implementation

static void printLoggerMessageInfo(EmberAfPluginCompactLoggerMessageInfo* info)
{
  note("  Timestamp:      0x%08X\n", info->timestampSeconds);
  note("  Bitmask:        0x%X\n", info->bitmask);
  note("  Message ID:     0x%2X\n", info->messageId);
  note("  Message Length: 0x%2x\n", info->dataLength);
}

static void verifyCompactLogInfoWithFileAndLineNumber(uint16_t index,
                                                      EmberAfPluginCompactLoggerMessageInfo* expectedInfo,
                                                      const char* file,
                                                      int lineNumber)
{
  EmberAfPluginCompactLoggerMessageInfo actualInfo;
  expectComparisonHex(EMBER_SUCCESS,
                      emberAfPluginCompactLoggerGetEntryByNumber(index, &actualInfo),
                      "EMBER_SUCCESS",
                      "emberAfPluginCompactLoggerGetEntryByNumber()");

  if (0 != MEMCOMPARE(expectedInfo, &actualInfo, sizeof(EmberAfPluginCompactLoggerMessageInfo))) {
    note("Expected log info does not match actual log info at index %d\n", index);
    note("Expected:\n");
    printLoggerMessageInfo(expectedInfo);
    note("Actual:\n");
    printLoggerMessageInfo(&actualInfo);
    expectFunction(0, file, lineNumber, __PRETTY_FUNCTION__, NULL);
  } else {
    note(".");
  }

/*
   verifyDataWithFileInfo("EmberAfPluginCompactLoggerMessageInfo",
                         sizeof(EmberAfPluginCompactLoggerMessageInfo),
                         (uint8_t*)expectedInfo,
                         sizeof(EmberAfPluginCompactLoggerMessageInfo),
                         (uint8_t*)&actualInfo,
                         file,
                         lineNumber);
 */
}

static void addLogMessage(uint16_t dataLength, uint8_t* data)
{
  expectComparisonHex(EMBER_SUCCESS,
                      emberAfPluginCompactLoggerAdd(globalLogMessageId,
                                                    dataLength,
                                                    data),
                      "EMBER_SUCCESS",
                      "emberAfPluginCompactLoggerAdd()");
  globalLogMessageId++;
}

static void runAllTests(void)
{
  // Tests
  // 1. Simple test - add logs, print the results
  // 2. After time sync, rewrite all logs with updated time stamps

  // Tests to write
  // 3.  Add logs with data elments.

  // 1. Simple test - add logs, print the results
  uint16_t i;

  emberAfPluginCompactLoggerInit();

  for (i = 0; i < MAX_LOG_MESSAGES; i++) {
    addLogMessage(0, NULL); // no extra log data
  }
  emberAfPluginCompactLoggerPrintAllMessages();

  for (i = 0; i < MAX_LOG_MESSAGES; i++) {
    EmberAfPluginCompactLoggerMessageInfo expectedInfo;
    MEMSET(&expectedInfo, 0, sizeof(EmberAfPluginCompactLoggerMessageInfo));

    expectedInfo.messageId = i;
    expectedInfo.timestampSeconds = ((i + 1) * TIME_INCREMENT_SECONDS);

    debug("Verifying log mesasge %d\n", i);

    verifyCompactLogInfo(i, &expectedInfo);
  }

  // 2. After time sync, rewrite all logs with updated time stamps
  expectComparisonHex(EMBER_SUCCESS,
                      emberAfPluginCompactLoggerUpdateAllLogsWithUtcTime(UPDATED_UTC_TIME_SECONDS),
                      "EMBER_SUCCESS",
                      "emberAfPluginCompactLoggerUpdateAllLogsWithUtcTime()");

  emberAfPluginCompactLoggerPrintAllMessages();

  for (i = 0; i < MAX_LOG_MESSAGES; i++) {
    EmberAfPluginCompactLoggerMessageInfo expectedInfo;
    MEMSET(&expectedInfo, 0, sizeof(EmberAfPluginCompactLoggerMessageInfo));

    expectedInfo.messageId = i;
    expectedInfo.timestampSeconds = ((i + 1) * TIME_INCREMENT_SECONDS);
    expectedInfo.timestampSeconds += UPDATED_UTC_TIME_SECONDS;
    expectedInfo.timestampSeconds -= (MILLISECOND_TICKS_SINCE_BOOT / MILLISECOND_TICKS_PER_SECOND);
    expectedInfo.bitmask |= EMBER_AF_PLUGIN_COMPACT_LOGGER_BITMASK_UTC_TIME_SYNC;
    debug("Verifying log mesasge %d\n", i);

    verifyCompactLogInfo(i, &expectedInfo);
  }
}

int main(int argc, char* argv[])
{
  const TestCase allTests[] = {
    { "all-tests", runAllTests },
    { NULL },
  };

  return parseCommandLineAndExecuteTest(argc,
                                        argv,
                                        "af-plugin-compact-logger-test",
                                        allTests);
}
