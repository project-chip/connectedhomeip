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
 * @brief APIs for the Calendar Server plugin.
 *******************************************************************************
   ******************************************************************************/

/**
 * @brief Publishes a calendar.
 *
 * Locates the calendar in the calendar table at the specified location and
 * sends a PublishCalendar command using its information.
 *
 * @param nodeId The destination nodeId.
 * @param srcEndpoint The source endpoint.
 * @param dstEndpoint The destination endpoint.
 * @param calendarIndex The index in the calendar table.
 **/
void emberAfCalendarServerPublishCalendarMessage(EmberNodeId nodeId,
                                                 uint8_t srcEndpoint,
                                                 uint8_t dstEndpoint,
                                                 uint8_t calendarIndex);

/**
 * @brief Publish the day profiles of the specified day in the specified calendar.
 *
 * Locates the calendar in the calendar table at the specified location and
 * sends a PublishDayProfiles command using its information.
 *
 * @param nodeId The destination nodeId.
 * @param srcEndpoint The source endpoint.
 * @param dstEndpoint The destination endpoint.
 * @param calendarIndex The index in the calendar table.
 * @param dayIndex The index of the day in the calendar.
 **/
void emberAfCalendarServerPublishDayProfilesMessage(EmberNodeId nodeId,
                                                    uint8_t srcEndpoint,
                                                    uint8_t dstEndpoint,
                                                    uint8_t calendarIndex,
                                                    uint8_t dayIndex);

/**
 * @brief Publish the week profile of the specified week in the specified calendar.
 *
 * Locates the calendar in the calendar table at the specified location and
 * sends a PublishWeekProfile command using its information.
 *
 * @param nodeId The destination nodeId
 * @param srcEndpoint The source endpoint
 * @param dstEndpoint The destination endpoint
 * @param calendarIndex The index in the calendar table.
 * @param weekIndex The index of the week in the calendar.
 **/
void emberAfCalendarServerPublishWeekProfileMessage(EmberNodeId nodeId,
                                                    uint8_t srcEndpoint,
                                                    uint8_t dstEndpoint,
                                                    uint8_t calendarIndex,
                                                    uint8_t weekIndex);

/**
 * @brief Publish the seasons in the specified calendar.
 *
 * Locates the calendar in the calendar table at the specified location and
 * sends a PublishSeasons command using its information.
 *
 * @param nodeId The destination nodeId.
 * @param srcEndpoint The source endpoint.
 * @param dstEndpoint The destination endpoint.
 * @param calendarIndex The index in the calendar table.
 **/
void emberAfCalendarServerPublishSeasonsMessage(EmberNodeId nodeId,
                                                uint8_t srcEndpoint,
                                                uint8_t dstEndpoint,
                                                uint8_t calendarIndex);

/**
 * @brief Publish the special days of the specified calendar.
 *
 * Locates the calendar in the calendar table at the specified location and
 * sends a PublishSpecialDays command using its information.
 *
 * @param nodeId The destination nodeId.
 * @param srcEndpoint The source endpoint.
 * @param dstEndpoint The destination endpoint.
 * @param calendarIndex The index in the calendar table.
 **/
void emberAfCalendarServerPublishSpecialDaysMessage(EmberNodeId nodeId,
                                                    uint8_t srcEndpoint,
                                                    uint8_t dstEndpoint,
                                                    uint8_t calendarIndex);

/**
 * @brief Publish the special days of the specified calendar.
 *
 * Locates the calendar in the calendar table at the specified location and
 * sends a CancelCalendar command using its information.
 * Note: It is up to the caller to invalidate the local copy of the calendar.
 *
 * @param nodeId The destination nodeId.
 * @param srcEndpoint The source endpoint.
 * @param dstEndpoint The destination endpoint.
 * @param calendarIndex The index in the calendar table.
 **/
void emberAfCalendarServerCancelCalendarMessage(EmberNodeId nodeId,
                                                uint8_t srcEndpoint,
                                                uint8_t dstEndpoint,
                                                uint8_t calendarIndex);
