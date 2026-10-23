// DO NOT EDIT MANUALLY - Generated file
//
// Identifier constant values for cluster TimeSynchronization (cluster code: 56/0x38)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace Clusters {
namespace TimeSynchronization {
namespace Commands {

// Total number of client to server commands supported by the cluster
inline constexpr uint32_t kAcceptedCommandsCount = 5;

// Total number of server to client commands supported by the cluster (response commands)
inline constexpr uint32_t kGeneratedCommandsCount = 1;

namespace SetUTCTime {
inline constexpr CommandId Id = 0x00000000;
} // namespace SetUTCTime

namespace SetTrustedTimeSource {
inline constexpr CommandId Id = 0x00000001;
} // namespace SetTrustedTimeSource

namespace SetTimeZone {
inline constexpr CommandId Id = 0x00000002;
} // namespace SetTimeZone

namespace SetDSTOffset {
inline constexpr CommandId Id = 0x00000004;
} // namespace SetDSTOffset

namespace SetDefaultNTP {
inline constexpr CommandId Id = 0x00000005;
} // namespace SetDefaultNTP

namespace SetTimeZoneResponse {
inline constexpr CommandId Id = 0x00000003;
} // namespace SetTimeZoneResponse

} // namespace Commands
} // namespace TimeSynchronization
} // namespace Clusters
} // namespace app
} // namespace chip
