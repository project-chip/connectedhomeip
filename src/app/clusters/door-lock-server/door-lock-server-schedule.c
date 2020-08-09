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
/****************************************************************************
 * @file
 * @brief Routines for the Door Lock Server plugin.
 *******************************************************************************
 ******************************************************************************/

#include "af.h"
#include "door-lock-server.h"

static EmberAfPluginDoorLockServerWeekdayScheduleEntry
    weekdayScheduleTable[EMBER_AF_PLUGIN_DOOR_LOCK_SERVER_WEEKDAY_SCHEDULE_TABLE_SIZE];
static EmberAfPluginDoorLockServerYeardayScheduleEntry
    yeardayScheduleTable[EMBER_AF_PLUGIN_DOOR_LOCK_SERVER_YEARDAY_SCHEDULE_TABLE_SIZE];
static EmberAfPluginDoorLockServerHolidayScheduleEntry
    holidayScheduleTable[EMBER_AF_PLUGIN_DOOR_LOCK_SERVER_HOLIDAY_SCHEDULE_TABLE_SIZE];

static void printWeekdayScheduleTable(void)
{
    uint8_t i;
    emberAfDoorLockClusterPrintln("id uid dm strth strtm stph stpm");
    for (i = 0; i < EMBER_AF_PLUGIN_DOOR_LOCK_SERVER_WEEKDAY_SCHEDULE_TABLE_SIZE; i++)
    {
        EmberAfDoorLockScheduleEntry * entry = &weekdayScheduleTable[i];
        if (entry->inUse)
        {
            emberAfDoorLockClusterPrintln("%x %2x  %x %4x   %4x   %4x  %4x", i, entry->userId, entry->daysMask, entry->startHour,
                                          entry->stopHour, entry->stopMinute);
        }
    }
}

static void clearWeekdayScheduleTable(void)
{
    for (uint8_t i = 0; i < EMBER_AF_PLUGIN_DOOR_LOCK_SERVER_WEEKDAY_SCHEDULE_TABLE_SIZE; i++)
    {
        weekdayScheduleTable[i].inUse = false;
    }
}

static void clearYeardayScheduleTable(void)
{
    for (uint8_t i = 0; i < EMBER_AF_PLUGIN_DOOR_LOCK_SERVER_YEARDAY_SCHEDULE_TABLE_SIZE; i++)
    {
        yeardayScheduleTable[i].inUse = false;
    }
}

static void clearHolidayScheduleTable(void)
{
    for (uint8_t i = 0; i < EMBER_AF_PLUGIN_DOOR_LOCK_SERVER_HOLIDAY_SCHEDULE_TABLE_SIZE; i++)
    {
        holidayScheduleTable[i].inUse = false;
    }
}

void emAfPluginDoorLockServerInitSchedule(void)
{
    clearWeekdayScheduleTable();
    clearYeardayScheduleTable();
    clearHolidayScheduleTable();

#if defined(ZCL_USING_DOOR_LOCK_CLUSTER_NUM_WEEKDAY_SCHEDULES_SUPPORTED_PER_USER_ATTRIBUTE) ||                                     \
    defined(ZCL_USING_DOOR_LOCK_CLUSTER_NUM_YEARDAY_SCHEDULES_SUPPORTED_PER_USER_ATTRIBUTE) ||                                     \
    defined(ZCL_USING_DOOR_LOCK_CLUSTER_NUM_HOLIDAY_SCHEDULES_SUPPORTED_PER_USER_ATTRIBUTE)
    const EmAfPluginDoorLockServerAttributeData data[] = {
#ifdef ZCL_USING_DOOR_LOCK_CLUSTER_NUM_WEEKDAY_SCHEDULES_SUPPORTED_PER_USER_ATTRIBUTE
        { ZCL_NUM_WEEKDAY_SCHEDULES_SUPPORTED_PER_USER_ATTRIBUTE_ID, EMBER_AF_PLUGIN_DOOR_LOCK_SERVER_WEEKDAY_SCHEDULE_TABLE_SIZE },
#endif

#ifdef ZCL_USING_DOOR_LOCK_CLUSTER_NUM_YEARDAY_SCHEDULES_SUPPORTED_PER_USER_ATTRIBUTE
        { ZCL_NUM_YEARDAY_SCHEDULES_SUPPORTED_PER_USER_ATTRIBUTE_ID, EMBER_AF_PLUGIN_DOOR_LOCK_SERVER_YEARDAY_SCHEDULE_TABLE_SIZE },
#endif

#ifdef ZCL_USING_DOOR_LOCK_CLUSTER_NUM_HOLIDAY_SCHEDULES_SUPPORTED_PER_USER_ATTRIBUTE
        { ZCL_NUM_HOLIDAY_SCHEDULES_SUPPORTED_PER_USER_ATTRIBUTE_ID, EMBER_AF_PLUGIN_DOOR_LOCK_SERVER_HOLIDAY_SCHEDULE_TABLE_SIZE },
#endif
    };
    emAfPluginDoorLockServerWriteAttributes(data, COUNTOF(data), "schedule table");
#endif
}

static void sendResponse(const char * responseName)
{
    EmberStatus status = emberAfSendResponse();
    if (status != EMBER_SUCCESS)
    {
        emberAfDoorLockClusterPrintln("Failed to send %s: 0x%X", responseName, status);
    }
}

bool emberAfDoorLockClusterSetWeekdayScheduleCallback(uint8_t scheduleId, uint16_t userId, uint8_t daysMask, uint8_t startHour,
                                                      uint8_t startMinute, uint8_t stopHour, uint8_t stopMinute)
{
    uint8_t status                  = 0x00;
    uint8_t userPin                 = 0x00;
    uint16_t rfProgrammingEventMask = 0xffff; // event sent by default
    if (!emAfPluginDoorLockServerCheckForSufficientSpace(scheduleId,
                                                         EMBER_AF_PLUGIN_DOOR_LOCK_SERVER_WEEKDAY_SCHEDULE_TABLE_SIZE) ||
        !emAfPluginDoorLockServerCheckForSufficientSpace(userId, EMBER_AF_PLUGIN_DOOR_LOCK_SERVER_PIN_USER_TABLE_SIZE))
    {
        status = 0x01;
    }
    if (!status)
    {
        EmberAfDoorLockScheduleEntry * entry = &weekdayScheduleTable[scheduleId];
        entry->inUse                         = true;
        entry->userId                        = userId;
        entry->daysMask                      = daysMask;
        entry->startHour                     = startHour;
        entry->startMinute                   = startMinute;
        entry->stopHour                      = stopHour;
        entry->stopMinute                    = stopMinute;
        emberAfDoorLockClusterPrintln("***RX SET WEEKDAY SCHEDULE***");
        printWeekdayScheduleTable();
    }
    emberAfFillCommandDoorLockClusterSetWeekdayScheduleResponse(status);
    emberAfSendResponse();

    // get bitmask so we can check if we should send event notification
    emberAfReadServerAttribute(DOOR_LOCK_SERVER_ENDPOINT, ZCL_DOOR_LOCK_CLUSTER_ID, ZCL_RF_PROGRAMMING_EVENT_MASK_ATTRIBUTE_ID,
                               (uint8_t *) &rfProgrammingEventMask, sizeof(rfProgrammingEventMask));

    if (rfProgrammingEventMask & BIT(0))
    {
        emberAfFillCommandDoorLockClusterProgrammingEventNotification(0x01, 0x00, userId, &userPin, 0x00, 0x00, 0x00, &userPin);
        SEND_COMMAND_UNICAST_TO_BINDINGS();
    }

    return true;
}

bool emberAfDoorLockClusterGetWeekdayScheduleCallback(uint8_t scheduleId, uint16_t userId)
{
    EmberAfStatus zclStatus =
        ((scheduleId > EMBER_AF_PLUGIN_DOOR_LOCK_SERVER_WEEKDAY_SCHEDULE_TABLE_SIZE) ? EMBER_ZCL_STATUS_INVALID_FIELD
                                                                                     : EMBER_ZCL_STATUS_SUCCESS);
    EmberAfDoorLockScheduleEntry * entry = &weekdayScheduleTable[0];
    if (zclStatus == EMBER_ZCL_STATUS_SUCCESS)
    {
        entry     = &weekdayScheduleTable[scheduleId];
        zclStatus = (!entry->inUse ? EMBER_ZCL_STATUS_NOT_FOUND
                                   : (entry->userId != userId ? EMBER_ZCL_STATUS_NOT_FOUND : EMBER_ZCL_STATUS_SUCCESS));
    }

    emberAfFillCommandDoorLockClusterGetWeekdayScheduleResponse(scheduleId, userId, zclStatus, entry->daysMask, entry->startHour,
                                                                entry->startMinute, entry->stopHour, entry->stopMinute);

    sendResponse("GetWeekdayScheduleResponse");

    return true;
}

bool emberAfDoorLockClusterClearWeekdayScheduleCallback(uint8_t scheduleId, uint16_t userId)
{
    EmberAfStatus zclStatus =
        ((scheduleId > EMBER_AF_PLUGIN_DOOR_LOCK_SERVER_WEEKDAY_SCHEDULE_TABLE_SIZE) ? EMBER_ZCL_STATUS_INVALID_FIELD
                                                                                     : EMBER_ZCL_STATUS_SUCCESS);
    if (zclStatus == EMBER_ZCL_STATUS_SUCCESS)
    {
        weekdayScheduleTable[scheduleId].inUse = false;
        emAfPluginDoorLockServerSetPinUserType(userId, EMBER_ZCL_DOOR_LOCK_USER_TYPE_UNRESTRICTED);
    }

    emberAfFillCommandDoorLockClusterClearWeekdayScheduleResponse(zclStatus);

    sendResponse("ClearWeekdayScheduleResponse");

    return true;
}

bool emberAfDoorLockClusterSetYeardayScheduleCallback(uint8_t scheduleId, uint16_t userId, uint32_t localStartTime,
                                                      uint32_t localEndTime)
{
    uint8_t status;
    if (scheduleId >= EMBER_AF_PLUGIN_DOOR_LOCK_SERVER_YEARDAY_SCHEDULE_TABLE_SIZE)
    {
        status = 0x01; // failure (per 7.3.2.17.15)
    }
    else
    {
        yeardayScheduleTable[scheduleId].userId         = userId;
        yeardayScheduleTable[scheduleId].localStartTime = localStartTime;
        yeardayScheduleTable[scheduleId].localEndTime   = localEndTime;
        yeardayScheduleTable[scheduleId].inUse          = true;
        status                                          = 0x00; // success (per 7.3.2.17.15)
    }
    emberAfFillCommandDoorLockClusterSetYeardayScheduleResponse(status);

    sendResponse("SetYeardayScheduleResponse");

    return true;
}

bool emberAfDoorLockClusterGetYeardayScheduleCallback(uint8_t scheduleId, uint16_t userId)
{
    EmberAfPluginDoorLockServerYeardayScheduleEntry * entry = &yeardayScheduleTable[0];
    EmberAfStatus zclStatus;
    if (scheduleId >= EMBER_AF_PLUGIN_DOOR_LOCK_SERVER_YEARDAY_SCHEDULE_TABLE_SIZE ||
        userId >= EMBER_AF_PLUGIN_DOOR_LOCK_SERVER_PIN_USER_TABLE_SIZE)
    {
        zclStatus = EMBER_ZCL_STATUS_INVALID_FIELD;
    }
    else
    {
        entry = &yeardayScheduleTable[scheduleId];
        if (!entry->inUse || entry->userId != userId)
        {
            zclStatus = EMBER_ZCL_STATUS_NOT_FOUND;
        }
        else
        {
            zclStatus = EMBER_ZCL_STATUS_SUCCESS;
        }
    }

    emberAfFillCommandDoorLockClusterGetYeardayScheduleResponse(scheduleId, userId, zclStatus, entry->localStartTime,
                                                                entry->localEndTime);

    sendResponse("GetYeardayScheduleResponse");

    return true;
}

bool emberAfDoorLockClusterClearYeardayScheduleCallback(uint8_t scheduleId, uint16_t userId)
{
    uint8_t status;
    if (scheduleId >= EMBER_AF_PLUGIN_DOOR_LOCK_SERVER_YEARDAY_SCHEDULE_TABLE_SIZE)
    {
        status = 0x01; // failure (per 7.3.2.17.17)
    }
    else
    {
        yeardayScheduleTable[scheduleId].inUse = false;
        emAfPluginDoorLockServerSetPinUserType(userId, EMBER_ZCL_DOOR_LOCK_USER_TYPE_UNRESTRICTED);
        status = 0x00; // success (per 7.3.2.17.17)
    }
    emberAfFillCommandDoorLockClusterClearYeardayScheduleResponse(status);

    sendResponse("ClearYeardayScheduleResponse");

    return true;
}

bool emberAfDoorLockClusterSetHolidayScheduleCallback(uint8_t holidayScheduleId, uint32_t localStartTime, uint32_t localEndTime,
                                                      uint8_t operatingModeDuringHoliday)
{
    uint8_t status;
    if (holidayScheduleId >= EMBER_AF_PLUGIN_DOOR_LOCK_SERVER_HOLIDAY_SCHEDULE_TABLE_SIZE)
    {
        status = 0x01; // failure (per 7.3.2.17.18)
    }
    else
    {
        holidayScheduleTable[holidayScheduleId].localStartTime             = localStartTime;
        holidayScheduleTable[holidayScheduleId].localEndTime               = localEndTime;
        holidayScheduleTable[holidayScheduleId].operatingModeDuringHoliday = operatingModeDuringHoliday;
        holidayScheduleTable[holidayScheduleId].inUse                      = true;
        status                                                             = 0x00; // success (per 7.3.2.17.18)
    }
    emberAfFillCommandDoorLockClusterSetHolidayScheduleResponse(status);

    sendResponse("SetHolidayScheduleResponse");

    return true;
}

bool emberAfDoorLockClusterGetHolidayScheduleCallback(uint8_t holidayScheduleId)
{
    EmberAfPluginDoorLockServerHolidayScheduleEntry * entry = &holidayScheduleTable[0];
    EmberAfStatus zclStatus;
    if (holidayScheduleId >= EMBER_AF_PLUGIN_DOOR_LOCK_SERVER_HOLIDAY_SCHEDULE_TABLE_SIZE)
    {
        zclStatus = EMBER_ZCL_STATUS_INVALID_FIELD;
    }
    else
    {
        entry = &holidayScheduleTable[holidayScheduleId];
        if (!entry->inUse)
        {
            zclStatus = EMBER_ZCL_STATUS_NOT_FOUND;
        }
        else
        {
            zclStatus = EMBER_ZCL_STATUS_SUCCESS;
        }
    }

    emberAfFillCommandDoorLockClusterGetHolidayScheduleResponse(holidayScheduleId, zclStatus, entry->localStartTime,
                                                                entry->localEndTime, entry->operatingModeDuringHoliday);

    sendResponse("GetHolidayScheduleResponse");

    return true;
}

bool emberAfDoorLockClusterClearHolidayScheduleCallback(uint8_t holidayScheduleId)
{
    uint8_t status;
    if (holidayScheduleId >= EMBER_AF_PLUGIN_DOOR_LOCK_SERVER_YEARDAY_SCHEDULE_TABLE_SIZE)
    {
        status = 0x01; // failure (per 7.3.2.17.20)
    }
    else
    {
        holidayScheduleTable[holidayScheduleId].inUse = false;
        status                                        = 0x00; // success (per 7.3.2.17.20)
    }
    emberAfFillCommandDoorLockClusterClearHolidayScheduleResponse(status);

    sendResponse("ClearYeardayScheduleResponse");

    return true;
}
