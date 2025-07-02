// DO NOT EDIT MANUALLY - Generated file
//
// Identifier constant values for cluster Thermostat (cluster code: 513/0x201)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace Clusters {
namespace Thermostat {
namespace Commands {
namespace SetpointRaiseLower {
inline constexpr CommandId Id = 0x00000000;
} // namespace SetpointRaiseLower

namespace SetWeeklySchedule {
inline constexpr CommandId Id = 0x00000001;
} // namespace SetWeeklySchedule

namespace GetWeeklySchedule {
inline constexpr CommandId Id = 0x00000002;
} // namespace GetWeeklySchedule

namespace ClearWeeklySchedule {
inline constexpr CommandId Id = 0x00000003;
} // namespace ClearWeeklySchedule

namespace SetActiveScheduleRequest {
inline constexpr CommandId Id = 0x00000005;
} // namespace SetActiveScheduleRequest

namespace SetActivePresetRequest {
inline constexpr CommandId Id = 0x00000006;
} // namespace SetActivePresetRequest

namespace AddThermostatSuggestion {
inline constexpr CommandId Id = 0x00000007;
} // namespace AddThermostatSuggestion

namespace RemoveThermostatSuggestion {
inline constexpr CommandId Id = 0x00000008;
} // namespace RemoveThermostatSuggestion

namespace AtomicRequest {
inline constexpr CommandId Id = 0x000000FE;
} // namespace AtomicRequest

namespace GetWeeklyScheduleResponse {
inline constexpr CommandId Id = 0x00000000;
} // namespace GetWeeklyScheduleResponse

namespace AddThermostatSuggestionResponse {
inline constexpr CommandId Id = 0x00000002;
} // namespace AddThermostatSuggestionResponse

namespace AtomicResponse {
inline constexpr CommandId Id = 0x000000FD;
} // namespace AtomicResponse

} // namespace Commands
} // namespace Thermostat
} // namespace Clusters
} // namespace app
} // namespace chip
