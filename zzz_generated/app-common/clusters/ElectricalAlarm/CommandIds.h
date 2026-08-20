// DO NOT EDIT MANUALLY - Generated file
//
// Identifier constant values for cluster ElectricalAlarm (cluster code: 161/0xA1)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace Clusters {
namespace ElectricalAlarm {
namespace Commands {

// Total number of client to server commands supported by the cluster
inline constexpr uint32_t kAcceptedCommandsCount = 3;

// Total number of server to client commands supported by the cluster (response commands)
inline constexpr uint32_t kGeneratedCommandsCount = 0;

namespace Reset {
inline constexpr CommandId Id = 0x00000000;
} // namespace Reset

namespace ModifyEnabledAlarms {
inline constexpr CommandId Id = 0x00000001;
} // namespace ModifyEnabledAlarms

namespace SetElectricalAlarmThresholds {
inline constexpr CommandId Id = 0x00000080;
} // namespace SetElectricalAlarmThresholds

} // namespace Commands
} // namespace ElectricalAlarm
} // namespace Clusters
} // namespace app
} // namespace chip
