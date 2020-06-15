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
 * @brief APIs for the Calendar Common plugin.
 *******************************************************************************
   ******************************************************************************/

#define fieldLength(field) \
  (emberAfCurrentCommand()->bufLen - (field - emberAfCurrentCommand()->buffer));

#define EMBER_AF_PLUGIN_CALENDAR_COMMON_INVALID_SCHEDULE_ENTRY 0xFFFF
#define EMBER_AF_PLUGIN_CALENDAR_COMMON_INVALID_ID 0xFF
#define EMBER_AF_PLUGIN_CALENDAR_MAX_CALENDAR_NAME_LENGTH 12
#define EMBER_AF_PLUGIN_CALENDAR_COMMON_INVALID_INDEX 0xFF

#define SCHEDULE_ENTRY_SIZE (3)
typedef struct {
  uint16_t minutesFromMidnight;

  // The format of the actual data in the entry depends on the calendar type.
  //   For calendar type 00 - 0x02, it is a rate switch time and
  //     the data is a price tier enum (8-bit).
  //   For calendar type 0x03, it is a friendly credit switch time and
  //     the data is a boolean (8-bit), which is friendly credit enabled.
  //   For calendar type 0x04, it is an auxiliary load switch time and
  //     the data is a bitmap (8-bit).
  uint8_t data;
} EmberAfCalendarDayScheduleEntryStruct;

// Season start date (4-bytes) and week ID ref (1-byte).
#define SEASON_ENTRY_SIZE (5)

typedef struct {
  EmberAfCalendarDayScheduleEntryStruct scheduleEntries[EMBER_AF_PLUGIN_CALENDAR_COMMON_SCHEDULE_ENTRIES_MAX];
  uint8_t id;
  uint8_t numberOfScheduleEntries;
} EmberAfCalendarDayStruct;

// Special day date (4 bytes) and Day ID ref (1-byte).
#define SPECIAL_DAY_ENTRY_SIZE (5)
typedef struct {
  EmberAfDate startDate;
  uint8_t normalDayIndex;
  uint8_t flags;
} EmberAfCalendarSpecialDayStruct;

#define EMBER_AF_PLUGIN_CALENDAR_COMMON_MONDAY_INDEX (0)
#define EMBER_AF_PLUGIN_CALENDAR_COMMON_SUNDAY_INDEX (6)
#define EMBER_AF_DAYS_IN_THE_WEEK (7)

typedef struct {
  uint8_t normalDayIndexes[EMBER_AF_DAYS_IN_THE_WEEK];
  uint8_t id;
} EmberAfCalendarWeekStruct;

typedef struct {
  EmberAfDate startDate;
  uint8_t weekIndex;
} EmberAfCalendarSeasonStruct;

#define EMBER_AF_PLUGIN_CALENDAR_COMMON_INVALID_CALENDAR_ID 0xFFFFFFFF
#define EMBER_AF_PLUGIN_CALENDAR_COMMON_WILDCARD_CALENDAR_ID 0x00000000
#define EMBER_AF_PLUGIN_CALENDAR_COMMON_WILDCARD_PROVIDER_ID 0xFFFFFFFF
#define EMBER_AF_PLUGIN_CALENDAR_COMMON_WILDCARD_ISSUER_ID 0xFFFFFFFF
#define EMBER_AF_PLUGIN_CALENDAR_COMMON_WILDCARD_CALENDAR_TYPE 0xFF

enum {
  EMBER_AF_PLUGIN_CALENDAR_COMMON_FLAGS_SENT = 0x01,
};

typedef struct {
  EmberAfCalendarWeekStruct weeks[EMBER_AF_PLUGIN_CALENDAR_COMMON_WEEK_PROFILE_MAX];
  EmberAfCalendarDayStruct normalDays[EMBER_AF_PLUGIN_CALENDAR_COMMON_DAY_PROFILE_MAX];
  EmberAfCalendarSpecialDayStruct specialDays[EMBER_AF_PLUGIN_CALENDAR_COMMON_SPECIAL_DAY_PROFILE_MAX];
  EmberAfCalendarSeasonStruct seasons[EMBER_AF_PLUGIN_CALENDAR_COMMON_SEASON_PROFILE_MAX];
  uint32_t providerId;
  uint32_t issuerEventId;
  uint32_t calendarId;
  uint32_t startTimeUtc;
  uint8_t name[EMBER_AF_PLUGIN_CALENDAR_MAX_CALENDAR_NAME_LENGTH + 1];
  uint8_t calendarType;
  uint8_t numberOfSeasons;
  uint8_t numberOfWeekProfiles;
  uint8_t numberOfDayProfiles;
  uint8_t numberOfSpecialDayProfiles;

  /* These "received" counters do not belong here. They are here to help with
   * replaying TOM messages correctly, which will serve as the destination index
   * for the next publish command.
   */
  uint8_t numberOfReceivedSeasons;
  uint8_t numberOfReceivedWeekProfiles;
  uint8_t numberOfReceivedDayProfiles;
  uint8_t flags;
} EmberAfCalendarStruct;

extern EmberAfCalendarStruct calendars[];
#if defined(EMBER_AF_PLUGIN_GAS_PROXY_FUNCTION)
#define GBCS_TARIFF_SWITCHING_CALENDAR_ID 0xFFFFFFFF
#define GBCS_NON_DISABLEMENT_CALENDAR_ID  0xFFFFFFFE
extern uint32_t tariffSwitchingCalendarId;
extern uint32_t nonDisablementCalendarId;
#endif

uint8_t emberAfPluginCalendarCommonGetCalendarById(uint32_t calendarId,
                                                   uint32_t providerId);
uint32_t emberAfPluginCalendarCommonEndTimeUtc(const EmberAfCalendarStruct *calendar);
bool emberAfCalendarCommonSetCalInfo(uint8_t index,
                                     uint32_t providerId,
                                     uint32_t issuerEventId,
                                     uint32_t issuerCalendarId,
                                     uint32_t startTimeUtc,
                                     uint8_t calendarType,
                                     uint8_t *calendarName,
                                     uint8_t numberOfSeasons,
                                     uint8_t numberOfWeekProfiles,
                                     uint8_t numberOfDayProfiles);

/* @brief Adds a new entry corresponding to the PublishCalendar command.
 *
 * Tries to handle the new entry in the following method:
 *
 * 1) Try to apply new data to a matching existing entry.
 *    Fields such as providerId, issuerEventId, and startTime, will be used.
 * 3) Overwrite the oldest entry (one with smallest event ID) with new information.
 *
 */
bool emberAfCalendarCommonAddCalInfo(uint32_t providerId,
                                     uint32_t issuerEventId,
                                     uint32_t issuerCalendarId,
                                     uint32_t startTimeUtc,
                                     uint8_t calendarType,
                                     uint8_t *calendarName,
                                     uint8_t numberOfSeasons,
                                     uint8_t numberOfWeekProfiles,
                                     uint8_t numberOfDayProfiles);
bool emberAfCalendarServerSetSeasonsInfo(uint8_t index,
                                         uint8_t seasonId,
                                         EmberAfDate startDate,
                                         uint8_t weekIndex);
bool emberAfCalendarServerAddSeasonsInfo(uint32_t issuerCalendarId,
                                         uint8_t * seasonsEntries,
                                         uint8_t seasonsEntriesLength,
                                         uint8_t * unknownWeekIdSeasonsMask);

bool emberAfCalendarCommonSetDayProfInfo(uint8_t index,
                                         uint8_t dayId,
                                         uint8_t entryId,
                                         uint16_t minutesFromMidnight,
                                         uint8_t data);
bool emberAfCalendarCommonAddDayProfInfo(uint32_t issuerCalendarId,
                                         uint8_t dayId,
                                         uint8_t * dayScheduleEntries,
                                         uint16_t dayScheduleEntriesLength);
bool emberAfCalendarServerSetWeekProfInfo(uint8_t index,
                                          uint8_t weekId,
                                          uint8_t dayIdRefMon,
                                          uint8_t dayIdRefTue,
                                          uint8_t dayIdRefWed,
                                          uint8_t dayIdRefThu,
                                          uint8_t dayIdRefFri,
                                          uint8_t dayIdRefSat,
                                          uint8_t dayIdRefSun);
bool emberAfCalendarServerAddWeekProfInfo(uint32_t issuerCalendarId,
                                          uint8_t weekId,
                                          uint8_t dayIdRefMon,
                                          uint8_t dayIdRefTue,
                                          uint8_t dayIdRefWed,
                                          uint8_t dayIdRefThu,
                                          uint8_t dayIdRefFri,
                                          uint8_t dayIdRefSat,
                                          uint8_t dayIdRefSun);

/* @brief Updating special days information of the specified calendar.
 *
 * Assumes that the value of totalNumberOfSpecialDays will match
 * up with the information passed in between specialDaysEntries and
 * specialDaysEntriesLength.
 *
 */
bool emberAfCalendarCommonAddSpecialDaysInfo(uint32_t issuerCalendarId,
                                             uint8_t totalNumberOfSpecialDays,
                                             uint8_t * specialDaysEntries,
                                             uint16_t specialDaysEntriesLength,
                                             uint8_t * unknownSpecialDaysMask);
