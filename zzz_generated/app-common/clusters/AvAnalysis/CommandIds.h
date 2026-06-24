// DO NOT EDIT MANUALLY - Generated file
//
// Identifier constant values for cluster AvAnalysis (cluster code: 1367/0x557)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace Clusters {
namespace AvAnalysis {
namespace Commands {

// Total number of client to server commands supported by the cluster
inline constexpr uint32_t kAcceptedCommandsCount = 6;

// Total number of server to client commands supported by the cluster (response commands)
inline constexpr uint32_t kGeneratedCommandsCount = 1;

namespace EnableContextTriggers {
inline constexpr CommandId Id = 0x00000000;
} // namespace EnableContextTriggers

namespace DisableContextTriggers {
inline constexpr CommandId Id = 0x00000001;
} // namespace DisableContextTriggers

namespace EstablishAnalysisStream {
inline constexpr CommandId Id = 0x00000002;
} // namespace EstablishAnalysisStream

namespace ActivateAnalysisStream {
inline constexpr CommandId Id = 0x00000004;
} // namespace ActivateAnalysisStream

namespace DeactivateAnalysisStream {
inline constexpr CommandId Id = 0x00000005;
} // namespace DeactivateAnalysisStream

namespace RemoveAnalysisStream {
inline constexpr CommandId Id = 0x00000006;
} // namespace RemoveAnalysisStream

namespace EstablishAnalysisStreamResponse {
inline constexpr CommandId Id = 0x00000003;
} // namespace EstablishAnalysisStreamResponse

} // namespace Commands
} // namespace AvAnalysis
} // namespace Clusters
} // namespace app
} // namespace chip
