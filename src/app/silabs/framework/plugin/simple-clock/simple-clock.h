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
 * @brief Definitions for the Simple Clock plugin.
 *******************************************************************************
   ******************************************************************************/

typedef enum {
  EMBER_AF_SIMPLE_CLOCK_NEVER_UTC_SYNC = 0,
  EMBER_AF_SIMPLE_CLOCK_STALE_UTC_SYNC = 1,
  EMBER_AF_SIMPLE_CLOCK_UTC_SYNCED = 2,
} EmberAfPluginSimpleClockTimeSyncStatus;

// This function sets the time and notes it as synchronized with UTC.  The
// Sync status will be set to EMBER_AF_SIMPLE_CLOCK_UTC_SYNCED.
void emberAfPluginSimpleClockSetUtcSyncedTime(uint32_t utcTimeSeconds);

// This function retrieves the status of the simple-clock and whether it has been
// syncchronized with UTC via a previous call to emberAfPluginSimpleClockSetUtcSyncedTime().
EmberAfPluginSimpleClockTimeSyncStatus emberAfPluginSimpleClockGetTimeSyncStatus(void);

// This retrieves the current time in seconds, and any millisecond remainder is returned
// in the passed pointer to the milliseconds value.
uint32_t emberAfGetCurrentTimeSecondsWithMsPrecision(uint16_t* millisecondsRemainderReturn);
