/**
 *
 *    Copyright (c) 2020 - 2024 Project CHIP Authors
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
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/EventLogging.h>
#include <app/server/Server.h>
#include <app/util/af.h>
#include <app/util/attribute-storage.h>
#include <cinttypes>

#include <app/CommandHandler.h>
#include <app/ConcreteAttributePath.h>
#include <app/ConcreteCommandPath.h>
#include <lib/support/CodeUtils.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::DataModel;
using namespace chip::app::Clusters::DoorLock;
using namespace chip::app::Clusters::DoorLock::Attributes;
using chip::Protocols::InteractionModel::ClusterStatusCode;
using chip::Protocols::InteractionModel::Status;

static constexpr uint8_t DOOR_LOCK_SCHEDULE_MAX_HOUR     = 23;
static constexpr uint8_t DOOR_LOCK_SCHEDULE_MAX_MINUTE   = 59;
static constexpr uint8_t DOOR_LOCK_ALIRO_CREDENTIAL_SIZE = 65;

static constexpr uint32_t DOOR_LOCK_MAX_LOCK_TIMEOUT_SEC = MAX_INT32U_VALUE / MILLISECOND_TICKS_PER_SECOND;

static constexpr size_t kDoorLockDelegateTableSize =
    MATTER_DM_DOOR_LOCK_CLUSTER_SERVER_ENDPOINT_COUNT + CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT;

static_assert(kDoorLockDelegateTableSize <= kEmberInvalidEndpointIndex, "Door Lock Delegate table size error");

namespace chip {
namespace app {
namespace Clusters {
namespace DoorLock {

Delegate * gDelegateTable[kDoorLockDelegateTableSize] = { nullptr };

Delegate * GetDelegate(EndpointId endpoint)
{
    uint16_t ep = emberAfGetClusterServerEndpointIndex(endpoint, DoorLock::Id, MATTER_DM_DOOR_LOCK_CLUSTER_SERVER_ENDPOINT_COUNT);
    return (ep >= kDoorLockDelegateTableSize ? nullptr : gDelegateTable[ep]);
}

void SetDefaultDelegate(EndpointId endpoint, Delegate * delegate)
{
    uint16_t ep = emberAfGetClusterServerEndpointIndex(endpoint, DoorLock::Id, MATTER_DM_DOOR_LOCK_CLUSTER_SERVER_ENDPOINT_COUNT);
    // if endpoint is found
    if (ep < ArraySize(gDelegateTable))
    {
        gDelegateTable[ep] = delegate;
    }
}

} // namespace DoorLock
} // namespace Clusters
} // namespace app
} // namespace chip

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
    ChipLogProgress(Zcl, "Door Lock cluster initialized at endpoint #%u", endpointId);

    auto status = Attributes::LockState::SetNull(endpointId);
    if (Status::Success != status)
    {
        ChipLogError(Zcl, "[InitDoorLockServer] Unable to set the Lock State attribute to null [status=%d]", to_underlying(status));
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

bool DoorLockServer::SetLockState(chip::EndpointId endpointId, DlLockState newLockState, OperationSourceEnum opSource,
                                  const Nullable<uint16_t> & userIndex, const Nullable<List<const LockOpCredentials>> & credentials,
                                  const Nullable<chip::FabricIndex> & fabricIdx, const Nullable<chip::NodeId> & nodeId)
{
    bool success = SetLockState(endpointId, newLockState);

    // DlLockState::kNotFullyLocked has no appropriate event to send. Also it is unclear whether
    // it should schedule auto-relocking. So skip it here. Check for supported states explicitly
    // to handle possible enum extending in future.
    VerifyOrReturnError(DlLockState::kLocked == newLockState || DlLockState::kUnlocked == newLockState ||
                            DlLockState::kUnlatched == newLockState,
                        success);

    // Send LockOperation event
    auto opType = LockOperationTypeEnum::kUnlock;

    if (DlLockState::kLocked == newLockState)
    {
        opType = LockOperationTypeEnum::kLock;
    }
    else if (DlLockState::kUnlatched == newLockState)
    {
        opType = LockOperationTypeEnum::kUnlatch;
    }

    if (OperationSourceEnum::kRemote == opSource && (fabricIdx.IsNull() || nodeId.IsNull()))
    {
        ChipLogError(Zcl, "Received SetLockState for remote operation without fabricIdx or nodeId");
    }

    SendLockOperationEvent(endpointId, opType, opSource, OperationErrorEnum::kUnspecified, userIndex, fabricIdx, nodeId,
                           credentials, success);

    // Reset wrong entry attempts (in case there were any incorrect credentials presented before) if lock/unlock was a success
    // and a valid credential was presented.
    if (success && !credentials.IsNull() && !(credentials.Value().empty()))
    {
        ResetWrongCodeEntryAttempts(endpointId);
    }

    // Schedule auto-relocking
    if (success && LockOperationTypeEnum::kUnlock == opType)
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

bool DoorLockServer::SetDoorState(chip::EndpointId endpointId, DoorStateEnum newDoorState)
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

void DoorLockServer::HandleLocalLockOperationError(chip::EndpointId endpointId, LockOperationTypeEnum opType,
                                                   OperationSourceEnum opSource, Nullable<uint16_t> userId)
{
    SendLockOperationEvent(endpointId, opType, opSource, OperationErrorEnum::kInvalidCredential, userId,
                           Nullable<chip::FabricIndex>(), Nullable<chip::NodeId>(), Nullable<List<const LockOpCredentials>>(),
                           false);

    HandleWrongCodeEntry(endpointId);

    ChipLogProgress(Zcl, "Handling a local Lock Operation Error: [endpoint=%d]", endpointId);
}

bool DoorLockServer::HandleWrongCodeEntry(chip::EndpointId endpointId)
{
    auto endpointContext = getContext(endpointId);
    if (nullptr == endpointContext)
    {
        ChipLogError(Zcl, "Failed to get endpoint index for cluster [endpoint=%d]", endpointId);
        return false;
    }

    uint8_t wrongCodeEntryLimit = 0xFF;
    auto status                 = Attributes::WrongCodeEntryLimit::Get(endpointId, &wrongCodeEntryLimit);
    if (Status::Success == status)
    {
        if (++endpointContext->wrongCodeEntryAttempts >= wrongCodeEntryLimit)
        {
            ChipLogProgress(Zcl, "Too many wrong code entry attempts, engaging lockout [endpoint=%d,wrongCodeAttempts=%d]",
                            endpointId, endpointContext->wrongCodeEntryAttempts);
            engageLockout(endpointId);
        }
    }
    else if (Status::UnsupportedAttribute != status)
    {
        ChipLogError(Zcl, "Failed to read Wrong Code Entry Limit attribute, status=0x%x", to_underlying(status));
        return false;
    }
    return true;
}

void DoorLockServer::ResetWrongCodeEntryAttempts(chip::EndpointId endpointId)
{
    auto endpointContext = getContext(endpointId);
    if (nullptr == endpointContext)
    {
        ChipLogError(Zcl, "Failed to reset wrong code entry attempts. No context for endpoint %d", endpointId);
        return;
    }
    endpointContext->wrongCodeEntryAttempts = 0;
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
    if (Status::UnsupportedAttribute == status)
    {
        return false;
    }
    if (Status::Success != status)
    {
        ChipLogError(Zcl, "Unable to read the UserCodeTemporaryDisableTime attribute [status=%d]", to_underlying(status));
        return false;
    }

    endpointContext->wrongCodeEntryAttempts = 0;
    endpointContext->lockoutEndTimestamp =
        chip::System::SystemClock().GetMonotonicTimestamp() + chip::System::Clock::Seconds32(lockoutTimeout);

    ChipLogProgress(Zcl, "Lockout engaged [endpointId=%d,lockoutTimeout=%d]", endpointId, lockoutTimeout);

    SendLockAlarmEvent(endpointId, AlarmCodeEnum::kWrongCodeEntryLimit);

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

bool DoorLockServer::SendLockAlarmEvent(chip::EndpointId endpointId, AlarmCodeEnum alarmCode)
{
    Events::DoorLockAlarm::Type event{ alarmCode };
    SendEvent(endpointId, event);

    return true;
}

namespace {
// Check whether this is valid UserStatus for a SetUser or SetCredential
// command.
bool IsValidUserStatusForSet(const Nullable<UserStatusEnum> & userStatus)
{
    return userStatus.IsNull() || (userStatus.Value() == UserStatusEnum::kOccupiedEnabled) ||
        (userStatus.Value() == UserStatusEnum::kOccupiedDisabled);
}
} // anonymous namespace

void DoorLockServer::setUserCommandHandler(chip::app::CommandHandler * commandObj,
                                           const chip::app::ConcreteCommandPath & commandPath,
                                           const chip::app::Clusters::DoorLock::Commands::SetUser::DecodableType & commandData)
{
    auto & userIndex = commandData.userIndex;
    ChipLogProgress(Zcl, "[SetUser] Incoming command [endpointId=%d,userIndex=%d]", commandPath.mEndpointId, userIndex);

    if (!SupportsUSR(commandPath.mEndpointId))
    {
        ChipLogProgress(Zcl, "[SetUser] User management is not supported [endpointId=%d]", commandPath.mEndpointId);
        sendClusterResponse(commandObj, commandPath, ClusterStatusCode(Status::UnsupportedCommand));
        return;
    }

    auto fabricIdx = getFabricIndex(commandObj);
    if (chip::kUndefinedFabricIndex == fabricIdx)
    {
        ChipLogError(Zcl, "[SetUser] Unable to get the fabric IDX [endpointId=%d,userIndex=%d]", commandPath.mEndpointId,
                     userIndex);
        sendClusterResponse(commandObj, commandPath, ClusterStatusCode(Status::Failure));
        return;
    }

    auto sourceNodeId = getNodeId(commandObj);
    if (chip::kUndefinedNodeId == sourceNodeId)
    {
        ChipLogError(Zcl, "[SetUser] Unable to get the source node index [endpointId=%d,userIndex=%d]", commandPath.mEndpointId,
                     userIndex);
        sendClusterResponse(commandObj, commandPath, ClusterStatusCode(Status::Failure));
        return;
    }

    auto & operationType  = commandData.operationType;
    auto & userName       = commandData.userName;
    auto & userUniqueId   = commandData.userUniqueID;
    auto & userStatus     = commandData.userStatus;
    auto & userType       = commandData.userType;
    auto & credentialRule = commandData.credentialRule;

    if (!userIndexValid(commandPath.mEndpointId, userIndex))
    {
        ChipLogProgress(Zcl, "[SetUser] User index out of bounds [endpointId=%d,userIndex=%d]", commandPath.mEndpointId, userIndex);
        sendClusterResponse(commandObj, commandPath, ClusterStatusCode(Status::InvalidCommand));
        return;
    }

    // appclusters, 5.2.4.34: UserName has maximum DOOR_LOCK_MAX_USER_NAME_SIZE (10) characters excluding NUL terminator in it.
    if (!userName.IsNull() && userName.Value().size() > DOOR_LOCK_MAX_USER_NAME_SIZE)
    {
        ChipLogProgress(Zcl, "[SetUser] Unable to set user: userName too long [endpointId=%d,userIndex=%d,userNameSize=%u]",
                        commandPath.mEndpointId, userIndex, static_cast<unsigned int>(userName.Value().size()));

        sendClusterResponse(commandObj, commandPath, ClusterStatusCode(Status::InvalidCommand));
        return;
    }

    if (!IsValidUserStatusForSet(userStatus))
    {
        ChipLogProgress(Zcl,
                        "[SetUser] Unable to set the user: user status is out of range [endpointId=%d,userIndex=%d,userStatus=%u]",
                        commandPath.mEndpointId, userIndex, to_underlying(userStatus.Value()));

        sendClusterResponse(commandObj, commandPath, ClusterStatusCode(Status::InvalidCommand));
        return;
    }

    if (userType == UserTypeEnum::kUnknownEnumValue)
    {
        ChipLogProgress(Zcl, "[SetUser] Unable to set the user: user type is unknown [endpointId=%d,userIndex=%d,userType=%u]",
                        commandPath.mEndpointId, userIndex, to_underlying(userType.Value()));

        sendClusterResponse(commandObj, commandPath, ClusterStatusCode(Status::InvalidCommand));
        return;
    }

    ClusterStatusCode status(Status::Success);
    switch (operationType)
    {
    case DataOperationTypeEnum::kAdd:
        status = createUser(commandPath.mEndpointId, fabricIdx, sourceNodeId, userIndex, userName, userUniqueId, userStatus,
                            userType, credentialRule);
        break;
    case DataOperationTypeEnum::kModify:
        status = ClusterStatusCode(modifyUser(commandPath.mEndpointId, fabricIdx, sourceNodeId, userIndex, userName, userUniqueId,
                                              userStatus, userType, credentialRule));
        break;
    case DataOperationTypeEnum::kClear:
    default:
        // appclusters, 5.2.4.34: SetUser command allow only kAdd/kModify, we should respond with INVALID_COMMAND if we got kClear
        // or anything else
        status = ClusterStatusCode(Status::InvalidCommand);
        ChipLogProgress(Zcl, "[SetUser] Invalid operation type [endpointId=%d,operationType=%u]", commandPath.mEndpointId,
                        to_underlying(operationType));
        break;
    }

    sendClusterResponse(commandObj, commandPath, status);
}

void DoorLockServer::getUserCommandHandler(chip::app::CommandHandler * commandObj,
                                           const chip::app::ConcreteCommandPath & commandPath, uint16_t userIndex)
{
    ChipLogProgress(Zcl, "[GetUser] Incoming command [endpointId=%d,userIndex=%d]", commandPath.mEndpointId, userIndex);

    if (!SupportsUSR(commandPath.mEndpointId))
    {
        ChipLogProgress(Zcl, "[GetUser] User management is not supported [endpointId=%d]", commandPath.mEndpointId);
        commandObj->AddStatus(commandPath, Status::UnsupportedCommand);
        return;
    }

    uint16_t maxNumberOfUsers = 0;
    if (!userIndexValid(commandPath.mEndpointId, userIndex, maxNumberOfUsers))
    {
        ChipLogProgress(Zcl, "[GetUser] User index out of bounds [userIndex=%d,numberOfTotalUsersSupported=%d]", userIndex,
                        maxNumberOfUsers);
        commandObj->AddStatus(commandPath, Status::InvalidCommand);
        return;
    }

    Commands::GetUserResponse::Type response;

    // appclusters, 5.2.4.36.1: We need to add next occupied user after userIndex if any.
    //
    // We want to do this before we call emberAfPluginDoorLockGetUser, because this will
    // make its own emberAfPluginDoorLockGetUser calls, and a
    // EmberAfPluginDoorLockUserInfo might be pointing into some application-static
    // buffers (for its credentials and whatnot).
    uint16_t nextAvailableUserIndex = 0;
    if (findOccupiedUserSlot(commandPath.mEndpointId, static_cast<uint16_t>(userIndex + 1), nextAvailableUserIndex))
    {
        response.nextUserIndex.SetNonNull(nextAvailableUserIndex);
    }

    EmberAfPluginDoorLockUserInfo user;
    if (!emberAfPluginDoorLockGetUser(commandPath.mEndpointId, userIndex, user))
    {
        ChipLogProgress(Zcl, "[GetUser] Could not get user info [userIndex=%d]", userIndex);
        commandObj->AddStatus(commandPath, Status::Failure);
        return;
    }

    response.userIndex = userIndex;

    // appclusters, 5.2.4.36: we should not set user-specific fields to non-null if the user status is set to Available
    if (UserStatusEnum::kAvailable != user.userStatus)
    {
        ChipLogProgress(Zcl,
                        "Found user in storage: "
                        "[userIndex=%d,userName=\"%.*s\",userStatus=%u,userType=%u"
                        ",credentialRule=%u,createdBy=%u,modifiedBy=%u]",
                        userIndex, static_cast<int>(user.userName.size()), user.userName.data(), to_underlying(user.userStatus),
                        to_underlying(user.userType), to_underlying(user.credentialRule), user.createdBy, user.lastModifiedBy);

        response.userName.SetNonNull(user.userName);
        if (0xFFFFFFFFU != user.userUniqueId)
        {
            response.userUniqueID.SetNonNull(user.userUniqueId);
        }
        response.userStatus.SetNonNull(user.userStatus);
        response.userType.SetNonNull(user.userType);
        response.credentialRule.SetNonNull(user.credentialRule);
        response.credentials.SetNonNull(user.credentials);
        // Set fabric indices only if the user was created/modified by matter.
        if (user.creationSource == DlAssetSource::kMatterIM)
        {
            response.creatorFabricIndex.SetNonNull(user.createdBy);
        }
        if (user.modificationSource == DlAssetSource::kMatterIM)
        {
            response.lastModifiedFabricIndex.SetNonNull(user.lastModifiedBy);
        }
    }
    else
    {
        ChipLogProgress(Zcl, "[GetUser] User not found [userIndex=%d]", userIndex);
    }

    commandObj->AddResponse(commandPath, response);
}

void DoorLockServer::clearUserCommandHandler(chip::app::CommandHandler * commandObj,
                                             const chip::app::ConcreteCommandPath & commandPath, uint16_t userIndex)
{
    ChipLogProgress(Zcl, "[ClearUser] Incoming command [endpointId=%d,userIndex=%d]", commandPath.mEndpointId, userIndex);

    if (!SupportsUSR(commandPath.mEndpointId))
    {
        ChipLogProgress(Zcl, "[ClearUser] User management is not supported [endpointId=%d]", commandPath.mEndpointId);
        commandObj->AddStatus(commandPath, Status::UnsupportedCommand);
        return;
    }

    auto fabricIdx = getFabricIndex(commandObj);
    if (chip::kUndefinedFabricIndex == fabricIdx)
    {
        ChipLogError(Zcl, "[ClearUser] Unable to get the fabric IDX [endpointId=%d,userIndex=%d]", commandPath.mEndpointId,
                     userIndex);
        commandObj->AddStatus(commandPath, Status::Failure);
        return;
    }

    auto sourceNodeId = getNodeId(commandObj);
    if (chip::kUndefinedNodeId == sourceNodeId)
    {
        ChipLogError(Zcl, "[ClearUser] Unable to get the source node index [endpointId=%d,userIndex=%d]", commandPath.mEndpointId,
                     userIndex);
        commandObj->AddStatus(commandPath, Status::Failure);
        return;
    }

    uint16_t maxNumberOfUsers = 0;
    if (!userIndexValid(commandPath.mEndpointId, userIndex, maxNumberOfUsers) && userIndex != 0xFFFE)
    {
        ChipLogProgress(Zcl, "[ClearUser] User index out of bounds [userIndex=%d,numberOfTotalUsersSupported=%d]", userIndex,
                        maxNumberOfUsers);
        commandObj->AddStatus(commandPath, Status::InvalidCommand);
        return;
    }

    if (0xFFFE != userIndex)
    {
        auto status = clearUser(commandPath.mEndpointId, fabricIdx, sourceNodeId, userIndex, true);
        if (Status::Success != status)
        {
            ChipLogError(Zcl, "[ClearUser] App reported failure when resetting the user [userIndex=%d,status=0x%x]", userIndex,
                         to_underlying(status));
        }
        commandObj->AddStatus(commandPath, status);
        return;
    }

    ChipLogProgress(Zcl, "[ClearUser] Removing all users from storage");
    for (uint16_t i = 1; i <= maxNumberOfUsers; ++i)
    {
        auto status = clearUser(commandPath.mEndpointId, fabricIdx, sourceNodeId, i, false);
        if (Status::Success != status)
        {
            ChipLogError(Zcl, "[ClearUser] App reported failure when resetting the user [userIndex=%d,status=0x%x]", i,
                         to_underlying(status));

            commandObj->AddStatus(commandPath, Status::Failure);
            return;
        }
    }
    ChipLogProgress(Zcl, "[ClearUser] Removed all users from storage [users=%d]", maxNumberOfUsers);

    sendRemoteLockUserChange(commandPath.mEndpointId, LockDataTypeEnum::kUserIndex, DataOperationTypeEnum::kClear, sourceNodeId,
                             fabricIdx, 0xFFFE, 0xFFFE);

    commandObj->AddStatus(commandPath, Status::Success);
}

void DoorLockServer::setCredentialCommandHandler(
    chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
    const chip::app::Clusters::DoorLock::Commands::SetCredential::DecodableType & commandData)
{
    ChipLogProgress(Zcl, "[SetCredential] Incoming command [endpointId=%d]", commandPath.mEndpointId);

    auto fabricIdx = getFabricIndex(commandObj);
    if (kUndefinedFabricIndex == fabricIdx)
    {
        ChipLogError(Zcl, "[SetCredential] Unable to get the fabric IDX [endpointId=%d]", commandPath.mEndpointId);
        commandObj->AddStatus(commandPath, Status::Failure);
        return;
    }

    auto sourceNodeId = getNodeId(commandObj);
    if (chip::kUndefinedNodeId == sourceNodeId)
    {
        ChipLogError(Zcl, "[SetCredential] Unable to get the source node index [endpointId=%d]", commandPath.mEndpointId);
        commandObj->AddStatus(commandPath, Status::Failure);
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
        ChipLogProgress(Zcl, "[SetCredential] Credential type is not supported [endpointId=%d,credentialType=%u]",
                        commandPath.mEndpointId, to_underlying(credentialType));
        commandObj->AddStatus(commandPath, Status::UnsupportedCommand);
        return;
    }

    // appclusters, 5.2.4.41: response should contain next available credential slot
    uint16_t nextAvailableCredentialSlot = 0;
    findUnoccupiedCredentialSlot(commandPath.mEndpointId, credentialType, static_cast<uint16_t>(credentialIndex + 1),
                                 nextAvailableCredentialSlot);

    uint16_t maxNumberOfCredentials = 0;
    if (!credentialIndexValid(commandPath.mEndpointId, credentialType, credentialIndex, maxNumberOfCredentials))
    {
        ChipLogProgress(Zcl,
                        "[SetCredential] Credential index is out of range [endpointId=%d,credentialType=%u"
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
    for (uint16_t i = 1; CredentialTypeEnum::kProgrammingPIN != credentialType && (i <= maxNumberOfCredentials); ++i)
    {
        EmberAfPluginDoorLockCredentialInfo currentCredential;
        if (!emberAfPluginDoorLockGetCredential(commandPath.mEndpointId, i, credentialType, currentCredential))
        {
            ChipLogProgress(Zcl,
                            "[SetCredential] Unable to get the credential to exclude duplicated entry "
                            "[endpointId=%d,credentialType=%u,credentialIndex=%d]",
                            commandPath.mEndpointId, to_underlying(credentialType), i);
            sendSetCredentialResponse(commandObj, commandPath, DlStatus::kFailure, 0, nextAvailableCredentialSlot);
            return;
        }
        if (DlCredentialStatus::kAvailable != currentCredential.status && currentCredential.credentialType == credentialType &&
            currentCredential.credentialData.data_equal(credentialData))
        {
            ChipLogProgress(Zcl,
                            "[SetCredential] Credential with the same data and type already exist "
                            "[endpointId=%d,credentialType=%u,dataLength=%u,existingCredentialIndex=%d,credentialIndex=%d]",
                            commandPath.mEndpointId, to_underlying(credentialType),
                            static_cast<unsigned int>(credentialData.size()), i, credentialIndex);
            sendSetCredentialResponse(commandObj, commandPath, DlStatus::kDuplicate, 0, nextAvailableCredentialSlot);
            return;
        }
    }

    EmberAfPluginDoorLockCredentialInfo existingCredential;
    if (!emberAfPluginDoorLockGetCredential(commandPath.mEndpointId, credentialIndex, credentialType, existingCredential))
    {
        ChipLogProgress(Zcl, "[SetCredential] Unable to check if credential exists: app error [endpointId=%d,credentialIndex=%d]",
                        commandPath.mEndpointId, credentialIndex);

        sendSetCredentialResponse(commandObj, commandPath, DlStatus::kFailure, 0, nextAvailableCredentialSlot);
        return;
    }

    if (!IsValidUserStatusForSet(userStatus))
    {
        ChipLogProgress(Zcl,
                        "[SetCredential] Unable to set the credential: user status is out of range "
                        "[endpointId=%d,credentialIndex=%d,userStatus=%u]",
                        commandPath.mEndpointId, credentialIndex, to_underlying(userStatus.Value()));
        sendSetCredentialResponse(commandObj, commandPath, DlStatus::kInvalidField, 0, nextAvailableCredentialSlot);
        return;
    }

    if (userType == UserTypeEnum::kUnknownEnumValue)
    {
        ChipLogProgress(Zcl,
                        "[SetCredential] Unable to set the credential: user type is unknown "
                        "[endpointId=%d,credentialIndex=%d,userType=%u]",
                        commandPath.mEndpointId, credentialIndex, to_underlying(userType.Value()));
        sendSetCredentialResponse(commandObj, commandPath, DlStatus::kInvalidField, 0, nextAvailableCredentialSlot);
        return;
    }

    switch (operationType)
    {
    case DataOperationTypeEnum::kAdd: {
        uint16_t createdUserIndex = 0;

        status = createCredential(commandPath.mEndpointId, fabricIdx, sourceNodeId, credentialIndex, credentialType,
                                  existingCredential, credentialData, userIndex, userStatus, userType, createdUserIndex);

        sendSetCredentialResponse(commandObj, commandPath, status, createdUserIndex, nextAvailableCredentialSlot);
        return;
    }
    case DataOperationTypeEnum::kModify: {
        // appclusters, 5.2.4.41.1: should send the INVALID_COMMAND in the response when the credential is in use
        if (DlCredentialStatus::kAvailable == existingCredential.status)
        {
            ChipLogProgress(Zcl,
                            "[SetCredential] Unable to modify the credential: credential slot is not occupied "
                            "[endpointId=%d,credentialIndex=%d]",
                            commandPath.mEndpointId, credentialIndex);

            sendSetCredentialResponse(commandObj, commandPath, DlStatus::kInvalidField, 0, nextAvailableCredentialSlot);
            return;
        }

        // if userIndex is NULL then we're changing the programming user PIN
        if (userIndex.IsNull())
        {
            if (!userStatus.IsNull() || userType != UserTypeEnum::kProgrammingUser)
            {
                ChipLogProgress(Zcl,
                                "[SetCredential] Unable to modify programming PIN: invalid argument "
                                "[endpointId=%d,credentialIndex=%d]",
                                commandPath.mEndpointId, credentialIndex);
            }
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
    case DataOperationTypeEnum::kClear:
    default:
        // appclusters, 5.2.4.40: set credential command supports only Add and Modify operational type.
        sendSetCredentialResponse(commandObj, commandPath, DlStatus::kInvalidField, 0, nextAvailableCredentialSlot);
    }
}

void DoorLockServer::getCredentialStatusCommandHandler(chip::app::CommandHandler * commandObj,
                                                       const chip::app::ConcreteCommandPath & commandPath,
                                                       CredentialTypeEnum credentialType, uint16_t credentialIndex)
{
    ChipLogProgress(Zcl, "[GetCredentialStatus] Incoming command [endpointId=%d]", commandPath.mEndpointId);

    if (!credentialTypeSupported(commandPath.mEndpointId, credentialType))
    {
        ChipLogProgress(Zcl,
                        "[GetCredentialStatus] Credential type is not supported [endpointId=%d,credentialType=%u"
                        "]",
                        commandPath.mEndpointId, to_underlying(credentialType));
        commandObj->AddStatus(commandPath, Status::UnsupportedCommand);
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
        ChipLogProgress(Zcl,
                        "[GetCredentialStatus] Unable to get the credential: app error "
                        "[endpointId=%d,credentialIndex=%d,credentialType=%u,creator=%u,modifier=%u]",
                        commandPath.mEndpointId, credentialIndex, to_underlying(credentialType), credentialInfo.createdBy,
                        credentialInfo.lastModifiedBy);
        commandObj->AddStatus(commandPath, Status::Failure);
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
            commandObj->AddStatus(commandPath, Status::Failure);
            return;
        }
    }

    sendGetCredentialResponse(commandObj, commandPath, credentialType, credentialIndex, userIndexWithCredential, &credentialInfo,
                              credentialExists);
}

void DoorLockServer::sendGetCredentialResponse(chip::app::CommandHandler * commandObj,
                                               const chip::app::ConcreteCommandPath & commandPath,
                                               CredentialTypeEnum credentialType, uint16_t credentialIndex,
                                               uint16_t userIndexWithCredential,
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

    ChipLogProgress(Zcl,
                    "[GetCredentialStatus] Prepared credential status "
                    "[endpointId=%d,credentialType=%u,credentialIndex=%d,userIndex=%d,nextCredentialIndex=%d]",
                    commandPath.mEndpointId, to_underlying(credentialType), credentialIndex, userIndexWithCredential,
                    nextCredentialIndex);
}

void DoorLockServer::clearCredentialCommandHandler(
    chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
    const chip::app::Clusters::DoorLock::Commands::ClearCredential::DecodableType & commandData)
{
    ChipLogProgress(Zcl, "[ClearCredential] Incoming command [endpointId=%d]", commandPath.mEndpointId);

    auto modifier = getFabricIndex(commandObj);
    if (kUndefinedFabricIndex == modifier)
    {
        commandObj->AddStatus(commandPath, Status::Failure);
        return;
    }

    auto sourceNodeId = getNodeId(commandObj);
    if (chip::kUndefinedNodeId == sourceNodeId)
    {
        commandObj->AddStatus(commandPath, Status::Failure);
        return;
    }

    const auto & credential = commandData.credential;
    if (credential.IsNull())
    {
        ChipLogProgress(Zcl, "[ClearCredential] Clearing all credentials [endpointId=%d]", commandPath.mEndpointId);
        commandObj->AddStatus(commandPath, clearCredentials(commandPath.mEndpointId, modifier, sourceNodeId));
        return;
    }

    // Remove all the credentials of the particular type.
    auto credentialType  = credential.Value().credentialType;
    auto credentialIndex = credential.Value().credentialIndex;
    if (0xFFFE == credentialIndex)
    {
        commandObj->AddStatus(commandPath, clearCredentials(commandPath.mEndpointId, modifier, sourceNodeId, credentialType));
        return;
    }

    commandObj->AddStatus(commandPath,
                          clearCredential(commandPath.mEndpointId, modifier, sourceNodeId, credentialType, credentialIndex, false));
}

void DoorLockServer::setWeekDayScheduleCommandHandler(chip::app::CommandHandler * commandObj,
                                                      const chip::app::ConcreteCommandPath & commandPath, uint8_t weekDayIndex,
                                                      uint16_t userIndex, const chip::BitMask<DaysMaskMap> & daysMask,
                                                      uint8_t startHour, uint8_t startMinute, uint8_t endHour, uint8_t endMinute)
{
    auto endpointId = commandPath.mEndpointId;
    if (!SupportsWeekDaySchedules(endpointId))
    {
        ChipLogProgress(Zcl, "[SetWeekDaySchedule] Ignore command (not supported) [endpointId=%d]", endpointId);
        commandObj->AddStatus(commandPath, Status::InvalidCommand);
        return;
    }

    ChipLogProgress(Zcl, "[SetWeekDaySchedule] Incoming command [endpointId=%d]", endpointId);

    auto fabricIdx = getFabricIndex(commandObj);
    if (kUndefinedFabricIndex == fabricIdx)
    {
        ChipLogError(Zcl, "[SetWeekDaySchedule] Unable to get the fabric IDX [endpointId=%d]", commandPath.mEndpointId);
        commandObj->AddStatus(commandPath, Status::Failure);
        return;
    }

    auto sourceNodeId = getNodeId(commandObj);
    if (chip::kUndefinedNodeId == sourceNodeId)
    {
        ChipLogError(Zcl, "[SetWeekDaySchedule] Unable to get the source node index [endpointId=%d]", commandPath.mEndpointId);
        commandObj->AddStatus(commandPath, Status::Failure);
        return;
    }

    if (!weekDayIndexValid(endpointId, weekDayIndex) || !userIndexValid(endpointId, userIndex))
    {
        ChipLogProgress(
            Zcl, "[SetWeekDaySchedule] Unable to add schedule - index out of range [endpointId=%d,weekDayIndex=%d,userIndex=%d]",
            endpointId, weekDayIndex, userIndex);
        commandObj->AddStatus(commandPath, Status::InvalidCommand);
        return;
    }

    if (!userExists(endpointId, userIndex))
    {
        ChipLogProgress(Zcl,
                        "[SetWeekDaySchedule] Unable to add schedule - user does not exist "
                        "[endpointId=%d,weekDayIndex=%d,userIndex=%d]",
                        endpointId, weekDayIndex, userIndex);
        commandObj->AddStatus(commandPath, Status::Failure);
        return;
    }

    uint8_t rawDaysMask = daysMask.Raw();

    // Check that bits are within range
    if ((0 == rawDaysMask) || (rawDaysMask & 0x80))
    {
        ChipLogProgress(Zcl,
                        "[SetWeekDaySchedule] Unable to add schedule - daysMask is out of range "
                        "[endpointId=%d,weekDayIndex=%d,userIndex=%d,daysMask=%x]",
                        endpointId, weekDayIndex, userIndex, daysMask.Raw());
        commandObj->AddStatus(commandPath, Status::InvalidCommand);
        return;
    }

    if (startHour > DOOR_LOCK_SCHEDULE_MAX_HOUR || startMinute > DOOR_LOCK_SCHEDULE_MAX_MINUTE ||
        endHour > DOOR_LOCK_SCHEDULE_MAX_HOUR || endMinute > DOOR_LOCK_SCHEDULE_MAX_MINUTE)
    {
        ChipLogProgress(Zcl,
                        "[SetWeekDaySchedule] Unable to add schedule - start time out of range "
                        "[endpointId=%d,weekDayIndex=%d,userIndex=%d,startTime=\"%d:%d\",endTime=\"%d:%d\"]",
                        endpointId, weekDayIndex, userIndex, startHour, startMinute, endHour, endMinute);
        commandObj->AddStatus(commandPath, Status::InvalidCommand);
        return;
    }

    if (startHour > endHour || (startHour == endHour && startMinute >= endMinute))
    {
        ChipLogProgress(Zcl,
                        "[SetWeekDaySchedule] Unable to add schedule - invalid time "
                        "[endpointId=%d,weekDayIndex=%d,userIndex=%d,startTime=\"%d:%d\",endTime=\"%d:%d\"]",
                        endpointId, weekDayIndex, userIndex, startHour, startMinute, endHour, endMinute);
        commandObj->AddStatus(commandPath, Status::InvalidCommand);
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
        commandObj->AddStatus(commandPath, Status::Failure);
        return;
    }

    ChipLogProgress(Zcl,
                    "[SetWeekDaySchedule] Successfully created new schedule "
                    "[endpointId=%d,weekDayIndex=%d,userIndex=%d,daysMask=%d,startTime=\"%d:%d\",endTime=\"%d:%d\"]",
                    endpointId, weekDayIndex, userIndex, daysMask.Raw(), startHour, startMinute, endHour, endMinute);

    sendRemoteLockUserChange(endpointId, LockDataTypeEnum::kWeekDaySchedule, DataOperationTypeEnum::kAdd, sourceNodeId, fabricIdx,
                             userIndex, static_cast<uint16_t>(weekDayIndex));

    commandObj->AddStatus(commandPath, Status::Success);
}

void DoorLockServer::getWeekDayScheduleCommandHandler(chip::app::CommandHandler * commandObj,
                                                      const chip::app::ConcreteCommandPath & commandPath, uint8_t weekDayIndex,
                                                      uint16_t userIndex)
{
    auto endpointId = commandPath.mEndpointId;
    if (!SupportsWeekDaySchedules(endpointId))
    {
        ChipLogProgress(Zcl, "[GetWeekDaySchedule] Ignore command (not supported) [endpointId=%d]", endpointId);
        commandObj->AddStatus(commandPath, Status::InvalidCommand);
        return;
    }

    ChipLogProgress(Zcl, "[GetWeekDaySchedule] Incoming command [endpointId=%d]", endpointId);

    if (!weekDayIndexValid(endpointId, weekDayIndex) || !userIndexValid(endpointId, userIndex))
    {
        ChipLogProgress(
            Zcl, "[GetWeekDaySchedule] Unable to get schedule - index out of range [endpointId=%d,weekDayIndex=%d,userIndex=%d]",
            endpointId, weekDayIndex, userIndex);
        sendGetWeekDayScheduleResponse(commandObj, commandPath, weekDayIndex, userIndex, DlStatus::kInvalidField);
        return;
    }

    if (!userExists(endpointId, userIndex))
    {
        ChipLogProgress(Zcl, "[GetWeekDaySchedule] User does not exist [endpointId=%d,weekDayIndex=%d,userIndex=%d]", endpointId,
                        weekDayIndex, userIndex);
        sendGetWeekDayScheduleResponse(commandObj, commandPath, weekDayIndex, userIndex, DlStatus::kNotFound);
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
        ChipLogProgress(Zcl, "[ClearWeekDaySchedule] Ignore command (not supported) [endpointId=%d]", endpointId);
        commandObj->AddStatus(commandPath, Status::InvalidCommand);
        return;
    }

    ChipLogProgress(Zcl, "[ClearWeekDaySchedule] Incoming command [endpointId=%d]", endpointId);

    auto fabricIdx = getFabricIndex(commandObj);
    if (kUndefinedFabricIndex == fabricIdx)
    {
        ChipLogError(Zcl, "[ClearWeekDaySchedule] Unable to get the fabric IDX [endpointId=%d]", commandPath.mEndpointId);
        commandObj->AddStatus(commandPath, Status::Failure);
        return;
    }

    auto sourceNodeId = getNodeId(commandObj);
    if (chip::kUndefinedNodeId == sourceNodeId)
    {
        ChipLogError(Zcl, "[ClearWeekDaySchedule] Unable to get the source node index [endpointId=%d]", commandPath.mEndpointId);
        commandObj->AddStatus(commandPath, Status::Failure);
        return;
    }

    if (!userIndexValid(endpointId, userIndex) || (!weekDayIndexValid(endpointId, weekDayIndex) && 0xFE != weekDayIndex))
    {
        ChipLogProgress(Zcl,
                        "[ClearWeekDaySchedule] User or WeekDay index is out of range [endpointId=%d,weekDayIndex=%d,userIndex=%d]",
                        endpointId, weekDayIndex, userIndex);
        commandObj->AddStatus(commandPath, Status::InvalidCommand);
        return;
    }

    if (!userExists(endpointId, userIndex))
    {
        ChipLogProgress(Zcl, "[ClearWeekDaySchedule] User does not exist [endpointId=%d,weekDayIndex=%d,userIndex=%d]", endpointId,
                        weekDayIndex, userIndex);
        commandObj->AddStatus(commandPath, Status::Failure);
        return;
    }

    DlStatus clearStatus = DlStatus::kSuccess;
    if (0xFE == weekDayIndex)
    {
        ChipLogProgress(Zcl, "[ClearWeekDaySchedule] Clearing all schedules for a single user [endpointId=%d,userIndex=%d]",
                        endpointId, userIndex);
        clearStatus = clearWeekDaySchedules(endpointId, userIndex);
    }
    else
    {
        ChipLogProgress(Zcl, "[ClearWeekDaySchedule] Clearing a single schedule [endpointId=%d,weekDayIndex=%d,userIndex=%d]",
                        endpointId, weekDayIndex, userIndex);
        clearStatus = clearWeekDaySchedule(endpointId, userIndex, weekDayIndex);
    }

    if (DlStatus::kSuccess != clearStatus)
    {
        ChipLogProgress(
            Zcl, "[ClearWeekDaySchedule] Unable to clear the user schedules - app error [endpointId=%d,userIndex=%d,status=%u]",
            endpointId, userIndex, to_underlying(clearStatus));
        commandObj->AddStatus(commandPath, Status::Failure);
        return;
    }

    sendRemoteLockUserChange(endpointId, LockDataTypeEnum::kWeekDaySchedule, DataOperationTypeEnum::kClear, sourceNodeId, fabricIdx,
                             userIndex, static_cast<uint16_t>(weekDayIndex));

    commandObj->AddStatus(commandPath, Status::Success);
}

void DoorLockServer::setYearDayScheduleCommandHandler(chip::app::CommandHandler * commandObj,
                                                      const chip::app::ConcreteCommandPath & commandPath, uint8_t yearDayIndex,
                                                      uint16_t userIndex, uint32_t localStartTime, uint32_t localEndTime)
{
    auto endpointId = commandPath.mEndpointId;
    if (!SupportsYearDaySchedules(endpointId))
    {
        ChipLogProgress(Zcl, "[SetYearDaySchedule] Ignore command (not supported) [endpointId=%d]", endpointId);
        commandObj->AddStatus(commandPath, Status::InvalidCommand);
        return;
    }

    ChipLogProgress(Zcl, "[SetYearDaySchedule] incoming command [endpointId=%d]", endpointId);

    auto fabricIdx = getFabricIndex(commandObj);
    if (kUndefinedFabricIndex == fabricIdx)
    {
        ChipLogError(Zcl, "[SetYearDaySchedule] Unable to get the fabric IDX [endpointId=%d]", commandPath.mEndpointId);
        commandObj->AddStatus(commandPath, Status::Failure);
        return;
    }

    auto sourceNodeId = getNodeId(commandObj);
    if (chip::kUndefinedNodeId == sourceNodeId)
    {
        ChipLogError(Zcl, "[SetYearDaySchedule] Unable to get the source node index [endpointId=%d]", commandPath.mEndpointId);
        commandObj->AddStatus(commandPath, Status::Failure);
        return;
    }

    if (!yearDayIndexValid(endpointId, yearDayIndex) || !userIndexValid(endpointId, userIndex))
    {
        ChipLogProgress(
            Zcl, "[SetYearDaySchedule] Unable to add schedule - index out of range [endpointId=%d,yearDayIndex=%d,userIndex=%d]",
            endpointId, yearDayIndex, userIndex);
        commandObj->AddStatus(commandPath, Status::InvalidCommand);
        return;
    }

    if (!userExists(endpointId, userIndex))
    {
        ChipLogProgress(Zcl,
                        "[SetYearDaySchedule] Unable to add schedule - user does not exist "
                        "[endpointId=%d,yearDayIndex=%d,userIndex=%d]",
                        endpointId, yearDayIndex, userIndex);
        commandObj->AddStatus(commandPath, Status::Failure);
        return;
    }

    if (localEndTime <= localStartTime)
    {
        ChipLogProgress(Zcl,
                        "[SetYearDaySchedule] Unable to add schedule - schedule ends earlier than starts"
                        "[endpointId=%d,yearDayIndex=%d,userIndex=%d,localStarTime=%" PRIu32 ",localEndTime=%" PRIu32 "]",
                        endpointId, yearDayIndex, userIndex, localStartTime, localEndTime);
        commandObj->AddStatus(commandPath, Status::InvalidCommand);
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
        commandObj->AddStatus(commandPath, Status::Failure);
        return;
    }

    ChipLogProgress(Zcl,
                    "[SetYearDaySchedule] Successfully created new schedule "
                    "[endpointId=%d,yearDayIndex=%d,userIndex=%d,localStartTime=%" PRIu32 ",endTime=%" PRIu32 "]",
                    endpointId, yearDayIndex, userIndex, localStartTime, localEndTime);

    sendRemoteLockUserChange(endpointId, LockDataTypeEnum::kYearDaySchedule, DataOperationTypeEnum::kAdd, sourceNodeId, fabricIdx,
                             userIndex, static_cast<uint16_t>(yearDayIndex));

    commandObj->AddStatus(commandPath, Status::Success);
}

void DoorLockServer::getYearDayScheduleCommandHandler(chip::app::CommandHandler * commandObj,
                                                      const chip::app::ConcreteCommandPath & commandPath, uint8_t yearDayIndex,
                                                      uint16_t userIndex)
{
    auto endpointId = commandPath.mEndpointId;
    if (!SupportsYearDaySchedules(endpointId))
    {
        ChipLogProgress(Zcl, "[GetYearDaySchedule] Ignore command (not supported) [endpointId=%d]", endpointId);
        commandObj->AddStatus(commandPath, Status::InvalidCommand);
        return;
    }
    ChipLogProgress(Zcl, "[GetYearDaySchedule] incoming command [endpointId=%d]", endpointId);

    if (!yearDayIndexValid(endpointId, yearDayIndex) || !userIndexValid(endpointId, userIndex))
    {
        ChipLogProgress(
            Zcl, "[GetYearDaySchedule] Unable to get schedule - index out of range [endpointId=%d,yearDayIndex=%d,userIndex=%d]",
            endpointId, yearDayIndex, userIndex);
        sendGetYearDayScheduleResponse(commandObj, commandPath, yearDayIndex, userIndex, DlStatus::kInvalidField);
        return;
    }

    if (!userExists(endpointId, userIndex))
    {
        ChipLogProgress(Zcl, "[GetYearDaySchedule] User does not exist [endpointId=%d,yearDayIndex=%d,userIndex=%d]", endpointId,
                        yearDayIndex, userIndex);
        sendGetYearDayScheduleResponse(commandObj, commandPath, yearDayIndex, userIndex, DlStatus::kNotFound);
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
        ChipLogProgress(Zcl, "[ClearYearDaySchedule] Ignore command (not supported) [endpointId=%d]", endpointId);
        commandObj->AddStatus(commandPath, Status::InvalidCommand);
        return;
    }
    ChipLogProgress(Zcl, "[ClearYearDaySchedule] incoming command [endpointId=%d]", endpointId);

    auto fabricIdx = getFabricIndex(commandObj);
    if (kUndefinedFabricIndex == fabricIdx)
    {
        ChipLogError(Zcl, "[ClearYearDaySchedule] Unable to get the fabric IDX [endpointId=%d]", commandPath.mEndpointId);
        commandObj->AddStatus(commandPath, Status::Failure);
        return;
    }

    auto sourceNodeId = getNodeId(commandObj);
    if (chip::kUndefinedNodeId == sourceNodeId)
    {
        ChipLogError(Zcl, "[ClearYearDaySchedule] Unable to get the source node index [endpointId=%d]", commandPath.mEndpointId);
        commandObj->AddStatus(commandPath, Status::Failure);
        return;
    }

    if (!userIndexValid(endpointId, userIndex) || (!yearDayIndexValid(endpointId, yearDayIndex) && 0xFE != yearDayIndex))
    {
        ChipLogProgress(Zcl,
                        "[ClearYearDaySchedule] User or YearDay index is out of range [endpointId=%d,yearDayIndex=%d,userIndex=%d]",
                        endpointId, yearDayIndex, userIndex);
        commandObj->AddStatus(commandPath, Status::InvalidCommand);
        return;
    }

    if (!userExists(endpointId, userIndex))
    {
        ChipLogProgress(Zcl, "[ClearYearDaySchedule] User does not exist [endpointId=%d,yearDayIndex=%d,userIndex=%d]", endpointId,
                        yearDayIndex, userIndex);
        commandObj->AddStatus(commandPath, Status::Failure);
        return;
    }

    DlStatus clearStatus = DlStatus::kSuccess;
    if (0xFE == yearDayIndex)
    {
        ChipLogProgress(Zcl, "[ClearYearDaySchedule] Clearing all schedules for a single user [endpointId=%d,userIndex=%d]",
                        endpointId, userIndex);
        clearStatus = clearYearDaySchedules(endpointId, userIndex);
    }
    else
    {
        ChipLogProgress(Zcl, "[ClearYearDaySchedule] Clearing a single schedule [endpointId=%d,yearDayIndex=%d,userIndex=%d]",
                        endpointId, yearDayIndex, userIndex);
        clearStatus = clearYearDaySchedule(endpointId, userIndex, yearDayIndex);
    }

    if (DlStatus::kSuccess != clearStatus)
    {
        ChipLogProgress(
            Zcl, "[ClearYearDaySchedule] Unable to clear the user schedules - app error [endpointId=%d,userIndex=%d,status=%u]",
            endpointId, userIndex, to_underlying(clearStatus));
        commandObj->AddStatus(commandPath, Status::Failure);
        return;
    }

    sendRemoteLockUserChange(endpointId, LockDataTypeEnum::kYearDaySchedule, DataOperationTypeEnum::kClear, sourceNodeId, fabricIdx,
                             userIndex, static_cast<uint16_t>(yearDayIndex));

    commandObj->AddStatus(commandPath, Status::Success);
}

chip::BitFlags<Feature> DoorLockServer::GetFeatures(chip::EndpointId endpointId)
{
    chip::BitFlags<Feature> featureMap;
    if (!GetAttribute(endpointId, Attributes::FeatureMap::Id, Attributes::FeatureMap::Get, *featureMap.RawStorage()))
    {
        ChipLogError(Zcl, "Unable to get the door lock feature map: attribute read error");
        featureMap.ClearAll();
    }
    return featureMap;
}

bool DoorLockServer::OnFabricRemoved(chip::EndpointId endpointId, chip::FabricIndex fabricIndex)
{
    ChipLogProgress(Zcl, "[OnFabricRemoved] Handling a fabric removal from the door lock server [endpointId=%d,fabricIndex=%d]",
                    endpointId, fabricIndex);

    bool status{ true };
    // Iterate over all the users and clean up the deleted fabric
    if (!clearFabricFromUsers(endpointId, fabricIndex))
    {
        ChipLogError(Zcl, "[OnFabricRemoved] Unable to cleanup fabric from users - internal error [endpointId=%d,fabricIndex=%d]",
                     endpointId, fabricIndex);
        status = false;
    }

    // Iterate over all the credentials and clean up the fabrics
    if (!clearFabricFromCredentials(endpointId, fabricIndex))
    {
        ChipLogError(Zcl,
                     "[OnFabricRemoved] Unable to cleanup fabric from credentials - internal error [endpointId=%d,fabricIndex=%d]",
                     endpointId, fabricIndex);
        status = false;
    }

    if (mOnFabricRemovedCustomCallback)
    {
        mOnFabricRemovedCustomCallback(endpointId, fabricIndex);
    }

    return status;
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
    // TODO: Why are we doing all these checks?  At all the callsites we have
    // just received a command, so we better have a handler, exchange, session,
    // etc.  The only thing we should be checking is that it's a CASE session.
    if (nullptr == commandObj || nullptr == commandObj->GetExchangeContext())
    {
        ChipLogError(Zcl, "Cannot access ExchangeContext of Command Object for Node ID");
        return kUndefinedNodeId;
    }

    if (!commandObj->GetExchangeContext()->HasSessionHandle())
    {
        ChipLogError(Zcl, "Cannot access session of Command Object for Node ID");
        return kUndefinedNodeId;
    }

    auto descriptor = commandObj->GetExchangeContext()->GetSessionHandle()->GetSubjectDescriptor();
    if (descriptor.authMode != Access::AuthMode::kCase)
    {
        ChipLogError(Zcl, "Cannot get Node ID from non-CASE session of Command Object");
        return kUndefinedNodeId;
    }

    return descriptor.subject;
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
    return UserStatusEnum::kAvailable != user.userStatus;
}

bool DoorLockServer::credentialIndexValid(chip::EndpointId endpointId, CredentialTypeEnum type, uint16_t credentialIndex)
{
    uint16_t maxCredentials = 0;
    return credentialIndexValid(endpointId, type, credentialIndex, maxCredentials);
}

bool DoorLockServer::credentialIndexValid(chip::EndpointId endpointId, CredentialTypeEnum type, uint16_t credentialIndex,
                                          uint16_t & maxNumberOfCredentials)
{
    if (!getMaxNumberOfCredentials(endpointId, type, maxNumberOfCredentials))
    {
        return false;
    }

    // appclusters, 5.2.6.3.1: 0 is allowed index for Programming PIN credential only
    if (CredentialTypeEnum::kProgrammingPIN == type)
    {
        return (0 == credentialIndex);
    }

    if (0 == credentialIndex || credentialIndex > maxNumberOfCredentials)
    {
        return false;
    }

    return true;
}

DlStatus DoorLockServer::credentialLengthWithinRange(chip::EndpointId endpointId, CredentialTypeEnum type,
                                                     const chip::ByteSpan & credentialData)
{
    bool statusMin = true, statusMax = true;
    uint8_t minLen, maxLen;

    switch (type)
    {
    case CredentialTypeEnum::kProgrammingPIN:
    case CredentialTypeEnum::kPin:
        statusMin = GetAttribute(endpointId, Attributes::MinPINCodeLength::Id, Attributes::MinPINCodeLength::Get, minLen);
        statusMax = GetAttribute(endpointId, Attributes::MaxPINCodeLength::Id, Attributes::MaxPINCodeLength::Get, maxLen);
        break;
    case CredentialTypeEnum::kRfid:
        statusMin = GetAttribute(endpointId, Attributes::MinRFIDCodeLength::Id, Attributes::MinRFIDCodeLength::Get, minLen);
        statusMax = GetAttribute(endpointId, Attributes::MaxRFIDCodeLength::Id, Attributes::MaxRFIDCodeLength::Get, maxLen);
        break;
    case CredentialTypeEnum::kFingerprint:
        statusMin = statusMax = emberAfPluginDoorLockGetFingerprintCredentialLengthConstraints(endpointId, minLen, maxLen);
        break;
    case CredentialTypeEnum::kFingerVein:
        statusMin = statusMax = emberAfPluginDoorLockGetFingerVeinCredentialLengthConstraints(endpointId, minLen, maxLen);
        break;
    case CredentialTypeEnum::kFace:
        statusMin = statusMax = emberAfPluginDoorLockGetFaceCredentialLengthConstraints(endpointId, minLen, maxLen);
        break;
    case CredentialTypeEnum::kAliroCredentialIssuerKey:
    case CredentialTypeEnum::kAliroEvictableEndpointKey:
    case CredentialTypeEnum::kAliroNonEvictableEndpointKey:
        minLen = maxLen = DOOR_LOCK_ALIRO_CREDENTIAL_SIZE;
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
        ChipLogProgress(Zcl,
                        "Credential data size is out of range "
                        "[endpointId=%d,credentialType=%u,minLength=%u,maxLength=%u,length=%u]",
                        endpointId, to_underlying(type), minLen, maxLen, static_cast<unsigned int>(credentialData.size()));
        return DlStatus::kInvalidField;
    }

    return DlStatus::kSuccess;
}

bool DoorLockServer::getMaxNumberOfCredentials(chip::EndpointId endpointId, CredentialTypeEnum credentialType,
                                               uint16_t & maxNumberOfCredentials)
{
    maxNumberOfCredentials = 0;
    bool status            = false;
    switch (credentialType)
    {
    case CredentialTypeEnum::kProgrammingPIN:
        maxNumberOfCredentials = 1;
        return true;
    case CredentialTypeEnum::kPin:
        status = GetNumberOfPINCredentialsSupported(endpointId, maxNumberOfCredentials);
        break;
    case CredentialTypeEnum::kRfid:
        status = GetNumberOfRFIDCredentialsSupported(endpointId, maxNumberOfCredentials);
        break;
    case CredentialTypeEnum::kFingerprint:
        status = emberAfPluginDoorLockGetNumberOfFingerprintCredentialsSupported(endpointId, maxNumberOfCredentials);
        break;
    case CredentialTypeEnum::kFingerVein:
        status = emberAfPluginDoorLockGetNumberOfFingerVeinCredentialsSupported(endpointId, maxNumberOfCredentials);
        break;
    case CredentialTypeEnum::kFace:
        status = emberAfPluginDoorLockGetNumberOfFaceCredentialsSupported(endpointId, maxNumberOfCredentials);
        break;
    case CredentialTypeEnum::kAliroCredentialIssuerKey: {
        Delegate * delegate = GetDelegate(endpointId);
        if (delegate == nullptr)
        {
            ChipLogError(Zcl, "Delegate is null");
            return false;
        }

        maxNumberOfCredentials = delegate->GetNumberOfAliroCredentialIssuerKeysSupported();
        status                 = true;
        break;
    }
    case CredentialTypeEnum::kAliroEvictableEndpointKey:
    case CredentialTypeEnum::kAliroNonEvictableEndpointKey: {
        Delegate * delegate = GetDelegate(endpointId);
        if (delegate == nullptr)
        {
            ChipLogError(Zcl, "Delegate is null");
            return false;
        }

        // For AliroEvictableEndpointKey and AliroNonEvictableEndpointKey credential type, return the total
        // number of endpoint keys supported.
        maxNumberOfCredentials = delegate->GetNumberOfAliroEndpointKeysSupported();
        status                 = true;
        break;
    }
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

        if (UserStatusEnum::kAvailable != user.userStatus)
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

        if (UserStatusEnum::kAvailable == user.userStatus)
        {
            userIndex = i;
            return true;
        }
    }
    return false;
}

bool DoorLockServer::findOccupiedCredentialSlot(chip::EndpointId endpointId, CredentialTypeEnum credentialType, uint16_t startIndex,
                                                uint16_t & credentialIndex)
{
    uint16_t maxNumberOfCredentials = 0;
    if (!getMaxNumberOfCredentials(endpointId, credentialType, maxNumberOfCredentials))
    {
        return false;
    }

    // Programming PIN index starts with 0, and it is assumed that it is unique. Therefore different bounds checking for that
    // credential type
    if (CredentialTypeEnum::kProgrammingPIN == credentialType)
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

bool DoorLockServer::findUnoccupiedCredentialSlot(chip::EndpointId endpointId, CredentialTypeEnum credentialType,
                                                  uint16_t startIndex, uint16_t & credentialIndex)
{
    uint16_t maxNumberOfCredentials = 0;
    if (!getMaxNumberOfCredentials(endpointId, credentialType, maxNumberOfCredentials))
    {
        return false;
    }

    // Programming PIN index starts with 0, and it is assumed that it is unique. Therefor different bounds checking for that
    // credential type
    if (CredentialTypeEnum::kProgrammingPIN == credentialType)
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

bool DoorLockServer::findUserIndexByCredential(chip::EndpointId endpointId, CredentialTypeEnum credentialType,
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
            return false;
        }

        // Go through occupied users only
        if (UserStatusEnum::kAvailable == user.userStatus)
        {
            continue;
        }

        for (size_t j = 0; j < user.credentials.size(); ++j)
        {
            if (user.credentials.data()[j].credentialIndex == credentialIndex &&
                user.credentials.data()[j].credentialType == credentialType)
            {
                userIndex = i;
                return true;
            }
        }
    }

    return false;
}

bool DoorLockServer::findUserIndexByCredential(chip::EndpointId endpointId, CredentialTypeEnum credentialType,
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
            return false;
        }

        // Go through occupied users only
        if (UserStatusEnum::kAvailable == user.userStatus)
        {
            continue;
        }

        for (const auto & credential : user.credentials)
        {
            if (credential.credentialType != credentialType)
            {
                continue;
            }

            EmberAfPluginDoorLockCredentialInfo credentialInfo;
            if (!emberAfPluginDoorLockGetCredential(endpointId, credential.credentialIndex, credentialType, credentialInfo))
            {
                ChipLogError(Zcl,
                             "[findUserIndexByCredential] Unable to get credential: app error "
                             "[userIndex=%d,credentialIndex=%d,credentialType=%u]",
                             i, credential.credentialIndex, to_underlying(credentialType));
                return false;
            }

            if (credentialInfo.status != DlCredentialStatus::kOccupied)
            {
                ChipLogError(Zcl,
                             "[findUserIndexByCredential] Users/Credentials database error: credential index attached to user is "
                             "not occupied "
                             "[userIndex=%d,credentialIndex=%d,credentialType=%u]",
                             i, credential.credentialIndex, to_underlying(credentialType));
                return false;
            }

            if (credentialInfo.credentialData.data_equal(credentialData))
            {
                userIndex       = i;
                credentialIndex = credential.credentialIndex;
                userInfo        = user;
                return true;
            }
        }
    }

    return false;
}

ClusterStatusCode DoorLockServer::createUser(chip::EndpointId endpointId, chip::FabricIndex creatorFabricIdx,
                                             chip::NodeId sourceNodeId, uint16_t userIndex,
                                             const Nullable<chip::CharSpan> & userName, const Nullable<uint32_t> & userUniqueId,
                                             const Nullable<UserStatusEnum> & userStatus, const Nullable<UserTypeEnum> & userType,
                                             const Nullable<CredentialRuleEnum> & credentialRule,
                                             const Nullable<CredentialStruct> & credential)
{
    EmberAfPluginDoorLockUserInfo user;
    if (!emberAfPluginDoorLockGetUser(endpointId, userIndex, user))
    {
        ChipLogError(Zcl, "[createUser] Unable to get the user from app [endpointId=%d,userIndex=%d]", endpointId, userIndex);
        return ClusterStatusCode(Status::Failure);
    }

    // appclusters, 5.2.4.34: to modify user its status should be set to Available. If it is we should return OCCUPIED.
    if (UserStatusEnum::kAvailable != user.userStatus)
    {
        ChipLogProgress(Zcl, "[createUser] Unable to overwrite existing user [endpointId=%d,userIndex=%d]", endpointId, userIndex);
        return ClusterStatusCode::ClusterSpecificFailure(DlStatus::kOccupied);
    }

    const auto & newUserName                = !userName.IsNull() ? userName.Value() : ""_span;
    auto newUserUniqueId                    = userUniqueId.IsNull() ? 0xFFFFFFFF : userUniqueId.Value();
    auto newUserStatus                      = userStatus.IsNull() ? UserStatusEnum::kOccupiedEnabled : userStatus.Value();
    auto newUserType                        = userType.IsNull() ? UserTypeEnum::kUnrestrictedUser : userType.Value();
    auto newCredentialRule                  = credentialRule.IsNull() ? CredentialRuleEnum::kSingle : credentialRule.Value();
    const CredentialStruct * newCredentials = nullptr;
    size_t newTotalCredentials              = 0;
    if (!credential.IsNull())
    {
        newCredentials      = &credential.Value();
        newTotalCredentials = 1;
    }

    if (!emberAfPluginDoorLockSetUser(endpointId, userIndex, creatorFabricIdx, creatorFabricIdx, newUserName, newUserUniqueId,
                                      newUserStatus, newUserType, newCredentialRule, newCredentials, newTotalCredentials))
    {
        ChipLogProgress(Zcl,
                        "[createUser] Unable to create user: app error "
                        "[endpointId=%d,creatorFabricId=%d,userIndex=%d,userName=\"%.*s\",userUniqueId=0x%" PRIx32 ",userStatus=%u,"
                        "userType=%u,credentialRule=%u,totalCredentials=%u]",
                        endpointId, creatorFabricIdx, userIndex, static_cast<int>(newUserName.size()), newUserName.data(),
                        newUserUniqueId, to_underlying(newUserStatus), to_underlying(newUserType), to_underlying(newCredentialRule),
                        static_cast<unsigned int>(newTotalCredentials));
        return ClusterStatusCode(Status::Failure);
    }

    ChipLogProgress(Zcl,
                    "[createUser] User created "
                    "[endpointId=%d,creatorFabricId=%d,userIndex=%d,userName=\"%.*s\",userUniqueId=0x%" PRIx32 ",userStatus=%u,"
                    "userType=%u,credentialRule=%u,totalCredentials=%u]",
                    endpointId, creatorFabricIdx, userIndex, static_cast<int>(newUserName.size()), newUserName.data(),
                    newUserUniqueId, to_underlying(newUserStatus), to_underlying(newUserType), to_underlying(newCredentialRule),
                    static_cast<unsigned int>(newTotalCredentials));

    sendRemoteLockUserChange(endpointId, LockDataTypeEnum::kUserIndex, DataOperationTypeEnum::kAdd, sourceNodeId, creatorFabricIdx,
                             userIndex, userIndex);

    return ClusterStatusCode(Status::Success);
}

Status DoorLockServer::modifyUser(chip::EndpointId endpointId, chip::FabricIndex modifierFabricIndex, chip::NodeId sourceNodeId,
                                  uint16_t userIndex, const Nullable<chip::CharSpan> & userName,
                                  const Nullable<uint32_t> & userUniqueId, const Nullable<UserStatusEnum> & userStatus,
                                  const Nullable<UserTypeEnum> & userType, const Nullable<CredentialRuleEnum> & credentialRule)
{
    // We should get the user by that index first
    EmberAfPluginDoorLockUserInfo user;
    if (!emberAfPluginDoorLockGetUser(endpointId, userIndex, user))
    {
        ChipLogError(Zcl, "[modifyUser] Unable to get the user from app [endpointId=%d,userIndex=%d]", endpointId, userIndex);
        return Status::Failure;
    }

    // appclusters, 5.2.4.34: to modify user its status should NOT be set to Available. If it is we should return INVALID_COMMAND.
    if (UserStatusEnum::kAvailable == user.userStatus)
    {
        ChipLogProgress(Zcl, "[modifyUser] Unable to modify non-existing user [endpointId=%d,userIndex=%d]", endpointId, userIndex);
        return Status::InvalidCommand;
    }

    // appclusters, 5.2.4.34: UserName SHALL be null if modifying a user record that was not created by the accessing fabric
    if (user.createdBy != modifierFabricIndex && !userName.IsNull())
    {
        ChipLogProgress(Zcl,
                        "[modifyUser] Unable to modify name of user created by different fabric "
                        "[endpointId=%d,userIndex=%d,creatorIdx=%d,modifierIdx=%d]",
                        endpointId, userIndex, user.createdBy, modifierFabricIndex);
        return Status::InvalidCommand;
    }

    // appclusters, 5.2.4.34: UserUniqueID SHALL be null if modifying the user record that was not created by the accessing fabric.
    if (user.createdBy != modifierFabricIndex && !userUniqueId.IsNull())
    {
        ChipLogProgress(Zcl,
                        "[modifyUser] Unable to modify UUID of user created by different fabric "
                        "[endpointId=%d,userIndex=%d,creatorIdx=%d,modifierIdx=%d]",
                        endpointId, userIndex, user.createdBy, modifierFabricIndex);
        return Status::InvalidCommand;
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
        return Status::Failure;
    }

    ChipLogProgress(Zcl,
                    "[modifyUser] User modified "
                    "[endpointId=%d,modifierFabric=%d,userIndex=%d,userName=\"%.*s\",userUniqueId=0x%" PRIx32
                    ",userStatus=%u,userType=%u,credentialRule=%u]",
                    endpointId, modifierFabricIndex, userIndex, static_cast<int>(newUserName.size()), newUserName.data(),
                    newUserUniqueId, to_underlying(newUserStatus), to_underlying(newUserType), to_underlying(newCredentialRule));

    sendRemoteLockUserChange(endpointId, LockDataTypeEnum::kUserIndex, DataOperationTypeEnum::kModify, sourceNodeId,
                             modifierFabricIndex, userIndex, userIndex);

    return Status::Success;
}

Status DoorLockServer::clearUser(chip::EndpointId endpointId, chip::FabricIndex modifierFabricId, chip::NodeId sourceNodeId,
                                 uint16_t userIndex, bool sendUserChangeEvent)
{
    EmberAfPluginDoorLockUserInfo user;
    if (!emberAfPluginDoorLockGetUser(endpointId, userIndex, user))
    {
        return Status::Failure;
    }

    return clearUser(endpointId, modifierFabricId, sourceNodeId, userIndex, user, sendUserChangeEvent);
}

Status DoorLockServer::clearUser(chip::EndpointId endpointId, chip::FabricIndex modifierFabricId, chip::NodeId sourceNodeId,
                                 uint16_t userIndex, const EmberAfPluginDoorLockUserInfo & user, bool sendUserChangeEvent)
{
    // appclusters, 5.2.4.37: all the credentials associated with user should be cleared when clearing the user
    for (const auto & credential : user.credentials)
    {
        ChipLogProgress(
            Zcl, "[ClearUser] Clearing associated credential [endpointId=%d,userIndex=%d,credentialType=%u,credentialIndex=%d]",
            endpointId, userIndex, to_underlying(credential.credentialType), credential.credentialIndex);

        if (!emberAfPluginDoorLockSetCredential(endpointId, credential.credentialIndex, kUndefinedFabricIndex,
                                                kUndefinedFabricIndex, DlCredentialStatus::kAvailable, credential.credentialType,
                                                chip::ByteSpan()))
        {
            ChipLogError(Zcl,
                         "[ClearUser] Unable to remove credentials associated with user - internal error "
                         "[endpointId=%d,userIndex=%d,credentialIndex=%d,credentialType=%u]",
                         endpointId, userIndex, credential.credentialIndex, to_underlying(credential.credentialType));
            return Status::Failure;
        }
    }

    // Clear all the user schedules
    auto status = clearSchedules(endpointId, userIndex);
    if (DlStatus::kSuccess != status)
    {
        ChipLogError(Zcl, "[ClearUser] Unable to delete schedules - internal error [endpointId=%d,userIndex=%d]", endpointId,
                     userIndex);
        // TODO: Figure out whether this should still clear the user even though
        // schedule clearing failed?
    }

    // Remove the user entry
    if (!emberAfPluginDoorLockSetUser(endpointId, userIndex, kUndefinedFabricIndex, kUndefinedFabricIndex, ""_span, 0,
                                      UserStatusEnum::kAvailable, UserTypeEnum::kUnrestrictedUser, CredentialRuleEnum::kSingle,
                                      nullptr, 0))
    {
        return Status::Failure;
    }

    if (sendUserChangeEvent)
    {
        sendRemoteLockUserChange(endpointId, LockDataTypeEnum::kUserIndex, DataOperationTypeEnum::kClear, sourceNodeId,
                                 modifierFabricId, userIndex, userIndex);
    }
    return Status::Success;
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
        if (UserStatusEnum::kAvailable == user.userStatus || (fabricIndex != user.createdBy && fabricIndex != user.lastModifiedBy))
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
                                                    chip::NodeId sourceNodeId, const Nullable<UserStatusEnum> & userStatus,
                                                    const Nullable<UserTypeEnum> & userType, const CredentialStruct & credential,
                                                    const chip::ByteSpan & credentialData, uint16_t & createdUserIndex)
{
    uint16_t availableUserIndex = 0;
    if (!findUnoccupiedUserSlot(endpointId, availableUserIndex))
    {
        ChipLogProgress(Zcl,
                        "[SetCredential] Unable to create new user for credential: no available user slots "
                        "[endpointId=%d,credentialIndex=%d]",
                        endpointId, credential.credentialIndex);
        return DlStatus::kOccupied;
    }

    ClusterStatusCode status =
        createUser(endpointId, creatorFabricIdx, sourceNodeId, availableUserIndex, Nullable<CharSpan>(), Nullable<uint32_t>(),
                   userStatus, userType, Nullable<CredentialRuleEnum>(), Nullable<CredentialStruct>(credential));
    if (!status.IsSuccess())
    {
        ChipLogProgress(Zcl,
                        "[SetCredential] Unable to create new user for credential: internal error "
                        "[endpointId=%d,credentialIndex=%d,userIndex=%d,status=%d]",
                        endpointId, credential.credentialIndex, availableUserIndex,
                        status.HasClusterSpecificCode() ? status.GetClusterSpecificCode().Value()
                                                        : (to_underlying(status.GetStatus())));
        return DlStatus::kFailure;
    }

    if (!emberAfPluginDoorLockSetCredential(endpointId, credential.credentialIndex, creatorFabricIdx, creatorFabricIdx,
                                            DlCredentialStatus::kOccupied, credential.credentialType, credentialData))
    {
        ChipLogProgress(Zcl,
                        "[SetCredential] Unable to set the credential: app error "
                        "[endpointId=%d,credentialIndex=%d,credentialType=%u,dataLength=%u]",
                        endpointId, credential.credentialIndex, to_underlying(credential.credentialType),
                        static_cast<unsigned int>(credentialData.size()));
        return DlStatus::kFailure;
    }

    ChipLogProgress(Zcl,
                    "[SetCredential] Credential and user were created "
                    "[endpointId=%d,credentialIndex=%d,credentialType=%u,dataLength=%u,userIndex=%d]",
                    endpointId, credential.credentialIndex, to_underlying(credential.credentialType),
                    static_cast<unsigned int>(credentialData.size()), availableUserIndex);
    createdUserIndex = availableUserIndex;

    return DlStatus::kSuccess;
}

DlStatus DoorLockServer::createNewCredentialAndAddItToUser(chip::EndpointId endpointId, chip::FabricIndex modifierFabricIdx,
                                                           uint16_t userIndex, const CredentialStruct & credential,
                                                           const chip::ByteSpan & credentialData)
{
    if (!userIndexValid(endpointId, userIndex))
    {
        ChipLogProgress(Zcl,
                        "[SetCredential] Unable to add new credential to user: user out of bounds "
                        "[endpointId=%d,credentialIndex=%d,userIndex=%d]",
                        endpointId, credential.credentialIndex, userIndex);
        return DlStatus::kInvalidField;
    }

    EmberAfPluginDoorLockUserInfo user;
    if (!emberAfPluginDoorLockGetUser(endpointId, userIndex, user))
    {
        ChipLogProgress(
            Zcl, "[SetCredential] Unable to check if credential exists: app error [endpointId=%d,credentialIndex=%d,userIndex=%d]",
            endpointId, credential.credentialIndex, userIndex);

        return DlStatus::kFailure;
    }

    // Not in the spec, but common sense: I don't think we need to modify the credential if user slot is not occupied
    if (user.userStatus == UserStatusEnum::kAvailable)
    {
        ChipLogProgress(Zcl,
                        "[SetCredential] Unable to add credential to user: user slot is empty "
                        "[endpointId=%d,credentialIndex=%d,userIndex=%d]",
                        endpointId, credential.credentialIndex, userIndex);
        return DlStatus::kInvalidField;
    }

    // Add new credential to the user
    auto status = addCredentialToUser(endpointId, modifierFabricIdx, userIndex, credential);
    if (DlStatus::kSuccess != status)
    {
        ChipLogProgress(Zcl,
                        "[SetCredential] Unable to add credential to a user: internal error "
                        "[endpointId=%d,credentialIndex=%d,userIndex=%d,status=%u]",
                        endpointId, credential.credentialIndex, userIndex, to_underlying(status));
        return status;
    }

    if (!emberAfPluginDoorLockSetCredential(endpointId, credential.credentialIndex, modifierFabricIdx, modifierFabricIdx,
                                            DlCredentialStatus::kOccupied, credential.credentialType, credentialData))
    {
        ChipLogProgress(Zcl,
                        "[SetCredential] Unable to set the credential: app error "
                        "[endpointId=%d,credentialIndex=%d,credentialType=%u,dataLength=%u]",
                        endpointId, credential.credentialIndex, to_underlying(credential.credentialType),
                        static_cast<unsigned int>(credentialData.size()));
        return DlStatus::kFailure;
    }

    return DlStatus::kSuccess;
}

DlStatus DoorLockServer::addCredentialToUser(chip::EndpointId endpointId, chip::FabricIndex modifierFabricIdx, uint16_t userIndex,
                                             const CredentialStruct & credential)
{
    // We should get the user by that index first
    EmberAfPluginDoorLockUserInfo user;
    if (!emberAfPluginDoorLockGetUser(endpointId, userIndex, user))
    {
        ChipLogProgress(Zcl, "[AddCredentialToUser] Unable to get the user from app [endpointId=%d,userIndex=%d]", endpointId,
                        userIndex);
        return DlStatus::kFailure;
    }

    for (size_t i = 0; i < user.credentials.size(); ++i)
    {
        // appclusters, 5.2.4.40: CredentialIndex in CredentialStruct provided SHALL be for an available credential slot.
        // appclusters, 5.6.3.2: This is the index of the specific credential used to authorize
        // the lock operation in the list of credentials identified by CredentialType
        if (user.credentials.data()[i].credentialIndex == credential.credentialIndex &&
            user.credentials.data()[i].credentialType == credential.credentialType)
        {
            ChipLogProgress(
                Zcl,
                "[AddCredentialToUser] Unable to add credential to user: credential with this index is already associated "
                "with user [endpointId=%d,userIndex=%d,credentialType=%d,credentialIndex=%d]",
                endpointId, userIndex, to_underlying(credential.credentialType), credential.credentialIndex);
            return DlStatus::kInvalidField;
        }
    }

    uint8_t maxCredentialsPerUser;
    if (!GetNumberOfCredentialsSupportedPerUser(endpointId, maxCredentialsPerUser))
    {
        ChipLogError(Zcl,
                     "[AddCredentialToUser] Unable to get the number of available credentials per user: internal error "
                     "[endpointId=%d,userIndex=%d,credentialType=%d,credentialIndex=%d]",
                     endpointId, userIndex, to_underlying(credential.credentialType), credential.credentialIndex);
        return DlStatus::kFailure;
    }

    // appclusters: spec defines up to NumberOfCredentialsSupportedPerUser credentials per user
    if (user.credentials.size() + 1 > maxCredentialsPerUser)
    {
        ChipLogProgress(Zcl,
                        "[AddCredentialToUser] Unable to add credentials to user: too many credentials "
                        "[endpointId=%d,userIndex=%d,userTotalCredentials=%u]",
                        endpointId, userIndex, static_cast<unsigned int>(user.credentials.size()));
        return DlStatus::kResourceExhausted;
    }

    chip::Platform::ScopedMemoryBuffer<CredentialStruct> newCredentials;
    if (!newCredentials.Alloc(user.credentials.size() + 1))
    {
        ChipLogError(Zcl,
                     "[AddCredentialToUser] Unable to allocate the buffer for credentials "
                     "[endpointId=%d,userIndex=%d,userTotalCredentials=%u]",
                     endpointId, userIndex, static_cast<unsigned int>(user.credentials.size()));
        return DlStatus::kFailure;
    }

    memcpy(newCredentials.Get(), user.credentials.data(), sizeof(CredentialStruct) * user.credentials.size());
    newCredentials[user.credentials.size()] = credential;

    if (!emberAfPluginDoorLockSetUser(endpointId, userIndex, user.createdBy, modifierFabricIdx, user.userName, user.userUniqueId,
                                      user.userStatus, user.userType, user.credentialRule, newCredentials.Get(),
                                      user.credentials.size() + 1))
    {
        ChipLogProgress(Zcl,
                        "[AddCredentialToUser] Unable to add credential to user: credential with this index is already associated "
                        "with user [endpointId=%d,userIndex=%d,credentialType=%d,credentialIndex=%d,userTotalCredentials=%u]",
                        endpointId, userIndex, to_underlying(credential.credentialType), credential.credentialIndex,
                        static_cast<unsigned int>(user.credentials.size()));
        return DlStatus::kFailure;
    }

    ChipLogProgress(Zcl,
                    "[AddCredentialToUser] Credential added to user "
                    "[endpointId=%d,userIndex=%d,credentialType=%d,credentialIndex=%d,userTotalCredentials=%u]",
                    endpointId, userIndex, to_underlying(credential.credentialType), credential.credentialIndex,
                    static_cast<unsigned int>(user.credentials.size() + 1));

    return DlStatus::kSuccess;
}

DlStatus DoorLockServer::modifyCredentialForUser(chip::EndpointId endpointId, chip::FabricIndex modifierFabricIdx,
                                                 uint16_t userIndex, const CredentialStruct & credential)
{
    // We should get the user by that index first
    EmberAfPluginDoorLockUserInfo user;
    if (!emberAfPluginDoorLockGetUser(endpointId, userIndex, user))
    {
        ChipLogProgress(Zcl, "[ModifyUserCredential] Unable to get the user from app [endpointId=%d,userIndex=%d]", endpointId,
                        userIndex);
        return DlStatus::kFailure;
    }

    for (size_t i = 0; i < user.credentials.size(); ++i)
    {
        // appclusters, 5.2.4.40: user should already be associated with given credentialIndex
        if (user.credentials.data()[i].credentialIndex == credential.credentialIndex)
        {
            chip::Platform::ScopedMemoryBuffer<CredentialStruct> newCredentials;
            if (!newCredentials.Alloc(user.credentials.size()))
            {
                ChipLogError(Zcl,
                             "[ModifyUserCredential] Unable to allocate the buffer for credentials "
                             "[endpointId=%d,userIndex=%d,userTotalCredentials=%u,credentialType=%d,credentialIndex=%d]",
                             endpointId, userIndex, static_cast<unsigned int>(user.credentials.size()),
                             to_underlying(credential.credentialType), credential.credentialIndex);
                return DlStatus::kFailure;
            }
            memcpy(newCredentials.Get(), user.credentials.data(), sizeof(CredentialStruct) * user.credentials.size());
            newCredentials[i] = credential;

            ChipLogProgress(
                Zcl,
                "[ModifyUserCredential] Unable to add credential to user: credential with this index is already associated "
                "[endpointId=%d,userIndex=%d,credentialType=%d,credentialIndex=%d]",
                endpointId, userIndex, to_underlying(credential.credentialType), credential.credentialIndex);

            if (!emberAfPluginDoorLockSetUser(endpointId, userIndex, user.createdBy, modifierFabricIdx, user.userName,
                                              user.userUniqueId, user.userStatus, user.userType, user.credentialRule,
                                              newCredentials.Get(), user.credentials.size()))
            {
                ChipLogProgress(
                    Zcl,
                    "[ModifyUserCredential] Unable to modify user credential: credential with this index is already associated "
                    "with user [endpointId=%d,userIndex=%d,credentialType=%d,credentialIndex=%d,userTotalCredentials=%u]",
                    endpointId, userIndex, to_underlying(credential.credentialType), credential.credentialIndex,
                    static_cast<unsigned int>(user.credentials.size()));
                return DlStatus::kFailure;
            }

            ChipLogProgress(Zcl,
                            "[ModifyUserCredential] User credential modified "
                            "[endpointId=%d,userIndex=%d,credentialType=%d,credentialIndex=%d,userTotalCredentials=%u]",
                            endpointId, userIndex, to_underlying(credential.credentialType), credential.credentialIndex,
                            static_cast<unsigned int>(user.credentials.size()));

            return DlStatus::kSuccess;
        }
    }

    // appclusters, 5.2.4.40: if user is not associated with credential index we should return INVALID_COMMAND
    ChipLogProgress(Zcl,
                    "[ModifyUserCredential] Unable to modify user credential: user is not associated with credential index "
                    "[endpointId=%d,userIndex=%d,credentialIndex=%d]",
                    endpointId, userIndex, credential.credentialIndex);

    return DlStatus::kInvalidField;
}

DlStatus DoorLockServer::createCredential(chip::EndpointId endpointId, chip::FabricIndex creatorFabricIdx,
                                          chip::NodeId sourceNodeId, uint16_t credentialIndex, CredentialTypeEnum credentialType,
                                          const EmberAfPluginDoorLockCredentialInfo & existingCredential,
                                          const chip::ByteSpan & credentialData, Nullable<uint16_t> userIndex,
                                          const Nullable<UserStatusEnum> & userStatus, Nullable<UserTypeEnum> userType,
                                          uint16_t & createdUserIndex)
{
    // appclusters, 5.2.4.41.1: should send the OCCUPIED in the response when the credential is in use
    if (DlCredentialStatus::kAvailable != existingCredential.status)
    {
        ChipLogProgress(
            Zcl, "[SetCredential] Unable to set the credential: credential slot is occupied [endpointId=%d,credentialIndex=%d]",
            endpointId, credentialIndex);

        return DlStatus::kOccupied;
    }

    if (!userType.IsNull() && UserTypeEnum::kProgrammingUser == userType.Value())
    {
        ChipLogProgress(Zcl,
                        "[SetCredential] Unable to set the credential: user type is invalid "
                        "[endpointId=%d,credentialIndex=%d,userType=%u]",
                        endpointId, credentialIndex, to_underlying(userType.Value()));

        return DlStatus::kInvalidField;
    }

    CredentialStruct credential{ credentialType, credentialIndex };
    // appclusters, 5.2.4.40: if userIndex is not provided we should create new user
    DlStatus status = DlStatus::kSuccess;
    if (userIndex.IsNull())
    {
        ChipLogProgress(Zcl, "[SetCredential] UserIndex is not set, creating new user [endpointId=%d,credentialIndex=%d]",
                        endpointId, credentialIndex);

        status = createNewCredentialAndUser(endpointId, creatorFabricIdx, sourceNodeId, userStatus, userType, credential,
                                            credentialData, createdUserIndex);
    }
    else
    {
        // appclusters, 5.2.4.40: if user index is NULL, we should try to modify
        // the existing user.  In this case userStatus and userType shall both
        // be null.
        if (!userStatus.IsNull() || !userType.IsNull())
        {
            ChipLogProgress(Zcl,
                            "[SetCredential] Unable to add credential: invalid arguments "
                            "[endpointId=%d,credentialIndex=%d,credentialType=%u]",
                            endpointId, credentialIndex, to_underlying(credentialType));
            return DlStatus::kInvalidField;
        }
        status = createNewCredentialAndAddItToUser(endpointId, creatorFabricIdx, userIndex.Value(), credential, credentialData);
    }

    if (DlStatus::kSuccess == status)
    {
        sendRemoteLockUserChange(endpointId, credentialTypeToLockDataType(credentialType), DataOperationTypeEnum::kAdd,
                                 sourceNodeId, creatorFabricIdx, createdUserIndex == 0 ? userIndex.Value() : createdUserIndex,
                                 credentialIndex);
    }

    return status;
}

DlStatus DoorLockServer::modifyProgrammingPIN(chip::EndpointId endpointId, chip::FabricIndex modifierFabricIndex,
                                              chip::NodeId sourceNodeId, uint16_t credentialIndex,
                                              CredentialTypeEnum credentialType,
                                              const EmberAfPluginDoorLockCredentialInfo & existingCredential,
                                              const chip::ByteSpan & credentialData)
{
    if (CredentialTypeEnum::kProgrammingPIN != credentialType || 0 != credentialIndex)
    {
        ChipLogProgress(Zcl,
                        "[SetCredential] Unable to modify programming PIN: invalid argument [endpointId=%d,credentialIndex=%d]",
                        endpointId, credentialIndex);

        return DlStatus::kInvalidField;
    }

    ChipLogProgress(Zcl, "[SetCredential] Modifying the programming PIN [endpointId=%d,credentialIndex=%d]", endpointId,
                    credentialIndex);

    uint16_t relatedUserIndex = 0;
    if (!findUserIndexByCredential(endpointId, CredentialTypeEnum::kProgrammingPIN, 0, relatedUserIndex))
    {
        ChipLogError(Zcl, "[SetCredential] Unable to modify PIN - related user not found (internal error) [endpointId=%d]",
                     endpointId);
        return DlStatus::kFailure;
    }

    if (!emberAfPluginDoorLockSetCredential(endpointId, credentialIndex, existingCredential.createdBy, modifierFabricIndex,
                                            existingCredential.status, existingCredential.credentialType, credentialData))
    {
        ChipLogProgress(Zcl,
                        "[SetCredential] Unable to modify the credential: app error "
                        "[endpointId=%d,credentialIndex=%d,credentialType=%u,credentialDataSize=%u]",
                        endpointId, credentialIndex, to_underlying(credentialType),
                        static_cast<unsigned int>(credentialData.size()));
        return DlStatus::kFailure;
    }

    ChipLogProgress(Zcl,
                    "[SetCredential] Successfully         modified the credential "
                    "[endpointId=%d,credentialIndex=%d,credentialType=%u,credentialDataSize=%u]",
                    endpointId, credentialIndex, to_underlying(credentialType), static_cast<unsigned int>(credentialData.size()));

    sendRemoteLockUserChange(endpointId, credentialTypeToLockDataType(credentialType), DataOperationTypeEnum::kModify, sourceNodeId,
                             modifierFabricIndex, relatedUserIndex, credentialIndex);

    return DlStatus::kSuccess;
}

DlStatus DoorLockServer::modifyCredential(chip::EndpointId endpointId, chip::FabricIndex modifierFabricIndex,
                                          chip::NodeId sourceNodeId, uint16_t credentialIndex, CredentialTypeEnum credentialType,
                                          const EmberAfPluginDoorLockCredentialInfo & existingCredential,
                                          const chip::ByteSpan & credentialData, uint16_t userIndex,
                                          const Nullable<UserStatusEnum> & userStatus, Nullable<UserTypeEnum> userType)
{

    // appclusters, 5.2.4.40: when modifying a credential, userStatus and userType shall both be NULL.
    if (!userStatus.IsNull() || !userType.IsNull())
    {
        ChipLogProgress(Zcl,
                        "[SetCredential] Unable to modify the credential: invalid arguments "
                        "[endpointId=%d,credentialIndex=%d,credentialType=%u]",
                        endpointId, credentialIndex, to_underlying(credentialType));
        return DlStatus::kInvalidField;
    }

    CredentialStruct credential{ credentialType, credentialIndex };
    auto status = modifyCredentialForUser(endpointId, modifierFabricIndex, userIndex, credential);

    if (DlStatus::kSuccess == status)
    {
        if (!emberAfPluginDoorLockSetCredential(endpointId, credentialIndex, existingCredential.createdBy, modifierFabricIndex,
                                                existingCredential.status, existingCredential.credentialType, credentialData))
        {
            ChipLogProgress(Zcl,
                            "[SetCredential] Unable to modify the credential: app error "
                            "[endpointId=%d,credentialIndex=%d,credentialType=%u,credentialDataSize=%u]",
                            endpointId, credentialIndex, to_underlying(credentialType),
                            static_cast<unsigned int>(credentialData.size()));

            return DlStatus::kFailure;
        }

        ChipLogProgress(Zcl,
                        "[SetCredential] Successfully modified the credential "
                        "[endpointId=%d,credentialIndex=%d,credentialType=%u,credentialDataSize=%u]",
                        endpointId, credentialIndex, to_underlying(credentialType),
                        static_cast<unsigned int>(credentialData.size()));

        sendRemoteLockUserChange(endpointId, credentialTypeToLockDataType(credentialType), DataOperationTypeEnum::kModify,
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

bool DoorLockServer::credentialTypeSupported(chip::EndpointId endpointId, CredentialTypeEnum type)
{
    switch (type)
    {
    case CredentialTypeEnum::kProgrammingPIN:
    case CredentialTypeEnum::kPin:
        return SupportsPIN(endpointId);
    case CredentialTypeEnum::kRfid:
        return SupportsRFID(endpointId);
    case CredentialTypeEnum::kFingerprint:
    case CredentialTypeEnum::kFingerVein:
        return SupportsFingers(endpointId);
    case CredentialTypeEnum::kFace:
        return SupportsFace(endpointId);
    case CredentialTypeEnum::kAliroEvictableEndpointKey:
    case CredentialTypeEnum::kAliroCredentialIssuerKey:
    case CredentialTypeEnum::kAliroNonEvictableEndpointKey:
        return SupportsAliroProvisioning(endpointId);
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
                                                   DaysMaskMap(0), 0, 0, 0, 0);
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
                                                    uint16_t userIndex, DlStatus status, DaysMaskMap daysMask, uint8_t startHour,
                                                    uint8_t startMinute, uint8_t endHour, uint8_t endMinute)
{
    VerifyOrDie(nullptr != commandObj);

    Commands::GetWeekDayScheduleResponse::Type response;
    response.weekDayIndex = weekdayIndex;
    response.userIndex    = userIndex;
    response.status       = status;
    if (DlStatus::kSuccess == status)
    {
        response.daysMask    = Optional<chip::BitMask<DaysMaskMap>>(daysMask);
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
        emberAfPluginDoorLockSetSchedule(endpointId, holidayIndex, DlScheduleStatus::kAvailable, 0, 0, OperatingModeEnum::kNormal);
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
                                                 OperatingModeEnum operatingMode)
{
    VerifyOrDie(nullptr != commandObj);

    auto response = Commands::GetHolidayScheduleResponse::Type{ holidayIndex, status };
    if (DlStatus::kSuccess == status)
    {
        response.localStartTime = Optional<uint32_t>(localStartTime);
        response.localEndTime   = Optional<uint32_t>(localEndTime);
        response.operatingMode  = Optional<OperatingModeEnum>(operatingMode);
    }
    commandObj->AddResponse(commandPath, response);
}

Status DoorLockServer::clearCredential(chip::EndpointId endpointId, chip::FabricIndex modifier, chip::NodeId sourceNodeId,
                                       CredentialTypeEnum credentialType, uint16_t credentialIndex, bool sendUserChangeEvent)
{
    if (CredentialTypeEnum::kProgrammingPIN == credentialType)
    {
        ChipLogProgress(Zcl,
                        "[clearCredential] Cannot clear programming PIN credentials "
                        "[endpointId=%d,credentialType=%u,credentialIndex=%d,modifier=%d]",
                        endpointId, to_underlying(credentialType), credentialIndex, modifier);
        return Status::InvalidCommand;
    }

    if (!credentialIndexValid(endpointId, credentialType, credentialIndex))
    {
        ChipLogProgress(Zcl,
                        "[clearCredential] Cannot clear credential - index out of bounds "
                        "[endpointId=%d,credentialType=%u,credentialIndex=%d,modifier=%d]",
                        endpointId, to_underlying(credentialType), credentialIndex, modifier);
        return Status::InvalidCommand;
    }

    // 1. Fetch the credential from storage, so we know what we're deleting
    EmberAfPluginDoorLockCredentialInfo credential;
    if (!emberAfPluginDoorLockGetCredential(endpointId, credentialIndex, credentialType, credential))
    {
        ChipLogError(Zcl,
                     "[clearCredential] Unable to clear credential - couldn't read credential from database "
                     "[endpointId=%d,credentialType=%u,credentialIndex=%d,modifier=%d]",
                     endpointId, to_underlying(credentialType), credentialIndex, modifier);
        return Status::Failure;
    }

    if (DlCredentialStatus::kAvailable == credential.status)
    {
        ChipLogProgress(Zcl,
                        "[clearCredential] Ignored attempt to clear unoccupied credential slot "
                        "[endpointId=%d,credentialType=%u,credentialIndex=%d,modifier=%d]",
                        endpointId, to_underlying(credentialType), credentialIndex, modifier);
        return Status::Success;
    }

    if (credentialType != credential.credentialType)
    {
        ChipLogProgress(Zcl,
                        "[clearCredential] Ignored attempt to clear credential of different type "
                        "[endpointId=%d,credentialType=%u,credentialIndex=%d,modifier=%d,actualCredentialType=%u]",
                        endpointId, to_underlying(credentialType), credentialIndex, modifier,
                        to_underlying(credential.credentialType));
        return Status::Success;
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
        return Status::Failure;
    }
    EmberAfPluginDoorLockUserInfo relatedUser;
    if (!emberAfPluginDoorLockGetUser(endpointId, relatedUserIndex, relatedUser))
    {
        ChipLogError(Zcl,
                     "[clearCredential] Unable to clear credential for related user - couldn't get user from database "
                     "[endpointId=%d,credentialType=%u,credentialIndex=%d,modifier=%d,userIndex=%d]",
                     endpointId, to_underlying(credentialType), credentialIndex, modifier, relatedUserIndex);

        return Status::Failure;
    }
    if (1 == relatedUser.credentials.size())
    {
        ChipLogProgress(Zcl,
                        "[clearCredential] Clearing related user - no credentials left "
                        "[endpointId=%d,credentialType=%u,credentialIndex=%d,modifier=%d,userIndex=%d]",
                        endpointId, to_underlying(credentialType), credentialIndex, modifier, relatedUserIndex);
        auto clearStatus = clearUser(endpointId, modifier, sourceNodeId, relatedUserIndex, relatedUser, true);
        if (Status::Success != clearStatus)
        {
            ChipLogError(Zcl,
                         "[clearCredential] Unable to clear related credential user - internal error "
                         "[endpointId=%d,credentialType=%u,credentialIndex=%d,modifier=%d,userIndex=%d,status=%d]",
                         endpointId, to_underlying(credentialType), credentialIndex, modifier, relatedUserIndex,
                         to_underlying(clearStatus));

            return Status::Failure;
        }
        ChipLogProgress(Zcl,
                        "[clearCredential] Successfully clear credential and related user "
                        "[endpointId=%d,credentialType=%u,credentialIndex=%d,modifier=%d,userIndex=%d]",
                        endpointId, to_underlying(credentialType), credentialIndex, modifier, relatedUserIndex);
        return Status::Success;
    }

    // 3. If the user wasn't deleted, delete the credential and adjust the list of credentials for related user in the storage
    if (!emberAfPluginDoorLockSetCredential(endpointId, credentialIndex, kUndefinedFabricIndex, kUndefinedFabricIndex,
                                            DlCredentialStatus::kAvailable, credentialType, chip::ByteSpan()))
    {
        ChipLogError(Zcl,
                     "[clearCredential] Unable to clear credential - couldn't write new credential to database "
                     "[endpointId=%d,credentialType=%u,credentialIndex=%d,modifier=%d]",
                     endpointId, to_underlying(credentialType), credentialIndex, modifier);
        return Status::Failure;
    }

    uint8_t maxCredentialsPerUser;
    if (!GetNumberOfCredentialsSupportedPerUser(endpointId, maxCredentialsPerUser))
    {
        ChipLogError(Zcl,
                     "[clearCredential] Unable to get the number of available credentials per user: internal error "
                     "[endpointId=%d,credentialType=%d,credentialIndex=%d]",
                     endpointId, to_underlying(credentialType), credentialIndex);
        return Status::Failure;
    }

    // Should never happen, only possible if the implementation of application is incorrect
    if (relatedUser.credentials.size() > maxCredentialsPerUser)
    {
        ChipLogError(Zcl,
                     "[clearCredential] Unable to clear credential for related user - user has too many credentials associated"
                     "[endpointId=%d,credentialType=%u,credentialIndex=%d,modifier=%d,userIndex=%d,credentialsCount=%u]",
                     endpointId, to_underlying(credentialType), credentialIndex, modifier, relatedUserIndex,
                     static_cast<unsigned int>(relatedUser.credentials.size()));

        return Status::Failure;
    }

    chip::Platform::ScopedMemoryBuffer<CredentialStruct> newCredentials;
    if (!newCredentials.Alloc(relatedUser.credentials.size()))
    {
        ChipLogError(Zcl,
                     "[clearCredential] Unable to allocate the buffer for credentials "
                     "[endpointId=%d,credentialType=%u,credentialIndex=%d,modifier=%d,userIndex=%d,credentialsCount=%u]",
                     endpointId, to_underlying(credentialType), credentialIndex, modifier, relatedUserIndex,
                     static_cast<unsigned int>(relatedUser.credentials.size()));
        return Status::Failure;
    }

    size_t newCredentialsCount = 0;
    for (const auto & c : relatedUser.credentials)
    {
        if (c.credentialType == credentialType && c.credentialIndex == credentialIndex)
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
        return Status::Failure;
    }

    ChipLogProgress(Zcl,
                    "[clearCredential] Successfully clear credential and related user "
                    "[endpointId=%d,credentialType=%u,credentialIndex=%d,modifier=%d,userIndex=%d,newCredentialsCount=%u]",
                    endpointId, to_underlying(credentialType), credentialIndex, modifier, relatedUserIndex,
                    static_cast<unsigned int>(newCredentialsCount));

    if (sendUserChangeEvent)
    {
        sendRemoteLockUserChange(endpointId, credentialTypeToLockDataType(credentialType), DataOperationTypeEnum::kClear,
                                 sourceNodeId, modifier, relatedUserIndex, credentialIndex);
    }

    return Status::Success;
}

Status DoorLockServer::clearCredentials(chip::EndpointId endpointId, chip::FabricIndex modifier, chip::NodeId sourceNodeId)
{
    if (SupportsPIN(endpointId))
    {
        auto status = clearCredentials(endpointId, modifier, sourceNodeId, CredentialTypeEnum::kPin);
        if (Status::Success != status)
        {
            ChipLogError(Zcl, "[clearCredentials] Unable to clear all PIN credentials [endpointId=%d,status=%d]", endpointId,
                         to_underlying(status));
            return status;
        }

        ChipLogProgress(Zcl, "[clearCredentials] All PIN credentials were cleared [endpointId=%d]", endpointId);
    }

    if (SupportsRFID(endpointId))
    {
        auto status = clearCredentials(endpointId, modifier, sourceNodeId, CredentialTypeEnum::kRfid);
        if (Status::Success != status)
        {
            ChipLogError(Zcl, "[clearCredentials] Unable to clear all RFID credentials [endpointId=%d,status=%d]", endpointId,
                         to_underlying(status));
            return status;
        }
        ChipLogProgress(Zcl, "[clearCredentials] All RFID credentials were cleared [endpointId=%d]", endpointId);
    }

    if (SupportsFingers(endpointId))
    {
        auto status = clearCredentials(endpointId, modifier, sourceNodeId, CredentialTypeEnum::kFingerprint);
        if (Status::Success != status)
        {
            ChipLogError(Zcl, "[clearCredentials] Unable to clear all Fingerprint credentials [endpointId=%d,status=%d]",
                         endpointId, to_underlying(status));
            return status;
        }

        status = clearCredentials(endpointId, modifier, sourceNodeId, CredentialTypeEnum::kFingerVein);
        if (Status::Success != status)
        {
            ChipLogError(Zcl, "[clearCredentials] Unable to clear all Finger Vein credentials [endpointId=%d,status=%d]",
                         endpointId, to_underlying(status));
            return status;
        }

        ChipLogProgress(Zcl, "[clearCredentials] All Finger credentials were cleared [endpointId=%d]", endpointId);
    }

    if (SupportsFace(endpointId))
    {
        auto status = clearCredentials(endpointId, modifier, sourceNodeId, CredentialTypeEnum::kFace);
        if (Status::Success != status)
        {
            ChipLogError(Zcl, "[clearCredentials] Unable to clear all face credentials [endpointId=%d,status=%d]", endpointId,
                         to_underlying(status));
            return status;
        }
        ChipLogProgress(Zcl, "[clearCredentials] All face credentials were cleared [endpointId=%d]", endpointId);
    }

    return Status::Success;
}

Status DoorLockServer::clearCredentials(chip::EndpointId endpointId, chip::FabricIndex modifier, chip::NodeId sourceNodeId,
                                        CredentialTypeEnum credentialType)
{
    uint16_t maxNumberOfCredentials = 0;
    if (!getMaxNumberOfCredentials(endpointId, credentialType, maxNumberOfCredentials))
    {
        ChipLogError(Zcl,
                     "[clearCredentials] Unable to get max number of credentials to clear - can't get max number of credentials "
                     "[endpointId=%d,credentialType=%u]",
                     endpointId, to_underlying(credentialType));
        return Status::Failure;
    }

    Status status          = Status::Success;
    bool clearedCredential = false;
    for (uint16_t i = 1; i < maxNumberOfCredentials; ++i)
    {
        Status clearStatus = clearCredential(endpointId, modifier, sourceNodeId, credentialType, i, false);
        if (Status::Success != clearStatus)
        {
            ChipLogError(Zcl,
                         "[clearCredentials] Unable to clear the credential - internal error "
                         "[endpointId=%d,credentialType=%u,credentialIndex=%d,status=%d]",
                         endpointId, to_underlying(credentialType), i, to_underlying(status));
            if (status == Status::Success)
            {
                status = clearStatus;
            }
        }
        else
        {
            clearedCredential = true;
        }
    }

    // Generate the event if we cleared any credentials, even if we then had errors
    // clearing other ones, so we don't have credentials silently disappearing.
    if (clearedCredential)
    {
        sendRemoteLockUserChange(endpointId, credentialTypeToLockDataType(credentialType), DataOperationTypeEnum::kClear,
                                 sourceNodeId, modifier, 0xFFFE, 0xFFFE);
    }

    return status;
}

bool DoorLockServer::clearFabricFromCredentials(chip::EndpointId endpointId, CredentialTypeEnum credentialType,
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
    if (CredentialTypeEnum::kProgrammingPIN == credentialType)
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
        clearFabricFromCredentials(endpointId, CredentialTypeEnum::kRfid, fabricToRemove);
    }

    if (SupportsPIN(endpointId))
    {
        clearFabricFromCredentials(endpointId, CredentialTypeEnum::kPin, fabricToRemove);
    }

    if (SupportsFingers(endpointId))
    {
        clearFabricFromCredentials(endpointId, CredentialTypeEnum::kFingerprint, fabricToRemove);
        clearFabricFromCredentials(endpointId, CredentialTypeEnum::kFingerVein, fabricToRemove);
    }

    if (SupportsFace(endpointId))
    {
        clearFabricFromCredentials(endpointId, CredentialTypeEnum::kFace, fabricToRemove);
    }

    clearFabricFromCredentials(endpointId, CredentialTypeEnum::kProgrammingPIN, fabricToRemove);

    return true;
}

bool DoorLockServer::sendRemoteLockUserChange(chip::EndpointId endpointId, LockDataTypeEnum dataType,
                                              DataOperationTypeEnum operation, chip::NodeId nodeId, chip::FabricIndex fabricIndex,
                                              uint16_t userIndex, uint16_t dataIndex)
{
    Events::LockUserChange::Type event;
    event.lockDataType      = dataType;
    event.dataOperationType = operation;
    event.operationSource   = OperationSourceEnum::kRemote;
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
    ChipLogProgress(Zcl,
                    "[RemoteLockUserChange] Sent lock user change event "
                    "[endpointId=%d,eventNumber=0x" ChipLogFormatX64 ",dataType=%u,operation=%u,nodeId=0x" ChipLogFormatX64
                    ",fabricIndex=%d]",
                    endpointId, ChipLogValueX64(eventNumber), to_underlying(dataType), to_underlying(operation),
                    ChipLogValueX64(nodeId), fabricIndex);
    return true;
}

LockDataTypeEnum DoorLockServer::credentialTypeToLockDataType(CredentialTypeEnum credentialType)
{
    switch (credentialType)
    {
    case CredentialTypeEnum::kProgrammingPIN:
        return LockDataTypeEnum::kProgrammingCode;
    case CredentialTypeEnum::kPin:
        return LockDataTypeEnum::kPin;
    case CredentialTypeEnum::kRfid:
        return LockDataTypeEnum::kRfid;
    case CredentialTypeEnum::kFingerprint:
        return LockDataTypeEnum::kFingerprint;
    case CredentialTypeEnum::kFingerVein:
        return LockDataTypeEnum::kFingerVein;
    case CredentialTypeEnum::kFace:
        return LockDataTypeEnum::kFace;
    case CredentialTypeEnum::kAliroCredentialIssuerKey:
        return LockDataTypeEnum::kAliroCredentialIssuerKey;
    case CredentialTypeEnum::kAliroEvictableEndpointKey:
        return LockDataTypeEnum::kAliroEvictableEndpointKey;
    case CredentialTypeEnum::kAliroNonEvictableEndpointKey:
        return LockDataTypeEnum::kAliroNonEvictableEndpointKey;
    case CredentialTypeEnum::kUnknownEnumValue:
        return LockDataTypeEnum::kUnspecified;
    }

    return LockDataTypeEnum::kUnspecified;
}

bool DoorLockServer::isUserScheduleRestricted(chip::EndpointId endpointId, const EmberAfPluginDoorLockUserInfo & user)
{
    return false;
}

void DoorLockServer::setHolidayScheduleCommandHandler(chip::app::CommandHandler * commandObj,
                                                      const chip::app::ConcreteCommandPath & commandPath, uint8_t holidayIndex,
                                                      uint32_t localStartTime, uint32_t localEndTime,
                                                      OperatingModeEnum operatingMode)
{
    VerifyOrDie(nullptr != commandObj);

    auto endpointId = commandPath.mEndpointId;
    if (!SupportsHolidaySchedules(endpointId))
    {
        ChipLogProgress(Zcl, "[SetHolidaySchedule] Ignore command (not supported) [endpointId=%d]", endpointId);
        commandObj->AddStatus(commandPath, Status::InvalidCommand);
        return;
    }

    ChipLogProgress(Zcl, "[SetHolidaySchedule] incoming command [endpointId=%d]", endpointId);

    if (!holidayIndexValid(endpointId, holidayIndex))
    {
        ChipLogProgress(Zcl, "[SetHolidaySchedule] Unable to add schedule - index out of range [endpointId=%d,scheduleIndex=%d]",
                        endpointId, holidayIndex);
        commandObj->AddStatus(commandPath, Status::InvalidCommand);
        return;
    }

    if (localEndTime <= localStartTime)
    {
        ChipLogProgress(Zcl,
                        "[SetHolidaySchedule] Unable to add schedule - schedule ends earlier than starts"
                        "[endpointId=%d,scheduleIndex=%d,localStarTime=%" PRIu32 ",localEndTime=%" PRIu32 "]",
                        endpointId, holidayIndex, localStartTime, localEndTime);
        commandObj->AddStatus(commandPath, Status::InvalidCommand);
        return;
    }

    if (operatingMode == OperatingModeEnum::kUnknownEnumValue)
    {
        ChipLogProgress(Zcl,
                        "[SetHolidaySchedule] Unable to add schedule - operating mode is unknown"
                        "[endpointId=%d,scheduleIndex=%d,localStarTime=%" PRIu32 ",localEndTime=%" PRIu32 ", operatingMode=%d]",
                        endpointId, holidayIndex, localStartTime, localEndTime, to_underlying(operatingMode));
        commandObj->AddStatus(commandPath, Status::InvalidCommand);
        return;
    }

    auto status = emberAfPluginDoorLockSetSchedule(endpointId, holidayIndex, DlScheduleStatus::kOccupied, localStartTime,
                                                   localEndTime, operatingMode);
    if (DlStatus::kSuccess != status)
    {
        ChipLogError(Zcl, "[SetHolidaySchedule] Unable to add schedule - internal error [endpointId=%d,scheduleIndex=%d,status=%u]",
                     endpointId, holidayIndex, to_underlying(status));
        commandObj->AddStatus(commandPath, Status::Failure);
        return;
    }

    ChipLogProgress(Zcl,
                    "[SetHolidaySchedule] Successfully created new schedule "
                    "[endpointId=%d,scheduleIndex=%d,localStartTime=%" PRIu32 ",endTime=%" PRIu32 ",operatingMode=%d]",
                    endpointId, holidayIndex, localStartTime, localEndTime, to_underlying(operatingMode));

    commandObj->AddStatus(commandPath, Status::Success);
}

void DoorLockServer::getHolidayScheduleCommandHandler(chip::app::CommandHandler * commandObj,
                                                      const ConcreteCommandPath & commandPath, uint8_t holidayIndex)
{
    auto endpointId = commandPath.mEndpointId;
    if (!SupportsHolidaySchedules(endpointId))
    {
        ChipLogProgress(Zcl, "[GetHolidaySchedule] Ignore command (not supported) [endpointId=%d,scheduleIndex=%d]", endpointId,
                        holidayIndex);
        commandObj->AddStatus(commandPath, Status::InvalidCommand);
        return;
    }
    ChipLogProgress(Zcl, "[GetHolidaySchedule] incoming command [endpointId=%d,scheduleIndex=%d]", endpointId, holidayIndex);

    if (!holidayIndexValid(endpointId, holidayIndex))
    {
        ChipLogProgress(Zcl, "[GetYearDaySchedule] Unable to get schedule - index out of range [endpointId=%d,scheduleIndex=%d]",
                        endpointId, holidayIndex);
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
        ChipLogProgress(Zcl, "[ClearHolidaySchedule] Ignore command (not supported) [endpointId=%d]", endpointId);
        commandObj->AddStatus(commandPath, Status::InvalidCommand);
        return;
    }
    ChipLogProgress(Zcl, "[ClearHolidaySchedule] incoming command [endpointId=%d,scheduleIndex=%d]", endpointId, holidayIndex);

    if (!holidayIndexValid(endpointId, holidayIndex) && 0xFE != holidayIndex)
    {
        ChipLogProgress(Zcl, "[ClearHolidaySchedule] Holiday index is out of range [endpointId=%d,scheduleIndex=%d]", endpointId,
                        holidayIndex);
        commandObj->AddStatus(commandPath, Status::InvalidCommand);
        return;
    }

    DlStatus clearStatus = DlStatus::kSuccess;
    if (0xFE == holidayIndex)
    {
        ChipLogProgress(Zcl,
                        "[ClearHolidaySchedule] Clearing all holiday schedules for a single user [endpointId=%d,scheduleIndex=%d]",
                        endpointId, holidayIndex);
        clearStatus = clearHolidaySchedules(endpointId);
    }
    else
    {
        ChipLogProgress(Zcl, "[ClearHolidaySchedule] Clearing a single schedule [endpointId=%d,scheduleIndex=%d]", endpointId,
                        holidayIndex);
        clearStatus = clearHolidaySchedule(endpointId, holidayIndex);
    }

    if (DlStatus::kSuccess != clearStatus)
    {
        ChipLogProgress(
            Zcl, "[ClearHolidaySchedule] Unable to clear the user schedules - app error [endpointId=%d,scheduleIndex=%d,status=%u]",
            endpointId, holidayIndex, to_underlying(clearStatus));
        commandObj->AddStatus(commandPath, Status::Failure);
        return;
    }
    commandObj->AddStatus(commandPath, Status::Success);
}

bool DoorLockServer::RemoteOperationEnabled(chip::EndpointId endpointId) const
{
    OperatingModeEnum mode;

    return GetAttribute(endpointId, Attributes::OperatingMode::Id, Attributes::OperatingMode::Get, mode) &&
        mode != OperatingModeEnum::kPrivacy && mode != OperatingModeEnum::kNoRemoteLockUnlock;
}

void DoorLockServer::sendClusterResponse(chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
                                         ClusterStatusCode status)
{
    VerifyOrDie(nullptr != commandObj);

    if (status.HasClusterSpecificCode())
    {
        VerifyOrDie(commandObj->AddClusterSpecificFailure(commandPath, status.GetClusterSpecificCode().Value()) == CHIP_NO_ERROR);
    }
    else
    {
        commandObj->AddStatus(commandPath, status.GetStatus());
    }
}

EmberAfDoorLockEndpointContext * DoorLockServer::getContext(chip::EndpointId endpointId)
{
    auto index = emberAfGetClusterServerEndpointIndex(endpointId, ::Id, MATTER_DM_DOOR_LOCK_CLUSTER_SERVER_ENDPOINT_COUNT);
    if (index < kDoorLockClusterServerMaxEndpointCount)
    {
        return &mEndpointCtx[index];
    }
    return nullptr;
}

bool DoorLockServer::HandleRemoteLockOperation(chip::app::CommandHandler * commandObj,
                                               const chip::app::ConcreteCommandPath & commandPath, LockOperationTypeEnum opType,
                                               RemoteLockOpHandler opHandler, const Optional<ByteSpan> & pinCode)
{
    VerifyOrDie(LockOperationTypeEnum::kLock == opType || LockOperationTypeEnum::kUnlock == opType ||
                LockOperationTypeEnum::kUnlatch == opType);
    VerifyOrDie(nullptr != opHandler);

    EndpointId endpoint       = commandPath.mEndpointId;
    OperationErrorEnum reason = OperationErrorEnum::kUnspecified;
    Nullable<uint16_t> pinUserIdx; // Will get set to non-null if we find a user for the PIN.
    Optional<uint16_t> pinCredIdx; // Will get set to a value if the PIN is one we know about.
    bool success   = false;
    bool sendEvent = true;

    auto currentTime = chip::System::SystemClock().GetMonotonicTimestamp();

    EmberAfDoorLockEndpointContext * endpointContext;

    VerifyOrExit(RemoteOperationEnabled(endpoint), reason = OperationErrorEnum::kUnspecified);

    // appclusters.pdf 5.3.4.1:
    // When the PINCode field is provided an invalid PIN will count towards the WrongCodeEntryLimit and the
    // UserCodeTemporaryDisableTime will be triggered if the WrongCodeEntryLimit is exceeded. The lock SHALL ignore any attempts
    // to lock/unlock the door until the UserCodeTemporaryDisableTime expires.
    endpointContext = getContext(endpoint);
    VerifyOrExit(nullptr != endpointContext, ChipLogError(Zcl, "Failed to get endpoint index for cluster [endpoint=%d]", endpoint));
    if (endpointContext->lockoutEndTimestamp >= currentTime)
    {
        ChipLogProgress(Zcl, "Rejecting remote lock operation -- lockout is in action [endpoint=%d,lockoutEnd=%u,currentTime=%u]",
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
                     ChipLogProgress(Zcl,
                                     "PIN code is supplied while USR/PIN features are disabled. Exiting [endpoint=%d, lock_op=%d]",
                                     endpoint, chip::to_underlying(opType)));

        // Look up the user index and credential index -- it should be used in the Lock Operation event
        EmberAfPluginDoorLockUserInfo user;
        uint16_t userIdx;
        uint16_t credIdx;
        if (findUserIndexByCredential(endpoint, CredentialTypeEnum::kPin, pinCode.Value(), userIdx, credIdx, user))
        {
            pinUserIdx.SetNonNull(userIdx);
            pinCredIdx.Emplace(credIdx);
        }
        else
        {
            ChipLogProgress(Zcl, "Rejecting lock operation: unknown PIN provided [endpoint=%d, lock_op=%d]", endpoint,
                            to_underlying(opType));
            reason = OperationErrorEnum::kInvalidCredential;
            goto exit;
        }

        // If the user status is OccupiedDisabled we should deny the access and send out the appropriate event
        VerifyOrExit(user.userStatus != UserStatusEnum::kOccupiedDisabled, {
            reason = OperationErrorEnum::kDisabledUserDenied;
            ChipLogProgress(Zcl,
                            "Unable to perform remote lock operation: user is disabled [endpoint=%d, lock_op=%d, userIndex=%d]",
                            endpoint, to_underlying(opType), userIdx);
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
                Status::UnsupportedAttribute == status || Status::Success == status,
                ChipLogError(Zcl, "Failed to read Require PIN For Remote Operation attribute, status=0x%x", to_underlying(status)));
        }
        // If the PIN is required but not provided we should exit
        VerifyOrExit(!requirePin, {
            reason = OperationErrorEnum::kInvalidCredential;
            ChipLogProgress(Zcl, "Checking credentials failed: PIN is not provided when it is required");
        });
    }

    // credentials check succeeded, try to lock/unlock door
    success = opHandler(endpoint, MakeNullable(getFabricIndex(commandObj)), MakeNullable(getNodeId(commandObj)), pinCode, reason);
    // The app should trigger the lock state change as it may take a while before the lock actually locks/unlocks
exit:
    if (!success && reason == OperationErrorEnum::kInvalidCredential)
    {
        HandleWrongCodeEntry(endpoint);
    }

    // Reset the wrong code retry attempts if a valid credential is presented during lock/unlock
    if (success && pinCode.HasValue())
    {
        ResetWrongCodeEntryAttempts(endpoint);
    }

    // Send command response
    commandObj->AddStatus(commandPath, success ? Status::Success : Status::Failure);

    // Most of the time we want to send the lock operation event but sometimes (when the lockout is active) we don't want it.
    if (!sendEvent)
    {
        return success;
    }

    // Send LockOperation/LockOperationError event.  The credential index in
    // foundCred will be filled in if we actually have a value to fill in.
    Nullable<List<const LockOpCredentials>> credentials{};
    LockOpCredentials foundCred[] = { { CredentialTypeEnum::kPin, UINT16_MAX } };

    // appclusters.pdf 5.3.5.3, 5.3.5.4:
    // The list of credentials used in performing the lock operation. This SHALL be null if no credentials were involved.
    if (pinCode.HasValue() && pinCredIdx.HasValue())
    {
        foundCred[0].credentialIndex = pinCredIdx.Value();
        credentials.SetNonNull(foundCred);
    }

    if (!success)
    {
        // Failed Unlatch requests SHALL generate only a LockOperationError event with LockOperationType set to Unlock
        if (LockOperationTypeEnum::kUnlatch == opType)
        {
            opType = LockOperationTypeEnum::kUnlock;
        }

        SendLockOperationEvent(endpoint, opType, OperationSourceEnum::kRemote, reason, pinUserIdx,
                               Nullable<chip::FabricIndex>(getFabricIndex(commandObj)),
                               Nullable<chip::NodeId>(getNodeId(commandObj)), credentials, success);
    }

    return success;
}

void DoorLockServer::SendLockOperationEvent(chip::EndpointId endpointId, LockOperationTypeEnum opType, OperationSourceEnum opSource,
                                            OperationErrorEnum opErr, const Nullable<uint16_t> & userId,
                                            const Nullable<chip::FabricIndex> & fabricIdx, const Nullable<chip::NodeId> & nodeId,
                                            const Nullable<List<const LockOpCredentials>> & credentials, bool opSuccess)
{

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
    uint32_t timeoutMs =
        (DOOR_LOCK_MAX_LOCK_TIMEOUT_SEC >= timeoutSec) ? timeoutSec * MILLISECOND_TICKS_PER_SECOND : DOOR_LOCK_MAX_LOCK_TIMEOUT_SEC;

    CHIP_ERROR err =
        DeviceLayer::SystemLayer().StartTimer(chip::System::Clock::Milliseconds32(timeoutMs), DoorLockOnAutoRelockCallback,
                                              reinterpret_cast<void *>(static_cast<uintptr_t>(endpointId)));

    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "Failed to schedule autorelock: timeout=%" PRIu32 ", status=%" CHIP_ERROR_FORMAT, timeoutSec,
                     err.Format());
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
                                  Status (*getFn)(chip::EndpointId endpointId, T * value), T & value) const
{
    Status status = getFn(endpointId, &value);
    bool success  = (Status::Success == status);

    if (!success)
    {
        ChipLogError(Zcl, "Failed to read DoorLock attribute: attribute=0x%" PRIx32 ", status=0x%x", attributeId,
                     to_underlying(status));
    }
    return success;
}

template <typename T>
bool DoorLockServer::SetAttribute(chip::EndpointId endpointId, chip::AttributeId attributeId,
                                  Status (*setFn)(chip::EndpointId endpointId, T value), T value)
{
    Status status = setFn(endpointId, value);
    bool success  = (Status::Success == status);

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
    ChipLogProgress(Zcl, "Received command: LockDoor");
    DoorLockServer::Instance().HandleRemoteLockOperation(commandObj, commandPath, LockOperationTypeEnum::kLock,
                                                         emberAfPluginDoorLockOnDoorLockCommand, commandData.PINCode);
    return true;
}

bool emberAfDoorLockClusterUnlockDoorCallback(
    chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
    const chip::app::Clusters::DoorLock::Commands::UnlockDoor::DecodableType & commandData)
{
    ChipLogProgress(Zcl, "Received command: UnlockDoor");

    LockOperationTypeEnum lockOp = LockOperationTypeEnum::kUnlock;

    if (DoorLockServer::Instance().SupportsUnbolt(commandPath.mEndpointId))
    {
        lockOp = LockOperationTypeEnum::kUnlatch;
    }

    if (DoorLockServer::Instance().HandleRemoteLockOperation(commandObj, commandPath, lockOp,
                                                             emberAfPluginDoorLockOnDoorUnlockCommand, commandData.PINCode))
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
    ChipLogProgress(Zcl, "Received command: UnlockWithTimeout");

    LockOperationTypeEnum lockOp = LockOperationTypeEnum::kUnlock;

    if (DoorLockServer::Instance().SupportsUnbolt(commandPath.mEndpointId))
    {
        lockOp = LockOperationTypeEnum::kUnlatch;
    }

    if (DoorLockServer::Instance().HandleRemoteLockOperation(commandObj, commandPath, lockOp,
                                                             emberAfPluginDoorLockOnDoorUnlockCommand, commandData.PINCode))
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

bool emberAfDoorLockClusterUnboltDoorCallback(
    chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
    const chip::app::Clusters::DoorLock::Commands::UnboltDoor::DecodableType & commandData)
{
    ChipLogProgress(Zcl, "Received command: UnboltDoor");

    if (DoorLockServer::Instance().HandleRemoteLockOperation(commandObj, commandPath, LockOperationTypeEnum::kUnlock,
                                                             emberAfPluginDoorLockOnDoorUnboltCommand, commandData.PINCode))
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

bool emberAfDoorLockClusterSetAliroReaderConfigCallback(CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
                                                        const Commands::SetAliroReaderConfig::DecodableType & commandData)
{
    DoorLockServer::Instance().setAliroReaderConfigCommandHandler(commandObj, commandPath, commandData.signingKey,
                                                                  commandData.verificationKey, commandData.groupIdentifier,
                                                                  commandData.groupResolvingKey);
    return true;
}

bool emberAfDoorLockClusterClearAliroReaderConfigCallback(CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
                                                          const Commands::ClearAliroReaderConfig::DecodableType & commandData)
{
    DoorLockServer::Instance().clearAliroReaderConfigCommandHandler(commandObj, commandPath);
    return true;
}

void DoorLockServer::setAliroReaderConfigCommandHandler(CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
                                                        const ByteSpan & signingKey, const ByteSpan & verificationKey,
                                                        const ByteSpan & groupIdentifier,
                                                        const Optional<ByteSpan> & groupResolvingKey)
{
    EndpointId endpointID = commandPath.mEndpointId;
    ChipLogProgress(Zcl, "[SetAliroReaderConfig] Incoming command [endpointId=%d]", endpointID);

    // If Aliro Provisioning feature is not supported, return UNSUPPORTED_COMMAND.
    if (!SupportsAliroProvisioning(endpointID))
    {
        ChipLogProgress(Zcl, "[SetAliroReaderConfig] Aliro Provisioning is not supported [endpointId=%d]", endpointID);
        commandObj->AddStatus(commandPath, Status::UnsupportedCommand);
        return;
    }

    Delegate * delegate = GetDelegate(endpointID);
    VerifyOrReturn(delegate != nullptr, ChipLogError(Zcl, "Delegate is null"));

    // If Aliro BLE UWB feature is supported and groupResolvingKey is not provided in the command, return INVALID_COMMAND.
    if (SupportsAliroBLEUWB(endpointID) && !groupResolvingKey.HasValue())
    {
        ChipLogProgress(Zcl, "[SetAliroReaderConfig] Aliro BLE UWB supported but Group Resolving Key is not provided");
        commandObj->AddStatus(commandPath, Status::InvalidCommand);
        return;
    }

    // Check if the size of the signingKey, verificationKey, groupIdentifier, groupResolvingKey parameters conform to the spec.
    // Return INVALID_COMMAND if not.
    if (signingKey.size() != kAliroSigningKeySize || verificationKey.size() != kAliroReaderVerificationKeySize ||
        groupIdentifier.size() != kAliroReaderGroupIdentifierSize ||
        (groupResolvingKey.HasValue() && groupResolvingKey.Value().size() != kAliroGroupResolvingKeySize))
    {
        ChipLogProgress(Zcl,
                        "[SetAliroReaderConfig] One or more parameters in the command do not meet the size constraint as per spec");
        commandObj->AddStatus(commandPath, Status::ConstraintError);
        return;
    }

    uint8_t buffer[kAliroReaderVerificationKeySize];
    MutableByteSpan readerVerificationKey(buffer);

    CHIP_ERROR err = delegate->GetAliroReaderVerificationKey(readerVerificationKey);

    // If Aliro reader verification key attribute was not read successfuly, return INVALID_IN_STATE. Or if the verification key was
    // read and is not null (i.e not empty), we can't set a new reader config without clearing the previous one, return
    // INVALID_IN_STATE.
    if (err != CHIP_NO_ERROR || !readerVerificationKey.empty())
    {
        ChipLogProgress(
            Zcl, "[SetAliroReaderConfig] Aliro reader verification key was not read or is not null. Return INVALID_IN_STATE");
        commandObj->AddStatus(commandPath, Status::InvalidInState);
        return;
    }

    Status status = Status::Success;
    if (!emberAfPluginDoorLockSetAliroReaderConfig(endpointID, signingKey, verificationKey, groupIdentifier, groupResolvingKey))
    {
        ChipLogProgress(Zcl, "[SetAliroReaderConfig] Unable to set aliro reader config [endpointId=%d]", endpointID);
        status = Status::Failure;
    }
    sendClusterResponse(commandObj, commandPath, ClusterStatusCode(status));
}

void DoorLockServer::clearAliroReaderConfigCommandHandler(CommandHandler * commandObj, const ConcreteCommandPath & commandPath)
{
    EndpointId endpointID = commandPath.mEndpointId;
    ChipLogProgress(Zcl, "[ClearAliroReaderConfig] Incoming command [endpointId=%d]", endpointID);

    // If Aliro Provisioning feature is not supported, return UNSUPPORTED_COMMAND.
    if (!SupportsAliroProvisioning(endpointID))
    {
        ChipLogProgress(Zcl, "[ClearAliroReaderConfig] Aliro Provisioning is not supported [endpointId=%d]", endpointID);
        commandObj->AddStatus(commandPath, Status::UnsupportedCommand);
        return;
    }

    Status status = Status::Success;
    if (!emberAfPluginDoorLockClearAliroReaderConfig(endpointID))
    {
        ChipLogProgress(Zcl, "[SetAliroReaderConfig] Unable to set aliro reader config [endpointId=%d]", endpointID);
        status = Status::Failure;
    }
    sendClusterResponse(commandObj, commandPath, ClusterStatusCode(status));
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
    ChipLogProgress(Zcl, "Door Lock server initialized");
    Server::GetInstance().GetFabricTable().AddFabricDelegate(&gFabricDelegate);

    registerAttributeAccessOverride(&DoorLockServer::Instance());
}

void MatterDoorLockClusterServerAttributeChangedCallback(const app::ConcreteAttributePath & attributePath) {}

void MatterDoorLockClusterServerShutdownCallback(EndpointId endpoint)
{
    ChipLogProgress(Zcl, "Shuting door lock server cluster on endpoint %d", endpoint);

    DeviceLayer::SystemLayer().CancelTimer(DoorLockServer::DoorLockOnAutoRelockCallback,
                                           reinterpret_cast<void *>(static_cast<uintptr_t>(endpoint)));
}

// =============================================================================
// Timer callbacks
// =============================================================================

void DoorLockServer::DoorLockOnAutoRelockCallback(System::Layer *, void * callbackContext)
{
    auto endpointId = static_cast<EndpointId>(reinterpret_cast<uintptr_t>(callbackContext));

    Nullable<DlLockState> lockState;
    if (Attributes::LockState::Get(endpointId, lockState) != Status::Success || lockState.IsNull() ||
        lockState.Value() != DlLockState::kLocked)
    {
        ChipLogProgress(Zcl, "Door Auto relock timer expired. %s", "Locking...");

        DoorLockServer::Instance().SetLockState(endpointId, DlLockState::kLocked, OperationSourceEnum::kAuto);
        emberAfPluginDoorLockOnAutoRelock(endpointId);
    }
    else
    {
        ChipLogProgress(Zcl, "Door Auto relock timer expired. %s", "Already locked.");
    }
}

CHIP_ERROR DoorLockServer::ReadAliroExpeditedTransactionSupportedProtocolVersions(const ConcreteReadAttributePath & aPath,
                                                                                  AttributeValueEncoder & aEncoder,
                                                                                  Delegate * delegate)
{
    VerifyOrReturnValue(delegate != nullptr, aEncoder.EncodeEmptyList());

    return aEncoder.EncodeList([delegate](const auto & encoder) -> CHIP_ERROR {
        for (uint8_t i = 0; true; i++)
        {
            uint8_t buffer[kAliroProtocolVersionSize];
            MutableByteSpan protocolVersion(buffer);
            auto err = delegate->GetAliroExpeditedTransactionSupportedProtocolVersionAtIndex(i, protocolVersion);
            if (err == CHIP_ERROR_PROVIDER_LIST_EXHAUSTED)
            {
                return CHIP_NO_ERROR;
            }
            ReturnErrorOnFailure(err);
            ReturnErrorOnFailure(encoder.Encode(protocolVersion));
        }
    });
}

CHIP_ERROR DoorLockServer::ReadAliroSupportedBLEUWBProtocolVersions(const ConcreteReadAttributePath & aPath,
                                                                    AttributeValueEncoder & aEncoder, Delegate * delegate)
{
    VerifyOrReturnValue(delegate != nullptr, aEncoder.EncodeEmptyList());

    return aEncoder.EncodeList([delegate](const auto & encoder) -> CHIP_ERROR {
        for (uint8_t i = 0; true; i++)
        {
            uint8_t buffer[kAliroProtocolVersionSize];
            MutableByteSpan protocolVersion(buffer);
            auto err = delegate->GetAliroSupportedBLEUWBProtocolVersionAtIndex(i, protocolVersion);
            if (err == CHIP_ERROR_PROVIDER_LIST_EXHAUSTED)
            {
                return CHIP_NO_ERROR;
            }
            ReturnErrorOnFailure(err);
            ReturnErrorOnFailure(encoder.Encode(protocolVersion));
        }
    });
}

CHIP_ERROR DoorLockServer::ReadAliroByteSpanAttribute(CHIP_ERROR (Delegate::*func)(MutableByteSpan &), MutableByteSpan & data,
                                                      Delegate * delegate, AttributeValueEncoder & aEncoder,
                                                      AttributeNullabilityType nullabilityType)
{
    VerifyOrReturnError(delegate != nullptr, CHIP_ERROR_INCORRECT_STATE, ChipLogError(Zcl, "Delegate is null"));

    ReturnErrorOnFailure((delegate->*func)(data));
    if (nullabilityType == AttributeNullabilityType::kNullable && data.empty())
    {
        ReturnErrorOnFailure(aEncoder.EncodeNull());
    }
    else
    {
        ReturnErrorOnFailure(aEncoder.Encode(data));
    }
    return CHIP_NO_ERROR;
}

// Implements the read functionality for the AttributeAccessInterface.
CHIP_ERROR DoorLockServer::Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    if (aPath.mClusterId != Clusters::DoorLock::Id)
    {
        // We shouldn't have been called at all.
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    Delegate * delegate = GetDelegate(aPath.mEndpointId);

    switch (aPath.mAttributeId)
    {
    case AliroReaderVerificationKey::Id: {
        uint8_t buffer[kAliroReaderVerificationKeySize];
        MutableByteSpan readerVerificationKey(buffer);
        return ReadAliroByteSpanAttribute(&Delegate::GetAliroReaderVerificationKey, readerVerificationKey, delegate, aEncoder,
                                          AttributeNullabilityType::kNullable);
    }
    case AliroReaderGroupIdentifier::Id: {
        uint8_t buffer[kAliroReaderGroupIdentifierSize];
        MutableByteSpan readerGroupIdentifier(buffer);
        return ReadAliroByteSpanAttribute(&Delegate::GetAliroReaderGroupIdentifier, readerGroupIdentifier, delegate, aEncoder,
                                          AttributeNullabilityType::kNullable);
    }
    case AliroReaderGroupSubIdentifier::Id: {
        uint8_t buffer[kAliroReaderGroupSubIdentifierSize];
        MutableByteSpan readerGroupSubIdentifier(buffer);
        return ReadAliroByteSpanAttribute(&Delegate::GetAliroReaderGroupSubIdentifier, readerGroupSubIdentifier, delegate, aEncoder,
                                          AttributeNullabilityType::kNotNullable);
    }
    case AliroExpeditedTransactionSupportedProtocolVersions::Id: {
        return ReadAliroExpeditedTransactionSupportedProtocolVersions(aPath, aEncoder, delegate);
    }
    case AliroGroupResolvingKey::Id: {
        uint8_t buffer[kAliroGroupResolvingKeySize];
        MutableByteSpan groupResolvingKey(buffer);
        return ReadAliroByteSpanAttribute(&Delegate::GetAliroGroupResolvingKey, groupResolvingKey, delegate, aEncoder,
                                          AttributeNullabilityType::kNullable);
    }
    case AliroSupportedBLEUWBProtocolVersions::Id: {
        return ReadAliroSupportedBLEUWBProtocolVersions(aPath, aEncoder, delegate);
    }
    case AliroBLEAdvertisingVersion::Id: {
        uint8_t bleAdvertisingVersion = delegate->GetAliroBLEAdvertisingVersion();
        ReturnErrorOnFailure(aEncoder.Encode(bleAdvertisingVersion));
        return CHIP_NO_ERROR;
    }
    case NumberOfAliroCredentialIssuerKeysSupported::Id: {
        uint16_t numberOfCredentialIssuerKeysSupported = delegate->GetNumberOfAliroCredentialIssuerKeysSupported();
        ReturnErrorOnFailure(aEncoder.Encode(numberOfCredentialIssuerKeysSupported));
        return CHIP_NO_ERROR;
    }
    case NumberOfAliroEndpointKeysSupported::Id: {
        uint16_t numberOfEndpointKeysSupported = delegate->GetNumberOfAliroEndpointKeysSupported();
        ReturnErrorOnFailure(aEncoder.Encode(numberOfEndpointKeysSupported));
        return CHIP_NO_ERROR;
    }
    default:
        break;
    }
    return CHIP_NO_ERROR;
}
