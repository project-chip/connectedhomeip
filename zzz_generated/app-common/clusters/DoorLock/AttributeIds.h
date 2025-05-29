// DO NOT EDIT MANUALLY - Generated file
//
// Identifier constant values for cluster DoorLock (cluster code: 257/0x101)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <clusters/shared/GlobalIds.h>
#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace Clusters {
namespace DoorLock {
namespace Attributes {
namespace LockState {
inline constexpr AttributeId Id = 0x00000000;
} // namespace LockState

namespace LockType {
inline constexpr AttributeId Id = 0x00000001;
} // namespace LockType

namespace ActuatorEnabled {
inline constexpr AttributeId Id = 0x00000002;
} // namespace ActuatorEnabled

namespace DoorState {
inline constexpr AttributeId Id = 0x00000003;
} // namespace DoorState

namespace DoorOpenEvents {
inline constexpr AttributeId Id = 0x00000004;
} // namespace DoorOpenEvents

namespace DoorClosedEvents {
inline constexpr AttributeId Id = 0x00000005;
} // namespace DoorClosedEvents

namespace OpenPeriod {
inline constexpr AttributeId Id = 0x00000006;
} // namespace OpenPeriod

namespace NumberOfTotalUsersSupported {
inline constexpr AttributeId Id = 0x00000011;
} // namespace NumberOfTotalUsersSupported

namespace NumberOfPINUsersSupported {
inline constexpr AttributeId Id = 0x00000012;
} // namespace NumberOfPINUsersSupported

namespace NumberOfRFIDUsersSupported {
inline constexpr AttributeId Id = 0x00000013;
} // namespace NumberOfRFIDUsersSupported

namespace NumberOfWeekDaySchedulesSupportedPerUser {
inline constexpr AttributeId Id = 0x00000014;
} // namespace NumberOfWeekDaySchedulesSupportedPerUser

namespace NumberOfYearDaySchedulesSupportedPerUser {
inline constexpr AttributeId Id = 0x00000015;
} // namespace NumberOfYearDaySchedulesSupportedPerUser

namespace NumberOfHolidaySchedulesSupported {
inline constexpr AttributeId Id = 0x00000016;
} // namespace NumberOfHolidaySchedulesSupported

namespace MaxPINCodeLength {
inline constexpr AttributeId Id = 0x00000017;
} // namespace MaxPINCodeLength

namespace MinPINCodeLength {
inline constexpr AttributeId Id = 0x00000018;
} // namespace MinPINCodeLength

namespace MaxRFIDCodeLength {
inline constexpr AttributeId Id = 0x00000019;
} // namespace MaxRFIDCodeLength

namespace MinRFIDCodeLength {
inline constexpr AttributeId Id = 0x0000001A;
} // namespace MinRFIDCodeLength

namespace CredentialRulesSupport {
inline constexpr AttributeId Id = 0x0000001B;
} // namespace CredentialRulesSupport

namespace NumberOfCredentialsSupportedPerUser {
inline constexpr AttributeId Id = 0x0000001C;
} // namespace NumberOfCredentialsSupportedPerUser

namespace Language {
inline constexpr AttributeId Id = 0x00000021;
} // namespace Language

namespace LEDSettings {
inline constexpr AttributeId Id = 0x00000022;
} // namespace LEDSettings

namespace AutoRelockTime {
inline constexpr AttributeId Id = 0x00000023;
} // namespace AutoRelockTime

namespace SoundVolume {
inline constexpr AttributeId Id = 0x00000024;
} // namespace SoundVolume

namespace OperatingMode {
inline constexpr AttributeId Id = 0x00000025;
} // namespace OperatingMode

namespace SupportedOperatingModes {
inline constexpr AttributeId Id = 0x00000026;
} // namespace SupportedOperatingModes

namespace DefaultConfigurationRegister {
inline constexpr AttributeId Id = 0x00000027;
} // namespace DefaultConfigurationRegister

namespace EnableLocalProgramming {
inline constexpr AttributeId Id = 0x00000028;
} // namespace EnableLocalProgramming

namespace EnableOneTouchLocking {
inline constexpr AttributeId Id = 0x00000029;
} // namespace EnableOneTouchLocking

namespace EnableInsideStatusLED {
inline constexpr AttributeId Id = 0x0000002A;
} // namespace EnableInsideStatusLED

namespace EnablePrivacyModeButton {
inline constexpr AttributeId Id = 0x0000002B;
} // namespace EnablePrivacyModeButton

namespace LocalProgrammingFeatures {
inline constexpr AttributeId Id = 0x0000002C;
} // namespace LocalProgrammingFeatures

namespace WrongCodeEntryLimit {
inline constexpr AttributeId Id = 0x00000030;
} // namespace WrongCodeEntryLimit

namespace UserCodeTemporaryDisableTime {
inline constexpr AttributeId Id = 0x00000031;
} // namespace UserCodeTemporaryDisableTime

namespace SendPINOverTheAir {
inline constexpr AttributeId Id = 0x00000032;
} // namespace SendPINOverTheAir

namespace RequirePINforRemoteOperation {
inline constexpr AttributeId Id = 0x00000033;
} // namespace RequirePINforRemoteOperation

namespace ExpiringUserTimeout {
inline constexpr AttributeId Id = 0x00000035;
} // namespace ExpiringUserTimeout

namespace AliroReaderVerificationKey {
inline constexpr AttributeId Id = 0x00000080;
} // namespace AliroReaderVerificationKey

namespace AliroReaderGroupIdentifier {
inline constexpr AttributeId Id = 0x00000081;
} // namespace AliroReaderGroupIdentifier

namespace AliroReaderGroupSubIdentifier {
inline constexpr AttributeId Id = 0x00000082;
} // namespace AliroReaderGroupSubIdentifier

namespace AliroExpeditedTransactionSupportedProtocolVersions {
inline constexpr AttributeId Id = 0x00000083;
} // namespace AliroExpeditedTransactionSupportedProtocolVersions

namespace AliroGroupResolvingKey {
inline constexpr AttributeId Id = 0x00000084;
} // namespace AliroGroupResolvingKey

namespace AliroSupportedBLEUWBProtocolVersions {
inline constexpr AttributeId Id = 0x00000085;
} // namespace AliroSupportedBLEUWBProtocolVersions

namespace AliroBLEAdvertisingVersion {
inline constexpr AttributeId Id = 0x00000086;
} // namespace AliroBLEAdvertisingVersion

namespace NumberOfAliroCredentialIssuerKeysSupported {
inline constexpr AttributeId Id = 0x00000087;
} // namespace NumberOfAliroCredentialIssuerKeysSupported

namespace NumberOfAliroEndpointKeysSupported {
inline constexpr AttributeId Id = 0x00000088;
} // namespace NumberOfAliroEndpointKeysSupported

namespace GeneratedCommandList {
inline constexpr AttributeId Id = Globals::Attributes::GeneratedCommandList::Id;
} // namespace GeneratedCommandList

namespace AcceptedCommandList {
inline constexpr AttributeId Id = Globals::Attributes::AcceptedCommandList::Id;
} // namespace AcceptedCommandList

namespace AttributeList {
inline constexpr AttributeId Id = Globals::Attributes::AttributeList::Id;
} // namespace AttributeList

namespace FeatureMap {
inline constexpr AttributeId Id = Globals::Attributes::FeatureMap::Id;
} // namespace FeatureMap

namespace ClusterRevision {
inline constexpr AttributeId Id = Globals::Attributes::ClusterRevision::Id;
} // namespace ClusterRevision

} // namespace Attributes
} // namespace DoorLock
} // namespace Clusters
} // namespace app
} // namespace chip
