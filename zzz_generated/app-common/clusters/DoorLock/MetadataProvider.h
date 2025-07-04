// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster DoorLock (cluster code: 257/0x101)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <optional>

#include <app/data-model-provider/ClusterMetadataProvider.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <clusters/DoorLock/Ids.h>
#include <clusters/DoorLock/Metadata.h>

namespace chip {
namespace app {
namespace DataModel {

template <>
struct ClusterMetadataProvider<DataModel::AttributeEntry, Clusters::DoorLock::Id>
{
    static constexpr std::optional<DataModel::AttributeEntry> EntryFor(AttributeId attributeId)
    {
        using namespace Clusters::DoorLock::Attributes;
        switch (attributeId)
        {
        case LockState::Id:
            return LockState::kMetadataEntry;
        case LockType::Id:
            return LockType::kMetadataEntry;
        case ActuatorEnabled::Id:
            return ActuatorEnabled::kMetadataEntry;
        case DoorState::Id:
            return DoorState::kMetadataEntry;
        case DoorOpenEvents::Id:
            return DoorOpenEvents::kMetadataEntry;
        case DoorClosedEvents::Id:
            return DoorClosedEvents::kMetadataEntry;
        case OpenPeriod::Id:
            return OpenPeriod::kMetadataEntry;
        case NumberOfTotalUsersSupported::Id:
            return NumberOfTotalUsersSupported::kMetadataEntry;
        case NumberOfPINUsersSupported::Id:
            return NumberOfPINUsersSupported::kMetadataEntry;
        case NumberOfRFIDUsersSupported::Id:
            return NumberOfRFIDUsersSupported::kMetadataEntry;
        case NumberOfWeekDaySchedulesSupportedPerUser::Id:
            return NumberOfWeekDaySchedulesSupportedPerUser::kMetadataEntry;
        case NumberOfYearDaySchedulesSupportedPerUser::Id:
            return NumberOfYearDaySchedulesSupportedPerUser::kMetadataEntry;
        case NumberOfHolidaySchedulesSupported::Id:
            return NumberOfHolidaySchedulesSupported::kMetadataEntry;
        case MaxPINCodeLength::Id:
            return MaxPINCodeLength::kMetadataEntry;
        case MinPINCodeLength::Id:
            return MinPINCodeLength::kMetadataEntry;
        case MaxRFIDCodeLength::Id:
            return MaxRFIDCodeLength::kMetadataEntry;
        case MinRFIDCodeLength::Id:
            return MinRFIDCodeLength::kMetadataEntry;
        case CredentialRulesSupport::Id:
            return CredentialRulesSupport::kMetadataEntry;
        case NumberOfCredentialsSupportedPerUser::Id:
            return NumberOfCredentialsSupportedPerUser::kMetadataEntry;
        case Language::Id:
            return Language::kMetadataEntry;
        case LEDSettings::Id:
            return LEDSettings::kMetadataEntry;
        case AutoRelockTime::Id:
            return AutoRelockTime::kMetadataEntry;
        case SoundVolume::Id:
            return SoundVolume::kMetadataEntry;
        case OperatingMode::Id:
            return OperatingMode::kMetadataEntry;
        case SupportedOperatingModes::Id:
            return SupportedOperatingModes::kMetadataEntry;
        case DefaultConfigurationRegister::Id:
            return DefaultConfigurationRegister::kMetadataEntry;
        case EnableLocalProgramming::Id:
            return EnableLocalProgramming::kMetadataEntry;
        case EnableOneTouchLocking::Id:
            return EnableOneTouchLocking::kMetadataEntry;
        case EnableInsideStatusLED::Id:
            return EnableInsideStatusLED::kMetadataEntry;
        case EnablePrivacyModeButton::Id:
            return EnablePrivacyModeButton::kMetadataEntry;
        case LocalProgrammingFeatures::Id:
            return LocalProgrammingFeatures::kMetadataEntry;
        case WrongCodeEntryLimit::Id:
            return WrongCodeEntryLimit::kMetadataEntry;
        case UserCodeTemporaryDisableTime::Id:
            return UserCodeTemporaryDisableTime::kMetadataEntry;
        case SendPINOverTheAir::Id:
            return SendPINOverTheAir::kMetadataEntry;
        case RequirePINforRemoteOperation::Id:
            return RequirePINforRemoteOperation::kMetadataEntry;
        case ExpiringUserTimeout::Id:
            return ExpiringUserTimeout::kMetadataEntry;
        case AliroReaderVerificationKey::Id:
            return AliroReaderVerificationKey::kMetadataEntry;
        case AliroReaderGroupIdentifier::Id:
            return AliroReaderGroupIdentifier::kMetadataEntry;
        case AliroReaderGroupSubIdentifier::Id:
            return AliroReaderGroupSubIdentifier::kMetadataEntry;
        case AliroExpeditedTransactionSupportedProtocolVersions::Id:
            return AliroExpeditedTransactionSupportedProtocolVersions::kMetadataEntry;
        case AliroGroupResolvingKey::Id:
            return AliroGroupResolvingKey::kMetadataEntry;
        case AliroSupportedBLEUWBProtocolVersions::Id:
            return AliroSupportedBLEUWBProtocolVersions::kMetadataEntry;
        case AliroBLEAdvertisingVersion::Id:
            return AliroBLEAdvertisingVersion::kMetadataEntry;
        case NumberOfAliroCredentialIssuerKeysSupported::Id:
            return NumberOfAliroCredentialIssuerKeysSupported::kMetadataEntry;
        case NumberOfAliroEndpointKeysSupported::Id:
            return NumberOfAliroEndpointKeysSupported::kMetadataEntry;
        default:
            return std::nullopt;
        }
    }
};

template <>
struct ClusterMetadataProvider<DataModel::AcceptedCommandEntry, Clusters::DoorLock::Id>
{
    static constexpr std::optional<DataModel::AcceptedCommandEntry> EntryFor(CommandId commandId)
    {
        using namespace Clusters::DoorLock::Commands;
        switch (commandId)
        {
        case LockDoor::Id:
            return LockDoor::kMetadataEntry;
        case UnlockDoor::Id:
            return UnlockDoor::kMetadataEntry;
        case UnlockWithTimeout::Id:
            return UnlockWithTimeout::kMetadataEntry;
        case SetWeekDaySchedule::Id:
            return SetWeekDaySchedule::kMetadataEntry;
        case GetWeekDaySchedule::Id:
            return GetWeekDaySchedule::kMetadataEntry;
        case ClearWeekDaySchedule::Id:
            return ClearWeekDaySchedule::kMetadataEntry;
        case SetYearDaySchedule::Id:
            return SetYearDaySchedule::kMetadataEntry;
        case GetYearDaySchedule::Id:
            return GetYearDaySchedule::kMetadataEntry;
        case ClearYearDaySchedule::Id:
            return ClearYearDaySchedule::kMetadataEntry;
        case SetHolidaySchedule::Id:
            return SetHolidaySchedule::kMetadataEntry;
        case GetHolidaySchedule::Id:
            return GetHolidaySchedule::kMetadataEntry;
        case ClearHolidaySchedule::Id:
            return ClearHolidaySchedule::kMetadataEntry;
        case SetUser::Id:
            return SetUser::kMetadataEntry;
        case GetUser::Id:
            return GetUser::kMetadataEntry;
        case ClearUser::Id:
            return ClearUser::kMetadataEntry;
        case SetCredential::Id:
            return SetCredential::kMetadataEntry;
        case GetCredentialStatus::Id:
            return GetCredentialStatus::kMetadataEntry;
        case ClearCredential::Id:
            return ClearCredential::kMetadataEntry;
        case UnboltDoor::Id:
            return UnboltDoor::kMetadataEntry;
        case SetAliroReaderConfig::Id:
            return SetAliroReaderConfig::kMetadataEntry;
        case ClearAliroReaderConfig::Id:
            return ClearAliroReaderConfig::kMetadataEntry;

        default:
            return std::nullopt;
        }
    }
};

} // namespace DataModel
} // namespace app
} // namespace chip
