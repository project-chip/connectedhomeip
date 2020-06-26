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
 * @brief APIs for the Calendar Client plugin.
 *******************************************************************************
   ******************************************************************************/

#define EMBER_AF_PLUGIN_CALENDAR_CLIENT_INVALID_CALENDAR_ID 0xFFFFFFFF
#define EMBER_AF_CALENDAR_MAXIMUM_CALENDAR_NAME_LENGTH 12

typedef struct {
  EmberAfDate seasonStartDate;
  uint8_t weekIdRef;
} EmberAfCalendarSeason;

typedef struct {
  bool inUse;
  uint8_t dayIdRefMonday;
  uint8_t dayIdRefTuesday;
  uint8_t dayIdRefWednesday;
  uint8_t dayIdRefThursday;
  uint8_t dayIdRefFriday;
  uint8_t dayIdRefSaturday;
  uint8_t dayIdRefSunday;
} EmberAfCalendarWeekProfile;

// All valid calendar types have the same general format for schedule entries:
// a two-byte start time followed by a one-byte, type-specific value. The code
// in this plugin takes advantage of this similarity to simplify the logic. If
// new types are added, the code will need to change. See
// emberAfCalendarClusterPublishDayProfileCallback.
typedef union {
  struct {
    uint16_t startTimeM;
    uint8_t priceTier;
  } rateSwitchTime;
  struct {
    uint16_t startTimeM;
    bool friendlyCreditEnable;
  } friendlyCreditSwitchTime;
  struct {
    uint16_t startTimeM;
    uint8_t auxiliaryLoadSwitchState;
  } auxilliaryLoadSwitchTime;
} EmberAfCalendarScheduleEntry;

typedef struct {
  bool inUse;
  uint8_t numberOfScheduleEntries;
  uint8_t receivedScheduleEntries;
  EmberAfCalendarScheduleEntry scheduleEntries[EMBER_AF_PLUGIN_CALENDAR_CLIENT_SCHEDULE_ENTRIES];
} EmberAfCalendarDayProfile;

typedef struct {
  EmberAfDate specialDayDate;
  uint8_t dayIdRef;
} EmberAfCalendarSpecialDayEntry;

typedef struct {
  bool inUse;
  uint32_t startTimeUtc;
  uint8_t numberOfSpecialDayEntries;
  uint8_t receivedSpecialDayEntries;
  EmberAfCalendarSpecialDayEntry specialDayEntries[EMBER_AF_PLUGIN_CALENDAR_CLIENT_SPECIAL_DAY_ENTRIES];
} EmberAfCalendarSpecialDayProfile;

typedef struct {
  bool inUse;
  uint32_t providerId;
  uint32_t issuerEventId;
  uint32_t issuerCalendarId;
  uint32_t startTimeUtc;
  EmberAfCalendarType calendarType;
  uint8_t calendarName[EMBER_AF_CALENDAR_MAXIMUM_CALENDAR_NAME_LENGTH + 1];
  uint8_t numberOfSeasons;
  uint8_t receivedSeasons;
  uint8_t numberOfWeekProfiles;
  uint8_t numberOfDayProfiles;
  EmberAfCalendarSeason seasons[EMBER_AF_PLUGIN_CALENDAR_CLIENT_SEASONS];
  EmberAfCalendarWeekProfile weekProfiles[EMBER_AF_PLUGIN_CALENDAR_CLIENT_WEEK_PROFILES];
  EmberAfCalendarDayProfile dayProfiles[EMBER_AF_PLUGIN_CALENDAR_CLIENT_DAY_PROFILES];
  EmberAfCalendarSpecialDayProfile specialDayProfile;
} EmberAfCalendar;

/**
 * @brief Gets the first calendar index based on the calendar type.
 *
 * @param endpoint The relevant endpoint.
 * @param calendarType The type of calendar that should be searched for in the table.
 * @return The index of the first matching calendar, or EMBER_AF_PLUGIN_CALENDAR_CLIENT_CALENDARS
 * if a match cannot be found.
 *
 **/
uint8_t emberAfPluginCalendarClientGetCalendarIndexByType(uint8_t endpoint, uint8_t calendarType);

/**
 * @brief Gets the calendar ID at the specified index.
 *
 * @param endpoint The relevant endpoint.
 * @param index The index in the calendar table whose calendar ID should be returned.
 * @return The calendar ID of the calendar at the specified index. If index is
 * out of bounds, or if a match cannot be found,
 * EMBER_AF_PLUGIN_CALENDAR_CLIENT_INVALID_CALENDAR_ID will be returned.
 *
 **/
uint32_t emberAfPluginCalendarClientGetCalendarId(uint8_t endpoint, uint8_t index);
