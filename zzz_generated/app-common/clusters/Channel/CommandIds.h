// DO NOT EDIT MANUALLY - Generated file
//
// Identifier constant values for cluster Channel (cluster code: 1284/0x504)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace Clusters {
namespace Channel {
namespace Commands {

// Total number of client to server commands supported by the cluster
inline constexpr uint32_t kAcceptedCommandsCount = 6;

// Total number of server to client commands supported by the cluster (response commands)
inline constexpr uint32_t kGeneratedCommandsCount = 2;

namespace ChangeChannel {
inline constexpr CommandId Id = 0x00000000;
} // namespace ChangeChannel

namespace ChangeChannelByNumber {
inline constexpr CommandId Id = 0x00000002;
} // namespace ChangeChannelByNumber

namespace SkipChannel {
inline constexpr CommandId Id = 0x00000003;
} // namespace SkipChannel

namespace GetProgramGuide {
inline constexpr CommandId Id = 0x00000004;
} // namespace GetProgramGuide

namespace RecordProgram {
inline constexpr CommandId Id = 0x00000006;
} // namespace RecordProgram

namespace CancelRecordProgram {
inline constexpr CommandId Id = 0x00000007;
} // namespace CancelRecordProgram

namespace ChangeChannelResponse {
inline constexpr CommandId Id = 0x00000001;
} // namespace ChangeChannelResponse

namespace ProgramGuideResponse {
inline constexpr CommandId Id = 0x00000005;
} // namespace ProgramGuideResponse

} // namespace Commands
} // namespace Channel
} // namespace Clusters
} // namespace app
} // namespace chip
