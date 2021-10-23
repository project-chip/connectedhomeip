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
 * @brief APIs and defines for the Door Lock Server
 *plugin.
 *******************************************************************************
 ******************************************************************************/

#pragma once

// ------------------------------------------------------------------------------
// Core

#include <app-common/zap-generated/af-structs.h>
#include <app/util/af.h>

#ifndef DOXYGEN_SHOULD_SKIP_THIS
typedef struct
{
    chip::AttributeId id;
    uint16_t value;
} EmAfPluginDoorLockServerAttributeData;

// For each of the provided attribute ID/value pairs, write the attribute to the
// value on the DOOR_LOCK_SERVER_ENDPOINT. If this function encounters a failure,
// it will print something out. The description parameter is a string that
// describes the type of attributes that are being written.
void emAfPluginDoorLockServerWriteAttributes(const EmAfPluginDoorLockServerAttributeData * data, uint8_t dataLength,
                                             const char * description);

// This function should be called when the door state has changed. A status
// describing the success or failure of the update will be returned.
EmberAfStatus emAfPluginDoorLockServerNoteDoorStateChanged(EmberAfDoorState state);
#endif

// At boot, the NumberOfPINUsersSupported attribute will be written to this
// value.
// Note: the DOOR_LOCK_USER_TABLE_SIZE symbol is respected because it
// was used originally as a configuration value for this plugin.
#ifdef DOOR_LOCK_USER_TABLE_SIZE
#define EMBER_AF_PLUGIN_DOOR_LOCK_SERVER_PIN_USER_TABLE_SIZE DOOR_LOCK_USER_TABLE_SIZE
#else
#define EMBER_AF_PLUGIN_DOOR_LOCK_SERVER_PIN_USER_TABLE_SIZE 8
#endif

// At boot, the NumberOfRFIDUsersSupported attribute will be written to this
// value.
#define EMBER_AF_PLUGIN_DOOR_LOCK_SERVER_RFID_USER_TABLE_SIZE 8

// This value should reflect the value of the MaxPINCodeLength attribute.
// Note: the DOOR_LOCK_MAX_PIN_LENGTH symbol is respected because it was used
// originally as a configuration value for this plugin.
#ifdef DOOR_LOCK_MAX_PIN_LENGTH
#define EMBER_AF_PLUGIN_DOOR_LOCK_SERVER_MAX_PIN_LENGTH DOOR_LOCK_MAX_PIN_LENGTH
#else
#define EMBER_AF_PLUGIN_DOOR_LOCK_SERVER_MAX_PIN_LENGTH 8
#endif

// This value should reflect the value of the MaxRFIDCodeLength attribute.
#define EMBER_AF_PLUGIN_DOOR_LOCK_SERVER_MAX_RFID_LENGTH 8

// At boot, the NumberOfWeekDaySchedulesSupportedPerUser attribute will be writen
// to this value.
// Note: the DOOR_LOCK_SCHEDULE_TABLE_SIZE symbol is respected because it was
// used originally as a configuration value for this plugin.
// Also note: technically, this is the _total_ number of weekday schedules that
// can be stored across all users.
#ifdef DOOR_LOCK_SCHEDULE_TABLE_SIZE
#define EMBER_AF_PLUGIN_DOOR_LOCK_SERVER_WEEKDAY_SCHEDULE_TABLE_SIZE DOOR_LOCK_SCHEDULE_TABLE_SIZE
#else
#define EMBER_AF_PLUGIN_DOOR_LOCK_SERVER_WEEKDAY_SCHEDULE_TABLE_SIZE 4
#endif

// At boot, the NumberOfYearDaySchedulesSupportedPerUser attribute will be
// written to this value.
// Note: technically, this is the _total_ number of yearday schedules that can be
// stored across all users.
#define EMBER_AF_PLUGIN_DOOR_LOCK_SERVER_YEARDAY_SCHEDULE_TABLE_SIZE 8

// At boot, the NumberOfHolidaySchedulesSupported attribute will be written to
// this value.
#define EMBER_AF_PLUGIN_DOOR_LOCK_SERVER_HOLIDAY_SCHEDULE_TABLE_SIZE 8

#ifndef DOOR_LOCK_SERVER_ENDPOINT
#define DOOR_LOCK_SERVER_ENDPOINT 1
#endif

// ------------------------------------------------------------------------------
// Logging

// This value should reflect the value of the NumberOfLogRecordsSupported
// attribute.
#define EMBER_AF_PLUGIN_DOOR_LOCK_SERVER_MAX_LOG_ENTRIES 16

typedef struct
{
    uint16_t logEntryId;
    uint32_t timestamp;
    EmberAfDoorLockEventType eventType;
    EmberAfDoorLockEventSource source;

    // This field is either a EmberAfDoorLockOperationEventCode or a
    // EmberAfDoorLockProgrammingEventCode.
    uint8_t eventId;

    uint16_t userId;

    // This field is a ZCL string representing the PIN code (i.e., the first byte
    // of the buffer is the length of the total buffer).
    uint8_t pin[EMBER_AF_PLUGIN_DOOR_LOCK_SERVER_MAX_PIN_LENGTH + 1];
} EmberAfPluginDoorLockServerLogEntry;

// Add a log entry. Returns true iff the entry was added. Note that the eventId
// parameter should be of type EmberAfDoorLockOperationEventCode or
// EmberAfDoorLockProgrammingEventCode.
bool emberAfPluginDoorLockServerAddLogEntry(EmberAfDoorLockEventType eventType, EmberAfDoorLockEventSource source, uint8_t eventId,
                                            uint16_t userId, uint8_t pinLength, uint8_t * pin);

// Get a log entry associated with the entry ID. If the entry ID does not exist,
// the most recent entry is returned and the entryId parameter is updated. The
// entryId is a 1-based index into an array of log entries in order to match
// GetLogRecord ZCL command. This will return true iff the entry was successfully
// returned.
bool emberAfPluginDoorLockServerGetLogEntry(uint16_t * entryId, EmberAfPluginDoorLockServerLogEntry * entry);

// ------------------------------------------------------------------------------
// Users

#ifndef DOXYGEN_SHOULD_SKIP_THIS
// Initialize the Door Lock users.
void emAfPluginDoorLockServerInitUser(void);

// Set the user type associated with the provided user ID (userId) and return
// true iff successful.
bool emAfPluginDoorLockServerSetPinUserType(uint16_t userId, EmberAfDoorLockUserType type);
#endif

typedef struct
{
    EmberAfDoorLockUserStatus status;
    EmberAfDoorLockUserType type;

    // This field is a Zigbee string, so the first byte is the length of the
    // remaining bytes.
    union
    {
        uint8_t pin[EMBER_AF_PLUGIN_DOOR_LOCK_SERVER_MAX_PIN_LENGTH + 1];
        uint8_t rfid[EMBER_AF_PLUGIN_DOOR_LOCK_SERVER_MAX_RFID_LENGTH + 1];
    } code;
} EmberAfPluginDoorLockServerUser;

// These functions will attempt to unlock the door with a PIN/RFID.
EmberAfStatus emberAfPluginDoorLockServerApplyPin(uint8_t * pin, uint8_t pinLength);
EmberAfStatus emberAfPluginDoorLockServerApplyRfid(uint8_t * rfid, uint8_t rfidLength);

// ------------------------------------------------------------------------------
// Schedule

#ifndef DOXYGEN_SHOULD_SKIP_THIS
// Initialize the Door Lock schedules.
void emAfPluginDoorLockServerInitSchedule(void);
#endif

typedef struct
{
    uint16_t userId;
    uint8_t daysMask;
    uint8_t startHour;
    uint8_t startMinute;
    uint8_t stopHour;
    uint8_t stopMinute;
    bool inUse;
} EmberAfPluginDoorLockServerWeekdayScheduleEntry;

typedef struct
{
    uint16_t userId;
    uint32_t localStartTime;
    uint32_t localEndTime;
    bool inUse;
} EmberAfPluginDoorLockServerYeardayScheduleEntry;

typedef struct
{
    uint32_t localStartTime;
    uint32_t localEndTime;
    EmberAfDoorLockOperatingMode operatingModeDuringHoliday;
    bool inUse;
} EmberAfPluginDoorLockServerHolidayScheduleEntry;

// ------------------------------------------------------------------------------
// Utilities

#ifndef DOXYGEN_SHOULD_SKIP_THIS
// This function will see if the space required (spaceReq) is greater than the
// space available (spaceAvail) and if so it will send a DefaultResponse
// command with the status of EMBER_ZCL_STATUS_INSUFFICIENT_SPACE and return
// false. Otherwise, it will return true.
bool emAfPluginDoorLockServerCheckForSufficientSpace(uint16_t spaceReq, uint8_t spaceAvail);
#endif

// Critical Message Queue
// If the Critical Message Queue Plugin is available, use it for event notifications
#ifdef EMBER_AF_PLUGIN_CRITICAL_MESSAGE_QUEUE
#include "../critical-message-queue/critical-message-queue.h"
#define SEND_COMMAND_UNICAST_TO_BINDINGS()                                                                                         \
    emberAfSendCommandUnicastToBindingsWithCallback(emberAfPluginCriticalMessageQueueEnqueueCallback)
#else
#define SEND_COMMAND_UNICAST_TO_BINDINGS() emberAfSendCommandUnicastToBindings()
#endif

// ------------------------------------------------------------------------------
// Legacy

// The following preprocessor logic serves as a conversion layer from the old
// names for the symbols used by this plugin.

#define EmberAfDoorLockScheduleEntry EmberAfPluginDoorLockServerWeekdayScheduleEntry
#define EmberAfDoorLockUser EmberAfPluginDoorLockServerUser

/** @brief Activate Door Lock Callback
 * This function is provided by the door lock server plugin.
 *
 * @param activate True if the lock should move to the locked position,
 *  false if it should move to the unlocked position Ver.: always
 *
 * @returns true if the callback was able to activate/deactivate the Lock.
 */
bool emberAfPluginDoorLockServerActivateDoorLockCallback(bool activate);
