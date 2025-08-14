// DO NOT EDIT MANUALLY - Generated file
//
// Identifier constant values for cluster DoorLock (cluster code: 257/0x101)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace Clusters {
namespace DoorLock {
namespace Commands {

// Total number of client to server commands supported by the cluster
inline constexpr uint32_t kAcceptedCommandsCount = 21;

// Total number of server to client commands supported by the cluster (response commands)
inline constexpr uint32_t kGeneratedCommandsCount = 6;

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
} // namespace DoorLock
} // namespace Clusters
} // namespace app
} // namespace chip
