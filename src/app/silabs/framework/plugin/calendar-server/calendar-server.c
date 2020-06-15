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
 * @brief Routines for the Calendar Server plugin.
 *******************************************************************************
   ******************************************************************************/

#include "app/framework/include/af.h"
#include "app/framework/util/common.h"
#include "app/framework/plugin/calendar-common/calendar-common.h"
#include "calendar-server.h"

//-----------------------------------------------------------------------------
// Globals

static uint8_t myEndpoint = 0;

// The PublishInfo structure is used to manage the multiple Publish responses
// that may be required when a one of the various Get commands are received.
typedef struct {
  uint8_t publishCommandId;
  uint8_t totalCommands;
  uint8_t commandIndex;
  EmberNodeId nodeId;
  uint8_t clientEndpoint;
  uint8_t serverEndpoint;
  uint8_t sequence;
  uint8_t calendarsToPublish[EMBER_AF_PLUGIN_CALENDAR_COMMON_TOTAL_CALENDARS];
  uint8_t numberOfEvents;
  uint8_t calendarIndex;
  uint8_t startIndex;
} PublishInfo;
static PublishInfo publishInfo;

typedef struct {
  uint32_t providerId;
  uint32_t issuerEventId;
  uint8_t calendarType;
} CancelCalendarInfo;
static CancelCalendarInfo cancelCalendarInfo;
static CancelCalendarInfo *lastCancelCalendar = NULL;
//-----------------------------------------------------------------------------

void emberAfCalendarClusterServerInitCallback(uint8_t endpoint)
{
  if (endpoint != 0 && myEndpoint == 0) {
    myEndpoint = endpoint;
  }

  publishInfo.commandIndex = EMBER_AF_PLUGIN_CALENDAR_COMMON_INVALID_INDEX;
}

// This function is scheduled upon receipt of one of the various Get commands.
// Every tick we'll send an apprpriate Publish command back to the requestor
// until all responses are sent.
void emberAfCalendarClusterServerTickCallback(uint8_t endpoint)
{
  bool commandSent = false;

  if (publishInfo.commandIndex == 0) {
    emberAfPluginCalendarServerPublishInfoCallback(publishInfo.publishCommandId,
                                                   publishInfo.nodeId,
                                                   publishInfo.clientEndpoint,
                                                   publishInfo.totalCommands);
  }

  if (publishInfo.publishCommandId == ZCL_PUBLISH_CALENDAR_COMMAND_ID) {
    uint8_t calendarIndex = publishInfo.calendarsToPublish[publishInfo.commandIndex];
    emberAfCalendarServerPublishCalendarMessage(publishInfo.nodeId,
                                                publishInfo.serverEndpoint,
                                                publishInfo.clientEndpoint,
                                                calendarIndex);
    commandSent = true;
  } else if (publishInfo.publishCommandId == ZCL_PUBLISH_DAY_PROFILE_COMMAND_ID) {
    uint8_t dayIndex = publishInfo.startIndex + publishInfo.commandIndex;
    emberAfCalendarServerPublishDayProfilesMessage(publishInfo.nodeId,
                                                   publishInfo.serverEndpoint,
                                                   publishInfo.clientEndpoint,
                                                   publishInfo.calendarIndex,
                                                   dayIndex);
    commandSent = true;
  } else if (publishInfo.publishCommandId == ZCL_PUBLISH_WEEK_PROFILE_COMMAND_ID) {
    uint8_t weekIndex = publishInfo.startIndex + publishInfo.commandIndex;
    emberAfCalendarServerPublishWeekProfileMessage(publishInfo.nodeId,
                                                   publishInfo.serverEndpoint,
                                                   publishInfo.clientEndpoint,
                                                   publishInfo.calendarIndex,
                                                   weekIndex);
    commandSent = true;
  } else if (publishInfo.publishCommandId == ZCL_PUBLISH_SEASONS_COMMAND_ID) {
    emberAfCalendarServerPublishSeasonsMessage(publishInfo.nodeId,
                                               publishInfo.serverEndpoint,
                                               publishInfo.clientEndpoint,
                                               publishInfo.calendarIndex);
    commandSent = true;
  } else if (publishInfo.publishCommandId == ZCL_PUBLISH_SPECIAL_DAYS_COMMAND_ID) {
    uint8_t calendarIndex = publishInfo.calendarsToPublish[publishInfo.commandIndex];
    emberAfCalendarServerPublishSpecialDaysMessage(publishInfo.nodeId,
                                                   publishInfo.serverEndpoint,
                                                   publishInfo.clientEndpoint,
                                                   calendarIndex);
    commandSent = true;
  }

  if (commandSent && ++publishInfo.commandIndex < publishInfo.totalCommands) {
    emberAfScheduleServerTick(endpoint,
                              ZCL_CALENDAR_CLUSTER_ID,
                              MILLISECOND_TICKS_PER_QUARTERSECOND);
  } else {
    publishInfo.commandIndex = EMBER_AF_PLUGIN_CALENDAR_COMMON_INVALID_INDEX;
  }
}

bool emberAfCalendarClusterGetCalendarCallback(uint32_t earliestStartTime,
                                               uint32_t minIssuerEventId,
                                               uint8_t numberOfCalendars,
                                               uint8_t calendarType,
                                               uint32_t providerId)
{
  EmberAfClusterCommand *cmd = emberAfCurrentCommand();
  uint8_t i;

  emberAfCalendarClusterPrintln("RX: GetCalendar 0x%4x, 0x%4x, 0x%x, 0x%x, 0x%4x",
                                earliestStartTime,
                                minIssuerEventId,
                                numberOfCalendars,
                                calendarType,
                                providerId);

  // Only one Get can be processed at a time.
  if (publishInfo.commandIndex != EMBER_AF_PLUGIN_CALENDAR_COMMON_INVALID_INDEX) {
    emberAfCalendarClusterPrintln("%p%p%p",
                                  "Error: ",
                                  "Cannot get calendar: ",
                                  "only one Get command can be processed at a time");
    emberAfSendDefaultResponse(cmd, EMBER_ZCL_STATUS_FAILURE);
    return true;
  }

  publishInfo.totalCommands = 0;

  while (numberOfCalendars == 0 || publishInfo.totalCommands < numberOfCalendars) {
    uint32_t referenceUtc = MAX_INT32U_VALUE;
    uint8_t indexToSend = 0xFF;

    // Find active or scheduled calendars matching the filter fields in the
    // request that have not been sent out yet.  Of those, find the one that
    // starts the earliest.
    for (i = 0; i < EMBER_AF_PLUGIN_CALENDAR_COMMON_TOTAL_CALENDARS; i++) {
      if (calendars[i].calendarId != EMBER_AF_PLUGIN_CALENDAR_COMMON_INVALID_CALENDAR_ID
          && !READBITS(calendars[i].flags, EMBER_AF_PLUGIN_CALENDAR_COMMON_FLAGS_SENT)
          && (minIssuerEventId == EMBER_AF_PLUGIN_CALENDAR_COMMON_WILDCARD_ISSUER_ID
              || minIssuerEventId <= calendars[i].issuerEventId)
          && (calendarType == EMBER_AF_PLUGIN_CALENDAR_COMMON_WILDCARD_CALENDAR_TYPE
              || calendarType == calendars[i].calendarType)
          && (providerId == EMBER_AF_PLUGIN_CALENDAR_COMMON_WILDCARD_PROVIDER_ID
              || providerId == calendars[i].providerId)
          && earliestStartTime < emberAfPluginCalendarCommonEndTimeUtc(&(calendars[i]))
          && calendars[i].startTimeUtc < referenceUtc) {
        referenceUtc = calendars[i].startTimeUtc;
        indexToSend = i;
      }
    }

    // If no active or scheduled calendar were found, it either means there are
    // no active or scheduled calendars at the specified time or we've already
    // found all of them in previous iterations.  If we did find one, we send
    // it, mark it as sent, and move on.
    if (indexToSend == 0xFF) {
      break;
    } else {
      publishInfo.calendarsToPublish[publishInfo.totalCommands++] = indexToSend;
      SETBITS(calendars[indexToSend].flags, EMBER_AF_PLUGIN_CALENDAR_COMMON_FLAGS_SENT);
    }
  }

  // If we sent nothing, we return an error.  Otherwise, we need to roll
  // through all the calendars and clear the sent bit.
  if (publishInfo.totalCommands == 0) {
    emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_NOT_FOUND);
  } else {
    for (i = 0; i < EMBER_AF_PLUGIN_CALENDAR_COMMON_TOTAL_CALENDARS; i++) {
      if (READBITS(calendars[i].flags, EMBER_AF_PLUGIN_CALENDAR_COMMON_FLAGS_SENT)) {
        CLEARBITS(calendars[i].flags, EMBER_AF_PLUGIN_CALENDAR_COMMON_FLAGS_SENT);
      }
    }

    publishInfo.publishCommandId = ZCL_PUBLISH_CALENDAR_COMMAND_ID;
    publishInfo.commandIndex = 0;
    publishInfo.nodeId = cmd->source;
    publishInfo.clientEndpoint = cmd->apsFrame->sourceEndpoint;
    publishInfo.serverEndpoint = cmd->apsFrame->destinationEndpoint;
    publishInfo.sequence = cmd->seqNum;
    emberAfScheduleServerTick(emberAfCurrentEndpoint(),
                              ZCL_CALENDAR_CLUSTER_ID,
                              MILLISECOND_TICKS_PER_QUARTERSECOND);
  }

  return true;
}

bool emberAfCalendarClusterGetDayProfilesCallback(uint32_t providerId,
                                                  uint32_t issuerCalendarId,
                                                  uint8_t startDayId,
                                                  uint8_t numberOfDays)
{
  EmberAfClusterCommand *cmd = emberAfCurrentCommand();

  uint8_t calendarIndex = emberAfPluginCalendarCommonGetCalendarById(issuerCalendarId,
                                                                     providerId);

  emberAfCalendarClusterPrintln("RX: GetDayProfiles 0x%4x, 0x%4x, 0x%x, 0x%x",
                                providerId,
                                issuerCalendarId,
                                startDayId,
                                numberOfDays);

  if (calendarIndex == EMBER_AF_PLUGIN_CALENDAR_COMMON_INVALID_INDEX) {
    emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_NOT_FOUND);
    return true;
  }

  if (startDayId == 0) {
    emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_INVALID_FIELD);
    return true;
  }

  if (calendars[calendarIndex].numberOfDayProfiles < startDayId) {
    emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_NOT_FOUND);
    return true;
  }

  // Only one Get can be processed at a time.
  if (publishInfo.commandIndex != EMBER_AF_PLUGIN_CALENDAR_COMMON_INVALID_INDEX) {
    emberAfCalendarClusterPrintln("%p%p%p",
                                  "Error: ",
                                  "Cannot get day profiles: ",
                                  "only one Get command can be processed at a time");
    emberAfSendDefaultResponse(cmd, EMBER_ZCL_STATUS_FAILURE);
    return true;
  }

  publishInfo.totalCommands = calendars[calendarIndex].numberOfDayProfiles - startDayId + 1;
  if (numberOfDays != 0 && numberOfDays < publishInfo.totalCommands) {
    publishInfo.totalCommands = numberOfDays;
  }

  publishInfo.calendarIndex = calendarIndex;
  publishInfo.startIndex = startDayId - 1;

  publishInfo.publishCommandId = ZCL_PUBLISH_DAY_PROFILE_COMMAND_ID;
  publishInfo.commandIndex = 0;
  publishInfo.nodeId = cmd->source;
  publishInfo.clientEndpoint = cmd->apsFrame->sourceEndpoint;
  publishInfo.serverEndpoint = cmd->apsFrame->destinationEndpoint;
  publishInfo.sequence = cmd->seqNum;
  emberAfScheduleServerTick(emberAfCurrentEndpoint(),
                            ZCL_CALENDAR_CLUSTER_ID,
                            MILLISECOND_TICKS_PER_QUARTERSECOND);

  return true;
}

bool emberAfCalendarClusterGetWeekProfilesCallback(uint32_t providerId,
                                                   uint32_t issuerCalendarId,
                                                   uint8_t startWeekId,
                                                   uint8_t numberOfWeeks)
{
  EmberAfClusterCommand *cmd = emberAfCurrentCommand();

  uint8_t calendarIndex = emberAfPluginCalendarCommonGetCalendarById(issuerCalendarId,
                                                                     providerId);

  emberAfCalendarClusterPrintln("RX: GetWeekProfiles 0x%4x, 0x%4x, 0x%x, 0x%x",
                                providerId,
                                issuerCalendarId,
                                startWeekId,
                                numberOfWeeks);

  if (calendarIndex == EMBER_AF_PLUGIN_CALENDAR_COMMON_INVALID_INDEX) {
    emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_NOT_FOUND);
    return true;
  }

  if (startWeekId == 0) {
    emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_INVALID_FIELD);
    return true;
  }

  if (calendars[calendarIndex].numberOfWeekProfiles < startWeekId) {
    emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_NOT_FOUND);
    return true;
  }

  // Only one Get can be processed at a time.
  if (publishInfo.commandIndex != EMBER_AF_PLUGIN_CALENDAR_COMMON_INVALID_INDEX) {
    emberAfCalendarClusterPrintln("%p%p%p",
                                  "Error: ",
                                  "Cannot get week profile: ",
                                  "only one Get command can be processed at a time");
    emberAfSendDefaultResponse(cmd, EMBER_ZCL_STATUS_FAILURE);
    return true;
  }

  publishInfo.totalCommands = calendars[calendarIndex].numberOfWeekProfiles - startWeekId + 1;
  if (numberOfWeeks != 0 && numberOfWeeks < publishInfo.totalCommands) {
    publishInfo.totalCommands = numberOfWeeks;
  }

  publishInfo.calendarIndex = calendarIndex;
  publishInfo.startIndex = startWeekId - 1;

  publishInfo.publishCommandId = ZCL_PUBLISH_WEEK_PROFILE_COMMAND_ID;
  publishInfo.commandIndex = 0;
  publishInfo.nodeId = cmd->source;
  publishInfo.clientEndpoint = cmd->apsFrame->sourceEndpoint;
  publishInfo.serverEndpoint = cmd->apsFrame->destinationEndpoint;
  publishInfo.sequence = cmd->seqNum;
  emberAfScheduleServerTick(emberAfCurrentEndpoint(),
                            ZCL_CALENDAR_CLUSTER_ID,
                            MILLISECOND_TICKS_PER_QUARTERSECOND);

  return true;
}

bool emberAfCalendarClusterGetSeasonsCallback(uint32_t providerId,
                                              uint32_t issuerCalendarId)
{
  EmberAfClusterCommand *cmd = emberAfCurrentCommand();

  uint8_t calendarIndex = emberAfPluginCalendarCommonGetCalendarById(issuerCalendarId,
                                                                     providerId);
  emberAfCalendarClusterPrintln("RX: GetSeasons 0x%4x, 0x%4x",
                                providerId,
                                issuerCalendarId);

  if (calendarIndex == EMBER_AF_PLUGIN_CALENDAR_COMMON_INVALID_INDEX
      || calendars[calendarIndex].numberOfSeasons == 0) {
    emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_NOT_FOUND);
    return true;
  }

  // Only one Get can be processed at a time.
  if (publishInfo.commandIndex != EMBER_AF_PLUGIN_CALENDAR_COMMON_INVALID_INDEX) {
    emberAfCalendarClusterPrintln("%p%p%p",
                                  "Error: ",
                                  "Cannot get seasons: ",
                                  "only one Get command can be processed at a time");
    emberAfSendDefaultResponse(cmd, EMBER_ZCL_STATUS_FAILURE);
    return true;
  }

  publishInfo.totalCommands = 1;

  publishInfo.calendarIndex = calendarIndex;
  publishInfo.startIndex = 0;

  publishInfo.publishCommandId = ZCL_PUBLISH_SEASONS_COMMAND_ID;
  publishInfo.commandIndex = 0;
  publishInfo.nodeId = cmd->source;
  publishInfo.clientEndpoint = cmd->apsFrame->sourceEndpoint;
  publishInfo.serverEndpoint = cmd->apsFrame->destinationEndpoint;
  publishInfo.sequence = cmd->seqNum;
  emberAfScheduleServerTick(emberAfCurrentEndpoint(),
                            ZCL_CALENDAR_CLUSTER_ID,
                            MILLISECOND_TICKS_PER_QUARTERSECOND);

  return true;
}

bool emberAfCalendarClusterGetSpecialDaysCallback(uint32_t startTime,
                                                  uint8_t numberOfEvents,
                                                  uint8_t calendarType,
                                                  uint32_t providerId,
                                                  uint32_t issuerCalendarId)
{
  EmberAfClusterCommand *cmd = emberAfCurrentCommand();
  uint8_t numberOfSpecialDaysSent = 0;
  uint8_t i;

  emberAfCalendarClusterPrintln("RX: GetSpecialDays 0x%4x, 0x%x, 0x%x, 0x%4x, 0x%4x",
                                startTime,
                                numberOfEvents,
                                calendarType,
                                providerId,
                                issuerCalendarId);

  // TODO-SPEC: 12-0517-11 says that a start time of zero means now, but this
  // is apparently going away.  See comment TE6-7 in 13-0546-06 and Ian
  // Winterburn's email to zigbee_pro_energy@mail.zigbee.org on March 25, 2014.
  //if (startTime == 0) {
  //  startTime = emberAfGetCurrentTime();
  //}

  // Only one Get can be processed at a time.
  if (publishInfo.commandIndex != EMBER_AF_PLUGIN_CALENDAR_COMMON_INVALID_INDEX) {
    emberAfCalendarClusterPrintln("%p%p%p",
                                  "Error: ",
                                  "Cannot get special days: ",
                                  "only one Get command can be processed at a time");
    emberAfSendDefaultResponse(cmd, EMBER_ZCL_STATUS_FAILURE);
    return true;
  }

#if defined(EMBER_AF_PLUGIN_GAS_PROXY_FUNCTION)
  if (issuerCalendarId == GBCS_TARIFF_SWITCHING_CALENDAR_ID) {
    issuerCalendarId = tariffSwitchingCalendarId;
  } else if (issuerCalendarId == GBCS_NON_DISABLEMENT_CALENDAR_ID) {
    issuerCalendarId = nonDisablementCalendarId;
  }
#endif

  publishInfo.totalCommands = 0;

  while (numberOfEvents == 0 || numberOfSpecialDaysSent < numberOfEvents) {
    uint32_t referenceUtc = MAX_INT32U_VALUE;
    uint8_t indexToSend = 0xFF;

    // Find active or scheduled calendars matching the filter fields in the
    // request that have not been sent out yet.  Of those, find the one that
    // starts the earliest.
    for (i = 0; i < EMBER_AF_PLUGIN_CALENDAR_COMMON_TOTAL_CALENDARS; i++) {
      if (calendars[i].calendarId != EMBER_AF_PLUGIN_CALENDAR_COMMON_INVALID_CALENDAR_ID
          && !READBITS(calendars[i].flags, EMBER_AF_PLUGIN_CALENDAR_COMMON_FLAGS_SENT)
          && (calendarType == EMBER_AF_PLUGIN_CALENDAR_COMMON_WILDCARD_CALENDAR_TYPE
              || calendarType == calendars[i].calendarType)
          && (providerId == EMBER_AF_PLUGIN_CALENDAR_COMMON_WILDCARD_PROVIDER_ID
              || providerId == calendars[i].providerId)
          && (issuerCalendarId == EMBER_AF_PLUGIN_CALENDAR_COMMON_WILDCARD_CALENDAR_ID
              || issuerCalendarId == calendars[i].calendarId)
          && startTime < emberAfPluginCalendarCommonEndTimeUtc(&(calendars[i]))
          && calendars[i].startTimeUtc < referenceUtc) {
        referenceUtc = calendars[i].startTimeUtc;
        indexToSend = i;
      }
    }

    // If no active or scheduled calendar were found, it either means there are
    // no active or scheduled calendars at the specified time or we've already
    // found all of them in previous iterations.  If we did find one, we need
    // to look at, and maybe send, its special days before we move on.
    if (indexToSend == 0xFF) {
      break;
    } else {
      if (calendars[indexToSend].numberOfSpecialDayProfiles != 0) {
        publishInfo.calendarsToPublish[publishInfo.totalCommands++] = indexToSend;
        numberOfSpecialDaysSent += calendars[indexToSend].numberOfSpecialDayProfiles;
      }

      SETBITS(calendars[indexToSend].flags, EMBER_AF_PLUGIN_CALENDAR_COMMON_FLAGS_SENT);
    }
  }

  // Roll through all the calendars and clear the sent bit.
  for (i = 0; i < EMBER_AF_PLUGIN_CALENDAR_COMMON_TOTAL_CALENDARS; i++) {
    if (READBITS(calendars[i].flags, EMBER_AF_PLUGIN_CALENDAR_COMMON_FLAGS_SENT)) {
      CLEARBITS(calendars[i].flags, EMBER_AF_PLUGIN_CALENDAR_COMMON_FLAGS_SENT);
    }
  }

  // If there is nothing to send, we return an error.
  if (publishInfo.totalCommands == 0) {
    emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_NOT_FOUND);
  } else {
    publishInfo.publishCommandId = ZCL_PUBLISH_SPECIAL_DAYS_COMMAND_ID;
    publishInfo.commandIndex = 0;
    if (numberOfEvents == 0 || numberOfSpecialDaysSent < numberOfEvents) {
      publishInfo.numberOfEvents = numberOfSpecialDaysSent;
    } else {
      publishInfo.numberOfEvents = numberOfEvents;
    }
    publishInfo.nodeId = cmd->source;
    publishInfo.clientEndpoint = cmd->apsFrame->sourceEndpoint;
    publishInfo.serverEndpoint = cmd->apsFrame->destinationEndpoint;
    publishInfo.sequence = cmd->seqNum;
    emberAfScheduleServerTick(emberAfCurrentEndpoint(),
                              ZCL_CALENDAR_CLUSTER_ID,
                              MILLISECOND_TICKS_PER_QUARTERSECOND);
  }

  return true;
}

bool emberAfCalendarClusterGetCalendarCancellationCallback(void)
{
  emberAfCalendarClusterPrintln("RX: GetCalendarCancellation");

  if (lastCancelCalendar == NULL) {
    emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_NOT_FOUND);
  } else {
    emberAfFillCommandCalendarClusterCancelCalendar(lastCancelCalendar->providerId,
                                                    lastCancelCalendar->issuerEventId,
                                                    lastCancelCalendar->calendarType);
    emberAfSendResponse();
    lastCancelCalendar = NULL;
  }

  return true;
}

// Publish a calendar.
void emberAfCalendarServerPublishCalendarMessage(EmberNodeId nodeId,
                                                 uint8_t srcEndpoint,
                                                 uint8_t dstEndpoint,
                                                 uint8_t calendarIndex)
{
  EmberStatus status;

  if (EMBER_AF_PLUGIN_CALENDAR_COMMON_TOTAL_CALENDARS <= calendarIndex) {
    emberAfCalendarClusterPrintln("%p%p%p",
                                  "Error: ",
                                  "Cannot publish calendar: ",
                                  "invalid calendar index");
    return;
  }

  emberAfFillCommandCalendarClusterPublishCalendar(calendars[calendarIndex].providerId,
                                                   calendars[calendarIndex].issuerEventId,
                                                   calendars[calendarIndex].calendarId,
                                                   calendars[calendarIndex].startTimeUtc,
                                                   calendars[calendarIndex].calendarType,
                                                   EMBER_ZCL_CALENDAR_TIME_REFERENCE_UTC_TIME,
                                                   calendars[calendarIndex].name,
                                                   calendars[calendarIndex].numberOfSeasons,
                                                   calendars[calendarIndex].numberOfWeekProfiles,
                                                   calendars[calendarIndex].numberOfDayProfiles);
  // If we are in the process of sending multiple publish calendar commands
  // in response to the get calendar request then set the sequence number from
  // the request.
  if (publishInfo.commandIndex != EMBER_AF_PLUGIN_CALENDAR_COMMON_INVALID_INDEX) {
    appResponseData[1] = publishInfo.sequence;
  }
  emberAfSetCommandEndpoints(srcEndpoint, dstEndpoint);
  emberAfGetCommandApsFrame()->options |= EMBER_APS_OPTION_SOURCE_EUI64;
  status = emberAfSendCommandUnicast(EMBER_OUTGOING_DIRECT, nodeId);
  if (status != EMBER_SUCCESS) {
    emberAfCalendarClusterPrintln("%p%p0x%x",
                                  "Error: ",
                                  "Cannot publish calendar: ",
                                  status);
  }
}

void emberAfCalendarServerPublishDayProfilesMessage(EmberNodeId nodeId,
                                                    uint8_t srcEndpoint,
                                                    uint8_t dstEndpoint,
                                                    uint8_t calendarIndex,
                                                    uint8_t dayIndex)
{
  EmberStatus status;
  uint8_t i;

  if (EMBER_AF_PLUGIN_CALENDAR_COMMON_TOTAL_CALENDARS <= calendarIndex) {
    emberAfCalendarClusterPrintln("%p%p%p",
                                  "Error: ",
                                  "Cannot publish day profiles: ",
                                  "invalid calendar index");
    return;
  }

  if (calendars[calendarIndex].numberOfDayProfiles <= dayIndex) {
    emberAfCalendarClusterPrintln("%p%p%p",
                                  "Error: ",
                                  "Cannot publish day profiles: ",
                                  "invalid day index");
    return;
  }

  // For GBCS use cases, we should be setting the disable default response bit.
  emberAfFillExternalBuffer((ZCL_CLUSTER_SPECIFIC_COMMAND
                             | ZCL_FRAME_CONTROL_SERVER_TO_CLIENT
#ifdef EMBER_AF_GBCS_COMPATIBLE
                             | ZCL_DISABLE_DEFAULT_RESPONSE_MASK
#endif
                             ),
                            ZCL_CALENDAR_CLUSTER_ID,
                            ZCL_PUBLISH_DAY_PROFILE_COMMAND_ID,
                            "wwwuuuuu",
                            calendars[calendarIndex].providerId,
                            calendars[calendarIndex].issuerEventId,
                            calendars[calendarIndex].calendarId,
                            calendars[calendarIndex].normalDays[dayIndex].id,
                            calendars[calendarIndex].normalDays[dayIndex].numberOfScheduleEntries,
                            0, // command index
                            1, // total commands
                            calendars[calendarIndex].calendarType);

  for (i = 0; i < calendars[calendarIndex].normalDays[dayIndex].numberOfScheduleEntries; i++) {
    emberAfPutInt16uInResp(calendars[calendarIndex].normalDays[dayIndex].scheduleEntries[i].minutesFromMidnight);
    emberAfPutInt8uInResp(calendars[calendarIndex].normalDays[dayIndex].scheduleEntries[i].data);
  }

  // If we are in the process of sending multiple publish day profiles commands
  // in response to the get day profiles request then set the sequence number from
  // the request.
  if (publishInfo.commandIndex != EMBER_AF_PLUGIN_CALENDAR_COMMON_INVALID_INDEX) {
    appResponseData[1] = publishInfo.sequence;
  }
  emberAfSetCommandEndpoints(srcEndpoint, dstEndpoint);
  emberAfGetCommandApsFrame()->options |= EMBER_APS_OPTION_SOURCE_EUI64;
  status = emberAfSendCommandUnicast(EMBER_OUTGOING_DIRECT, nodeId);
  if (status != EMBER_SUCCESS) {
    emberAfCalendarClusterPrintln("%p%p0x%x",
                                  "Error: ",
                                  "Cannot publish day profiles: ",
                                  status);
  }
}

void emberAfCalendarServerPublishWeekProfileMessage(EmberNodeId nodeId,
                                                    uint8_t srcEndpoint,
                                                    uint8_t dstEndpoint,
                                                    uint8_t calendarIndex,
                                                    uint8_t weekIndex)
{
  EmberStatus status;

  if (EMBER_AF_PLUGIN_CALENDAR_COMMON_TOTAL_CALENDARS <= calendarIndex) {
    emberAfCalendarClusterPrintln("%p%p%p",
                                  "Error: ",
                                  "Cannot publish week profile: ",
                                  "invalid calendar index");
    return;
  }

  if (calendars[calendarIndex].numberOfWeekProfiles <= weekIndex) {
    emberAfCalendarClusterPrintln("%p%p%p",
                                  "Error: ",
                                  "Cannot publish week profile: ",
                                  "invalid week index");
    return;
  }

#ifdef EMBER_AF_GBCS_COMPATIBLE
  // GBCS explicitly lists some commands that need to be sent with "disable
  // default response" flag set. This is one of them.
  // We make it conditional on GBCS so it does not affect standard SE apps.
  emberAfSetDisableDefaultResponse(EMBER_AF_DISABLE_DEFAULT_RESPONSE_ONE_SHOT);
#endif

  emberAfFillCommandCalendarClusterPublishWeekProfile(calendars[calendarIndex].providerId,
                                                      calendars[calendarIndex].issuerEventId,
                                                      calendars[calendarIndex].calendarId,
                                                      calendars[calendarIndex].weeks[weekIndex].id,
                                                      calendars[calendarIndex].normalDays[calendars[calendarIndex].weeks[weekIndex].normalDayIndexes[0]].id,
                                                      calendars[calendarIndex].normalDays[calendars[calendarIndex].weeks[weekIndex].normalDayIndexes[1]].id,
                                                      calendars[calendarIndex].normalDays[calendars[calendarIndex].weeks[weekIndex].normalDayIndexes[2]].id,
                                                      calendars[calendarIndex].normalDays[calendars[calendarIndex].weeks[weekIndex].normalDayIndexes[3]].id,
                                                      calendars[calendarIndex].normalDays[calendars[calendarIndex].weeks[weekIndex].normalDayIndexes[4]].id,
                                                      calendars[calendarIndex].normalDays[calendars[calendarIndex].weeks[weekIndex].normalDayIndexes[5]].id,
                                                      calendars[calendarIndex].normalDays[calendars[calendarIndex].weeks[weekIndex].normalDayIndexes[6]].id);
  // If we are in the process of sending multiple publish day profiles commands
  // in response to the get day profiles request then set the sequence number from
  // the request.
  if (publishInfo.commandIndex != EMBER_AF_PLUGIN_CALENDAR_COMMON_INVALID_INDEX) {
    appResponseData[1] = publishInfo.sequence;
  }
  emberAfSetCommandEndpoints(srcEndpoint, dstEndpoint);
  emberAfGetCommandApsFrame()->options |= EMBER_APS_OPTION_SOURCE_EUI64;
  status = emberAfSendCommandUnicast(EMBER_OUTGOING_DIRECT, nodeId);
  if (status != EMBER_SUCCESS) {
    emberAfCalendarClusterPrintln("%p%p0x%x",
                                  "Error: ",
                                  "Cannot publish week profile: ",
                                  status);
  }
}

void emberAfCalendarServerPublishSeasonsMessage(EmberNodeId nodeId,
                                                uint8_t srcEndpoint,
                                                uint8_t dstEndpoint,
                                                uint8_t calendarIndex)
{
  EmberStatus status;
  uint8_t i;

  if (EMBER_AF_PLUGIN_CALENDAR_COMMON_TOTAL_CALENDARS <= calendarIndex) {
    emberAfCalendarClusterPrintln("%p%p%p",
                                  "Error: ",
                                  "Cannot publish seasons: ",
                                  "invalid calendar index");
    return;
  }

  // For now we don't support segmenting commands since it isn't clear in the
  // spec how this is done.  APS Fragmentation would be better since it is
  // already used by other clusters.
  emberAfFillExternalBuffer((ZCL_CLUSTER_SPECIFIC_COMMAND
                             | ZCL_FRAME_CONTROL_SERVER_TO_CLIENT
#ifdef EMBER_AF_GBCS_COMPATIBLE
                             | ZCL_DISABLE_DEFAULT_RESPONSE_MASK
#endif
                             ),
                            ZCL_CALENDAR_CLUSTER_ID,
                            ZCL_PUBLISH_SEASONS_COMMAND_ID,
                            "wwwuu",
                            calendars[calendarIndex].providerId,
                            calendars[calendarIndex].issuerEventId,
                            calendars[calendarIndex].calendarId,
                            0, // command index
                            1); // total commands

  for (i = 0; i < calendars[calendarIndex].numberOfSeasons; i++) {
    emberAfPutDateInResp(&calendars[calendarIndex].seasons[i].startDate);
    emberAfPutInt8uInResp(calendars[calendarIndex].weeks[calendars[calendarIndex].seasons[i].weekIndex].id);
  }

  // If we are in the process of sending multiple publish day profiles commands
  // in response to the get day profiles request then set the sequence number from
  // the request.
  if (publishInfo.commandIndex != EMBER_AF_PLUGIN_CALENDAR_COMMON_INVALID_INDEX) {
    appResponseData[1] = publishInfo.sequence;
  }
  emberAfSetCommandEndpoints(srcEndpoint, dstEndpoint);
  emberAfGetCommandApsFrame()->options |= EMBER_APS_OPTION_SOURCE_EUI64;
  status = emberAfSendCommandUnicast(EMBER_OUTGOING_DIRECT, nodeId);
  if (status != EMBER_SUCCESS) {
    emberAfCalendarClusterPrintln("%p%p0x%x",
                                  "Error: ",
                                  "Cannot publish seasons: ",
                                  status);
  }
}

void emberAfCalendarServerPublishSpecialDaysMessage(EmberNodeId nodeId,
                                                    uint8_t srcEndpoint,
                                                    uint8_t dstEndpoint,
                                                    uint8_t calendarIndex)
{
  EmberStatus status;
  uint8_t maxDaysToSend = 0;
  uint8_t daysToSend = 0;
  uint8_t daysSent = 0;
  uint8_t i;

  if (EMBER_AF_PLUGIN_CALENDAR_COMMON_TOTAL_CALENDARS <= calendarIndex) {
    emberAfCalendarClusterPrintln("%p%p%p",
                                  "Error: ",
                                  "Cannot publish special days: ",
                                  "invalid calendar index");
    return;
  }

  // If we are in the process of sending multiple publish special days commands
  // grab the numberOfEvents remaining.
  if (publishInfo.commandIndex != EMBER_AF_PLUGIN_CALENDAR_COMMON_INVALID_INDEX) {
    maxDaysToSend = publishInfo.numberOfEvents;
  }

  if (maxDaysToSend == 0 || calendars[calendarIndex].numberOfSpecialDayProfiles < maxDaysToSend) {
    daysToSend = calendars[calendarIndex].numberOfSpecialDayProfiles;
  } else {
    daysToSend = maxDaysToSend;
  }

  emberAfFillExternalBuffer((ZCL_CLUSTER_SPECIFIC_COMMAND
                             | ZCL_FRAME_CONTROL_SERVER_TO_CLIENT
#ifdef EMBER_AF_GBCS_COMPATIBLE
                             | ZCL_DISABLE_DEFAULT_RESPONSE_MASK
#endif
                             ),
                            ZCL_CALENDAR_CLUSTER_ID,
                            ZCL_PUBLISH_SPECIAL_DAYS_COMMAND_ID,
                            "wwwwuuuu",
                            calendars[calendarIndex].providerId,
                            calendars[calendarIndex].issuerEventId,
                            calendars[calendarIndex].calendarId,
                            calendars[calendarIndex].startTimeUtc,
                            calendars[calendarIndex].calendarType,
                            daysToSend,
                            0, // command index
                            1); // total commands

  // Find active or scheduled special days that have not been added to the
  // payload.  Of those, find the one that starts the earliest.
  while (daysSent < daysToSend) {
    uint8_t indexToAdd = 0xFF;
    for (i = 0; i < calendars[calendarIndex].numberOfSpecialDayProfiles; i++) {
      if (!READBITS(calendars[calendarIndex].specialDays[i].flags, EMBER_AF_PLUGIN_CALENDAR_COMMON_FLAGS_SENT)
          && (indexToAdd == 0xFF
              || emberAfCompareDates(&calendars[calendarIndex].specialDays[i].startDate,
                                     &calendars[calendarIndex].specialDays[indexToAdd].startDate) < 0)) {
        indexToAdd = i;
      }
    }

    // If no active or scheduled special days were found, it either means
    // there are no active or scheduled special days at the specified time
    // or we've already found all of them in previous iterations.  If we
    // did find one, we add it, mark it as added, and move on.
    if (indexToAdd == 0xFF) {
      break;
    } else {
      emberAfPutDateInResp(&calendars[calendarIndex].specialDays[indexToAdd].startDate);
      emberAfPutInt8uInResp(calendars[calendarIndex].normalDays[calendars[calendarIndex].specialDays[indexToAdd].normalDayIndex].id);
      daysSent++;
      SETBITS(calendars[calendarIndex].specialDays[indexToAdd].flags, EMBER_AF_PLUGIN_CALENDAR_COMMON_FLAGS_SENT);
    }
  }

  // If we are in the process of sending multiple publish special days commands
  // in response to the get special days request then set the sequence number from
  // the request.
  if (publishInfo.commandIndex != EMBER_AF_PLUGIN_CALENDAR_COMMON_INVALID_INDEX) {
    appResponseData[1] = publishInfo.sequence;
    publishInfo.numberOfEvents -= daysSent;
  }
  emberAfSetCommandEndpoints(srcEndpoint, dstEndpoint);
  emberAfGetCommandApsFrame()->options |= EMBER_APS_OPTION_SOURCE_EUI64;
  status = emberAfSendCommandUnicast(EMBER_OUTGOING_DIRECT, nodeId);
  if (status != EMBER_SUCCESS) {
    emberAfCalendarClusterPrintln("%p%p0x%x",
                                  "Error: ",
                                  "Cannot publish special days: ",
                                  status);
  }

  // Roll through the special days for the given calendar and clear the sent bits
  for (i = 0; i < calendars[calendarIndex].numberOfSpecialDayProfiles; i++) {
    if (READBITS(calendars[calendarIndex].specialDays[i].flags, EMBER_AF_PLUGIN_CALENDAR_COMMON_FLAGS_SENT)) {
      CLEARBITS(calendars[calendarIndex].specialDays[i].flags, EMBER_AF_PLUGIN_CALENDAR_COMMON_FLAGS_SENT);
    }
  }
}

// Send CancelCalendar.
void emberAfCalendarServerCancelCalendarMessage(EmberNodeId nodeId,
                                                uint8_t srcEndpoint,
                                                uint8_t dstEndpoint,
                                                uint8_t calendarIndex)
{
  EmberStatus status;

  if (EMBER_AF_PLUGIN_CALENDAR_COMMON_TOTAL_CALENDARS <= calendarIndex) {
    emberAfCalendarClusterPrintln("%p%p%p",
                                  "Error: ",
                                  "Cannot send cancel calendar: ",
                                  "invalid calendar index");
    return;
  }

  emberAfFillCommandCalendarClusterCancelCalendar(calendars[calendarIndex].providerId,
                                                  calendars[calendarIndex].issuerEventId,
                                                  calendars[calendarIndex].calendarType);
  emberAfSetCommandEndpoints(srcEndpoint, dstEndpoint);
  emberAfGetCommandApsFrame()->options |= EMBER_APS_OPTION_SOURCE_EUI64;
  status = emberAfSendCommandUnicast(EMBER_OUTGOING_DIRECT, nodeId);
  if (status != EMBER_SUCCESS) {
    emberAfCalendarClusterPrintln("%p%p0x%x",
                                  "Error: ",
                                  "Cannot publish calendar: ",
                                  status);
  } else {
    cancelCalendarInfo.providerId = calendars[calendarIndex].providerId;
    cancelCalendarInfo.issuerEventId = calendars[calendarIndex].issuerEventId;
    cancelCalendarInfo.calendarType = calendars[calendarIndex].calendarType;
    lastCancelCalendar = &cancelCalendarInfo;
  }
}
