// DO NOT EDIT MANUALLY - Generated file
//
// Identifier constant values for cluster DoorLock (cluster code: 257/0x101)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/common/GlobalIds.h>
#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace Clusters {
namespace DoorLock {

inline constexpr ClusterId Id = 0x00000101;

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
namespace EventList {
inline constexpr AttributeId Id = Globals::Attributes::EventList::Id;
} // namespace EventList
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

namespace Commands {
namespace LockDoor {
inline constexpr CommandId Id = 0x00000000;
} // namespace LockDoor
namespace UnlockDoor {
inline constexpr CommandId Id = 0x00000001;
} // namespace UnlockDoor
namespace UnlockWithTimeout {
inline constexpr CommandId Id = 0x00000003;
} // namespace UnlockWithTimeout
namespace SetWeekDaySchedule {
inline constexpr CommandId Id = 0x0000000B;
} // namespace SetWeekDaySchedule
namespace GetWeekDaySchedule {
inline constexpr CommandId Id = 0x0000000C;
} // namespace GetWeekDaySchedule
namespace ClearWeekDaySchedule {
inline constexpr CommandId Id = 0x0000000D;
} // namespace ClearWeekDaySchedule
namespace SetYearDaySchedule {
inline constexpr CommandId Id = 0x0000000E;
} // namespace SetYearDaySchedule
namespace GetYearDaySchedule {
inline constexpr CommandId Id = 0x0000000F;
} // namespace GetYearDaySchedule
namespace ClearYearDaySchedule {
inline constexpr CommandId Id = 0x00000010;
} // namespace ClearYearDaySchedule
namespace SetHolidaySchedule {
inline constexpr CommandId Id = 0x00000011;
} // namespace SetHolidaySchedule
namespace GetHolidaySchedule {
inline constexpr CommandId Id = 0x00000012;
} // namespace GetHolidaySchedule
namespace ClearHolidaySchedule {
inline constexpr CommandId Id = 0x00000013;
} // namespace ClearHolidaySchedule
namespace SetUser {
inline constexpr CommandId Id = 0x0000001A;
} // namespace SetUser
namespace GetUser {
inline constexpr CommandId Id = 0x0000001B;
} // namespace GetUser
namespace ClearUser {
inline constexpr CommandId Id = 0x0000001D;
} // namespace ClearUser
namespace SetCredential {
inline constexpr CommandId Id = 0x00000022;
} // namespace SetCredential
namespace GetCredentialStatus {
inline constexpr CommandId Id = 0x00000024;
} // namespace GetCredentialStatus
namespace ClearCredential {
inline constexpr CommandId Id = 0x00000026;
} // namespace ClearCredential
namespace UnboltDoor {
inline constexpr CommandId Id = 0x00000027;
} // namespace UnboltDoor
namespace SetAliroReaderConfig {
inline constexpr CommandId Id = 0x00000028;
} // namespace SetAliroReaderConfig
namespace ClearAliroReaderConfig {
inline constexpr CommandId Id = 0x00000029;
} // namespace ClearAliroReaderConfig
namespace GetWeekDayScheduleResponse {
inline constexpr CommandId Id = 0x0000000C;
} // namespace GetWeekDayScheduleResponse
namespace GetYearDayScheduleResponse {
inline constexpr CommandId Id = 0x0000000F;
} // namespace GetYearDayScheduleResponse
namespace GetHolidayScheduleResponse {
inline constexpr CommandId Id = 0x00000012;
} // namespace GetHolidayScheduleResponse
namespace GetUserResponse {
inline constexpr CommandId Id = 0x0000001C;
} // namespace GetUserResponse
namespace SetCredentialResponse {
inline constexpr CommandId Id = 0x00000023;
} // namespace SetCredentialResponse
namespace GetCredentialStatusResponse {
inline constexpr CommandId Id = 0x00000025;
} // namespace GetCredentialStatusResponse
} // namespace Commands

namespace Events {
namespace DoorLockAlarm {
inline constexpr EventId Id = 0x00000000;
} // namespace DoorLockAlarm
namespace DoorStateChange {
inline constexpr EventId Id = 0x00000001;
} // namespace DoorStateChange
namespace LockOperation {
inline constexpr EventId Id = 0x00000002;
} // namespace LockOperation
namespace LockOperationError {
inline constexpr EventId Id = 0x00000003;
} // namespace LockOperationError
namespace LockUserChange {
inline constexpr EventId Id = 0x00000004;
} // namespace LockUserChange
} // namespace Events

} // namespace DoorLock
} // namespace Clusters
} // namespace app
} // namespace chip
