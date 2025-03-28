// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster DoorLock (cluster code: 257/0x101)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/DoorLock/DoorLockIds.h>

namespace chip {
namespace app {
namespace clusters {
namespace DoorLock {
namespace Metadata {

inline constexpr uint32_t kRevision = 7;

namespace Attributes {

inline constexpr DataModel::AttributeEntry kLockStateEntry = {
    .attributeId    = DoorLock::Attributes::LockState::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kLockTypeEntry = {
    .attributeId    = DoorLock::Attributes::LockType::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kActuatorEnabledEntry = {
    .attributeId    = DoorLock::Attributes::ActuatorEnabled::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kDoorStateEntry = {
    .attributeId    = DoorLock::Attributes::DoorState::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kDoorOpenEventsEntry = {
    .attributeId    = DoorLock::Attributes::DoorOpenEvents::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kManage,
};
inline constexpr DataModel::AttributeEntry kDoorClosedEventsEntry = {
    .attributeId    = DoorLock::Attributes::DoorClosedEvents::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kManage,
};
inline constexpr DataModel::AttributeEntry kOpenPeriodEntry = {
    .attributeId    = DoorLock::Attributes::OpenPeriod::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kManage,
};
inline constexpr DataModel::AttributeEntry kNumberOfTotalUsersSupportedEntry = {
    .attributeId    = DoorLock::Attributes::NumberOfTotalUsersSupported::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kNumberOfPINUsersSupportedEntry = {
    .attributeId    = DoorLock::Attributes::NumberOfPINUsersSupported::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kNumberOfRFIDUsersSupportedEntry = {
    .attributeId    = DoorLock::Attributes::NumberOfRFIDUsersSupported::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kNumberOfWeekDaySchedulesSupportedPerUserEntry = {
    .attributeId    = DoorLock::Attributes::NumberOfWeekDaySchedulesSupportedPerUser::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kNumberOfYearDaySchedulesSupportedPerUserEntry = {
    .attributeId    = DoorLock::Attributes::NumberOfYearDaySchedulesSupportedPerUser::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kNumberOfHolidaySchedulesSupportedEntry = {
    .attributeId    = DoorLock::Attributes::NumberOfHolidaySchedulesSupported::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kMaxPINCodeLengthEntry = {
    .attributeId    = DoorLock::Attributes::MaxPINCodeLength::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kMinPINCodeLengthEntry = {
    .attributeId    = DoorLock::Attributes::MinPINCodeLength::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kMaxRFIDCodeLengthEntry = {
    .attributeId    = DoorLock::Attributes::MaxRFIDCodeLength::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kMinRFIDCodeLengthEntry = {
    .attributeId    = DoorLock::Attributes::MinRFIDCodeLength::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kCredentialRulesSupportEntry = {
    .attributeId    = DoorLock::Attributes::CredentialRulesSupport::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kNumberOfCredentialsSupportedPerUserEntry = {
    .attributeId    = DoorLock::Attributes::NumberOfCredentialsSupportedPerUser::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kLanguageEntry = {
    .attributeId    = DoorLock::Attributes::Language::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kManage,
};
inline constexpr DataModel::AttributeEntry kLEDSettingsEntry = {
    .attributeId    = DoorLock::Attributes::LEDSettings::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kManage,
};
inline constexpr DataModel::AttributeEntry kAutoRelockTimeEntry = {
    .attributeId    = DoorLock::Attributes::AutoRelockTime::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kManage,
};
inline constexpr DataModel::AttributeEntry kSoundVolumeEntry = {
    .attributeId    = DoorLock::Attributes::SoundVolume::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kManage,
};
inline constexpr DataModel::AttributeEntry kOperatingModeEntry = {
    .attributeId    = DoorLock::Attributes::OperatingMode::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kManage,
};
inline constexpr DataModel::AttributeEntry kSupportedOperatingModesEntry = {
    .attributeId    = DoorLock::Attributes::SupportedOperatingModes::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kDefaultConfigurationRegisterEntry = {
    .attributeId    = DoorLock::Attributes::DefaultConfigurationRegister::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kEnableLocalProgrammingEntry = {
    .attributeId    = DoorLock::Attributes::EnableLocalProgramming::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kAdminister,
};
inline constexpr DataModel::AttributeEntry kEnableOneTouchLockingEntry = {
    .attributeId    = DoorLock::Attributes::EnableOneTouchLocking::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kManage,
};
inline constexpr DataModel::AttributeEntry kEnableInsideStatusLEDEntry = {
    .attributeId    = DoorLock::Attributes::EnableInsideStatusLED::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kManage,
};
inline constexpr DataModel::AttributeEntry kEnablePrivacyModeButtonEntry = {
    .attributeId    = DoorLock::Attributes::EnablePrivacyModeButton::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kManage,
};
inline constexpr DataModel::AttributeEntry kLocalProgrammingFeaturesEntry = {
    .attributeId    = DoorLock::Attributes::LocalProgrammingFeatures::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kAdminister,
};
inline constexpr DataModel::AttributeEntry kWrongCodeEntryLimitEntry = {
    .attributeId    = DoorLock::Attributes::WrongCodeEntryLimit::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kAdminister,
};
inline constexpr DataModel::AttributeEntry kUserCodeTemporaryDisableTimeEntry = {
    .attributeId    = DoorLock::Attributes::UserCodeTemporaryDisableTime::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kAdminister,
};
inline constexpr DataModel::AttributeEntry kSendPINOverTheAirEntry = {
    .attributeId    = DoorLock::Attributes::SendPINOverTheAir::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kAdminister,
};
inline constexpr DataModel::AttributeEntry kRequirePINforRemoteOperationEntry = {
    .attributeId    = DoorLock::Attributes::RequirePINforRemoteOperation::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kAdminister,
};
inline constexpr DataModel::AttributeEntry kExpiringUserTimeoutEntry = {
    .attributeId    = DoorLock::Attributes::ExpiringUserTimeout::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kAdminister,
};
inline constexpr DataModel::AttributeEntry kAliroReaderVerificationKeyEntry = {
    .attributeId    = DoorLock::Attributes::AliroReaderVerificationKey::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kAdminister,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kAliroReaderGroupIdentifierEntry = {
    .attributeId    = DoorLock::Attributes::AliroReaderGroupIdentifier::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kAdminister,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kAliroReaderGroupSubIdentifierEntry = {
    .attributeId    = DoorLock::Attributes::AliroReaderGroupSubIdentifier::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kAdminister,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kAliroExpeditedTransactionSupportedProtocolVersionsEntry = {
    .attributeId    = DoorLock::Attributes::AliroExpeditedTransactionSupportedProtocolVersions::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kAdminister,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kAliroGroupResolvingKeyEntry = {
    .attributeId    = DoorLock::Attributes::AliroGroupResolvingKey::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kAdminister,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kAliroSupportedBLEUWBProtocolVersionsEntry = {
    .attributeId    = DoorLock::Attributes::AliroSupportedBLEUWBProtocolVersions::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kAdminister,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kAliroBLEAdvertisingVersionEntry = {
    .attributeId    = DoorLock::Attributes::AliroBLEAdvertisingVersion::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kAdminister,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kNumberOfAliroCredentialIssuerKeysSupportedEntry = {
    .attributeId    = DoorLock::Attributes::NumberOfAliroCredentialIssuerKeysSupported::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kNumberOfAliroEndpointKeysSupportedEntry = {
    .attributeId    = DoorLock::Attributes::NumberOfAliroEndpointKeysSupported::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};

} // namespace Attributes

namespace Commands {

inline constexpr DataModel::AcceptedCommandEntry kLockDoorEntry = {
    .commandId       = DoorLock::Commands::LockDoor::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{ DataModel::CommandQualityFlags::kTimed },
    .invokePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AcceptedCommandEntry kUnlockDoorEntry = {
    .commandId       = DoorLock::Commands::UnlockDoor::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{ DataModel::CommandQualityFlags::kTimed },
    .invokePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AcceptedCommandEntry kUnlockWithTimeoutEntry = {
    .commandId       = DoorLock::Commands::UnlockWithTimeout::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{ DataModel::CommandQualityFlags::kTimed },
    .invokePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AcceptedCommandEntry kSetWeekDayScheduleEntry = {
    .commandId       = DoorLock::Commands::SetWeekDaySchedule::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kAdminister,
};
inline constexpr DataModel::AcceptedCommandEntry kGetWeekDayScheduleEntry = {
    .commandId       = DoorLock::Commands::GetWeekDaySchedule::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kAdminister,
};
inline constexpr DataModel::AcceptedCommandEntry kClearWeekDayScheduleEntry = {
    .commandId       = DoorLock::Commands::ClearWeekDaySchedule::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kAdminister,
};
inline constexpr DataModel::AcceptedCommandEntry kSetYearDayScheduleEntry = {
    .commandId       = DoorLock::Commands::SetYearDaySchedule::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kAdminister,
};
inline constexpr DataModel::AcceptedCommandEntry kGetYearDayScheduleEntry = {
    .commandId       = DoorLock::Commands::GetYearDaySchedule::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kAdminister,
};
inline constexpr DataModel::AcceptedCommandEntry kClearYearDayScheduleEntry = {
    .commandId       = DoorLock::Commands::ClearYearDaySchedule::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kAdminister,
};
inline constexpr DataModel::AcceptedCommandEntry kSetHolidayScheduleEntry = {
    .commandId       = DoorLock::Commands::SetHolidaySchedule::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kAdminister,
};
inline constexpr DataModel::AcceptedCommandEntry kGetHolidayScheduleEntry = {
    .commandId       = DoorLock::Commands::GetHolidaySchedule::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kAdminister,
};
inline constexpr DataModel::AcceptedCommandEntry kClearHolidayScheduleEntry = {
    .commandId       = DoorLock::Commands::ClearHolidaySchedule::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kAdminister,
};
inline constexpr DataModel::AcceptedCommandEntry kSetUserEntry = {
    .commandId       = DoorLock::Commands::SetUser::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{ DataModel::CommandQualityFlags::kTimed },
    .invokePrivilege = Access::Privilege::kAdminister,
};
inline constexpr DataModel::AcceptedCommandEntry kGetUserEntry = {
    .commandId       = DoorLock::Commands::GetUser::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kAdminister,
};
inline constexpr DataModel::AcceptedCommandEntry kClearUserEntry = {
    .commandId       = DoorLock::Commands::ClearUser::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{ DataModel::CommandQualityFlags::kTimed },
    .invokePrivilege = Access::Privilege::kAdminister,
};
inline constexpr DataModel::AcceptedCommandEntry kSetCredentialEntry = {
    .commandId       = DoorLock::Commands::SetCredential::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{ DataModel::CommandQualityFlags::kTimed },
    .invokePrivilege = Access::Privilege::kAdminister,
};
inline constexpr DataModel::AcceptedCommandEntry kGetCredentialStatusEntry = {
    .commandId       = DoorLock::Commands::GetCredentialStatus::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kAdminister,
};
inline constexpr DataModel::AcceptedCommandEntry kClearCredentialEntry = {
    .commandId       = DoorLock::Commands::ClearCredential::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{ DataModel::CommandQualityFlags::kTimed },
    .invokePrivilege = Access::Privilege::kAdminister,
};
inline constexpr DataModel::AcceptedCommandEntry kUnboltDoorEntry = {
    .commandId       = DoorLock::Commands::UnboltDoor::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{ DataModel::CommandQualityFlags::kTimed },
    .invokePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AcceptedCommandEntry kSetAliroReaderConfigEntry = {
    .commandId       = DoorLock::Commands::SetAliroReaderConfig::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{ DataModel::CommandQualityFlags::kTimed },
    .invokePrivilege = Access::Privilege::kAdminister,
};
inline constexpr DataModel::AcceptedCommandEntry kClearAliroReaderConfigEntry = {
    .commandId       = DoorLock::Commands::ClearAliroReaderConfig::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{ DataModel::CommandQualityFlags::kTimed },
    .invokePrivilege = Access::Privilege::kAdminister,
};

} // namespace Commands
} // namespace Metadata
} // namespace DoorLock
} // namespace clusters
} // namespace app
} // namespace chip
