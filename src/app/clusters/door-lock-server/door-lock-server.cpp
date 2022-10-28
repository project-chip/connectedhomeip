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
#include <app/EventLogging.h>
#include <app/server/Server.h>
#include <app/util/af-event.h>
#include <app/util/af.h>
#include <app/util/error-mapping.h>
#include <cinttypes>

#include <app/CommandHandler.h>
#include <app/ConcreteAttributePath.h>
#include <app/ConcreteCommandPath.h>
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

class DoorLockClusterFabricDelegate : public chip::FabricTable::Delegate
{
    void OnFabricRemoved(const FabricTable & fabricTable, FabricIndex fabricIndex) override
    {
        for (auto endpointId : EnabledEndpointsWithServerCluster(chip::app::Clusters::DoorLock::Id))
        {
            if (!DoorLockServer::Instance().OnFabricRemoved(endpointId, fabricIndex))
            {
                ChipLogError(Zcl,
                             "Unable to handle fabric removal from the Door Lock Server instance [endpointId=%d,fabricIndex=%d]",
                             endpointId, fabricIndex);
            }
        }
    }
};
static DoorLockClusterFabricDelegate gFabricDelegate;

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
    emberAfDoorLockClusterPrintln("Door Lock cluster initialized at endpoint #%u", endpointId);

    auto status = Attributes::LockState::SetNull(endpointId);
    if (EMBER_ZCL_STATUS_SUCCESS != status)
    {
        ChipLogError(Zcl, "[InitDoorLockServer] Unable to set the Lock State attribute to null [status=%d]", status);
    }
    SetActuatorEnabled(endpointId, true);

    for (auto & ep : mEndpointCtx)
    {
        ep.lockoutEndTimestamp    = ep.lockoutEndTimestamp.zero();
        ep.wrongCodeEntryAttempts = 0;
    }
}

bool DoorLockServer::SetLockState(chip::EndpointId endpointId, DlLockState newLockState)
{
    return SetAttribute(endpointId, Attributes::LockState::Id, Attributes::LockState::Set, newLockState);
}

bool DoorLockServer::SetLockState(chip::EndpointId endpointId, DlLockState newLockState, DlOperationSource opSource)
{
    bool success = SetLockState(endpointId, newLockState);

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

bool DoorLockServer::TrackWrongCodeEntry(chip::EndpointId endpointId)
{
    auto endpointContext = getContext(endpointId);
    if (nullptr == endpointContext)
    {
        ChipLogError(Zcl, "Failed to get endpoint index for cluster [endpoint=%d]", endpointId);
        return false;
    }

    uint8_t wrongCodeEntryLimit = 0xFF;
    auto status                 = Attributes::WrongCodeEntryLimit::Get(endpointId, &wrongCodeEntryLimit);
    if (EMBER_ZCL_STATUS_SUCCESS == status)
    {
        if (++endpointContext->wrongCodeEntryAttempts >= wrongCodeEntryLimit)
        {
            emberAfDoorLockClusterPrintln("Too many wrong code entry attempts, engaging lockout [endpoint=%d,wrongCodeAttempts=%d]",
                                          endpointId, endpointContext->wrongCodeEntryAttempts);
            engageLockout(endpointId);
        }
    }
    else if (EMBER_ZCL_STATUS_UNSUPPORTED_ATTRIBUTE != status)
    {
        ChipLogError(Zcl, "Failed to read Wrong Code Entry Limit attribute, status=0x%x", to_underlying(status));
        return false;
    }
    return true;
}

bool DoorLockServer::engageLockout(chip::EndpointId endpointId)
{
    uint8_t lockoutTimeout;

    auto endpointContext = getContext(endpointId);
    if (nullptr == endpointContext)
    {
        ChipLogError(Zcl, "Failed to get endpoint index for cluster [endpoint=%d]", endpointId);
        return false;
    }

    auto status = Attributes::UserCodeTemporaryDisableTime::Get(endpointId, &lockoutTimeout);
    if (EMBER_ZCL_STATUS_UNSUPPORTED_ATTRIBUTE == status)
    {
        return false;
    }
    if (EMBER_ZCL_STATUS_SUCCESS != status)
    {
        ChipLogError(Zcl, "Unable to read the UserCodeTemporaryDisableTime attribute [status=%d]", to_underlying(status));
        return false;
    }

    endpointContext->wrongCodeEntryAttempts = 0;
    endpointContext->lockoutEndTimestamp =
        chip::System::SystemClock().GetMonotonicTimestamp() + chip::System::Clock::Seconds32(lockoutTimeout);

    emberAfDoorLockClusterPrintln("Lockout engaged [endpointId=%d,lockoutTimeout=%d]", endpointId, lockoutTimeout);

    emberAfPluginDoorLockLockoutStarted(endpointId, endpointContext->lockoutEndTimestamp);

    return true;
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

bool DoorLockServer::GetNumberOfCredentialsSupportedPerUser(chip::EndpointId endpointId,
                                                            uint8_t & numberOfCredentialsSupportedPerUser)
{
    return GetAttribute(endpointId, Attributes::NumberOfCredentialsSupportedPerUser::Id,
                        Attributes::NumberOfCredentialsSupportedPerUser::Get, numberOfCredentialsSupportedPerUser);
}

bool DoorLockServer::GetNumberOfHolidaySchedulesSupported(chip::EndpointId endpointId, uint8_t & numberOfHolidaySchedules)
{
    return GetAttribute(endpointId, Attributes::NumberOfHolidaySchedulesSupported::Id,
                        Attributes::NumberOfHolidaySchedulesSupported::Get, numberOfHolidaySchedules);
}

bool DoorLockServer::SendLockAlarmEvent(chip::EndpointId endpointId, DlAlarmCode alarmCode)
{
    Events::DoorLockAlarm::Type event{ alarmCode };
    SendEvent(endpointId, event);

    return true;
}

namespace {
// Check whether this is valid UserStatus for a SetUser or SetCredential
// command.
bool IsValidUserStatusForSet(const Nullable<DlUserStatus> & userStatus)
{
    return userStatus.IsNull() || (userStatus.Value() == DlUserStatus::kOccupiedEnabled) ||
        (userStatus.Value() == DlUserStatus::kOccupiedDisabled);
}
} // anonymous namespace

void DoorLockServer::setUserCommandHandler(chip::app::CommandHandler * commandObj,
                                           const chip::app::ConcreteCommandPath & commandPath,
                                           const chip::app::Clusters::DoorLock::Commands::SetUser::DecodableType & commandData)
{
    auto & userIndex = commandData.userIndex;
    emberAfDoorLockClusterPrintln("[SetUser] Incoming command [endpointId=%d,userIndex=%d]", commandPath.mEndpointId, userIndex);

    if (!SupportsUSR(commandPath.mEndpointId))
    {
        emberAfDoorLockClusterPrintln("[SetUser] User management is not supported [endpointId=%d]", commandPath.mEndpointId);
        sendClusterResponse(commandObj, commandPath, EMBER_ZCL_STATUS_UNSUPPORTED_COMMAND);
        return;
    }

    auto fabricIdx = getFabricIndex(commandObj);
    if (chip::kUndefinedFabricIndex == fabricIdx)
    {
        ChipLogError(Zcl, "[SetUser] Unable to get the fabric IDX [endpointId=%d,userIndex=%d]", commandPath.mEndpointId,
                     userIndex);
        sendClusterResponse(commandObj, commandPath, EMBER_ZCL_STATUS_FAILURE);
        return;
    }

    auto sourceNodeId = getNodeId(commandObj);
    if (chip::kUndefinedNodeId == sourceNodeId)
    {
        ChipLogError(Zcl, "[SetUser] Unable to get the source node index [endpointId=%d,userIndex=%d]", commandPath.mEndpointId,
                     userIndex);
        sendClusterResponse(commandObj, commandPath, EMBER_ZCL_STATUS_FAILURE);
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
        emberAfDoorLockClusterPrintln("[SetUser] User index out of bounds [endpointId=%d,userIndex=%d]", commandPath.mEndpointId,
                                      userIndex);
        sendClusterResponse(commandObj, commandPath, EMBER_ZCL_STATUS_INVALID_COMMAND);
        return;
    }

    // appclusters, 5.2.4.34: UserName has maximum DOOR_LOCK_MAX_USER_NAME_SIZE (10) characters excluding NUL terminator in it.
    if (!userName.IsNull() && userName.Value().size() > DOOR_LOCK_MAX_USER_NAME_SIZE)
    {
        emberAfDoorLockClusterPrintln(
            "[SetUser] Unable to set user: userName too long [endpointId=%d,userIndex=%d,userNameSize=%u]", commandPath.mEndpointId,
            userIndex, static_cast<unsigned int>(userName.Value().size()));

        sendClusterResponse(commandObj, commandPath, EMBER_ZCL_STATUS_INVALID_COMMAND);
        return;
    }

    if (!IsValidUserStatusForSet(userStatus))
    {
        emberAfDoorLockClusterPrintln(
            "[SetUser] Unable to set the user: user status is out of range [endpointId=%d,userIndex=%d,userStatus=%u]",
            commandPath.mEndpointId, userIndex, to_underlying(userStatus.Value()));

        sendClusterResponse(commandObj, commandPath, EMBER_ZCL_STATUS_INVALID_COMMAND);
        return;
    }

    if (!userType.IsNull() && (userType.Value() < DlUserType::kUnrestrictedUser || userType.Value() > DlUserType::kRemoteOnlyUser))
    {
        emberAfDoorLockClusterPrintln(
            "[SetUser] Unable to set the user: user type is out of range [endpointId=%d,userIndex=%d,userType=%u]",
            commandPath.mEndpointId, userIndex, to_underlying(userType.Value()));

        sendClusterResponse(commandObj, commandPath, EMBER_ZCL_STATUS_INVALID_COMMAND);
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
    default:
        // appclusters, 5.2.4.34: SetUser command allow only kAdd/kModify, we should respond with INVALID_COMMAND if we got kClear
        // or anything else
        status = EMBER_ZCL_STATUS_INVALID_COMMAND;
        emberAfDoorLockClusterPrintln("[SetUser] Invalid operation type [endpointId=%d,operationType=%u]", commandPath.mEndpointId,
                                      to_underlying(operationType));
        break;
    }

    sendClusterResponse(commandObj, commandPath, status);
}

void DoorLockServer::getUserCommandHandler(chip::app::CommandHandler * commandObj,
                                           const chip::app::ConcreteCommandPath & commandPath, uint16_t userIndex)
{
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
                                          "[userIndex=%d,userName=\"%.*s\",userStatus=%u,userType=%u"
                                          ",credentialRule=%u,createdBy=%u,modifiedBy=%u]",
                                          userIndex, static_cast<int>(user.userName.size()), user.userName.data(),
                                          to_underlying(user.userStatus), to_underlying(user.userType),
                                          to_underlying(user.credentialRule), user.createdBy, user.lastModifiedBy);

            SuccessOrExit(err = writer->PutString(TLV::ContextTag(to_underlying(ResponseFields::kUserName)), user.userName));
            if (0xFFFFFFFFU != user.userUniqueId)
            {
                SuccessOrExit(err = writer->Put(TLV::ContextTag(to_underlying(ResponseFields::kUserUniqueId)), user.userUniqueId));
            }
            SuccessOrExit(err = writer->Put(TLV::ContextTag(to_underlying(ResponseFields::kUserStatus)), user.userStatus));
            SuccessOrExit(err = writer->Put(TLV::ContextTag(to_underlying(ResponseFields::kUserType)), user.userType));
            SuccessOrExit(err = writer->Put(TLV::ContextTag(to_underlying(ResponseFields::kCredentialRule)), user.credentialRule));
            if (!user.credentials.empty())
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
            // Append fabric IDs only if the user was created/modified by matter
            if (user.creationSource == DlAssetSource::kMatterIM)
            {
                SuccessOrExit(err =
                                  writer->Put(TLV::ContextTag(to_underlying(ResponseFields::kCreatorFabricIndex)), user.createdBy));
            }
            if (user.modificationSource == DlAssetSource::kMatterIM)
            {
                SuccessOrExit(err = writer->Put(TLV::ContextTag(to_underlying(ResponseFields::kLastModifiedFabricIndex)),
                                                user.lastModifiedBy));
            }
        }
        else
        {
            emberAfDoorLockClusterPrintln("[GetUser] User not found [userIndex=%d]", userIndex);
        }

        // appclusters, 5.2.4.36.1: We need to add next occupied user after userIndex if any.
        uint16_t nextAvailableUserIndex = 0;
        if (findOccupiedUserSlot(commandPath.mEndpointId, static_cast<uint16_t>(userIndex + 1), nextAvailableUserIndex))
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

void DoorLockServer::clearUserCommandHandler(chip::app::CommandHandler * commandObj,
                                             const chip::app::ConcreteCommandPath & commandPath, uint16_t userIndex)
{
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

void DoorLockServer::setCredentialCommandHandler(
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
        sendSetCredentialResponse(commandObj, commandPath, DlStatus::kInvalidField, 0, nextAvailableCredentialSlot);
        return;
    }

    // appclusters, 5.2.4.40.3: If the credential data length is out of bounds we should return INVALID_COMMAND
    auto status = credentialLengthWithinRange(commandPath.mEndpointId, credentialType, credentialData);
    if (DlStatus::kSuccess != status)
    {
        sendSetCredentialResponse(commandObj, commandPath, status, 0, nextAvailableCredentialSlot);
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
            sendSetCredentialResponse(commandObj, commandPath, DlStatus::kFailure, 0, nextAvailableCredentialSlot);
            return;
        }
        if (DlCredentialStatus::kAvailable != currentCredential.status && currentCredential.credentialType == credentialType &&
            currentCredential.credentialData.data_equal(credentialData))
        {
            emberAfDoorLockClusterPrintln(
                "[SetCredential] Credential with the same data and type already exist "
                "[endpointId=%d,credentialType=%u,dataLength=%u,existingCredentialIndex=%d,credentialIndex=%d]",
                commandPath.mEndpointId, to_underlying(credentialType), static_cast<unsigned int>(credentialData.size()), i,
                credentialIndex);
            sendSetCredentialResponse(commandObj, commandPath, DlStatus::kDuplicate, 0, nextAvailableCredentialSlot);
            return;
        }
    }

    EmberAfPluginDoorLockCredentialInfo existingCredential;
    if (!emberAfPluginDoorLockGetCredential(commandPath.mEndpointId, credentialIndex, credentialType, existingCredential))
    {
        emberAfDoorLockClusterPrintln(
            "[SetCredential] Unable to check if credential exists: app error [endpointId=%d,credentialIndex=%d]",
            commandPath.mEndpointId, credentialIndex);

        sendSetCredentialResponse(commandObj, commandPath, DlStatus::kFailure, 0, nextAvailableCredentialSlot);
        return;
    }

    if (!IsValidUserStatusForSet(userStatus))
    {
        emberAfDoorLockClusterPrintln("[SetCredential] Unable to set the credential: user status is out of range "
                                      "[endpointId=%d,credentialIndex=%d,userStatus=%u]",
                                      commandPath.mEndpointId, credentialIndex, to_underlying(userStatus.Value()));
        sendSetCredentialResponse(commandObj, commandPath, DlStatus::kInvalidField, 0, nextAvailableCredentialSlot);
        return;
    }

    if (!userType.IsNull() && (userType.Value() < DlUserType::kUnrestrictedUser || userType.Value() > DlUserType::kRemoteOnlyUser))
    {
        emberAfDoorLockClusterPrintln("[SetCredential] Unable to set the credential: user type is out of range "
                                      "[endpointId=%d,credentialIndex=%d,userType=%u]",
                                      commandPath.mEndpointId, credentialIndex, to_underlying(userType.Value()));
        sendSetCredentialResponse(commandObj, commandPath, DlStatus::kInvalidField, 0, nextAvailableCredentialSlot);
        return;
    }

    switch (operationType)
    {
    case DlDataOperationType::kAdd: {
        uint16_t createdUserIndex = 0;

        status = createCredential(commandPath.mEndpointId, fabricIdx, sourceNodeId, credentialIndex, credentialType,
                                  existingCredential, credentialData, userIndex, userStatus, userType, createdUserIndex);

        sendSetCredentialResponse(commandObj, commandPath, status, createdUserIndex, nextAvailableCredentialSlot);
        return;
    }
    case DlDataOperationType::kModify: {
        // appclusters, 5.2.4.41.1: should send the INVALID_COMMAND in the response when the credential is in use
        if (DlCredentialStatus::kAvailable == existingCredential.status)
        {
            emberAfDoorLockClusterPrintln("[SetCredential] Unable to modify the credential: credential slot is not occupied "
                                          "[endpointId=%d,credentialIndex=%d]",
                                          commandPath.mEndpointId, credentialIndex);

            sendSetCredentialResponse(commandObj, commandPath, DlStatus::kInvalidField, 0, nextAvailableCredentialSlot);
            return;
        }

        // if userIndex is NULL then we're changing the programming user PIN
        if (userIndex.IsNull())
        {
            status = modifyProgrammingPIN(commandPath.mEndpointId, fabricIdx, sourceNodeId, credentialIndex, credentialType,
                                          existingCredential, credentialData);
            sendSetCredentialResponse(commandObj, commandPath, status, 0, nextAvailableCredentialSlot);
            return;
        }

        status = modifyCredential(commandPath.mEndpointId, fabricIdx, sourceNodeId, credentialIndex, credentialType,
                                  existingCredential, credentialData, userIndex.Value(), userStatus, userType);
        sendSetCredentialResponse(commandObj, commandPath, status, 0, nextAvailableCredentialSlot);
        return;
    }
    case DlDataOperationType::kClear:
    default:
        // appclusters, 5.2.4.40: set credential command supports only Add and Modify operational type.
        sendSetCredentialResponse(commandObj, commandPath, DlStatus::kInvalidField, 0, nextAvailableCredentialSlot);
    }
}

void DoorLockServer::getCredentialStatusCommandHandler(chip::app::CommandHandler * commandObj,
                                                       const chip::app::ConcreteCommandPath & commandPath,
                                                       DlCredentialType credentialType, uint16_t credentialIndex)
{
    emberAfDoorLockClusterPrintln("[GetCredentialStatus] Incoming command [endpointId=%d]", commandPath.mEndpointId);

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
        sendGetCredentialResponse(commandObj, commandPath, credentialType, credentialIndex, 0, nullptr, false);
        return;
    }

    EmberAfPluginDoorLockCredentialInfo credentialInfo;
    if (!emberAfPluginDoorLockGetCredential(commandPath.mEndpointId, credentialIndex, credentialType, credentialInfo))
    {
        emberAfDoorLockClusterPrintln("[GetCredentialStatus] Unable to get the credential: app error "
                                      "[endpointId=%d,credentialIndex=%d,credentialType=%u,creator=%u,modifier=%u]",
                                      commandPath.mEndpointId, credentialIndex, to_underlying(credentialType),
                                      credentialInfo.createdBy, credentialInfo.lastModifiedBy);
        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_FAILURE);
        return;
    }

    bool credentialExists            = DlCredentialStatus::kAvailable != credentialInfo.status;
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

    sendGetCredentialResponse(commandObj, commandPath, credentialType, credentialIndex, userIndexWithCredential, &credentialInfo,
                              credentialExists);
}

void DoorLockServer::sendGetCredentialResponse(chip::app::CommandHandler * commandObj,
                                               const chip::app::ConcreteCommandPath & commandPath, DlCredentialType credentialType,
                                               uint16_t credentialIndex, uint16_t userIndexWithCredential,
                                               EmberAfPluginDoorLockCredentialInfo * credentialInfo, bool credentialExists)
{
    Commands::GetCredentialStatusResponse::Type response{ .credentialExists = credentialExists };
    if (credentialExists && !(nullptr == credentialInfo))
    {
        if (0 != userIndexWithCredential)
        {
            response.userIndex.SetNonNull(userIndexWithCredential);
        }
        if (credentialInfo->creationSource == DlAssetSource::kMatterIM)
        {
            response.creatorFabricIndex.SetNonNull(credentialInfo->createdBy);
        }
        if (credentialInfo->modificationSource == DlAssetSource::kMatterIM)
        {
            response.lastModifiedFabricIndex.SetNonNull(credentialInfo->lastModifiedBy);
        }
    }
    else
    {
        response.userIndex.SetNull();
    }
    uint16_t nextCredentialIndex = 0;
    if (findOccupiedCredentialSlot(commandPath.mEndpointId, credentialType, static_cast<uint16_t>(credentialIndex + 1),
                                   nextCredentialIndex))
    {
        response.nextCredentialIndex.SetNonNull(nextCredentialIndex);
    }
    commandObj->AddResponse(commandPath, response);

    emberAfDoorLockClusterPrintln("[GetCredentialStatus] Prepared credential status "
                                  "[endpointId=%d,credentialType=%u,credentialIndex=%d,userIndex=%d,nextCredentialIndex=%d]",
                                  commandPath.mEndpointId, to_underlying(credentialType), credentialIndex, userIndexWithCredential,
                                  nextCredentialIndex);
}

void DoorLockServer::clearCredentialCommandHandler(
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

void DoorLockServer::setWeekDayScheduleCommandHandler(chip::app::CommandHandler * commandObj,
                                                      const chip::app::ConcreteCommandPath & commandPath, uint8_t weekDayIndex,
                                                      uint16_t userIndex, const chip::BitMask<DlDaysMaskMap> & daysMask,
                                                      uint8_t startHour, uint8_t startMinute, uint8_t endHour, uint8_t endMinute)
{
    auto endpointId = commandPath.mEndpointId;
    if (!SupportsWeekDaySchedules(endpointId))
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

    if (!weekDayIndexValid(endpointId, weekDayIndex) || !userIndexValid(endpointId, userIndex))
    {
        emberAfDoorLockClusterPrintln(
            "[SetWeekDaySchedule] Unable to add schedule - index out of range [endpointId=%d,weekDayIndex=%d,userIndex=%d]",
            endpointId, weekDayIndex, userIndex);
        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_INVALID_COMMAND);
        return;
    }

    if (!userExists(endpointId, userIndex))
    {
        emberAfDoorLockClusterPrintln("[SetWeekDaySchedule] Unable to add schedule - user does not exist "
                                      "[endpointId=%d,weekDayIndex=%d,userIndex=%d]",
                                      endpointId, weekDayIndex, userIndex);
        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_FAILURE);
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

    // TODO: Check that bits are within range
    if (setBitsInDaysMask == 0 || setBitsInDaysMask > 1)
    {
        emberAfDoorLockClusterPrintln("[SetWeekDaySchedule] Unable to add schedule - daysMask is out of range "
                                      "[endpointId=%d,weekDayIndex=%d,userIndex=%d,daysMask=%x]",
                                      endpointId, weekDayIndex, userIndex, daysMask.Raw());
        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_INVALID_COMMAND);
        return;
    }

    if (startHour > DOOR_LOCK_SCHEDULE_MAX_HOUR || startMinute > DOOR_LOCK_SCHEDULE_MAX_MINUTE ||
        endHour > DOOR_LOCK_SCHEDULE_MAX_HOUR || endMinute > DOOR_LOCK_SCHEDULE_MAX_MINUTE)
    {
        emberAfDoorLockClusterPrintln("[SetWeekDaySchedule] Unable to add schedule - start time out of range "
                                      "[endpointId=%d,weekDayIndex=%d,userIndex=%d,startTime=\"%d:%d\",endTime=\"%d:%d\"]",
                                      endpointId, weekDayIndex, userIndex, startHour, startMinute, endHour, endMinute);
        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_INVALID_COMMAND);
        return;
    }

    if (startHour > endHour || (startHour == endHour && startMinute >= endMinute))
    {
        emberAfDoorLockClusterPrintln("[SetWeekDaySchedule] Unable to add schedule - invalid time "
                                      "[endpointId=%d,weekDayIndex=%d,userIndex=%d,startTime=\"%d:%d\",endTime=\"%d:%d\"]",
                                      endpointId, weekDayIndex, userIndex, startHour, startMinute, endHour, endMinute);
        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_INVALID_COMMAND);
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

void DoorLockServer::getWeekDayScheduleCommandHandler(chip::app::CommandHandler * commandObj,
                                                      const chip::app::ConcreteCommandPath & commandPath, uint8_t weekDayIndex,
                                                      uint16_t userIndex)
{
    auto endpointId = commandPath.mEndpointId;
    if (!SupportsWeekDaySchedules(endpointId))
    {
        emberAfDoorLockClusterPrintln("[GetWeekDaySchedule] Ignore command (not supported) [endpointId=%d]", endpointId);
        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_INVALID_COMMAND);
        return;
    }

    emberAfDoorLockClusterPrintln("[GetWeekDaySchedule] Incoming command [endpointId=%d]", endpointId);

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
        sendGetWeekDayScheduleResponse(commandObj, commandPath, weekDayIndex, userIndex, DlStatus::kFailure);
        return;
    }

    EmberAfPluginDoorLockWeekDaySchedule scheduleInfo{};
    auto status = emberAfPluginDoorLockGetSchedule(endpointId, weekDayIndex, userIndex, scheduleInfo);
    if (DlStatus::kSuccess != status)
    {
        sendGetWeekDayScheduleResponse(commandObj, commandPath, weekDayIndex, userIndex, status);
        return;
    }

    sendGetWeekDayScheduleResponse(commandObj, commandPath, weekDayIndex, userIndex, DlStatus::kSuccess, scheduleInfo.daysMask,
                                   scheduleInfo.startHour, scheduleInfo.startMinute, scheduleInfo.endHour, scheduleInfo.endMinute);
}

void DoorLockServer::clearWeekDayScheduleCommandHandler(chip::app::CommandHandler * commandObj,
                                                        const chip::app::ConcreteCommandPath & commandPath, uint8_t weekDayIndex,
                                                        uint16_t userIndex)
{
    auto endpointId = commandPath.mEndpointId;
    if (!SupportsWeekDaySchedules(endpointId))
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
        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_FAILURE);
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

void DoorLockServer::setYearDayScheduleCommandHandler(chip::app::CommandHandler * commandObj,
                                                      const chip::app::ConcreteCommandPath & commandPath, uint8_t yearDayIndex,
                                                      uint16_t userIndex, uint32_t localStartTime, uint32_t localEndTime)
{
    auto endpointId = commandPath.mEndpointId;
    if (!SupportsYearDaySchedules(endpointId))
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

    if (!yearDayIndexValid(endpointId, yearDayIndex) || !userIndexValid(endpointId, userIndex))
    {
        emberAfDoorLockClusterPrintln(
            "[SetYearDaySchedule] Unable to add schedule - index out of range [endpointId=%d,yearDayIndex=%d,userIndex=%d]",
            endpointId, yearDayIndex, userIndex);
        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_INVALID_COMMAND);
        return;
    }

    if (!userExists(endpointId, userIndex))
    {
        emberAfDoorLockClusterPrintln("[SetYearDaySchedule] Unable to add schedule - user does not exist "
                                      "[endpointId=%d,yearDayIndex=%d,userIndex=%d]",
                                      endpointId, yearDayIndex, userIndex);
        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_FAILURE);
        return;
    }

    if (localEndTime <= localStartTime)
    {
        emberAfDoorLockClusterPrintln("[SetYearDaySchedule] Unable to add schedule - schedule ends earlier than starts"
                                      "[endpointId=%d,yearDayIndex=%d,userIndex=%d,localStarTime=%" PRIu32 ",localEndTime=%" PRIu32
                                      "]",
                                      endpointId, yearDayIndex, userIndex, localStartTime, localEndTime);
        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_INVALID_COMMAND);
        return;
    }

    auto status = emberAfPluginDoorLockSetSchedule(endpointId, yearDayIndex, userIndex, DlScheduleStatus::kOccupied, localStartTime,
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
                                  endpointId, yearDayIndex, userIndex, localStartTime, localEndTime);

    sendRemoteLockUserChange(endpointId, DlLockDataType::kYearDaySchedule, DlDataOperationType::kAdd, sourceNodeId, fabricIdx,
                             userIndex, static_cast<uint16_t>(yearDayIndex));

    emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_SUCCESS);
}

void DoorLockServer::getYearDayScheduleCommandHandler(chip::app::CommandHandler * commandObj,
                                                      const chip::app::ConcreteCommandPath & commandPath, uint8_t yearDayIndex,
                                                      uint16_t userIndex)
{
    auto endpointId = commandPath.mEndpointId;
    if (!SupportsYearDaySchedules(endpointId))
    {
        emberAfDoorLockClusterPrintln("[GetYearDaySchedule] Ignore command (not supported) [endpointId=%d]", endpointId);
        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_INVALID_COMMAND);
        return;
    }
    emberAfDoorLockClusterPrintln("[GetYearDaySchedule] incoming command [endpointId=%d]", endpointId);

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
        sendGetYearDayScheduleResponse(commandObj, commandPath, yearDayIndex, userIndex, DlStatus::kFailure);
        return;
    }

    EmberAfPluginDoorLockYearDaySchedule scheduleInfo{};
    auto status = emberAfPluginDoorLockGetSchedule(endpointId, yearDayIndex, userIndex, scheduleInfo);
    if (DlStatus::kSuccess != status)
    {
        sendGetYearDayScheduleResponse(commandObj, commandPath, yearDayIndex, userIndex, status);
        return;
    }

    sendGetYearDayScheduleResponse(commandObj, commandPath, yearDayIndex, userIndex, DlStatus::kSuccess,
                                   scheduleInfo.localStartTime, scheduleInfo.localEndTime);
}

void DoorLockServer::clearYearDayScheduleCommandHandler(chip::app::CommandHandler * commandObj,
                                                        const chip::app::ConcreteCommandPath & commandPath, uint8_t yearDayIndex,
                                                        uint16_t userIndex)
{
    auto endpointId = commandPath.mEndpointId;
    if (!SupportsYearDaySchedules(endpointId))
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
        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_FAILURE);
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

chip::BitFlags<DoorLockFeature> DoorLockServer::GetFeatures(chip::EndpointId endpointId)
{
    chip::BitFlags<DoorLockFeature> featureMap;
    if (!GetAttribute(endpointId, Attributes::FeatureMap::Id, Attributes::FeatureMap::Get, *featureMap.RawStorage()))
    {
        ChipLogError(Zcl, "Unable to get the door lock feature map: attribute read error");
        featureMap.ClearAll();
    }
    return featureMap;
}

bool DoorLockServer::OnFabricRemoved(chip::EndpointId endpointId, chip::FabricIndex fabricIndex)
{
    emberAfDoorLockClusterPrintln(
        "[OnFabricRemoved] Handling a fabric removal from the door lock server [endpointId=%d,fabricIndex=%d]", endpointId,
        fabricIndex);

    // Iterate over all the users and clean up the deleted fabric
    if (!clearFabricFromUsers(endpointId, fabricIndex))
    {
        ChipLogError(Zcl, "[OnFabricRemoved] Unable to cleanup fabric from users - internal error [endpointId=%d,fabricIndex=%d]",
                     endpointId, fabricIndex);
        return false;
    }

    // Iterate over all the credentials and clean up the fabrics
    if (!clearFabricFromCredentials(endpointId, fabricIndex))
    {
        ChipLogError(Zcl,
                     "[OnFabricRemoved] Unable to cleanup fabric from credentials - internal error [endpointId=%d,fabricIndex=%d]",
                     endpointId, fabricIndex);
        return false;
    }

    return true;
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

DlStatus DoorLockServer::credentialLengthWithinRange(chip::EndpointId endpointId, DlCredentialType type,
                                                     const chip::ByteSpan & credentialData)
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
        return DlStatus::kFailure;
    }

    if (!statusMin || !statusMax)
    {
        ChipLogError(Zcl, "Unable to read attributes to get min/max length for credentials [endpointId=%d,credentialType=%u]",
                     endpointId, to_underlying(type));
        return DlStatus::kFailure;
    }

    if (credentialData.size() < minLen || credentialData.size() > maxLen)
    {
        emberAfDoorLockClusterPrintln("Credential data size is out of range "
                                      "[endpointId=%d,credentialType=%u,minLength=%u,maxLength=%u,length=%u]",
                                      endpointId, to_underlying(type), minLen, maxLen,
                                      static_cast<unsigned int>(credentialData.size()));
        return DlStatus::kInvalidField;
    }

    return DlStatus::kSuccess;
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

bool DoorLockServer::findOccupiedUserSlot(chip::EndpointId endpointId, uint16_t startIndex, uint16_t & userIndex)
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

        if (DlUserStatus::kAvailable != user.userStatus)
        {
            userIndex = i;
            return true;
        }
    }
    return false;
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

bool DoorLockServer::findOccupiedCredentialSlot(chip::EndpointId endpointId, DlCredentialType credentialType, uint16_t startIndex,
                                                uint16_t & credentialIndex)
{
    uint16_t maxNumberOfCredentials = 0;
    if (!getMaxNumberOfCredentials(endpointId, credentialType, maxNumberOfCredentials))
    {
        return false;
    }

    // Programming PIN index starts with 0, and it is assumed that it is unique. Therefore different bounds checking for that
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

        if (DlCredentialStatus::kAvailable != info.status)
        {
            credentialIndex = i;
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

        for (size_t j = 0; j < user.credentials.size(); ++j)
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
                                               chip::ByteSpan credentialData, uint16_t & userIndex, uint16_t & credentialIndex,
                                               EmberAfPluginDoorLockUserInfo & userInfo)
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
                userInfo        = user;
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
        return static_cast<EmberAfStatus>(DlStatus::kOccupied);
    }

    const auto & newUserName            = !userName.IsNull() ? userName.Value() : chip::CharSpan::fromCharString("");
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
        emberAfDoorLockClusterPrintln("[createUser] Unable to create user: app error "
                                      "[endpointId=%d,creatorFabricId=%d,userIndex=%d,userName=\"%.*s\",userUniqueId=0x%" PRIx32
                                      ",userStatus=%u,"
                                      "userType=%u,credentialRule=%u,totalCredentials=%u]",
                                      endpointId, creatorFabricIdx, userIndex, static_cast<int>(newUserName.size()),
                                      newUserName.data(), newUserUniqueId, to_underlying(newUserStatus), to_underlying(newUserType),
                                      to_underlying(newCredentialRule), static_cast<unsigned int>(newTotalCredentials));
        return EMBER_ZCL_STATUS_FAILURE;
    }

    emberAfDoorLockClusterPrintln("[createUser] User created "
                                  "[endpointId=%d,creatorFabricId=%d,userIndex=%d,userName=\"%.*s\",userUniqueId=0x%" PRIx32
                                  ",userStatus=%u,"
                                  "userType=%u,credentialRule=%u,totalCredentials=%u]",
                                  endpointId, creatorFabricIdx, userIndex, static_cast<int>(newUserName.size()), newUserName.data(),
                                  newUserUniqueId, to_underlying(newUserStatus), to_underlying(newUserType),
                                  to_underlying(newCredentialRule), static_cast<unsigned int>(newTotalCredentials));

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
                     "[endpointId=%d,modifierFabric=%d,userIndex=%d,userName=\"%.*s\",userUniqueId=0x%" PRIx32 ",userStatus=%u"
                     ",userType=%u,credentialRule=%u]",
                     endpointId, modifierFabricIndex, userIndex, static_cast<int>(newUserName.size()), newUserName.data(),
                     newUserUniqueId, to_underlying(newUserStatus), to_underlying(newUserType), to_underlying(newCredentialRule));
        return EMBER_ZCL_STATUS_FAILURE;
    }

    emberAfDoorLockClusterPrintln("[modifyUser] User modified "
                                  "[endpointId=%d,modifierFabric=%d,userIndex=%d,userName=\"%.*s\",userUniqueId=0x%" PRIx32
                                  ",userStatus=%u,userType=%u,credentialRule=%u]",
                                  endpointId, modifierFabricIndex, userIndex, static_cast<int>(newUserName.size()),
                                  newUserName.data(), newUserUniqueId, to_underlying(newUserStatus), to_underlying(newUserType),
                                  to_underlying(newCredentialRule));

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

        if (!emberAfPluginDoorLockSetCredential(endpointId, credential.CredentialIndex, kUndefinedFabricIndex,
                                                kUndefinedFabricIndex, DlCredentialStatus::kAvailable,
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

bool DoorLockServer::clearFabricFromUsers(chip::EndpointId endpointId, chip::FabricIndex fabricIndex)
{
    uint16_t maxNumberOfUsers;
    VerifyOrReturnError(GetAttribute(endpointId, Attributes::NumberOfTotalUsersSupported::Id,
                                     Attributes::NumberOfTotalUsersSupported::Get, maxNumberOfUsers),
                        false);

    for (uint16_t userIndex = 1; userIndex <= maxNumberOfUsers; ++userIndex)
    {
        EmberAfPluginDoorLockUserInfo user;
        if (!emberAfPluginDoorLockGetUser(endpointId, userIndex, user))
        {
            ChipLogError(Zcl,
                         "[OnFabricRemoved] Unable to get the user - internal error [endpointId=%d,fabricIndex=%d,userIndex=%d]",
                         endpointId, fabricIndex, userIndex);
            continue;
        }

        // Filter out unoccupied slots and users that don't have corresponding fabricIndex in the created/modified fields
        if (DlUserStatus::kAvailable == user.userStatus || (fabricIndex != user.createdBy && fabricIndex != user.lastModifiedBy))
        {
            continue;
        }

        if (user.createdBy == fabricIndex)
        {
            user.createdBy = kUndefinedFabricIndex;
        }

        if (user.lastModifiedBy == fabricIndex)
        {
            user.lastModifiedBy = kUndefinedFabricIndex;
        }

        if (!emberAfPluginDoorLockSetUser(endpointId, userIndex, user.createdBy, user.lastModifiedBy, user.userName,
                                          user.userUniqueId, user.userStatus, user.userType, user.credentialRule,
                                          user.credentials.data(), user.credentials.size()))
        {
            ChipLogError(
                Zcl,
                "[OnFabricRemoved] Unable to update the user fabrics - internal error [endpointId=%d,fabricIndex=%d,userIndex=%d]",
                endpointId, fabricIndex, userIndex);
        }
    }
    return true;
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

    if (!emberAfPluginDoorLockSetCredential(endpointId, credential.CredentialIndex, creatorFabricIdx, creatorFabricIdx,
                                            DlCredentialStatus::kOccupied, static_cast<DlCredentialType>(credential.CredentialType),
                                            credentialData))
    {
        emberAfDoorLockClusterPrintln("[SetCredential] Unable to set the credential: app error "
                                      "[endpointId=%d,credentialIndex=%d,credentialType=%u,dataLength=%u]",
                                      endpointId, credential.CredentialIndex, credential.CredentialType,
                                      static_cast<unsigned int>(credentialData.size()));
        return DlStatus::kFailure;
    }

    emberAfDoorLockClusterPrintln("[SetCredential] Credential and user were created "
                                  "[endpointId=%d,credentialIndex=%d,credentialType=%u,dataLength=%u,userIndex=%d]",
                                  endpointId, credential.CredentialIndex, credential.CredentialType,
                                  static_cast<unsigned int>(credentialData.size()), availableUserIndex);
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
        emberAfDoorLockClusterPrintln("[SetCredential] Unable to add credential to user: user slot is empty "
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

    if (!emberAfPluginDoorLockSetCredential(endpointId, credential.CredentialIndex, modifierFabricIdx, modifierFabricIdx,
                                            DlCredentialStatus::kOccupied, static_cast<DlCredentialType>(credential.CredentialType),
                                            credentialData))
    {
        emberAfDoorLockClusterPrintln("[SetCredential] Unable to set the credential: app error "
                                      "[endpointId=%d,credentialIndex=%d,credentialType=%u,dataLength=%u]",
                                      endpointId, credential.CredentialIndex, credential.CredentialType,
                                      static_cast<unsigned int>(credentialData.size()));
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

    for (size_t i = 0; i < user.credentials.size(); ++i)
    {
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

    uint8_t maxCredentialsPerUser;
    if (!GetNumberOfCredentialsSupportedPerUser(endpointId, maxCredentialsPerUser))
    {
        ChipLogError(Zcl,
                     "[AddCredentialToUser] Unable to get the number of available credentials per user: internal error "
                     "[endpointId=%d,userIndex=%d,credentialType=%d,credentialIndex=%d]",
                     endpointId, userIndex, credential.CredentialType, credential.CredentialIndex);
        return DlStatus::kFailure;
    }

    // appclusters: spec defines up to NumberOfCredentialsSupportedPerUser credentials per user
    if (user.credentials.size() + 1 > maxCredentialsPerUser)
    {
        emberAfDoorLockClusterPrintln("[AddCredentialToUser] Unable to add credentials to user: too many credentials "
                                      "[endpointId=%d,userIndex=%d,userTotalCredentials=%u]",
                                      endpointId, userIndex, static_cast<unsigned int>(user.credentials.size()));
        return DlStatus::kResourceExhausted;
    }

    chip::Platform::ScopedMemoryBuffer<DlCredential> newCredentials;
    if (!newCredentials.Alloc(user.credentials.size() + 1))
    {
        ChipLogError(Zcl,
                     "[AddCredentialToUser] Unable to allocate the buffer for credentials "
                     "[endpointId=%d,userIndex=%d,userTotalCredentials=%u]",
                     endpointId, userIndex, static_cast<unsigned int>(user.credentials.size()));
        return DlStatus::kFailure;
    }

    memcpy(newCredentials.Get(), user.credentials.data(), sizeof(DlCredential) * user.credentials.size());
    newCredentials[user.credentials.size()] = credential;

    if (!emberAfPluginDoorLockSetUser(endpointId, userIndex, user.createdBy, modifierFabricIdx, user.userName, user.userUniqueId,
                                      user.userStatus, user.userType, user.credentialRule, newCredentials.Get(),
                                      user.credentials.size() + 1))
    {
        emberAfDoorLockClusterPrintln(
            "[AddCredentialToUser] Unable to add credential to user: credential with this index is already associated "
            "with user [endpointId=%d,userIndex=%d,credentialType=%d,credentialIndex=%d,userTotalCredentials=%u]",
            endpointId, userIndex, credential.CredentialType, credential.CredentialIndex,
            static_cast<unsigned int>(user.credentials.size()));
        return DlStatus::kFailure;
    }

    emberAfDoorLockClusterPrintln("[AddCredentialToUser] Credential added to user "
                                  "[endpointId=%d,userIndex=%d,credentialType=%d,credentialIndex=%d,userTotalCredentials=%u]",
                                  endpointId, userIndex, credential.CredentialType, credential.CredentialIndex,
                                  static_cast<unsigned int>(user.credentials.size() + 1));

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

    for (size_t i = 0; i < user.credentials.size(); ++i)
    {
        // appclusters, 5.2.4.40: user should already be associated with given credentialIndex
        if (user.credentials.data()[i].CredentialIndex == credential.CredentialIndex)
        {
            chip::Platform::ScopedMemoryBuffer<DlCredential> newCredentials;
            if (!newCredentials.Alloc(user.credentials.size()))
            {
                ChipLogError(Zcl,
                             "[ModifyUserCredential] Unable to allocate the buffer for credentials "
                             "[endpointId=%d,userIndex=%d,userTotalCredentials=%u,credentialType=%d,credentialIndex=%d]",
                             endpointId, userIndex, static_cast<unsigned int>(user.credentials.size()), credential.CredentialType,
                             credential.CredentialIndex);
                return DlStatus::kFailure;
            }
            memcpy(newCredentials.Get(), user.credentials.data(), sizeof(DlCredential) * user.credentials.size());
            newCredentials[i] = credential;

            emberAfDoorLockClusterPrintln(
                "[ModifyUserCredential] Unable to add credential to user: credential with this index is already associated "
                "[endpointId=%d,userIndex=%d,credentialType=%d,credentialIndex=%d]",
                endpointId, userIndex, credential.CredentialType, credential.CredentialIndex);

            if (!emberAfPluginDoorLockSetUser(endpointId, userIndex, user.createdBy, modifierFabricIdx, user.userName,
                                              user.userUniqueId, user.userStatus, user.userType, user.credentialRule,
                                              newCredentials.Get(), user.credentials.size()))
            {
                emberAfDoorLockClusterPrintln(
                    "[ModifyUserCredential] Unable to modify user credential: credential with this index is already associated "
                    "with user [endpointId=%d,userIndex=%d,credentialType=%d,credentialIndex=%d,userTotalCredentials=%u]",
                    endpointId, userIndex, credential.CredentialType, credential.CredentialIndex,
                    static_cast<unsigned int>(user.credentials.size()));
                return DlStatus::kFailure;
            }

            emberAfDoorLockClusterPrintln(
                "[ModifyUserCredential] User credential modified "
                "[endpointId=%d,userIndex=%d,credentialType=%d,credentialIndex=%d,userTotalCredentials=%u]",
                endpointId, userIndex, credential.CredentialType, credential.CredentialIndex,
                static_cast<unsigned int>(user.credentials.size()));

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
                                          const Nullable<DlUserStatus> & userStatus, Nullable<DlUserType> userType,
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

    if (!emberAfPluginDoorLockSetCredential(endpointId, credentialIndex, existingCredential.createdBy, modifierFabricIndex,
                                            existingCredential.status, existingCredential.credentialType, credentialData))
    {
        emberAfDoorLockClusterPrintln("[SetCredential] Unable to modify the credential: app error "
                                      "[endpointId=%d,credentialIndex=%d,credentialType=%u,credentialDataSize=%u]",
                                      endpointId, credentialIndex, to_underlying(credentialType),
                                      static_cast<unsigned int>(credentialData.size()));
        return DlStatus::kFailure;
    }

    emberAfDoorLockClusterPrintln("[SetCredential] Successfully         modified the credential "
                                  "[endpointId=%d,credentialIndex=%d,credentialType=%u,credentialDataSize=%u]",
                                  endpointId, credentialIndex, to_underlying(credentialType),
                                  static_cast<unsigned int>(credentialData.size()));

    sendRemoteLockUserChange(endpointId, credentialTypeToLockDataType(credentialType), DlDataOperationType::kModify, sourceNodeId,
                             modifierFabricIndex, relatedUserIndex, credentialIndex);

    return DlStatus::kSuccess;
}

DlStatus DoorLockServer::modifyCredential(chip::EndpointId endpointId, chip::FabricIndex modifierFabricIndex,
                                          chip::NodeId sourceNodeId, uint16_t credentialIndex, DlCredentialType credentialType,
                                          const EmberAfPluginDoorLockCredentialInfo & existingCredential,
                                          const chip::ByteSpan & credentialData, uint16_t userIndex,
                                          const Nullable<DlUserStatus> & userStatus, Nullable<DlUserType> userType)
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
        if (!emberAfPluginDoorLockSetCredential(endpointId, credentialIndex, existingCredential.createdBy, modifierFabricIndex,
                                                existingCredential.status, existingCredential.credentialType, credentialData))
        {
            emberAfDoorLockClusterPrintln("[SetCredential] Unable to modify the credential: app error "
                                          "[endpointId=%d,credentialIndex=%d,credentialType=%u,credentialDataSize=%u]",
                                          endpointId, credentialIndex, to_underlying(credentialType),
                                          static_cast<unsigned int>(credentialData.size()));

            return DlStatus::kFailure;
        }

        emberAfDoorLockClusterPrintln("[SetCredential] Successfully modified the credential "
                                      "[endpointId=%d,credentialIndex=%d,credentialType=%u,credentialDataSize=%u]",
                                      endpointId, credentialIndex, to_underlying(credentialType),
                                      static_cast<unsigned int>(credentialData.size()));

        sendRemoteLockUserChange(endpointId, credentialTypeToLockDataType(credentialType), DlDataOperationType::kModify,
                                 sourceNodeId, modifierFabricIndex, userIndex, credentialIndex);
    }
    return status;
}

void DoorLockServer::sendSetCredentialResponse(chip::app::CommandHandler * commandObj,
                                               const chip::app::ConcreteCommandPath & commandPath, DlStatus status,
                                               uint16_t userIndex, uint16_t nextCredentialIndex)
{
    Commands::SetCredentialResponse::Type command{};
    command.status = status;
    if (0 != userIndex)
    {
        command.userIndex.SetNonNull(userIndex);
    }
    if (0 != nextCredentialIndex)
    {
        command.nextCredentialIndex.SetNonNull(nextCredentialIndex);
    }
    commandObj->AddResponse(commandPath, command);
}

bool DoorLockServer::credentialTypeSupported(chip::EndpointId endpointId, DlCredentialType type)
{
    switch (type)
    {
    case DlCredentialType::kProgrammingPIN:
    case DlCredentialType::kPin:
        return SupportsPIN(endpointId);
    case DlCredentialType::kRfid:
        return SupportsRFID(endpointId);
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

    // No need to clear the holiday schedules here as they are not specific for user.
    return DlStatus::kSuccess;
}

void DoorLockServer::sendGetWeekDayScheduleResponse(chip::app::CommandHandler * commandObj,
                                                    const chip::app::ConcreteCommandPath & commandPath, uint8_t weekdayIndex,
                                                    uint16_t userIndex, DlStatus status, DlDaysMaskMap daysMask, uint8_t startHour,
                                                    uint8_t startMinute, uint8_t endHour, uint8_t endMinute)
{
    VerifyOrDie(nullptr != commandObj);

    Commands::GetWeekDayScheduleResponse::Type response;
    response.weekDayIndex = weekdayIndex;
    response.userIndex    = userIndex;
    response.status       = status;
    if (DlStatus::kSuccess == status)
    {
        response.daysMask    = Optional<chip::BitMask<DlDaysMaskMap>>(daysMask);
        response.startHour   = Optional<uint8_t>(startHour);
        response.startMinute = Optional<uint8_t>(startMinute);
        response.endHour     = Optional<uint8_t>(endHour);
        response.endMinute   = Optional<uint8_t>(endMinute);
    }

    commandObj->AddResponse(commandPath, response);
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
    uint8_t yearDaySchedulesPerUser = 0;
    if (!GetNumberOfYearDaySchedulesPerUserSupported(endpointId, yearDaySchedulesPerUser))
    {
        return DlStatus::kFailure;
    }

    for (uint8_t i = 1; i <= yearDaySchedulesPerUser; ++i)
    {
        auto status = clearYearDaySchedule(endpointId, userIndex, i);
        if (DlStatus::kSuccess != status)
        {
            return status;
        }
    }
    return DlStatus::kSuccess;
}

void DoorLockServer::sendGetYearDayScheduleResponse(chip::app::CommandHandler * commandObj,
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

    commandObj->AddResponse(commandPath, response);
}

bool DoorLockServer::holidayIndexValid(chip::EndpointId endpointId, uint8_t holidayIndex)
{
    uint8_t holidaysSupported;
    if (!GetNumberOfHolidaySchedulesSupported(endpointId, holidaysSupported))
    {
        return false;
    }

    // appclusters, 5.2.4.22-25: year day index changes from 1 to maxNumberOfHolidaySchedules
    if (0 == holidayIndex || holidayIndex > holidaysSupported)
    {
        return false;
    }
    return true;
}

DlStatus DoorLockServer::clearHolidaySchedule(chip::EndpointId endpointId, uint8_t holidayIndex)
{
    auto status =
        emberAfPluginDoorLockSetSchedule(endpointId, holidayIndex, DlScheduleStatus::kAvailable, 0, 0, DlOperatingMode::kNormal);
    if (DlStatus::kSuccess != status && DlStatus::kNotFound != status)
    {
        ChipLogError(
            Zcl, "[ClearHolidaySchedule] Unable to clear the schedule - internal error [endpointId=%d,scheduleIndex=%d,status=%u]",
            endpointId, holidayIndex, to_underlying(status));
        return status;
    }
    return DlStatus::kSuccess;
}

DlStatus DoorLockServer::clearHolidaySchedules(chip::EndpointId endpointId)
{
    uint8_t totalHolidaySchedules = 0;
    if (!GetNumberOfHolidaySchedulesSupported(endpointId, totalHolidaySchedules))
    {
        return DlStatus::kFailure;
    }

    for (uint8_t i = 1; i <= totalHolidaySchedules; ++i)
    {
        auto status = clearHolidaySchedule(endpointId, i);
        if (DlStatus::kSuccess != status)
        {
            return status;
        }
    }
    return DlStatus::kSuccess;
}

void DoorLockServer::sendHolidayScheduleResponse(chip::app::CommandHandler * commandObj,
                                                 const chip::app::ConcreteCommandPath & commandPath, uint8_t holidayIndex,
                                                 DlStatus status, uint32_t localStartTime, uint32_t localEndTime,
                                                 DlOperatingMode operatingMode)
{
    VerifyOrDie(nullptr != commandObj);

    auto response = Commands::GetHolidayScheduleResponse::Type{ holidayIndex, status };
    if (DlStatus::kSuccess == status)
    {
        response.localStartTime = Optional<uint32_t>(localStartTime);
        response.localEndTime   = Optional<uint32_t>(localEndTime);
        response.operatingMode  = Optional<DlOperatingMode>(operatingMode);
    }
    commandObj->AddResponse(commandPath, response);
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
    if (!emberAfPluginDoorLockSetCredential(endpointId, credentialIndex, kUndefinedFabricIndex, kUndefinedFabricIndex,
                                            DlCredentialStatus::kAvailable, credentialType, chip::ByteSpan()))
    {
        ChipLogError(Zcl,
                     "[clearCredential] Unable to clear credential - couldn't write new credential to database "
                     "[endpointId=%d,credentialType=%u,credentialIndex=%d,modifier=%d]",
                     endpointId, to_underlying(credentialType), credentialIndex, modifier);
        return EMBER_ZCL_STATUS_FAILURE;
    }

    uint8_t maxCredentialsPerUser;
    if (!GetNumberOfCredentialsSupportedPerUser(endpointId, maxCredentialsPerUser))
    {
        ChipLogError(Zcl,
                     "[clearCredential] Unable to get the number of available credentials per user: internal error "
                     "[endpointId=%d,credentialType=%d,credentialIndex=%d]",
                     endpointId, to_underlying(credentialType), credentialIndex);
        return EMBER_ZCL_STATUS_FAILURE;
    }

    // Should never happen, only possible if the implementation of application is incorrect
    if (relatedUser.credentials.size() > maxCredentialsPerUser)
    {
        ChipLogError(Zcl,
                     "[clearCredential] Unable to clear credential for related user - user has too many credentials associated"
                     "[endpointId=%d,credentialType=%u,credentialIndex=%d,modifier=%d,userIndex=%d,credentialsCount=%u]",
                     endpointId, to_underlying(credentialType), credentialIndex, modifier, relatedUserIndex,
                     static_cast<unsigned int>(relatedUser.credentials.size()));

        return EMBER_ZCL_STATUS_FAILURE;
    }

    chip::Platform::ScopedMemoryBuffer<DlCredential> newCredentials;
    if (!newCredentials.Alloc(relatedUser.credentials.size()))
    {
        ChipLogError(Zcl,
                     "[clearCredential] Unable to allocate the buffer for credentials "
                     "[endpointId=%d,credentialType=%u,credentialIndex=%d,modifier=%d,userIndex=%d,credentialsCount=%u]",
                     endpointId, to_underlying(credentialType), credentialIndex, modifier, relatedUserIndex,
                     static_cast<unsigned int>(relatedUser.credentials.size()));
        return EMBER_ZCL_STATUS_FAILURE;
    }

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
                                      relatedUser.credentialRule, newCredentials.Get(), newCredentialsCount))
    {
        ChipLogError(Zcl,
                     "[clearCredential] Unable to clear credential for related user - unable to update database "
                     "[endpointId=%d,credentialType=%u"
                     ",credentialIndex=%d,modifier=%d,userIndex=%d,newCredentialsCount=%u]",
                     endpointId, to_underlying(credentialType), credentialIndex, modifier, relatedUserIndex,
                     static_cast<unsigned int>(newCredentialsCount));
        return EMBER_ZCL_STATUS_FAILURE;
    }

    emberAfDoorLockClusterPrintln(
        "[clearCredential] Successfully clear credential and related user "
        "[endpointId=%d,credentialType=%u,credentialIndex=%d,modifier=%d,userIndex=%d,newCredentialsCount=%u]",
        endpointId, to_underlying(credentialType), credentialIndex, modifier, relatedUserIndex,
        static_cast<unsigned int>(newCredentialsCount));

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

    if (SupportsRFID(endpointId))
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

bool DoorLockServer::clearFabricFromCredentials(chip::EndpointId endpointId, DlCredentialType credentialType,
                                                chip::FabricIndex fabricToRemove)
{
    uint16_t maxNumberOfCredentials = 0;
    if (!getMaxNumberOfCredentials(endpointId, credentialType, maxNumberOfCredentials))
    {
        ChipLogError(
            Zcl,
            "[clearFabricFromCredentials] Unable to get max number of credentials to clear - can't get max number of credentials "
            "[endpointId=%d,credentialType=%u]",
            endpointId, to_underlying(credentialType));
        return false;
    }

    uint16_t startIndex = 1;
    // Programming PIN is a special case -- it is unique and its index assumed to be 0.
    if (DlCredentialType::kProgrammingPIN == credentialType)
    {
        startIndex = 0;
        maxNumberOfCredentials--;
    }

    for (uint16_t credentialIndex = startIndex; credentialIndex <= maxNumberOfCredentials; ++credentialIndex)
    {
        EmberAfPluginDoorLockCredentialInfo credential;
        if (!emberAfPluginDoorLockGetCredential(endpointId, credentialIndex, credentialType, credential))
        {
            ChipLogError(
                Zcl,
                "[clearFabricFromCredentials] Unable to clear fabric from credential - couldn't read credential from database "
                "[endpointId=%d,credentialType=%u,credentialIndex=%d,fabricIdToRemove=%d]",
                endpointId, to_underlying(credentialType), credentialIndex, fabricToRemove);

            // Go on and try to clear all the remaining credentials
            continue;
        }

        if (DlCredentialStatus::kAvailable == credential.status ||
            (credential.createdBy != fabricToRemove && credential.lastModifiedBy != fabricToRemove))
        {
            continue;
        }

        if (credential.createdBy == fabricToRemove)
        {
            credential.createdBy = kUndefinedFabricIndex;
        }

        if (credential.lastModifiedBy == fabricToRemove)
        {
            credential.lastModifiedBy = kUndefinedFabricIndex;
        }

        if (!emberAfPluginDoorLockSetCredential(endpointId, credentialIndex, credential.createdBy, credential.lastModifiedBy,
                                                credential.status, credential.credentialType, credential.credentialData))
        {
            ChipLogError(Zcl,
                         "[clearFabricFromCredentials] Unable to clear fabric from credential - internal error "
                         "[endpointId=%d,credentialType=%u,credentialIndex=%d,fabricIdToRemove=%d]",
                         endpointId, to_underlying(credentialType), credentialIndex, fabricToRemove);
            return false;
        }
    }

    return true;
}

bool DoorLockServer::clearFabricFromCredentials(chip::EndpointId endpointId, chip::FabricIndex fabricToRemove)
{
    if (SupportsRFID(endpointId))
    {
        clearFabricFromCredentials(endpointId, DlCredentialType::kRfid, fabricToRemove);
    }

    if (SupportsPIN(endpointId))
    {
        clearFabricFromCredentials(endpointId, DlCredentialType::kPin, fabricToRemove);
    }

    if (SupportsFingers(endpointId))
    {
        clearFabricFromCredentials(endpointId, DlCredentialType::kFingerprint, fabricToRemove);
        clearFabricFromCredentials(endpointId, DlCredentialType::kFingerVein, fabricToRemove);
    }

    if (SupportsFace(endpointId))
    {
        clearFabricFromCredentials(endpointId, DlCredentialType::kFace, fabricToRemove);
    }

    clearFabricFromCredentials(endpointId, DlCredentialType::kProgrammingPIN, fabricToRemove);

    return true;
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
        event.userIndex.SetNonNull(userIndex);
    }
    event.fabricIndex.SetNonNull(fabricIndex);
    event.sourceNode.SetNonNull(nodeId);
    if (0 != dataIndex)
    {
        event.dataIndex.SetNonNull(dataIndex);
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
    case DlCredentialType::kUnknownEnumValue:
        return DlLockDataType::kUnspecified;
    }

    return DlLockDataType::kUnspecified;
}

bool DoorLockServer::isUserScheduleRestricted(chip::EndpointId endpointId, const EmberAfPluginDoorLockUserInfo & user)
{
    return false;
}

void DoorLockServer::setHolidayScheduleCommandHandler(chip::app::CommandHandler * commandObj,
                                                      const chip::app::ConcreteCommandPath & commandPath, uint8_t holidayIndex,
                                                      uint32_t localStartTime, uint32_t localEndTime, DlOperatingMode operatingMode)
{
    VerifyOrDie(nullptr != commandObj);

    auto endpointId = commandPath.mEndpointId;
    if (!SupportsHolidaySchedules(endpointId))
    {
        emberAfDoorLockClusterPrintln("[SetHolidaySchedule] Ignore command (not supported) [endpointId=%d]", endpointId);
        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_INVALID_COMMAND);
        return;
    }

    emberAfDoorLockClusterPrintln("[SetHolidaySchedule] incoming command [endpointId=%d]", endpointId);

    if (!holidayIndexValid(endpointId, holidayIndex))
    {
        emberAfDoorLockClusterPrintln(
            "[SetHolidaySchedule] Unable to add schedule - index out of range [endpointId=%d,scheduleIndex=%d]", endpointId,
            holidayIndex);
        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_INVALID_COMMAND);
        return;
    }

    if (localEndTime <= localStartTime)
    {
        emberAfDoorLockClusterPrintln("[SetHolidaySchedule] Unable to add schedule - schedule ends earlier than starts"
                                      "[endpointId=%d,scheduleIndex=%d,localStarTime=%" PRIu32 ",localEndTime=%" PRIu32 "]",
                                      endpointId, holidayIndex, localStartTime, localEndTime);
        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_INVALID_COMMAND);
        return;
    }

    if (operatingMode > DlOperatingMode::kPassage || operatingMode < DlOperatingMode::kNormal)
    {
        emberAfDoorLockClusterPrintln("[SetHolidaySchedule] Unable to add schedule - operating mode is out of range"
                                      "[endpointId=%d,scheduleIndex=%d,localStarTime=%" PRIu32 ",localEndTime=%" PRIu32
                                      ", operatingMode=%d]",
                                      endpointId, holidayIndex, localStartTime, localEndTime, to_underlying(operatingMode));
        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_INVALID_COMMAND);
        return;
    }

    auto status = emberAfPluginDoorLockSetSchedule(endpointId, holidayIndex, DlScheduleStatus::kOccupied, localStartTime,
                                                   localEndTime, operatingMode);
    if (DlStatus::kSuccess != status)
    {
        ChipLogError(Zcl, "[SetHolidaySchedule] Unable to add schedule - internal error [endpointId=%d,scheduleIndex=%d,status=%u]",
                     endpointId, holidayIndex, to_underlying(status));
        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_FAILURE);
        return;
    }

    emberAfDoorLockClusterPrintln("[SetHolidaySchedule] Successfully created new schedule "
                                  "[endpointId=%d,scheduleIndex=%d,localStartTime=%" PRIu32 ",endTime=%" PRIu32
                                  ",operatingMode=%d]",
                                  endpointId, holidayIndex, localStartTime, localEndTime, to_underlying(operatingMode));

    emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_SUCCESS);
}

void DoorLockServer::getHolidayScheduleCommandHandler(chip::app::CommandHandler * commandObj,
                                                      const ConcreteCommandPath & commandPath, uint8_t holidayIndex)
{
    auto endpointId = commandPath.mEndpointId;
    if (!SupportsHolidaySchedules(endpointId))
    {
        emberAfDoorLockClusterPrintln("[GetHolidaySchedule] Ignore command (not supported) [endpointId=%d,scheduleIndex=%d]",
                                      endpointId, holidayIndex);
        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_INVALID_COMMAND);
        return;
    }
    emberAfDoorLockClusterPrintln("[GetHolidaySchedule] incoming command [endpointId=%d,scheduleIndex=%d]", endpointId,
                                  holidayIndex);

    if (!holidayIndexValid(endpointId, holidayIndex))
    {
        emberAfDoorLockClusterPrintln(
            "[GetYearDaySchedule] Unable to get schedule - index out of range [endpointId=%d,scheduleIndex=%d]", endpointId,
            holidayIndex);
        sendHolidayScheduleResponse(commandObj, commandPath, holidayIndex, DlStatus::kInvalidField);
        return;
    }

    EmberAfPluginDoorLockHolidaySchedule scheduleInfo{};
    auto status = emberAfPluginDoorLockGetSchedule(endpointId, holidayIndex, scheduleInfo);
    if (DlStatus::kSuccess != status)
    {
        sendHolidayScheduleResponse(commandObj, commandPath, holidayIndex, status);
        return;
    }
    sendHolidayScheduleResponse(commandObj, commandPath, holidayIndex, DlStatus::kSuccess, scheduleInfo.localStartTime,
                                scheduleInfo.localEndTime, scheduleInfo.operatingMode);
}

void DoorLockServer::clearHolidayScheduleCommandHandler(chip::app::CommandHandler * commandObj,
                                                        const chip::app::ConcreteCommandPath & commandPath, uint8_t holidayIndex)
{
    auto endpointId = commandPath.mEndpointId;
    if (!SupportsHolidaySchedules(endpointId))
    {
        emberAfDoorLockClusterPrintln("[ClearHolidaySchedule] Ignore command (not supported) [endpointId=%d]", endpointId);
        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_INVALID_COMMAND);
        return;
    }
    emberAfDoorLockClusterPrintln("[ClearHolidaySchedule] incoming command [endpointId=%d,scheduleIndex=%d]", endpointId,
                                  holidayIndex);

    if (!holidayIndexValid(endpointId, holidayIndex) && 0xFE != holidayIndex)
    {
        emberAfDoorLockClusterPrintln("[ClearHolidaySchedule] Holiday index is out of range [endpointId=%d,scheduleIndex=%d]",
                                      endpointId, holidayIndex);
        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_INVALID_COMMAND);
        return;
    }

    DlStatus clearStatus = DlStatus::kSuccess;
    if (0xFE == holidayIndex)
    {
        emberAfDoorLockClusterPrintln(
            "[ClearHolidaySchedule] Clearing all holiday schedules for a single user [endpointId=%d,scheduleIndex=%d]", endpointId,
            holidayIndex);
        clearStatus = clearHolidaySchedules(endpointId);
    }
    else
    {
        emberAfDoorLockClusterPrintln("[ClearHolidaySchedule] Clearing a single schedule [endpointId=%d,scheduleIndex=%d]",
                                      endpointId, holidayIndex);
        clearStatus = clearHolidaySchedule(endpointId, holidayIndex);
    }

    if (DlStatus::kSuccess != clearStatus)
    {
        emberAfDoorLockClusterPrintln(
            "[ClearHolidaySchedule] Unable to clear the user schedules - app error [endpointId=%d,scheduleIndex=%d,status=%u]",
            endpointId, holidayIndex, to_underlying(clearStatus));
        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_FAILURE);
        return;
    }
    emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_SUCCESS);
}

bool DoorLockServer::RemoteOperationEnabled(chip::EndpointId endpointId) const
{
    DlOperatingMode mode;

    return GetAttribute(endpointId, Attributes::OperatingMode::Id, Attributes::OperatingMode::Get, mode) &&
        mode != DlOperatingMode::kPrivacy && mode != DlOperatingMode::kNoRemoteLockUnlock;
}

CHIP_ERROR DoorLockServer::sendClusterResponse(chip::app::CommandHandler * commandObj,
                                               const chip::app::ConcreteCommandPath & commandPath, EmberAfStatus status)
{
    VerifyOrDie(nullptr != commandObj);

    auto err             = CHIP_NO_ERROR;
    auto statusAsInteger = to_underlying(status);
    if (statusAsInteger == to_underlying(DlStatus::kOccupied) || statusAsInteger == to_underlying(DlStatus::kDuplicate))
    {
        err = commandObj->AddClusterSpecificFailure(commandPath, static_cast<chip::ClusterStatus>(status));
    }
    else
    {
        err = commandObj->AddStatus(commandPath, ToInteractionModelStatus(status));
    }

    return err;
}

EmberAfDoorLockEndpointContext * DoorLockServer::getContext(chip::EndpointId endpointId)
{
    auto index = emberAfFindClusterServerEndpointIndex(endpointId, ::Id);
    if (index != 0xFFFF)
    {
        return &mEndpointCtx[index];
    }
    return nullptr;
}

bool DoorLockServer::HandleRemoteLockOperation(chip::app::CommandHandler * commandObj,
                                               const chip::app::ConcreteCommandPath & commandPath, DlLockOperationType opType,
                                               RemoteLockOpHandler opHandler, const Optional<ByteSpan> & pinCode)
{
    VerifyOrDie(DlLockOperationType::kLock == opType || DlLockOperationType::kUnlock == opType);
    VerifyOrDie(nullptr != opHandler);

    EndpointId endpoint     = commandPath.mEndpointId;
    DlOperationError reason = DlOperationError::kUnspecified;
    uint16_t pinUserIdx     = 0;
    uint16_t pinCredIdx     = 0;
    bool success            = false;
    bool sendEvent          = true;

    auto currentTime = chip::System::SystemClock().GetMonotonicTimestamp();

    EmberAfDoorLockEndpointContext * endpointContext;

    VerifyOrExit(RemoteOperationEnabled(endpoint), reason = DlOperationError::kUnspecified);

    // appclusters.pdf 5.3.4.1:
    // When the PINCode field is provided an invalid PIN will count towards the WrongCodeEntryLimit and the
    // UserCodeTemporaryDisableTime will be triggered if the WrongCodeEntryLimit is exceeded. The lock SHALL ignore any attempts
    // to lock/unlock the door until the UserCodeTemporaryDisableTime expires.
    endpointContext = getContext(endpoint);
    VerifyOrExit(nullptr != endpointContext, ChipLogError(Zcl, "Failed to get endpoint index for cluster [endpoint=%d]", endpoint));
    if (endpointContext->lockoutEndTimestamp >= currentTime)
    {
        emberAfDoorLockClusterPrintln("Rejecting unlock command -- lockout is in action [endpoint=%d,lockoutEnd=%u,currentTime=%u]",
                                      endpoint, static_cast<unsigned>(endpointContext->lockoutEndTimestamp.count()),
                                      static_cast<unsigned>(currentTime.count()));
        sendEvent = false;
        goto exit;
    }

    if (pinCode.HasValue())
    {
        // appclusters.pdf 5.3.4.1:
        // If the PINCode field is provided, the door lock SHALL verify PINCode before granting access regardless of the value
        // of RequirePINForRemoteOperation attribute.
        VerifyOrExit(SupportsPIN(endpoint) && SupportsUSR(endpoint),
                     emberAfDoorLockClusterPrintln(
                         "PIN code is supplied while USR/PIN features are disabled. Exiting [endpoint=%d, lock_op=%d]", endpoint,
                         chip::to_underlying(opType)));

        // Look up the user index and credential index -- it should be used in the Lock Operation event
        EmberAfPluginDoorLockUserInfo user;
        findUserIndexByCredential(endpoint, DlCredentialType::kPin, pinCode.Value(), pinUserIdx, pinCredIdx, user);

        // If the user status is OccupiedDisabled we should deny the access and send out the appropriate event
        VerifyOrExit(user.userStatus != DlUserStatus::kOccupiedDisabled, {
            reason = DlOperationError::kDisabledUserDenied;
            emberAfDoorLockClusterPrintln(
                "Unable to perform remote lock operation: user is disabled [endpoint=%d, lock_op=%d, userIndex=%d]", endpoint,
                to_underlying(opType), pinUserIdx);
        });
    }
    else
    {
        bool requirePin = false;

        // appclusters.pdf 5.3.4.1:
        // If the RequirePINForRemoteOperation attribute is True then PINCode field SHALL be provided and the door lock SHALL
        // NOT grant access if it is not provided. This attribute exists when COTA and PIN features are both enabled. Otherwise
        // we assume PIN to be OK.
        if (SupportsCredentialsOTA(endpoint) && SupportsPIN(endpoint))
        {
            auto status = Attributes::RequirePINforRemoteOperation::Get(endpoint, &requirePin);
            VerifyOrExit(
                EMBER_ZCL_STATUS_UNSUPPORTED_ATTRIBUTE == status || EMBER_ZCL_STATUS_SUCCESS == status,
                ChipLogError(Zcl, "Failed to read Require PIN For Remote Operation attribute, status=0x%x", to_underlying(status)));
        }
        // If the PIN is required but not provided we should exit
        VerifyOrExit(!requirePin, {
            reason = DlOperationError::kInvalidCredential;
            emberAfDoorLockClusterPrintln("Checking credentials failed: PIN is not provided when it is required");
        });
    }

    // credentials check succeeded, try to lock/unlock door
    success = opHandler(endpoint, pinCode, reason);
    if (!success && reason == DlOperationError::kInvalidCredential)
    {
        TrackWrongCodeEntry(endpoint);
    }
    // The app should trigger the lock state change as it may take a while before the lock actually locks/unlocks
exit:
    // Send command response
    emberAfSendImmediateDefaultResponse(success ? EMBER_ZCL_STATUS_SUCCESS : EMBER_ZCL_STATUS_FAILURE);

    // Most of the time we want to send the lock operation event but sometimes (when the lockout is active) we don't want it.
    if (!sendEvent)
    {
        return success;
    }

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
                                            DlOperationError opErr, const Nullable<uint16_t> & userId,
                                            const Nullable<chip::FabricIndex> & fabricIdx, const Nullable<chip::NodeId> & nodeId,
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
    AutolockEvent.callback = DoorLockOnAutoRelockCallback;

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
                                  EmberAfStatus (*getFn)(chip::EndpointId endpointId, T * value), T & value) const
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
        auto timeout = static_cast<uint32_t>(commandData.timeout);

        VerifyOrReturnError(0 != timeout, true);
        DoorLockServer::Instance().ScheduleAutoRelock(commandPath.mEndpointId, timeout);
    }

    return true;
}

bool emberAfDoorLockClusterSetUserCallback(chip::app::CommandHandler * commandObj,
                                           const chip::app::ConcreteCommandPath & commandPath,
                                           const chip::app::Clusters::DoorLock::Commands::SetUser::DecodableType & commandData)
{
    DoorLockServer::Instance().setUserCommandHandler(commandObj, commandPath, commandData);
    return true;
}

bool emberAfDoorLockClusterGetUserCallback(chip::app::CommandHandler * commandObj,
                                           const chip::app::ConcreteCommandPath & commandPath,
                                           const chip::app::Clusters::DoorLock::Commands::GetUser::DecodableType & commandData)
{
    DoorLockServer::Instance().getUserCommandHandler(commandObj, commandPath, commandData.userIndex);
    return true;
}

bool emberAfDoorLockClusterClearUserCallback(chip::app::CommandHandler * commandObj,
                                             const chip::app::ConcreteCommandPath & commandPath,
                                             const chip::app::Clusters::DoorLock::Commands::ClearUser::DecodableType & commandData)
{
    DoorLockServer::Instance().clearUserCommandHandler(commandObj, commandPath, commandData.userIndex);
    return true;
}

bool emberAfDoorLockClusterSetCredentialCallback(
    chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
    const chip::app::Clusters::DoorLock::Commands::SetCredential::DecodableType & commandData)
{
    DoorLockServer::Instance().setCredentialCommandHandler(commandObj, commandPath, commandData);
    return true;
}

bool emberAfDoorLockClusterGetCredentialStatusCallback(
    chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
    const chip::app::Clusters::DoorLock::Commands::GetCredentialStatus::DecodableType & commandData)
{
    DoorLockServer::Instance().getCredentialStatusCommandHandler(commandObj, commandPath, commandData.credential.credentialType,
                                                                 commandData.credential.credentialIndex);
    return true;
}

bool emberAfDoorLockClusterClearCredentialCallback(
    chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
    const chip::app::Clusters::DoorLock::Commands::ClearCredential::DecodableType & commandData)
{
    DoorLockServer::Instance().clearCredentialCommandHandler(commandObj, commandPath, commandData);
    return true;
}

bool emberAfDoorLockClusterSetWeekDayScheduleCallback(
    chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
    const chip::app::Clusters::DoorLock::Commands::SetWeekDaySchedule::DecodableType & commandData)
{
    DoorLockServer::Instance().setWeekDayScheduleCommandHandler(
        commandObj, commandPath, commandData.weekDayIndex, commandData.userIndex, commandData.daysMask, commandData.startHour,
        commandData.startMinute, commandData.endHour, commandData.endMinute);
    return true;
}

bool emberAfDoorLockClusterGetWeekDayScheduleCallback(
    chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
    const chip::app::Clusters::DoorLock::Commands::GetWeekDaySchedule::DecodableType & commandData)
{
    DoorLockServer::Instance().getWeekDayScheduleCommandHandler(commandObj, commandPath, commandData.weekDayIndex,
                                                                commandData.userIndex);
    return true;
}

bool emberAfDoorLockClusterClearWeekDayScheduleCallback(
    chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
    const chip::app::Clusters::DoorLock::Commands::ClearWeekDaySchedule::DecodableType & commandData)
{
    DoorLockServer::Instance().clearWeekDayScheduleCommandHandler(commandObj, commandPath, commandData.weekDayIndex,
                                                                  commandData.userIndex);
    return true;
}

bool emberAfDoorLockClusterSetYearDayScheduleCallback(
    chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
    const chip::app::Clusters::DoorLock::Commands::SetYearDaySchedule::DecodableType & commandData)
{
    DoorLockServer::Instance().setYearDayScheduleCommandHandler(commandObj, commandPath, commandData.yearDayIndex,
                                                                commandData.userIndex, commandData.localStartTime,
                                                                commandData.localEndTime);
    return true;
}

bool emberAfDoorLockClusterGetYearDayScheduleCallback(
    chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
    const chip::app::Clusters::DoorLock::Commands::GetYearDaySchedule::DecodableType & commandData)
{
    DoorLockServer::Instance().getYearDayScheduleCommandHandler(commandObj, commandPath, commandData.yearDayIndex,
                                                                commandData.userIndex);
    return true;
}

bool emberAfDoorLockClusterClearYearDayScheduleCallback(
    chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
    const chip::app::Clusters::DoorLock::Commands::ClearYearDaySchedule::DecodableType & commandData)
{
    DoorLockServer::Instance().clearYearDayScheduleCommandHandler(commandObj, commandPath, commandData.yearDayIndex,
                                                                  commandData.userIndex);
    return true;
}

bool emberAfDoorLockClusterSetHolidayScheduleCallback(
    chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
    const chip::app::Clusters::DoorLock::Commands::SetHolidaySchedule::DecodableType & commandData)
{
    DoorLockServer::Instance().setHolidayScheduleCommandHandler(commandObj, commandPath, commandData.holidayIndex,
                                                                commandData.localStartTime, commandData.localEndTime,
                                                                commandData.operatingMode);
    return true;
}

bool emberAfDoorLockClusterGetHolidayScheduleCallback(
    chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
    const chip::app::Clusters::DoorLock::Commands::GetHolidaySchedule::DecodableType & commandData)
{
    DoorLockServer::Instance().getHolidayScheduleCommandHandler(commandObj, commandPath, commandData.holidayIndex);
    return true;
}

bool emberAfDoorLockClusterClearHolidayScheduleCallback(
    chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
    const chip::app::Clusters::DoorLock::Commands::ClearHolidaySchedule::DecodableType & commandData)
{
    DoorLockServer::Instance().clearHolidayScheduleCommandHandler(commandObj, commandPath, commandData.holidayIndex);
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
        res = emberAfPluginDoorLockOnUnhandledAttributeChange(attributePath.mEndpointId, attributePath, attributeType, size, value);
        break;
    }

    return res;
}

void emberAfPluginDoorLockServerLockoutEventHandler() {}

void emberAfPluginDoorLockServerRelockEventHandler() {}

void MatterDoorLockPluginServerInitCallback()
{
    emberAfDoorLockClusterPrintln("Door Lock server initialized");
    Server::GetInstance().GetFabricTable().AddFabricDelegate(&gFabricDelegate);
}

void MatterDoorLockClusterServerAttributeChangedCallback(const app::ConcreteAttributePath & attributePath) {}

// =============================================================================
// Timer callbacks
// =============================================================================

void DoorLockServer::DoorLockOnAutoRelockCallback(chip::EndpointId endpointId)
{
    emberAfDoorLockClusterPrintln("Door Auto relock timer expired. Locking...");
    emberEventControlSetInactive(&DoorLockServer::Instance().AutolockEvent);
    DoorLockServer::Instance().SetLockState(endpointId, DlLockState::kLocked, DlOperationSource::kAuto);
    emberAfPluginDoorLockOnAutoRelock(endpointId);
}
