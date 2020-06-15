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
 * @brief CLI for the Calendar Common plugin.
 *******************************************************************************
   ******************************************************************************/

#include "app/framework/include/af.h"
#include "calendar-common.h"

//-----------------------------------------------------------------------------
// Globals

EmberAfCalendarStruct calendars[EMBER_AF_PLUGIN_CALENDAR_COMMON_TOTAL_CALENDARS];

/*
 * From GBCS V0.8.1 section 10.4.2.11.
 *
 * When processing a command where Issuer Calendar ID has the value 0xFFFFFFFF
 * or 0xFFFFFFFE, a GPF or GSME shall interpret 0xFFFFFFFF as meaning the
 * currently in force Tariff Switching Table calendar and 0xFFFFFFFE as meaning
 * the currently in force Non-Disablement Calendar
 */
#if defined(EMBER_AF_PLUGIN_GAS_PROXY_FUNCTION)
uint32_t tariffSwitchingCalendarId = EMBER_AF_PLUGIN_CALENDAR_COMMON_INVALID_CALENDAR_ID;
uint32_t nonDisablementCalendarId  = EMBER_AF_PLUGIN_CALENDAR_COMMON_INVALID_CALENDAR_ID;
#endif

//-----------------------------------------------------------------------------

void emberAfPluginCalendarCommonInitCallback(uint8_t endpoint)
{
  uint8_t i;
  for (i = 0; i < EMBER_AF_PLUGIN_CALENDAR_COMMON_TOTAL_CALENDARS; i++) {
    MEMSET(&(calendars[i]), 0, sizeof(EmberAfCalendarStruct));
    calendars[i].calendarId = EMBER_AF_PLUGIN_CALENDAR_COMMON_INVALID_CALENDAR_ID;
  }
}

uint8_t emberAfPluginCalendarCommonGetCalendarById(uint32_t calendarId,
                                                   uint32_t providerId)
{
  uint8_t i;

#if defined(EMBER_AF_PLUGIN_GAS_PROXY_FUNCTION)
  if (calendarId == GBCS_TARIFF_SWITCHING_CALENDAR_ID) {
    calendarId = tariffSwitchingCalendarId;
  } else if (calendarId == GBCS_NON_DISABLEMENT_CALENDAR_ID) {
    calendarId = nonDisablementCalendarId;
  }
#endif

  for (i = 0; i < EMBER_AF_PLUGIN_CALENDAR_COMMON_TOTAL_CALENDARS; i++) {
    if (calendarId == calendars[i].calendarId
        && (providerId == EMBER_AF_PLUGIN_CALENDAR_COMMON_WILDCARD_PROVIDER_ID
            || providerId == calendars[i].providerId)) {
      return i;
    }
  }
  return EMBER_AF_PLUGIN_CALENDAR_COMMON_INVALID_INDEX;
}

EmberAfCalendarStruct * findCalendarByCalId(uint32_t issuerCalendarId)
{
  uint8_t i;
  for (i = 0; i < EMBER_AF_PLUGIN_CALENDAR_COMMON_TOTAL_CALENDARS; i++) {
    EmberAfCalendarStruct * cal = &calendars[i];
    if (cal->calendarId == issuerCalendarId) {
      return cal;
    }
  }
  emberAfCalendarClusterPrintln("ERR: Unable to find calendar with id(0x%4X) ", issuerCalendarId);
  return NULL;
}

uint32_t emberAfPluginCalendarCommonEndTimeUtc(const EmberAfCalendarStruct *calendar)
{
  uint32_t endTimeUtc = MAX_INT32U_VALUE;
  uint8_t i;
  for (i = 0; i < EMBER_AF_PLUGIN_CALENDAR_COMMON_TOTAL_CALENDARS; i++) {
    if (calendar->providerId == calendars[i].providerId
        && calendar->issuerEventId < calendars[i].issuerEventId
        && calendar->startTimeUtc < calendars[i].startTimeUtc
        && calendar->calendarType == calendars[i].calendarType
        && calendars[i].startTimeUtc < endTimeUtc) {
      endTimeUtc = calendars[i].startTimeUtc;
    }
  }

  return endTimeUtc;
}

bool emberAfCalendarCommonSetCalInfo(uint8_t index,
                                     uint32_t providerId,
                                     uint32_t issuerEventId,
                                     uint32_t issuerCalendarId,
                                     uint32_t startTimeUtc,
                                     uint8_t calendarType,
                                     uint8_t *calendarName,
                                     uint8_t numberOfSeasons,
                                     uint8_t numberOfWeekProfiles,
                                     uint8_t numberOfDayProfiles)
{
  EmberAfCalendarStruct * cal;
  if (index >= EMBER_AF_PLUGIN_CALENDAR_COMMON_TOTAL_CALENDARS) {
    emberAfCalendarClusterPrintln("Index must be in the range of 0 to %d", EMBER_AF_PLUGIN_CALENDAR_COMMON_TOTAL_CALENDARS - 1);
    return false;
  }

  if (calendarName == NULL || calendarName[0] > EMBER_AF_PLUGIN_CALENDAR_MAX_CALENDAR_NAME_LENGTH) {
    emberAfCalendarClusterPrintln("Invalid calendar name!");
    return false;
  }

  cal = &calendars[index];
  cal->providerId = providerId;
  cal->issuerEventId = issuerEventId;
  cal->calendarId = issuerCalendarId;
  cal->startTimeUtc = startTimeUtc;
  cal->calendarType = calendarType;
  emberAfCopyString(cal->name, calendarName, EMBER_AF_PLUGIN_CALENDAR_MAX_CALENDAR_NAME_LENGTH);
  cal->numberOfSeasons = numberOfSeasons;
  cal->numberOfWeekProfiles = numberOfWeekProfiles;
  cal->numberOfDayProfiles = numberOfDayProfiles;
  cal->flags = 0;
  return true;
}

//[[
/*
 * TODO: Nested and overlapping calendars are not allowed. SE specifications
 * elaborate more on the details in PublishCalendar Command description.
 */
//]]
bool emberAfCalendarCommonAddCalInfo(uint32_t providerId,
                                     uint32_t issuerEventId,
                                     uint32_t issuerCalendarId,
                                     uint32_t startTimeUtc,
                                     uint8_t calendarType,
                                     uint8_t *calendarName,
                                     uint8_t numberOfSeasons,
                                     uint8_t numberOfWeekProfiles,
                                     uint8_t numberOfDayProfiles)
{
  bool status = false;
  uint8_t i = 0;
  uint8_t matchingEntryIndex = 0xFF;
  uint8_t smallestEventIdEntryIndex = 0x0;
  EmberAfCalendarStruct * cal;

  // Try to find an existing entry to overwrite.
  for (i = 0; i < EMBER_AF_PLUGIN_CALENDAR_COMMON_TOTAL_CALENDARS; i++) {
    EmberAfCalendarStruct * cal = &calendars[i];
    if (cal->providerId == providerId
        && cal->calendarId == issuerCalendarId) {
      matchingEntryIndex = i;
    }

    if (cal->issuerEventId < calendars[smallestEventIdEntryIndex].issuerEventId) {
      smallestEventIdEntryIndex = i;
    }
  }

  if (matchingEntryIndex == 0xFF) {
    i = smallestEventIdEntryIndex;
  } else {
    i = matchingEntryIndex;
  }

  cal = &calendars[i];

  if (numberOfSeasons > EMBER_AF_PLUGIN_CALENDAR_COMMON_SEASON_PROFILE_MAX) {
    emberAfCalendarClusterPrintln("ERR: Insufficient space for requested number of seasons(%d)!", numberOfSeasons);
    status = false;
    goto kickout;
  }
  if (numberOfWeekProfiles > EMBER_AF_PLUGIN_CALENDAR_COMMON_WEEK_PROFILE_MAX) {
    emberAfCalendarClusterPrintln("ERR: Insufficient space for requested number of week profiles(%d)!", numberOfWeekProfiles);
    status = false;
    goto kickout;
  }
  if (numberOfDayProfiles > EMBER_AF_PLUGIN_CALENDAR_COMMON_DAY_PROFILE_MAX) {
    emberAfCalendarClusterPrintln("ERR: Insufficient space for requested number of day profiles(%d)!", numberOfDayProfiles);
    status = false;
    goto kickout;
  }

  // Calendars must be replaced as a whole.
  MEMSET(cal, 0x00, sizeof(EmberAfCalendarStruct));
  cal->providerId = providerId;
  cal->issuerEventId = issuerEventId;
  cal->calendarId = issuerCalendarId;
  cal->startTimeUtc = startTimeUtc;
  cal->calendarType = calendarType;
  emberAfCopyString(cal->name, calendarName, EMBER_AF_PLUGIN_CALENDAR_MAX_CALENDAR_NAME_LENGTH);
  cal->numberOfSeasons = numberOfSeasons;
  cal->numberOfWeekProfiles = numberOfWeekProfiles;
  cal->numberOfDayProfiles = numberOfDayProfiles;
  cal->flags = 0;
  status = true;

#if defined(EMBER_AF_PLUGIN_GAS_PROXY_FUNCTION)
  if (cal->calendarType == EMBER_ZCL_CALENDAR_TYPE_DELIVERED_CALENDAR) {
    tariffSwitchingCalendarId = cal->calendarId;
  } else if (cal->calendarType == EMBER_ZCL_CALENDAR_TYPE_FRIENDLY_CREDIT_CALENDAR) {
    nonDisablementCalendarId = cal->calendarId;
  }
#endif

  kickout:
  return status;
}

bool emberAfCalendarCommonAddSpecialDaysInfo(uint32_t issuerCalendarId,
                                             uint8_t totalNumberOfSpecialDays,
                                             uint8_t * specialDaysEntries,
                                             uint16_t specialDaysEntriesLength,
                                             uint8_t * unknownSpecialDaysMask)
{
  EmberAfCalendarStruct * cal = findCalendarByCalId(issuerCalendarId);
  bool status = false;
  uint8_t numberOfSpecialDaysEntries = specialDaysEntriesLength / SPECIAL_DAY_ENTRY_SIZE;
  uint8_t i;

  *unknownSpecialDaysMask = 0;

  if (cal == NULL) {
    status = false;
    goto kickout;
  }

  if (totalNumberOfSpecialDays > EMBER_AF_PLUGIN_CALENDAR_COMMON_SPECIAL_DAY_PROFILE_MAX) {
    emberAfCalendarClusterPrintln("ERR: %d number of special days being added is above the current maximum(%d).",
                                  totalNumberOfSpecialDays,
                                  EMBER_AF_PLUGIN_CALENDAR_COMMON_SPECIAL_DAY_PROFILE_MAX);
    status = false;
    goto kickout;
  }

  if (totalNumberOfSpecialDays != numberOfSpecialDaysEntries) {
    emberAfCalendarClusterPrintln("ERR: adding special days with inconsistent information.");
    status = false;
    goto kickout;
  }

  if ((specialDaysEntries == NULL) || (numberOfSpecialDaysEntries == 0)) {
    status = false;
    emberAfCalendarClusterPrintln("ERR: Unable to add special days.");
    goto kickout;
  }

  for (i = 0; i < numberOfSpecialDaysEntries; i++) {
    EmberAfDate startDate = { 0 };
    uint8_t normalDayId;

    emberAfGetDate(specialDaysEntries,
                   SPECIAL_DAY_ENTRY_SIZE * i,
                   specialDaysEntriesLength,
                   &startDate);
    normalDayId = emberAfGetInt8u(specialDaysEntries,
                                  SPECIAL_DAY_ENTRY_SIZE * i + sizeof(EmberAfDate),
                                  specialDaysEntriesLength);
    if (cal->numberOfSpecialDayProfiles < EMBER_AF_PLUGIN_CALENDAR_COMMON_SPECIAL_DAY_PROFILE_MAX) {
      uint8_t normalDayIndex;
      bool update = true;

      // find corresponding normal day index
      for (normalDayIndex = 0; normalDayIndex < EMBER_AF_PLUGIN_CALENDAR_COMMON_DAY_PROFILE_MAX; normalDayIndex++) {
        if (cal->normalDays[normalDayIndex].id == normalDayId) {
          uint8_t index;
          // check for redundant specialDays
          for (index = 0; index < cal->numberOfSpecialDayProfiles; index++) {
            EmberAfCalendarSpecialDayStruct * specialDay = &cal->specialDays[index];
            if ((specialDay->normalDayIndex == normalDayIndex)
                && (emberAfCompareDates(&specialDay->startDate, &startDate) == 0)) {
              update = false;
            }
          }

          break;
        }
      }

      if (normalDayIndex >= EMBER_AF_PLUGIN_CALENDAR_COMMON_DAY_PROFILE_MAX) {
        // The normal day ID is unknown, so do not publish this special
        // day. Set a bit in the unknown day mask to alert the caller
        // of this function. By not incrementing numberOfSpecialDayProfiles,
        // the calendar will not be updated with this special day.
        *unknownSpecialDaysMask |= BIT(i);
      } else if (update) {
        cal->specialDays[cal->numberOfSpecialDayProfiles].normalDayIndex = normalDayIndex;
        cal->specialDays[cal->numberOfSpecialDayProfiles].flags = 0;
        cal->specialDays[cal->numberOfSpecialDayProfiles].startDate = startDate;

        emberAfCalendarClusterPrintln("Updated: Calendar(calId=0x%4X)",
                                      cal->calendarId);
        emberAfCalendarClusterPrintln("         SpecialDays[%d]", cal->numberOfSpecialDayProfiles);
        emberAfCalendarClusterPrint("           startDate: ");
        emberAfPrintDateln(&cal->specialDays[cal->numberOfSpecialDayProfiles].startDate);
        emberAfCalendarClusterPrintln("           normalDayIndex: %d", cal->specialDays[cal->numberOfSpecialDayProfiles].normalDayIndex);
        cal->numberOfSpecialDayProfiles++;
      } else {
        emberAfCalendarClusterPrintln("ERR: Invalid dayId! Unable to store SpecialDays info.");
      }
    } else {
      emberAfCalendarClusterPrintln("ERR: Insufficient space to store more SpecialDays info!");
    }
  }
  status = (*unknownSpecialDaysMask == 0 ? true : false);

  kickout:
  return status;
}

bool emberAfCalendarCommonAddDayProfInfo(uint32_t issuerCalendarId,
                                         uint8_t dayId,
                                         uint8_t * dayScheduleEntries,
                                         uint16_t dayScheduleEntriesLength)
{
  EmberAfCalendarStruct * cal = findCalendarByCalId(issuerCalendarId);
  bool status = false;
  uint8_t numberOfScheduledEntries = dayScheduleEntriesLength / SCHEDULE_ENTRY_SIZE;
  uint8_t scheduleEntryIndex = 0;

  if (cal == NULL) {
    emberAfCalendarClusterPrintln("ERR: Unable to find calendar with id(0x%4X)",
                                  issuerCalendarId);
    status = false;
    goto kickout;
  }

  if ((dayScheduleEntries == NULL)
      || (dayScheduleEntriesLength == 0)
      || ((dayScheduleEntriesLength % SCHEDULE_ENTRY_SIZE) != 0) // each struct occupy 3 bytes.
      || (cal->numberOfReceivedDayProfiles >= EMBER_AF_PLUGIN_CALENDAR_COMMON_DAY_PROFILE_MAX)
      || (numberOfScheduledEntries > EMBER_AF_PLUGIN_CALENDAR_COMMON_SCHEDULE_ENTRIES_MAX)) {
    status = false;
    emberAfCalendarClusterPrintln("ERR: Unable to add DayProfile");
    goto kickout;
  }

  emberAfCalendarClusterPrintln("Updated: DayProfile[%d]",
                                cal->numberOfReceivedDayProfiles);
  emberAfCalendarClusterPrintln("           DayId=%d",
                                dayId);

  cal->normalDays[cal->numberOfReceivedDayProfiles].id = dayId;
  cal->normalDays[cal->numberOfReceivedDayProfiles].numberOfScheduleEntries = numberOfScheduledEntries;

  for (scheduleEntryIndex = 0; scheduleEntryIndex < numberOfScheduledEntries; scheduleEntryIndex++) {
    uint8_t priceTier;
    uint16_t minutesFromMidnight;
    EmberAfCalendarDayScheduleEntryStruct * normalDay;
    normalDay = &cal->normalDays[cal->numberOfReceivedDayProfiles].scheduleEntries[scheduleEntryIndex];
    minutesFromMidnight = emberAfGetInt16u(dayScheduleEntries,
                                           scheduleEntryIndex * SCHEDULE_ENTRY_SIZE,
                                           dayScheduleEntriesLength);
    priceTier = emberAfGetInt8u(dayScheduleEntries,
                                scheduleEntryIndex * SCHEDULE_ENTRY_SIZE + 2,
                                dayScheduleEntriesLength);
    normalDay->minutesFromMidnight =  minutesFromMidnight;
    normalDay->data =  priceTier;
    emberAfCalendarClusterPrintln("           ScheduledEntries[%d]",
                                  scheduleEntryIndex);
    emberAfCalendarClusterPrintln("             startTime: 0x%2X from midnight",
                                  minutesFromMidnight);
    emberAfCalendarClusterPrintln("             price tier: 0x%X",
                                  priceTier);
  }

  cal->numberOfReceivedDayProfiles++;
  status = true;

  kickout:
  return status;
}

bool emberAfCalendarCommonSetDayProfInfo(uint8_t index,
                                         uint8_t dayId,
                                         uint8_t entryId,
                                         uint16_t minutesFromMidnight,
                                         uint8_t data)
{
  if (index >= EMBER_AF_PLUGIN_CALENDAR_COMMON_TOTAL_CALENDARS) {
    emberAfCalendarClusterPrintln("Index must be in the range of 0 to %d", EMBER_AF_PLUGIN_CALENDAR_COMMON_TOTAL_CALENDARS - 1);
    return false;
  } else if (dayId >= EMBER_AF_PLUGIN_CALENDAR_COMMON_DAY_PROFILE_MAX) {
    emberAfCalendarClusterPrintln("DayId must be in the range of 0 to %d", EMBER_AF_PLUGIN_CALENDAR_COMMON_DAY_PROFILE_MAX - 1);
    return false;
  } else if (entryId >= EMBER_AF_PLUGIN_CALENDAR_COMMON_SCHEDULE_ENTRIES_MAX) {
    emberAfCalendarClusterPrintln("EntryId must be in the range of 0 to %d", EMBER_AF_PLUGIN_CALENDAR_COMMON_SCHEDULE_ENTRIES_MAX - 1);
    return false;
  }

  calendars[index].normalDays[dayId].scheduleEntries[entryId].minutesFromMidnight =  minutesFromMidnight;
  calendars[index].normalDays[dayId].scheduleEntries[entryId].data = data;
  return true;
}

bool emberAfCalendarServerAddWeekProfInfo(uint32_t issuerCalendarId,
                                          uint8_t weekId,
                                          uint8_t dayIdRefMon,
                                          uint8_t dayIdRefTue,
                                          uint8_t dayIdRefWed,
                                          uint8_t dayIdRefThu,
                                          uint8_t dayIdRefFri,
                                          uint8_t dayIdRefSat,
                                          uint8_t dayIdRefSun)
{
  EmberAfCalendarStruct * cal = findCalendarByCalId(issuerCalendarId);
  uint8_t dayIdRefs[7];
  uint8_t dayCount = 7;
  uint8_t * normalDayIndexes;
  EmberAfCalendarWeekStruct * weeks;
  EmberAfCalendarDayStruct * normalDays;
  uint8_t dayIdRefsIndex;

  if ((cal == NULL)
      || (cal->numberOfReceivedWeekProfiles >= EMBER_AF_PLUGIN_CALENDAR_COMMON_WEEK_PROFILE_MAX)) {
    return false;
  }

  dayIdRefs[0] = dayIdRefMon;
  dayIdRefs[1] = dayIdRefTue;
  dayIdRefs[2] = dayIdRefWed;
  dayIdRefs[3] = dayIdRefThu;
  dayIdRefs[4] = dayIdRefFri;
  dayIdRefs[5] = dayIdRefSat;
  dayIdRefs[6] = dayIdRefSun;
  normalDays = (EmberAfCalendarDayStruct *)&cal->normalDays;
  normalDayIndexes = (uint8_t *)&cal->weeks[cal->numberOfReceivedWeekProfiles].normalDayIndexes;
  weeks = &cal->weeks[cal->numberOfReceivedWeekProfiles];

  cal->weeks[cal->numberOfReceivedWeekProfiles].id = weekId;
  emberAfCalendarClusterPrintln("Updated: WeekProfile[%d]",
                                cal->numberOfReceivedWeekProfiles);
  emberAfCalendarClusterPrintln("           weekId=%d", weekId);

  for (dayIdRefsIndex = 0; dayIdRefsIndex < dayCount; dayIdRefsIndex++) {
    uint8_t normalDayIndex;
    uint8_t dayId = dayIdRefs[dayIdRefsIndex];
    for (normalDayIndex = 0; normalDayIndex < EMBER_AF_PLUGIN_CALENDAR_COMMON_DAY_PROFILE_MAX; normalDayIndex++) {
      if (normalDays[normalDayIndex].id == dayId) {
        normalDayIndexes[dayIdRefsIndex] = normalDayIndex;
        emberAfCalendarClusterPrintln("           normalDayIndexes[%d]=%d",
                                      dayIdRefsIndex,
                                      normalDayIndex);
      }
    }
  }
  cal->numberOfReceivedWeekProfiles++;
  return true;
}

bool emberAfCalendarServerSetWeekProfInfo(uint8_t index,
                                          uint8_t weekId,
                                          uint8_t dayIdRefMon,
                                          uint8_t dayIdRefTue,
                                          uint8_t dayIdRefWed,
                                          uint8_t dayIdRefThu,
                                          uint8_t dayIdRefFri,
                                          uint8_t dayIdRefSat,
                                          uint8_t dayIdRefSun)
{
  if (index >= EMBER_AF_PLUGIN_CALENDAR_COMMON_TOTAL_CALENDARS) {
    emberAfCalendarClusterPrintln("Index must be in the range of 0 to %d", EMBER_AF_PLUGIN_CALENDAR_COMMON_TOTAL_CALENDARS - 1);
    return false;
  } else if (weekId >= EMBER_AF_PLUGIN_CALENDAR_COMMON_WEEK_PROFILE_MAX) {
    emberAfCalendarClusterPrintln("WeekId must be in the range of 0 to %d", EMBER_AF_PLUGIN_CALENDAR_COMMON_WEEK_PROFILE_MAX - 1);
    return false;
  }

  calendars[index].weeks[weekId].normalDayIndexes[0] = dayIdRefMon;
  calendars[index].weeks[weekId].normalDayIndexes[1] = dayIdRefTue;
  calendars[index].weeks[weekId].normalDayIndexes[2] = dayIdRefWed;
  calendars[index].weeks[weekId].normalDayIndexes[3] = dayIdRefThu;
  calendars[index].weeks[weekId].normalDayIndexes[4] = dayIdRefFri;
  calendars[index].weeks[weekId].normalDayIndexes[5] = dayIdRefSat;
  calendars[index].weeks[weekId].normalDayIndexes[6] = dayIdRefSun;
  return true;
}

bool emberAfCalendarServerAddSeasonsInfo(uint32_t issuerCalendarId,
                                         uint8_t * seasonsEntries,
                                         uint8_t seasonsEntriesLength,
                                         uint8_t * unknownWeekIdSeasonsMask)
{
  bool status = false;
  EmberAfCalendarStruct * cal = findCalendarByCalId(issuerCalendarId);
  uint8_t seasonEntryCount = seasonsEntriesLength / SEASON_ENTRY_SIZE;
  uint8_t seasonEntryIndex;

  *unknownWeekIdSeasonsMask = 0;

  if (cal == NULL) {
    status = false;
    goto kickout;
  }

  if (cal->numberOfReceivedSeasons + seasonEntryCount > cal->numberOfSeasons) {
    status = false;
    goto kickout;
  }

  for (seasonEntryIndex = 0; seasonEntryIndex < seasonEntryCount; seasonEntryIndex++) {
    EmberAfCalendarSeasonStruct  * season = &cal->seasons[cal->numberOfReceivedSeasons];
    EmberAfDate startDate;
    uint8_t weekId;
    emberAfGetDate(seasonsEntries,
                   seasonEntryIndex * SEASON_ENTRY_SIZE,
                   seasonsEntriesLength,
                   &startDate);
    weekId = emberAfGetInt8u(seasonsEntries,
                             seasonEntryIndex * SEASON_ENTRY_SIZE + sizeof(EmberAfDate),
                             seasonsEntriesLength);
    season->startDate = startDate;
    emberAfCalendarClusterPrintln("Updated: Seasons[%d]", cal->numberOfReceivedSeasons);
    emberAfCalendarClusterPrint("            startDate: ");
    emberAfPrintDateln(&startDate);
    {
      // search for week index.
      uint8_t i;
      season->weekIndex = 0xFF;
      for (i = 0; i < EMBER_AF_PLUGIN_CALENDAR_COMMON_WEEK_PROFILE_MAX; i++) {
        if (cal->weeks[i].id == weekId) {
          season->weekIndex = i;
        }
      }
    }

    emberAfCalendarClusterPrintln("            weekIndex: %d", season->weekIndex);

    if (season->weekIndex != 0xFF) {
      cal->numberOfReceivedSeasons++;
    } else {
      // Do not increment numberOfReceivedSeasons, and overwrite
      // this seasonEntryIndex with the next seasonEntry.
      *unknownWeekIdSeasonsMask |= BIT(seasonEntryIndex);
    }
  }
  status = (*unknownWeekIdSeasonsMask == 0 ? true : false);

  kickout:
  return status;
}

bool emberAfCalendarServerSetSeasonsInfo(uint8_t index,
                                         uint8_t seasonId,
                                         EmberAfDate startDate,
                                         uint8_t weekIdRef)
{
  if (index >= EMBER_AF_PLUGIN_CALENDAR_COMMON_TOTAL_CALENDARS) {
    emberAfCalendarClusterPrintln("Index must be in the range of 0 to %d", EMBER_AF_PLUGIN_CALENDAR_COMMON_TOTAL_CALENDARS - 1);
    return false;
  } else if (seasonId >= EMBER_AF_PLUGIN_CALENDAR_COMMON_SEASON_PROFILE_MAX) {
    emberAfCalendarClusterPrintln("SeasonId must be in the range of 0 to %d", EMBER_AF_PLUGIN_CALENDAR_COMMON_SEASON_PROFILE_MAX - 1);
    return false;
  }

  calendars[index].seasons[seasonId].startDate = startDate;
  calendars[index].seasons[seasonId].weekIndex = weekIdRef;
  return true;
}
