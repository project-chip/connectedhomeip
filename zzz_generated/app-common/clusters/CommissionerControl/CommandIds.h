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

// Total number of client to server commands supported by the cluster
inline constexpr uint32_t kAcceptedCommandsCount = 2;

// Total number of server to client commands supported by the cluster (response commands)
inline constexpr uint32_t kGeneratedCommandsCount = 1;

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
