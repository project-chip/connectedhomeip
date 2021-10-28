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

#include "door-lock-server.h"
#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <app/util/af.h>

#include <app/CommandHandler.h>
#include <app/ConcreteCommandPath.h>
#include <lib/support/CodeUtils.h>

using namespace chip;
using namespace chip::app::Clusters::DoorLock;

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
                                          entry->startMinute, entry->stopHour, entry->stopMinute);
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
        { Attributes::NumWeekdaySchedulesSupportedPerUser::Id, EMBER_AF_PLUGIN_DOOR_LOCK_SERVER_WEEKDAY_SCHEDULE_TABLE_SIZE },
#endif

#ifdef ZCL_USING_DOOR_LOCK_CLUSTER_NUM_YEARDAY_SCHEDULES_SUPPORTED_PER_USER_ATTRIBUTE
        { Attributes::NumYeardaySchedulesSupportedPerUser::Id, EMBER_AF_PLUGIN_DOOR_LOCK_SERVER_YEARDAY_SCHEDULE_TABLE_SIZE },
#endif

#ifdef ZCL_USING_DOOR_LOCK_CLUSTER_NUM_HOLIDAY_SCHEDULES_SUPPORTED_PER_USER_ATTRIBUTE
        { Attributes::NumHolidaySchedulesSupportedPerUser::Id, EMBER_AF_PLUGIN_DOOR_LOCK_SERVER_HOLIDAY_SCHEDULE_TABLE_SIZE },
#endif
    };
    emAfPluginDoorLockServerWriteAttributes(data, ArraySize(data), "schedule table");
#endif
}

bool emberAfDoorLockClusterSetWeekdayScheduleCallback(app::CommandHandler * commandObj,
                                                      const app::ConcreteCommandPath & commandPath,
                                                      const Commands::SetWeekdaySchedule::DecodableType & commandData)
{
    auto & scheduleId  = commandData.scheduleId;
    auto & userId      = commandData.userId;
    auto & daysMask    = commandData.daysMask;
    auto & startHour   = commandData.startHour;
    auto & startMinute = commandData.startMinute;
    auto & stopHour    = commandData.endHour;
    auto & stopMinute  = commandData.endMinute;

    uint8_t status                  = 0x00;
    uint8_t userPin                 = 0x00;
    uint16_t rfProgrammingEventMask = 0xffff; // event sent by default
    CHIP_ERROR err                  = CHIP_NO_ERROR;
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
        entry->daysMask                      = daysMask.Raw();
        entry->startHour                     = startHour;
        entry->startMinute                   = startMinute;
        entry->stopHour                      = stopHour;
        entry->stopMinute                    = stopMinute;
        emberAfDoorLockClusterPrintln("***RX SET WEEKDAY SCHEDULE***");
        printWeekdayScheduleTable();
    }

    {
        app::CommandPathParams cmdParams = { emberAfCurrentEndpoint(), /* group id */ 0, ::Id,
                                             Commands::SetWeekdayScheduleResponse::Id, (app::CommandPathFlags::kEndpointIdValid) };
        TLV::TLVWriter * writer          = nullptr;
        SuccessOrExit(err = commandObj->PrepareCommand(cmdParams));
        VerifyOrExit((writer = commandObj->GetCommandDataIBTLVWriter()) != nullptr, err = CHIP_ERROR_INCORRECT_STATE);
        SuccessOrExit(err = writer->Put(TLV::ContextTag(0), status));
        SuccessOrExit(err = commandObj->FinishCommand());
    }

    // get bitmask so we can check if we should send event notification
    Attributes::RfProgrammingEventMask::Get(DOOR_LOCK_SERVER_ENDPOINT, &rfProgrammingEventMask);

    if (rfProgrammingEventMask & EMBER_BIT(0))
    {
        emberAfFillExternalBuffer((ZCL_CLUSTER_SPECIFIC_COMMAND | ZCL_FRAME_CONTROL_SERVER_TO_CLIENT), ::Id,
                                  Commands::ProgrammingEventNotification::Id, "uuvsuuws", 0x01, 0x00, userId, &userPin, 0x00, 0x00,
                                  0x00, &userPin);
        SEND_COMMAND_UNICAST_TO_BINDINGS();
    }
exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "Failed to encode response command.");
    }
    return true;
}

bool emberAfDoorLockClusterGetWeekdayScheduleCallback(app::CommandHandler * commandObj,
                                                      const app::ConcreteCommandPath & commandPath,
                                                      const Commands::GetWeekdaySchedule::DecodableType & commandData)
{
    auto & scheduleId = commandData.scheduleId;
    auto & userId     = commandData.userId;

    EmberAfStatus zclStatus =
        ((scheduleId > EMBER_AF_PLUGIN_DOOR_LOCK_SERVER_WEEKDAY_SCHEDULE_TABLE_SIZE) ? EMBER_ZCL_STATUS_INVALID_FIELD
                                                                                     : EMBER_ZCL_STATUS_SUCCESS);
    EmberAfDoorLockScheduleEntry * entry = &weekdayScheduleTable[0];
    CHIP_ERROR err                       = CHIP_NO_ERROR;
    if (zclStatus == EMBER_ZCL_STATUS_SUCCESS)
    {
        entry     = &weekdayScheduleTable[scheduleId];
        zclStatus = (!entry->inUse ? EMBER_ZCL_STATUS_NOT_FOUND
                                   : (entry->userId != userId ? EMBER_ZCL_STATUS_NOT_FOUND : EMBER_ZCL_STATUS_SUCCESS));
    }

    {
        app::CommandPathParams cmdParams = { emberAfCurrentEndpoint(), /* group id */ 0, ::Id,
                                             Commands::GetWeekdayScheduleResponse::Id, (app::CommandPathFlags::kEndpointIdValid) };
        TLV::TLVWriter * writer          = nullptr;
        SuccessOrExit(err = commandObj->PrepareCommand(cmdParams));
        VerifyOrExit((writer = commandObj->GetCommandDataIBTLVWriter()) != nullptr, err = CHIP_ERROR_INCORRECT_STATE);
        SuccessOrExit(err = writer->Put(TLV::ContextTag(0), scheduleId));
        SuccessOrExit(err = writer->Put(TLV::ContextTag(1), userId));
        SuccessOrExit(err = writer->Put(TLV::ContextTag(2), zclStatus));
        SuccessOrExit(err = writer->Put(TLV::ContextTag(3), entry->daysMask));
        SuccessOrExit(err = writer->Put(TLV::ContextTag(4), entry->startHour));
        SuccessOrExit(err = writer->Put(TLV::ContextTag(5), entry->startMinute));
        SuccessOrExit(err = writer->Put(TLV::ContextTag(6), entry->stopHour));
        SuccessOrExit(err = writer->Put(TLV::ContextTag(7), entry->stopMinute));
        SuccessOrExit(err = commandObj->FinishCommand());
    }
exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "Failed to encode response command.");
    }
    return true;
}

bool emberAfDoorLockClusterClearWeekdayScheduleCallback(app::CommandHandler * commandObj,
                                                        const app::ConcreteCommandPath & commandPath,
                                                        const Commands::ClearWeekdaySchedule::DecodableType & commandData)
{
    auto & scheduleId = commandData.scheduleId;
    auto & userId     = commandData.userId;

    EmberAfStatus zclStatus =
        ((scheduleId > EMBER_AF_PLUGIN_DOOR_LOCK_SERVER_WEEKDAY_SCHEDULE_TABLE_SIZE) ? EMBER_ZCL_STATUS_INVALID_FIELD
                                                                                     : EMBER_ZCL_STATUS_SUCCESS);
    CHIP_ERROR err = CHIP_NO_ERROR;
    if (zclStatus == EMBER_ZCL_STATUS_SUCCESS)
    {
        weekdayScheduleTable[scheduleId].inUse = false;
        emAfPluginDoorLockServerSetPinUserType(userId, EMBER_ZCL_DOOR_LOCK_USER_TYPE_UNRESTRICTED);
    }

    {
        app::CommandPathParams cmdParams = { emberAfCurrentEndpoint(), /* group id */ 0, ::Id,
                                             Commands::ClearWeekdayScheduleResponse::Id,
                                             (app::CommandPathFlags::kEndpointIdValid) };
        TLV::TLVWriter * writer          = nullptr;
        SuccessOrExit(err = commandObj->PrepareCommand(cmdParams));
        VerifyOrExit((writer = commandObj->GetCommandDataIBTLVWriter()) != nullptr, err = CHIP_ERROR_INCORRECT_STATE);
        SuccessOrExit(err = writer->Put(TLV::ContextTag(0), zclStatus));
        SuccessOrExit(err = commandObj->FinishCommand());
    }
exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "Failed to encode response command.");
    }
    return true;
}

bool emberAfDoorLockClusterSetYeardayScheduleCallback(app::CommandHandler * commandObj,
                                                      const app::ConcreteCommandPath & commandPath,
                                                      const Commands::SetYeardaySchedule::DecodableType & commandData)
{
    auto & scheduleId     = commandData.scheduleId;
    auto & userId         = commandData.userId;
    auto & localStartTime = commandData.localStartTime;
    auto & localEndTime   = commandData.localEndTime;

    uint8_t status;
    CHIP_ERROR err = CHIP_NO_ERROR;
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

    {
        app::CommandPathParams cmdParams = { emberAfCurrentEndpoint(), /* group id */ 0, ::Id,
                                             Commands::SetYeardayScheduleResponse::Id, (app::CommandPathFlags::kEndpointIdValid) };
        TLV::TLVWriter * writer          = nullptr;
        SuccessOrExit(err = commandObj->PrepareCommand(cmdParams));
        VerifyOrExit((writer = commandObj->GetCommandDataIBTLVWriter()) != nullptr, err = CHIP_ERROR_INCORRECT_STATE);
        SuccessOrExit(err = writer->Put(TLV::ContextTag(0), status));
        SuccessOrExit(err = commandObj->FinishCommand());
    }
exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "Failed to encode response command.");
    }
    return true;
}

bool emberAfDoorLockClusterGetYeardayScheduleCallback(app::CommandHandler * commandObj,
                                                      const app::ConcreteCommandPath & commandPath,
                                                      const Commands::GetYeardaySchedule::DecodableType & commandData)
{
    auto & scheduleId = commandData.scheduleId;
    auto & userId     = commandData.userId;

    EmberAfPluginDoorLockServerYeardayScheduleEntry * entry = &yeardayScheduleTable[0];
    EmberAfStatus zclStatus;
    CHIP_ERROR err = CHIP_NO_ERROR;
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

    {
        app::CommandPathParams cmdParams = { emberAfCurrentEndpoint(), /* group id */ 0, ::Id,
                                             Commands::GetYeardayScheduleResponse::Id, (app::CommandPathFlags::kEndpointIdValid) };
        TLV::TLVWriter * writer          = nullptr;
        SuccessOrExit(err = commandObj->PrepareCommand(cmdParams));
        VerifyOrExit((writer = commandObj->GetCommandDataIBTLVWriter()) != nullptr, err = CHIP_ERROR_INCORRECT_STATE);
        SuccessOrExit(err = writer->Put(TLV::ContextTag(0), scheduleId));
        SuccessOrExit(err = writer->Put(TLV::ContextTag(1), userId));
        SuccessOrExit(err = writer->Put(TLV::ContextTag(2), zclStatus));
        SuccessOrExit(err = writer->Put(TLV::ContextTag(3), entry->localStartTime));
        SuccessOrExit(err = writer->Put(TLV::ContextTag(4), entry->localEndTime));
        SuccessOrExit(err = commandObj->FinishCommand());
    }
exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "Failed to encode response command.");
    }
    return true;
}

bool emberAfDoorLockClusterClearYeardayScheduleCallback(app::CommandHandler * commandObj,
                                                        const app::ConcreteCommandPath & commandPath,
                                                        const Commands::ClearYeardaySchedule::DecodableType & commandData)
{
    auto & scheduleId = commandData.scheduleId;
    auto & userId     = commandData.userId;

    uint8_t status;
    CHIP_ERROR err = CHIP_NO_ERROR;
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

    {
        app::CommandPathParams cmdParams = { emberAfCurrentEndpoint(), /* group id */ 0, ::Id,
                                             Commands::ClearYeardayScheduleResponse::Id,
                                             (app::CommandPathFlags::kEndpointIdValid) };
        TLV::TLVWriter * writer          = nullptr;
        SuccessOrExit(err = commandObj->PrepareCommand(cmdParams));
        VerifyOrExit((writer = commandObj->GetCommandDataIBTLVWriter()) != nullptr, err = CHIP_ERROR_INCORRECT_STATE);
        SuccessOrExit(err = writer->Put(TLV::ContextTag(0), status));
        SuccessOrExit(err = commandObj->FinishCommand());
    }
exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "Failed to encode response command.");
    }
    return true;
}

bool emberAfDoorLockClusterSetHolidayScheduleCallback(app::CommandHandler * commandObj,
                                                      const app::ConcreteCommandPath & commandPath,
                                                      const Commands::SetHolidaySchedule::DecodableType & commandData)
{
    auto & holidayScheduleId          = commandData.scheduleId;
    auto & localStartTime             = commandData.localStartTime;
    auto & localEndTime               = commandData.localEndTime;
    auto & operatingModeDuringHoliday = commandData.operatingModeDuringHoliday;

    uint8_t status;
    CHIP_ERROR err = CHIP_NO_ERROR;
    if (holidayScheduleId >= EMBER_AF_PLUGIN_DOOR_LOCK_SERVER_HOLIDAY_SCHEDULE_TABLE_SIZE)
    {
        status = 0x01; // failure (per 7.3.2.17.18)
    }
    else
    {
        holidayScheduleTable[holidayScheduleId].localStartTime = localStartTime;
        holidayScheduleTable[holidayScheduleId].localEndTime   = localEndTime;
        // TODO: This cast may not be safe.  https://github.com/project-chip/connectedhomeip/issues/3578
        holidayScheduleTable[holidayScheduleId].operatingModeDuringHoliday =
            static_cast<EmberAfDoorLockOperatingMode>(operatingModeDuringHoliday);
        holidayScheduleTable[holidayScheduleId].inUse = true;
        status                                        = 0x00; // success (per 7.3.2.17.18)
    }

    {
        app::CommandPathParams cmdParams = { emberAfCurrentEndpoint(), /* group id */ 0, ::Id,
                                             Commands::SetHolidayScheduleResponse::Id, (app::CommandPathFlags::kEndpointIdValid) };
        TLV::TLVWriter * writer          = nullptr;
        SuccessOrExit(err = commandObj->PrepareCommand(cmdParams));
        VerifyOrExit((writer = commandObj->GetCommandDataIBTLVWriter()) != nullptr, err = CHIP_ERROR_INCORRECT_STATE);
        SuccessOrExit(err = writer->Put(TLV::ContextTag(0), status));
        SuccessOrExit(err = commandObj->FinishCommand());
    }
exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "Failed to encode response command.");
    }
    return true;
}

bool emberAfDoorLockClusterGetHolidayScheduleCallback(app::CommandHandler * commandObj,
                                                      const app::ConcreteCommandPath & commandPath,
                                                      const Commands::GetHolidaySchedule::DecodableType & commandData)
{
    auto & holidayScheduleId = commandData.scheduleId;

    EmberAfPluginDoorLockServerHolidayScheduleEntry * entry = &holidayScheduleTable[0];
    EmberAfStatus zclStatus;
    CHIP_ERROR err = CHIP_NO_ERROR;
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

    {
        app::CommandPathParams cmdParams = { emberAfCurrentEndpoint(), /* group id */ 0, ::Id,
                                             Commands::GetHolidayScheduleResponse::Id, (app::CommandPathFlags::kEndpointIdValid) };
        TLV::TLVWriter * writer          = nullptr;
        SuccessOrExit(err = commandObj->PrepareCommand(cmdParams));
        VerifyOrExit((writer = commandObj->GetCommandDataIBTLVWriter()) != nullptr, err = CHIP_ERROR_INCORRECT_STATE);
        SuccessOrExit(err = writer->Put(TLV::ContextTag(0), holidayScheduleId));
        SuccessOrExit(err = writer->Put(TLV::ContextTag(1), zclStatus));
        SuccessOrExit(err = writer->Put(TLV::ContextTag(2), entry->localStartTime));
        SuccessOrExit(err = writer->Put(TLV::ContextTag(3), entry->localEndTime));
        SuccessOrExit(err = writer->Put(TLV::ContextTag(4), entry->operatingModeDuringHoliday));
        SuccessOrExit(err = commandObj->FinishCommand());
    }
exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "Failed to encode response command.");
    }
    return true;
}

bool emberAfDoorLockClusterClearHolidayScheduleCallback(app::CommandHandler * commandObj,
                                                        const app::ConcreteCommandPath & commandPath,
                                                        const Commands::ClearHolidaySchedule::DecodableType & commandData)
{
    auto & holidayScheduleId = commandData.scheduleId;

    uint8_t status;
    CHIP_ERROR err = CHIP_NO_ERROR;
    if (holidayScheduleId >= EMBER_AF_PLUGIN_DOOR_LOCK_SERVER_YEARDAY_SCHEDULE_TABLE_SIZE)
    {
        status = 0x01; // failure (per 7.3.2.17.20)
    }
    else
    {
        holidayScheduleTable[holidayScheduleId].inUse = false;
        status                                        = 0x00; // success (per 7.3.2.17.20)
    }

    {
        app::CommandPathParams cmdParams = { emberAfCurrentEndpoint(), /* group id */ 0, ::Id,
                                             Commands::ClearHolidayScheduleResponse::Id,
                                             (app::CommandPathFlags::kEndpointIdValid) };
        TLV::TLVWriter * writer          = nullptr;
        SuccessOrExit(err = commandObj->PrepareCommand(cmdParams));
        VerifyOrExit((writer = commandObj->GetCommandDataIBTLVWriter()) != nullptr, err = CHIP_ERROR_INCORRECT_STATE);
        SuccessOrExit(err = writer->Put(TLV::ContextTag(0), status));
        SuccessOrExit(err = commandObj->FinishCommand());
    }
exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "Failed to encode response command.");
    }
    return true;
}
