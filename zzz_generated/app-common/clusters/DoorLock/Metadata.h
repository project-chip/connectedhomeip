// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster DoorLock (cluster code: 257/0x101)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/DoorLock/Ids.h>

namespace chip {
namespace app {
namespace Clusters {
namespace DoorLock {

inline constexpr uint32_t kRevision = 7;

namespace Attributes {
namespace LockState {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = LockState::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace LockState
namespace LockType {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = LockType::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace LockType
namespace ActuatorEnabled {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = ActuatorEnabled::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace ActuatorEnabled
namespace DoorState {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = DoorState::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace DoorState
namespace DoorOpenEvents {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = DoorOpenEvents::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kManage,
};
} // namespace DoorOpenEvents
namespace DoorClosedEvents {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = DoorClosedEvents::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kManage,
};
} // namespace DoorClosedEvents
namespace OpenPeriod {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = OpenPeriod::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kManage,
};
} // namespace OpenPeriod
namespace NumberOfTotalUsersSupported {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = NumberOfTotalUsersSupported::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace NumberOfTotalUsersSupported
namespace NumberOfPINUsersSupported {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = NumberOfPINUsersSupported::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace NumberOfPINUsersSupported
namespace NumberOfRFIDUsersSupported {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = NumberOfRFIDUsersSupported::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace NumberOfRFIDUsersSupported
namespace NumberOfWeekDaySchedulesSupportedPerUser {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = NumberOfWeekDaySchedulesSupportedPerUser::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace NumberOfWeekDaySchedulesSupportedPerUser
namespace NumberOfYearDaySchedulesSupportedPerUser {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = NumberOfYearDaySchedulesSupportedPerUser::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace NumberOfYearDaySchedulesSupportedPerUser
namespace NumberOfHolidaySchedulesSupported {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = NumberOfHolidaySchedulesSupported::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace NumberOfHolidaySchedulesSupported
namespace MaxPINCodeLength {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = MaxPINCodeLength::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace MaxPINCodeLength
namespace MinPINCodeLength {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = MinPINCodeLength::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace MinPINCodeLength
namespace MaxRFIDCodeLength {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = MaxRFIDCodeLength::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace MaxRFIDCodeLength
namespace MinRFIDCodeLength {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = MinRFIDCodeLength::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace MinRFIDCodeLength
namespace CredentialRulesSupport {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = CredentialRulesSupport::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace CredentialRulesSupport
namespace NumberOfCredentialsSupportedPerUser {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = NumberOfCredentialsSupportedPerUser::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace NumberOfCredentialsSupportedPerUser
namespace Language {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Language::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kManage,
};
} // namespace Language
namespace LEDSettings {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = LEDSettings::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kManage,
};
} // namespace LEDSettings
namespace AutoRelockTime {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = AutoRelockTime::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kManage,
};
} // namespace AutoRelockTime
namespace SoundVolume {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = SoundVolume::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kManage,
};
} // namespace SoundVolume
namespace OperatingMode {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = OperatingMode::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kManage,
};
} // namespace OperatingMode
namespace SupportedOperatingModes {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = SupportedOperatingModes::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace SupportedOperatingModes
namespace DefaultConfigurationRegister {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = DefaultConfigurationRegister::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace DefaultConfigurationRegister
namespace EnableLocalProgramming {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = EnableLocalProgramming::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kAdminister,
};
} // namespace EnableLocalProgramming
namespace EnableOneTouchLocking {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = EnableOneTouchLocking::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kManage,
};
} // namespace EnableOneTouchLocking
namespace EnableInsideStatusLED {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = EnableInsideStatusLED::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kManage,
};
} // namespace EnableInsideStatusLED
namespace EnablePrivacyModeButton {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = EnablePrivacyModeButton::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kManage,
};
} // namespace EnablePrivacyModeButton
namespace LocalProgrammingFeatures {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = LocalProgrammingFeatures::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kAdminister,
};
} // namespace LocalProgrammingFeatures
namespace WrongCodeEntryLimit {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = WrongCodeEntryLimit::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kAdminister,
};
} // namespace WrongCodeEntryLimit
namespace UserCodeTemporaryDisableTime {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = UserCodeTemporaryDisableTime::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kAdminister,
};
} // namespace UserCodeTemporaryDisableTime
namespace SendPINOverTheAir {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = SendPINOverTheAir::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kAdminister,
};
} // namespace SendPINOverTheAir
namespace RequirePINforRemoteOperation {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = RequirePINforRemoteOperation::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kAdminister,
};
} // namespace RequirePINforRemoteOperation
namespace ExpiringUserTimeout {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = ExpiringUserTimeout::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kAdminister,
};
} // namespace ExpiringUserTimeout
namespace AliroReaderVerificationKey {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = AliroReaderVerificationKey::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kAdminister,
    .writePrivilege = std::nullopt,
};
} // namespace AliroReaderVerificationKey
namespace AliroReaderGroupIdentifier {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = AliroReaderGroupIdentifier::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kAdminister,
    .writePrivilege = std::nullopt,
};
} // namespace AliroReaderGroupIdentifier
namespace AliroReaderGroupSubIdentifier {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = AliroReaderGroupSubIdentifier::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kAdminister,
    .writePrivilege = std::nullopt,
};
} // namespace AliroReaderGroupSubIdentifier
namespace AliroExpeditedTransactionSupportedProtocolVersions {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = AliroExpeditedTransactionSupportedProtocolVersions::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kAdminister,
    .writePrivilege = std::nullopt,
};
} // namespace AliroExpeditedTransactionSupportedProtocolVersions
namespace AliroGroupResolvingKey {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = AliroGroupResolvingKey::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kAdminister,
    .writePrivilege = std::nullopt,
};
} // namespace AliroGroupResolvingKey
namespace AliroSupportedBLEUWBProtocolVersions {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = AliroSupportedBLEUWBProtocolVersions::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kAdminister,
    .writePrivilege = std::nullopt,
};
} // namespace AliroSupportedBLEUWBProtocolVersions
namespace AliroBLEAdvertisingVersion {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = AliroBLEAdvertisingVersion::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kAdminister,
    .writePrivilege = std::nullopt,
};
} // namespace AliroBLEAdvertisingVersion
namespace NumberOfAliroCredentialIssuerKeysSupported {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = NumberOfAliroCredentialIssuerKeysSupported::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace NumberOfAliroCredentialIssuerKeysSupported
namespace NumberOfAliroEndpointKeysSupported {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = NumberOfAliroEndpointKeysSupported::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace NumberOfAliroEndpointKeysSupported

} // namespace Attributes

namespace Commands {
namespace LockDoor {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = LockDoor::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{ DataModel::CommandQualityFlags::kTimed },
    .invokePrivilege = Access::Privilege::kOperate,
};
} // namespace LockDoor
namespace UnlockDoor {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = UnlockDoor::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{ DataModel::CommandQualityFlags::kTimed },
    .invokePrivilege = Access::Privilege::kOperate,
};
} // namespace UnlockDoor
namespace UnlockWithTimeout {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = UnlockWithTimeout::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{ DataModel::CommandQualityFlags::kTimed },
    .invokePrivilege = Access::Privilege::kOperate,
};
} // namespace UnlockWithTimeout
namespace SetWeekDaySchedule {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = SetWeekDaySchedule::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kAdminister,
};
} // namespace SetWeekDaySchedule
namespace GetWeekDaySchedule {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = GetWeekDaySchedule::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kAdminister,
};
} // namespace GetWeekDaySchedule
namespace ClearWeekDaySchedule {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = ClearWeekDaySchedule::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kAdminister,
};
} // namespace ClearWeekDaySchedule
namespace SetYearDaySchedule {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = SetYearDaySchedule::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kAdminister,
};
} // namespace SetYearDaySchedule
namespace GetYearDaySchedule {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = GetYearDaySchedule::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kAdminister,
};
} // namespace GetYearDaySchedule
namespace ClearYearDaySchedule {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = ClearYearDaySchedule::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kAdminister,
};
} // namespace ClearYearDaySchedule
namespace SetHolidaySchedule {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = SetHolidaySchedule::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kAdminister,
};
} // namespace SetHolidaySchedule
namespace GetHolidaySchedule {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = GetHolidaySchedule::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kAdminister,
};
} // namespace GetHolidaySchedule
namespace ClearHolidaySchedule {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = ClearHolidaySchedule::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kAdminister,
};
} // namespace ClearHolidaySchedule
namespace SetUser {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = SetUser::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{ DataModel::CommandQualityFlags::kTimed },
    .invokePrivilege = Access::Privilege::kAdminister,
};
} // namespace SetUser
namespace GetUser {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = GetUser::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kAdminister,
};
} // namespace GetUser
namespace ClearUser {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = ClearUser::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{ DataModel::CommandQualityFlags::kTimed },
    .invokePrivilege = Access::Privilege::kAdminister,
};
} // namespace ClearUser
namespace SetCredential {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = SetCredential::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{ DataModel::CommandQualityFlags::kTimed },
    .invokePrivilege = Access::Privilege::kAdminister,
};
} // namespace SetCredential
namespace GetCredentialStatus {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = GetCredentialStatus::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kAdminister,
};
} // namespace GetCredentialStatus
namespace ClearCredential {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = ClearCredential::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{ DataModel::CommandQualityFlags::kTimed },
    .invokePrivilege = Access::Privilege::kAdminister,
};
} // namespace ClearCredential
namespace UnboltDoor {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = UnboltDoor::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{ DataModel::CommandQualityFlags::kTimed },
    .invokePrivilege = Access::Privilege::kOperate,
};
} // namespace UnboltDoor
namespace SetAliroReaderConfig {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = SetAliroReaderConfig::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{ DataModel::CommandQualityFlags::kTimed },
    .invokePrivilege = Access::Privilege::kAdminister,
};
} // namespace SetAliroReaderConfig
namespace ClearAliroReaderConfig {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = ClearAliroReaderConfig::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{ DataModel::CommandQualityFlags::kTimed },
    .invokePrivilege = Access::Privilege::kAdminister,
};
} // namespace ClearAliroReaderConfig

} // namespace Commands
} // namespace DoorLock
} // namespace Clusters
} // namespace app
} // namespace chip
