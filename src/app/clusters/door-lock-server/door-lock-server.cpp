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

/****************************************************************************
 * @file
 * @brief Routines for the Door Lock Server plugin.
 *******************************************************************************
 ******************************************************************************/

#include "door-lock-server.h"
#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/callback.h>
#include <app-common/zap-generated/cluster-id.h>
#include <app-common/zap-generated/command-id.h>
#include <app/EventLogging.h>
#include <app/util/af-event.h>
#include <app/util/af.h>
#include <cinttypes>

#include <app/CommandHandler.h>
#include <app/ConcreteAttributePath.h>
#include <app/ConcreteCommandPath.h>
#include <app/EventLogging.h>
#include <lib/support/CodeUtils.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::DataModel;
using namespace chip::app::Clusters::DoorLock;

static constexpr uint8_t DOOR_LOCK_SCHEDULE_MAX_HOUR   = 23;
static constexpr uint8_t DOOR_LOCK_SCHEDULE_MAX_MINUTE = 59;

// emberEventControlSetDelayMS() uses uint32_t for timeout in milliseconds but doesn't accept
// values more than MAX(UINT32) / 2. This is internal value. Thus, lets limit our relock timeout
// in seconds with the appropriate maximum to ensure that delay setting won't fail.
static constexpr uint32_t DOOR_LOCK_MAX_LOCK_TIMEOUT_SEC = MAX_INT32U_VALUE / (2 * MILLISECOND_TICKS_PER_SECOND);

DoorLockServer DoorLockServer::instance;

void emberAfPluginDoorLockOnAutoRelock(chip::EndpointId endpointId);

/**********************************************************
 * DoorLockServer public methods
 *********************************************************/

DoorLockServer & DoorLockServer::Instance()
{
    return instance;
}

/**
 * @brief Initializes given endpoint for a server.
 *
 * @param endpointId
 */
void DoorLockServer::InitServer(chip::EndpointId endpointId)
{
    emberAfDoorLockClusterPrintln("Door Lock cluster initialized at endpoint #%" PRIu16, endpointId);

    SetLockState(endpointId, DlLockState::kLocked, DlOperationSource::kUnspecified);
    SetActuatorEnabled(endpointId, true);
}

bool DoorLockServer::SetLockState(chip::EndpointId endpointId, DlLockState newLockState, DlOperationSource opSource)
{
    bool success = SetAttribute(endpointId, Attributes::LockState::Id, Attributes::LockState::Set, newLockState);

    // Remote operations are handled separately as they use more data unavailable here
    VerifyOrReturnError(DlOperationSource::kRemote != opSource, success);

    // DlLockState::kNotFullyLocked has no appropriate event to send. Also it is unclear whether
    // it should schedule auto-relocking. So skip it here. Check for supported states explicitly
    // to handle possible enum extending in future.
    VerifyOrReturnError(DlLockState::kLocked == newLockState || DlLockState::kUnlocked == newLockState, success);

    // Send LockOperation event
    auto opType = (DlLockState::kLocked == newLockState) ? DlLockOperationType::kLock : DlLockOperationType::kUnlock;

    SendLockOperationEvent(endpointId, opType, opSource, DlOperationError::kUnspecified, Nullable<uint16_t>(),
                           Nullable<chip::FabricIndex>(), Nullable<chip::NodeId>(), nullptr, 0, success);

    // Schedule auto-relocking
    if (success && DlLockOperationType::kUnlock == opType)
    {
        // appclusters.pdf 5.3.3.25:
        // The number of seconds to wait after unlocking a lock before it automatically locks again. 0=disabled. If set, unlock
        // operations from any source will be timed. For one time unlock with timeout use the specific command.
        uint32_t autoRelockTime = 0;

        VerifyOrReturnError(GetAutoRelockTime(endpointId, autoRelockTime), false);
        VerifyOrReturnError(0 != autoRelockTime, true);
        ScheduleAutoRelock(endpointId, autoRelockTime);
    }

    return success;
}

bool DoorLockServer::SetActuatorEnabled(chip::EndpointId endpointId, bool newActuatorState)
{
    return SetAttribute(endpointId, Attributes::ActuatorEnabled::Id, Attributes::ActuatorEnabled::Set, newActuatorState);
}

bool DoorLockServer::SetDoorState(chip::EndpointId endpointId, DlDoorState newDoorState)
{
    bool success = SetAttribute(endpointId, Attributes::DoorState::Id, Attributes::DoorState::Set, newDoorState);

    if (success)
    {
        Events::DoorStateChange::Type event{ newDoorState };
        SendEvent(endpointId, event);
    }

    return success;
}

bool DoorLockServer::SetLanguage(chip::EndpointId endpointId, chip::CharSpan newLanguage)
{
    return SetAttribute(endpointId, Attributes::Language::Id, Attributes::Language::Set, newLanguage);
}

bool DoorLockServer::SetAutoRelockTime(chip::EndpointId endpointId, uint32_t newAutoRelockTimeSec)
{
    return SetAttribute(endpointId, Attributes::AutoRelockTime::Id, Attributes::AutoRelockTime::Set, newAutoRelockTimeSec);
}

bool DoorLockServer::SetSoundVolume(chip::EndpointId endpointId, uint8_t newSoundVolume)
{
    return SetAttribute(endpointId, Attributes::SoundVolume::Id, Attributes::SoundVolume::Set, newSoundVolume);
}

bool DoorLockServer::SetOneTouchLocking(chip::EndpointId endpointId, bool isEnabled)
{
    return SetAttribute(endpointId, Attributes::EnableOneTouchLocking::Id, Attributes::EnableOneTouchLocking::Set, isEnabled);
}

bool DoorLockServer::SetPrivacyModeButton(chip::EndpointId endpointId, bool isEnabled)
{
    return SetAttribute(endpointId, Attributes::EnablePrivacyModeButton::Id, Attributes::EnablePrivacyModeButton::Set, isEnabled);
}

bool DoorLockServer::GetAutoRelockTime(chip::EndpointId endpointId, uint32_t & autoRelockTime)
{
    return GetAttribute(endpointId, Attributes::AutoRelockTime::Id, Attributes::AutoRelockTime::Get, autoRelockTime);
}

bool DoorLockServer::GetNumberOfUserSupported(chip::EndpointId endpointId, uint16_t & numberOfUsersSupported)
{
    return GetAttribute(endpointId, Attributes::NumberOfTotalUsersSupported::Id, Attributes::NumberOfTotalUsersSupported::Get,
                        numberOfUsersSupported);
}

bool DoorLockServer::GetNumberOfPINCredentialsSupported(chip::EndpointId endpointId, uint16_t & numberOfPINCredentials)
{
    return GetAttribute(endpointId, Attributes::NumberOfPINUsersSupported::Id, Attributes::NumberOfPINUsersSupported::Get,
                        numberOfPINCredentials);
}

bool DoorLockServer::GetNumberOfRFIDCredentialsSupported(chip::EndpointId endpointId, uint16_t & numberOfRFIDCredentials)
{
    return GetAttribute(endpointId, Attributes::NumberOfRFIDUsersSupported::Id, Attributes::NumberOfRFIDUsersSupported::Get,
                        numberOfRFIDCredentials);
}

bool DoorLockServer::GetNumberOfWeekDaySchedulesPerUserSupported(chip::EndpointId endpointId,
                                                                 uint8_t & numberOfWeekDaySchedulesPerUser)
{
    return GetAttribute(endpointId, Attributes::NumberOfWeekDaySchedulesSupportedPerUser::Id,
                        Attributes::NumberOfWeekDaySchedulesSupportedPerUser::Get, numberOfWeekDaySchedulesPerUser);
}

bool DoorLockServer::GetNumberOfYearDaySchedulesPerUserSupported(chip::EndpointId endpointId,
                                                                 uint8_t & numberOfYearDaySchedulesPerUser)
{
    return GetAttribute(endpointId, Attributes::NumberOfYearDaySchedulesSupportedPerUser::Id,
                        Attributes::NumberOfYearDaySchedulesSupportedPerUser::Get, numberOfYearDaySchedulesPerUser);
}

void DoorLockServer::SetUserCommandHandler(chip::app::CommandHandler * commandObj,
                                           const chip::app::ConcreteCommandPath & commandPath,
                                           const chip::app::Clusters::DoorLock::Commands::SetUser::DecodableType & commandData)
{
    auto & userIndex = commandData.userIndex;
    emberAfDoorLockClusterPrintln("[SetUser] Incoming command [endpointId=%d,userIndex=%d]", commandPath.mEndpointId, userIndex);

    if (!SupportsUSR(commandPath.mEndpointId))
    {
        emberAfDoorLockClusterPrintln("[SetUser] User management is not supported [endpointId=%d]", commandPath.mEndpointId);
        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_UNSUPPORTED_COMMAND);
        return;
    }

    auto fabricIdx = getFabricIndex(commandObj);
    if (chip::kUndefinedFabricIndex == fabricIdx)
    {
        ChipLogError(Zcl, "[SetUser] Unable to get the fabric IDX [endpointId=%d,userIndex=%d]", commandPath.mEndpointId,
                     userIndex);
        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_FAILURE);
        return;
    }

    auto sourceNodeId = getNodeId(commandObj);
    if (chip::kUndefinedNodeId == sourceNodeId)
    {
        ChipLogError(Zcl, "[SetUser] Unable to get the source node index [endpointId=%d,userIndex=%d]", commandPath.mEndpointId,
                     userIndex);
        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_FAILURE);
        return;
    }

    auto & operationType  = commandData.operationType;
    auto & userName       = commandData.userName;
    auto & userUniqueId   = commandData.userUniqueId;
    auto & userStatus     = commandData.userStatus;
    auto & userType       = commandData.userType;
    auto & credentialRule = commandData.credentialRule;

    if (!userIndexValid(commandPath.mEndpointId, userIndex))
    {
        emberAfDoorLockClusterPrintln("[SetUser] User index out of bounds [userIndex=%d]", userIndex);
        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_INVALID_COMMAND);
        return;
    }

    // appclusters, 5.2.4.34: UserName has maximum DOOR_LOCK_MAX_USER_NAME_SIZE (10) characters excluding NUL terminator in it.
    if (!userName.IsNull() && userName.Value().size() > DOOR_LOCK_MAX_USER_NAME_SIZE)
    {
        emberAfDoorLockClusterPrintln(
            "[SetUser] Unable to set user: userName too long [endpointId=%d,userIndex=%d,userNameSize=%zu]",
            commandPath.mEndpointId, userIndex, userName.Value().size());

        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_INVALID_COMMAND);
        return;
    }

    EmberAfStatus status = EMBER_ZCL_STATUS_SUCCESS;
    switch (operationType)
    {
    case DlDataOperationType::kAdd:
        status = createUser(commandPath.mEndpointId, fabricIdx, sourceNodeId, userIndex, userName, userUniqueId, userStatus,
                            userType, credentialRule);
        break;
    case DlDataOperationType::kModify:
        status = modifyUser(commandPath.mEndpointId, fabricIdx, sourceNodeId, userIndex, userName, userUniqueId, userStatus,
                            userType, credentialRule);
        break;
    case DlDataOperationType::kClear:
        // appclusters, 5.2.4.34: SetUser command allow only kAdd/kModify, we should respond with INVALID_COMMAND if we got kClear
        status = EMBER_ZCL_STATUS_INVALID_COMMAND;
        break;
    }

    emberAfSendImmediateDefaultResponse(status);
}

void DoorLockServer::GetUserCommandHandler(chip::app::CommandHandler * commandObj,
                                           const chip::app::ConcreteCommandPath & commandPath,
                                           const chip::app::Clusters::DoorLock::Commands::GetUser::DecodableType & commandData)
{
    auto & userIndex = commandData.userIndex;

    emberAfDoorLockClusterPrintln("[GetUser] Incoming command [endpointId=%d,userIndex=%d]", commandPath.mEndpointId, userIndex);

    if (!SupportsUSR(commandPath.mEndpointId))
    {
        emberAfDoorLockClusterPrintln("[GetUser] User management is not supported [endpointId=%d]", commandPath.mEndpointId);
        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_UNSUPPORTED_COMMAND);
        return;
    }

    uint16_t maxNumberOfUsers = 0;
    if (!userIndexValid(commandPath.mEndpointId, userIndex, maxNumberOfUsers))
    {
        emberAfDoorLockClusterPrintln("[GetUser] User index out of bounds [userIndex=%d,numberOfTotalUsersSupported=%d]", userIndex,
                                      maxNumberOfUsers);
        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_INVALID_COMMAND);
        return;
    }

    CHIP_ERROR err = CHIP_NO_ERROR;
    EmberAfPluginDoorLockUserInfo user;
    VerifyOrExit(emberAfPluginDoorLockGetUser(commandPath.mEndpointId, userIndex, user), err = CHIP_ERROR_INTERNAL);
    {
        chip::app::ConcreteCommandPath path = { emberAfCurrentEndpoint(), ::Id, Commands::GetUserResponse::Id };
        chip::TLV::TLVWriter * writer;
        SuccessOrExit(err = commandObj->PrepareCommand(path));
        VerifyOrExit((writer = commandObj->GetCommandDataIBTLVWriter()) != nullptr, err = CHIP_ERROR_INCORRECT_STATE);
        SuccessOrExit(
            err = writer->Put(chip::TLV::ContextTag(to_underlying(Commands::GetUserResponse::Fields::kUserIndex)), userIndex));

        using ResponseFields = Commands::GetUserResponse::Fields;

        // appclusters, 5.2.4.36: we should not add user-specific field if the user status is set to Available
        if (DlUserStatus::kAvailable != user.userStatus)
        {
            emberAfDoorLockClusterPrintln("Found user in storage: "
                                          "[userIndex=%d,userName=\"%s\",userStatus=%u,userType=%u"
                                          ",credentialRule=%u,createdBy=%u,modifiedBy=%u]",
                                          userIndex, user.userName.data(), to_underlying(user.userStatus),
                                          to_underlying(user.userType), to_underlying(user.credentialRule), user.createdBy,
                                          user.lastModifiedBy);

            SuccessOrExit(err = writer->PutString(TLV::ContextTag(to_underlying(ResponseFields::kUserName)), user.userName));
            if (0xFFFFFFFFU != user.userUniqueId)
            {
                SuccessOrExit(err = writer->Put(TLV::ContextTag(to_underlying(ResponseFields::kUserUniqueId)), user.userUniqueId));
            }
            SuccessOrExit(err = writer->Put(TLV::ContextTag(to_underlying(ResponseFields::kUserStatus)), user.userStatus));
            SuccessOrExit(err = writer->Put(TLV::ContextTag(to_underlying(ResponseFields::kUserType)), user.userType));
            SuccessOrExit(err = writer->Put(TLV::ContextTag(to_underlying(ResponseFields::kCredentialRule)), user.credentialRule));
            if (user.credentials.size() > 0)
            {
                TLV::TLVType credentialsContainer;
                SuccessOrExit(err = writer->StartContainer(TLV::ContextTag(to_underlying(ResponseFields::kCredentials)),
                                                           TLV::kTLVType_Array, credentialsContainer));
                for (size_t i = 0; i < user.credentials.size(); ++i)
                {
                    Structs::DlCredential::Type credential;
                    credential.credentialIndex = user.credentials.data()[i].CredentialIndex;
                    credential.credentialType  = static_cast<DlCredentialType>(user.credentials.data()[i].CredentialType);
                    SuccessOrExit(err = credential.Encode(*writer, TLV::AnonymousTag()));
                }
                SuccessOrExit(err = writer->EndContainer(credentialsContainer));
            }
            SuccessOrExit(err = writer->Put(TLV::ContextTag(to_underlying(ResponseFields::kCreatorFabricIndex)), user.createdBy));
            SuccessOrExit(
                err = writer->Put(TLV::ContextTag(to_underlying(ResponseFields::kLastModifiedFabricIndex)), user.lastModifiedBy));
        }
        else
        {
            emberAfDoorLockClusterPrintln("[GetUser] User not found [userIndex=%d]", userIndex);
        }

        // appclusters, 5.2.4.36.1: We need to add next available user after userIndex if any.
        uint16_t nextAvailableUserIndex = 0;
        if (findUnoccupiedUserSlot(commandPath.mEndpointId, static_cast<uint16_t>(userIndex + 1), nextAvailableUserIndex))
        {
            SuccessOrExit(err =
                              writer->Put(TLV::ContextTag(to_underlying(ResponseFields::kNextUserIndex)), nextAvailableUserIndex));
        }
        SuccessOrExit(err = commandObj->FinishCommand());
    }

exit:
    if (CHIP_NO_ERROR != err)
    {
        ChipLogError(Zcl, "[GetUser] Command processing failed [endpointId=%d,userIndex=%d,err=\"%s\"]", commandPath.mEndpointId,
                     userIndex, err.AsString());
        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_FAILURE);
    }
}

void DoorLockServer::ClearUserCommandHandler(chip::app::CommandHandler * commandObj,
                                             const chip::app::ConcreteCommandPath & commandPath,
                                             const chip::app::Clusters::DoorLock::Commands::ClearUser::DecodableType & commandData)
{
    auto & userIndex = commandData.userIndex;
    emberAfDoorLockClusterPrintln("[ClearUser] Incoming command [endpointId=%d,userIndex=%d]", commandPath.mEndpointId, userIndex);

    if (!SupportsUSR(commandPath.mEndpointId))
    {
        emberAfDoorLockClusterPrintln("[ClearUser] User management is not supported [endpointId=%d]", commandPath.mEndpointId);
        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_UNSUPPORTED_COMMAND);
        return;
    }

    auto fabricIdx = getFabricIndex(commandObj);
    if (chip::kUndefinedFabricIndex == fabricIdx)
    {
        ChipLogError(Zcl, "[ClearUser] Unable to get the fabric IDX [endpointId=%d,userIndex=%d]", commandPath.mEndpointId,
                     userIndex);
        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_FAILURE);
        return;
    }

    auto sourceNodeId = getNodeId(commandObj);
    if (chip::kUndefinedNodeId == sourceNodeId)
    {
        ChipLogError(Zcl, "[ClearUser] Unable to get the source node index [endpointId=%d,userIndex=%d]", commandPath.mEndpointId,
                     userIndex);
        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_FAILURE);
        return;
    }

    uint16_t maxNumberOfUsers = 0;
    if (!userIndexValid(commandPath.mEndpointId, userIndex, maxNumberOfUsers) && userIndex != 0xFFFE)
    {
        emberAfDoorLockClusterPrintln("[ClearUser] User index out of bounds [userIndex=%d,numberOfTotalUsersSupported=%d]",
                                      userIndex, maxNumberOfUsers);
        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_INVALID_COMMAND);
        return;
    }

    if (0xFFFE != userIndex)
    {
        auto status = clearUser(commandPath.mEndpointId, fabricIdx, sourceNodeId, userIndex, true);
        if (EMBER_ZCL_STATUS_SUCCESS != status)
        {
            ChipLogError(Zcl, "[ClearUser] App reported failure when resetting the user [userIndex=%d,status=0x%x]", userIndex,
                         status);
        }
        emberAfSendImmediateDefaultResponse(status);
        return;
    }

    emberAfDoorLockClusterPrintln("[ClearUser] Removing all users from storage");
    for (uint16_t i = 1; i <= maxNumberOfUsers; ++i)
    {
        auto status = clearUser(commandPath.mEndpointId, fabricIdx, sourceNodeId, i, false);
        if (EMBER_ZCL_STATUS_SUCCESS != status)
        {
            ChipLogError(Zcl, "[ClearUser] App reported failure when resetting the user [userIndex=%d,status=0x%x]", i, status);

            emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_FAILURE);
            return;
        }
    }
    emberAfDoorLockClusterPrintln("[ClearUser] Removed all users from storage [users=%d]", maxNumberOfUsers);

    sendRemoteLockUserChange(commandPath.mEndpointId, DlLockDataType::kUserIndex, DlDataOperationType::kClear, sourceNodeId,
                             fabricIdx, 0xFFFE, 0xFFFE);

    emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_SUCCESS);
}

void DoorLockServer::SetCredentialCommandHandler(
    chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
    const chip::app::Clusters::DoorLock::Commands::SetCredential::DecodableType & commandData)
{
    emberAfDoorLockClusterPrintln("[SetCredential] Incoming command [endpointId=%d]", commandPath.mEndpointId);

    auto fabricIdx = getFabricIndex(commandObj);
    if (kUndefinedFabricIndex == fabricIdx)
    {
        ChipLogError(Zcl, "[SetCredential] Unable to get the fabric IDX [endpointId=%d]", commandPath.mEndpointId);
        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_FAILURE);
        return;
    }

    auto sourceNodeId = getNodeId(commandObj);
    if (chip::kUndefinedNodeId == sourceNodeId)
    {
        ChipLogError(Zcl, "[SetCredential] Unable to get the source node index [endpointId=%d]", commandPath.mEndpointId);
        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_FAILURE);
        return;
    }

    auto & operationType   = commandData.operationType;
    auto & credentialType  = commandData.credential.credentialType;
    auto & credentialIndex = commandData.credential.credentialIndex;
    auto & credentialData  = commandData.credentialData;
    auto & userIndex       = commandData.userIndex;
    auto & userStatus      = commandData.userStatus;
    auto & userType        = commandData.userType;

    if (!credentialTypeSupported(commandPath.mEndpointId, credentialType))
    {
        emberAfDoorLockClusterPrintln("[SetCredential] Credential type is not supported [endpointId=%d,credentialType=%u]",
                                      commandPath.mEndpointId, to_underlying(credentialType));
        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_UNSUPPORTED_COMMAND);
        return;
    }

    // appclusters, 5.2.4.41: response should contain next available credential slot
    uint16_t nextAvailableCredentialSlot = 0;
    findUnoccupiedCredentialSlot(commandPath.mEndpointId, credentialType, static_cast<uint16_t>(credentialIndex + 1),
                                 nextAvailableCredentialSlot);

    uint16_t maxNumberOfCredentials = 0;
    if (!credentialIndexValid(commandPath.mEndpointId, credentialType, credentialIndex, maxNumberOfCredentials))
    {
        emberAfDoorLockClusterPrintln("[SetCredential] Credential index is out of range [endpointId=%d,credentialType=%u"
                                      ",credentialIndex=%d]",
                                      commandPath.mEndpointId, to_underlying(credentialType), credentialIndex);
        sendSetCredentialResponse(commandObj, DlStatus::kInvalidField, 0, nextAvailableCredentialSlot);
        return;
    }

    // appclusters, 5.2.4.40.3: If the credential data length is out of bounds we should return INVALID_COMMAND
    size_t minSize, maxSize;
    if (!getCredentialRange(commandPath.mEndpointId, credentialType, minSize, maxSize))
    {
        emberAfDoorLockClusterPrintln(
            "[SetCredential] Unable to get min/max range for credential: internal error [endpointId=%d,credentialIndex=%d]",
            commandPath.mEndpointId, credentialIndex);
        sendSetCredentialResponse(commandObj, DlStatus::kFailure, 0, nextAvailableCredentialSlot);
        return;
    }
    if (credentialData.size() < minSize || credentialData.size() > maxSize)
    {
        emberAfDoorLockClusterPrintln("[SetCredential] Credential data size is out of range "
                                      "[endpointId=%d,credentialType=%u,minLength=%zu,maxLength=%zu,length=%zu]",
                                      commandPath.mEndpointId, to_underlying(credentialType), minSize, maxSize,
                                      credentialData.size());
        sendSetCredentialResponse(commandObj, DlStatus::kInvalidField, 0, nextAvailableCredentialSlot);
        return;
    }

    // appclusters, 5.2.4.41.1: we should return DUPLICATE in the response if we're trying to create duplicated credential entry
    for (uint16_t i = 1; DlCredentialType::kProgrammingPIN != credentialType && (i <= maxNumberOfCredentials); ++i)
    {
        EmberAfPluginDoorLockCredentialInfo currentCredential;
        if (!emberAfPluginDoorLockGetCredential(commandPath.mEndpointId, i, credentialType, currentCredential))
        {
            emberAfDoorLockClusterPrintln("[SetCredential] Unable to get the credential to exclude duplicated entry "
                                          "[endpointId=%d,credentialType=%u,credentialIndex=%d]",
                                          commandPath.mEndpointId, to_underlying(credentialType), i);
            sendSetCredentialResponse(commandObj, DlStatus::kFailure, 0, nextAvailableCredentialSlot);
            return;
        }
        if (DlCredentialStatus::kAvailable != currentCredential.status && currentCredential.credentialType == credentialType &&
            currentCredential.credentialData.data_equal(credentialData))
        {
            emberAfDoorLockClusterPrintln(
                "[SetCredential] Credential with the same data and type already exist "
                "[endpointId=%d,credentialType=%u,dataLength=%zu,existingCredentialIndex=%d,credentialIndex=%d]",
                commandPath.mEndpointId, to_underlying(credentialType), credentialData.size(), i, credentialIndex);
            sendSetCredentialResponse(commandObj, DlStatus::kDuplicate, 0, nextAvailableCredentialSlot);
            return;
        }
    }

    EmberAfPluginDoorLockCredentialInfo existingCredential;
    if (!emberAfPluginDoorLockGetCredential(commandPath.mEndpointId, credentialIndex, credentialType, existingCredential))
    {
        emberAfDoorLockClusterPrintln(
            "[SetCredential] Unable to check if credential exists: app error [endpointId=%d,credentialIndex=%d]",
            commandPath.mEndpointId, credentialIndex);

        sendSetCredentialResponse(commandObj, DlStatus::kFailure, 0, nextAvailableCredentialSlot);
        return;
    }

    switch (operationType)
    {
    case DlDataOperationType::kAdd: {
        uint16_t createdUserIndex = 0;

        DlStatus status = createCredential(commandPath.mEndpointId, fabricIdx, sourceNodeId, credentialIndex, credentialType,
                                           existingCredential, credentialData, userIndex, userStatus, userType, createdUserIndex);
        sendSetCredentialResponse(commandObj, status, createdUserIndex, nextAvailableCredentialSlot);
        return;
    }
    case DlDataOperationType::kModify: {
        // appclusters, 5.2.4.41.1: should send the INVALID_COMMAND in the response when the credential is in use
        if (DlCredentialStatus::kAvailable == existingCredential.status)
        {
            emberAfDoorLockClusterPrintln("[SetCredential] Unable to modify the credential: credential slot is not occupied "
                                          "[endpointId=%d,credentialIndex=%d]",
                                          commandPath.mEndpointId, credentialIndex);

            sendSetCredentialResponse(commandObj, DlStatus::kInvalidField, 0, nextAvailableCredentialSlot);
            return;
        }

        // if userIndex is NULL then we're changing the programming user PIN
        if (userIndex.IsNull())
        {
            auto status = modifyProgrammingPIN(commandPath.mEndpointId, fabricIdx, sourceNodeId, credentialIndex, credentialType,
                                               existingCredential, credentialData);
            sendSetCredentialResponse(commandObj, status, 0, nextAvailableCredentialSlot);
            return;
        }

        auto status = modifyCredential(commandPath.mEndpointId, fabricIdx, sourceNodeId, credentialIndex, credentialType,
                                       existingCredential, credentialData, userIndex.Value(), userStatus, userType);
        sendSetCredentialResponse(commandObj, status, 0, nextAvailableCredentialSlot);
        return;
    }
    case DlDataOperationType::kClear:
        // appclusters, 5.2.4.40: set credential command supports only Add and Modify operational type.
        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_INVALID_COMMAND);
        return;
    }

    emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_SUCCESS);
}

void DoorLockServer::GetCredentialStatusCommandHandler(
    chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
    const chip::app::Clusters::DoorLock::Commands::GetCredentialStatus::DecodableType & commandData)
{
    emberAfDoorLockClusterPrintln("[GetCredentialStatus] Incoming command [endpointId=%d]", commandPath.mEndpointId);

    const auto & credentialType  = commandData.credential.credentialType;
    const auto & credentialIndex = commandData.credential.credentialIndex;

    if (!credentialTypeSupported(commandPath.mEndpointId, credentialType))
    {
        emberAfDoorLockClusterPrintln("[GetCredentialStatus] Credential type is not supported [endpointId=%d,credentialType=%u"
                                      "]",
                                      commandPath.mEndpointId, to_underlying(credentialType));
        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_UNSUPPORTED_COMMAND);
        return;
    }

    uint16_t maxNumberOfCredentials = 0;
    if (!credentialIndexValid(commandPath.mEndpointId, credentialType, credentialIndex, maxNumberOfCredentials))
    {
        emberAfDoorLockClusterPrintln("[GetCredentialStatus] Credential index is out of range "
                                      "[endpointId=%d,credentialType=%u,credentialIndex=%d,maxNumberOfCredentials=%d]",
                                      commandPath.mEndpointId, to_underlying(credentialType), credentialIndex,
                                      maxNumberOfCredentials);
        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_INVALID_COMMAND);
        return;
    }

    EmberAfPluginDoorLockCredentialInfo credentialInfo;
    if (!emberAfPluginDoorLockGetCredential(commandPath.mEndpointId, credentialIndex, credentialType, credentialInfo))
    {
        emberAfDoorLockClusterPrintln("[GetCredentialStatus] Unable to get the credential: app error "
                                      "[endpointId=%d,credentialIndex=%d,credentialType=%u]",
                                      commandPath.mEndpointId, credentialIndex, to_underlying(credentialType));
        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_FAILURE);
        return;
    }

    auto credentialExists            = DlCredentialStatus::kAvailable != credentialInfo.status;
    uint16_t userIndexWithCredential = 0;
    if (credentialExists)
    {
        if (!findUserIndexByCredential(commandPath.mEndpointId, credentialType, credentialIndex, userIndexWithCredential))
        {
            // That means that there's some kind of error in our database -- there is an unassociated credential. I'm not sure how
            // to handle that properly other than panic in the log.
            ChipLogError(Zcl,
                         "[GetCredentialStatus] Database possibly corrupted - credential exists without user assigned "
                         "[endpointId=%d,credentialType=%u,credentialIndex=%d]",
                         commandPath.mEndpointId, to_underlying(credentialType), credentialIndex);
            emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_FAILURE);
            return;
        }
    }

    uint16_t nextCredentialIndex = 0;

    CHIP_ERROR err                = CHIP_NO_ERROR;
    using ResponseFields          = Commands::GetCredentialStatusResponse::Fields;
    app::ConcreteCommandPath path = { emberAfCurrentEndpoint(), ::Id, Commands::GetCredentialStatusResponse::Id };
    TLV::TLVWriter * writer       = nullptr;
    SuccessOrExit(err = commandObj->PrepareCommand(path));
    VerifyOrExit((writer = commandObj->GetCommandDataIBTLVWriter()) != nullptr, err = CHIP_ERROR_INCORRECT_STATE);
    SuccessOrExit(err = writer->Put(TLV::ContextTag(to_underlying(ResponseFields::kCredentialExists)), credentialExists));
    if (0 != userIndexWithCredential)
    {
        SuccessOrExit(err = writer->Put(TLV::ContextTag(to_underlying(ResponseFields::kUserIndex)), userIndexWithCredential));
    }
    if (findUnoccupiedCredentialSlot(commandPath.mEndpointId, credentialType, static_cast<uint16_t>(credentialIndex + 1),
                                     nextCredentialIndex))
    {
        SuccessOrExit(err = writer->Put(TLV::ContextTag(to_underlying(ResponseFields::kNextCredentialIndex)), nextCredentialIndex));
    }
    SuccessOrExit(err = commandObj->FinishCommand());

    emberAfDoorLockClusterPrintln("[GetCredentialStatus] Prepared credential status "
                                  "[endpointId=%d,credentialType=%u,credentialIndex=%d,userIndex=%d,nextCredentialIndex=%d]",
                                  commandPath.mEndpointId, to_underlying(credentialType), credentialIndex, userIndexWithCredential,
                                  nextCredentialIndex);

exit:
    if (CHIP_NO_ERROR != err)
    {
        ChipLogError(Zcl,
                     "[GetCredentialStatus] Error occurred when preparing response: %s "
                     "[endpointId=%d,credentialType=%u,credentialIndex=%d,userIndex=%d,nextCredentialIndex=%d]",
                     err.AsString(), commandPath.mEndpointId, to_underlying(credentialType), credentialIndex,
                     userIndexWithCredential, nextCredentialIndex);
        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_FAILURE);
    }
}

void DoorLockServer::ClearCredentialCommandHandler(
    chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
    const chip::app::Clusters::DoorLock::Commands::ClearCredential::DecodableType & commandData)
{
    emberAfDoorLockClusterPrintln("[ClearCredential] Incoming command [endpointId=%d]", commandPath.mEndpointId);

    auto modifier = getFabricIndex(commandObj);
    if (kUndefinedFabricIndex == modifier)
    {
        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_FAILURE);
        return;
    }

    auto sourceNodeId = getNodeId(commandObj);
    if (chip::kUndefinedNodeId == sourceNodeId)
    {
        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_FAILURE);
        return;
    }

    const auto & credential = commandData.credential;
    if (credential.IsNull())
    {
        emberAfDoorLockClusterPrintln("[ClearCredential] Clearing all credentials [endpointId=%d]", commandPath.mEndpointId);
        emberAfSendImmediateDefaultResponse(clearCredentials(commandPath.mEndpointId, modifier, sourceNodeId));
        return;
    }

    // Remove all the credentials of the particular type.
    auto credentialType  = credential.Value().credentialType;
    auto credentialIndex = credential.Value().credentialIndex;
    if (0xFFFE == credentialIndex)
    {
        emberAfSendImmediateDefaultResponse(clearCredentials(commandPath.mEndpointId, modifier, sourceNodeId, credentialType));
        return;
    }

    emberAfSendImmediateDefaultResponse(
        clearCredential(commandPath.mEndpointId, modifier, sourceNodeId, credentialType, credentialIndex, false));
}

void DoorLockServer::SetWeekDayScheduleCommandHandler(
    chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
    const chip::app::Clusters::DoorLock::Commands::SetWeekDaySchedule::DecodableType & commandData)
{
    auto endpointId = commandPath.mEndpointId;
    if (!SupportsSchedules(endpointId))
    {
        emberAfDoorLockClusterPrintln("[SetWeekDaySchedule] Ignore command (not supported) [endpointId=%d]", endpointId);
        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_INVALID_COMMAND);
        return;
    }

    emberAfDoorLockClusterPrintln("[SetWeekDaySchedule] Incoming command [endpointId=%d]", endpointId);

    auto fabricIdx = getFabricIndex(commandObj);
    if (kUndefinedFabricIndex == fabricIdx)
    {
        ChipLogError(Zcl, "[SetWeekDaySchedule] Unable to get the fabric IDX [endpointId=%d]", commandPath.mEndpointId);
        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_FAILURE);
        return;
    }

    auto sourceNodeId = getNodeId(commandObj);
    if (chip::kUndefinedNodeId == sourceNodeId)
    {
        ChipLogError(Zcl, "[SetWeekDaySchedule] Unable to get the source node index [endpointId=%d]", commandPath.mEndpointId);
        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_FAILURE);
        return;
    }

    auto weekDayIndex     = commandData.weekDayIndex;
    auto userIndex        = commandData.userIndex;
    const auto & daysMask = commandData.daysMask;
    auto startHour        = commandData.startHour;
    auto startMinute      = commandData.startMinute;
    auto endHour          = commandData.endHour;
    auto endMinute        = commandData.endMinute;

    if (!weekDayIndexValid(endpointId, weekDayIndex) || !userIndexValid(endpointId, userIndex))
    {
        emberAfDoorLockClusterPrintln(
            "[SetWeekDaySchedule] Unable to add schedule - index out of range [endpointId=%d,weekDayIndex=%d,userIndex=%d]",
            endpointId, weekDayIndex, userIndex);
        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_INVALID_FIELD);
        return;
    }

    if (!userExists(endpointId, userIndex))
    {
        emberAfDoorLockClusterPrintln("[SetWeekDaySchedule] Unable to add schedule - user does not exist "
                                      "[endpointId=%d,weekDayIndex=%d,userIndex=%d]",
                                      endpointId, weekDayIndex, userIndex);
        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_NOT_FOUND);
        return;
    }

    // appclusters, 5.2.4.14 - spec does not allow setting the schedule for multiple days in the bitmask
    int setBitsInDaysMask = 0;
    uint8_t rawDaysMask   = daysMask.Raw();
    for (size_t i = 0; i < sizeof(rawDaysMask) * 8; ++i)
    {
        setBitsInDaysMask += rawDaysMask & 0x1;
        rawDaysMask = static_cast<uint8_t>(rawDaysMask >> 1);
    }

    if (setBitsInDaysMask == 0 || setBitsInDaysMask > 1)
    {
        emberAfDoorLockClusterPrintln("[SetWeekDaySchedule] Unable to add schedule - daysMask is out of range "
                                      "[endpointId=%d,weekDayIndex=%d,userIndex=%d,daysMask=%x]",
                                      endpointId, weekDayIndex, userIndex, daysMask.Raw());
        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_INVALID_FIELD);
        return;
    }

    if (startHour > DOOR_LOCK_SCHEDULE_MAX_HOUR || startMinute > DOOR_LOCK_SCHEDULE_MAX_MINUTE ||
        endHour > DOOR_LOCK_SCHEDULE_MAX_HOUR || endMinute > DOOR_LOCK_SCHEDULE_MAX_MINUTE)
    {
        emberAfDoorLockClusterPrintln("[SetWeekDaySchedule] Unable to add schedule - start time out of range "
                                      "[endpointId=%d,weekDayIndex=%d,userIndex=%d,startTime=\"%d:%d\",endTime=\"%d:%d\"]",
                                      endpointId, weekDayIndex, userIndex, startHour, startMinute, endHour, endMinute);
        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_INVALID_FIELD);
        return;
    }

    if (startHour > endHour || (startHour == endHour && startMinute >= endMinute))
    {
        emberAfDoorLockClusterPrintln("[SetWeekDaySchedule] Unable to add schedule - invalid time "
                                      "[endpointId=%d,weekDayIndex=%d,userIndex=%d,startTime=\"%d:%d\",endTime=\"%d:%d\"]",
                                      endpointId, weekDayIndex, userIndex, startHour, startMinute, endHour, endMinute);
        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_INVALID_FIELD);
        return;
    }

    auto status = emberAfPluginDoorLockSetSchedule(endpointId, weekDayIndex, userIndex, DlScheduleStatus::kOccupied, daysMask,
                                                   startHour, startMinute, endHour, endMinute);
    if (DlStatus::kSuccess != status)
    {
        ChipLogError(Zcl,
                     "[SetWeekDaySchedule] Unable to add schedule - internal error "
                     "[endpointId=%d,weekDayIndex=%d,userIndex=%d,status=%u]",
                     endpointId, weekDayIndex, userIndex, to_underlying(status));
        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_FAILURE);
        return;
    }

    emberAfDoorLockClusterPrintln("[SetWeekDaySchedule] Successfully created new schedule "
                                  "[endpointId=%d,weekDayIndex=%d,userIndex=%d,daysMask=%d,startTime=\"%d:%d\",endTime=\"%d:%d\"]",
                                  endpointId, weekDayIndex, userIndex, daysMask.Raw(), startHour, startMinute, endHour, endMinute);

    sendRemoteLockUserChange(endpointId, DlLockDataType::kWeekDaySchedule, DlDataOperationType::kAdd, sourceNodeId, fabricIdx,
                             userIndex, static_cast<uint16_t>(weekDayIndex));

    emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_SUCCESS);
}

void DoorLockServer::GetWeekDayScheduleCommandHandler(
    chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
    const chip::app::Clusters::DoorLock::Commands::GetWeekDaySchedule::DecodableType & commandData)
{
    auto endpointId = commandPath.mEndpointId;
    if (!SupportsSchedules(endpointId))
    {
        emberAfDoorLockClusterPrintln("[GetWeekDaySchedule] Ignore command (not supported) [endpointId=%d]", endpointId);
        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_INVALID_COMMAND);
        return;
    }

    emberAfDoorLockClusterPrintln("[GetWeekDaySchedule] Incoming command [endpointId=%d]", endpointId);

    auto weekDayIndex = commandData.weekDayIndex;
    auto userIndex    = commandData.userIndex;

    if (!weekDayIndexValid(endpointId, weekDayIndex) || !userIndexValid(endpointId, userIndex))
    {
        emberAfDoorLockClusterPrintln(
            "[GetWeekDaySchedule] Unable to get schedule - index out of range [endpointId=%d,weekDayIndex=%d,userIndex=%d]",
            endpointId, weekDayIndex, userIndex);
        sendGetWeekDayScheduleResponse(commandObj, commandPath, weekDayIndex, userIndex, DlStatus::kInvalidField);
        return;
    }

    if (!userExists(endpointId, userIndex))
    {
        emberAfDoorLockClusterPrintln("[GetWeekDaySchedule] User does not exist [endpointId=%d,weekDayIndex=%d,userIndex=%d]",
                                      endpointId, weekDayIndex, userIndex);
        sendGetWeekDayScheduleResponse(commandObj, commandPath, weekDayIndex, userIndex, DlStatus::kNotFound);
        return;
    }

    EmberAfPluginDoorLockWeekDaySchedule scheduleInfo;
    auto status = emberAfPluginDoorLockGetSchedule(endpointId, weekDayIndex, userIndex, scheduleInfo);
    if (DlStatus::kSuccess != status)
    {
        sendGetWeekDayScheduleResponse(commandObj, commandPath, weekDayIndex, userIndex, status);
        return;
    }

    sendGetWeekDayScheduleResponse(commandObj, commandPath, weekDayIndex, userIndex, DlStatus::kSuccess, scheduleInfo.daysMask,
                                   scheduleInfo.startHour, scheduleInfo.startMinute, scheduleInfo.endHour, scheduleInfo.endMinute);
}

void DoorLockServer::ClearWeekDayScheduleCommandHandler(
    chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
    const chip::app::Clusters::DoorLock::Commands::ClearWeekDaySchedule::DecodableType & commandData)
{
    auto endpointId = commandPath.mEndpointId;
    if (!SupportsSchedules(endpointId))
    {
        emberAfDoorLockClusterPrintln("[ClearWeekDaySchedule] Ignore command (not supported) [endpointId=%d]", endpointId);
        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_INVALID_COMMAND);
        return;
    }

    emberAfDoorLockClusterPrintln("[ClearWeekDaySchedule] Incoming command [endpointId=%d]", endpointId);

    auto fabricIdx = getFabricIndex(commandObj);
    if (kUndefinedFabricIndex == fabricIdx)
    {
        ChipLogError(Zcl, "[ClearWeekDaySchedule] Unable to get the fabric IDX [endpointId=%d]", commandPath.mEndpointId);
        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_FAILURE);
        return;
    }

    auto sourceNodeId = getNodeId(commandObj);
    if (chip::kUndefinedNodeId == sourceNodeId)
    {
        ChipLogError(Zcl, "[ClearWeekDaySchedule] Unable to get the source node index [endpointId=%d]", commandPath.mEndpointId);
        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_FAILURE);
        return;
    }

    auto weekDayIndex = commandData.weekDayIndex;
    auto userIndex    = commandData.userIndex;

    if (!userIndexValid(endpointId, userIndex) || (!weekDayIndexValid(endpointId, weekDayIndex) && 0xFE != weekDayIndex))
    {
        emberAfDoorLockClusterPrintln(
            "[ClearWeekDaySchedule] User or WeekDay index is out of range [endpointId=%d,weekDayIndex=%d,userIndex=%d]", endpointId,
            weekDayIndex, userIndex);
        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_INVALID_COMMAND);
        return;
    }

    if (!userExists(endpointId, userIndex))
    {
        emberAfDoorLockClusterPrintln("[ClearWeekDaySchedule] User does not exist [endpointId=%d,weekDayIndex=%d,userIndex=%d]",
                                      endpointId, weekDayIndex, userIndex);
        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_NOT_FOUND);
        return;
    }

    DlStatus clearStatus = DlStatus::kSuccess;
    if (0xFE == weekDayIndex)
    {
        emberAfDoorLockClusterPrintln(
            "[ClearWeekDaySchedule] Clearing all schedules for a single user [endpointId=%d,userIndex=%d]", endpointId, userIndex);
        clearStatus = clearWeekDaySchedules(endpointId, userIndex);
    }
    else
    {
        emberAfDoorLockClusterPrintln(
            "[ClearWeekDaySchedule] Clearing a single schedule [endpointId=%d,weekDayIndex=%d,userIndex=%d]", endpointId,
            weekDayIndex, userIndex);
        clearStatus = clearWeekDaySchedule(endpointId, userIndex, weekDayIndex);
    }

    if (DlStatus::kSuccess != clearStatus)
    {
        emberAfDoorLockClusterPrintln(
            "[ClearWeekDaySchedule] Unable to clear the user schedules - app error [endpointId=%d,userIndex=%d,status=%u]",
            endpointId, userIndex, to_underlying(clearStatus));
        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_FAILURE);
        return;
    }

    sendRemoteLockUserChange(endpointId, DlLockDataType::kWeekDaySchedule, DlDataOperationType::kClear, sourceNodeId, fabricIdx,
                             userIndex, static_cast<uint16_t>(weekDayIndex));

    emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_SUCCESS);
}

void DoorLockServer::SetYearDayScheduleCommandHandler(
    chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
    const chip::app::Clusters::DoorLock::Commands::SetYearDaySchedule::DecodableType & commandData)
{
    auto endpointId = commandPath.mEndpointId;
    if (!SupportsSchedules(endpointId))
    {
        emberAfDoorLockClusterPrintln("[SetYearDaySchedule] Ignore command (not supported) [endpointId=%d]", endpointId);
        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_INVALID_COMMAND);
        return;
    }

    emberAfDoorLockClusterPrintln("[SetYearDaySchedule] incoming command [endpointId=%d]", endpointId);

    auto fabricIdx = getFabricIndex(commandObj);
    if (kUndefinedFabricIndex == fabricIdx)
    {
        ChipLogError(Zcl, "[SetYearDaySchedule] Unable to get the fabric IDX [endpointId=%d]", commandPath.mEndpointId);
        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_FAILURE);
        return;
    }

    auto sourceNodeId = getNodeId(commandObj);
    if (chip::kUndefinedNodeId == sourceNodeId)
    {
        ChipLogError(Zcl, "[SetYearDaySchedule] Unable to get the source node index [endpointId=%d]", commandPath.mEndpointId);
        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_FAILURE);
        return;
    }

    auto yearDayIndex  = commandData.yearDayIndex;
    auto userIndex     = commandData.userIndex;
    auto localStarTime = commandData.localStartTime;
    auto localEndTime  = commandData.localEndTime;

    if (!yearDayIndexValid(endpointId, yearDayIndex) || !userIndexValid(endpointId, userIndex))
    {
        emberAfDoorLockClusterPrintln(
            "[SetYearDaySchedule] Unable to add schedule - index out of range [endpointId=%d,yearDayIndex=%d,userIndex=%d]",
            endpointId, yearDayIndex, userIndex);
        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_INVALID_FIELD);
        return;
    }

    if (!userExists(endpointId, userIndex))
    {
        emberAfDoorLockClusterPrintln("[SetYearDaySchedule] Unable to add schedule - user does not exist "
                                      "[endpointId=%d,yearDayIndex=%d,userIndex=%d]",
                                      endpointId, yearDayIndex, userIndex);
        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_NOT_FOUND);
        return;
    }

    if (localEndTime <= localStarTime)
    {
        emberAfDoorLockClusterPrintln("[SetYearDaySchedule] Unable to add schedule - schedule ends earlier than starts"
                                      "[endpointId=%d,yearDayIndex=%d,userIndex=%d,localStarTime=%" PRIu32 ",localEndTime=%" PRIu32
                                      "]",
                                      endpointId, yearDayIndex, userIndex, localStarTime, localEndTime);
        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_INVALID_FIELD);
        return;
    }

    auto status = emberAfPluginDoorLockSetSchedule(endpointId, yearDayIndex, userIndex, DlScheduleStatus::kOccupied, localStarTime,
                                                   localEndTime);
    if (DlStatus::kSuccess != status)
    {
        ChipLogError(Zcl,
                     "[SetYearDaySchedule] Unable to add schedule - internal error "
                     "[endpointId=%d,yearDayIndex=%d,userIndex=%d,status=%u]",
                     endpointId, yearDayIndex, userIndex, to_underlying(status));
        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_FAILURE);
        return;
    }

    emberAfDoorLockClusterPrintln("[SetYearDaySchedule] Successfully created new schedule "
                                  "[endpointId=%d,yearDayIndex=%d,userIndex=%d,localStartTime=%" PRIu32 ",endTime=%" PRIu32 "]",
                                  endpointId, yearDayIndex, userIndex, localStarTime, localEndTime);

    sendRemoteLockUserChange(endpointId, DlLockDataType::kYearDaySchedule, DlDataOperationType::kAdd, sourceNodeId, fabricIdx,
                             userIndex, static_cast<uint16_t>(yearDayIndex));

    emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_SUCCESS);
}

void DoorLockServer::GetYearDayScheduleCommandHandler(
    chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
    const chip::app::Clusters::DoorLock::Commands::GetYearDaySchedule::DecodableType & commandData)
{
    auto endpointId = commandPath.mEndpointId;
    if (!SupportsSchedules(endpointId))
    {
        emberAfDoorLockClusterPrintln("[GetYearDaySchedule] Ignore command (not supported) [endpointId=%d]", endpointId);
        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_INVALID_COMMAND);
        return;
    }
    emberAfDoorLockClusterPrintln("[GetYearDaySchedule] incoming command [endpointId=%d]", endpointId);

    auto yearDayIndex = commandData.yearDayIndex;
    auto userIndex    = commandData.userIndex;

    if (!yearDayIndexValid(endpointId, yearDayIndex) || !userIndexValid(endpointId, userIndex))
    {
        emberAfDoorLockClusterPrintln(
            "[GetYearDaySchedule] Unable to get schedule - index out of range [endpointId=%d,yearDayIndex=%d,userIndex=%d]",
            endpointId, yearDayIndex, userIndex);
        sendGetYearDayScheduleResponse(commandObj, commandPath, yearDayIndex, userIndex, DlStatus::kInvalidField);
        return;
    }

    if (!userExists(endpointId, userIndex))
    {
        emberAfDoorLockClusterPrintln("[GetYearDaySchedule] User does not exist [endpointId=%d,yearDayIndex=%d,userIndex=%d]",
                                      endpointId, yearDayIndex, userIndex);
        sendGetYearDayScheduleResponse(commandObj, commandPath, yearDayIndex, userIndex, DlStatus::kNotFound);
        return;
    }

    EmberAfPluginDoorLockYearDaySchedule scheduleInfo;
    auto status = emberAfPluginDoorLockGetSchedule(endpointId, yearDayIndex, userIndex, scheduleInfo);
    if (DlStatus::kSuccess != status)
    {
        sendGetYearDayScheduleResponse(commandObj, commandPath, yearDayIndex, userIndex, status);
        return;
    }

    sendGetYearDayScheduleResponse(commandObj, commandPath, yearDayIndex, userIndex, DlStatus::kSuccess,
                                   scheduleInfo.localStartTime, scheduleInfo.localEndTime);
}

void DoorLockServer::ClearYearDayScheduleCommandHandler(
    chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
    const chip::app::Clusters::DoorLock::Commands::ClearYearDaySchedule::DecodableType & commandData)
{
    auto endpointId = commandPath.mEndpointId;
    if (!SupportsSchedules(endpointId))
    {
        emberAfDoorLockClusterPrintln("[ClearYearDaySchedule] Ignore command (not supported) [endpointId=%d]", endpointId);
        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_INVALID_COMMAND);
        return;
    }
    emberAfDoorLockClusterPrintln("[ClearYearDaySchedule] incoming command [endpointId=%d]", endpointId);

    auto fabricIdx = getFabricIndex(commandObj);
    if (kUndefinedFabricIndex == fabricIdx)
    {
        ChipLogError(Zcl, "[ClearYearDaySchedule] Unable to get the fabric IDX [endpointId=%d]", commandPath.mEndpointId);
        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_FAILURE);
        return;
    }

    auto sourceNodeId = getNodeId(commandObj);
    if (chip::kUndefinedNodeId == sourceNodeId)
    {
        ChipLogError(Zcl, "[ClearYearDaySchedule] Unable to get the source node index [endpointId=%d]", commandPath.mEndpointId);
        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_FAILURE);
        return;
    }

    auto yearDayIndex = commandData.yearDayIndex;
    auto userIndex    = commandData.userIndex;

    if (!userIndexValid(endpointId, userIndex) || (!yearDayIndexValid(endpointId, yearDayIndex) && 0xFE != yearDayIndex))
    {
        emberAfDoorLockClusterPrintln(
            "[ClearYearDaySchedule] User or YearDay index is out of range [endpointId=%d,yearDayIndex=%d,userIndex=%d]", endpointId,
            yearDayIndex, userIndex);
        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_INVALID_COMMAND);
        return;
    }

    if (!userExists(endpointId, userIndex))
    {
        emberAfDoorLockClusterPrintln("[ClearYearDaySchedule] User does not exist [endpointId=%d,yearDayIndex=%d,userIndex=%d]",
                                      endpointId, yearDayIndex, userIndex);
        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_NOT_FOUND);
        return;
    }

    DlStatus clearStatus = DlStatus::kSuccess;
    if (0xFE == yearDayIndex)
    {
        emberAfDoorLockClusterPrintln(
            "[ClearYearDaySchedule] Clearing all schedules for a single user [endpointId=%d,userIndex=%d]", endpointId, userIndex);
        clearStatus = clearYearDaySchedules(endpointId, userIndex);
    }
    else
    {
        emberAfDoorLockClusterPrintln(
            "[ClearYearDaySchedule] Clearing a single schedule [endpointId=%d,yearDayIndex=%d,userIndex=%d]", endpointId,
            yearDayIndex, userIndex);
        clearStatus = clearYearDaySchedule(endpointId, userIndex, yearDayIndex);
    }

    if (DlStatus::kSuccess != clearStatus)
    {
        emberAfDoorLockClusterPrintln(
            "[ClearYearDaySchedule] Unable to clear the user schedules - app error [endpointId=%d,userIndex=%d,status=%u]",
            endpointId, userIndex, to_underlying(clearStatus));
        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_FAILURE);
        return;
    }

    sendRemoteLockUserChange(endpointId, DlLockDataType::kYearDaySchedule, DlDataOperationType::kClear, sourceNodeId, fabricIdx,
                             userIndex, static_cast<uint16_t>(yearDayIndex));

    emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_SUCCESS);
}

bool DoorLockServer::HasFeature(chip::EndpointId endpointId, DoorLockFeature feature)
{
    uint32_t featureMap = 0;
    bool success        = GetAttribute(endpointId, Attributes::FeatureMap::Id, Attributes::FeatureMap::Get, featureMap);

    return success ? ((featureMap & to_underlying(feature)) != 0) : false;
}

/**********************************************************
 * DoorLockServer private methods
 *********************************************************/

chip::FabricIndex DoorLockServer::getFabricIndex(const chip::app::CommandHandler * commandObj)
{
    if (nullptr == commandObj || nullptr == commandObj->GetExchangeContext())
    {
        ChipLogError(Zcl, "Cannot access ExchangeContext of Command Object for Fabric Index");
        return kUndefinedFabricIndex;
    }

    return commandObj->GetAccessingFabricIndex();
}

chip::NodeId DoorLockServer::getNodeId(const chip::app::CommandHandler * commandObj)
{
    if (nullptr == commandObj || nullptr == commandObj->GetExchangeContext())
    {
        ChipLogError(Zcl, "Cannot access ExchangeContext of Command Object for Node ID");
        return kUndefinedNodeId;
    }

    auto secureSession = commandObj->GetExchangeContext()->GetSessionHandle()->AsSecureSession();
    if (nullptr == secureSession)
    {
        ChipLogError(Zcl, "Cannot access Secure session handle of Command Object for Node ID");
    }
    return secureSession->GetPeerNodeId();
}

bool DoorLockServer::userIndexValid(chip::EndpointId endpointId, uint16_t userIndex)
{
    uint16_t maxNumberOfUsers;
    return userIndexValid(endpointId, userIndex, maxNumberOfUsers);
}

bool DoorLockServer::userIndexValid(chip::EndpointId endpointId, uint16_t userIndex, uint16_t & maxNumberOfUser)
{
    if (!GetNumberOfUserSupported(endpointId, maxNumberOfUser))
    {
        return false;
    }

    // appclusters, 5.2.4.34-37: user index changes from 1 to maxNumberOfUsers
    if (0 == userIndex || userIndex > maxNumberOfUser)
    {
        return false;
    }
    return true;
}

bool DoorLockServer::userExists(chip::EndpointId endpointId, uint16_t userIndex)
{
    // Check if user actually exist
    EmberAfPluginDoorLockUserInfo user;
    if (!emberAfPluginDoorLockGetUser(endpointId, userIndex, user))
    {
        ChipLogError(Zcl, "[UserExists] Unable to get the user - internal error [endpointId=%d,userIndex=%d]", endpointId,
                     userIndex);
        return false;
    }
    return DlUserStatus::kAvailable != user.userStatus;
}

bool DoorLockServer::credentialIndexValid(chip::EndpointId endpointId, DlCredentialType type, uint16_t credentialIndex)
{
    uint16_t maxCredentials = 0;
    return credentialIndexValid(endpointId, type, credentialIndex, maxCredentials);
}

bool DoorLockServer::credentialIndexValid(chip::EndpointId endpointId, DlCredentialType type, uint16_t credentialIndex,
                                          uint16_t & maxNumberOfCredentials)
{
    if (!getMaxNumberOfCredentials(endpointId, type, maxNumberOfCredentials))
    {
        return false;
    }

    // appclusters, 5.2.6.3.1: 0 is allowed index for Programming PIN credential only
    if (DlCredentialType::kProgrammingPIN == type)
    {
        return (0 == credentialIndex);
    }

    if (0 == credentialIndex || credentialIndex > maxNumberOfCredentials)
    {
        return false;
    }

    return true;
}

bool DoorLockServer::getCredentialRange(chip::EndpointId endpointId, DlCredentialType type, size_t & minSize, size_t & maxSize)
{
    bool statusMin = true, statusMax = true;
    uint8_t minLen, maxLen;

    switch (type)
    {
    case DlCredentialType::kProgrammingPIN:
    case DlCredentialType::kPin:
        statusMin = GetAttribute(endpointId, Attributes::MinPINCodeLength::Id, Attributes::MinPINCodeLength::Get, minLen);
        statusMax = GetAttribute(endpointId, Attributes::MaxPINCodeLength::Id, Attributes::MaxPINCodeLength::Get, maxLen);
        break;
    case DlCredentialType::kRfid:
        statusMin = GetAttribute(endpointId, Attributes::MinRFIDCodeLength::Id, Attributes::MinRFIDCodeLength::Get, minLen);
        statusMax = GetAttribute(endpointId, Attributes::MaxRFIDCodeLength::Id, Attributes::MaxRFIDCodeLength::Get, maxLen);
        break;
    default:
        return false;
    }

    if (!statusMin || !statusMax)
    {
        ChipLogError(Zcl, "Unable to read attributes to get min/max length for credentials [endpointId=%d,credentialType=%u]",
                     endpointId, to_underlying(type));
        return false;
    }

    minSize = minLen;
    maxSize = maxLen;

    return true;
}

bool DoorLockServer::getMaxNumberOfCredentials(chip::EndpointId endpointId, DlCredentialType credentialType,
                                               uint16_t & maxNumberOfCredentials)
{
    maxNumberOfCredentials = 0;
    bool status            = false;
    switch (credentialType)
    {
    case DlCredentialType::kProgrammingPIN:
        maxNumberOfCredentials = 1;
        return true;
    case DlCredentialType::kPin:
        status = GetNumberOfPINCredentialsSupported(endpointId, maxNumberOfCredentials);
        break;
    case DlCredentialType::kRfid:
        status = GetNumberOfRFIDCredentialsSupported(endpointId, maxNumberOfCredentials);
        break;
    default:
        return false;
    }

    return status;
}

bool DoorLockServer::findUnoccupiedUserSlot(chip::EndpointId endpointId, uint16_t & userIndex)
{
    return findUnoccupiedUserSlot(endpointId, 1, userIndex);
}

bool DoorLockServer::findUnoccupiedUserSlot(chip::EndpointId endpointId, uint16_t startIndex, uint16_t & userIndex)
{
    uint16_t maxNumberOfUsers;
    VerifyOrReturnError(GetAttribute(endpointId, Attributes::NumberOfTotalUsersSupported::Id,
                                     Attributes::NumberOfTotalUsersSupported::Get, maxNumberOfUsers),
                        false);

    userIndex = 0;
    for (uint16_t i = startIndex; i <= maxNumberOfUsers; ++i)
    {
        EmberAfPluginDoorLockUserInfo user;
        if (!emberAfPluginDoorLockGetUser(endpointId, i, user))
        {
            ChipLogError(Zcl, "Unable to get user to check if slot is occupied: app error [userIndex=%d]", i);
            return false;
        }

        if (DlUserStatus::kAvailable == user.userStatus)
        {
            userIndex = i;
            return true;
        }
    }
    return false;
}

bool DoorLockServer::findUnoccupiedCredentialSlot(chip::EndpointId endpointId, DlCredentialType credentialType, uint16_t startIndex,
                                                  uint16_t & credentialIndex)
{
    uint16_t maxNumberOfCredentials = 0;
    if (!getMaxNumberOfCredentials(endpointId, credentialType, maxNumberOfCredentials))
    {
        return false;
    }

    // Programming PIN index starts with 0, and it is assumed that it is unique. Therefor different bounds checking for that
    // credential type
    if (DlCredentialType::kProgrammingPIN == credentialType)
    {
        maxNumberOfCredentials--;
    }

    for (uint16_t i = startIndex; i <= maxNumberOfCredentials; ++i)
    {
        EmberAfPluginDoorLockCredentialInfo info;
        if (!emberAfPluginDoorLockGetCredential(endpointId, i, credentialType, info))
        {
            ChipLogError(Zcl, "Unable to get credential: app error [endpointId=%d,credentialType=%u,credentialIndex=%d]",
                         endpointId, to_underlying(credentialType), i);
            return false;
        }

        if (DlCredentialStatus::kAvailable == info.status)
        {
            credentialIndex = i;
            return true;
        }
    }

    return false;
}

bool DoorLockServer::findUserIndexByCredential(chip::EndpointId endpointId, DlCredentialType credentialType,
                                               uint16_t credentialIndex, uint16_t & userIndex)
{
    uint16_t maxNumberOfUsers = 0;
    VerifyOrReturnError(GetAttribute(endpointId, Attributes::NumberOfTotalUsersSupported::Id,
                                     Attributes::NumberOfTotalUsersSupported::Get, maxNumberOfUsers),
                        false);

    for (uint16_t i = 1; i <= maxNumberOfUsers; ++i)
    {
        EmberAfPluginDoorLockUserInfo user;
        if (!emberAfPluginDoorLockGetUser(endpointId, i, user))
        {
            ChipLogError(Zcl, "[GetCredentialStatus] Unable to get user: app error [userIndex=%d]", i);
            emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_FAILURE);
            return false;
        }

        // Go through occupied users only
        if (DlUserStatus::kAvailable == user.userStatus)
        {
            continue;
        }

        for (uint16_t j = 0; j < user.credentials.size(); ++j)
        {
            if (user.credentials.data()[j].CredentialIndex == credentialIndex &&
                user.credentials.data()[j].CredentialType == to_underlying(credentialType))
            {
                userIndex = i;
                return true;
            }
        }
    }

    return false;
}

bool DoorLockServer::findUserIndexByCredential(chip::EndpointId endpointId, DlCredentialType credentialType,
                                               chip::ByteSpan credentialData, uint16_t & userIndex, uint16_t & credentialIndex)
{
    uint16_t maxNumberOfUsers = 0;
    VerifyOrReturnError(GetAttribute(endpointId, Attributes::NumberOfTotalUsersSupported::Id,
                                     Attributes::NumberOfTotalUsersSupported::Get, maxNumberOfUsers),
                        false);

    for (uint16_t i = 1; i <= maxNumberOfUsers; ++i)
    {
        EmberAfPluginDoorLockUserInfo user;
        if (!emberAfPluginDoorLockGetUser(endpointId, i, user))
        {
            ChipLogError(Zcl, "[findUserIndexByCredential] Unable to get user: app error [userIndex=%d]", i);
            emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_FAILURE);
            return false;
        }

        // Go through occupied users only
        if (DlUserStatus::kAvailable == user.userStatus)
        {
            continue;
        }

        for (const auto & credential : user.credentials)
        {
            if (credential.CredentialType != to_underlying(credentialType))
            {
                continue;
            }

            EmberAfPluginDoorLockCredentialInfo credentialInfo;
            if (!emberAfPluginDoorLockGetCredential(endpointId, credential.CredentialIndex, credentialType, credentialInfo))
            {
                ChipLogError(Zcl,
                             "[findUserIndexByCredential] Unable to get credential: app error "
                             "[userIndex=%d,credentialIndex=%d,credentialType=%u]",
                             i, credential.CredentialIndex, to_underlying(credentialType));
                emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_FAILURE);
                return false;
            }

            if (credentialInfo.status != DlCredentialStatus::kOccupied)
            {
                ChipLogError(Zcl,
                             "[findUserIndexByCredential] Users/Credentials database error: credential index attached to user is "
                             "not occupied "
                             "[userIndex=%d,credentialIndex=%d,credentialType=%u]",
                             i, credential.CredentialIndex, to_underlying(credentialType));
                emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_FAILURE);
                return false;
            }

            if (credentialInfo.credentialData.data_equal(credentialData))
            {
                userIndex       = i;
                credentialIndex = i;
                return true;
            }
        }
    }

    return false;
}

EmberAfStatus DoorLockServer::createUser(chip::EndpointId endpointId, chip::FabricIndex creatorFabricIdx, chip::NodeId sourceNodeId,
                                         uint16_t userIndex, const Nullable<chip::CharSpan> & userName,
                                         const Nullable<uint32_t> & userUniqueId, const Nullable<DlUserStatus> & userStatus,
                                         const Nullable<DlUserType> & userType, const Nullable<DlCredentialRule> & credentialRule,
                                         const Nullable<DlCredential> & credential)
{
    EmberAfPluginDoorLockUserInfo user;
    if (!emberAfPluginDoorLockGetUser(endpointId, userIndex, user))
    {
        ChipLogError(Zcl, "[createUser] Unable to get the user from app [endpointId=%d,userIndex=%d]", endpointId, userIndex);
        return EMBER_ZCL_STATUS_FAILURE;
    }

    // appclusters, 5.2.4.34: to modify user its status should be set to Available. If it is we should return OCCUPIED.
    if (DlUserStatus::kAvailable != user.userStatus)
    {
        emberAfDoorLockClusterPrintln("[createUser] Unable to overwrite existing user [endpointId=%d,userIndex=%d]", endpointId,
                                      userIndex);
        // TODO: Add cluster-specific failure of DlStatus::kOccupied
        return EMBER_ZCL_STATUS_FAILURE;
    }

    const auto & newUserName            = !userName.IsNull() ? userName.Value() : chip::CharSpan("");
    auto newUserUniqueId                = userUniqueId.IsNull() ? 0xFFFFFFFF : userUniqueId.Value();
    auto newUserStatus                  = userStatus.IsNull() ? DlUserStatus::kOccupiedEnabled : userStatus.Value();
    auto newUserType                    = userType.IsNull() ? DlUserType::kUnrestrictedUser : userType.Value();
    auto newCredentialRule              = credentialRule.IsNull() ? DlCredentialRule::kSingle : credentialRule.Value();
    const DlCredential * newCredentials = nullptr;
    size_t newTotalCredentials          = 0;
    if (!credential.IsNull())
    {
        newCredentials      = &credential.Value();
        newTotalCredentials = 1;
    }

    if (!emberAfPluginDoorLockSetUser(endpointId, userIndex, creatorFabricIdx, creatorFabricIdx, newUserName, newUserUniqueId,
                                      newUserStatus, newUserType, newCredentialRule, newCredentials, newTotalCredentials))
    {
        emberAfDoorLockClusterPrintln(
            "[createUser] Unable to create user: app error "
            "[endpointId=%d,creatorFabricId=%d,userIndex=%d,userName=\"%s\",userUniqueId=0x%" PRIx32 ",userStatus="
            "%u,userType=%u,credentialRule=%u,totalCredentials=%zu]",
            endpointId, creatorFabricIdx, userIndex, newUserName.data(), newUserUniqueId, to_underlying(newUserStatus),
            to_underlying(newUserType), to_underlying(newCredentialRule), newTotalCredentials);
        return EMBER_ZCL_STATUS_FAILURE;
    }

    emberAfDoorLockClusterPrintln(
        "[createUser] User created "
        "[endpointId=%d,creatorFabricId=%d,userIndex=%d,userName=\"%s\",userUniqueId=0x%" PRIx32 ",userStatus=%"
        "u,userType=%u,credentialRule=%u,totalCredentials=%zu]",
        endpointId, creatorFabricIdx, userIndex, newUserName.data(), newUserUniqueId, to_underlying(newUserStatus),
        to_underlying(newUserType), to_underlying(newCredentialRule), newTotalCredentials);

    sendRemoteLockUserChange(endpointId, DlLockDataType::kUserIndex, DlDataOperationType::kAdd, sourceNodeId, creatorFabricIdx,
                             userIndex, userIndex);

    return EMBER_ZCL_STATUS_SUCCESS;
}

EmberAfStatus DoorLockServer::modifyUser(chip::EndpointId endpointId, chip::FabricIndex modifierFabricIndex,
                                         chip::NodeId sourceNodeId, uint16_t userIndex, const Nullable<chip::CharSpan> & userName,
                                         const Nullable<uint32_t> & userUniqueId, const Nullable<DlUserStatus> & userStatus,
                                         const Nullable<DlUserType> & userType, const Nullable<DlCredentialRule> & credentialRule)
{
    // We should get the user by that index first
    EmberAfPluginDoorLockUserInfo user;
    if (!emberAfPluginDoorLockGetUser(endpointId, userIndex, user))
    {
        ChipLogError(Zcl, "[modifyUser] Unable to get the user from app [endpointId=%d,userIndex=%d]", endpointId, userIndex);
        return EMBER_ZCL_STATUS_FAILURE;
    }

    // appclusters, 5.2.4.34: to modify user its status should NOT be set to Available. If it is we should return INVALID_COMMAND.
    if (DlUserStatus::kAvailable == user.userStatus)
    {
        emberAfDoorLockClusterPrintln("[modifyUser] Unable to modify non-existing user [endpointId=%d,userIndex=%d]", endpointId,
                                      userIndex);
        return EMBER_ZCL_STATUS_INVALID_COMMAND;
    }

    // appclusters, 5.2.4.34: UserName SHALL be null if modifying a user record that was not created by the accessing fabric
    if (user.createdBy != modifierFabricIndex && !userName.IsNull())
    {
        emberAfDoorLockClusterPrintln("[modifyUser] Unable to modify name of user created by different fabric "
                                      "[endpointId=%d,userIndex=%d,creatorIdx=%d,modifierIdx=%d]",
                                      endpointId, userIndex, user.createdBy, modifierFabricIndex);
        return EMBER_ZCL_STATUS_INVALID_COMMAND;
    }

    // appclusters, 5.2.4.34: UserUniqueID SHALL be null if modifying the user record that was not created by the accessing fabric.
    if (user.createdBy != modifierFabricIndex && !userUniqueId.IsNull())
    {
        emberAfDoorLockClusterPrintln("[modifyUser] Unable to modify UUID of user created by different fabric "
                                      "[endpointId=%d,userIndex=%d,creatorIdx=%d,modifierIdx=%d]",
                                      endpointId, userIndex, user.createdBy, modifierFabricIndex);
        return EMBER_ZCL_STATUS_INVALID_COMMAND;
    }

    const auto & newUserName = !userName.IsNull() ? userName.Value() : user.userName;
    auto newUserUniqueId     = userUniqueId.IsNull() ? user.userUniqueId : userUniqueId.Value();
    auto newUserStatus       = userStatus.IsNull() ? user.userStatus : userStatus.Value();
    auto newUserType         = userType.IsNull() ? user.userType : userType.Value();
    auto newCredentialRule   = credentialRule.IsNull() ? user.credentialRule : credentialRule.Value();

    if (!emberAfPluginDoorLockSetUser(endpointId, userIndex, user.createdBy, modifierFabricIndex, newUserName, newUserUniqueId,
                                      newUserStatus, newUserType, newCredentialRule, user.credentials.data(),
                                      user.credentials.size()))
    {
        ChipLogError(Zcl,
                     "[modifyUser] Unable to modify the user: app error "
                     "[endpointId=%d,modifierFabric=%d,userIndex=%d,userName=\"%s\",userUniqueId=0x%" PRIx32 ",userStatus=%u"
                     ",userType=%u,credentialRule=%u]",
                     endpointId, modifierFabricIndex, userIndex, newUserName.data(), newUserUniqueId, to_underlying(newUserStatus),
                     to_underlying(newUserType), to_underlying(newCredentialRule));
        return EMBER_ZCL_STATUS_FAILURE;
    }

    emberAfDoorLockClusterPrintln("[modifyUser] User modified "
                                  "[endpointId=%d,modifierFabric=%d,userIndex=%d,userName=\"%s\",userUniqueId=0x%" PRIx32
                                  ",userStatus=%u,"
                                  "userType=%u,credentialRule=%u]",
                                  endpointId, modifierFabricIndex, userIndex, newUserName.data(), newUserUniqueId,
                                  to_underlying(newUserStatus), to_underlying(newUserType), to_underlying(newCredentialRule));

    sendRemoteLockUserChange(endpointId, DlLockDataType::kUserIndex, DlDataOperationType::kModify, sourceNodeId,
                             modifierFabricIndex, userIndex, userIndex);

    return EMBER_ZCL_STATUS_SUCCESS;
}

EmberAfStatus DoorLockServer::clearUser(chip::EndpointId endpointId, chip::FabricIndex modifierFabricId, chip::NodeId sourceNodeId,
                                        uint16_t userIndex, bool sendUserChangeEvent)
{
    EmberAfPluginDoorLockUserInfo user;
    if (!emberAfPluginDoorLockGetUser(endpointId, userIndex, user))
    {
        return EMBER_ZCL_STATUS_FAILURE;
    }

    return clearUser(endpointId, modifierFabricId, sourceNodeId, userIndex, user, sendUserChangeEvent);
}

EmberAfStatus DoorLockServer::clearUser(chip::EndpointId endpointId, chip::FabricIndex modifierFabricId, chip::NodeId sourceNodeId,
                                        uint16_t userIndex, const EmberAfPluginDoorLockUserInfo & user, bool sendUserChangeEvent)
{
    // appclusters, 5.2.4.37: all the credentials associated with user should be cleared when clearing the user
    for (const auto & credential : user.credentials)
    {
        emberAfDoorLockClusterPrintln(
            "[ClearUser] Clearing associated credential [endpointId=%d,userIndex=%d,credentialType=%u,credentialIndex=%d]",
            endpointId, userIndex, credential.CredentialType, credential.CredentialIndex);

        if (!emberAfPluginDoorLockSetCredential(endpointId, credential.CredentialIndex, DlCredentialStatus::kAvailable,
                                                static_cast<DlCredentialType>(credential.CredentialType), chip::ByteSpan()))
        {
            ChipLogError(Zcl,
                         "[ClearUser] Unable to remove credentials associated with user - internal error "
                         "[endpointId=%d,userIndex=%d,credentialIndex=%d,credentialType=%u]",
                         endpointId, userIndex, credential.CredentialIndex, credential.CredentialType);
            return EMBER_ZCL_STATUS_FAILURE;
        }
    }

    // Clear all the user schedules
    auto status = clearSchedules(endpointId, userIndex);
    if (DlStatus::kSuccess != status)
    {
        ChipLogError(Zcl, "[ClearUser] Unable to delete schedules - internal error [endpointId=%d,userIndex=%d]", endpointId,
                     userIndex);
    }

    // Remove the user entry
    if (!emberAfPluginDoorLockSetUser(endpointId, userIndex, kUndefinedFabricIndex, kUndefinedFabricIndex, chip::CharSpan(""), 0,
                                      DlUserStatus::kAvailable, DlUserType::kUnrestrictedUser, DlCredentialRule::kSingle, nullptr,
                                      0))
    {
        return EMBER_ZCL_STATUS_FAILURE;
    }

    if (sendUserChangeEvent)
    {
        sendRemoteLockUserChange(endpointId, DlLockDataType::kUserIndex, DlDataOperationType::kClear, sourceNodeId,
                                 modifierFabricId, userIndex, userIndex);
    }
    return EMBER_ZCL_STATUS_SUCCESS;
}

DlStatus DoorLockServer::createNewCredentialAndUser(chip::EndpointId endpointId, chip::FabricIndex creatorFabricIdx,
                                                    chip::NodeId sourceNodeId, const Nullable<DlUserStatus> & userStatus,
                                                    const Nullable<DlUserType> & userType, const DlCredential & credential,
                                                    const chip::ByteSpan & credentialData, uint16_t & createdUserIndex)
{
    uint16_t availableUserIndex = 0;
    if (!findUnoccupiedUserSlot(endpointId, availableUserIndex))
    {
        emberAfDoorLockClusterPrintln("[SetCredential] Unable to create new user for credential: no available user slots "
                                      "[endpointId=%d,credentialIndex=%d]",
                                      endpointId, credential.CredentialIndex);
        return DlStatus::kOccupied;
    }

    auto status =
        createUser(endpointId, creatorFabricIdx, sourceNodeId, availableUserIndex, Nullable<CharSpan>(), Nullable<uint32_t>(),
                   userStatus, userType, Nullable<DlCredentialRule>(), Nullable<DlCredential>(credential));
    if (EMBER_ZCL_STATUS_SUCCESS != status)
    {
        emberAfDoorLockClusterPrintln("[SetCredential] Unable to create new user for credential: internal error "
                                      "[endpointId=%d,credentialIndex=%d,userIndex=%d,status=%d]",
                                      endpointId, credential.CredentialIndex, availableUserIndex, status);
        return DlStatus::kFailure;
    }

    if (!emberAfPluginDoorLockSetCredential(endpointId, credential.CredentialIndex, DlCredentialStatus::kOccupied,
                                            static_cast<DlCredentialType>(credential.CredentialType), credentialData))
    {
        emberAfDoorLockClusterPrintln("[SetCredential] Unable to set the credential: app error "
                                      "[endpointId=%d,credentialIndex=%d,credentialType=%u,dataLength=%zu]",
                                      endpointId, credential.CredentialIndex, credential.CredentialType, credentialData.size());
        return DlStatus::kFailure;
    }

    emberAfDoorLockClusterPrintln("[SetCredential] Credential and user were created "
                                  "[endpointId=%d,credentialIndex=%d,credentialType=%u,dataLength=%zu,userIndex=%d]",
                                  endpointId, credential.CredentialIndex, credential.CredentialType, credentialData.size(),
                                  availableUserIndex);
    createdUserIndex = availableUserIndex;

    return DlStatus::kSuccess;
}

DlStatus DoorLockServer::createNewCredentialAndAddItToUser(chip::EndpointId endpointId, chip::FabricIndex modifierFabricIdx,
                                                           uint16_t userIndex, const DlCredential & credential,
                                                           const chip::ByteSpan & credentialData)
{
    if (!userIndexValid(endpointId, userIndex))
    {
        emberAfDoorLockClusterPrintln("[SetCredential] Unable to add new credential to user: user out of bounds "
                                      "[endpointId=%d,credentialIndex=%d,userIndex=%d]",
                                      endpointId, credential.CredentialIndex, userIndex);
        return DlStatus::kInvalidField;
    }

    EmberAfPluginDoorLockUserInfo user;
    if (!emberAfPluginDoorLockGetUser(endpointId, userIndex, user))
    {
        emberAfDoorLockClusterPrintln(
            "[SetCredential] Unable to check if credential exists: app error [endpointId=%d,credentialIndex=%d,userIndex=%d]",
            endpointId, credential.CredentialIndex, userIndex);

        return DlStatus::kFailure;
    }

    // Not in the spec, but common sense: I don't think we need to modify the credential if user slot is not occupied
    if (user.userStatus == DlUserStatus::kAvailable)
    {
        emberAfDoorLockClusterPrintln("[SetCredential] Unable to add credential to user: user clot is empty "
                                      "[endpointId=%d,credentialIndex=%d,userIndex=%d]",
                                      endpointId, credential.CredentialIndex, userIndex);
        return DlStatus::kInvalidField;
    }

    // Add new credential to the user
    auto status = addCredentialToUser(endpointId, modifierFabricIdx, userIndex, credential);
    if (DlStatus::kSuccess != status)
    {
        emberAfDoorLockClusterPrintln("[SetCredential] Unable to add credential to a user: internal error "
                                      "[endpointId=%d,credentialIndex=%d,userIndex=%d,status=%u]",
                                      endpointId, credential.CredentialIndex, userIndex, to_underlying(status));
        return status;
    }

    if (!emberAfPluginDoorLockSetCredential(endpointId, credential.CredentialIndex, DlCredentialStatus::kOccupied,
                                            static_cast<DlCredentialType>(credential.CredentialType), credentialData))
    {
        emberAfDoorLockClusterPrintln("[SetCredential] Unable to set the credential: app error "
                                      "[endpointId=%d,credentialIndex=%d,credentialType=%u,dataLength=%zu]",
                                      endpointId, credential.CredentialIndex, credential.CredentialType, credentialData.size());
        return DlStatus::kFailure;
    }

    return DlStatus::kSuccess;
}

DlStatus DoorLockServer::addCredentialToUser(chip::EndpointId endpointId, chip::FabricIndex modifierFabricIdx, uint16_t userIndex,
                                             const DlCredential & credential)
{
    // We should get the user by that index first
    EmberAfPluginDoorLockUserInfo user;
    if (!emberAfPluginDoorLockGetUser(endpointId, userIndex, user))
    {
        emberAfDoorLockClusterPrintln("[AddCredentialToUser] Unable to get the user from app [endpointId=%d,userIndex=%d]",
                                      endpointId, userIndex);
        return DlStatus::kFailure;
    }

    // TODO: Do we need to check the modifier fabric here? Discuss with Spec team and add it if necessary.

    for (size_t i = 0; i < user.credentials.size(); ++i)
    {
        // appclusters, 5.2.4.40: If user already contains the credential of the same type we should return INVALID_COMMAND
        if (user.credentials.data()[i].CredentialType == credential.CredentialType)
        {
            emberAfDoorLockClusterPrintln(
                "[AddCredentialToUser] Unable to add credential to user: credential with this type already exists "
                "[endpointId=%d,userIndex=%d,credentialType=%d]",
                endpointId, userIndex, credential.CredentialType);
            return DlStatus::kOccupied;
        }

        // appclusters, 5.2.4.40: user should not be already associated with given credentialIndex
        if (user.credentials.data()[i].CredentialIndex == credential.CredentialIndex)
        {
            emberAfDoorLockClusterPrintln(
                "[AddCredentialToUser] Unable to add credential to user: credential with this index is already associated "
                "with user [endpointId=%d,userIndex=%d,credentialType=%d,credentialIndex=%d]",
                endpointId, userIndex, credential.CredentialType, credential.CredentialIndex);
            return DlStatus::kInvalidField;
        }
    }

    // appclusters: spec defines up to 5 credentials per user
    if (user.credentials.size() + 1 > DOOR_LOCK_MAX_CREDENTIALS_PER_USER)
    {
        emberAfDoorLockClusterPrintln("[AddCredentialToUser] Unable to add credentials to user: too many credentials "
                                      "[endpointId=%d,userIndex=%d,userTotalCredentials=%zu]",
                                      endpointId, userIndex, user.credentials.size());
        return DlStatus::kInvalidField;
    }

    DlCredential newCredentials[DOOR_LOCK_MAX_CREDENTIALS_PER_USER];
    memcpy(newCredentials, user.credentials.data(), sizeof(DlCredential) * user.credentials.size());
    newCredentials[user.credentials.size()] = credential;

    if (!emberAfPluginDoorLockSetUser(endpointId, userIndex, user.createdBy, modifierFabricIdx, user.userName, user.userUniqueId,
                                      user.userStatus, user.userType, user.credentialRule, newCredentials,
                                      user.credentials.size() + 1))
    {
        emberAfDoorLockClusterPrintln(
            "[AddCredentialToUser] Unable to add credential to user: credential with this index is already associated "
            "with user [endpointId=%d,userIndex=%d,credentialType=%d,credentialIndex=%d,userTotalCredentials=%zu]",
            endpointId, userIndex, credential.CredentialType, credential.CredentialIndex, user.credentials.size());
        return DlStatus::kFailure;
    }

    emberAfDoorLockClusterPrintln("[AddCredentialToUser] Credential added to user "
                                  "[endpointId=%d,userIndex=%d,credentialType=%d,credentialIndex=%d,userTotalCredentials=%zu]",
                                  endpointId, userIndex, credential.CredentialType, credential.CredentialIndex,
                                  user.credentials.size() + 1);

    return DlStatus::kSuccess;
}

DlStatus DoorLockServer::modifyCredentialForUser(chip::EndpointId endpointId, chip::FabricIndex modifierFabricIdx,
                                                 uint16_t userIndex, const DlCredential & credential)
{
    // We should get the user by that index first
    EmberAfPluginDoorLockUserInfo user;
    if (!emberAfPluginDoorLockGetUser(endpointId, userIndex, user))
    {
        emberAfDoorLockClusterPrintln("[ModifyUserCredential] Unable to get the user from app [endpointId=%d,userIndex=%d]",
                                      endpointId, userIndex);
        return DlStatus::kFailure;
    }

    // TODO: Do we need to check the modifier fabric here? Discuss with Spec team and add it if necessary.

    for (size_t i = 0; i < user.credentials.size(); ++i)
    {
        // appclusters, 5.2.4.40: user should already be associated with given credentialIndex
        if (user.credentials.data()[i].CredentialIndex == credential.CredentialIndex)
        {
            DlCredential newCredentials[DOOR_LOCK_MAX_CREDENTIALS_PER_USER];
            memcpy(newCredentials, user.credentials.data(), sizeof(DlCredential) * user.credentials.size());
            newCredentials[i] = credential;

            emberAfDoorLockClusterPrintln(
                "[ModifyUserCredential] Unable to add credential to user: credential with this index is already associated "
                "[endpointId=%d,userIndex=%d,credentialType=%d,credentialIndex=%d]",
                endpointId, userIndex, credential.CredentialType, credential.CredentialIndex);

            if (!emberAfPluginDoorLockSetUser(endpointId, userIndex, user.createdBy, modifierFabricIdx, user.userName,
                                              user.userUniqueId, user.userStatus, user.userType, user.credentialRule,
                                              newCredentials, user.credentials.size()))
            {
                emberAfDoorLockClusterPrintln(
                    "[ModifyUserCredential] Unable to modify user credential: credential with this index is already associated "
                    "with user [endpointId=%d,userIndex=%d,credentialType=%d,credentialIndex=%d,userTotalCredentials=%zu]",
                    endpointId, userIndex, credential.CredentialType, credential.CredentialIndex, user.credentials.size());
                return DlStatus::kFailure;
            }

            emberAfDoorLockClusterPrintln(
                "[ModifyUserCredential] User credential modified "
                "[endpointId=%d,userIndex=%d,credentialType=%d,credentialIndex=%d,userTotalCredentials=%zu]",
                endpointId, userIndex, credential.CredentialType, credential.CredentialIndex, user.credentials.size());

            return DlStatus::kSuccess;
        }
    }

    // appclusters, 5.2.4.40: if user is not associated with credential index we should return INVALID_COMMAND
    emberAfDoorLockClusterPrintln(
        "[ModifyUserCredential] Unable to modify user credential: user is not associated with credential index "
        "[endpointId=%d,userIndex=%d,credentialIndex=%d]",
        endpointId, userIndex, credential.CredentialIndex);

    return DlStatus::kInvalidField;
}

DlStatus DoorLockServer::createCredential(chip::EndpointId endpointId, chip::FabricIndex creatorFabricIdx,
                                          chip::NodeId sourceNodeId, uint16_t credentialIndex, DlCredentialType credentialType,
                                          const EmberAfPluginDoorLockCredentialInfo & existingCredential,
                                          const chip::ByteSpan & credentialData, Nullable<uint16_t> userIndex,
                                          Nullable<DlUserStatus> userStatus, Nullable<DlUserType> userType,
                                          uint16_t & createdUserIndex)
{
    // appclusters, 5.2.4.41.1: should send the OCCUPIED in the response when the credential is in use
    if (DlCredentialStatus::kAvailable != existingCredential.status)
    {
        emberAfDoorLockClusterPrintln(
            "[SetCredential] Unable to set the credential: credential slot is occupied [endpointId=%d,credentialIndex=%d]",
            endpointId, credentialIndex);

        return DlStatus::kOccupied;
    }

    if (!userType.IsNull() && DlUserType::kProgrammingUser == userType.Value())
    {
        emberAfDoorLockClusterPrintln("[SetCredential] Unable to set the credential: user type is invalid "
                                      "[endpointId=%d,credentialIndex=%d,userType=%u]",
                                      endpointId, credentialIndex, to_underlying(userType.Value()));

        return DlStatus::kInvalidField;
    }

    DlCredential credential{ to_underlying(credentialType), credentialIndex };
    // appclusters, 5.2.4.40: if userIndex is not provided we should create new user
    DlStatus status = DlStatus::kSuccess;
    if (userIndex.IsNull())
    {
        emberAfDoorLockClusterPrintln("[SetCredential] UserIndex is not set, creating new user [endpointId=%d,credentialIndex=%d]",
                                      endpointId, credentialIndex);

        status = createNewCredentialAndUser(endpointId, creatorFabricIdx, sourceNodeId, userStatus, userType, credential,
                                            credentialData, createdUserIndex);
    }
    else
    {
        // appclusters, 5.2.4.40: if user index is NULL, we should try to modify the existing user
        status = createNewCredentialAndAddItToUser(endpointId, creatorFabricIdx, userIndex.Value(), credential, credentialData);
    }

    if (DlStatus::kSuccess == status)
    {
        sendRemoteLockUserChange(endpointId, credentialTypeToLockDataType(credentialType), DlDataOperationType::kAdd, sourceNodeId,
                                 creatorFabricIdx, createdUserIndex == 0 ? userIndex.Value() : createdUserIndex, credentialIndex);
    }

    return status;
}

DlStatus DoorLockServer::modifyProgrammingPIN(chip::EndpointId endpointId, chip::FabricIndex modifierFabricIndex,
                                              chip::NodeId sourceNodeId, uint16_t credentialIndex, DlCredentialType credentialType,
                                              const EmberAfPluginDoorLockCredentialInfo & existingCredential,
                                              const chip::ByteSpan & credentialData)
{
    if (DlCredentialType::kProgrammingPIN != credentialType || 0 != credentialIndex)
    {
        emberAfDoorLockClusterPrintln(
            "[SetCredential] Unable to modify programming PIN: invalid argument [endpointId=%d,credentialIndex=%d]", endpointId,
            credentialIndex);

        return DlStatus::kInvalidField;
    }

    emberAfDoorLockClusterPrintln("[SetCredential] Modifying the programming PIN [endpointId=%d,credentialIndex=%d]", endpointId,
                                  credentialIndex);

    uint16_t relatedUserIndex = 0;
    if (!findUserIndexByCredential(endpointId, DlCredentialType::kProgrammingPIN, 0, relatedUserIndex))
    {
        ChipLogError(Zcl, "[SetCredential] Unable to modify PIN - related user not found (internal error) [endpointId=%d]",
                     endpointId);
        return DlStatus::kFailure;
    }

    if (!emberAfPluginDoorLockSetCredential(endpointId, credentialIndex, existingCredential.status,
                                            existingCredential.credentialType, credentialData))
    {
        emberAfDoorLockClusterPrintln("[SetCredential] Unable to modify the credential: app error "
                                      "[endpointId=%d,credentialIndex=%d,credentialType=%u,credentialDataSize=%zu]",
                                      endpointId, credentialIndex, to_underlying(credentialType), credentialData.size());
        return DlStatus::kFailure;
    }
    else
    {
        emberAfDoorLockClusterPrintln("[SetCredential] Successfully modified the credential "
                                      "[endpointId=%d,credentialIndex=%d,credentialType=%u,credentialDataSize=%zu]",
                                      endpointId, credentialIndex, to_underlying(credentialType), credentialData.size());

        sendRemoteLockUserChange(endpointId, credentialTypeToLockDataType(credentialType), DlDataOperationType::kModify,
                                 sourceNodeId, modifierFabricIndex, relatedUserIndex, credentialIndex);
    }

    return DlStatus::kSuccess;
}

DlStatus DoorLockServer::modifyCredential(chip::EndpointId endpointId, chip::FabricIndex modifierFabricIndex,
                                          chip::NodeId sourceNodeId, uint16_t credentialIndex, DlCredentialType credentialType,
                                          const EmberAfPluginDoorLockCredentialInfo & existingCredential,
                                          const chip::ByteSpan & credentialData, uint16_t userIndex,
                                          Nullable<DlUserStatus> userStatus, Nullable<DlUserType> userType)
{

    // appclusters, 5.2.4.40: when modifying a credential, userStatus and userType shall both be NULL.
    if (!userStatus.IsNull() || (!userType.IsNull() && DlUserType::kProgrammingUser != userType.Value()))
    {
        emberAfDoorLockClusterPrintln("[SetCredential] Unable to modify the credential: invalid arguments "
                                      "[endpointId=%d,credentialIndex=%d,credentialType=%u]",
                                      endpointId, credentialIndex, to_underlying(credentialType));
        return DlStatus::kInvalidField;
    }

    DlCredential credential{ to_underlying(credentialType), credentialIndex };
    auto status = modifyCredentialForUser(endpointId, modifierFabricIndex, userIndex, credential);

    if (DlStatus::kSuccess == status)
    {
        if (!emberAfPluginDoorLockSetCredential(endpointId, credentialIndex, existingCredential.status,
                                                existingCredential.credentialType, credentialData))
        {
            emberAfDoorLockClusterPrintln("[SetCredential] Unable to modify the credential: app error "
                                          "[endpointId=%d,credentialIndex=%d,credentialType=%u,credentialDataSize=%zu]",
                                          endpointId, credentialIndex, to_underlying(credentialType), credentialData.size());

            return DlStatus::kFailure;
        }

        emberAfDoorLockClusterPrintln("[SetCredential] Successfully modified the credential "
                                      "[endpointId=%d,credentialIndex=%d,credentialType=%u,credentialDataSize=%zu]",
                                      endpointId, credentialIndex, to_underlying(credentialType), credentialData.size());

        sendRemoteLockUserChange(endpointId, credentialTypeToLockDataType(credentialType), DlDataOperationType::kModify,
                                 sourceNodeId, modifierFabricIndex, userIndex, credentialIndex);
    }
    return status;
}

CHIP_ERROR DoorLockServer::sendSetCredentialResponse(chip::app::CommandHandler * commandObj, DlStatus status, uint16_t userIndex,
                                                     uint16_t nextCredentialIndex)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    using ResponseFields = Commands::SetCredentialResponse::Fields;

    app::ConcreteCommandPath path = { emberAfCurrentEndpoint(), ::Id, Commands::SetCredentialResponse::Id };
    TLV::TLVWriter * writer       = nullptr;
    SuccessOrExit(err = commandObj->PrepareCommand(path));
    VerifyOrExit((writer = commandObj->GetCommandDataIBTLVWriter()) != nullptr, err = CHIP_ERROR_INCORRECT_STATE);
    SuccessOrExit(err = writer->Put(TLV::ContextTag(to_underlying(ResponseFields::kStatus)), status));

    if (0 != userIndex)
    {
        SuccessOrExit(err = writer->Put(TLV::ContextTag(to_underlying(ResponseFields::kUserIndex)), userIndex));
    }

    if (0 != nextCredentialIndex)
    {
        SuccessOrExit(err = writer->Put(TLV::ContextTag(to_underlying(ResponseFields::kNextCredentialIndex)), nextCredentialIndex));
    }
    SuccessOrExit(err = commandObj->FinishCommand());

exit:
    return err;
}

bool DoorLockServer::credentialTypeSupported(chip::EndpointId endpointId, DlCredentialType type)
{
    switch (type)
    {
    case DlCredentialType::kProgrammingPIN:
    case DlCredentialType::kPin:
        return SupportsPIN(endpointId);
    case DlCredentialType::kRfid:
        return SupportsPFID(endpointId);
    default:
        return false;
    }
    return false;
}

bool DoorLockServer::weekDayIndexValid(chip::EndpointId endpointId, uint8_t weekDayIndex)
{
    uint8_t weekDaysSupported;
    if (!GetNumberOfWeekDaySchedulesPerUserSupported(endpointId, weekDaysSupported))
    {
        return false;
    }

    // appclusters, 5.2.4.14-17: weekday index changes from 1 to maxNumberOfUsers
    if (0 == weekDayIndex || weekDayIndex > weekDaysSupported)
    {
        return false;
    }
    return true;
}

DlStatus DoorLockServer::clearWeekDaySchedule(chip::EndpointId endpointId, uint16_t userIndex, uint8_t weekDayIndex)
{
    auto status = emberAfPluginDoorLockSetSchedule(endpointId, weekDayIndex, userIndex, DlScheduleStatus::kAvailable,
                                                   DlDaysMaskMap(0), 0, 0, 0, 0);
    if (DlStatus::kSuccess != status && DlStatus::kNotFound != status)
    {
        ChipLogError(Zcl,
                     "[ClearWeekDaySchedule] Unable to clear the schedule - internal error "
                     "[endpointId=%d,userIndex=%d,scheduleIndex=%d,status=%u]",
                     endpointId, userIndex, weekDayIndex, to_underlying(status));
        return status;
    }
    return DlStatus::kSuccess;
}

DlStatus DoorLockServer::clearWeekDaySchedules(chip::EndpointId endpointId, uint16_t userIndex)
{
    uint8_t weekDaySchedulesPerUser = 0;
    if (!GetNumberOfWeekDaySchedulesPerUserSupported(endpointId, weekDaySchedulesPerUser))
    {
        return DlStatus::kFailure;
    }

    for (uint8_t i = 1; i <= weekDaySchedulesPerUser; ++i)
    {
        auto status = clearWeekDaySchedule(endpointId, userIndex, i);
        if (DlStatus::kSuccess != status)
        {
            return status;
        }
    }
    return DlStatus::kSuccess;
}

DlStatus DoorLockServer::clearSchedules(chip::EndpointId endpointId, uint16_t userIndex)
{
    auto status = clearWeekDaySchedules(endpointId, userIndex);
    if (DlStatus::kSuccess != status)
    {
        ChipLogError(Zcl,
                     "[CleaAllSchedules] Unable to clear week day schedules for user - internal error "
                     "[endpointId=%d,userIndex=%d,status=%u]",
                     endpointId, userIndex, to_underlying(status));
        return status;
    }

    status = clearYearDaySchedules(endpointId, userIndex);
    if (DlStatus::kSuccess != status)
    {
        ChipLogError(Zcl,
                     "[CleaAllSchedules] Unable to clear year day schedules for user - internal error "
                     "[endpointId=%d,userIndex=%d,status=%u]",
                     endpointId, userIndex, to_underlying(status));
        return status;
    }

    // TODO: Clear holiday schedules when they're implemented
    return DlStatus::kSuccess;
}

CHIP_ERROR DoorLockServer::sendGetWeekDayScheduleResponse(chip::app::CommandHandler * commandObj,
                                                          const chip::app::ConcreteCommandPath & commandPath, uint8_t weekdayIndex,
                                                          uint16_t userIndex, DlStatus status, DlDaysMaskMap daysMask,
                                                          uint8_t startHour, uint8_t startMinute, uint8_t endHour,
                                                          uint8_t endMinute)
{
    VerifyOrDie(nullptr != commandObj);

    Commands::GetWeekDayScheduleResponse::Type response;
    response.weekDayIndex = weekdayIndex;
    response.userIndex    = userIndex;
    response.status       = status;
    if (DlStatus::kSuccess == status)
    {
        response.daysMask    = Optional<chip::BitFlags<DlDaysMaskMap>>(daysMask);
        response.startHour   = Optional<uint8_t>(startHour);
        response.startMinute = Optional<uint8_t>(startMinute);
        response.endHour     = Optional<uint8_t>(endHour);
        response.endMinute   = Optional<uint8_t>(endMinute);
    }

    return commandObj->AddResponseData(commandPath, response);
}

bool DoorLockServer::yearDayIndexValid(chip::EndpointId endpointId, uint8_t yearDayIndex)
{
    uint8_t yearDaysSupported;
    if (!GetNumberOfYearDaySchedulesPerUserSupported(endpointId, yearDaysSupported))
    {
        return false;
    }

    // appclusters, 5.2.4.18-21: year day index changes from 1 to maxNumberOfUsers
    if (0 == yearDayIndex || yearDayIndex > yearDaysSupported)
    {
        return false;
    }
    return true;
}

DlStatus DoorLockServer::clearYearDaySchedule(chip::EndpointId endpointId, uint16_t userIndex, uint8_t yearDayIndex)
{
    auto status = emberAfPluginDoorLockSetSchedule(endpointId, yearDayIndex, userIndex, DlScheduleStatus::kAvailable, 0, 0);
    if (DlStatus::kSuccess != status && DlStatus::kNotFound != status)
    {
        ChipLogError(Zcl,
                     "[ClearYearDaySchedule] Unable to clear the schedule - internal error "
                     "[endpointId=%d,userIndex=%d,scheduleIndex=%d,status=%u]",
                     endpointId, userIndex, yearDayIndex, to_underlying(status));
        return status;
    }
    return DlStatus::kSuccess;
}

DlStatus DoorLockServer::clearYearDaySchedules(chip::EndpointId endpointId, uint16_t userIndex)
{
    uint8_t weekDaySchedulesPerUser = 0;
    if (!GetNumberOfYearDaySchedulesPerUserSupported(endpointId, weekDaySchedulesPerUser))
    {
        return DlStatus::kFailure;
    }

    for (uint8_t i = 1; i <= weekDaySchedulesPerUser; ++i)
    {
        auto status = clearYearDaySchedule(endpointId, userIndex, i);
        if (DlStatus::kSuccess != status)
        {
            return status;
        }
    }
    return DlStatus::kSuccess;
}

CHIP_ERROR DoorLockServer::sendGetYearDayScheduleResponse(chip::app::CommandHandler * commandObj,
                                                          const chip::app::ConcreteCommandPath & commandPath, uint8_t yearDayIndex,
                                                          uint16_t userIndex, DlStatus status, uint32_t localStartTime,
                                                          uint32_t localEndTime)
{
    VerifyOrDie(nullptr != commandObj);

    Commands::GetYearDayScheduleResponse::Type response;
    response.yearDayIndex = yearDayIndex;
    response.userIndex    = userIndex;
    response.status       = status;
    if (DlStatus::kSuccess == status)
    {
        response.localStartTime = Optional<uint32_t>(localStartTime);
        response.localEndTime   = Optional<uint32_t>(localEndTime);
    }

    return commandObj->AddResponseData(commandPath, response);
}

EmberAfStatus DoorLockServer::clearCredential(chip::EndpointId endpointId, chip::FabricIndex modifier, chip::NodeId sourceNodeId,
                                              DlCredentialType credentialType, uint16_t credentialIndex, bool sendUserChangeEvent)
{
    if (DlCredentialType::kProgrammingPIN == credentialType)
    {
        emberAfDoorLockClusterPrintln("[clearCredential] Cannot clear programming PIN credentials "
                                      "[endpointId=%d,credentialType=%u,credentialIndex=%d,modifier=%d]",
                                      endpointId, to_underlying(credentialType), credentialIndex, modifier);
        return EMBER_ZCL_STATUS_INVALID_COMMAND;
    }

    if (!credentialIndexValid(endpointId, credentialType, credentialIndex))
    {
        emberAfDoorLockClusterPrintln("[clearCredential] Cannot clear credential - index out of bounds "
                                      "[endpointId=%d,credentialType=%u,credentialIndex=%d,modifier=%d]",
                                      endpointId, to_underlying(credentialType), credentialIndex, modifier);
        return EMBER_ZCL_STATUS_INVALID_COMMAND;
    }

    // 1. Fetch the credential from storage, so we know what we're deleting
    EmberAfPluginDoorLockCredentialInfo credential;
    if (!emberAfPluginDoorLockGetCredential(endpointId, credentialIndex, credentialType, credential))
    {
        ChipLogError(Zcl,
                     "[clearCredential] Unable to clear credential - couldn't read credential from database "
                     "[endpointId=%d,credentialType=%u,credentialIndex=%d,modifier=%d]",
                     endpointId, to_underlying(credentialType), credentialIndex, modifier);
        return EMBER_ZCL_STATUS_FAILURE;
    }

    if (DlCredentialStatus::kAvailable == credential.status)
    {
        emberAfDoorLockClusterPrintln("[clearCredential] Ignored attempt to clear unoccupied credential slot "
                                      "[endpointId=%d,credentialType=%u,credentialIndex=%d,modifier=%d]",
                                      endpointId, to_underlying(credentialType), credentialIndex, modifier);
        return EMBER_ZCL_STATUS_SUCCESS;
    }

    if (credentialType != credential.credentialType)
    {
        emberAfDoorLockClusterPrintln("[clearCredential] Ignored attempt to clear credential of different type "
                                      "[endpointId=%d,credentialType=%u,credentialIndex=%d,modifier=%d,actualCredentialType=%u]",
                                      endpointId, to_underlying(credentialType), credentialIndex, modifier,
                                      to_underlying(credential.credentialType));
        return EMBER_ZCL_STATUS_SUCCESS;
    }

    // 2. Get the associated user and if it is the only attached credential -- delete the user. This operation will
    // also remove the associated credential.
    uint16_t relatedUserIndex = 0;
    if (!findUserIndexByCredential(endpointId, credentialType, credentialIndex, relatedUserIndex))
    {
        ChipLogError(Zcl,
                     "[clearCredential] Unable to clear related credential user - couldn't find index of related user "
                     "[endpointId=%d,credentialType=%u,credentialIndex=%d,modifier=%d]",
                     endpointId, to_underlying(credentialType), credentialIndex, modifier);
        return EMBER_ZCL_STATUS_FAILURE;
    }
    EmberAfPluginDoorLockUserInfo relatedUser;
    if (!emberAfPluginDoorLockGetUser(endpointId, relatedUserIndex, relatedUser))
    {
        ChipLogError(Zcl,
                     "[clearCredential] Unable to clear credential for related user - couldn't get user from database "
                     "[endpointId=%d,credentialType=%u,credentialIndex=%d,modifier=%d,userIndex=%d]",
                     endpointId, to_underlying(credentialType), credentialIndex, modifier, relatedUserIndex);

        return EMBER_ZCL_STATUS_FAILURE;
    }
    if (1 == relatedUser.credentials.size())
    {
        emberAfDoorLockClusterPrintln("[clearCredential] Clearing related user - no credentials left "
                                      "[endpointId=%d,credentialType=%u,credentialIndex=%d,modifier=%d,userIndex=%d]",
                                      endpointId, to_underlying(credentialType), credentialIndex, modifier, relatedUserIndex);
        auto clearStatus = clearUser(endpointId, modifier, sourceNodeId, relatedUserIndex, relatedUser, true);
        if (EMBER_ZCL_STATUS_SUCCESS != clearStatus)
        {
            ChipLogError(Zcl,
                         "[clearCredential] Unable to clear related credential user - internal error "
                         "[endpointId=%d,credentialType=%u,credentialIndex=%d,modifier=%d,userIndex=%d,status=%d]",
                         endpointId, to_underlying(credentialType), credentialIndex, modifier, relatedUserIndex, clearStatus);

            return EMBER_ZCL_STATUS_FAILURE;
        }
        emberAfDoorLockClusterPrintln("[clearCredential] Successfully clear credential and related user "
                                      "[endpointId=%d,credentialType=%u,credentialIndex=%d,modifier=%d,userIndex=%d]",
                                      endpointId, to_underlying(credentialType), credentialIndex, modifier, relatedUserIndex);
        return EMBER_ZCL_STATUS_SUCCESS;
    }

    // 3. If the user wasn't deleted, delete the credential and adjust the list of credentials for related user in the storage
    if (!emberAfPluginDoorLockSetCredential(endpointId, credentialIndex, DlCredentialStatus::kAvailable, credentialType,
                                            chip::ByteSpan()))
    {
        ChipLogError(Zcl,
                     "[clearCredential] Unable to clear credential - couldn't write new credential to database "
                     "[endpointId=%d,credentialType=%u,credentialIndex=%d,modifier=%d]",
                     endpointId, to_underlying(credentialType), credentialIndex, modifier);
        return EMBER_ZCL_STATUS_FAILURE;
    }

    // Should never happen, only possible if the implementation of application is incorrect
    if (relatedUser.credentials.size() > DOOR_LOCK_MAX_CREDENTIALS_PER_USER)
    {
        ChipLogError(Zcl,
                     "[clearCredential] Unable to clear credential for related user - user has too many credentials associated"
                     "[endpointId=%d,credentialType=%u,credentialIndex=%d,modifier=%d,userIndex=%d,credentialsCount=%zu]",
                     endpointId, to_underlying(credentialType), credentialIndex, modifier, relatedUserIndex,
                     relatedUser.credentials.size());

        return EMBER_ZCL_STATUS_FAILURE;
    }

    DlCredential newCredentials[DOOR_LOCK_MAX_CREDENTIALS_PER_USER];
    size_t newCredentialsCount = 0;
    for (const auto & c : relatedUser.credentials)
    {
        if (static_cast<DlCredentialType>(c.CredentialType) == credentialType && c.CredentialIndex == credentialIndex)
        {
            continue;
        }
        newCredentials[newCredentialsCount++] = c;
    }

    if (!emberAfPluginDoorLockSetUser(endpointId, relatedUserIndex, relatedUser.createdBy, modifier, relatedUser.userName,
                                      relatedUser.userUniqueId, relatedUser.userStatus, relatedUser.userType,
                                      relatedUser.credentialRule, newCredentials, newCredentialsCount))
    {
        ChipLogError(Zcl,
                     "[clearCredential] Unable to clear credential for related user - unable to update database "
                     "[endpointId=%d,credentialType=%u"
                     ",credentialIndex=%d,modifier=%d,userIndex=%d,newCredentialsCount=%zu]",
                     endpointId, to_underlying(credentialType), credentialIndex, modifier, relatedUserIndex, newCredentialsCount);
        return EMBER_ZCL_STATUS_FAILURE;
    }

    emberAfDoorLockClusterPrintln(
        "[clearCredential] Successfully clear credential and related user "
        "[endpointId=%d,credentialType=%u,credentialIndex=%d,modifier=%d,userIndex=%d,newCredentialsCount=%zu]",
        endpointId, to_underlying(credentialType), credentialIndex, modifier, relatedUserIndex, newCredentialsCount);

    if (sendUserChangeEvent)
    {
        sendRemoteLockUserChange(endpointId, credentialTypeToLockDataType(credentialType), DlDataOperationType::kClear,
                                 sourceNodeId, modifier, relatedUserIndex, credentialIndex);
    }

    return EMBER_ZCL_STATUS_SUCCESS;
}

EmberAfStatus DoorLockServer::clearCredentials(chip::EndpointId endpointId, chip::FabricIndex modifier, chip::NodeId sourceNodeId)
{
    if (SupportsPIN(endpointId))
    {
        auto status = clearCredentials(endpointId, modifier, sourceNodeId, DlCredentialType::kPin);
        if (EMBER_ZCL_STATUS_SUCCESS != status)
        {
            ChipLogError(Zcl, "[clearCredentials] Unable to clear all PIN credentials [endpointId=%d,status=%d]", endpointId,
                         status);
            return status;
        }

        emberAfDoorLockClusterPrintln("[clearCredentials] All PIN credentials were cleared [endpointId=%d]", endpointId);
    }

    if (SupportsPFID(endpointId))
    {
        auto status = clearCredentials(endpointId, modifier, sourceNodeId, DlCredentialType::kRfid);
        if (EMBER_ZCL_STATUS_SUCCESS != status)
        {
            ChipLogError(Zcl, "[clearCredentials] Unable to clear all RFID credentials [endpointId=%d,status=%d]", endpointId,
                         status);
            return status;
        }
        emberAfDoorLockClusterPrintln("[clearCredentials] All RFID credentials were cleared [endpointId=%d]", endpointId);
    }

    if (SupportsFingers(endpointId))
    {
        auto status = clearCredentials(endpointId, modifier, sourceNodeId, DlCredentialType::kFingerprint);
        if (EMBER_ZCL_STATUS_SUCCESS != status)
        {
            ChipLogError(Zcl, "[clearCredentials] Unable to clear all Fingerprint credentials [endpointId=%d,status=%d]",
                         endpointId, status);
            return status;
        }

        status = clearCredentials(endpointId, modifier, sourceNodeId, DlCredentialType::kFingerVein);
        if (EMBER_ZCL_STATUS_SUCCESS != status)
        {
            ChipLogError(Zcl, "[clearCredentials] Unable to clear all Finger Vein credentials [endpointId=%d,status=%d]",
                         endpointId, status);
            return status;
        }

        emberAfDoorLockClusterPrintln("[clearCredentials] All Finger credentials were cleared [endpointId=%d]", endpointId);
    }

    if (SupportsFace(endpointId))
    {
        auto status = clearCredentials(endpointId, modifier, sourceNodeId, DlCredentialType::kFace);
        if (EMBER_ZCL_STATUS_SUCCESS != status)
        {
            ChipLogError(Zcl, "[clearCredentials] Unable to clear all face credentials [endpointId=%d,status=%d]", endpointId,
                         status);
            return status;
        }
        emberAfDoorLockClusterPrintln("[clearCredentials] All face credentials were cleared [endpointId=%d]", endpointId);
    }

    return EMBER_ZCL_STATUS_SUCCESS;
}

EmberAfStatus DoorLockServer::clearCredentials(chip::EndpointId endpointId, chip::FabricIndex modifier, chip::NodeId sourceNodeId,
                                               DlCredentialType credentialType)
{
    uint16_t maxNumberOfCredentials = 0;
    if (!getMaxNumberOfCredentials(endpointId, credentialType, maxNumberOfCredentials))
    {
        ChipLogError(Zcl,
                     "[clearCredentials] Unable to get max number of credentials to clear - can't get max number of credentials "
                     "[endpointId=%d,credentialType=%u]",
                     endpointId, to_underlying(credentialType));
        return EMBER_ZCL_STATUS_FAILURE;
    }

    for (uint16_t i = 1; i < maxNumberOfCredentials; ++i)
    {
        auto status = clearCredential(endpointId, modifier, sourceNodeId, credentialType, i, false);
        if (EMBER_ZCL_STATUS_SUCCESS != status)
        {
            ChipLogError(Zcl,
                         "[clearCredentials] Unable to clear the credential - internal error "
                         "[endpointId=%d,credentialType=%u,credentialIndex=%d,status=%d]",
                         endpointId, to_underlying(credentialType), i, status);
            return status;
        }
    }

    sendRemoteLockUserChange(endpointId, credentialTypeToLockDataType(credentialType), DlDataOperationType::kClear, sourceNodeId,
                             modifier, 0xFFFE, 0xFFFE);

    return EMBER_ZCL_STATUS_SUCCESS;
}

bool DoorLockServer::sendRemoteLockUserChange(chip::EndpointId endpointId, DlLockDataType dataType, DlDataOperationType operation,
                                              chip::NodeId nodeId, chip::FabricIndex fabricIndex, uint16_t userIndex,
                                              uint16_t dataIndex)
{
    Events::LockUserChange::Type event;
    event.lockDataType      = dataType;
    event.dataOperationType = operation;
    event.operationSource   = DlOperationSource::kRemote;
    if (0 != userIndex)
    {
        event.userIndex = Nullable<uint16_t>(userIndex);
    }
    event.fabricIndex = Nullable<chip::FabricIndex>(fabricIndex);
    event.sourceNode  = Nullable<chip::NodeId>(nodeId);
    if (0 != dataIndex)
    {
        event.dataIndex = Nullable<uint16_t>(dataIndex);
    }

    EventNumber eventNumber;
    CHIP_ERROR error = app::LogEvent(event, endpointId, eventNumber);
    if (CHIP_NO_ERROR != error)
    {
        ChipLogError(Zcl, "[RemoteLockUserChange] Unable to send lock user change event: %s [endpointId=%d]", error.AsString(),
                     endpointId);
        return false;
    }
    emberAfDoorLockClusterPrintln("[RemoteLockUserChange] Sent lock user change event "
                                  "[endpointId=%d,eventNumber=%" PRIu64 ",dataType=%u,operation=%u,nodeId=%" PRIu64
                                  ",fabricIndex=%d]",
                                  endpointId, eventNumber, to_underlying(dataType), to_underlying(operation), nodeId, fabricIndex);
    return true;
}

DlLockDataType DoorLockServer::credentialTypeToLockDataType(DlCredentialType credentialType)
{
    switch (credentialType)
    {
    case DlCredentialType::kProgrammingPIN:
        return DlLockDataType::kProgrammingCode;
    case DlCredentialType::kPin:
        return DlLockDataType::kPin;
    case DlCredentialType::kRfid:
        return DlLockDataType::kRfid;
    case DlCredentialType::kFingerprint:
        return DlLockDataType::kFingerprint;
    case DlCredentialType::kFingerVein:
        return DlLockDataType::kFingerprint;
    case DlCredentialType::kFace:
        // So far there's no distinct data type for face credentials
        return DlLockDataType::kUnspecified;
    }

    return DlLockDataType::kUnspecified;
}

bool DoorLockServer::HandleRemoteLockOperation(chip::app::CommandHandler * commandObj,
                                               const chip::app::ConcreteCommandPath & commandPath, DlLockOperationType opType,
                                               RemoteLockOpHandler opHandler, const Optional<ByteSpan> & pinCode)
{
    VerifyOrDie(DlLockOperationType::kLock == opType || DlLockOperationType::kUnlock == opType);
    VerifyOrDie(nullptr != opHandler);

    DlLockState newLockState = (DlLockOperationType::kLock == opType) ? DlLockState::kLocked : DlLockState::kUnlocked;
    EndpointId endpoint      = commandPath.mEndpointId;
    DlOperationError reason  = DlOperationError::kUnspecified;
    uint16_t pinUserIdx      = 0;
    uint16_t pinCredIdx      = 0;
    bool credentialsOk       = false;
    bool operationOk         = false;

    // appclusters.pdf 5.3.4.1:
    // When the PINCode field is provided an invalid PIN will count towards the WrongCodeEntryLimit and the
    // UserCodeTemporaryDisableTime will be triggered if the WrongCodeEntryLimit is exceeded. The lock SHALL ignore any attempts to
    // lock/unlock the door until the UserCodeTemporaryDisableTime expires.
    // TODO: check whether UserCodeTemporaryDisableTime expired or not.

    if (pinCode.HasValue())
    {
        // appclusters.pdf 5.3.4.1:
        // If the PINCode field is provided, the door lock SHALL verify PINCode before granting access regardless of the value of
        // RequirePINForRemoteOperation attribute.
        VerifyOrExit(SupportsPIN(endpoint) && SupportsUSR(endpoint),
                     emberAfDoorLockClusterPrintln(
                         "PIN code is supplied while USR/PIN features are disabled. Exiting [endpoint=%d, lock_op=%d]", endpoint,
                         chip::to_underlying(opType)));

        // Look up the user index and credential index -- it should be used in the Lock Operation event
        findUserIndexByCredential(endpoint, DlCredentialType::kPin, pinCode.Value(), pinUserIdx, pinCredIdx);

        // [EM]: I don't think we should prevent door lock/unlocking if we couldn't find credential associated with user. I
        // think if the app thinks that PIN is correct the door should be unlocked.
        //
        // [DV]: let's app decide on PIN correctness, we will fail only if 'opHandler' returns false.
        credentialsOk =
            true; // findUserIndexByCredential(endpoint, DlCredentialType::kPin, pinCode.Value(), pinUserIdx, pinCredIdx);
    }
    else
    {
        // appclusters.pdf 5.3.4.1:
        // If the RequirePINforRemoteOperation attribute is True then PINCode field SHALL be provided and the door lock SHALL NOT
        // grant access if it is not provided.
        bool requirePin = false;
        VerifyOrExit(GetAttribute(endpoint, Attributes::RequirePINforRemoteOperation::Id,
                                  Attributes::RequirePINforRemoteOperation::Get, requirePin),
                     /* credentialsOk is false here */);
        credentialsOk = !requirePin;
    }

    // TODO: increase WrongCodeEntryLimit if credentialsOk == false.
    // TODO: If limit is exceeded, lock remote operations for UserCodeTemporaryDisableTime.
    VerifyOrExit(credentialsOk, {
        reason = DlOperationError::kInvalidCredential;
        emberAfDoorLockClusterPrintln("Checking credentials failed: either PIN is invalid or not provided");
    });

    // credentials check succeeded, try to lock/unlock door
    operationOk = opHandler(endpoint, pinCode, reason);
    VerifyOrExit(operationOk, /* reason is set by the above call */);

    // door locked, set cluster attribute
    VerifyOrDie(SetLockState(endpoint, newLockState, DlOperationSource::kRemote));

exit:
    bool success = credentialsOk && operationOk;

    // Send command response
    emberAfSendImmediateDefaultResponse(success ? EMBER_ZCL_STATUS_SUCCESS : EMBER_ZCL_STATUS_FAILURE);

    // Send LockOperation/LockOperationError event
    LockOpCredentials foundCred[] = { { DlCredentialType::kPin, pinCredIdx } };
    LockOpCredentials * credList  = nullptr;
    size_t credListSize           = 0;

    // appclusters.pdf 5.3.5.3, 5.3.5.4:
    // The list of credentials used in performing the lock operation. This SHALL be null if no credentials were involved.
    if (pinCode.HasValue())
    {
        credList     = foundCred;
        credListSize = 1;
    }

    SendLockOperationEvent(endpoint, opType, DlOperationSource::kRemote, reason, Nullable<uint16_t>(pinUserIdx),
                           Nullable<chip::FabricIndex>(getFabricIndex(commandObj)), Nullable<chip::NodeId>(getNodeId(commandObj)),
                           credList, credListSize, success);
    return success;
}

void DoorLockServer::SendLockOperationEvent(chip::EndpointId endpointId, DlLockOperationType opType, DlOperationSource opSource,
                                            DlOperationError opErr, Nullable<uint16_t> userId,
                                            Nullable<chip::FabricIndex> fabricIdx, Nullable<chip::NodeId> nodeId,
                                            LockOpCredentials * credList, size_t credListSize, bool opSuccess)
{
    Nullable<List<const Structs::DlCredential::Type>> credentials{};

    // appclusters.pdf 5.3.5.3, 5.3.5.4:
    // The list of credentials used in performing the lock operation. This SHALL be null if no credentials were involved.
    if (nullptr == credList || 0 == credListSize)
    {
        credentials.SetNull();
    }
    else
    {
        credentials.SetNonNull(List<const Structs::DlCredential::Type>(credList, credListSize));
    }

    // TODO: if [USR] feature is not supported then credentials should be omitted (Optional.HasValue()==false)?
    // Spec just says that it should be NULL if no PIN were provided.

    if (opSuccess)
    {
        Events::LockOperation::Type event{ opType, opSource, userId, fabricIdx, nodeId, MakeOptional(credentials) };
        SendEvent(endpointId, event);
    }
    else
    {
        Events::LockOperationError::Type event{ opType, opSource, opErr, userId, fabricIdx, nodeId, MakeOptional(credentials) };
        SendEvent(endpointId, event);
    }
}

void DoorLockServer::ScheduleAutoRelock(chip::EndpointId endpointId, uint32_t timeoutSec)
{
    emberEventControlSetInactive(&AutolockEvent);

    AutolockEvent.endpoint = endpointId;
    AutolockEvent.callback = emberAfPluginDoorLockOnAutoRelock;

    uint32_t timeoutMs =
        (DOOR_LOCK_MAX_LOCK_TIMEOUT_SEC >= timeoutSec) ? timeoutSec * MILLISECOND_TICKS_PER_SECOND : DOOR_LOCK_MAX_LOCK_TIMEOUT_SEC;
    auto err = emberEventControlSetDelayMS(&AutolockEvent, timeoutMs);

    if (EMBER_SUCCESS != err)
    {
        ChipLogError(Zcl, "Failed to schedule autorelock: timeout=%" PRIu32 ", status=0x%x", timeoutSec, err);
    }
}

template <typename T>
void DoorLockServer::SendEvent(chip::EndpointId endpointId, T & event)
{
    EventNumber eventNumber;
    auto err = chip::app::LogEvent(event, endpointId, eventNumber);

    if (CHIP_NO_ERROR != err)
    {
        ChipLogError(Zcl, "Failed to log event: err=0x%" PRIx32 ", event_id=0x%" PRIx32, err.AsInteger(), event.GetEventId());
    }
}

template <typename T>
bool DoorLockServer::GetAttribute(chip::EndpointId endpointId, chip::AttributeId attributeId,
                                  EmberAfStatus (*getFn)(chip::EndpointId endpointId, T * value), T & value)
{
    EmberAfStatus status = getFn(endpointId, &value);
    bool success         = (EMBER_ZCL_STATUS_SUCCESS == status);

    if (!success)
    {
        ChipLogError(Zcl, "Failed to read DoorLock attribute: attribute=0x%" PRIx32 ", status=0x%x", attributeId,
                     to_underlying(status));
    }
    return success;
}

template <typename T>
bool DoorLockServer::SetAttribute(chip::EndpointId endpointId, chip::AttributeId attributeId,
                                  EmberAfStatus (*setFn)(chip::EndpointId endpointId, T value), T value)
{
    EmberAfStatus status = setFn(endpointId, value);
    bool success         = (EMBER_ZCL_STATUS_SUCCESS == status);

    if (!success)
    {
        ChipLogError(Zcl, "Failed to write DoorLock attribute: attribute=0x%" PRIx32 ", status=0x%x", attributeId,
                     to_underlying(status));
    }
    return success;
}

// =============================================================================
// Cluster commands callbacks
// =============================================================================

bool emberAfDoorLockClusterLockDoorCallback(chip::app::CommandHandler * commandObj,
                                            const chip::app::ConcreteCommandPath & commandPath,
                                            const chip::app::Clusters::DoorLock::Commands::LockDoor::DecodableType & commandData)
{
    emberAfDoorLockClusterPrintln("Received command: LockDoor");
    DoorLockServer::Instance().HandleRemoteLockOperation(commandObj, commandPath, DlLockOperationType::kLock,
                                                         emberAfPluginDoorLockOnDoorLockCommand, commandData.pinCode);
    return true;
}

bool emberAfDoorLockClusterUnlockDoorCallback(
    chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
    const chip::app::Clusters::DoorLock::Commands::UnlockDoor::DecodableType & commandData)
{
    emberAfDoorLockClusterPrintln("Received command: UnlockDoor");

    if (DoorLockServer::Instance().HandleRemoteLockOperation(commandObj, commandPath, DlLockOperationType::kUnlock,
                                                             emberAfPluginDoorLockOnDoorUnlockCommand, commandData.pinCode))
    {
        // appclusters.pdf 5.3.3.25:
        // The number of seconds to wait after unlocking a lock before it automatically locks again. 0=disabled. If set, unlock
        // operations from any source will be timed. For one time unlock with timeout use the specific command.
        uint32_t autoRelockTime = 0;

        VerifyOrReturnError(DoorLockServer::Instance().GetAutoRelockTime(commandPath.mEndpointId, autoRelockTime), true);
        VerifyOrReturnError(0 != autoRelockTime, true);
        DoorLockServer::Instance().ScheduleAutoRelock(commandPath.mEndpointId, autoRelockTime);
    }

    return true;
}

bool emberAfDoorLockClusterUnlockWithTimeoutCallback(
    chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
    const chip::app::Clusters::DoorLock::Commands::UnlockWithTimeout::DecodableType & commandData)
{
    emberAfDoorLockClusterPrintln("Received command: UnlockWithTimeout");

    if (DoorLockServer::Instance().HandleRemoteLockOperation(commandObj, commandPath, DlLockOperationType::kUnlock,
                                                             emberAfPluginDoorLockOnDoorUnlockCommand, commandData.pinCode))
    {
        // appclusters.pdf 5.3.4.3:
        // This command causes the lock device to unlock the door with a timeout parameter. After the time in seconds specified in
        // the timeout field, the lock device will relock itself automatically.
        // field: Timeout, type: uint16_t
        uint32_t timeout = static_cast<uint32_t>(commandData.timeout);

        VerifyOrReturnError(0 != timeout, true);
        DoorLockServer::Instance().ScheduleAutoRelock(commandPath.mEndpointId, timeout);
    }

    return true;
}

bool emberAfDoorLockClusterSetUserCallback(chip::app::CommandHandler * commandObj,
                                           const chip::app::ConcreteCommandPath & commandPath,
                                           const chip::app::Clusters::DoorLock::Commands::SetUser::DecodableType & commandData)
{
    DoorLockServer::Instance().SetUserCommandHandler(commandObj, commandPath, commandData);
    return true;
}

bool emberAfDoorLockClusterGetUserCallback(chip::app::CommandHandler * commandObj,
                                           const chip::app::ConcreteCommandPath & commandPath,
                                           const chip::app::Clusters::DoorLock::Commands::GetUser::DecodableType & commandData)
{
    DoorLockServer::Instance().GetUserCommandHandler(commandObj, commandPath, commandData);
    return true;
}

bool emberAfDoorLockClusterClearUserCallback(chip::app::CommandHandler * commandObj,
                                             const chip::app::ConcreteCommandPath & commandPath,
                                             const chip::app::Clusters::DoorLock::Commands::ClearUser::DecodableType & commandData)
{
    DoorLockServer::Instance().ClearUserCommandHandler(commandObj, commandPath, commandData);
    return true;
}

bool emberAfDoorLockClusterSetCredentialCallback(
    chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
    const chip::app::Clusters::DoorLock::Commands::SetCredential::DecodableType & commandData)
{
    DoorLockServer::Instance().SetCredentialCommandHandler(commandObj, commandPath, commandData);
    return true;
}

bool emberAfDoorLockClusterGetCredentialStatusCallback(
    chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
    const chip::app::Clusters::DoorLock::Commands::GetCredentialStatus::DecodableType & commandData)
{
    DoorLockServer::Instance().GetCredentialStatusCommandHandler(commandObj, commandPath, commandData);
    return true;
}

bool emberAfDoorLockClusterClearCredentialCallback(
    chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
    const chip::app::Clusters::DoorLock::Commands::ClearCredential::DecodableType & commandData)
{
    DoorLockServer::Instance().ClearCredentialCommandHandler(commandObj, commandPath, commandData);
    return true;
}

bool emberAfDoorLockClusterSetWeekDayScheduleCallback(
    chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
    const chip::app::Clusters::DoorLock::Commands::SetWeekDaySchedule::DecodableType & commandData)
{
    DoorLockServer::Instance().SetWeekDayScheduleCommandHandler(commandObj, commandPath, commandData);
    return true;
}

bool emberAfDoorLockClusterGetWeekDayScheduleCallback(
    chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
    const chip::app::Clusters::DoorLock::Commands::GetWeekDaySchedule::DecodableType & commandData)
{
    DoorLockServer::Instance().GetWeekDayScheduleCommandHandler(commandObj, commandPath, commandData);
    return true;
}

bool emberAfDoorLockClusterClearWeekDayScheduleCallback(
    chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
    const chip::app::Clusters::DoorLock::Commands::ClearWeekDaySchedule::DecodableType & commandData)
{
    DoorLockServer::Instance().ClearWeekDayScheduleCommandHandler(commandObj, commandPath, commandData);
    return true;
}

bool emberAfDoorLockClusterSetYearDayScheduleCallback(
    chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
    const chip::app::Clusters::DoorLock::Commands::SetYearDaySchedule::DecodableType & commandData)
{
    DoorLockServer::Instance().SetYearDayScheduleCommandHandler(commandObj, commandPath, commandData);
    return true;
}

bool emberAfDoorLockClusterGetYearDayScheduleCallback(
    chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
    const chip::app::Clusters::DoorLock::Commands::GetYearDaySchedule::DecodableType & commandData)
{
    DoorLockServer::Instance().GetYearDayScheduleCommandHandler(commandObj, commandPath, commandData);
    return true;
}

bool emberAfDoorLockClusterClearYearDayScheduleCallback(
    chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
    const chip::app::Clusters::DoorLock::Commands::ClearYearDaySchedule::DecodableType & commandData)
{
    DoorLockServer::Instance().ClearYearDayScheduleCommandHandler(commandObj, commandPath, commandData);
    return true;
}

bool emberAfDoorLockClusterSetHolidayScheduleCallback(
    chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
    const chip::app::Clusters::DoorLock::Commands::SetHolidaySchedule::DecodableType & commandData)
{
    emberAfDoorLockClusterPrintln("SetHolidaySchedule: command not implemented");

    // TODO: Implement setting holiday schedule
    emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_SUCCESS);
    return true;
}

bool emberAfDoorLockClusterGetHolidayScheduleCallback(
    chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
    const chip::app::Clusters::DoorLock::Commands::GetHolidaySchedule::DecodableType & commandData)
{
    emberAfDoorLockClusterPrintln("GetHolidaySchedule: command not implemented");

    // TODO: Implement getting holiday schedule
    emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_SUCCESS);
    return true;
}

bool emberAfDoorLockClusterClearHolidayScheduleCallback(
    chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
    const chip::app::Clusters::DoorLock::Commands::ClearHolidaySchedule::DecodableType & commandData)
{
    emberAfDoorLockClusterPrintln("ClearHolidaySchedule: command not implemented");

    // TODO: Implement clearing holiday schedule
    emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_SUCCESS);
    return true;
}

// =============================================================================
// SDK callbacks
// =============================================================================

chip::Protocols::InteractionModel::Status
MatterDoorLockClusterServerPreAttributeChangedCallback(const chip::app::ConcreteAttributePath & attributePath,
                                                       EmberAfAttributeType attributeType, uint16_t size, uint8_t * value)
{
    chip::Protocols::InteractionModel::Status res;

    switch (attributePath.mAttributeId)
    {
    case chip::app::Clusters::DoorLock::Attributes::Language::Id:
        if (value[0] <= 3)
        {
            auto lang = chip::CharSpan(reinterpret_cast<const char *>(&value[1]), static_cast<size_t>(value[0]));
            res       = emberAfPluginDoorLockOnLanguageChange(attributePath.mEndpointId, lang);
        }
        else
        {
            res = chip::Protocols::InteractionModel::Status::InvalidValue;
        }
        break;

    case chip::app::Clusters::DoorLock::Attributes::AutoRelockTime::Id:
        if (sizeof(uint32_t) == size)
        {
            uint32_t newRelockTime = *(reinterpret_cast<uint32_t *>(value));
            res                    = emberAfPluginDoorLockOnAutoRelockTimeChange(attributePath.mEndpointId, newRelockTime);
        }
        else
        {
            res = chip::Protocols::InteractionModel::Status::InvalidValue;
        }
        break;

    case chip::app::Clusters::DoorLock::Attributes::SoundVolume::Id:
        if (sizeof(uint8_t) == size)
        {
            res = emberAfPluginDoorLockOnSoundVolumeChange(attributePath.mEndpointId, *value);
        }
        else
        {
            res = chip::Protocols::InteractionModel::Status::InvalidValue;
        }
        break;

    case chip::app::Clusters::DoorLock::Attributes::OperatingMode::Id:
        if (sizeof(uint8_t) == size)
        {
            res = emberAfPluginDoorLockOnOperatingModeChange(attributePath.mEndpointId, *value);
        }
        else
        {
            res = chip::Protocols::InteractionModel::Status::InvalidValue;
        }
        break;

    case chip::app::Clusters::DoorLock::Attributes::EnableOneTouchLocking::Id:
        if (sizeof(bool) == size)
        {
            bool enable = *reinterpret_cast<bool *>(value);
            res         = emberAfPluginDoorLockOnEnableOneTouchLockingChange(attributePath.mEndpointId, enable);
        }
        else
        {
            res = chip::Protocols::InteractionModel::Status::InvalidValue;
        }
        break;

    case chip::app::Clusters::DoorLock::Attributes::EnablePrivacyModeButton::Id:
        if (sizeof(bool) == size)
        {
            bool enable = *reinterpret_cast<bool *>(value);
            res         = emberAfPluginDoorLockOnEnablePrivacyModeButtonChange(attributePath.mEndpointId, enable);
        }
        else
        {
            res = chip::Protocols::InteractionModel::Status::InvalidValue;
        }
        break;

    case chip::app::Clusters::DoorLock::Attributes::WrongCodeEntryLimit::Id:
        if (sizeof(uint8_t) == size)
        {
            res = emberAfPluginDoorLockOnWrongCodeEntryLimitChange(attributePath.mEndpointId, *value);
        }
        else
        {
            res = chip::Protocols::InteractionModel::Status::InvalidValue;
        }
        break;

    case chip::app::Clusters::DoorLock::Attributes::UserCodeTemporaryDisableTime::Id:
        if (sizeof(uint8_t) == size)
        {
            res = emberAfPluginDoorLockOnUserCodeTemporaryDisableTimeChange(attributePath.mEndpointId, *value);
        }
        else
        {
            res = chip::Protocols::InteractionModel::Status::InvalidValue;
        }
        break;

    default:
        res = emberAfPluginDoorLockOnUnhandledAttributeChange(attributePath.mEndpointId, attributeType, size, value);
        break;
    }

    return res;
}

void emberAfPluginDoorLockServerLockoutEventHandler(void) {}

void emberAfPluginDoorLockServerRelockEventHandler(void) {}

void MatterDoorLockPluginServerInitCallback()
{
    emberAfDoorLockClusterPrintln("Door Lock server initialized");
}

void MatterDoorLockClusterServerAttributeChangedCallback(const app::ConcreteAttributePath & attributePath) {}

// =============================================================================
// 'Default' callbacks for cluster commands
// =============================================================================

bool __attribute__((weak))
emberAfPluginDoorLockOnDoorLockCommand(chip::EndpointId endpointId, const Optional<ByteSpan> & pinCode, DlOperationError & err)
{
    err = DlOperationError::kUnspecified;
    return false;
}

bool __attribute__((weak))
emberAfPluginDoorLockOnDoorUnlockCommand(chip::EndpointId endpointId, const Optional<ByteSpan> & pinCode, DlOperationError & err)
{
    err = DlOperationError::kUnspecified;
    return false;
}

// =============================================================================
// 'Default' pre-change callbacks for cluster attributes
// =============================================================================

chip::Protocols::InteractionModel::Status __attribute__((weak))
emberAfPluginDoorLockOnLanguageChange(chip::EndpointId EndpointId, chip::CharSpan newLanguage)
{
    return chip::Protocols::InteractionModel::Status::Success;
}

chip::Protocols::InteractionModel::Status __attribute__((weak))
emberAfPluginDoorLockOnAutoRelockTimeChange(chip::EndpointId EndpointId, uint32_t newTime)
{
    return chip::Protocols::InteractionModel::Status::Success;
}

chip::Protocols::InteractionModel::Status __attribute__((weak))
emberAfPluginDoorLockOnSoundVolumeChange(chip::EndpointId EndpointId, uint8_t newVolume)
{
    return chip::Protocols::InteractionModel::Status::Success;
}

chip::Protocols::InteractionModel::Status __attribute__((weak))
emberAfPluginDoorLockOnOperatingModeChange(chip::EndpointId EndpointId, uint8_t newMode)
{
    return chip::Protocols::InteractionModel::Status::Success;
}

chip::Protocols::InteractionModel::Status __attribute__((weak))
emberAfPluginDoorLockOnEnableOneTouchLockingChange(chip::EndpointId EndpointId, bool enable)
{
    return chip::Protocols::InteractionModel::Status::Success;
}

chip::Protocols::InteractionModel::Status __attribute__((weak))
emberAfPluginDoorLockOnEnablePrivacyModeButtonChange(chip::EndpointId EndpointId, bool enable)
{
    return chip::Protocols::InteractionModel::Status::Success;
}

chip::Protocols::InteractionModel::Status __attribute__((weak))
emberAfPluginDoorLockOnWrongCodeEntryLimitChange(chip::EndpointId EndpointId, uint8_t newLimit)
{
    return chip::Protocols::InteractionModel::Status::Success;
}

chip::Protocols::InteractionModel::Status __attribute__((weak))
emberAfPluginDoorLockOnUserCodeTemporaryDisableTimeChange(chip::EndpointId EndpointId, uint8_t newTime)
{
    return chip::Protocols::InteractionModel::Status::Success;
}

chip::Protocols::InteractionModel::Status __attribute__((weak))
emberAfPluginDoorLockOnUnhandledAttributeChange(chip::EndpointId EndpointId, EmberAfAttributeType attrType, uint16_t attrSize,
                                                uint8_t * attrValue)
{
    return chip::Protocols::InteractionModel::Status::Success;
}

// =============================================================================
// Users and credentials access callbacks
// =============================================================================

bool __attribute__((weak))
emberAfPluginDoorLockGetUser(chip::EndpointId endpointId, uint16_t userIndex, EmberAfPluginDoorLockUserInfo & user)
{
    return false;
}

bool __attribute__((weak))
emberAfPluginDoorLockSetUser(chip::EndpointId endpointId, uint16_t userIndex, chip::FabricIndex creator, chip::FabricIndex modifier,
                             const chip::CharSpan & userName, uint32_t uniqueId, DlUserStatus userStatus, DlUserType usertype,
                             DlCredentialRule credentialRule, const DlCredential * credentials, size_t totalCredentials)
{
    return false;
}

bool __attribute__((weak))
emberAfPluginDoorLockGetCredential(chip::EndpointId endpointId, uint16_t credentialIndex, DlCredentialType credentialType,
                                   EmberAfPluginDoorLockCredentialInfo & credential)
{
    return false;
}

bool __attribute__((weak))
emberAfPluginDoorLockSetCredential(chip::EndpointId endpointId, uint16_t credentialIndex, DlCredentialStatus credentialStatus,
                                   DlCredentialType credentialType, const chip::ByteSpan & credentialData)
{
    return false;
}

DlStatus __attribute__((weak)) emberAfPluginDoorLockGetSchedule(chip::EndpointId endpointId, uint8_t weekdayIndex,
                                                                uint16_t userIndex, EmberAfPluginDoorLockWeekDaySchedule & schedule)
{
    return DlStatus::kFailure;
}

DlStatus __attribute__((weak)) emberAfPluginDoorLockGetSchedule(chip::EndpointId endpointId, uint8_t yearDayIndex,
                                                                uint16_t userIndex, EmberAfPluginDoorLockYearDaySchedule & schedule)
{
    return DlStatus::kFailure;
}

DlStatus __attribute__((weak))
emberAfPluginDoorLockSetSchedule(chip::EndpointId endpointId, uint8_t weekdayIndex, uint16_t userIndex, DlScheduleStatus status,
                                 DlDaysMaskMap daysMask, uint8_t startHour, uint8_t startMinute, uint8_t endHour, uint8_t endMinute)
{
    return DlStatus::kFailure;
}

DlStatus __attribute__((weak))
emberAfPluginDoorLockSetSchedule(chip::EndpointId endpointId, uint8_t yearDayIndex, uint16_t userIndex, DlScheduleStatus status,
                                 uint32_t localStartTime, uint32_t localEndTime)
{
    return DlStatus::kFailure;
}

// =============================================================================
// Timer callbacks
// =============================================================================

void emberAfPluginDoorLockOnAutoRelock(chip::EndpointId endpointId)
{
    emberEventControlSetInactive(&DoorLockServer::Instance().AutolockEvent);
    DoorLockServer::Instance().SetLockState(endpointId, DlLockState::kLocked, DlOperationSource::kAuto);
}
