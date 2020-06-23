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
 * @brief CLI for the Calendar Server plugin.
 *******************************************************************************
   ******************************************************************************/

#include "app/framework/include/af.h"
#include "app/framework/plugin/calendar-common/calendar-common.h"
#include "calendar-server.h"

#ifndef EMBER_AF_GENERATE_CLI
  #error The Calendar Server plugin is not compatible with the legacy CLI.
#endif

// plugin calendar-server publish-calendar <nodeId:2> <srcEndpoint:1> <dstEndpoint:1> <calendarIndex:1>
void emAfCalendarServerCliPublishCalendar(void)
{
  EmberNodeId nodeId = (EmberNodeId)emberUnsignedCommandArgument(0);
  uint8_t srcEndpoint = (uint8_t)emberUnsignedCommandArgument(1);
  uint8_t dstEndpoint = (uint8_t)emberUnsignedCommandArgument(2);
  uint8_t calendarIndex = (uint8_t)emberUnsignedCommandArgument(3);
  emberAfCalendarServerPublishCalendarMessage(nodeId,
                                              srcEndpoint,
                                              dstEndpoint,
                                              calendarIndex);
}

// plugin calendar-server publish-day-profiles <nodeId:2> <srcEndpoint:1> <dstEndpoint:1> <calendarIndex:1> <dayIndex:1>
void emAfCalendarServerCliPublishDayProfiles(void)
{
  EmberNodeId nodeId = (EmberNodeId)emberUnsignedCommandArgument(0);
  uint8_t srcEndpoint = (uint8_t)emberUnsignedCommandArgument(1);
  uint8_t dstEndpoint = (uint8_t)emberUnsignedCommandArgument(2);
  uint8_t calendarIndex = (uint8_t)emberUnsignedCommandArgument(3);
  uint8_t dayIndex = (uint8_t)emberUnsignedCommandArgument(4);
  emberAfCalendarServerPublishDayProfilesMessage(nodeId,
                                                 srcEndpoint,
                                                 dstEndpoint,
                                                 calendarIndex,
                                                 dayIndex);
}

// plugin calendar-server publish-week-profile <nodeId:2> <srcEndpoint:1> <dstEndpoint:1> <calendarIndex:1> <weekIndex:1>
void emAfCalendarServerCliPublishWeekProfile(void)
{
  EmberNodeId nodeId = (EmberNodeId)emberUnsignedCommandArgument(0);
  uint8_t srcEndpoint = (uint8_t)emberUnsignedCommandArgument(1);
  uint8_t dstEndpoint = (uint8_t)emberUnsignedCommandArgument(2);
  uint8_t calendarIndex = (uint8_t)emberUnsignedCommandArgument(3);
  uint8_t weekIndex = (uint8_t)emberUnsignedCommandArgument(4);
  emberAfCalendarServerPublishDayProfilesMessage(nodeId,
                                                 srcEndpoint,
                                                 dstEndpoint,
                                                 calendarIndex,
                                                 weekIndex);
}

// plugin calendar-server publish-week-profile <nodeId:2> <srcEndpoint:1> <dstEndpoint:1> <calendarIndex:1>
void emAfCalendarServerCliPublishSeasons(void)
{
  EmberNodeId nodeId = (EmberNodeId)emberUnsignedCommandArgument(0);
  uint8_t srcEndpoint = (uint8_t)emberUnsignedCommandArgument(1);
  uint8_t dstEndpoint = (uint8_t)emberUnsignedCommandArgument(2);
  uint8_t calendarIndex = (uint8_t)emberUnsignedCommandArgument(3);
  emberAfCalendarServerPublishSeasonsMessage(nodeId,
                                             srcEndpoint,
                                             dstEndpoint,
                                             calendarIndex);
}

// plugin calendar-server publish-special-days <nodeId:2> <srcEndpoint:1> <dstEndpoint:1> <calendarIndex:1>
void emAfCalendarServerCliPublishSpecialDays(void)
{
  EmberNodeId nodeId = (EmberNodeId)emberUnsignedCommandArgument(0);
  uint8_t srcEndpoint = (uint8_t)emberUnsignedCommandArgument(1);
  uint8_t dstEndpoint = (uint8_t)emberUnsignedCommandArgument(2);
  uint8_t calendarIndex = (uint8_t)emberUnsignedCommandArgument(3);
  emberAfCalendarServerPublishSpecialDaysMessage(nodeId,
                                                 srcEndpoint,
                                                 dstEndpoint,
                                                 calendarIndex);
}

// plugin calendar-server cancel-calendar <nodeId:2> <srcEndpoint:1> <dstEndpoint:1> <calendarIndex:1>
void emberAfCalendarServerCliCancelCalendar(void)
{
  EmberNodeId nodeId = (EmberNodeId)emberUnsignedCommandArgument(0);
  uint8_t srcEndpoint = (uint8_t)emberUnsignedCommandArgument(1);
  uint8_t dstEndpoint = (uint8_t)emberUnsignedCommandArgument(2);
  uint8_t calendarIndex = (uint8_t)emberUnsignedCommandArgument(3);
  emberAfCalendarServerCancelCalendarMessage(nodeId,
                                             srcEndpoint,
                                             dstEndpoint,
                                             calendarIndex);
  // now invalidate the calendar
  if (calendarIndex < EMBER_AF_PLUGIN_CALENDAR_COMMON_TOTAL_CALENDARS) {
    MEMSET(&(calendars[calendarIndex]), 0, sizeof(EmberAfCalendarStruct));
    calendars[calendarIndex].calendarId = EMBER_AF_PLUGIN_CALENDAR_COMMON_INVALID_CALENDAR_ID;
  }
}
