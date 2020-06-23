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
 * @brief Routines for the Simple Clock plugin.
 *******************************************************************************
   ******************************************************************************/

#include "app/framework/include/af.h"

#include "app/framework/plugin/simple-clock/simple-clock.h"

// The variable "timeS" represents seconds since the ZigBee epoch, which was 0
// hours, 0 minutes, 0 seconds, on the 1st of January, 2000 UTC.  The variable
// "tickMs" is the millsecond tick at which that time was set.  The variable
// "remainderMs" is used to track sub-second chunks of time when converting
// from ticks to seconds.
static uint32_t timeS = 0;
static uint32_t tickMs = 0;
static uint16_t remainderMs = 0;

EmberAfPluginSimpleClockTimeSyncStatus syncStatus = EMBER_AF_SIMPLE_CLOCK_NEVER_UTC_SYNC;

// This event is used to periodically force an update to the internal time in
// order to avoid potential rollover problems with the system ticks.  A call to
// GetCurrentTime or SetTime will reschedule the event.
EmberEventControl emberAfPluginSimpleClockUpdateEventControl;

void emberAfPluginSimpleClockInitCallback(void)
{
  emberAfSetTimeCallback(0);
}

uint32_t emberAfGetCurrentTimeCallback(void)
{
  // Using system ticks, calculate how many seconds have elapsed since we last
  // got the time.  That amount plus the old time is our new time.  Remember
  // the tick time right now too so we can do the same calculations again when
  // we are next asked for the time.  Also, keep track of the sub-second chunks
  // of time during the conversion from ticks to seconds so the clock does not
  // drift due to rounding.
  uint32_t elapsedMs, lastTickMs = tickMs;
  tickMs = halCommonGetInt32uMillisecondTick();
  elapsedMs = elapsedTimeInt32u(lastTickMs, tickMs);
  timeS += elapsedMs / MILLISECOND_TICKS_PER_SECOND;
  remainderMs += elapsedMs % MILLISECOND_TICKS_PER_SECOND;
  if (MILLISECOND_TICKS_PER_SECOND <= remainderMs) {
    timeS++;
    remainderMs -= MILLISECOND_TICKS_PER_SECOND;
  }

  // Schedule an event to recalculate time to help avoid rollover problems.
  emberAfEventControlSetDelay(&emberAfPluginSimpleClockUpdateEventControl,
                              MILLISECOND_TICKS_PER_DAY);
  return timeS;
}

uint32_t emberAfGetCurrentTimeSecondsWithMsPrecision(uint16_t* millisecondsRemainderReturn)
{
  uint32_t value = emberAfGetCurrentTimeCallback();
  *millisecondsRemainderReturn = remainderMs;
  return value;
}

void emberAfSetTimeCallback(uint32_t utcTime)
{
  tickMs = halCommonGetInt32uMillisecondTick();
  timeS = utcTime;
  remainderMs = 0;

  // Immediately get the new time in order to reschedule the event.
  emberAfGetCurrentTimeCallback();
}

void emberAfPluginSimpleClockSetUtcSyncedTime(uint32_t utcTime)
{
  emberAfSetTimeCallback(utcTime);
  syncStatus = EMBER_AF_SIMPLE_CLOCK_UTC_SYNCED;
}

EmberAfPluginSimpleClockTimeSyncStatus emberAfPluginSimpleClockGetTimeSyncStatus(void)
{
  return syncStatus;
}

void emberAfPluginSimpleClockUpdateEventHandler(void)
{
  // Get the time, which will reschedule the event.

  // TODO: If time has not been synced for a while, set the status to
  // EMBER_AF_SIMPLE_CLOCK_STALE_UTC_SYNC

  emberAfGetCurrentTimeCallback();
}
