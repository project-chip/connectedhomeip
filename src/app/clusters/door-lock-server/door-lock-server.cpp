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

EmberAfStatus
DoorLockServer::CreateUser(chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
                           uint16_t userIndex, const Nullable<chip::CharSpan> & userName, const Nullable<uint32_t> & userUniqueId,
                           const Nullable<DoorLock::DlUserStatus> & userStatus, const Nullable<DoorLock::DlUserType> & userType,
                           const Nullable<DoorLock::DlCredentialRule> & credentialRule, const Nullable<DlCredential> & credentials)
{
    auto endpointId = commandPath.mEndpointId;

    auto creatorFabricIdx = getFabricIndex(commandObj);
    if (kUndefinedFabricIndex == creatorFabricIdx)
    {
        ChipLogError(Zcl, "[CreateUser] Unable to get the creator fabric Index - internal error [endpointId=%d,userIndex=%d]",
                     endpointId, userIndex);
        return EMBER_ZCL_STATUS_FAILURE;
    }

    EmberAfPluginDoorLockUserInfo user;
    if (!emberAfPluginDoorLockGetUser(endpointId, userIndex, user))
    {
        ChipLogError(Zcl, "[CreateUser] Unable to get the user from app [endpointId=%d,userIndex=%d]", endpointId, userIndex);
        return EMBER_ZCL_STATUS_FAILURE;
    }

    // appclusters, 5.2.4.34: to modify user its status should be set to Available. If it is we should return OCCUPIED.
    if (DlUserStatus::kAvailable != user.userStatus)
    {
        emberAfDoorLockClusterPrintln("[CreateUser] Unable to overwrite existing user [endpointId=%d,userIndex=%d]", endpointId,
                                      userIndex);
        //        commandObj->AddClusterSpecificFailure(commandPath, to_underlying(DlStatus::kOccupied));
        return EMBER_ZCL_STATUS_FAILURE;
    }

    char newUserName[DOOR_LOCK_USER_NAME_BUFFER_SIZE] = { 0 };
    if (!userName.IsNull())
    {
        memcpy(newUserName, userName.Value().data(), userName.Value().size());
        newUserName[DOOR_LOCK_MAX_USER_NAME_SIZE] = '\0';
    }
    auto newUserUniqueId                = userUniqueId.IsNull() ? 0xFFFFFFFF : userUniqueId.Value();
    auto newUserStatus                  = userStatus.IsNull() ? DlUserStatus::kOccupiedEnabled : userStatus.Value();
    auto newUserType                    = userType.IsNull() ? DlUserType::kUnrestrictedUser : userType.Value();
    auto newCredentialRule              = credentialRule.IsNull() ? DlCredentialRule::kSingle : credentialRule.Value();
    const DlCredential * newCredentials = nullptr;
    size_t newTotalCredentials          = 0;
    if (!credentials.IsNull())
    {
        newCredentials      = &credentials.Value();
        newTotalCredentials = 1;
    }

    if (!emberAfPluginDoorLockSetUser(endpointId, userIndex, creatorFabricIdx, creatorFabricIdx, newUserName, newUserUniqueId,
                                      newUserStatus, newUserType, newCredentialRule, newCredentials, newTotalCredentials))
    {
        emberAfDoorLockClusterPrintln("[CreateUser] Unable to create user: app error "
                                      "[endpointId=%d,creatorFabricId=%d,userIndex=%d,userName=\"%s\",userUniqueId=0x%x,userStatus="
                                      "%hhu,userType=%hhu,credentialRule=%hhu,totalCredentials=%zu]",
                                      endpointId, creatorFabricIdx, userIndex, newUserName, newUserUniqueId, newUserStatus,
                                      newUserType, newCredentialRule, newTotalCredentials);
        return EMBER_ZCL_STATUS_FAILURE;
    }

    emberAfDoorLockClusterPrintln("[CreateUser] User created "
                                  "[endpointId=%d,creatorFabricId=%d,userIndex=%d,userName=\"%s\",userUniqueId=0x%x,userStatus=%"
                                  "hhu,userType=%hhu,credentialRule=%hhu,totalCredentials=%zu]",
                                  endpointId, creatorFabricIdx, userIndex, newUserName, newUserUniqueId, newUserStatus, newUserType,
                                  newCredentialRule, newTotalCredentials);

    // TODO: Send LockUserChange event

    return EMBER_ZCL_STATUS_SUCCESS;
}

EmberAfStatus DoorLockServer::ClearUser(chip::EndpointId endpointId, uint16_t userIndex)
{
    if (!emberAfPluginDoorLockSetUser(endpointId, userIndex, 0, 0, "", 0, DlUserStatus::kAvailable, DlUserType::kUnrestrictedUser,
                                      DlCredentialRule::kSingle, nullptr, 0))
    {
        return EMBER_ZCL_STATUS_FAILURE;
    }

    // TODO: Send LockUserChange event
    return EMBER_ZCL_STATUS_SUCCESS;
}

EmberAfStatus
DoorLockServer::ModifyUser(chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
                           uint16_t userIndex, const Nullable<chip::CharSpan> & userName, const Nullable<uint32_t> & userUniqueId,
                           const Nullable<DoorLock::DlUserStatus> & userStatus, const Nullable<DoorLock::DlUserType> & userType,
                           const Nullable<DoorLock::DlCredentialRule> & credentialRule, const Nullable<DlCredential> & credentials)
{
    auto endpointId = commandPath.mEndpointId;

    auto modifierFabricIndex = getFabricIndex(commandObj);
    if (kUndefinedFabricIndex == modifierFabricIndex)
    {
        ChipLogError(Zcl, "[ModifyUser] Unable to get the modifier fabric Index - internal error [endpointId=%d,userIndex=%d]",
                     endpointId, userIndex);
        return EMBER_ZCL_STATUS_FAILURE;
    }

    // We should get the user by that index first
    EmberAfPluginDoorLockUserInfo user;
    if (!emberAfPluginDoorLockGetUser(endpointId, userIndex, user))
    {
        ChipLogError(Zcl, "[ModifyUser] Unable to get the user from app [endpointId=%d,userIndex=%d]", endpointId, userIndex);
        ;
        return EMBER_ZCL_STATUS_FAILURE;
    }

    // appclusters, 5.2.4.34: to modify user its status should NOT be set to Available. If it is we should return INVALID_COMMAND.
    if (DlUserStatus::kAvailable == user.userStatus)
    {
        emberAfDoorLockClusterPrintln("[ModifyUser] Unable to modify non-existing user [endpointId=%d,userIndex=%d]", endpointId,
                                      userIndex);
        return EMBER_ZCL_STATUS_INVALID_COMMAND;
    }

    // appclusters, 5.2.4.34: UserName SHALL be null if modifying a user record that was not created by the accessing fabric
    if (user.createdBy != modifierFabricIndex && !userName.IsNull())
    {
        emberAfDoorLockClusterPrintln("[ModifyUser] Unable to modify name of user created by different fabric "
                                      "[endpointId=%d,userIndex=%d,creatorIdx=%d,modifierIdx=%d]",
                                      endpointId, userIndex, user.createdBy, modifierFabricIndex);
        return EMBER_ZCL_STATUS_INVALID_COMMAND;
    }

    // appclusters, 5.2.4.34: UserUniqueID SHALL be null if modifying the user record that was not created by the accessing fabric.
    if (user.createdBy != modifierFabricIndex && !userUniqueId.IsNull())
    {
        emberAfDoorLockClusterPrintln("[ModifyUser] Unable to modify UUID of user created by different fabric "
                                      "[endpointId=%d,userIndex=%d,creatorIdx=%d,modifierIdx=%d]",
                                      endpointId, userIndex, user.createdBy, modifierFabricIndex);
        return EMBER_ZCL_STATUS_INVALID_COMMAND;
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

    if (!credentials.IsNull())
    {
        if (user.totalCredentials + 1 > DOOR_LOCK_MAX_CREDENTIALS_PER_USER)
        {
            emberAfDoorLockClusterPrintln("[ModifyUser] Unable to append credentials to user: too many credentials "
                                          "[endpointId=%d,userIndex=%d,userTotalCredentials=%zu]",
                                          endpointId, userIndex, user.totalCredentials);
            return EMBER_ZCL_STATUS_INVALID_COMMAND;
        }

        user.credentials[user.totalCredentials++] = credentials.Value();
    }

    if (!emberAfPluginDoorLockSetUser(endpointId, userIndex, user.createdBy, modifierFabricIndex, newUserName, newUserUniqueId,
                                      newUserStatus, newUserType, newCredentialRule, user.credentials, user.totalCredentials))
    {
        ChipLogError(Zcl,
                     "[ModifyUser] Unable to modify the user: app error "
                     "[endpointId=%d,modifierFabric=%d,userIndex=%d,userName=\"%s\",userUniqueId=0x%x,userStatus=%hhu,userType=%"
                     "hhu,credentialRule=%hhu,totalCredentials=%zu]",
                     endpointId, modifierFabricIndex, userIndex, newUserName, newUserUniqueId, newUserStatus, newUserType,
                     newCredentialRule, user.totalCredentials);
        return EMBER_ZCL_STATUS_FAILURE;
    }

    emberAfDoorLockClusterPrintln(
        "[ModifyUser] User modified "
        "[endpointId=%d,modifierFabric=%d,userIndex=%d,userName=\"%s\",userUniqueId=0x%x,userStatus=%hhu,userType=%"
        "hhu,credentialRule=%hhu,totalCredentials=%zu]",
        endpointId, modifierFabricIndex, userIndex, newUserName, newUserUniqueId, newUserStatus, newUserType, newCredentialRule,
        user.totalCredentials);

    // TODO: Send LockUserChange event

    return EMBER_ZCL_STATUS_SUCCESS;
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
        return false;
    }

    // appclusters, 5.2.4.34-37: user index changes from 1 to maxNumberOfUsers
    if (0 == userIndex || userIndex > maxNumberOfUsers)
    {
        return false;
    }
    return true;
}

chip::FabricIndex DoorLockServer::getFabricIndex(const chip::app::CommandHandler * commandObj)
{
    if (nullptr == commandObj || nullptr == commandObj->GetExchangeContext())
    {
        ChipLogError(Zcl, "Cannot access ExchangeContext of Command Object for Fabric Index");
        return kUndefinedFabricIndex;
    }
    return commandObj->GetAccessingFabricIndex();
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

    EmberAfStatus status = EMBER_ZCL_STATUS_SUCCESS;
    switch (operationType)
    {
    case DlDataOperationType::kAdd:
        status = DoorLockServer::Instance().CreateUser(commandObj, commandPath, userIndex, userName, userUniqueId, userStatus,
                                                       userType, credentialRule);
        break;
    case DlDataOperationType::kModify:
        status = DoorLockServer::Instance().ModifyUser(commandObj, commandPath, userIndex, userName, userUniqueId, userStatus,
                                                       userType, credentialRule);
        break;
    case DlDataOperationType::kClear:
        // appclusters, 5.2.4.34: SetUser command allow only kAdd/kModify, we should respond with INVALID_COMMAND if we got kClear
        status = EMBER_ZCL_STATUS_INVALID_COMMAND;
        break;
    }

    emberAfSendImmediateDefaultResponse(status);
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

        using ResponseFields = Commands::GetUserResponse::Fields;

        // appclusters, 5.2.4.36: we should not add user-specific field if the user status is set to Available
        if (DlUserStatus::kAvailable != user.userStatus)
        {
            emberAfDoorLockClusterPrintln(
                "Found user in storage: "
                "[userIndex=%d,userName=\"%s\",userStatus=%hhu,userType=%hhu,credentialRule=%hhu,createdBy=%hhu,modifiedBy=%hhu]",
                userIndex, user.userName, user.userStatus, user.userType, user.credentialRule, user.createdBy, user.lastModifiedBy);

            chip::CharSpan userName(user.userName, strlen(user.userName));
            SuccessOrExit(err = writer->PutString(TLV::ContextTag(to_underlying(ResponseFields::kUserName)), userName));
            SuccessOrExit(err = writer->Put(TLV::ContextTag(to_underlying(ResponseFields::kUserUniqueId)), user.userUniqueId));
            SuccessOrExit(err = writer->Put(TLV::ContextTag(to_underlying(ResponseFields::kUserStatus)), user.userStatus));
            SuccessOrExit(err = writer->Put(TLV::ContextTag(to_underlying(ResponseFields::kUserType)), user.userType));
            SuccessOrExit(err = writer->Put(TLV::ContextTag(to_underlying(ResponseFields::kCredentialRule)), user.credentialRule));
            SuccessOrExit(err = writer->Put(TLV::ContextTag(to_underlying(ResponseFields::kCreatorFabricIndex)), user.createdBy));
            SuccessOrExit(
                err = writer->Put(TLV::ContextTag(to_underlying(ResponseFields::kLastModifiedFabricIndex)), user.lastModifiedBy));

            if (user.totalCredentials > 0)
            {
                TLV::TLVType credentialsContainer;
                SuccessOrExit(err = writer->StartContainer(TLV::ContextTag(to_underlying(ResponseFields::kCredentials)),
                                                           TLV::kTLVType_Array, credentialsContainer));
                for (size_t i = 0; i < user.totalCredentials; ++i)
                {
                    DoorLock::Structs::DlCredential::Type credential;
                    credential.credentialIndex = user.credentials[i].CredentialIndex;
                    credential.credentialType  = static_cast<DlCredentialType>(user.credentials[i].CredentialType);
                    SuccessOrExit(err = credential.Encode(*writer, TLV::AnonymousTag));
                }
                SuccessOrExit(err = writer->EndContainer(credentialsContainer));
            }
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
                SuccessOrExit(err = writer->Put(TLV::ContextTag(to_underlying(ResponseFields::kNextUserIndex)), i));
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
    if (!DoorLockServer::Instance().UserIndexValid(commandPath.mEndpointId, userIndex, maxNumberOfUsers) && userIndex != 0xFFFE)
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
    for (uint16_t i = 1; i <= maxNumberOfUsers; ++i)
    {
        auto status = DoorLockServer::Instance().ClearUser(commandPath.mEndpointId, i);
        if (EMBER_ZCL_STATUS_SUCCESS != status)
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

static CHIP_ERROR sendSetCredentialResponse(chip::app::CommandHandler * commandObj, DlStatus status,
                                            const Nullable<uint16_t> & userIndex, const Nullable<uint16_t> & nextCredentialIndex)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    app::ConcreteCommandPath path = { emberAfCurrentEndpoint(), DoorLock::Id, Commands::SetCredentialResponse::Id };
    TLV::TLVWriter * writer       = nullptr;
    SuccessOrExit(err = commandObj->PrepareCommand(path));
    VerifyOrExit((writer = commandObj->GetCommandDataIBTLVWriter()) != nullptr, err = CHIP_ERROR_INCORRECT_STATE);
    SuccessOrExit(err = writer->Put(TLV::ContextTag(to_underlying(Commands::SetCredentialResponse::Fields::kStatus)), status));

    if (!userIndex.IsNull())
    {
        SuccessOrExit(err = writer->Put(TLV::ContextTag(to_underlying(Commands::SetCredentialResponse::Fields::kUserIndex)),
                                        userIndex.Value()));
    }

    if (!nextCredentialIndex.IsNull())
    {
        SuccessOrExit(err =
                          writer->Put(TLV::ContextTag(to_underlying(Commands::SetCredentialResponse::Fields::kNextCredentialIndex)),
                                      nextCredentialIndex.Value()));
    }
    SuccessOrExit(err = commandObj->FinishCommand());

exit:
    return err;
}

bool findUnoccupiedUserSlot(chip::EndpointId endpointId, uint16_t & userIndex)
{
    uint16_t maxNumberOfUsers;
    EmberAfStatus status = Attributes::NumberOfTotalUsersSupported::Get(endpointId, &maxNumberOfUsers);
    if (EMBER_ZCL_STATUS_SUCCESS != status)
    {
        ChipLogError(Zcl, "Unable to read attribute 'NumberOfTotalUsersSupported' [status:%d]", status);
        return false;
    }

    userIndex = 0;
    for (uint16_t i = 1; i <= maxNumberOfUsers; ++i)
    {
        EmberAfPluginDoorLockUserInfo user;
        if (!emberAfPluginDoorLockGetUser(endpointId, i, user))
        {
            ChipLogError(Zcl, "Unable to get user to check if slot is occupied: app error [status:%d,userIndex=%d]", status, i);
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

bool emberAfDoorLockClusterSetCredentialCallback(
    chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
    const chip::app::Clusters::DoorLock::Commands::SetCredential::DecodableType & commandData)
{
    emberAfDoorLockClusterPrintln("[SetCredential] Incoming command [endpointId=%d]", commandPath.mEndpointId);

    auto & operationType   = commandData.operationType;
    auto & credentialType  = commandData.credential.credentialType;
    auto & credentialIndex = commandData.credential.credentialIndex;
    auto & credentialData  = commandData.credentialData;
    auto & userIndex       = commandData.userIndex;
    auto & userStatus      = commandData.userStatus;
    auto & userType        = commandData.userType;

    switch (operationType)
    {
    case DlDataOperationType::kAdd: {
        EmberAfPluginDoorLockCredentialInfo existingCredential;
        if (!emberAfPluginDoorLockGetCredential(commandPath.mEndpointId, credentialIndex, existingCredential))
        {
            emberAfDoorLockClusterPrintln(
                "[SetCredential] Unable to check if credential exists: app error [endpointId=%d,credentialIndex=%d]",
                commandPath.mEndpointId, credentialIndex);

            emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_FAILURE);
            return true;
        }

        // appclusters, 5.2.4.41.1: should send the OCCUPIED in the response when the credential is in use
        if (existingCredential.inUse)
        {
            emberAfDoorLockClusterPrintln(
                "[SetCredential] Unable to set the credential: credential slot is occupied [endpointId=%d,credentialIndex=%d]",
                commandPath.mEndpointId, credentialIndex);

            sendSetCredentialResponse(commandObj, DlStatus::kOccupied, Nullable<uint16_t>(), Nullable<uint16_t>());
            return true;
        }

        // TODO: Check credential size

        DlCredential credential{ to_underlying(credentialType), credentialIndex };
        // appclusters, 5.2.4.40: if userIndex is not provided we should create new user
        if (userIndex.IsNull())
        {
            emberAfDoorLockClusterPrintln(
                "[SetCredential] UserIndex is not set, creating new user [endpointId=%d,credentialIndex=%d]",
                commandPath.mEndpointId, credentialIndex);
            uint16_t availableUserIndex = 0;
            if (!findUnoccupiedUserSlot(commandPath.mEndpointId, availableUserIndex))
            {
                emberAfDoorLockClusterPrintln("[SetCredential] Unable to create new user for credential: no available user slots "
                                              "[endpointId=%d,credentialIndex=%d]",
                                              commandPath.mEndpointId, credentialIndex);

                sendSetCredentialResponse(commandObj, DlStatus::kOccupied, Nullable<uint16_t>(), Nullable<uint16_t>());
                return true;
            }

            auto status = DoorLockServer::Instance().CreateUser(commandObj, commandPath, availableUserIndex, Nullable<CharSpan>(),
                                                                Nullable<uint32_t>(), userStatus, userType,
                                                                Nullable<DlCredentialRule>(), Nullable<DlCredential>(credential));
            if (EMBER_ZCL_STATUS_SUCCESS != status)
            {
                emberAfDoorLockClusterPrintln("[SetCredential] Unable to create new user for credential: internal error "
                                              "[endpointId=%d,credentialIndex=%d,userIndex=%d,status=%d]",
                                              commandPath.mEndpointId, credential.CredentialIndex, availableUserIndex, status);

                sendSetCredentialResponse(commandObj, DlStatus::kFailure, Nullable<uint16_t>(), Nullable<uint16_t>());
                return true;
            }

            if (!emberAfPluginDoorLockSetCredential(commandPath.mEndpointId, credentialIndex, credentialType, credentialData))
            {
                emberAfDoorLockClusterPrintln("[SetCredential] Unable to set the credential: app error "
                                              "[endpointId=%d,credentialIndex=%d,credentialType=%hhu,dataLength=%zu]",
                                              commandPath.mEndpointId, credentialIndex, credentialType, credentialData.size());

                // TODO: Use appropriate status code
                sendSetCredentialResponse(commandObj, DlStatus::kOccupied, Nullable<uint16_t>(), Nullable<uint16_t>());
                return true;
            }

            emberAfDoorLockClusterPrintln("[SetCredential] Credential and user were created "
                                          "[endpointId=%d,credentialIndex=%d,credentialType=%hhu,dataLength=%zu,userIndex=%d]",
                                          commandPath.mEndpointId, credentialIndex, credentialType, credentialData.size(),
                                          availableUserIndex);

            sendSetCredentialResponse(commandObj, DlStatus::kSuccess, Nullable<uint16_t>(availableUserIndex),
                                      Nullable<uint16_t>(credentialIndex + 1));
            return true;
        }

        // TODO: Check if userIndex in bounds

        // appclusters, 5.2.4.40: if user index is NULL, we should try to modify the existing user
        EmberAfPluginDoorLockUserInfo user;
        if (!emberAfPluginDoorLockGetUser(commandPath.mEndpointId, userIndex.Value(), user))
        {
            emberAfDoorLockClusterPrintln(
                "[SetCredential] Unable to check if credential exists: app error [endpointId=%d,credentialIndex=%d,userIndex=%d]",
                commandPath.mEndpointId, credentialIndex, userIndex.Value());

            sendSetCredentialResponse(commandObj, DlStatus::kFailure, Nullable<uint16_t>(), Nullable<uint16_t>());
            return true;
        }

        // Not in the spec, but common sense: I don't think we need to modify the credential if user slot is not occupied
        if (user.userStatus == DlUserStatus::kAvailable)
        {
            emberAfDoorLockClusterPrintln("[SetCredential] Unable to add credential to user: user clot is empty "
                                          "[endpointId=%d,credentialIndex=%d,userIndex=%d]",
                                          commandPath.mEndpointId, credentialIndex, userIndex.Value());

            sendSetCredentialResponse(commandObj, DlStatus::kInvalidField, Nullable<uint16_t>(), Nullable<uint16_t>());
            return true;
        }

        // appclusters, 5.2.4.40: If user already contains the credential of the same type we should return INVALID_COMMAND
        // TODO: Check if credentials of the same type are already associated with user (maybe inside ModifyUser)

        // Modify the existing user
        auto status = DoorLockServer::Instance().ModifyUser(commandObj, commandPath, userIndex.Value(), Nullable<CharSpan>(),
                                                            Nullable<uint32_t>(), userStatus, userType,
                                                            Nullable<DlCredentialRule>(), Nullable<DlCredential>(credential));
        if (EMBER_ZCL_STATUS_SUCCESS != status)
        {
            emberAfDoorLockClusterPrintln("[SetCredential] Unable to add credential to a user: internal error "
                                          "[endpointId=%d,credentialIndex=%d,userIndex=%d,status=%d]",
                                          commandPath.mEndpointId, credential.CredentialIndex, userIndex.Value(), status);

            // TODO: Use proper status codes
            sendSetCredentialResponse(commandObj, DlStatus::kFailure, Nullable<uint16_t>(), Nullable<uint16_t>());
            return true;
        }

        sendSetCredentialResponse(commandObj, DlStatus::kSuccess, Nullable<uint16_t>(), Nullable<uint16_t>(credentialIndex + 1));
        return true;
    }
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
                                  DoorLock::DlCredentialRule credentialRule, const DlCredential * credentials,
                                  size_t totalCredentials)
{
    strcpy(gs_users[userIndex].userName, userName);
    gs_users[userIndex].userUniqueId   = uniqueId;
    gs_users[userIndex].userStatus     = userStatus;
    gs_users[userIndex].userType       = usertype;
    gs_users[userIndex].credentialRule = credentialRule;
    gs_users[userIndex].lastModifiedBy = modifier;
    gs_users[userIndex].createdBy      = creator;

    gs_users[userIndex].totalCredentials = totalCredentials;
    for (size_t i = 0; i < totalCredentials; ++i)
    {
        gs_users[userIndex].credentials[i] = credentials[i];
    }

    return true;
}

static EmberAfPluginDoorLockCredentialInfo gs_credentials[100];

bool emberAfPluginDoorLockGetCredential(chip::EndpointId endpointId, uint16_t credentialIndex,
                                        EmberAfPluginDoorLockCredentialInfo & credential)
{
    credential = gs_credentials[credentialIndex];
    return true;
}

bool emberAfPluginDoorLockSetCredential(chip::EndpointId endpointId, uint16_t credentialIndex,
                                        DoorLock::DlCredentialType credentialType, const chip::ByteSpan & credentialData)
{
    gs_credentials[credentialIndex].inUse          = true;
    gs_credentials[credentialIndex].credentialType = credentialType;
    gs_credentials[credentialIndex].credentialData = credentialData;

    return true;
}
#endif /* DOOR_LOCK_SERVER_ENABLE_DEFAULT_USERS_CREDENTIALS_IMPLEMENTATION */
