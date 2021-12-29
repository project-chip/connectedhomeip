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

bool DoorLockServer::SetLanguage(chip::EndpointId endpointId, const char * newLanguage)
{
    auto lang = chip::CharSpan(newLanguage, strlen(newLanguage));

    emberAfDoorLockClusterPrintln("Setting Language to '%s'", newLanguage);
    EmberAfStatus status = Attributes::Language::Set(endpointId, lang);

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

// =======================================================

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

bool emberAfDoorLockClusterSetUserCallback(chip::app::CommandHandler * commandObj,
                                           const chip::app::ConcreteCommandPath & commandPath,
                                           const chip::app::Clusters::DoorLock::Commands::SetUser::DecodableType & commandData)
{
    emberAfDoorLockClusterPrintln("Received Set User command (not implemented)");
    // SetUser command fields are:
    // DlDataOperationType operationType;
    // uint16_t userIndex;
    // DataModel::Nullable<chip::CharSpan> userName;
    // DataModel::Nullable<uint32_t> userUniqueId;
    // DlUserStatus userStatus;
    // DlUserType userType;
    // DlCredentialRule credentialRule;

    // TODO: Implement setting the user
    emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_SUCCESS);
    return true;
}

bool emberAfDoorLockClusterGetUserCallback(chip::app::CommandHandler * commandObj,
                                           const chip::app::ConcreteCommandPath & commandPath,
                                           const chip::app::Clusters::DoorLock::Commands::GetUser::DecodableType & commandData)
{
    emberAfDoorLockClusterPrintln("Received Get User command (not implemented)");
    // GetUser command fields are:
    // uint16_t userIndex;

    // TODO: Implement getting the user
    emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_SUCCESS);
    return true;
}

bool emberAfDoorLockClusterClearUserCallback(chip::app::CommandHandler * commandObj,
                                             const chip::app::ConcreteCommandPath & commandPath,
                                             const chip::app::Clusters::DoorLock::Commands::ClearUser::DecodableType & commandData)
{
    emberAfDoorLockClusterPrintln("Received Clear User command (not implemented)");
    // ClearUser command fields are:
    // uint16_t userIndex;

    // TODO: Implement clearing the user
    emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_SUCCESS);
    return true;
}

bool emberAfDoorLockClusterSetCredentialCallback(
    chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
    const chip::app::Clusters::DoorLock::Commands::SetCredential::DecodableType & commandData)
{
    emberAfDoorLockClusterPrintln("Received Set Credential command (not implemented)");
    // SetCredential command fields are:
    // DlDataOperationType operationType;
    // Structs::DlCredential::Type credential;
    // chip::ByteSpan credentialData;
    // uint16_t userIndex;
    // DlUserStatus userStatus;

    // TODO: Implement clearing the user
    emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_SUCCESS);
    return true;
}

bool emberAfDoorLockClusterGetCredentialStatusCallback(
    chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
    const chip::app::Clusters::DoorLock::Commands::GetCredentialStatus::DecodableType & commandData)
{
    emberAfDoorLockClusterPrintln("Received Get Credential Status command (not implemented)");
    // GetCredentialStatus command fields are:
    // Structs::DlCredential::Type credential;

    // TODO: Implement clearing the user
    emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_SUCCESS);
    return true;
}

bool emberAfDoorLockClusterClearCredentialCallback(
    chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
    const chip::app::Clusters::DoorLock::Commands::ClearCredential::DecodableType & commandData)
{
    emberAfDoorLockClusterPrintln("Received Clear Credential command (not implemented)");
    // ClearCredential command fields are:
    // Structs::DlCredential::Type credential;

    // TODO: Implement clearing the user
    emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_SUCCESS);
    return true;
}

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
            char lang[3 + 1] = { 0 };
            memcpy(lang, &value[1], value[0]);
            res = emberAfPluginDoorLockOnLanguageChange(attributePath.mEndpointId, lang);
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
emberAfPluginDoorLockOnLanguageChange(chip::EndpointId EndpointId, const char * newLanguage)
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
