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
 * @brief Logging routines for the Door Lock Server
 *plugin.
 *******************************************************************************
 ******************************************************************************/

#include "door-lock-server.h"
#include <app/common/gen/attribute-id.h>
#include <app/common/gen/cluster-id.h>
#include <app/common/gen/command-id.h>
#include <app/util/af.h>
#include <assert.h>

#include "gen/callback.h"

#include <app/Command.h>
#include <support/CodeUtils.h>

using namespace chip;

static EmberAfPluginDoorLockServerLogEntry entries[EMBER_AF_PLUGIN_DOOR_LOCK_SERVER_MAX_LOG_ENTRIES];
static uint8_t nextEntryId = 1;

#define ENTRY_ID_TO_INDEX(entryId) static_cast<uint8_t>((entryId) -1)
#define ENTRY_ID_IS_VALID(entryId) ((entryId) > 0 && (entryId) < nextEntryId)
#define MOST_RECENT_ENTRY_ID() (nextEntryId - 1)
#define LOG_IS_EMPTY() (nextEntryId == 1)

static bool loggingIsEnabled(void)
{
    // This is hardcoded to endpoint 1 because...we need to add endpoint support...
    EndpointId endpoint  = 1;
    bool logging         = false;
    EmberAfStatus status = emberAfReadServerAttribute(endpoint, ZCL_DOOR_LOCK_CLUSTER_ID, ZCL_ENABLE_LOGGING_ATTRIBUTE_ID,
                                                      (uint8_t *) &logging, sizeof(logging));
    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        emberAfDoorLockClusterPrintln("Could not read EnableLogging attribute: 0x%X", status);
    }
    return logging;
}

bool emberAfPluginDoorLockServerAddLogEntry(EmberAfDoorLockEventType eventType, EmberAfDoorLockEventSource source, uint8_t eventId,
                                            uint16_t userId, uint8_t pinLength, uint8_t * pin)
{
    if (!loggingIsEnabled() || ENTRY_ID_TO_INDEX(nextEntryId) >= ArraySize(entries))
    {
        return false;
    }

    EmberAfPluginDoorLockServerLogEntry * nextEntry = &entries[ENTRY_ID_TO_INDEX(nextEntryId)];
    nextEntry->logEntryId                           = nextEntryId;
    nextEntry->timestamp                            = emberAfGetCurrentTimeCallback();
    nextEntry->eventType                            = eventType;
    nextEntry->source                               = source;
    nextEntry->eventId                              = eventId;
    nextEntry->userId                               = userId;
    // Truncate logged PIN if larger than log entry capacity.
    uint8_t moveLength =
        (pinLength > EMBER_AF_PLUGIN_DOOR_LOCK_SERVER_MAX_PIN_LENGTH ? EMBER_AF_PLUGIN_DOOR_LOCK_SERVER_MAX_PIN_LENGTH : pinLength);
    nextEntry->pin[0] = moveLength;
    memmove(nextEntry->pin + 1, pin, moveLength); /*MEMMOVE #2500*/

    nextEntryId++;

    return true;
}

bool emberAfPluginDoorLockServerGetLogEntry(uint16_t * entryId, EmberAfPluginDoorLockServerLogEntry * entry)
{
    if (LOG_IS_EMPTY())
    {
        return false;
    }

    if (!ENTRY_ID_IS_VALID(*entryId))
    {
        *entryId = static_cast<uint16_t>(MOST_RECENT_ENTRY_ID());
    }
    assert(ENTRY_ID_IS_VALID(*entryId));

    *entry = entries[ENTRY_ID_TO_INDEX(*entryId)];

    return true;
}

bool emberAfDoorLockClusterGetLogRecordCallback(chip::app::Command * commandObj, uint16_t entryId)
{
    EmberStatus status;
    EmberAfPluginDoorLockServerLogEntry entry;
    CHIP_ERROR err = CHIP_NO_ERROR;
    if (LOG_IS_EMPTY() || !emberAfPluginDoorLockServerGetLogEntry(&entryId, &entry))
    {
        status = emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_INVALID_VALUE);
        if (status != EMBER_ZCL_STATUS_SUCCESS)
        {
            emberAfDoorLockClusterPrintln("Failed to send default response: 0x%X", status);
        }
    }
    else
    {
        {
            app::CommandPathParams cmdParams = { emberAfCurrentEndpoint(), /* group id */ 0, ZCL_DOOR_LOCK_CLUSTER_ID,
                                                 ZCL_GET_LOG_RECORD_RESPONSE_COMMAND_ID,
                                                 (chip::app::CommandPathFlags::kEndpointIdValid) };
            TLV::TLVWriter * writer          = nullptr;
            SuccessOrExit(err = commandObj->PrepareCommand(cmdParams));
            VerifyOrExit((writer = commandObj->GetCommandDataElementTLVWriter()) != nullptr, err = CHIP_ERROR_INCORRECT_STATE);
            SuccessOrExit(err = writer->Put(TLV::ContextTag(0), entry.logEntryId));
            SuccessOrExit(err = writer->Put(TLV::ContextTag(1), entry.timestamp));
            SuccessOrExit(err = writer->Put(TLV::ContextTag(2), entry.eventType));
            SuccessOrExit(err = writer->Put(TLV::ContextTag(3), entry.source));
            SuccessOrExit(err = writer->Put(TLV::ContextTag(4), entry.eventId));
            SuccessOrExit(err = writer->Put(TLV::ContextTag(5), entry.userId));
            SuccessOrExit(err = writer->PutBytes(TLV::ContextTag(6), entry.pin + 1, entry.pin[0]));
            SuccessOrExit(err = commandObj->FinishCommand());
        }
    }
exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "Failed to encode response command.");
    }
    return true;
}
