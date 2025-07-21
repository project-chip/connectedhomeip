// DO NOT EDIT MANUALLY - Generated file
//
// Identifier constant values for cluster CommissionerControl (cluster code: 1873/0x751)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace Clusters {
namespace CommissionerControl {
namespace Commands {

inline constexpr uint32_t kAcceptedCommandsCount  = 2; // Total number of commands without a response supported by the cluster
inline constexpr uint32_t kGeneratedCommandsCount = 1; // Total number of commands with a response supported by the cluster

namespace RequestCommissioningApproval {
inline constexpr CommandId Id = 0x00000000;
} // namespace RequestCommissioningApproval

namespace CommissionNode {
inline constexpr CommandId Id = 0x00000001;
} // namespace CommissionNode

namespace ReverseOpenCommissioningWindow {
inline constexpr CommandId Id = 0x00000002;
} // namespace ReverseOpenCommissioningWindow

} // namespace Commands
} // namespace CommissionerControl
} // namespace Clusters
} // namespace app
} // namespace chip
