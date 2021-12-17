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

EmberAfStatus DoorLockServer::ModifyUser(chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
                                         uint16_t userIndex, const Nullable<chip::CharSpan> & userName,
                                         const Nullable<uint32_t> & userUniqueId,
                                         const Nullable<DoorLock::DlUserStatus> & userStatus,
                                         const Nullable<DoorLock::DlUserType> & userType,
                                         const Nullable<DoorLock::DlCredentialRule> & credentialRule)
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

    if (!emberAfPluginDoorLockSetUser(endpointId, userIndex, user.createdBy, modifierFabricIndex, newUserName, newUserUniqueId,
                                      newUserStatus, newUserType, newCredentialRule, user.credentials, user.totalCredentials))
    {
        ChipLogError(Zcl,
                     "[ModifyUser] Unable to modify the user: app error "
                     "[endpointId=%d,modifierFabric=%d,userIndex=%d,userName=\"%s\",userUniqueId=0x%x,userStatus=%hhu,userType=%"
                     "hhu,credentialRule=%hhu]",
                     endpointId, modifierFabricIndex, userIndex, newUserName, newUserUniqueId, newUserStatus, newUserType,
                     newCredentialRule);
        return EMBER_ZCL_STATUS_FAILURE;
    }

    emberAfDoorLockClusterPrintln("[ModifyUser] User modified "
                                  "[endpointId=%d,modifierFabric=%d,userIndex=%d,userName=\"%s\",userUniqueId=0x%x,userStatus=%hhu,"
                                  "userType=%hhu,credentialRule=%hhu]",
                                  endpointId, modifierFabricIndex, userIndex, newUserName, newUserUniqueId, newUserStatus,
                                  newUserType, newCredentialRule);

    // TODO: Send LockUserChange event

    return EMBER_ZCL_STATUS_SUCCESS;
}

bool DoorLockServer::userIndexValid(chip::EndpointId endpointId, uint16_t userIndex)
{
    uint16_t maxNumberOfUsers;
    return userIndexValid(endpointId, userIndex, maxNumberOfUsers);
}

bool DoorLockServer::userIndexValid(chip::EndpointId endpointId, uint16_t userIndex, uint16_t & maxNumberOfUser)
{
    EmberAfStatus status = Attributes::NumberOfTotalUsersSupported::Get(endpointId, &maxNumberOfUser);
    if (EMBER_ZCL_STATUS_SUCCESS != status)
    {
        ChipLogError(Zcl, "Unable to read attribute 'NumberOfTotalUsersSupported' [status:%d]", status);
        return false;
    }

    // appclusters, 5.2.4.34-37: user index changes from 1 to maxNumberOfUsers
    if (0 == userIndex || userIndex > maxNumberOfUser)
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
        status = CreateUser(commandObj, commandPath, userIndex, userName, userUniqueId, userStatus, userType, credentialRule);
        break;
    case DlDataOperationType::kModify:
        status = ModifyUser(commandObj, commandPath, userIndex, userName, userUniqueId, userStatus, userType, credentialRule);
        break;
    case DlDataOperationType::kClear:
        // appclusters, 5.2.4.34: SetUser command allow only kAdd/kModify, we should respond with INVALID_COMMAND if we got kClear
        status = EMBER_ZCL_STATUS_INVALID_COMMAND;
        break;
    }

    emberAfSendImmediateDefaultResponse(status);
}

void DoorLockServer::GetUserCommandHandler(chip::app::CommandHandler * commandObj, const app::ConcreteCommandPath & commandPath,
                                           const Commands::GetUser::DecodableType & commandData)
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
        app::ConcreteCommandPath path = { emberAfCurrentEndpoint(), DoorLock::Id, Commands::GetUserResponse::Id };
        TLV::TLVWriter * writer;
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
        if (findUnoccupiedUserSlot(commandPath.mEndpointId, userIndex + 1, nextAvailableUserIndex))
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

void DoorLockServer::ClearUserCommandHandler(chip::app::CommandHandler * commandObj, const app::ConcreteCommandPath & commandPath,
                                             const Commands::ClearUser::DecodableType & commandData)
{
    auto & userIndex = commandData.userIndex;
    emberAfDoorLockClusterPrintln("[ClearUser] Incoming command [endpointId=%d,userIndex=%d]", commandPath.mEndpointId, userIndex);

    if (!SupportsUSR(commandPath.mEndpointId))
    {
        emberAfDoorLockClusterPrintln("[ClearUser] User management is not supported [endpointId=%d]", commandPath.mEndpointId);
        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_UNSUPPORTED_COMMAND);
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
        auto status = clearUser(commandPath.mEndpointId, userIndex);
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
        auto status = clearUser(commandPath.mEndpointId, i);
        if (EMBER_ZCL_STATUS_SUCCESS != status)
        {
            ChipLogError(Zcl, "[ClearUser] App reported failure when resetting the user [userIndex=%d,status=0x%x]", i, status);

            emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_FAILURE);
            return;
        }
    }
    emberAfDoorLockClusterPrintln("[ClearUser] Removed all users from storage [users=%d]", maxNumberOfUsers);

    emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_SUCCESS);
}

EmberAfStatus DoorLockServer::clearUser(chip::EndpointId endpointId, uint16_t userIndex)
{
    if (!emberAfPluginDoorLockSetUser(endpointId, userIndex, 0, 0, "", 0, DlUserStatus::kAvailable, DlUserType::kUnrestrictedUser,
                                      DlCredentialRule::kSingle, nullptr, 0))
    {
        return EMBER_ZCL_STATUS_FAILURE;
    }

    // TODO: Send LockUserChange event
    return EMBER_ZCL_STATUS_SUCCESS;
}

void DoorLockServer::SetCredentialCommandHandler(chip::app::CommandHandler * commandObj,
                                                 const app::ConcreteCommandPath & commandPath,
                                                 const Commands::SetCredential::DecodableType & commandData)
{
    emberAfDoorLockClusterPrintln("[SetCredential] Incoming command [endpointId=%d]", commandPath.mEndpointId);

    auto & operationType   = commandData.operationType;
    auto & credentialType  = commandData.credential.credentialType;
    auto & credentialIndex = commandData.credential.credentialIndex;
    auto & credentialData  = commandData.credentialData;
    auto & userIndex       = commandData.userIndex;
    auto & userStatus      = commandData.userStatus;
    auto & userType        = commandData.userType;

    if (!credentialTypeSupported(commandPath.mEndpointId, credentialType))
    {
        emberAfDoorLockClusterPrintln("[GetCredentialStatus] Credential type is not supported [endpointId=%d,credentialType=%hhu]",
                                      commandPath.mEndpointId, credentialType);
        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_UNSUPPORTED_COMMAND);
        return;
    }

    if (!credentialIndexValid(commandPath.mEndpointId, credentialType, credentialIndex))
    {
        emberAfDoorLockClusterPrintln(
            "[GetCredentialStatus] Credential index is out of range [endpointId=%d,credentialType=%hhu,credentialIndex=%d]",
            commandPath.mEndpointId, credentialType, credentialIndex);
        sendSetCredentialResponse(commandObj, DlStatus::kInvalidField, 0, 0);
        return;
    }

    size_t minSize, maxSize;
    if (!getCredentialRange(commandPath.mEndpointId, credentialType, minSize, maxSize))
    {
        emberAfDoorLockClusterPrintln(
            "[SetCredential] Unable to set min/max range for credential: internal error [endpointId=%d,credentialIndex=%d]",
            commandPath.mEndpointId, credentialIndex);
        sendSetCredentialResponse(commandObj, DlStatus::kFailure, 0, 0);
        return;
    }

    if (credentialData.size() < minSize || credentialData.size() > maxSize)
    {
        emberAfDoorLockClusterPrintln("[SetCredential] Credential data size is out of range "
                                      "[endpointId=%d,credentialType=%hhu,minLength=%zu,maxLength=%zu,length=%zu]",
                                      commandPath.mEndpointId, credentialType, minSize, maxSize, credentialData.size());
        sendSetCredentialResponse(commandObj, DlStatus::kInvalidField, 0, 0);
        return;
    }

    switch (operationType)
    {
    case DlDataOperationType::kAdd: {
        uint16_t nextAvailableCredentialSlot = 0;

        EmberAfPluginDoorLockCredentialInfo existingCredential;
        if (!emberAfPluginDoorLockGetCredential(commandPath.mEndpointId, credentialIndex, existingCredential))
        {
            emberAfDoorLockClusterPrintln(
                "[SetCredential] Unable to check if credential exists: app error [endpointId=%d,credentialIndex=%d]",
                commandPath.mEndpointId, credentialIndex);

            sendSetCredentialResponse(commandObj, DlStatus::kFailure, 0, 0);
            return;
        }

        // appclusters, 5.2.4.41.1: should send the OCCUPIED in the response when the credential is in use
        if (DlCredentialStatus::kAvailable != existingCredential.status)
        {
            emberAfDoorLockClusterPrintln(
                "[SetCredential] Unable to set the credential: credential slot is occupied [endpointId=%d,credentialIndex=%d]",
                commandPath.mEndpointId, credentialIndex);

            findUnoccupiedCredentialSlot(commandPath.mEndpointId, credentialType, credentialIndex + 1, nextAvailableCredentialSlot);
            sendSetCredentialResponse(commandObj, DlStatus::kOccupied, 0, nextAvailableCredentialSlot);
            return;
        }

        // TODO: Check if credential exists with the same data

        DlCredential credential{ to_underlying(credentialType), credentialIndex };
        // appclusters, 5.2.4.40: if userIndex is not provided we should create new user
        DlStatus status           = DlStatus::kSuccess;
        uint16_t createdUserIndex = 0;
        if (userIndex.IsNull())
        {
            emberAfDoorLockClusterPrintln(
                "[SetCredential] UserIndex is not set, creating new user [endpointId=%d,credentialIndex=%d]",
                commandPath.mEndpointId, credentialIndex);

            status = createNewCredentialAndUser(commandObj, commandPath, userStatus, userType, credential, credentialData,
                                                createdUserIndex);
        }
        else
        {
            // appclusters, 5.2.4.40: if user index is NULL, we should try to modify the existing user
            status = createNewCredentialAndAddItToUser(commandObj, commandPath, userIndex.Value(), credential, credentialData);
        }

        if (DlStatus::kSuccess != status)
        {
            nextAvailableCredentialSlot = credentialIndex;
        }
        else
        {
            findUnoccupiedCredentialSlot(commandPath.mEndpointId, credentialType, credentialIndex + 1, nextAvailableCredentialSlot);
        }

        sendSetCredentialResponse(commandObj, status, createdUserIndex, nextAvailableCredentialSlot);
        return;
    }
    case DlDataOperationType::kModify:
        // TODO: Modify the credential if possible
        break;
    case DlDataOperationType::kClear:
        // appclusters, 5.2.4.40: set credential command supports only Add and Modify operational type.
        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_INVALID_COMMAND);
        return;
    }

    emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_SUCCESS);
}

void DoorLockServer::GetCredentialStatusCommandHandler(chip::app::CommandHandler * commandObj,
                                                       const app::ConcreteCommandPath & commandPath,
                                                       const Commands::GetCredentialStatus::DecodableType & commandData)
{
    emberAfDoorLockClusterPrintln("[GetCredentialStatus] Incoming command [endpointId=%d]", commandPath.mEndpointId);

    auto & credentialType  = commandData.credential.credentialType;
    auto & credentialIndex = commandData.credential.credentialIndex;

    if (!credentialTypeSupported(commandPath.mEndpointId, credentialType))
    {
        emberAfDoorLockClusterPrintln("[GetCredentialStatus] Credential type is not supported [endpointId=%d,credentialType=%hhu]",
                                      commandPath.mEndpointId, credentialType);
        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_UNSUPPORTED_COMMAND);
        return;
    }

    uint16_t maxNumberOfCredentials = 0;
    if (!credentialIndexValid(commandPath.mEndpointId, credentialType, credentialIndex, maxNumberOfCredentials))
    {
        emberAfDoorLockClusterPrintln("[GetCredentialStatus] Credential index is out of range "
                                      "[endpointId=%d,credentialType=%hhu,credentialIndex=%d,maxNumberOfCredentials=%d]",
                                      commandPath.mEndpointId, credentialType, credentialIndex, maxNumberOfCredentials);
        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_INVALID_COMMAND);
        return;
    }

    EmberAfPluginDoorLockCredentialInfo credentialInfo;
    if (!emberAfPluginDoorLockGetCredential(commandPath.mEndpointId, credentialIndex, credentialInfo))
    {
        emberAfDoorLockClusterPrintln("[GetCredentialStatus] Unable to get the credential: app error "
                                      "[endpointId=%d,credentialIndex=%d,credentialType=%hhu]",
                                      commandPath.mEndpointId, credentialIndex, credentialType);
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
                         "[endpointId=%d,credentialType=%hhu,credentialIndex=%d]",
                         commandPath.mEndpointId, credentialType, credentialIndex);
            emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_FAILURE);
        }
    }

    uint16_t nextCredentialIndex = 0;

    CHIP_ERROR err                = CHIP_NO_ERROR;
    using ResponseFields          = Commands::GetCredentialStatusResponse::Fields;
    app::ConcreteCommandPath path = { emberAfCurrentEndpoint(), DoorLock::Id, Commands::GetCredentialStatusResponse::Id };
    TLV::TLVWriter * writer       = nullptr;
    SuccessOrExit(err = commandObj->PrepareCommand(path));
    VerifyOrExit((writer = commandObj->GetCommandDataIBTLVWriter()) != nullptr, err = CHIP_ERROR_INCORRECT_STATE);
    SuccessOrExit(err = writer->Put(TLV::ContextTag(to_underlying(ResponseFields::kCredentialExists)), credentialExists));
    if (0 != userIndexWithCredential)
    {
        SuccessOrExit(err = writer->Put(TLV::ContextTag(to_underlying(ResponseFields::kUserIndex)), userIndexWithCredential));
    }
    if (findUnoccupiedCredentialSlot(commandPath.mEndpointId, credentialType, credentialIndex + 1, nextCredentialIndex))
    {
        SuccessOrExit(err = writer->Put(TLV::ContextTag(to_underlying(ResponseFields::kNextCredentialIndex)), nextCredentialIndex));
    }
    SuccessOrExit(err = commandObj->FinishCommand());

    emberAfDoorLockClusterPrintln("[GetCredentialStatus] Prepared credential status "
                                  "[endpointId=%d,credentialType=%hhu,credentialIndex=%d,userIndex=%d,nextCredentialIndex=%d]",
                                  commandPath.mEndpointId, credentialType, credentialIndex, userIndexWithCredential,
                                  nextCredentialIndex);

exit:
    if (CHIP_NO_ERROR != err)
    {
        ChipLogError(Zcl,
                     "[GetCredentialStatus] Error occurred when preparing response: %s "
                     "[endpointId=%d,credentialType=%hhu,credentialIndex=%d,userIndex=%d,nextCredentialIndex=%d]",
                     err.AsString(), commandPath.mEndpointId, credentialType, credentialIndex, userIndexWithCredential,
                     nextCredentialIndex);
        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_FAILURE);
    }
}

void DoorLockServer::ClearCredentialCommandHandler(chip::app::CommandHandler * commandObj,
                                                   const app::ConcreteCommandPath & commandPath,
                                                   const Commands::ClearCredential::DecodableType & commandData)
{
    emberAfDoorLockClusterPrintln("[ClearCredential] Incoming command [endpointId=%d]", commandPath.mEndpointId);

    // TODO: Implement clearing the credential
    emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_SUCCESS);
}

bool DoorLockServer::findUnoccupiedUserSlot(chip::EndpointId endpointId, uint16_t & userIndex)
{
    return findUnoccupiedUserSlot(endpointId, 1, userIndex);
}

bool DoorLockServer::findUnoccupiedUserSlot(chip::EndpointId endpointId, uint16_t startIndex, uint16_t & userIndex)
{
    uint16_t maxNumberOfUsers;
    EmberAfStatus status = Attributes::NumberOfTotalUsersSupported::Get(endpointId, &maxNumberOfUsers);
    if (EMBER_ZCL_STATUS_SUCCESS != status)
    {
        ChipLogError(Zcl, "Unable to read attribute 'NumberOfTotalUsersSupported' [status:%d]", status);
        return false;
    }

    userIndex = 0;
    for (uint16_t i = startIndex; i <= maxNumberOfUsers; ++i)
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

CHIP_ERROR DoorLockServer::sendSetCredentialResponse(chip::app::CommandHandler * commandObj, DlStatus status, uint16_t userIndex,
                                                     uint16_t nextCredentialIndex)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    using ResponseFields = Commands::SetCredentialResponse::Fields;

    app::ConcreteCommandPath path = { emberAfCurrentEndpoint(), DoorLock::Id, Commands::SetCredentialResponse::Id };
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

DoorLock::DlStatus DoorLockServer::addCredentialToUser(chip::EndpointId endpointId, uint16_t userIndex,
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

    for (size_t i = 0; i < user.totalCredentials; ++i)
    {
        // appclusters, 5.2.4.40: If user already contains the credential of the same type we should return INVALID_COMMAND
        if (user.credentials[i].CredentialType == credential.CredentialType)
        {
            emberAfDoorLockClusterPrintln(
                "[AddCredentialToUser] Unable to add credential to user: credential with this type already exists "
                "[endpointId=%d,userIndex=%d,credentialType=%d]",
                endpointId, userIndex, credential.CredentialType);
            return DlStatus::kOccupied;
        }

        // appclusters, 5.2.4.40: user should not be already associated with given credentialIndex
        if (user.credentials[i].CredentialIndex == credential.CredentialIndex)
        {
            emberAfDoorLockClusterPrintln(
                "[AddCredentialToUser] Unable to add credential to user: credential with this index is already associated "
                "with user [endpointId=%d,userIndex=%d,credentialType=%d,credentialIndex=%d]",
                endpointId, userIndex, credential.CredentialType, credential.CredentialIndex);
            return DlStatus::kInvalidField;
        }
    }

    // appclusters: spec defines up to 5 credentials per user
    if (user.totalCredentials + 1 > DOOR_LOCK_MAX_CREDENTIALS_PER_USER)
    {
        emberAfDoorLockClusterPrintln("[AddCredentialToUser] Unable to add credentials to user: too many credentials "
                                      "[endpointId=%d,userIndex=%d,userTotalCredentials=%zu]",
                                      endpointId, userIndex, user.totalCredentials);
        return DlStatus::kInvalidField;
    }

    user.credentials[user.totalCredentials++] = credential;

    // TODO: user appropriate fabric here
    if (!emberAfPluginDoorLockSetUser(endpointId, userIndex, user.createdBy, user.lastModifiedBy, user.userName, user.userUniqueId,
                                      user.userStatus, user.userType, user.credentialRule, user.credentials, user.totalCredentials))
    {
        emberAfDoorLockClusterPrintln(
            "[AddCredentialToUser] Unable to add credential to user: credential with this index is already associated "
            "with user [endpointId=%d,userIndex=%d,credentialType=%d,credentialIndex=%d,userTotalCredentials=%zu]",
            endpointId, userIndex, credential.CredentialType, credential.CredentialIndex, user.totalCredentials);
        return DlStatus::kFailure;
    }

    emberAfDoorLockClusterPrintln("[AddCredentialToUser] Credential added to user "
                                  "[endpointId=%d,userIndex=%d,credentialType=%d,credentialIndex=%d,userTotalCredentials=%zu]",
                                  endpointId, userIndex, credential.CredentialType, credential.CredentialIndex,
                                  user.totalCredentials);

    // TODO: send lock user change event

    return DlStatus::kSuccess;
}

DoorLock::DlStatus DoorLockServer::modifyCredentialForUser(chip::EndpointId endpointId, uint16_t userIndex,
                                                           const DlCredential & credential)
{
    // We should get the user by that index first
    EmberAfPluginDoorLockUserInfo user;
    if (!emberAfPluginDoorLockGetUser(endpointId, userIndex, user))
    {
        emberAfDoorLockClusterPrintln("[ModifyUserCredential] Unable to get the user from app [endpointId=%d,userIndex=%d]",
                                      endpointId, userIndex);
        return DlStatus::kFailure;
    }

    for (size_t i = 0; i < user.totalCredentials; ++i)
    {
        // appclusters, 5.2.4.40: user should already be associated with given credentialIndex
        if (user.credentials[i].CredentialIndex == credential.CredentialIndex)
        {
            user.credentials[i] = credential;

            emberAfDoorLockClusterPrintln(
                "[ModifyUserCredential] Unable to add credential to user: credential with this index is already associated "
                "with user [endpointId=%d,userIndex=%d,credentialType=%d,credentialIndex=%d]",
                endpointId, userIndex, credential.CredentialType, credential.CredentialIndex);

            // TODO: send lock user change event

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

bool DoorLockServer::HasFeature(chip::EndpointId endpointId, DoorLock::DoorLockFeature feature)
{
    uint32_t featureMap = 0;
    if (EMBER_ZCL_STATUS_SUCCESS != Attributes::FeatureMap::Get(endpointId, &featureMap))
    {
        return false;
    }
    return (featureMap & to_underlying(feature)) != 0;
}

bool DoorLockServer::credentialTypeSupported(chip::EndpointId endpointId, DoorLock::DlCredentialType type)
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

bool DoorLockServer::credentialIndexValid(chip::EndpointId endpointId, DoorLock::DlCredentialType type, uint16_t credentialIndex)
{
    uint16_t maxCredentials = 0;
    return credentialIndexValid(endpointId, type, credentialIndex, maxCredentials);
}

bool DoorLockServer::credentialIndexValid(chip::EndpointId endpointId, DoorLock::DlCredentialType type, uint16_t credentialIndex,
                                          uint16_t & maxNumberOfCredentials)
{
    if (!getMaxNumberOfCredentials(endpointId, type, maxNumberOfCredentials))
    {
        return false;
    }

    if (0 == credentialIndex || credentialIndex > maxNumberOfCredentials)
    {
        return false;
    }

    return true;
}

bool DoorLockServer::findUserIndexByCredential(chip::EndpointId endpointId, DoorLock::DlCredentialType credentialType,
                                               uint16_t credentialIndex, uint16_t & userIndex)
{
    uint16_t maxNumberOfUsers = 0;
    EmberAfStatus status      = Attributes::NumberOfTotalUsersSupported::Get(endpointId, &maxNumberOfUsers);
    if (EMBER_ZCL_STATUS_SUCCESS != status)
    {
        ChipLogError(Zcl, "Unable to read attribute 'NumberOfTotalUsersSupported' [status=%d]", status);
        return false;
    }

    for (uint16_t i = 1; i <= maxNumberOfUsers; ++i)
    {
        EmberAfPluginDoorLockUserInfo user;
        if (!emberAfPluginDoorLockGetUser(endpointId, i, user))
        {
            ChipLogError(Zcl, "[GetCredentialStatus] Unable to get user: app error [status=%d,userIndex=%d]", status, i);
            emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_FAILURE);
            return false;
        }

        for (uint16_t j = 0; j < user.totalCredentials; ++j)
        {
            if (user.credentials[j].CredentialIndex == credentialIndex &&
                user.credentials[j].CredentialType == to_underlying(credentialType))
            {
                userIndex = i;
                return true;
            }
        }
    }

    return false;
}

bool DoorLockServer::findUnoccupiedCredentialSlot(chip::EndpointId endpointId, DoorLock::DlCredentialType credentialType,
                                                  uint16_t startIndex, uint16_t & credentialIndex)
{
    uint16_t maxNumberOfCredentials = 0;
    if (!getMaxNumberOfCredentials(endpointId, credentialType, maxNumberOfCredentials))
    {
        return false;
    }

    for (uint16_t i = startIndex; i < maxNumberOfCredentials; ++i)
    {
        EmberAfPluginDoorLockCredentialInfo info;
        if (!emberAfPluginDoorLockGetCredential(endpointId, i, info))
        {
            ChipLogError(Zcl, "Unable to get credential: app error [endpointId=%d,credentialType=%hhu,credentialIndex=%d]",
                         endpointId, credentialType, i);
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

bool DoorLockServer::getMaxNumberOfCredentials(chip::EndpointId endpointId, DoorLock::DlCredentialType credentialType,
                                               uint16_t & maxNumberOfCredentials)
{
    maxNumberOfCredentials = 0;
    EmberAfStatus status   = EMBER_ZCL_STATUS_SUCCESS;
    switch (credentialType)
    {
    case DlCredentialType::kProgrammingPIN:
        maxNumberOfCredentials = 1;
        return true;
    case DlCredentialType::kPin:
        status = Attributes::NumberOfPINUsersSupported::Get(endpointId, &maxNumberOfCredentials);
        break;
    case DlCredentialType::kRfid:
        status = Attributes::NumberOfRFIDUsersSupported::Get(endpointId, &maxNumberOfCredentials);
        break;
    default:
        return false;
    }

    if (EMBER_ZCL_STATUS_SUCCESS != status)
    {
        ChipLogError(
            Zcl, "Unable to read an attribute to get the max number of credentials [endpointId=%d,credentialType=%hhu,status=%d]",
            endpointId, credentialType, status);
        return false;
    }

    return true;
}

DoorLock::DlStatus DoorLockServer::createNewCredentialAndUser(app::CommandHandler * commandObj,
                                                              const app::ConcreteCommandPath & commandPath,
                                                              const Nullable<DoorLock::DlUserStatus> & userStatus,
                                                              const Nullable<DoorLock::DlUserType> & userType,
                                                              const DlCredential & credential,
                                                              const chip::ByteSpan & credentialData, uint16_t & createdUserIndex)
{
    auto endpointId = commandPath.mEndpointId;

    uint16_t availableUserIndex = 0;
    if (!findUnoccupiedUserSlot(endpointId, availableUserIndex))
    {
        emberAfDoorLockClusterPrintln("[SetCredential] Unable to create new user for credential: no available user slots "
                                      "[endpointId=%d,credentialIndex=%d]",
                                      endpointId, credential.CredentialIndex);
        return DlStatus::kOccupied;
    }

    auto status = CreateUser(commandObj, commandPath, availableUserIndex, Nullable<CharSpan>(), Nullable<uint32_t>(), userStatus,
                             userType, Nullable<DlCredentialRule>(), Nullable<DlCredential>(credential));
    if (EMBER_ZCL_STATUS_SUCCESS != status)
    {
        emberAfDoorLockClusterPrintln("[SetCredential] Unable to create new user for credential: internal error "
                                      "[endpointId=%d,credentialIndex=%d,userIndex=%d,status=%d]",
                                      commandPath.mEndpointId, credential.CredentialIndex, availableUserIndex, status);
        // TODO: use appropriate status
        return DlStatus::kFailure;
    }

    if (!emberAfPluginDoorLockSetCredential(commandPath.mEndpointId, credential.CredentialIndex, DlCredentialStatus::kOccupied,
                                            static_cast<DlCredentialType>(credential.CredentialType), credentialData))
    {
        emberAfDoorLockClusterPrintln("[SetCredential] Unable to set the credential: app error "
                                      "[endpointId=%d,credentialIndex=%d,credentialType=%hhu,dataLength=%zu]",
                                      commandPath.mEndpointId, credential.CredentialIndex, credential.CredentialType,
                                      credentialData.size());
        return DlStatus::kFailure;
    }

    emberAfDoorLockClusterPrintln("[SetCredential] Credential and user were created "
                                  "[endpointId=%d,credentialIndex=%d,credentialType=%hhu,dataLength=%zu,userIndex=%d]",
                                  commandPath.mEndpointId, credential.CredentialIndex, credential.CredentialType,
                                  credentialData.size(), availableUserIndex);
    createdUserIndex = availableUserIndex;

    return DlStatus::kSuccess;
}

DoorLock::DlStatus DoorLockServer::createNewCredentialAndAddItToUser(app::CommandHandler * commandObj,
                                                                     const app::ConcreteCommandPath & commandPath,
                                                                     uint16_t userIndex, const DlCredential & credential,
                                                                     const chip::ByteSpan & credentialData)
{
    if (!userIndexValid(commandPath.mEndpointId, userIndex))
    {
        emberAfDoorLockClusterPrintln("[SetCredential] Unable to add new credential to user: user out of bounds "
                                      "[endpointId=%d,credentialIndex=%d,userIndex=%d]",
                                      commandPath.mEndpointId, credential.CredentialIndex, userIndex);
        return DlStatus::kInvalidField;
    }

    EmberAfPluginDoorLockUserInfo user;
    if (!emberAfPluginDoorLockGetUser(commandPath.mEndpointId, userIndex, user))
    {
        emberAfDoorLockClusterPrintln(
            "[SetCredential] Unable to check if credential exists: app error [endpointId=%d,credentialIndex=%d,userIndex=%d]",
            commandPath.mEndpointId, credential.CredentialIndex, userIndex);

        return DlStatus::kFailure;
    }

    // Not in the spec, but common sense: I don't think we need to modify the credential if user slot is not occupied
    if (user.userStatus == DlUserStatus::kAvailable)
    {
        emberAfDoorLockClusterPrintln("[SetCredential] Unable to add credential to user: user clot is empty "
                                      "[endpointId=%d,credentialIndex=%d,userIndex=%d]",
                                      commandPath.mEndpointId, credential.CredentialIndex, userIndex);
        return DlStatus::kInvalidField;
    }

    // Add new credential to the user
    // TODO: Get the modifier fabric here
    auto status = addCredentialToUser(commandPath.mEndpointId, userIndex, credential);
    if (DlStatus::kSuccess != status)
    {
        emberAfDoorLockClusterPrintln("[SetCredential] Unable to add credential to a user: internal error "
                                      "[endpointId=%d,credentialIndex=%d,userIndex=%d,status=%hhu]",
                                      commandPath.mEndpointId, credential.CredentialIndex, userIndex, status);
        return status;
    }

    if (!emberAfPluginDoorLockSetCredential(commandPath.mEndpointId, credential.CredentialIndex, DlCredentialStatus::kOccupied,
                                            static_cast<DlCredentialType>(credential.CredentialType), credentialData))
    {
        emberAfDoorLockClusterPrintln("[SetCredential] Unable to set the credential: app error "
                                      "[endpointId=%d,credentialIndex=%d,credentialType=%hhu,dataLength=%zu]",
                                      commandPath.mEndpointId, credential.CredentialIndex, credential.CredentialType,
                                      credentialData.size());

        // TODO: Use appropriate status code
        return DlStatus::kFailure;
    }

    return DlStatus::kSuccess;
}

bool DoorLockServer::getCredentialRange(chip::EndpointId endpointId, DoorLock::DlCredentialType type, size_t & minSize,
                                        size_t & maxSize)
{
    EmberAfStatus statusMin = EMBER_ZCL_STATUS_SUCCESS, statusMax = EMBER_ZCL_STATUS_SUCCESS;
    uint8_t minLen, maxLen;
    switch (type)
    {
    case DlCredentialType::kProgrammingPIN:
    case DlCredentialType::kPin:
        statusMin = Attributes::MinPINCodeLength::Get(endpointId, &minLen);
        statusMax = Attributes::MaxPINCodeLength::Get(endpointId, &maxLen);
        break;
    case DlCredentialType::kRfid:
        statusMin = Attributes::MinRFIDCodeLength::Get(endpointId, &minLen);
        statusMax = Attributes::MaxRFIDCodeLength::Get(endpointId, &maxLen);
        break;
    default:
        return false;
    }

    if (EMBER_ZCL_STATUS_SUCCESS != statusMin || EMBER_ZCL_STATUS_SUCCESS != statusMax)
    {
        ChipLogError(Zcl, "Unable to read attributes to get min/max length for credentials [endpointId=%d,credentialType=%hhu]",
                     endpointId, type);
        return false;
    }

    minSize = minLen;
    maxSize = maxLen;

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
static EmberAfPluginDoorLockUserInfo gs_users[10];

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

static constexpr size_t DOOR_LOCK_CREDENTIAL_INFO_MAX_DATA_SIZE = 20;

struct CredentialInfo
{
    DlCredentialStatus status;
    DoorLock::DlCredentialType credentialType;
    uint8_t credentialData[DOOR_LOCK_CREDENTIAL_INFO_MAX_DATA_SIZE];
    size_t credentialDataSize;
};

static CredentialInfo gs_credentials[10];

bool emberAfPluginDoorLockGetCredential(chip::EndpointId endpointId, uint16_t credentialIndex,
                                        EmberAfPluginDoorLockCredentialInfo & credential)
{
    auto & credentialInStorage = gs_credentials[credentialIndex];

    credential.status = credentialInStorage.status;
    if (DlCredentialStatus::kAvailable != credential.status)
    {
        credential.credentialType = credentialInStorage.credentialType;
        credential.credentialData = chip::ByteSpan(credentialInStorage.credentialData, credentialInStorage.credentialDataSize);
    }
    return true;
}

bool emberAfPluginDoorLockSetCredential(chip::EndpointId endpointId, uint16_t credentialIndex, DlCredentialStatus credentialStatus,
                                        DoorLock::DlCredentialType credentialType, const chip::ByteSpan & credentialData)
{
    auto & credentialInStorage = gs_credentials[credentialIndex];

    if (credentialData.size() > DOOR_LOCK_CREDENTIAL_INFO_MAX_DATA_SIZE)
    {
        return false;
    }
    credentialInStorage.status         = credentialStatus;
    credentialInStorage.credentialType = credentialType;
    std::memcpy(credentialInStorage.credentialData, credentialData.data(), credentialData.size());

    return true;
}
#endif /* DOOR_LOCK_SERVER_ENABLE_DEFAULT_USERS_CREDENTIALS_IMPLEMENTATION */
