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
#include <app-common/zap-generated/attribute-id.h>
#include <app-common/zap-generated/attribute-type.h>
#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/callback.h>
#include <app-common/zap-generated/cluster-id.h>
#include <app-common/zap-generated/command-id.h>
#include <app/util/af-event.h>
#include <app/util/af.h>
#include <app/util/time-util.h>
#include <cinttypes>

#include <app/CommandHandler.h>
#include <app/ConcreteAttributePath.h>
#include <app/ConcreteCommandPath.h>
#include <lib/support/CodeUtils.h>

using namespace chip;
using namespace chip::app::DataModel;
using namespace chip::app::Clusters::DoorLock;

EmberEventControl emberAfPluginDoorLockServerLockoutEventControl;
EmberEventControl emberAfPluginDoorLockServerRelockEventControl;

DoorLockServer DoorLockServer::instance;

// TODO: Remove hardcoded pin when SetCredential command is implemented.
static const uint8_t HARD_CODED_PIN_CODE[] = { 1, 2, 3, 4 };
chip::ByteSpan mPin(HARD_CODED_PIN_CODE);

/**********************************************************
 * DoorLockServer Implementation
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

    SetLockState(endpointId, DlLockState::kLocked);
    SetActuatorEnabled(endpointId, true);
}

bool DoorLockServer::SetLockState(chip::EndpointId endpointId, DlLockState newLockState)
{
    auto lockState = chip::to_underlying(newLockState);

    emberAfDoorLockClusterPrintln("Setting LockState to '%" PRIu8 "'", lockState);
    EmberAfStatus status = Attributes::LockState::Set(endpointId, newLockState);

    if (EMBER_ZCL_STATUS_SUCCESS != status)
    {
        ChipLogError(Zcl, "Unable to set LockState attribute: status=0x%" PRIx8, status);
    }

    return (EMBER_ZCL_STATUS_SUCCESS == status);
}

bool DoorLockServer::SetActuatorEnabled(chip::EndpointId endpointId, bool newActuatorState)
{
    auto actuatorState = static_cast<uint8_t>(newActuatorState);

    emberAfDoorLockClusterPrintln("Setting ActuatorEnabled to '%" PRIu8 "'", actuatorState);
    EmberAfStatus status = Attributes::ActuatorEnabled::Set(endpointId, newActuatorState);

    if (EMBER_ZCL_STATUS_SUCCESS != status)
    {
        ChipLogError(Zcl, "Unable to set ActuatorEnabled attribute: status=0x%" PRIx8, status);
    }

    return (EMBER_ZCL_STATUS_SUCCESS == status);
}

bool DoorLockServer::SetDoorState(chip::EndpointId endpointId, DlDoorState newDoorState)
{
    auto doorState = chip::to_underlying(newDoorState);

    emberAfDoorLockClusterPrintln("Setting DoorState to '%" PRIu8 "'", doorState);
    EmberAfStatus status = Attributes::DoorState::Set(endpointId, newDoorState);

    if (EMBER_ZCL_STATUS_SUCCESS != status)
    {
        ChipLogError(Zcl, "Unable to set DoorState attribute: status=0x%" PRIx8, status);
    }

    return (EMBER_ZCL_STATUS_SUCCESS == status);
}

bool DoorLockServer::SetLanguage(chip::EndpointId endpointId, chip::CharSpan newLanguage)
{
    emberAfDoorLockClusterPrintln("Setting Language to '%.*s'", static_cast<int>(newLanguage.size()), newLanguage.data());
    EmberAfStatus status = Attributes::Language::Set(endpointId, newLanguage);

    if (EMBER_ZCL_STATUS_SUCCESS != status)
    {
        ChipLogError(Zcl, "Unable to set Language attribute: status=0x%" PRIx8, status);
    }

    return (EMBER_ZCL_STATUS_SUCCESS == status);
}

bool DoorLockServer::SetAutoRelockTime(chip::EndpointId endpointId, uint32_t newAutoRelockTimeSec)
{
    emberAfDoorLockClusterPrintln("Setting AutoRelockTime to '%" PRIu32 "'", newAutoRelockTimeSec);
    EmberAfStatus status = Attributes::AutoRelockTime::Set(endpointId, newAutoRelockTimeSec);

    if (EMBER_ZCL_STATUS_SUCCESS != status)
    {
        ChipLogError(Zcl, "Unable to set AutoRelockTime attribute to %" PRIu32 ": status=0x%" PRIx8, newAutoRelockTimeSec, status);
    }

    return (EMBER_ZCL_STATUS_SUCCESS == status);
}

bool DoorLockServer::SetSoundVolume(chip::EndpointId endpointId, uint8_t newSoundVolume)
{
    emberAfDoorLockClusterPrintln("Setting SoundVolume to '%" PRIu8 "'", newSoundVolume);
    EmberAfStatus status = Attributes::SoundVolume::Set(endpointId, newSoundVolume);

    if (EMBER_ZCL_STATUS_SUCCESS != status)
    {
        ChipLogError(Zcl, "Unable to set SoundVolume attribute: status=0x%" PRIx8, status);
    }

    return (EMBER_ZCL_STATUS_SUCCESS == status);
}

bool DoorLockServer::SetOneTouchLocking(chip::EndpointId endpointId, bool isEnabled)
{
    auto enable = static_cast<uint8_t>(isEnabled);

    emberAfDoorLockClusterPrintln("Setting EnableOneTouchLocking to '%" PRIu8 "'", enable);
    EmberAfStatus status = Attributes::EnableOneTouchLocking::Set(endpointId, isEnabled);

    if (EMBER_ZCL_STATUS_SUCCESS != status)
    {
        ChipLogError(Zcl, "Unable to set EnableOneTouchLocking attribute: status=0x%" PRIx8, status);
    }

    return (EMBER_ZCL_STATUS_SUCCESS == status);
}

bool DoorLockServer::SetPrivacyModeButton(chip::EndpointId endpointId, bool isEnabled)
{
    auto enable = static_cast<uint8_t>(isEnabled);

    emberAfDoorLockClusterPrintln("Setting EnablePrivacyModeButton to '%" PRIu8 "'", enable);
    EmberAfStatus status = Attributes::EnablePrivacyModeButton::Set(endpointId, isEnabled);

    if (EMBER_ZCL_STATUS_SUCCESS != status)
    {
        ChipLogError(Zcl, "Unable to set EnablePrivacyModeButton attribute: status=0x%" PRIx8, status);
    }

    return (EMBER_ZCL_STATUS_SUCCESS == status);
}

bool DoorLockServer::CreateUser(chip::EndpointId endpointId, uint16_t userIndex, chip::FabricIndex creatorFabricIdx,
                                const Nullable<chip::CharSpan> & userName, const Nullable<uint32_t> & userUniqueId,
                                const Nullable<DoorLock::DlUserStatus> & userStatus,
                                const Nullable<DoorLock::DlUserType> & userType,
                                const Nullable<DoorLock::DlCredentialRule> & credentialRule)
{
    EmberAfPluginDoorLockUserInfo user;
    if (!emberAfPluginDoorLockGetUser(endpointId, userIndex, user))
    {
        ChipLogError(Zcl, "[CreateUser] Unable to get the user from app [endpointId=%d,userIndex=%d]", endpointId, userIndex);
        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_FAILURE);
        return true;
    }

    // appclusters, 5.2.4.34: to modify user its status should be set to Available. If it is we should return INVALID_COMMAND.
    if (DlUserStatus::kAvailable != user.userStatus)
    {
        emberAfDoorLockClusterPrintln("[CreateUser] Unable to overwrite existing user [endpointId=%d,userIndex=%d]", endpointId,
                                      userIndex);
        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_INVALID_COMMAND);
        return true;
    }

    char newUserName[DOOR_LOCK_USER_NAME_BUFFER_SIZE] = { 0 };
    if (!userName.IsNull())
    {
        memcpy(newUserName, userName.Value().data(), userName.Value().size());
        newUserName[DOOR_LOCK_MAX_USER_NAME_SIZE] = '\0';
    }
    auto newUserUniqueId   = userUniqueId.IsNull() ? 0xFFFFFFFF : userUniqueId.Value();
    auto newUserStatus     = userStatus.IsNull() ? DlUserStatus::kOccupiedEnabled : userStatus.Value();
    auto newUserType       = userType.IsNull() ? DlUserType::kUnrestrictedUser : userType.Value();
    auto newCredentialRule = credentialRule.IsNull() ? DlCredentialRule::kSingle : credentialRule.Value();

    if (!emberAfPluginDoorLockSetUser(endpointId, userIndex, creatorFabricIdx, creatorFabricIdx, newUserName, newUserUniqueId,
                                      newUserStatus, newUserType, newCredentialRule))
    {
        emberAfDoorLockClusterPrintln("[CreateUser] Unable to create user: app error "
                                      "[endpointId=%d,creatorFabricId=%d,userIndex=%d,userName=\"%s\",userUniqueId=0x%x,userStatus="
                                      "%hhu,userType=%hhu,credentialRule=%hhu]",
                                      endpointId, creatorFabricIdx, userIndex, newUserName, newUserUniqueId, newUserStatus,
                                      newUserType, newCredentialRule);
        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_FAILURE);
        return true;
    }

    emberAfDoorLockClusterPrintln("[CreateUser] User created "
                                  "[endpointId=%d,creatorFabricId=%d,userIndex=%d,userName=\"%s\",userUniqueId=0x%x,userStatus=%"
                                  "hhu,userType=%hhu,credentialRule=%hhu]",
                                  endpointId, creatorFabricIdx, userIndex, newUserName, newUserUniqueId, newUserStatus, newUserType,
                                  newCredentialRule);

    // TODO: Send LockUserChange event

    emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_SUCCESS);
    return true;
}

EmberAfStatus DoorLockServer::ClearUser(chip::EndpointId endpointId, uint16_t userIndex)
{
    if (!emberAfPluginDoorLockSetUser(endpointId, userIndex, 0, 0, "", 0, DlUserStatus::kAvailable, DlUserType::kUnrestrictedUser,
                                      DlCredentialRule::kSingle))
    {
        return EMBER_ZCL_STATUS_FAILURE;
    }

    // TODO: Send LockUserChange event

    return EMBER_ZCL_STATUS_SUCCESS;
}

bool DoorLockServer::ModifyUser(chip::EndpointId endpointId, uint16_t userIndex, chip::FabricIndex modifierFabricIndex,
                                const Nullable<chip::CharSpan> & userName, const Nullable<uint32_t> & userUniqueId,
                                const Nullable<DoorLock::DlUserStatus> & userStatus,
                                const Nullable<DoorLock::DlUserType> & userType,
                                const Nullable<DoorLock::DlCredentialRule> & credentialRule)
{
    // We should get the user by that index first
    EmberAfPluginDoorLockUserInfo user;
    if (!emberAfPluginDoorLockGetUser(endpointId, userIndex, user))
    {
        ChipLogError(Zcl, "[ModifyUser] Unable to get the user from app [endpointId=%d,userIndex=%d]", endpointId, userIndex);
        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_FAILURE);
        return true;
    }

    // appclusters, 5.2.4.34: to modify user its status should NOT be set to Available. If it is we should return INVALID_COMMAND.
    if (DlUserStatus::kAvailable == user.userStatus)
    {
        emberAfDoorLockClusterPrintln("[ModifyUser] Unable to modify non-existing user [endpointId=%d,userIndex=%d]", endpointId,
                                      userIndex);
        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_INVALID_COMMAND);
        return true;
    }

    // appclusters, 5.2.4.34: UserName SHALL be null if modifying a user record that was not created by the accessing fabric
    if (user.createdBy != modifierFabricIndex && !userName.IsNull())
    {
        emberAfDoorLockClusterPrintln("[ModifyUser] Unable to modify name of user created by different fabric "
                                      "[endpointId=%d,userIndex=%d,creatorIdx=%d,modifierIdx=%d]",
                                      endpointId, userIndex, user.createdBy, modifierFabricIndex);
        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_INVALID_COMMAND);
        return true;
    }

    // appclusters, 5.2.4.34: UserUniqueID SHALL be null if modifying the user record that was not created by the accessing fabric.
    if (user.createdBy != modifierFabricIndex && !userUniqueId.IsNull())
    {
        emberAfDoorLockClusterPrintln("[ModifyUser] Unable to modify UUID of user created by different fabric "
                                      "[endpointId=%d,userIndex=%d,creatorIdx=%d,modifierIdx=%d]",
                                      endpointId, userIndex, user.createdBy, modifierFabricIndex);
        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_INVALID_COMMAND);
        return true;
    }

    char newUserName[DOOR_LOCK_USER_NAME_BUFFER_SIZE] = { 0 };
    if (!userName.IsNull())
    {
        memcpy(newUserName, userName.Value().data(), userName.Value().size());
        newUserName[DOOR_LOCK_MAX_USER_NAME_SIZE] = 0;
    }
    auto newUserUniqueId   = userUniqueId.IsNull() ? user.userUniqueId : userUniqueId.Value();
    auto newUserStatus     = userStatus.IsNull() ? user.userStatus : userStatus.Value();
    auto newUserType       = userType.IsNull() ? user.userType : userType.Value();
    auto newCredentialRule = credentialRule.IsNull() ? user.credentialRule : credentialRule.Value();

    if (!emberAfPluginDoorLockSetUser(endpointId, userIndex, user.createdBy, modifierFabricIndex, newUserName, newUserUniqueId,
                                      newUserStatus, newUserType, newCredentialRule))
    {
        ChipLogError(Zcl,
                     "[ModifyUser] Unable to modify the user: app error "
                     "[endpointId=%d,modifierFabric=%d,userIndex=%d,userName=\"%s\",userUniqueId=0x%x,userStatus=%hhu,userType=%"
                     "hhu,credentialRule=%hhu]",
                     endpointId, modifierFabricIndex, userIndex, newUserName, newUserUniqueId, newUserStatus, newUserType,
                     newCredentialRule);
        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_FAILURE);
    }

    ChipLogError(Zcl,
                 "[ModifyUser] User modified "
                 "[endpointId=%d,modifierFabric=%d,userIndex=%d,userName=\"%s\",userUniqueId=0x%x,userStatus=%hhu,userType=%"
                 "hhu,credentialRule=%hhu]",
                 endpointId, modifierFabricIndex, userIndex, newUserName, newUserUniqueId, newUserStatus, newUserType,
                 newCredentialRule);

    // TODO: Send LockUserChange event

    emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_SUCCESS);
    return true;
}

bool DoorLockServer::UserIndexValid(chip::EndpointId endpointId, uint16_t userIndex)
{
    uint16_t maxNumberOfUsers;
    return UserIndexValid(endpointId, userIndex, maxNumberOfUsers);
}

bool DoorLockServer::UserIndexValid(chip::EndpointId endpointId, uint16_t userIndex, uint16_t & maxNumberOfUsers)
{
    EmberAfStatus status = Attributes::NumberOfTotalUsersSupported::Get(endpointId, &maxNumberOfUsers);
    if (EMBER_ZCL_STATUS_SUCCESS != status)
    {
        ChipLogError(Zcl, "Unable to read attribute 'NumberOfTotalUsersSupported' [status:%d]", status);
        return true;
    }

    // appclusters, 5.2.4.34-37: user index changes from 1 to maxNumberOfUsers
    if (0 == userIndex || userIndex > maxNumberOfUsers)
    {
        return false;
    }
    return true;
}

// =============================================================================
// Cluster commands callbacks
// =============================================================================

bool emberAfDoorLockClusterLockDoorCallback(chip::app::CommandHandler * commandObj,
                                            const chip::app::ConcreteCommandPath & commandPath,
                                            const chip::app::Clusters::DoorLock::Commands::LockDoor::DecodableType & commandData)
{
    emberAfDoorLockClusterPrintln("Received Lock Door command");
    bool success = false;

    chip::EndpointId endpoint = commandPath.mEndpointId;

    bool require_pin = false;
    Attributes::RequirePINforRemoteOperation::Get(endpoint, &require_pin);

    if (commandData.pinCode.HasValue())
    {
        // TODO: Search through list of stored PINs and check each.
        if (mPin.data_equal(commandData.pinCode.Value()))
        {
            success = emberAfPluginDoorLockOnDoorLockCommand(endpoint, commandData.pinCode);
        }
        else
        {
            success = false; // Just to be explicit. success == false at this point anyway
        }
    }
    else
    {
        if (!require_pin)
        {
            success = emberAfPluginDoorLockOnDoorLockCommand(endpoint, commandData.pinCode);
        }
        else
        {
            success = false;
        }
    }

    if (success)
    {
        success = DoorLockServer::Instance().SetLockState(endpoint, DlLockState::kLocked) == EMBER_ZCL_STATUS_SUCCESS;
    }

    emberAfSendImmediateDefaultResponse(success ? EMBER_ZCL_STATUS_SUCCESS : EMBER_ZCL_STATUS_FAILURE);

    return true;
}

bool emberAfDoorLockClusterUnlockDoorCallback(
    chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
    const chip::app::Clusters::DoorLock::Commands::UnlockDoor::DecodableType & commandData)
{
    emberAfDoorLockClusterPrintln("Received Unlock Door command");
    bool success = false;

    chip::EndpointId endpoint = commandPath.mEndpointId;

    bool require_pin = false;
    Attributes::RequirePINforRemoteOperation::Get(endpoint, &require_pin);

    if (commandData.pinCode.HasValue())
    {
        // TODO: Search through list of stored PINs and check each.
        if (mPin.data_equal(commandData.pinCode.Value()))
        {
            success = emberAfPluginDoorLockOnDoorUnlockCommand(endpoint, commandData.pinCode);
        }
        else
        {
            success = false; // Just to be explicit. success == false at this point anyway
        }
    }
    else
    {
        if (!require_pin)
        {
            success = emberAfPluginDoorLockOnDoorUnlockCommand(endpoint, commandData.pinCode);
        }
        else
        {
            success = false;
        }
    }

    if (success)
    {
        success = DoorLockServer::Instance().SetLockState(endpoint, DlLockState::kUnlocked) == EMBER_ZCL_STATUS_SUCCESS;
    }

    emberAfSendImmediateDefaultResponse(success ? EMBER_ZCL_STATUS_SUCCESS : EMBER_ZCL_STATUS_FAILURE);

    return true;
}

bool emberAfDoorLockClusterUnlockWithTimeoutCallback(
    chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
    const chip::app::Clusters::DoorLock::Commands::UnlockWithTimeout::DecodableType & commandData)
{
    emberAfDoorLockClusterPrintln("UnlockWithTimeout: command not implemented");

    // TODO: Implement door unlocking with timeout
    emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_SUCCESS);
    return true;
}

bool emberAfDoorLockClusterSetUserCallback(chip::app::CommandHandler * commandObj,
                                           const chip::app::ConcreteCommandPath & commandPath,
                                           const chip::app::Clusters::DoorLock::Commands::SetUser::DecodableType & commandData)
{
    // TODO: Make sure that USR feature is enabled prior to executing command

    auto & userIndex = commandData.userIndex;
    emberAfDoorLockClusterPrintln("[SetUser] Incoming command [endpointId=%d,userIndex=%d]", commandPath.mEndpointId, userIndex);

    if (nullptr == commandObj || nullptr == commandObj->GetExchangeContext())
    {
        ChipLogError(Zcl, "Cannot access ExchangeContext of Command Object for Fabric Index");
        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_FAILURE);
        return true;
    }

    auto fabricIndex      = commandObj->GetExchangeContext()->GetSessionHandle().GetFabricIndex();
    auto & operationType  = commandData.operationType;
    auto & userName       = commandData.userName;
    auto & userUniqueId   = commandData.userUniqueId;
    auto & userStatus     = commandData.userStatus;
    auto & userType       = commandData.userType;
    auto & credentialRule = commandData.credentialRule;

    if (!DoorLockServer::Instance().UserIndexValid(commandPath.mEndpointId, userIndex))
    {
        emberAfDoorLockClusterPrintln("[SetUser] User index out of bounds [userIndex=%d]", userIndex);
        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_INVALID_COMMAND);
        return true;
    }

    // appclusters, 5.2.4.34: UserName has maximum DOOR_LOCK_MAX_USER_NAME_SIZE (10) characters excluding NUL terminator in it.
    if (!userName.IsNull() && userName.Value().size() > DOOR_LOCK_MAX_USER_NAME_SIZE)
    {
        emberAfDoorLockClusterPrintln(
            "[SetUser] Unable to set user: userName too long [endpointId=%d,userIndex=%d,userNameSize=%zu]",
            commandPath.mEndpointId, userIndex, userName.Value().size());

        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_INVALID_COMMAND);
        return true;
    }

    switch (operationType)
    {
    case DlDataOperationType::kAdd:
        return DoorLockServer::Instance().CreateUser(commandPath.mEndpointId, userIndex, fabricIndex, userName, userUniqueId,
                                                     userStatus, userType, credentialRule);
    case DlDataOperationType::kModify:
        return DoorLockServer::Instance().ModifyUser(commandPath.mEndpointId, userIndex, fabricIndex, commandData.userName,
                                                     commandData.userUniqueId, commandData.userStatus, commandData.userType,
                                                     commandData.credentialRule);
    case DlDataOperationType::kClear:
        // appclusters, 5.2.4.34: SetUser command allow only kAdd/kModify, we should respond with INVALID_COMMAND if we got kClear
        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_INVALID_COMMAND);
        return true;
    }

    emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_SUCCESS);
    return true;
}

bool emberAfDoorLockClusterGetUserCallback(chip::app::CommandHandler * commandObj,
                                           const chip::app::ConcreteCommandPath & commandPath,
                                           const chip::app::Clusters::DoorLock::Commands::GetUser::DecodableType & commandData)
{
    auto & userIndex = commandData.userIndex;

    emberAfDoorLockClusterPrintln("[GetUser] Incoming command [endpointId=%d,userIndex=%d]", commandPath.mEndpointId, userIndex);

    uint16_t maxNumberOfUsers = 0;
    if (!DoorLockServer::Instance().UserIndexValid(commandPath.mEndpointId, userIndex, maxNumberOfUsers))
    {
        emberAfDoorLockClusterPrintln("[GetUser] User index out of bounds [userIndex=%d,numberOfTotalUsersSupported=%d]", userIndex,
                                      maxNumberOfUsers);
        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_INVALID_COMMAND);
        return true;
    }

    CHIP_ERROR err = CHIP_NO_ERROR;
    EmberAfPluginDoorLockUserInfo user;
    VerifyOrExit(emberAfPluginDoorLockGetUser(commandPath.mEndpointId, userIndex, user), err = CHIP_ERROR_INTERNAL);
    {
        app::ConcreteCommandPath path = { emberAfCurrentEndpoint(), DoorLock::Id, Commands::GetUserResponse::Id };
        TLV::TLVWriter * writer       = nullptr;
        SuccessOrExit(err = commandObj->PrepareCommand(path));
        VerifyOrExit((writer = commandObj->GetCommandDataIBTLVWriter()) != nullptr, err = CHIP_ERROR_INCORRECT_STATE);
        SuccessOrExit(err = writer->Put(TLV::ContextTag(to_underlying(Commands::GetUserResponse::Fields::kUserIndex)), userIndex));

        // appclusters, 5.2.4.36: we should not add user-specific field if the user status is set to Available
        if (DlUserStatus::kAvailable != user.userStatus)
        {
            emberAfDoorLockClusterPrintln(
                "Found user in storage: "
                "[userIndex=%d,userName=\"%s\",userStatus=%hhu,userType=%hhu,credentialRule=%hhu,createdBy=%hhu,modifiedBy=%hhu]",
                userIndex, user.userName, user.userStatus, user.userType, user.credentialRule, user.createdBy, user.lastModifiedBy);

            chip::CharSpan userName(user.userName);
            SuccessOrExit(
                err = writer->PutString(TLV::ContextTag(to_underlying(Commands::GetUserResponse::Fields::kUserName)), userName));

            SuccessOrExit(err = writer->Put(TLV::ContextTag(to_underlying(Commands::GetUserResponse::Fields::kUserUniqueId)),
                                            user.userUniqueId));
            SuccessOrExit(
                err = writer->Put(TLV::ContextTag(to_underlying(Commands::GetUserResponse::Fields::kUserStatus)), user.userStatus));
            SuccessOrExit(
                err = writer->Put(TLV::ContextTag(to_underlying(Commands::GetUserResponse::Fields::kUserType)), user.userType));
            SuccessOrExit(err = writer->Put(TLV::ContextTag(to_underlying(Commands::GetUserResponse::Fields::kCredentialRule)),
                                            user.credentialRule));
            SuccessOrExit(err = writer->Put(TLV::ContextTag(to_underlying(Commands::GetUserResponse::Fields::kCreatorFabricIndex)),
                                            user.createdBy));
            SuccessOrExit(
                err = writer->Put(TLV::ContextTag(to_underlying(Commands::GetUserResponse::Fields::kLastModifiedFabricIndex)),
                                  user.lastModifiedBy));
        }
        else
        {
            emberAfDoorLockClusterPrintln("[GetUser] User not found [userIndex=%d]", userIndex);
        }

        // appclusters, 5.2.4.36.1: We need to add next available user after userIndex if any.
        for (uint16_t i = userIndex + 1; i < maxNumberOfUsers; ++i)
        {
            VerifyOrExit(emberAfPluginDoorLockGetUser(commandPath.mEndpointId, i, user), err = CHIP_ERROR_INTERNAL);
            if (DlUserStatus::kAvailable == user.userStatus)
            {
                SuccessOrExit(
                    err = writer->Put(TLV::ContextTag(to_underlying(Commands::GetUserResponse::Fields::kNextUserIndex)), i));
                break;
            }
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
    return true;
}

bool emberAfDoorLockClusterClearUserCallback(chip::app::CommandHandler * commandObj,
                                             const chip::app::ConcreteCommandPath & commandPath,
                                             const chip::app::Clusters::DoorLock::Commands::ClearUser::DecodableType & commandData)
{
    auto & userIndex = commandData.userIndex;
    emberAfDoorLockClusterPrintln("[ClearUser] Incoming command [endpointId=%d,userIndex=%d]", commandPath.mEndpointId, userIndex);

    uint16_t maxNumberOfUsers = 0;
    if (userIndex != 0xFFFE && !DoorLockServer::Instance().UserIndexValid(commandPath.mEndpointId, userIndex, maxNumberOfUsers))
    {
        emberAfDoorLockClusterPrintln("[ClearUser] User index out of bounds [userIndex=%d,numberOfTotalUsersSupported=%d]",
                                      userIndex, maxNumberOfUsers);
        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_INVALID_COMMAND);
        return true;
    }

    if (0xFFFE != userIndex)
    {
        auto status = DoorLockServer::Instance().ClearUser(commandPath.mEndpointId, userIndex);
        if (EMBER_ZCL_STATUS_SUCCESS != status)
        {
            ChipLogError(Zcl, "[ClearUser] App reported failure when resetting the user [userIndex=%d,status=0x%x]", userIndex,
                         status);
        }
        emberAfSendImmediateDefaultResponse(status);
        return true;
    }

    emberAfDoorLockClusterPrintln("[ClearUser] Removing all users from storage");
    for (uint16_t i = 0; i < maxNumberOfUsers; ++i)
    {
        auto status = DoorLockServer::Instance().ClearUser(commandPath.mEndpointId, i);
        if (!status)
        {
            ChipLogError(Zcl, "[ClearUser] App reported failure when resetting the user [userIndex=%d,status=0x%x]", i, status);

            emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_FAILURE);
            return true;
        }
    }
    emberAfDoorLockClusterPrintln("[ClearUser] Removed all users from storage [users=%d]", maxNumberOfUsers);

    emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_SUCCESS);
    return true;
}

bool emberAfDoorLockClusterSetCredentialCallback(
    chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
    const chip::app::Clusters::DoorLock::Commands::SetCredential::DecodableType & commandData)
{
    emberAfDoorLockClusterPrintln("[SetCredential] Incoming command [endpointId=%d]", commandPath.mEndpointId);

    auto & operationType = commandData.operationType;
    //    auto & credential = commandData.credential;
    //    auto & credentialData = commandData.credentialData;
    auto & userIndex = commandData.userIndex;
    //    auto & userStatus = commandData.userStatus;

    switch (operationType)
    {
    case DlDataOperationType::kAdd: {
        if (userIndex.IsNull())
        {
            // TODO: Create new credential and user
        }
        // TODO: Create new credential and modify the user if possible
    }
    break;
    case DlDataOperationType::kModify:
        // TODO: Modify the credential if possible
        break;
    case DlDataOperationType::kClear:
        // appclusters, 5.2.4.40: set credential command supports only Add and Modify operational type.
        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_INVALID_COMMAND);
        return true;
    }

    emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_SUCCESS);
    return true;
}

bool emberAfDoorLockClusterGetCredentialStatusCallback(
    chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
    const chip::app::Clusters::DoorLock::Commands::GetCredentialStatus::DecodableType & commandData)
{
    emberAfDoorLockClusterPrintln("[GetCredentialStatus] Incoming command [endpointId=%d]", commandPath.mEndpointId);

    // TODO: Implement getting the credential status
    emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_SUCCESS);
    return true;
}

bool emberAfDoorLockClusterClearCredentialCallback(
    chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
    const chip::app::Clusters::DoorLock::Commands::ClearCredential::DecodableType & commandData)
{
    emberAfDoorLockClusterPrintln("[ClearCredential] Incoming command [endpointId=%d]", commandPath.mEndpointId);

    // TODO: Implement clearing the credential
    emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_SUCCESS);
    return true;
}

bool emberAfDoorLockClusterSetWeekDayScheduleCallback(
    chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
    const chip::app::Clusters::DoorLock::Commands::SetWeekDaySchedule::DecodableType & commandData)
{
    emberAfDoorLockClusterPrintln("SetWeekDaySchedule: command not implemented");

    // TODO: Implement setting weekday schedule
    emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_SUCCESS);
    return true;
}

bool emberAfDoorLockClusterGetWeekDayScheduleCallback(
    chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
    const chip::app::Clusters::DoorLock::Commands::GetWeekDaySchedule::DecodableType & commandData)
{
    emberAfDoorLockClusterPrintln("GetWeekDaySchedule: command not implemented");

    // TODO: Implement getting weekday schedule
    emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_SUCCESS);
    return true;
}

bool emberAfDoorLockClusterClearWeekDayScheduleCallback(
    chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
    const chip::app::Clusters::DoorLock::Commands::ClearWeekDaySchedule::DecodableType & commandData)
{
    emberAfDoorLockClusterPrintln("ClearWeekDaySchedule: command not implemented");

    // TODO: Implement clearing weekday schedule
    emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_SUCCESS);
    return true;
}

bool emberAfDoorLockClusterSetYearDayScheduleCallback(
    chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
    const chip::app::Clusters::DoorLock::Commands::SetYearDaySchedule::DecodableType & commandData)
{
    emberAfDoorLockClusterPrintln("SetYearDaySchedule: command not implemented");

    // TODO: Implement setting year day schedule
    emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_SUCCESS);
    return true;
}

bool emberAfDoorLockClusterGetYearDayScheduleCallback(
    chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
    const chip::app::Clusters::DoorLock::Commands::GetYearDaySchedule::DecodableType & commandData)
{
    emberAfDoorLockClusterPrintln("GetYearDaySchedule: command not implemented");

    // TODO: Implement getting year day schedule
    emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_SUCCESS);
    return true;
}

bool emberAfDoorLockClusterClearYearDayScheduleCallback(
    chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
    const chip::app::Clusters::DoorLock::Commands::ClearYearDaySchedule::DecodableType & commandData)
{
    emberAfDoorLockClusterPrintln("ClearYearDaySchedule: command not implemented");

    // TODO: Implement clearing year day schedule
    emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_SUCCESS);
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

bool __attribute__((weak))
emberAfPluginDoorLockOnDoorLockCommand(chip::EndpointId endpointId, chip::Optional<chip::ByteSpan> pinCode)
{
    return false;
}

bool __attribute__((weak))
emberAfPluginDoorLockOnDoorUnlockCommand(chip::EndpointId endpointId, chip::Optional<chip::ByteSpan> pinCode)
{
    return false;
}

// =============================================================================
// Pre-change callbacks for cluster attributes
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

#if DOOR_LOCK_SERVER_ENABLE_DEFAULT_USERS_CREDENTIALS_IMPLEMENTATION
static EmberAfPluginDoorLockUserInfo gs_users[100];

bool emberAfPluginDoorLockGetUser(chip::EndpointId endpointId, uint16_t userIndex, EmberAfPluginDoorLockUserInfo & user)
{
    user = gs_users[userIndex];
    return true;
}

bool emberAfPluginDoorLockSetUser(chip::EndpointId endpointId, uint16_t userIndex, chip::FabricIndex creator,
                                  chip::FabricIndex modifier, const char * userName, uint32_t uniqueId,
                                  DoorLock::DlUserStatus userStatus, DoorLock::DlUserType usertype,
                                  DoorLock::DlCredentialRule credentialRule)
{
    strcpy(gs_users[userIndex].userName, userName);
    gs_users[userIndex].userUniqueId   = uniqueId;
    gs_users[userIndex].userStatus     = userStatus;
    gs_users[userIndex].userType       = usertype;
    gs_users[userIndex].credentialRule = credentialRule;
    gs_users[userIndex].lastModifiedBy = modifier;
    gs_users[userIndex].createdBy      = creator;

    return true;
}

static EmberAfPluginDoorLockCredentialInfo gs_credentials[100];

bool emberAfPluginDoorLockGetCredential(chip::EndpointId endpointId, uint16_t credentialIndex,
                                        EmberAfPluginDoorLockCredentialInfo & credential)
{
    return false;
}

bool emberAfPluginDoorLockSetCredential(chip::EndpointId endpointId, uint16_t credentialIndex,
                                        DoorLock::DlCredentialType credentialType, const chip::ByteSpan & credentialData)
{
    return false;
}
#endif /* DOOR_LOCK_SERVER_ENABLE_DEFAULT_USERS_CREDENTIALS_IMPLEMENTATION */
