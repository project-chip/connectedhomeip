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
#include <app/EventLogging.h>
#include <app/server/Server.h>
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

// =============================================================================
// 'Default' callbacks for cluster commands
// =============================================================================

bool __attribute__((weak))
emberAfPluginDoorLockOnDoorLockCommand(chip::EndpointId endpointId, const Nullable<chip::FabricIndex> & fabricIdx,
                                       const Nullable<chip::NodeId> & nodeId, const Optional<ByteSpan> & pinCode,
                                       OperationErrorEnum & err)
{
    err = OperationErrorEnum::kUnspecified;
    return false;
}

bool __attribute__((weak))
emberAfPluginDoorLockOnDoorUnlockCommand(chip::EndpointId endpointId, const Nullable<chip::FabricIndex> & fabricIdx,
                                         const Nullable<chip::NodeId> & nodeId, const Optional<ByteSpan> & pinCode,
                                         OperationErrorEnum & err)
{
    err = OperationErrorEnum::kUnspecified;
    return false;
}

bool __attribute__((weak))
emberAfPluginDoorLockOnDoorUnboltCommand(chip::EndpointId endpointId, const Nullable<chip::FabricIndex> & fabricIdx,
                                         const Nullable<chip::NodeId> & nodeId, const Optional<ByteSpan> & pinCode,
                                         OperationErrorEnum & err)
{
    err = OperationErrorEnum::kUnspecified;
    return false;
}

void __attribute__((weak)) emberAfPluginDoorLockOnAutoRelock(chip::EndpointId endpointId) {}

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
emberAfPluginDoorLockOnUnhandledAttributeChange(chip::EndpointId EndpointId, const chip::app::ConcreteAttributePath & attributePath,
                                                EmberAfAttributeType attrType, uint16_t attrSize, uint8_t * attrValue)
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
                             const chip::CharSpan & userName, uint32_t uniqueId, UserStatusEnum userStatus, UserTypeEnum usertype,
                             CredentialRuleEnum credentialRule, const CredentialStruct * credentials, size_t totalCredentials)
{
    return false;
}

bool __attribute__((weak))
emberAfPluginDoorLockGetCredential(chip::EndpointId endpointId, uint16_t credentialIndex, CredentialTypeEnum credentialType,
                                   EmberAfPluginDoorLockCredentialInfo & credential)
{
    return false;
}

bool __attribute__((weak))
emberAfPluginDoorLockSetCredential(chip::EndpointId endpointId, uint16_t credentialIndex, chip::FabricIndex creator,
                                   chip::FabricIndex modifier, DlCredentialStatus credentialStatus,
                                   CredentialTypeEnum credentialType, const chip::ByteSpan & credentialData)
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
emberAfPluginDoorLockGetSchedule(chip::EndpointId endpointId, uint8_t holidayIndex, EmberAfPluginDoorLockHolidaySchedule & schedule)
{
    return DlStatus::kFailure;
}

DlStatus __attribute__((weak))
emberAfPluginDoorLockSetSchedule(chip::EndpointId endpointId, uint8_t weekdayIndex, uint16_t userIndex, DlScheduleStatus status,
                                 DaysMaskMap daysMask, uint8_t startHour, uint8_t startMinute, uint8_t endHour, uint8_t endMinute)
{
    return DlStatus::kFailure;
}

DlStatus __attribute__((weak))
emberAfPluginDoorLockSetSchedule(chip::EndpointId endpointId, uint8_t yearDayIndex, uint16_t userIndex, DlScheduleStatus status,
                                 uint32_t localStartTime, uint32_t localEndTime)
{
    return DlStatus::kFailure;
}

DlStatus __attribute__((weak))
emberAfPluginDoorLockSetSchedule(chip::EndpointId endpointId, uint8_t holidayIndex, DlScheduleStatus status,
                                 uint32_t localStartTime, uint32_t localEndTime, OperatingModeEnum operatingMode)
{
    return DlStatus::kFailure;
}

void __attribute__((weak))
emberAfPluginDoorLockLockoutStarted(chip::EndpointId endpointId, chip::System::Clock::Timestamp lockoutEndTime)
{}

bool __attribute__((weak))
emberAfPluginDoorLockGetNumberOfFingerprintCredentialsSupported(chip::EndpointId endpointId, uint16_t & maxNumberOfCredentials)
{
    return false;
}

bool __attribute__((weak))
emberAfPluginDoorLockGetNumberOfFingerVeinCredentialsSupported(chip::EndpointId endpointId, uint16_t & maxNumberOfCredentials)
{
    return false;
}

bool __attribute__((weak))
emberAfPluginDoorLockGetNumberOfFaceCredentialsSupported(chip::EndpointId endpointId, uint16_t & maxNumberOfCredentials)
{
    return false;
}

bool __attribute__((weak))
emberAfPluginDoorLockGetFingerprintCredentialLengthConstraints(chip::EndpointId endpointId, uint8_t & minLen, uint8_t & maxLen)
{
    return false;
}

bool __attribute__((weak))
emberAfPluginDoorLockGetFingerVeinCredentialLengthConstraints(chip::EndpointId endpointId, uint8_t & minLen, uint8_t & maxLen)
{
    return false;
}

bool __attribute__((weak))
emberAfPluginDoorLockGetFaceCredentialLengthConstraints(chip::EndpointId endpointId, uint8_t & minLen, uint8_t & maxLen)
{
    return false;
}
