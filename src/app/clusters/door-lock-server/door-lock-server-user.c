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

#include "../../include/af.h"
#include "../../util/common.h"
#include "door-lock-server.h"

EmberEventControl emberAfPluginDoorLockServerLockoutEventControl;
EmberEventControl emberAfPluginDoorLockServerRelockEventControl;

// The index into these tables is a userId.
static EmberAfPluginDoorLockServerUser pinUserTable[EMBER_AF_PLUGIN_DOOR_LOCK_SERVER_PIN_USER_TABLE_SIZE];
static EmberAfPluginDoorLockServerUser rfidUserTable[EMBER_AF_PLUGIN_DOOR_LOCK_SERVER_PIN_USER_TABLE_SIZE];

// This is the current number of invalid PIN/RFID's in a row.
static uint8_t wrongCodeEntryCount = 0;

bool emAfPluginDoorLockServerCheckForSufficientSpace(uint8_t spaceReq, uint8_t spaceAvail)
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
    emAfPluginDoorLockServerWriteAttributes(data, COUNTOF(data), "user table");
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
        emberAfDoorLockClusterPrintln("");
    }
    emberAfDoorLockClusterPrintln("RFID:");
    for (i = 0; i < EMBER_AF_PLUGIN_DOOR_LOCK_SERVER_RFID_USER_TABLE_SIZE; i++)
    {
        EmberAfPluginDoorLockServerUser * user = &rfidUserTable[i];
        emberAfDoorLockClusterPrint("%2x %x %x ", i, user->status, user->type);
        printPin(user->code.rfid);
        emberAfDoorLockClusterPrintln("");
    }
}

// Returns status byte for use in SetPinResponse and SetRfidResponse commands.
static uint8_t setUser(uint16_t userId, uint8_t userStatus, uint8_t userType, uint8_t * code,
                       EmberAfPluginDoorLockServerUser * userTable, uint8_t userTableSize)
{
    bool success = false;
    // "code" (i.e. PIN/RFID) is stored in table entry in ZCL format (1-byte
    // length prefix). Don't allow a code with length that exceeds capacity
    // of the table entry field. Note there are potentially different max
    // lengths for PIN v. RFID.
    bool validCodeLength = false;
    if (code != NULL &&
        ((userTable == pinUserTable && emberAfStringLength(code) <= EMBER_AF_PLUGIN_DOOR_LOCK_SERVER_MAX_PIN_LENGTH) ||
         (emberAfStringLength(code) <= EMBER_AF_PLUGIN_DOOR_LOCK_SERVER_MAX_RFID_LENGTH)))
    {
        validCodeLength = true;
    }

    if (validCodeLength && userId < userTableSize)
    {
        EmberAfPluginDoorLockServerUser * user = &userTable[userId];
        user->status                           = userStatus;
        user->type                             = userType;
        MEMMOVE(user->code.rfid, code,
                emberAfStringLength(code) + 1); // + 1 for Zigbee string length byte

        emberAfDoorLockClusterPrintln("***RX SET %s ***", (userTable == pinUserTable ? "PIN" : "RFID"));
        printUserTables();

        success = true;
    }
    return (success ? 0x00 : 0x01); // See 7.3.2.17.6 and 7.3.2.17.23).
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
static uint8_t clearUserPinOrRfid(uint16_t userId, EmberAfPluginDoorLockServerUser * userTable, uint8_t userTableSize)
{
    bool success = false;
    if (userId < userTableSize)
    {
        // Since the rfid member of the struct is a Zigbee string, setting the first
        // byte to 0 will indicate that we have a 0-length pin.
        MEMSET((userTable == pinUserTable ? userTable[userId].code.pin : userTable[userId].code.rfid), 0x00,
               sizeof(userTable[userId].code));
        success = true;
    }
    return (success ? 0x00 : 0x01); // See 7.3.2.17.8 and 7.3.2.17.25).
}

bool emberAfDoorLockClusterGetUserTypeCallback(uint16_t userId)
{
    if (emAfPluginDoorLockServerCheckForSufficientSpace(userId, EMBER_AF_PLUGIN_DOOR_LOCK_SERVER_PIN_USER_TABLE_SIZE))
    {
        EmberAfPluginDoorLockServerUser * user = &pinUserTable[userId];
        emberAfFillCommandDoorLockClusterGetUserTypeResponse(userId, user->type);
        EmberStatus status = emberAfSendResponse();
        if (status != EMBER_SUCCESS)
        {
            emberAfDoorLockClusterPrintln("Failed to send GetUserTypeResponse: 0x%X", status);
        }
    }
    return true;
}

bool emberAfDoorLockClusterSetUserTypeCallback(uint16_t userId, uint8_t userType)
{
    uint8_t status = (emAfPluginDoorLockServerSetPinUserType(userId, userType) ? 0x00   // success (per 7.3.2.17.21)
                                                                               : 0x01); // failure (per 7.3.2.17.21)
    emberAfFillCommandDoorLockClusterSetUserTypeResponse(status);

    EmberStatus emberStatus = emberAfSendResponse();
    if (emberStatus != EMBER_SUCCESS)
    {
        emberAfDoorLockClusterPrintln("Failed to send SetUserTypeResponse: 0x%X", emberStatus);
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

bool emberAfDoorLockClusterSetPinCallback(uint16_t userId, uint8_t userStatus, uint8_t userType, uint8_t * pin)
{
    // send response
    uint8_t status = setUser(userId, userStatus, userType, pin, pinUserTable, EMBER_AF_PLUGIN_DOOR_LOCK_SERVER_PIN_USER_TABLE_SIZE);
    emberAfFillCommandDoorLockClusterSetPinResponse(status);
    emberAfSendResponse();

    // get bitmask so we can check if we should send event notification
    uint16_t rfProgrammingEventMask = 0xffff; // send event by default
    emberAfReadServerAttribute(DOOR_LOCK_SERVER_ENDPOINT, ZCL_DOOR_LOCK_CLUSTER_ID, ZCL_RF_PROGRAMMING_EVENT_MASK_ATTRIBUTE_ID,
                               (uint8_t *) &rfProgrammingEventMask, sizeof(rfProgrammingEventMask));
    if ((rfProgrammingEventMask & BIT(2)) && !status && (pin != NULL))
    {
        emberAfFillCommandDoorLockClusterProgrammingEventNotification(EMBER_ZCL_DOOR_LOCK_EVENT_SOURCE_RF,
                                                                      EMBER_ZCL_DOOR_LOCK_PROGRAMMING_EVENT_CODE_PIN_ADDED, userId,
                                                                      pin, userType, userStatus, emberAfGetCurrentTime(), pin);
        SEND_COMMAND_UNICAST_TO_BINDINGS();
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

bool emberAfDoorLockClusterGetPinCallback(uint16_t userId)
{
    EmberAfPluginDoorLockServerUser user;
    EmberStatus status;
    if (getUser(userId, pinUserTable, EMBER_AF_PLUGIN_DOOR_LOCK_SERVER_PIN_USER_TABLE_SIZE, &user))
    {
        uint8_t fakePin = 0x00;
        emberAfFillCommandDoorLockClusterGetPinResponse(userId, user.status, user.type,
                                                        (getSendPinOverTheAir() ? user.code.pin : &fakePin));
        status = emberAfSendResponse();
    }
    else
    {
        status = emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_INVALID_VALUE);
    }

    if (status != EMBER_SUCCESS)
    {
        emberAfDoorLockClusterPrintln("Failed to send response to GetPin: 0x%X", status);
    }

    return true;
}

bool emberAfDoorLockClusterClearPinCallback(uint16_t userId)
{
    uint8_t status = clearUserPinOrRfid(userId, pinUserTable, EMBER_AF_PLUGIN_DOOR_LOCK_SERVER_PIN_USER_TABLE_SIZE);
    emberAfFillCommandDoorLockClusterClearPinResponse(status);

    EmberStatus emberStatus = emberAfSendResponse();
    if (emberStatus != EMBER_SUCCESS)
    {
        emberAfDoorLockClusterPrintln("Failed to send ClearPinResponse: 0x%X", emberStatus);
    }

    // get bitmask so we can check if we should send event notification
    uint16_t rfProgrammingEventMask = 0xffff; // event sent by default
    uint8_t userPin                 = 0x00;   // Zero length Zigbee string
    emberAfReadServerAttribute(DOOR_LOCK_SERVER_ENDPOINT, ZCL_DOOR_LOCK_CLUSTER_ID, ZCL_RF_PROGRAMMING_EVENT_MASK_ATTRIBUTE_ID,
                               (uint8_t *) &rfProgrammingEventMask, sizeof(rfProgrammingEventMask));
    if ((rfProgrammingEventMask & BIT(2)) && !status)
    {
        emberAfFillCommandDoorLockClusterProgrammingEventNotification(0x01, 0x03, userId, &userPin, 0x00, 0x00, 0x00, &userPin);
        SEND_COMMAND_UNICAST_TO_BINDINGS();
    }
    else if ((rfProgrammingEventMask & BIT(0)) && status)
    {
        emberAfFillCommandDoorLockClusterProgrammingEventNotification(0x01, 0x00, userId, &userPin, 0x00, 0x00, 0x00, &userPin);
        SEND_COMMAND_UNICAST_TO_BINDINGS();
    }

    return true;
}

bool emberAfDoorLockClusterClearAllPinsCallback(void)
{
    uint8_t i;
    for (i = 0; i < EMBER_AF_PLUGIN_DOOR_LOCK_SERVER_PIN_USER_TABLE_SIZE; i++)
    {
        clearUserPinOrRfid(i, pinUserTable, EMBER_AF_PLUGIN_DOOR_LOCK_SERVER_PIN_USER_TABLE_SIZE);
    }

    // 7.3.2.17.9 says that "0x00" indicates success.
    emberAfFillCommandDoorLockClusterClearAllPinsResponse(0x00);
    EmberStatus status = emberAfSendResponse();
    if (status != EMBER_SUCCESS)
    {
        emberAfDoorLockClusterPrintln("Failed to send ClearAllPins: 0x%X", status);
    }
    return true;
}

// ------------------------------------------------------------------------------
// RFID handling

bool emberAfDoorLockClusterSetRfidCallback(uint16_t userId, uint8_t userStatus, uint8_t userType, uint8_t * rfid)
{
    uint8_t status =
        setUser(userId, userStatus, userType, rfid, rfidUserTable, EMBER_AF_PLUGIN_DOOR_LOCK_SERVER_RFID_USER_TABLE_SIZE);
    emberAfFillCommandDoorLockClusterSetRfidResponse(status);

    EmberStatus emberStatus = emberAfSendResponse();
    if (emberStatus != EMBER_SUCCESS)
    {
        emberAfDoorLockClusterPrintln("Failed to send SetRfidResponse: 0x%X", emberStatus);
    }
    return true;
}

bool emberAfDoorLockClusterGetRfidCallback(uint16_t userId)
{
    EmberAfPluginDoorLockServerUser user;
    EmberStatus status;
    if (getUser(userId, rfidUserTable, EMBER_AF_PLUGIN_DOOR_LOCK_SERVER_RFID_USER_TABLE_SIZE, &user))
    {
        emberAfFillCommandDoorLockClusterGetRfidResponse(userId, user.status, user.type, user.code.pin);
        status = emberAfSendResponse();
    }
    else
    {
        status = emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_INVALID_VALUE);
    }

    if (status != EMBER_SUCCESS)
    {
        emberAfDoorLockClusterPrintln("Failed to send response to GetRfid: 0x%X", status);
    }

    return true;
}

bool emberAfDoorLockClusterClearRfidCallback(uint16_t userId)
{
    uint8_t status = clearUserPinOrRfid(userId, rfidUserTable, EMBER_AF_PLUGIN_DOOR_LOCK_SERVER_RFID_USER_TABLE_SIZE);
    emberAfFillCommandDoorLockClusterClearRfidResponse(status);

    EmberStatus emberStatus = emberAfSendResponse();
    if (emberStatus != EMBER_SUCCESS)
    {
        emberAfDoorLockClusterPrintln("Failed to send ClearRfidResponse: 0x%X", emberStatus);
    }
    return true;
}

bool emberAfDoorLockClusterClearAllRfidsCallback(void)
{
    for (uint8_t i = 0; i < EMBER_AF_PLUGIN_DOOR_LOCK_SERVER_RFID_USER_TABLE_SIZE; i++)
    {
        clearUserPinOrRfid(i, rfidUserTable, EMBER_AF_PLUGIN_DOOR_LOCK_SERVER_RFID_USER_TABLE_SIZE);
    }

    // 7.3.2.17.26 says that "0x00" indicates success.
    emberAfFillCommandDoorLockClusterClearAllRfidsResponse(0x00);
    EmberStatus status = emberAfSendResponse();
    if (status != EMBER_SUCCESS)
    {
        emberAfDoorLockClusterPrintln("Failed to send ClearAllRfidsResponse: 0x%X", status);
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
static bool verifyPin(uint8_t * pin, uint8_t * userId)
{
    bool pinRequired = false;
    EmberStatus status;
    uint8_t i;

    status =
        emberAfReadServerAttribute(DOOR_LOCK_SERVER_ENDPOINT, ZCL_DOOR_LOCK_CLUSTER_ID,
                                   ZCL_REQUIRE_PIN_FOR_RF_OPERATION_ATTRIBUTE_ID, (uint8_t *) &pinRequired, sizeof(pinRequired));
    if (EMBER_SUCCESS != status || !pinRequired)
    {
        return true;
    }
    else if (pin == NULL)
    {
        return false;
    }

    for (i = 0; i < EMBER_AF_PLUGIN_DOOR_LOCK_SERVER_PIN_USER_TABLE_SIZE; i++)
    {
        EmberAfPluginDoorLockServerUser * user = &pinUserTable[i];
        uint8_t userPinLength                  = emberAfStringLength(user->code.pin);
        if (userPinLength == emberAfStringLength(pin) && 0 == MEMCOMPARE(&user->code.pin[1], &pin[1], userPinLength))
        {
            *userId = i;
            return true;
        }
    }

    return false;
}

bool emberAfDoorLockClusterLockDoorCallback(uint8_t * PIN)
{
    uint8_t userId                = 0;
    bool pinVerified              = verifyPin(PIN, &userId);
    bool doorLocked               = false;
    uint8_t lockStateLocked       = 0x01;
    uint16_t rfOperationEventMask = 0xffff; // will send events by default

    emberAfDoorLockClusterPrint("LOCK DOOR ");
    printSuccessOrFailure(pinVerified);

    if (pinVerified)
    {
        doorLocked = emberAfPluginDoorLockServerActivateDoorLockCallback(true); // lock door
    }

    if (doorLocked)
    {
        emberAfWriteServerAttribute(DOOR_LOCK_SERVER_ENDPOINT, ZCL_DOOR_LOCK_CLUSTER_ID, ZCL_LOCK_STATE_ATTRIBUTE_ID,
                                    &lockStateLocked, ZCL_INT8U_ATTRIBUTE_TYPE);
    }

    // send response
    emberAfFillCommandDoorLockClusterLockDoorResponse(doorLocked ? EMBER_ZCL_STATUS_SUCCESS : EMBER_ZCL_STATUS_FAILURE);
    emberAfSendResponse();

    // check if we should send event notification
    emberAfReadServerAttribute(DOOR_LOCK_SERVER_ENDPOINT, ZCL_DOOR_LOCK_CLUSTER_ID, ZCL_RF_OPERATION_EVENT_MASK_ATTRIBUTE_ID,
                               (uint8_t *) &rfOperationEventMask, sizeof(rfOperationEventMask));

    // Possibly send operation event
    if (doorLocked)
    {
        if (rfOperationEventMask & BIT(1) && (PIN != NULL))
        {
            emberAfFillCommandDoorLockClusterOperationEventNotification(0x01, 0x01, userId, PIN, 0x00, PIN);
        }
    }
    else
    {
        if (rfOperationEventMask & BIT(3) && (PIN != NULL))
        {
            emberAfFillCommandDoorLockClusterOperationEventNotification(0x01, 0x03, userId, PIN, 0x00, PIN);
        }
    }
    SEND_COMMAND_UNICAST_TO_BINDINGS();

    return true;
}

bool emberAfDoorLockClusterUnlockDoorCallback(uint8_t * pin)
{
    uint8_t userId                = 0;
    bool pinVerified              = verifyPin(pin, &userId);
    bool doorUnlocked             = false;
    uint8_t lockStateUnlocked     = 0x02;
    uint16_t rfOperationEventMask = 0xffff; // sends event by default
    emberAfDoorLockClusterPrint("UNLOCK DOOR ");
    printSuccessOrFailure(pinVerified);

    if (pinVerified)
    {
        doorUnlocked = emberAfPluginDoorLockServerActivateDoorLockCallback(false); // unlock door
    }
    if (doorUnlocked)
    {
        emberAfWriteServerAttribute(DOOR_LOCK_SERVER_ENDPOINT, ZCL_DOOR_LOCK_CLUSTER_ID, ZCL_LOCK_STATE_ATTRIBUTE_ID,
                                    &lockStateUnlocked, ZCL_INT8U_ATTRIBUTE_TYPE);
    }

    emberAfFillCommandDoorLockClusterUnlockDoorResponse(doorUnlocked ? EMBER_ZCL_STATUS_SUCCESS : EMBER_ZCL_STATUS_FAILURE);
    emberAfSendResponse();

    // get bitmask so we can check if we should send event notification
    emberAfReadServerAttribute(DOOR_LOCK_SERVER_ENDPOINT, ZCL_DOOR_LOCK_CLUSTER_ID, ZCL_RF_OPERATION_EVENT_MASK_ATTRIBUTE_ID,
                               (uint8_t *) &rfOperationEventMask, sizeof(rfOperationEventMask));

    // send operation event
    if (doorUnlocked && (rfOperationEventMask & BIT(2)) && (pin != NULL))
    {
        emberAfFillCommandDoorLockClusterOperationEventNotification(EMBER_ZCL_DOOR_LOCK_EVENT_SOURCE_RF,
                                                                    EMBER_ZCL_DOOR_LOCK_OPERATION_EVENT_CODE_UNLOCK, userId, pin,
                                                                    emberAfGetCurrentTime(), pin);
        SEND_COMMAND_UNICAST_TO_BINDINGS();
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
        emberEventControlSetDelayMS(emberAfPluginDoorLockServerLockoutEventControl, lockoutTimeS * MILLISECOND_TICKS_PER_SECOND);
    }
}

// If code is NULL, then the door will automatically be unlocked.
static EmberAfStatus applyCode(uint8_t * code, uint8_t codeLength, EmberAfPluginDoorLockServerUser * userTable,
                               uint8_t userTableLength)
{
    for (uint8_t i = 0; i < userTableLength; i++)
    {
        uint8_t * userCode = (userTable == pinUserTable ? userTable[i].code.pin : userTable[i].code.rfid);
        if (code == NULL || (emberAfStringLength(userCode) == codeLength && MEMCOMPARE(code, userCode + 1, codeLength) == 0))
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
    emberEventControlSetInactive(emberAfPluginDoorLockServerLockoutEventControl);
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
        emberEventControlSetInactive(emberAfPluginDoorLockServerRelockEventControl);
    }
    else
    {
        emberEventControlSetDelayMS(emberAfPluginDoorLockServerRelockEventControl,
                                    (autoRelockTimeS * MILLISECOND_TICKS_PER_SECOND));
    }
}

void emberAfPluginDoorLockServerRelockEventHandler(void)
{
    emberEventControlSetInactive(emberAfPluginDoorLockServerRelockEventControl);

    EmberAfStatus status = applyCode(NULL, 0, pinUserTable, EMBER_AF_PLUGIN_DOOR_LOCK_SERVER_PIN_USER_TABLE_SIZE);
    emberAfDoorLockClusterPrintln("Door automatically relocked: 0x%X", status);
}

void emberAfDoorLockClusterServerAttributeChangedCallback(uint8_t endpoint, EmberAfAttributeId attributeId)
{
    if (endpoint == DOOR_LOCK_SERVER_ENDPOINT && attributeId == ZCL_LOCK_STATE_ATTRIBUTE_ID)
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

bool emberAfDoorLockClusterUnlockWithTimeoutCallback(uint16_t timeoutS, uint8_t * pin)
{
    uint8_t userId;
    uint8_t status;
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
        status = 0x00; // success (per 7.3.2.17.4)
    }
    else
    {
        status = 0x01; // failure (per 7.3.2.17.4)
    }

    emberAfFillCommandDoorLockClusterUnlockWithTimeoutResponse(status);
    EmberStatus emberStatus = emberAfSendResponse();
    if (emberStatus != EMBER_SUCCESS)
    {
        emberAfDoorLockClusterPrintln("Failed to send UnlockWithTimeoutResponse: 0x%X", status);
    }

    return true;
}
