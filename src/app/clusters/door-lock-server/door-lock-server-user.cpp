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

#include "app/util/common.h"
#include "door-lock-server.h"
#include <app-common/zap-generated/attribute-id.h>
#include <app-common/zap-generated/attribute-type.h>
#include <app-common/zap-generated/cluster-id.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <app-common/zap-generated/command-id.h>
#include <app/util/af-event.h>
#include <app/util/af.h>
#include <app/util/time-util.h>

#include <app/CommandHandler.h>
#include <app/ConcreteAttributePath.h>
#include <app/ConcreteCommandPath.h>
#include <lib/support/CodeUtils.h>

using namespace chip;
using namespace chip::app::Clusters::DoorLock;

EmberEventControl emberAfPluginDoorLockServerLockoutEventControl;
EmberEventControl emberAfPluginDoorLockServerRelockEventControl;

// The index into these tables is a userId.
static EmberAfPluginDoorLockServerUser pinUserTable[EMBER_AF_PLUGIN_DOOR_LOCK_SERVER_PIN_USER_TABLE_SIZE];
static EmberAfPluginDoorLockServerUser rfidUserTable[EMBER_AF_PLUGIN_DOOR_LOCK_SERVER_PIN_USER_TABLE_SIZE];

// This is the current number of invalid PIN/RFID's in a row.
static uint8_t wrongCodeEntryCount = 0;

bool emAfPluginDoorLockServerCheckForSufficientSpace(uint16_t spaceReq, uint8_t spaceAvail)
{
    if (spaceReq > spaceAvail)
    {
        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_INSUFFICIENT_SPACE);
        return false;
    }
    return true;
}

// ------------------------------------------------------------------------------
// Initialization

static void enableSendPinOverTheAir(void)
{
#ifdef ZCL_USING_DOOR_LOCK_CLUSTER_SEND_PIN_OVER_THE_AIR_ATTRIBUTE
    bool troo = true;
    EmberAfStatus status =
        emberAfWriteServerAttribute(DOOR_LOCK_SERVER_ENDPOINT, ZCL_DOOR_LOCK_CLUSTER_ID, ZCL_SEND_PIN_OVER_THE_AIR_ATTRIBUTE_ID,
                                    (uint8_t *) &troo, ZCL_BOOLEAN_ATTRIBUTE_TYPE);
    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        emberAfDoorLockClusterPrintln("Failed to write SendPinOverTheAir attribute: 0x%X", status);
    }
#endif
}

void emAfPluginDoorLockServerInitUser(void)
{
#if defined(ZCL_USING_DOOR_LOCK_CLUSTER_NUM_TOTAL_USERS_SUPPORTED_ATTRIBUTE) ||                                                    \
    defined(ZCL_USING_DOOR_LOCK_CLUSTER_NUM_PIN_USERS_SUPPORTED_ATTRIBUTE) ||                                                      \
    defined(ZCL_USING_DOOR_LOCK_CLUSTER_NUM_RFID_USERS_SUPPORTED_ATTRIBUTE)
    const EmAfPluginDoorLockServerAttributeData data[] = {
#ifdef ZCL_USING_DOOR_LOCK_CLUSTER_NUM_TOTAL_USERS_SUPPORTED_ATTRIBUTE
        // This attribute is...confusing. Here is the description of it from
        // 7.3.2.12.2.
        //   "Number of total users supported by the lock. This value is equal to the
        //    higher one of [# of PIN Users Supported] and [# of RFID Users
        //    Supported]"
        { ZCL_NUM_TOTAL_USERS_SUPPORTED_ATTRIBUTE_ID,
          ((EMBER_AF_PLUGIN_DOOR_LOCK_SERVER_PIN_USER_TABLE_SIZE > EMBER_AF_PLUGIN_DOOR_LOCK_SERVER_RFID_USER_TABLE_SIZE)
               ? EMBER_AF_PLUGIN_DOOR_LOCK_SERVER_PIN_USER_TABLE_SIZE
               : EMBER_AF_PLUGIN_DOOR_LOCK_SERVER_RFID_USER_TABLE_SIZE) },
#endif

#ifdef ZCL_USING_DOOR_LOCK_CLUSTER_NUM_PIN_USERS_SUPPORTED_ATTRIBUTE
        { ZCL_NUM_PIN_USERS_SUPPORTED_ATTRIBUTE_ID, EMBER_AF_PLUGIN_DOOR_LOCK_SERVER_PIN_USER_TABLE_SIZE },
#endif

#ifdef ZCL_USING_DOOR_LOCK_CLUSTER_NUM_RFID_USERS_SUPPORTED_ATTRIBUTE
        { ZCL_NUM_RFID_USERS_SUPPORTED_ATTRIBUTE_ID, EMBER_AF_PLUGIN_DOOR_LOCK_SERVER_RFID_USER_TABLE_SIZE },
#endif
    };
    emAfPluginDoorLockServerWriteAttributes(data, ArraySize(data), "user table");
#endif

    enableSendPinOverTheAir();
}

// ------------------------------------------------------------------------------
// User management

// "pin" parameter is Zigbee string, so first byte is length.
static void printPin(uint8_t * pin)
{
    uint8_t pinLength = emberAfStringLength(pin);
    emberAfDoorLockClusterPrint("(%x)", pinLength);
    for (uint8_t i = 0; i < pinLength; i++)
    {
        emberAfDoorLockClusterPrint(" %c", pin[i + 1]);
    }
}

static void printUserTables(void)
{
    uint8_t i;
    emberAfDoorLockClusterPrintln("id   st ty PIN");
    emberAfDoorLockClusterPrintln("PIN:");
    for (i = 0; i < EMBER_AF_PLUGIN_DOOR_LOCK_SERVER_PIN_USER_TABLE_SIZE; i++)
    {
        EmberAfPluginDoorLockServerUser * user = &pinUserTable[i];
        emberAfDoorLockClusterPrint("%2x %x %x ", i, user->status, user->type);
        printPin(user->code.pin);
        emberAfDoorLockClusterPrintln("%s", "");
    }
    emberAfDoorLockClusterPrintln("RFID:");
    for (i = 0; i < EMBER_AF_PLUGIN_DOOR_LOCK_SERVER_RFID_USER_TABLE_SIZE; i++)
    {
        EmberAfPluginDoorLockServerUser * user = &rfidUserTable[i];
        emberAfDoorLockClusterPrint("%2x %x %x ", i, user->status, user->type);
        printPin(user->code.rfid);
        emberAfDoorLockClusterPrintln("%s", "");
    }
}

// Returns status byte for use in SetPinResponse and SetRfidResponse commands.
static EmberAfStatus setUser(uint16_t userId, DlUserStatus userStatus, DlUserType userType, const ByteSpan & code,
                             EmberAfPluginDoorLockServerUser * userTable, uint8_t userTableSize)
{
    bool success = false;
    // "code" (i.e. PIN/RFID) is stored in table entry in ZCL format (1-byte
    // length prefix). Don't allow a code with length that exceeds capacity
    // of the table entry field. Note there are potentially different max
    // lengths for PIN v. RFID.
    bool validCodeLength = false;
    if (!code.empty() &&
        ((userTable == pinUserTable && code.size() <= EMBER_AF_PLUGIN_DOOR_LOCK_SERVER_MAX_PIN_LENGTH) ||
         (code.size() <= EMBER_AF_PLUGIN_DOOR_LOCK_SERVER_MAX_RFID_LENGTH)))
    {
        validCodeLength = true;
    }

    if (validCodeLength && userId < userTableSize)
    {
        EmberAfPluginDoorLockServerUser * user = &userTable[userId];
        // TODO: Need to check validity.  https://github.com/project-chip/connectedhomeip/issues/3579
        user->status = static_cast<EmberAfDoorLockUserStatus>(userStatus);
        // TODO: Need to check validity.  https://github.com/project-chip/connectedhomeip/issues/3580
        user->type = static_cast<EmberAfDoorLockUserType>(userType);
        // Our length checks above make this cast safe.
        user->code.rfid[0] = static_cast<uint8_t>(code.size());
        memmove(user->code.rfid + 1, code.data(), code.size());

        emberAfDoorLockClusterPrintln("***RX SET %s ***", (userTable == pinUserTable ? "PIN" : "RFID"));
        printUserTables();

        success = true;
    }
    return (success ? EMBER_ZCL_STATUS_SUCCESS : EMBER_ZCL_STATUS_FAILURE); // See 7.3.2.17.6 and 7.3.2.17.23).
}

// Returns true for success, false for failure.
static bool getUser(uint16_t userId, EmberAfPluginDoorLockServerUser * userTable, uint8_t userTableSize,
                    EmberAfPluginDoorLockServerUser * returnedUser)
{
    bool success = false;
    if (userId < userTableSize)
    {
        *returnedUser = userTable[userId];
        success       = true;
    }
    return success;
}

// Returns status byte for use in ClearPin and ClearRfid response commands.
static EmberAfStatus clearUserPinOrRfid(uint16_t userId, EmberAfPluginDoorLockServerUser * userTable, uint8_t userTableSize)
{
    bool success = false;
    if (userId < userTableSize)
    {
        // Since the rfid member of the struct is a Zigbee string, setting the first
        // byte to 0 will indicate that we have a 0-length pin.
        memset((userTable == pinUserTable ? userTable[userId].code.pin : userTable[userId].code.rfid), 0x00,
               sizeof(userTable[userId].code));
        success = true;
    }
    return (success ? EMBER_ZCL_STATUS_SUCCESS : EMBER_ZCL_STATUS_FAILURE); // See 7.3.2.17.8 and 7.3.2.17.25).
}

bool emberAfDoorLockClusterGetUserTypeCallback(app::CommandHandler * commandObj, const app::ConcreteCommandPath & commandPath,
                                               const Commands::GetUserType::DecodableType & commandData)
{
    auto & userId = commandData.userId;

    CHIP_ERROR err = CHIP_NO_ERROR;
    if (emAfPluginDoorLockServerCheckForSufficientSpace(userId, EMBER_AF_PLUGIN_DOOR_LOCK_SERVER_PIN_USER_TABLE_SIZE))
    {
        EmberAfPluginDoorLockServerUser * user = &pinUserTable[userId];

        {
            app::ConcreteCommandPath path = { emberAfCurrentEndpoint(), ZCL_DOOR_LOCK_CLUSTER_ID,
                                              ZCL_GET_USER_TYPE_RESPONSE_COMMAND_ID };
            TLV::TLVWriter * writer       = nullptr;
            SuccessOrExit(err = commandObj->PrepareCommand(path));
            VerifyOrExit((writer = commandObj->GetCommandDataIBTLVWriter()) != nullptr, err = CHIP_ERROR_INCORRECT_STATE);
            SuccessOrExit(err = writer->Put(TLV::ContextTag(0), userId));
            SuccessOrExit(err = writer->Put(TLV::ContextTag(1), user->type));
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

bool emberAfDoorLockClusterSetUserTypeCallback(app::CommandHandler * commandObj, const app::ConcreteCommandPath & commandPath,
                                               const Commands::SetUserType::DecodableType & commandData)
{
    auto & userId   = commandData.userId;
    auto & userType = commandData.userType;

    // TODO: Need to validate userType.  https://github.com/project-chip/connectedhomeip/issues/3580
    EmberAfStatus status = (emAfPluginDoorLockServerSetPinUserType(userId, static_cast<EmberAfDoorLockUserType>(userType))
                                ? EMBER_ZCL_STATUS_SUCCESS   // success (per 7.3.2.17.21)
                                : EMBER_ZCL_STATUS_FAILURE); // failure (per 7.3.2.17.21)

    if (EMBER_ZCL_STATUS_SUCCESS != emberAfSendImmediateDefaultResponse(status))
    {
        ChipLogError(Zcl, "Failed to encode response command.");
    }
    return true;
}

bool emAfPluginDoorLockServerSetPinUserType(uint16_t userId, EmberAfDoorLockUserType type)
{
    if (userId >= EMBER_AF_PLUGIN_DOOR_LOCK_SERVER_PIN_USER_TABLE_SIZE)
    {
        return false;
    }
    else
    {
        pinUserTable[userId].type = type;
        return true;
    }
}

// ------------------------------------------------------------------------------
// PIN handling

bool emberAfDoorLockClusterSetPINCodeCallback(app::CommandHandler * commandObj, const app::ConcreteCommandPath & commandPath,
                                              const Commands::SetPINCode::DecodableType & commandData)
{
    auto & userId     = commandData.userId;
    auto & userStatus = commandData.userStatus;
    auto & userType   = commandData.userType;
    auto & pin        = commandData.pin;

    CHIP_ERROR err = CHIP_NO_ERROR;
    // send response
    EmberAfStatus status =
        setUser(userId, userStatus, userType, pin, pinUserTable, EMBER_AF_PLUGIN_DOOR_LOCK_SERVER_PIN_USER_TABLE_SIZE);
    uint16_t rfProgrammingEventMask = 0xffff; // send event by default

    VerifyOrExit(EMBER_ZCL_STATUS_SUCCESS == emberAfSendImmediateDefaultResponse(status), err = CHIP_ERROR_INCORRECT_STATE);

    // get bitmask so we can check if we should send event notification
    emberAfReadServerAttribute(DOOR_LOCK_SERVER_ENDPOINT, ::Id, Attributes::RemoteProgrammingEventMask::Id,
                               (uint8_t *) &rfProgrammingEventMask, sizeof(rfProgrammingEventMask));
    if ((rfProgrammingEventMask & EMBER_BIT(2)) && !status && !pin.empty())
    {
        emberAfFillExternalBuffer((ZCL_CLUSTER_SPECIFIC_COMMAND | ZCL_FRAME_CONTROL_SERVER_TO_CLIENT), ZCL_DOOR_LOCK_CLUSTER_ID,
                                  ZCL_PROGRAMMING_EVENT_NOTIFICATION_COMMAND_ID, "uuvSuuwS", EMBER_ZCL_DOOR_LOCK_EVENT_SOURCE_RF,
                                  EMBER_ZCL_DOOR_LOCK_PROGRAMMING_EVENT_CODE_PIN_ADDED, userId, pin.data(),
                                  static_cast<uint8_t>(pin.size()), userType, userStatus, 0 /*emberAfGetCurrentTime() #2507*/,
                                  pin.data(), static_cast<uint8_t>(pin.size()));
        SEND_COMMAND_UNICAST_TO_BINDINGS();
    }
exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "Failed to encode response command.");
    }
    return true;
}

static bool getSendPinOverTheAir(void)
{
    bool sendPinOverTheAir = true;
#ifdef ZCL_USING_DOOR_LOCK_CLUSTER_SEND_PIN_OVER_THE_AIR_ATTRIBUTE
    EmberAfStatus status =
        emberAfReadServerAttribute(DOOR_LOCK_SERVER_ENDPOINT, ZCL_DOOR_LOCK_CLUSTER_ID, ZCL_SEND_PIN_OVER_THE_AIR_ATTRIBUTE_ID,
                                   (uint8_t *) &sendPinOverTheAir, sizeof(sendPinOverTheAir));
    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        emberAfDoorLockClusterPrintln("Failed to read SendPinOverTheAir attribute: 0x%X", status);
    }
#endif
    return sendPinOverTheAir;
}

bool emberAfDoorLockClusterGetPINCodeCallback(app::CommandHandler * commandObj, const app::ConcreteCommandPath & commandPath,
                                              const Commands::GetPINCode::DecodableType & commandData)
{
    auto & userId = commandData.userId;

    EmberAfPluginDoorLockServerUser user;
    EmberStatus status = EMBER_SUCCESS;
    CHIP_ERROR err     = CHIP_NO_ERROR;
    if (getUser(userId, pinUserTable, EMBER_AF_PLUGIN_DOOR_LOCK_SERVER_PIN_USER_TABLE_SIZE, &user))
    {
        {
            app::ConcreteCommandPath path = { emberAfCurrentEndpoint(), ::Id, Commands::GetPINCodeResponse::Id };
            TLV::TLVWriter * writer       = nullptr;
            SuccessOrExit(err = commandObj->PrepareCommand(path));
            VerifyOrExit((writer = commandObj->GetCommandDataIBTLVWriter()) != nullptr, err = CHIP_ERROR_INCORRECT_STATE);
            SuccessOrExit(err = writer->Put(TLV::ContextTag(0), userId));
            SuccessOrExit(err = writer->Put(TLV::ContextTag(1), user.status));
            SuccessOrExit(err = writer->Put(TLV::ContextTag(2), user.type));
            if (getSendPinOverTheAir())
            {
                SuccessOrExit(err = writer->Put(TLV::ContextTag(3), ByteSpan::fromZclString(user.code.pin)));
            }
            else
            {
                SuccessOrExit(err = writer->PutBytes(TLV::ContextTag(3), nullptr, 0));
            }
            SuccessOrExit(err = commandObj->FinishCommand());
        }
    }
    else
    {
        status = emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_INVALID_VALUE);
    }

    if (status != EMBER_SUCCESS)
    {
        emberAfDoorLockClusterPrintln("Failed to send response to GetPin: 0x%X", status);
    }
exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "Failed to encode response command.");
    }
    return true;
}

bool emberAfDoorLockClusterClearPINCodeCallback(app::CommandHandler * commandObj, const app::ConcreteCommandPath & commandPath,
                                                const Commands::ClearPINCode::DecodableType & commandData)
{
    auto & userId = commandData.pinSlotIndex;

    CHIP_ERROR err       = CHIP_NO_ERROR;
    EmberAfStatus status = clearUserPinOrRfid(userId, pinUserTable, EMBER_AF_PLUGIN_DOOR_LOCK_SERVER_PIN_USER_TABLE_SIZE);
    // get bitmask so we can check if we should send event notification
    uint16_t rfProgrammingEventMask = 0xffff; // event sent by default
    uint8_t userPin                 = 0x00;   // Zero length Zigbee string

    VerifyOrExit(EMBER_ZCL_STATUS_SUCCESS != emberAfSendImmediateDefaultResponse(status), err = CHIP_ERROR_INCORRECT_STATE);

    emberAfReadServerAttribute(DOOR_LOCK_SERVER_ENDPOINT, ::Id, Attributes::RemoteProgrammingEventMask::Id,
                               (uint8_t *) &rfProgrammingEventMask, sizeof(rfProgrammingEventMask));
    if ((rfProgrammingEventMask & EMBER_BIT(2)) && !status)
    {
        emberAfFillExternalBuffer((ZCL_CLUSTER_SPECIFIC_COMMAND | ZCL_FRAME_CONTROL_SERVER_TO_CLIENT), ZCL_DOOR_LOCK_CLUSTER_ID,
                                  ZCL_PROGRAMMING_EVENT_NOTIFICATION_COMMAND_ID, "uuvsuuws", 0x01, 0x03, userId, &userPin, 0x00,
                                  0x00, 0x00, &userPin);
        SEND_COMMAND_UNICAST_TO_BINDINGS();
    }
    else if ((rfProgrammingEventMask & EMBER_BIT(0)) && status)
    {
        emberAfFillExternalBuffer((ZCL_CLUSTER_SPECIFIC_COMMAND | ZCL_FRAME_CONTROL_SERVER_TO_CLIENT), ZCL_DOOR_LOCK_CLUSTER_ID,
                                  ZCL_PROGRAMMING_EVENT_NOTIFICATION_COMMAND_ID, "uuvsuuws", 0x01, 0x00, userId, &userPin, 0x00,
                                  0x00, 0x00, &userPin);
        SEND_COMMAND_UNICAST_TO_BINDINGS();
    }
exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "Failed to encode response command.");
    }
    return true;
}

bool emberAfDoorLockClusterClearAllPINCodesCallback(app::CommandHandler * commandObj, const app::ConcreteCommandPath & commandPath,
                                                    const Commands::ClearAllPINCodes::DecodableType & commandData)
{
    uint8_t i;
    for (i = 0; i < EMBER_AF_PLUGIN_DOOR_LOCK_SERVER_PIN_USER_TABLE_SIZE; i++)
    {
        clearUserPinOrRfid(i, pinUserTable, EMBER_AF_PLUGIN_DOOR_LOCK_SERVER_PIN_USER_TABLE_SIZE);
    }

    // 7.3.2.17.9 says that "0x00" indicates success.
    if (EMBER_ZCL_STATUS_SUCCESS != emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_SUCCESS))
    {
        ChipLogError(Zcl, "Failed to encode response command.");
    }
    return true;
}

// ------------------------------------------------------------------------------
// RFID handling

bool emberAfDoorLockClusterSetRFIDCodeCallback(app::CommandHandler * commandObj, const app::ConcreteCommandPath & commandPath,
                                               const Commands::SetRFIDCode::DecodableType & commandData)
{
    auto & userId     = commandData.userId;
    auto & userStatus = commandData.userStatus;
    auto & userType   = commandData.userType;
    auto & rfid       = commandData.rfidCode;

    EmberAfStatus status =
        setUser(userId, userStatus, userType, rfid, rfidUserTable, EMBER_AF_PLUGIN_DOOR_LOCK_SERVER_RFID_USER_TABLE_SIZE);

    if (EMBER_ZCL_STATUS_SUCCESS != emberAfSendImmediateDefaultResponse(status))
    {
        ChipLogError(Zcl, "Failed to encode response command.");
    }
    return true;
}

bool emberAfDoorLockClusterGetRFIDCodeCallback(app::CommandHandler * commandObj, const app::ConcreteCommandPath & commandPath,
                                               const Commands::GetRFIDCode::DecodableType & commandData)
{
    auto & userId = commandData.userId;

    EmberAfPluginDoorLockServerUser user;
    EmberStatus status = EMBER_SUCCESS;
    CHIP_ERROR err     = CHIP_NO_ERROR;
    if (getUser(userId, rfidUserTable, EMBER_AF_PLUGIN_DOOR_LOCK_SERVER_RFID_USER_TABLE_SIZE, &user))
    {
        {
            app::ConcreteCommandPath path = { emberAfCurrentEndpoint(), ZCL_DOOR_LOCK_CLUSTER_ID,
                                              Commands::GetRFIDCodeResponse::Id };
            TLV::TLVWriter * writer       = nullptr;
            SuccessOrExit(err = commandObj->PrepareCommand(path));
            VerifyOrExit((writer = commandObj->GetCommandDataIBTLVWriter()) != nullptr, err = CHIP_ERROR_INCORRECT_STATE);
            SuccessOrExit(err = writer->Put(TLV::ContextTag(0), userId));
            SuccessOrExit(err = writer->Put(TLV::ContextTag(1), user.status));
            SuccessOrExit(err = writer->Put(TLV::ContextTag(2), user.type));
            SuccessOrExit(err = writer->Put(TLV::ContextTag(3), ByteSpan::fromZclString(user.code.pin)));
            SuccessOrExit(err = commandObj->FinishCommand());
        }
    }
    else
    {
        status = emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_INVALID_VALUE);
    }

    if (status != EMBER_SUCCESS)
    {
        emberAfDoorLockClusterPrintln("Failed to send response to GetRfid: 0x%X", status);
    }
exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "Failed to encode response command.");
    }
    return true;
}

bool emberAfDoorLockClusterClearRFIDCodeCallback(app::CommandHandler * commandObj, const app::ConcreteCommandPath & commandPath,
                                                 const Commands::ClearRFIDCode::DecodableType & commandData)
{
    auto & userId = commandData.rfidSlotIndex;

    EmberAfStatus status = clearUserPinOrRfid(userId, rfidUserTable, EMBER_AF_PLUGIN_DOOR_LOCK_SERVER_RFID_USER_TABLE_SIZE);

    // 7.3.2.17.9 says that "0x00" indicates success.
    if (EMBER_ZCL_STATUS_SUCCESS != emberAfSendImmediateDefaultResponse(status))
    {
        ChipLogError(Zcl, "Failed to encode response command.");
    }
    return true;
}

bool emberAfDoorLockClusterClearAllRFIDCodesCallback(app::CommandHandler * commandObj, const app::ConcreteCommandPath & commandPath,
                                                     const Commands::ClearAllRFIDCodes::DecodableType & commandData)
{
    for (uint8_t i = 0; i < EMBER_AF_PLUGIN_DOOR_LOCK_SERVER_RFID_USER_TABLE_SIZE; i++)
    {
        clearUserPinOrRfid(i, rfidUserTable, EMBER_AF_PLUGIN_DOOR_LOCK_SERVER_RFID_USER_TABLE_SIZE);
    }

    // 7.3.2.17.26 says that "0x00" indicates success.
    if (EMBER_ZCL_STATUS_SUCCESS != emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_SUCCESS))
    {
        ChipLogError(Zcl, "Failed to encode response command.");
    }
    return true;
}

// ------------------------------------------------------------------------------
// Lock state management

static void printSuccessOrFailure(bool success)
{
    if (success)
    {
        emberAfDoorLockClusterPrintln("SUCCESS!");
    }
    else
    {
        emberAfDoorLockClusterPrintln("FAILURE!");
    }
}

/**
 * This function checks to see if a pin is required and, if it is
 * and a pin is provided, it validates the pin against those known in
 * the user table.
 *
 * Note that the "pin" parameter is a Zigbee string, so the first byte is the
 * length of the remaining bytes
 */
static bool verifyPin(const ByteSpan & pin, uint8_t * userId)
{
    bool pinRequired = false;
    EmberAfStatus status;
    uint8_t i;

    status =
        emberAfReadServerAttribute(DOOR_LOCK_SERVER_ENDPOINT, ZCL_DOOR_LOCK_CLUSTER_ID,
                                   Attributes::RequirePINforRemoteOperation::Id, (uint8_t *) &pinRequired, sizeof(pinRequired));
    if (EMBER_ZCL_STATUS_SUCCESS != status || !pinRequired)
    {
        return true;
    }
    else if (pin.empty())
    {
        return false;
    }

    for (i = 0; i < EMBER_AF_PLUGIN_DOOR_LOCK_SERVER_PIN_USER_TABLE_SIZE; i++)
    {
        EmberAfPluginDoorLockServerUser * user = &pinUserTable[i];
        if (pin.data_equal(ByteSpan::fromZclString(user->code.pin)))
        {
            *userId = i;
            return true;
        }
    }

    return false;
}

bool emberAfDoorLockClusterLockDoorCallback(app::CommandHandler * commandObj, const app::ConcreteCommandPath & commandPath,
                                            const Commands::LockDoor::DecodableType & commandData)
{
    // NOTE: PIN code is optional for Lock commands in the Matter spec (5.2.4.1).
    // Don't care that it is optional for now, this should be properly addressed in the upcoming door lock
    // cluster implementation. For now we assume that the PIN code is always present.
    auto & PIN = commandData.pinCode.Value();

    uint8_t userId                = 0;
    bool pinVerified              = verifyPin(PIN, &userId);
    bool doorLocked               = false;
    uint8_t lockStateLocked       = 0x01;
    uint16_t rfOperationEventMask = 0xffff; // will send events by default
    CHIP_ERROR err                = CHIP_NO_ERROR;

    emberAfDoorLockClusterPrint("LOCK DOOR ");
    printSuccessOrFailure(pinVerified);

    if (pinVerified)
    {
        doorLocked = emberAfPluginDoorLockServerActivateDoorLockCallback(true); // lock door
    }

    if (doorLocked)
    {
        emberAfWriteServerAttribute(DOOR_LOCK_SERVER_ENDPOINT, ZCL_DOOR_LOCK_CLUSTER_ID, Attributes::LockState::Id,
                                    &lockStateLocked, ZCL_INT8U_ATTRIBUTE_TYPE);
    }

    // send response
    {
        auto status = doorLocked ? EMBER_ZCL_STATUS_SUCCESS : EMBER_ZCL_STATUS_FAILURE;
        VerifyOrExit(EMBER_ZCL_STATUS_SUCCESS == emberAfSendImmediateDefaultResponse(status), err = CHIP_ERROR_INCORRECT_STATE);
    }

    // check if we should send event notification
    emberAfReadServerAttribute(DOOR_LOCK_SERVER_ENDPOINT, ZCL_DOOR_LOCK_CLUSTER_ID, Attributes::RemoteOperationEventMask::Id,
                               (uint8_t *) &rfOperationEventMask, sizeof(rfOperationEventMask));

    // Possibly send operation event
    if (doorLocked)
    {
        if (rfOperationEventMask & EMBER_BIT(1) && !PIN.empty())
        {
            emberAfFillExternalBuffer((ZCL_CLUSTER_SPECIFIC_COMMAND | ZCL_FRAME_CONTROL_SERVER_TO_CLIENT), ZCL_DOOR_LOCK_CLUSTER_ID,
                                      Commands::OperatingEventNotification::Id, "uuvSwS", 0x01, 0x03, userId, PIN.data(),
                                      static_cast<uint8_t>(PIN.size()), 0X00, PIN.data(), static_cast<uint8_t>(PIN.size()));
        }
    }
    else
    {
        if (rfOperationEventMask & EMBER_BIT(3) && !PIN.empty())
        {
            emberAfFillExternalBuffer((ZCL_CLUSTER_SPECIFIC_COMMAND | ZCL_FRAME_CONTROL_SERVER_TO_CLIENT), ZCL_DOOR_LOCK_CLUSTER_ID,
                                      Commands::OperatingEventNotification::Id, "uuvSwS", 0x01, 0x03, userId, PIN.data(),
                                      static_cast<uint8_t>(PIN.size()), 0x00, PIN.data(), static_cast<uint8_t>(PIN.size()));
        }
    }
    SEND_COMMAND_UNICAST_TO_BINDINGS();
exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "Failed to encode response command.");
    }
    return true;
}

bool emberAfDoorLockClusterUnlockDoorCallback(app::CommandHandler * commandObj, const app::ConcreteCommandPath & commandPath,
                                              const Commands::UnlockDoor::DecodableType & commandData)
{
    // NOTE: PIN code is optional for Unlock command in the Matter spec (5.2.4.2).
    // Don't care that it is optional for now, this should be properly addressed in the upcoming door lock
    // cluster implementation. For now we assume that the PIN code is always present.
    auto & pin = commandData.pinCode.Value();

    uint8_t userId                = 0;
    bool pinVerified              = verifyPin(pin, &userId);
    bool doorUnlocked             = false;
    uint8_t lockStateUnlocked     = 0x02;
    uint16_t rfOperationEventMask = 0xffff; // sends event by default
    CHIP_ERROR err                = CHIP_NO_ERROR;

    emberAfDoorLockClusterPrint("UNLOCK DOOR ");
    printSuccessOrFailure(pinVerified);

    if (pinVerified)
    {
        doorUnlocked = emberAfPluginDoorLockServerActivateDoorLockCallback(false); // unlock door
    }
    if (doorUnlocked)
    {
        emberAfWriteServerAttribute(DOOR_LOCK_SERVER_ENDPOINT, ZCL_DOOR_LOCK_CLUSTER_ID, Attributes::LockState::Id,
                                    &lockStateUnlocked, ZCL_INT8U_ATTRIBUTE_TYPE);
    }

    {
        auto status = doorUnlocked ? EMBER_ZCL_STATUS_SUCCESS : EMBER_ZCL_STATUS_FAILURE;
        VerifyOrExit(EMBER_ZCL_STATUS_SUCCESS == emberAfSendImmediateDefaultResponse(status), err = CHIP_ERROR_INCORRECT_STATE);
    }

    // get bitmask so we can check if we should send event notification
    emberAfReadServerAttribute(DOOR_LOCK_SERVER_ENDPOINT, ZCL_DOOR_LOCK_CLUSTER_ID, Attributes::RemoteOperationEventMask::Id,
                               (uint8_t *) &rfOperationEventMask, sizeof(rfOperationEventMask));

    // send operation event
    if (doorUnlocked && (rfOperationEventMask & EMBER_BIT(2)) && !pin.empty())
    {
        emberAfFillExternalBuffer((ZCL_CLUSTER_SPECIFIC_COMMAND | ZCL_FRAME_CONTROL_SERVER_TO_CLIENT), ZCL_DOOR_LOCK_CLUSTER_ID,
                                  Commands::OperatingEventNotification::Id, "uuvSwS", EMBER_ZCL_DOOR_LOCK_EVENT_SOURCE_RF,
                                  EMBER_ZCL_DOOR_LOCK_OPERATION_EVENT_CODE_UNLOCK, userId, pin.data(),
                                  static_cast<uint8_t>(pin.size()), 0 /*emberAfGetCurrentTime() #2507 */, pin.data(),
                                  static_cast<uint8_t>(pin.size()));
        SEND_COMMAND_UNICAST_TO_BINDINGS();
    }
exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "Failed to encode response command.");
    }
    return true;
}

static uint8_t getWrongCodeEntryLimit()
{
    uint8_t limit = UINT8_MAX;
#ifdef ZCL_USING_DOOR_LOCK_CLUSTER_WRONG_CODE_ENTRY_LIMIT_ATTRIBUTE
    EmberAfStatus status = emberAfReadServerAttribute(DOOR_LOCK_SERVER_ENDPOINT, ZCL_DOOR_LOCK_CLUSTER_ID,
                                                      ZCL_WRONG_CODE_ENTRY_LIMIT_ATTRIBUTE_ID, &limit, sizeof(limit));
    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        emberAfDoorLockClusterPrintln("Failed to read WrongCodeEntryLimitAttribute: 0x%X", status);
    }
#endif
    return limit;
}

static uint8_t getUserCodeTemporaryDisableTime()
{
    uint8_t timeS = 0; // 0 effectively turns off the temporary disable time
#ifdef ZCL_USING_DOOR_LOCK_CLUSTER_USER_CODE_TEMPORARY_DISABLE_TIME_ATTRIBUTE
    EmberAfStatus status = emberAfReadServerAttribute(DOOR_LOCK_SERVER_ENDPOINT, ZCL_DOOR_LOCK_CLUSTER_ID,
                                                      ZCL_USER_CODE_TEMPORARY_DISABLE_TIME_ATTRIBUTE_ID, &timeS, sizeof(timeS));
    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        emberAfDoorLockClusterPrintln("Failed to read UserCodeTemporaryDisableTime: 0x%X", status);
    }
#endif
    return timeS;
}

static void startLockout()
{
    emberAfDoorLockClusterPrintln("Door lock entering lockout mode");

    uint8_t lockoutTimeS = getUserCodeTemporaryDisableTime();
    if (lockoutTimeS != 0)
    {
        emberEventControlSetDelayMS(&emberAfPluginDoorLockServerLockoutEventControl, lockoutTimeS * MILLISECOND_TICKS_PER_SECOND);
    }
}

// If code is NULL, then the door will automatically be unlocked.
static EmberAfStatus applyCode(uint8_t * code, uint8_t codeLength, EmberAfPluginDoorLockServerUser * userTable,
                               uint8_t userTableLength)
{
    for (uint8_t i = 0; i < userTableLength; i++)
    {
        uint8_t * userCode = (userTable == pinUserTable ? userTable[i].code.pin : userTable[i].code.rfid);
        if (code == NULL || (emberAfStringLength(userCode) == codeLength && memcmp(code, userCode + 1, codeLength) == 0))
        {
            EmberAfDoorLockState state = EMBER_ZCL_DOOR_LOCK_STATE_UNLOCKED;
            return emberAfWriteServerAttribute(DOOR_LOCK_SERVER_ENDPOINT, ZCL_DOOR_LOCK_CLUSTER_ID, ZCL_LOCK_STATE_ATTRIBUTE_ID,
                                               (uint8_t *) &state, ZCL_ENUM8_ATTRIBUTE_TYPE);
        }
    }

    wrongCodeEntryCount++;
    if (wrongCodeEntryCount >= getWrongCodeEntryLimit())
    {
        startLockout();
    }
    return EMBER_ZCL_STATUS_FAILURE;
}

void emberAfPluginDoorLockServerLockoutEventHandler(void)
{
    emberEventControlSetInactive(&emberAfPluginDoorLockServerLockoutEventControl);

    emberAfDoorLockClusterPrintln("Door lock entering normal mode");
}

EmberAfStatus emberAfPluginDoorLockServerApplyRfid(uint8_t * rfid, uint8_t rfidLength)
{
    return applyCode(rfid, rfidLength, rfidUserTable, EMBER_AF_PLUGIN_DOOR_LOCK_SERVER_RFID_USER_TABLE_SIZE);
}

EmberAfStatus emberAfPluginDoorLockServerApplyPin(uint8_t * pin, uint8_t pinLength)
{
    return applyCode(pin, pinLength, pinUserTable, EMBER_AF_PLUGIN_DOOR_LOCK_SERVER_PIN_USER_TABLE_SIZE);
}

// --------------------------------------
// Auto relock functionality

// Passing 0 to this function will disable the Relock event.
// Passing UINT32_MAX to this function will force it to use the AutoRelock
// attribute.
static void scheduleAutoRelock(uint32_t autoRelockTimeS)
{
    if (autoRelockTimeS == UINT32_MAX)
    {
        EmberAfStatus status =
            emberAfReadServerAttribute(DOOR_LOCK_SERVER_ENDPOINT, ZCL_DOOR_LOCK_CLUSTER_ID, ZCL_AUTO_RELOCK_TIME_ATTRIBUTE_ID,
                                       (uint8_t *) &autoRelockTimeS, sizeof(autoRelockTimeS));
        if (status != EMBER_ZCL_STATUS_SUCCESS)
        {
            emberAfDoorLockClusterPrintln("Failed to read AutoRelockTime attribute: 0x%X", status);
            return;
        }
    }

    if (autoRelockTimeS == 0)
    {
        emberEventControlSetInactive(&emberAfPluginDoorLockServerRelockEventControl);
    }
    else
    {
        emberEventControlSetDelayMS(&emberAfPluginDoorLockServerRelockEventControl,
                                    (autoRelockTimeS * MILLISECOND_TICKS_PER_SECOND));
    }
}

void emberAfPluginDoorLockServerRelockEventHandler(void)
{
    emberEventControlSetInactive(&emberAfPluginDoorLockServerRelockEventControl);

    EmberAfStatus status = applyCode(NULL, 0, pinUserTable, EMBER_AF_PLUGIN_DOOR_LOCK_SERVER_PIN_USER_TABLE_SIZE);
    emberAfDoorLockClusterPrintln("Door automatically relocked: 0x%X", status);
}

void MatterDoorLockClusterServerAttributeChangedCallback(const app::ConcreteAttributePath & attributePath)
{
    if (attributePath.mEndpointId == DOOR_LOCK_SERVER_ENDPOINT && attributePath.mAttributeId == ZCL_LOCK_STATE_ATTRIBUTE_ID)
    {
        uint8_t lockState;
        EmberAfStatus status = emberAfReadServerAttribute(DOOR_LOCK_SERVER_ENDPOINT, ZCL_DOOR_LOCK_CLUSTER_ID,
                                                          ZCL_LOCK_STATE_ATTRIBUTE_ID, &lockState, sizeof(lockState));
        if (status != EMBER_ZCL_STATUS_SUCCESS)
        {
            emberAfDoorLockClusterPrintln("Failed to read LockState attribute: 0x%X", status);
        }
        else if (lockState == EMBER_ZCL_DOOR_LOCK_STATE_UNLOCKED)
        {
            scheduleAutoRelock(UINT32_MAX);
        }
    }
}

bool emberAfDoorLockClusterUnlockWithTimeoutCallback(app::CommandHandler * commandObj, const app::ConcreteCommandPath & commandPath,
                                                     const Commands::UnlockWithTimeout::DecodableType & commandData)
{
    // NOTE: PIN code is optional for Lock With Timeout commands in the Matter spec (5.2.4.3).
    // Don't care that it is optional for now, this should be properly addressed in the upcoming door lock
    // cluster implementation. For now we assume that the PIN code is always present.
    auto & timeoutS = commandData.timeout;
    auto & pin      = commandData.pinCode.Value();

    uint8_t userId;
    EmberAfStatus status;
    if (verifyPin(pin, &userId))
    {
        uint8_t lockState        = EMBER_ZCL_DOOR_LOCK_STATE_LOCKED;
        EmberAfStatus readStatus = emberAfWriteServerAttribute(DOOR_LOCK_SERVER_ENDPOINT, ZCL_DOOR_LOCK_CLUSTER_ID,
                                                               ZCL_LOCK_STATE_ATTRIBUTE_ID, &lockState, ZCL_ENUM8_ATTRIBUTE_TYPE);
        if (readStatus != EMBER_ZCL_STATUS_SUCCESS)
        {
            emberAfDoorLockClusterPrintln("Failed to write LockState attribute: 0x%X", readStatus);
        }

        scheduleAutoRelock(timeoutS);
        status = EMBER_ZCL_STATUS_SUCCESS; // success (per 7.3.2.17.4)
    }
    else
    {
        status = EMBER_ZCL_STATUS_FAILURE; // failure (per 7.3.2.17.4)
    }

    if (EMBER_ZCL_STATUS_SUCCESS != emberAfSendImmediateDefaultResponse(status))
    {
        ChipLogError(Zcl, "Failed to encode response command.");
    }
    return true;
}

bool emberAfPluginDoorLockServerActivateDoorLockCallback(bool activate)
{
    return true;
}
