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
inline constexpr DataModel::AttributeEntry kMetadataEntry(LockState::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace LockState
namespace LockType {
inline constexpr DataModel::AttributeEntry kMetadataEntry(LockType::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace LockType
namespace ActuatorEnabled {
inline constexpr DataModel::AttributeEntry kMetadataEntry(ActuatorEnabled::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace ActuatorEnabled
namespace DoorState {
inline constexpr DataModel::AttributeEntry kMetadataEntry(DoorState::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace DoorState
namespace DoorOpenEvents {
inline constexpr DataModel::AttributeEntry kMetadataEntry(DoorOpenEvents::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, Access::Privilege::kManage);
} // namespace DoorOpenEvents
namespace DoorClosedEvents {
inline constexpr DataModel::AttributeEntry kMetadataEntry(DoorClosedEvents::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, Access::Privilege::kManage);
} // namespace DoorClosedEvents
namespace OpenPeriod {
inline constexpr DataModel::AttributeEntry kMetadataEntry(OpenPeriod::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, Access::Privilege::kManage);
} // namespace OpenPeriod
namespace NumberOfTotalUsersSupported {
inline constexpr DataModel::AttributeEntry kMetadataEntry(NumberOfTotalUsersSupported::Id,
                                                          BitFlags<DataModel::AttributeQualityFlags>(), Access::Privilege::kView,
                                                          std::nullopt);
} // namespace NumberOfTotalUsersSupported
namespace NumberOfPINUsersSupported {
inline constexpr DataModel::AttributeEntry kMetadataEntry(NumberOfPINUsersSupported::Id,
                                                          BitFlags<DataModel::AttributeQualityFlags>(), Access::Privilege::kView,
                                                          std::nullopt);
} // namespace NumberOfPINUsersSupported
namespace NumberOfRFIDUsersSupported {
inline constexpr DataModel::AttributeEntry kMetadataEntry(NumberOfRFIDUsersSupported::Id,
                                                          BitFlags<DataModel::AttributeQualityFlags>(), Access::Privilege::kView,
                                                          std::nullopt);
} // namespace NumberOfRFIDUsersSupported
namespace NumberOfWeekDaySchedulesSupportedPerUser {
inline constexpr DataModel::AttributeEntry kMetadataEntry(NumberOfWeekDaySchedulesSupportedPerUser::Id,
                                                          BitFlags<DataModel::AttributeQualityFlags>(), Access::Privilege::kView,
                                                          std::nullopt);
} // namespace NumberOfWeekDaySchedulesSupportedPerUser
namespace NumberOfYearDaySchedulesSupportedPerUser {
inline constexpr DataModel::AttributeEntry kMetadataEntry(NumberOfYearDaySchedulesSupportedPerUser::Id,
                                                          BitFlags<DataModel::AttributeQualityFlags>(), Access::Privilege::kView,
                                                          std::nullopt);
} // namespace NumberOfYearDaySchedulesSupportedPerUser
namespace NumberOfHolidaySchedulesSupported {
inline constexpr DataModel::AttributeEntry kMetadataEntry(NumberOfHolidaySchedulesSupported::Id,
                                                          BitFlags<DataModel::AttributeQualityFlags>(), Access::Privilege::kView,
                                                          std::nullopt);
} // namespace NumberOfHolidaySchedulesSupported
namespace MaxPINCodeLength {
inline constexpr DataModel::AttributeEntry kMetadataEntry(MaxPINCodeLength::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace MaxPINCodeLength
namespace MinPINCodeLength {
inline constexpr DataModel::AttributeEntry kMetadataEntry(MinPINCodeLength::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace MinPINCodeLength
namespace MaxRFIDCodeLength {
inline constexpr DataModel::AttributeEntry kMetadataEntry(MaxRFIDCodeLength::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace MaxRFIDCodeLength
namespace MinRFIDCodeLength {
inline constexpr DataModel::AttributeEntry kMetadataEntry(MinRFIDCodeLength::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace MinRFIDCodeLength
namespace CredentialRulesSupport {
inline constexpr DataModel::AttributeEntry kMetadataEntry(CredentialRulesSupport::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace CredentialRulesSupport
namespace NumberOfCredentialsSupportedPerUser {
inline constexpr DataModel::AttributeEntry kMetadataEntry(NumberOfCredentialsSupportedPerUser::Id,
                                                          BitFlags<DataModel::AttributeQualityFlags>(), Access::Privilege::kView,
                                                          std::nullopt);
} // namespace NumberOfCredentialsSupportedPerUser
namespace Language {
inline constexpr DataModel::AttributeEntry kMetadataEntry(Language::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, Access::Privilege::kManage);
} // namespace Language
namespace LEDSettings {
inline constexpr DataModel::AttributeEntry kMetadataEntry(LEDSettings::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, Access::Privilege::kManage);
} // namespace LEDSettings
namespace AutoRelockTime {
inline constexpr DataModel::AttributeEntry kMetadataEntry(AutoRelockTime::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, Access::Privilege::kManage);
} // namespace AutoRelockTime
namespace SoundVolume {
inline constexpr DataModel::AttributeEntry kMetadataEntry(SoundVolume::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, Access::Privilege::kManage);
} // namespace SoundVolume
namespace OperatingMode {
inline constexpr DataModel::AttributeEntry kMetadataEntry(OperatingMode::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, Access::Privilege::kManage);
} // namespace OperatingMode
namespace SupportedOperatingModes {
inline constexpr DataModel::AttributeEntry kMetadataEntry(SupportedOperatingModes::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace SupportedOperatingModes
namespace DefaultConfigurationRegister {
inline constexpr DataModel::AttributeEntry kMetadataEntry(DefaultConfigurationRegister::Id,
                                                          BitFlags<DataModel::AttributeQualityFlags>(), Access::Privilege::kView,
                                                          std::nullopt);
} // namespace DefaultConfigurationRegister
namespace EnableLocalProgramming {
inline constexpr DataModel::AttributeEntry kMetadataEntry(EnableLocalProgramming::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, Access::Privilege::kAdminister);
} // namespace EnableLocalProgramming
namespace EnableOneTouchLocking {
inline constexpr DataModel::AttributeEntry kMetadataEntry(EnableOneTouchLocking::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, Access::Privilege::kManage);
} // namespace EnableOneTouchLocking
namespace EnableInsideStatusLED {
inline constexpr DataModel::AttributeEntry kMetadataEntry(EnableInsideStatusLED::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, Access::Privilege::kManage);
} // namespace EnableInsideStatusLED
namespace EnablePrivacyModeButton {
inline constexpr DataModel::AttributeEntry kMetadataEntry(EnablePrivacyModeButton::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, Access::Privilege::kManage);
} // namespace EnablePrivacyModeButton
namespace LocalProgrammingFeatures {
inline constexpr DataModel::AttributeEntry kMetadataEntry(LocalProgrammingFeatures::Id,
                                                          BitFlags<DataModel::AttributeQualityFlags>(), Access::Privilege::kView,
                                                          Access::Privilege::kAdminister);
} // namespace LocalProgrammingFeatures
namespace WrongCodeEntryLimit {
inline constexpr DataModel::AttributeEntry kMetadataEntry(WrongCodeEntryLimit::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, Access::Privilege::kAdminister);
} // namespace WrongCodeEntryLimit
namespace UserCodeTemporaryDisableTime {
inline constexpr DataModel::AttributeEntry kMetadataEntry(UserCodeTemporaryDisableTime::Id,
                                                          BitFlags<DataModel::AttributeQualityFlags>(), Access::Privilege::kView,
                                                          Access::Privilege::kAdminister);
} // namespace UserCodeTemporaryDisableTime
namespace SendPINOverTheAir {
inline constexpr DataModel::AttributeEntry kMetadataEntry(SendPINOverTheAir::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, Access::Privilege::kAdminister);
} // namespace SendPINOverTheAir
namespace RequirePINforRemoteOperation {
inline constexpr DataModel::AttributeEntry kMetadataEntry(RequirePINforRemoteOperation::Id,
                                                          BitFlags<DataModel::AttributeQualityFlags>(), Access::Privilege::kView,
                                                          Access::Privilege::kAdminister);
} // namespace RequirePINforRemoteOperation
namespace ExpiringUserTimeout {
inline constexpr DataModel::AttributeEntry kMetadataEntry(ExpiringUserTimeout::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, Access::Privilege::kAdminister);
} // namespace ExpiringUserTimeout
namespace AliroReaderVerificationKey {
inline constexpr DataModel::AttributeEntry kMetadataEntry(AliroReaderVerificationKey::Id,
                                                          BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kAdminister, std::nullopt);
} // namespace AliroReaderVerificationKey
namespace AliroReaderGroupIdentifier {
inline constexpr DataModel::AttributeEntry kMetadataEntry(AliroReaderGroupIdentifier::Id,
                                                          BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kAdminister, std::nullopt);
} // namespace AliroReaderGroupIdentifier
namespace AliroReaderGroupSubIdentifier {
inline constexpr DataModel::AttributeEntry kMetadataEntry(AliroReaderGroupSubIdentifier::Id,
                                                          BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kAdminister, std::nullopt);
} // namespace AliroReaderGroupSubIdentifier
namespace AliroExpeditedTransactionSupportedProtocolVersions {
inline constexpr DataModel::AttributeEntry
    kMetadataEntry(AliroExpeditedTransactionSupportedProtocolVersions::Id,
                   BitFlags<DataModel::AttributeQualityFlags>(DataModel::AttributeQualityFlags::kListAttribute),
                   Access::Privilege::kAdminister, std::nullopt);
} // namespace AliroExpeditedTransactionSupportedProtocolVersions
namespace AliroGroupResolvingKey {
inline constexpr DataModel::AttributeEntry kMetadataEntry(AliroGroupResolvingKey::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kAdminister, std::nullopt);
} // namespace AliroGroupResolvingKey
namespace AliroSupportedBLEUWBProtocolVersions {
inline constexpr DataModel::AttributeEntry
    kMetadataEntry(AliroSupportedBLEUWBProtocolVersions::Id,
                   BitFlags<DataModel::AttributeQualityFlags>(DataModel::AttributeQualityFlags::kListAttribute),
                   Access::Privilege::kAdminister, std::nullopt);
} // namespace AliroSupportedBLEUWBProtocolVersions
namespace AliroBLEAdvertisingVersion {
inline constexpr DataModel::AttributeEntry kMetadataEntry(AliroBLEAdvertisingVersion::Id,
                                                          BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kAdminister, std::nullopt);
} // namespace AliroBLEAdvertisingVersion
namespace NumberOfAliroCredentialIssuerKeysSupported {
inline constexpr DataModel::AttributeEntry kMetadataEntry(NumberOfAliroCredentialIssuerKeysSupported::Id,
                                                          BitFlags<DataModel::AttributeQualityFlags>(), Access::Privilege::kView,
                                                          std::nullopt);
} // namespace NumberOfAliroCredentialIssuerKeysSupported
namespace NumberOfAliroEndpointKeysSupported {
inline constexpr DataModel::AttributeEntry kMetadataEntry(NumberOfAliroEndpointKeysSupported::Id,
                                                          BitFlags<DataModel::AttributeQualityFlags>(), Access::Privilege::kView,
                                                          std::nullopt);
} // namespace NumberOfAliroEndpointKeysSupported

} // namespace Attributes

namespace Commands {
namespace LockDoor {
inline constexpr DataModel::AcceptedCommandEntry
    kMetadataEntry(LockDoor::Id, BitFlags<DataModel::CommandQualityFlags>(DataModel::CommandQualityFlags::kTimed),
                   Access::Privilege::kOperate);
} // namespace LockDoor
namespace UnlockDoor {
inline constexpr DataModel::AcceptedCommandEntry
    kMetadataEntry(UnlockDoor::Id, BitFlags<DataModel::CommandQualityFlags>(DataModel::CommandQualityFlags::kTimed),
                   Access::Privilege::kOperate);
} // namespace UnlockDoor
namespace UnlockWithTimeout {
inline constexpr DataModel::AcceptedCommandEntry
    kMetadataEntry(UnlockWithTimeout::Id, BitFlags<DataModel::CommandQualityFlags>(DataModel::CommandQualityFlags::kTimed),
                   Access::Privilege::kOperate);
} // namespace UnlockWithTimeout
namespace SetWeekDaySchedule {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(SetWeekDaySchedule::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kAdminister);
} // namespace SetWeekDaySchedule
namespace GetWeekDaySchedule {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(GetWeekDaySchedule::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kAdminister);
} // namespace GetWeekDaySchedule
namespace ClearWeekDaySchedule {
inline constexpr DataModel::AcceptedCommandEntry
    kMetadataEntry(ClearWeekDaySchedule::Id, BitFlags<DataModel::CommandQualityFlags>(), Access::Privilege::kAdminister);
} // namespace ClearWeekDaySchedule
namespace SetYearDaySchedule {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(SetYearDaySchedule::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kAdminister);
} // namespace SetYearDaySchedule
namespace GetYearDaySchedule {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(GetYearDaySchedule::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kAdminister);
} // namespace GetYearDaySchedule
namespace ClearYearDaySchedule {
inline constexpr DataModel::AcceptedCommandEntry
    kMetadataEntry(ClearYearDaySchedule::Id, BitFlags<DataModel::CommandQualityFlags>(), Access::Privilege::kAdminister);
} // namespace ClearYearDaySchedule
namespace SetHolidaySchedule {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(SetHolidaySchedule::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kAdminister);
} // namespace SetHolidaySchedule
namespace GetHolidaySchedule {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(GetHolidaySchedule::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kAdminister);
} // namespace GetHolidaySchedule
namespace ClearHolidaySchedule {
inline constexpr DataModel::AcceptedCommandEntry
    kMetadataEntry(ClearHolidaySchedule::Id, BitFlags<DataModel::CommandQualityFlags>(), Access::Privilege::kAdminister);
} // namespace ClearHolidaySchedule
namespace SetUser {
inline constexpr DataModel::AcceptedCommandEntry
    kMetadataEntry(SetUser::Id, BitFlags<DataModel::CommandQualityFlags>(DataModel::CommandQualityFlags::kTimed),
                   Access::Privilege::kAdminister);
} // namespace SetUser
namespace GetUser {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(GetUser::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kAdminister);
} // namespace GetUser
namespace ClearUser {
inline constexpr DataModel::AcceptedCommandEntry
    kMetadataEntry(ClearUser::Id, BitFlags<DataModel::CommandQualityFlags>(DataModel::CommandQualityFlags::kTimed),
                   Access::Privilege::kAdminister);
} // namespace ClearUser
namespace SetCredential {
inline constexpr DataModel::AcceptedCommandEntry
    kMetadataEntry(SetCredential::Id, BitFlags<DataModel::CommandQualityFlags>(DataModel::CommandQualityFlags::kTimed),
                   Access::Privilege::kAdminister);
} // namespace SetCredential
namespace GetCredentialStatus {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(GetCredentialStatus::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kAdminister);
} // namespace GetCredentialStatus
namespace ClearCredential {
inline constexpr DataModel::AcceptedCommandEntry
    kMetadataEntry(ClearCredential::Id, BitFlags<DataModel::CommandQualityFlags>(DataModel::CommandQualityFlags::kTimed),
                   Access::Privilege::kAdminister);
} // namespace ClearCredential
namespace UnboltDoor {
inline constexpr DataModel::AcceptedCommandEntry
    kMetadataEntry(UnboltDoor::Id, BitFlags<DataModel::CommandQualityFlags>(DataModel::CommandQualityFlags::kTimed),
                   Access::Privilege::kOperate);
} // namespace UnboltDoor
namespace SetAliroReaderConfig {
inline constexpr DataModel::AcceptedCommandEntry
    kMetadataEntry(SetAliroReaderConfig::Id, BitFlags<DataModel::CommandQualityFlags>(DataModel::CommandQualityFlags::kTimed),
                   Access::Privilege::kAdminister);
} // namespace SetAliroReaderConfig
namespace ClearAliroReaderConfig {
inline constexpr DataModel::AcceptedCommandEntry
    kMetadataEntry(ClearAliroReaderConfig::Id, BitFlags<DataModel::CommandQualityFlags>(DataModel::CommandQualityFlags::kTimed),
                   Access::Privilege::kAdminister);
} // namespace ClearAliroReaderConfig

} // namespace Commands
} // namespace DoorLock
} // namespace Clusters
} // namespace app
} // namespace chip
