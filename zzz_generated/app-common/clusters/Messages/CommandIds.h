// DO NOT EDIT MANUALLY - Generated file
//
// Identifier constant values for cluster Messages (cluster code: 151/0x97)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace Clusters {
namespace Messages {
namespace Commands {

inline constexpr uint32_t kAcceptedCommandsCount  = 2; // Total number of commands without a response supported by the cluster
inline constexpr uint32_t kGeneratedCommandsCount = 0; // Total number of commands with a response supported by the cluster

namespace PresentMessagesRequest {
inline constexpr CommandId Id = 0x00000000;
} // namespace PresentMessagesRequest

namespace CancelMessagesRequest {
inline constexpr CommandId Id = 0x00000001;
} // namespace CancelMessagesRequest

} // namespace Commands
} // namespace Messages
} // namespace Clusters
} // namespace app
} // namespace chip
