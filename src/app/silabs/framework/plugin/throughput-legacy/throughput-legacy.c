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
 * @brief Definitions for the Throughtput Legacy plugin, which provides a means
 *        of measuring network throughput.
 *******************************************************************************
   ******************************************************************************/

#include "../../include/af.h"
#include "../../util/common.h"
#include "throughput-legacy.h"

static uint8_t inflight_count = 1;

static uint32_t packets_sent = 0;
static uint32_t packets_recd = 0;

static bool is_test_running = false;

static uint32_t start_time = 0;
static uint32_t end_time = 8000000;
static uint32_t run_time = 0;

void calc_run_time(void);

/**
 * Set the number of packets in flight for the next test (up to 4)
 */
void emAfPluginThroughputSetInFlightCount(void)
{
  //Is the test running already?
  if (is_test_running) {
    emberAfCorePrintln("ERROR: Packets in flight cannot be changed while the test is running.");
    return;
  }
  inflight_count = (uint8_t)emberUnsignedCommandArgument(0);
  if (inflight_count > 4) {
    inflight_count = 4;
  } else if (inflight_count == 0) {
    inflight_count = 1;
  }
  emberAfCorePrintln("Packets in flight for next test: %d", inflight_count);
}

/**
 * Set the duration in milliseconds for which the next test will run.
 */
void emAfPluginThroughputSetDuration(void)
{
  //Is the test running already?
  if (is_test_running) {
    emberAfCorePrintln("ERROR: Test duration cannot be changed while the test is running.");
    return;
  }
  end_time = (uint32_t)emberUnsignedCommandArgument(0);
  emberAfCorePrintln("Duration for next test: %d ms", end_time);
}

/**
 * Start a new test.
 */
void emAfPluginThroughputStartTest(void)
{
  //Is the test already running?
  if (is_test_running) {
    return;
  }
  is_test_running = true;
  packets_sent = 0;
  packets_recd = 0;
  start_time = halCommonGetInt32uMillisecondTick();
}

/**
 * This causes the test to stop running and will be called when the duration
 * has been reached.
 */
void emAfPluginThroughputEndTest(void)
{
  if (is_test_running) {
    is_test_running = false;
    emberAfCorePrintln("Test ended at %d ms: %d packets sent, %d packets received", run_time, packets_sent, packets_recd);
  }
}

/**
 * This can be used to print results on the command line for the last test. For
 * example, a user could disable CLI during the test and then enable it to get
 * the results later.
 */
void emAfPluginThroughputPrintResult(void)
{
  if (!is_test_running) {
    emberAfCorePrintln("Test ended at %d ms: %d packets sent, %d packets received", run_time, packets_sent, packets_recd);
  }
}

/**
 * Users will call this to notify the plugin that a packet has completed its
 * round trip.
 */
void emAfPluginThroughputNotifyPacketReceived(void)
{
  if (is_test_running) {
    packets_recd++;
  }
}

/**
 * Get the time elapsed since the test started
 */
void calc_run_time(void)
{
  uint32_t current_time = halCommonGetInt32uMillisecondTick();
  //Check if the timer overflowed since starting
  if ( start_time > current_time ) {
    run_time = (0xffffffff - (start_time - current_time));
  } else {
    run_time = (current_time - start_time);
  }
}

void emberAfMainTickCallback(void)
{
  //Is the test running?
  if (is_test_running) {
    //Should the test keep going?
    calc_run_time();
    if (run_time >= end_time) {
      emAfPluginThroughputEndTest();
      return;
    }
    //Should there be more packets in flight?
    if ((packets_sent - packets_recd) < inflight_count) {
      //Try to send one more.
      emberAfPluginThroughputSendPacketCallback();
      packets_sent++;
    }
  }
}
